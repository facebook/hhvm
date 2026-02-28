/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/mysqld_thd_manager.h"

#include "my_config.h"

#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <algorithm>
#include <functional>
#include <new>
#include <utility>

#include "mutex_lock.h"  // MUTEX_LOCK
#include "my_command.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/psi/psi_base.h"
#include "mysql/thread_pool_priv.h"  // inc_thread_created
#include "sql/rpl_master.h"          // unregister_slave
#include "sql/sql_class.h"           // THD
#include "thr_mutex.h"

std::atomic<uint> Global_THD_manager::atomic_global_thd_count{0U};
Global_THD_manager *Global_THD_manager::thd_manager = nullptr;

static inline int thd_partition(my_thread_id thread_id) {
  return thread_id % Global_THD_manager::NUM_PARTITIONS;
}

bool Find_thd_with_id::operator()(THD *thd) {
  if (thd->get_command() == COM_DAEMON) return false;
  if (thd->thread_id() == m_thread_id) {
    mysql_mutex_lock(&thd->LOCK_thd_data);
    return true;
  }
  return false;
}

/**
  Internal class used in do_for_all_thd() and do_for_all_thd_copy()
  implementation.
*/

class Do_THD {
 public:
  explicit Do_THD(Do_THD_Impl *impl) : m_impl(impl) {}

  /**
    Users of this class will override operator() in the _Impl class.

    @param thd THD of one element in global thread list
  */
  void operator()(THD *thd) { m_impl->operator()(thd); }

 private:
  Do_THD_Impl *m_impl;
};

/**
  Internal class used in find_thd() implementation.
*/

class Find_THD {
 public:
  explicit Find_THD(Find_THD_Impl *impl) : m_impl(impl) {}

  bool operator()(THD *thd) { return m_impl->operator()(thd); }

 private:
  Find_THD_Impl *m_impl;
};

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_LOCK_thd_list;
static PSI_mutex_key key_LOCK_thd_remove;
static PSI_mutex_key key_LOCK_thread_ids;

/* clang-format off */
static PSI_mutex_info all_thd_manager_mutexes[]=
{
  { &key_LOCK_thd_list, "LOCK_thd_list", 0, 0, PSI_DOCUMENT_ME},
  { &key_LOCK_thd_remove, "LOCK_thd_remove", 0, 0, PSI_DOCUMENT_ME},
  { &key_LOCK_thread_ids, "LOCK_thread_ids", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}
};
/* clang-format on */

static PSI_cond_key key_COND_thd_list;

static PSI_cond_info all_thd_manager_conds[] = {
    {&key_COND_thd_list, "COND_thd_list", 0, 0, PSI_DOCUMENT_ME}};
#endif  // HAVE_PSI_INTERFACE

const my_thread_id Global_THD_manager::reserved_thread_id = 0;

Global_THD_manager::Global_THD_manager()
  : thd_list {
      THD_array(PSI_INSTRUMENT_ME),
      THD_array(PSI_INSTRUMENT_ME),
      THD_array(PSI_INSTRUMENT_ME),
      THD_array(PSI_INSTRUMENT_ME),
      THD_array(PSI_INSTRUMENT_ME),
      THD_array(PSI_INSTRUMENT_ME),
      THD_array(PSI_INSTRUMENT_ME),
      THD_array(PSI_INSTRUMENT_ME),
    },
    thread_ids(PSI_INSTRUMENT_ME),
    atomic_num_thread_running(0),
    atomic_thread_created(0),
    atomic_num_thread_binlog_client(0),
    thread_id_counter(reserved_thread_id + 1),
    unit_test(false)
{
#ifdef HAVE_PSI_INTERFACE
  int count = static_cast<int>(array_elements(all_thd_manager_mutexes));
  mysql_mutex_register("sql", all_thd_manager_mutexes, count);

  count = static_cast<int>(array_elements(all_thd_manager_conds));
  mysql_cond_register("sql", all_thd_manager_conds, count);
#endif

  for (int i = 0; i < NUM_PARTITIONS; i++) {
    mysql_mutex_init(key_LOCK_thd_list, &LOCK_thd_list[i], MY_MUTEX_INIT_FAST);
    mysql_mutex_init(key_LOCK_thd_remove, &LOCK_thd_remove[i],
                     MY_MUTEX_INIT_FAST);
    mysql_cond_init(key_COND_thd_list, &COND_thd_list[i]);
  }

  mysql_mutex_init(key_LOCK_thread_ids, &LOCK_thread_ids, MY_MUTEX_INIT_FAST);

  // The reserved thread ID should never be used by normal threads,
  // so mark it as in-use. This ID is used by temporary THDs never
  // added to the list of THDs.
  thread_ids.push_back(reserved_thread_id);
}

Global_THD_manager::~Global_THD_manager() {
  thread_ids.erase_unique(reserved_thread_id);
  for (int i = 0; i < NUM_PARTITIONS; i++) {
    DBUG_ASSERT(thd_list[i].empty());
    mysql_mutex_destroy(&LOCK_thd_list[i]);
    mysql_mutex_destroy(&LOCK_thd_remove[i]);
    mysql_cond_destroy(&COND_thd_list[i]);
  }
  DBUG_ASSERT(thread_ids.empty());
  mysql_mutex_destroy(&LOCK_thread_ids);
}

/*
  Singleton Instance creation
  This method do not require mutex guard as it is called only from main thread.
*/
bool Global_THD_manager::create_instance() {
  if (thd_manager == nullptr)
    thd_manager = new (std::nothrow) Global_THD_manager();
  return (thd_manager == nullptr);
}

void Global_THD_manager::destroy_instance() {
  delete thd_manager;
  thd_manager = nullptr;
}

void Global_THD_manager::add_thd(THD *thd) {
  DBUG_PRINT("info", ("Global_THD_manager::add_thd %p", thd));
  // Should have an assigned ID before adding to the list.
  DBUG_ASSERT(thd->thread_id() != reserved_thread_id);
  const int partition = thd_partition(thd->thread_id());
  MUTEX_LOCK(lock_list, &LOCK_thd_list[partition]);
  // Technically it is not supported to compare pointers, but it works.
  std::pair<THD_array::iterator, bool> insert_result =
      thd_list[partition].insert_unique(thd);
  if (insert_result.second) ++atomic_global_thd_count;
  // Adding the same THD twice is an error.
  DBUG_ASSERT(insert_result.second);
}

void Global_THD_manager::remove_thd(THD *thd) {
  DBUG_PRINT("info", ("Global_THD_manager::remove_thd %p", thd));

  /*
    It is possible when a slave connection dies after register_slave,
    it may end up leaking SLAVE_INFO structure with a dangling THD pointer
    In most cases it may be freed by the same slave with the same server_id
    when it register itself again with the same server_id, but if the
    server_id ends up being different the entry could be leaked forever and
    with an invalid THD pointer
    To address this issue, we let the THD remove itself from the slave list
  */
  unregister_slave(thd, true, true);

  const int partition = thd_partition(thd->thread_id());
  MUTEX_LOCK(lock_remove, &LOCK_thd_remove[partition]);
  MUTEX_LOCK(lock_list, &LOCK_thd_list[partition]);

  DBUG_ASSERT(unit_test || thd->release_resources_done());

  /*
    Used by binlog_reset_master.  It would be cleaner to use
    DEBUG_SYNC here, but that's not possible because the THD's debug
    sync feature has been shut down at this point.
  */
  DBUG_EXECUTE_IF("sleep_after_lock_thread_count_before_delete_thd", sleep(5););

  const size_t num_erased = thd_list[partition].erase_unique(thd);
  if (num_erased == 1) --atomic_global_thd_count;
  // Removing a THD that was never added is an error.
  DBUG_ASSERT(1 == num_erased);
  mysql_cond_broadcast(&COND_thd_list[partition]);
}

my_thread_id Global_THD_manager::get_new_thread_id() {
  my_thread_id new_id;
  MUTEX_LOCK(lock, &LOCK_thread_ids);
  do {
    new_id = thread_id_counter++;
  } while (!thread_ids.insert_unique(new_id).second);
  return new_id;
}

void Global_THD_manager::release_thread_id(my_thread_id thread_id) {
  if (thread_id == reserved_thread_id)
    return;  // Some temporary THDs are never given a proper ID.
  MUTEX_LOCK(lock, &LOCK_thread_ids);
  const size_t num_erased MY_ATTRIBUTE((unused)) =
      thread_ids.erase_unique(thread_id);
  // Assert if the ID was not found in the list.
  DBUG_ASSERT(1 == num_erased);
}

void Global_THD_manager::set_thread_id_counter(my_thread_id new_id) {
  DBUG_ASSERT(unit_test == true);
  MUTEX_LOCK(lock, &LOCK_thread_ids);
  thread_id_counter = new_id;
}

void Global_THD_manager::wait_till_no_thd() {
  for (int i = 0; i < NUM_PARTITIONS; i++) {
    MUTEX_LOCK(lock, &LOCK_thd_list[i]);
    while (thd_list[i].size() > 0) {
      mysql_cond_wait(&COND_thd_list[i], &LOCK_thd_list[i]);
      DBUG_PRINT("quit", ("One thread died (count=%u)", get_thd_count()));
    }
  }
}

void Global_THD_manager::do_for_all_thd_copy(Do_THD_Impl *func) {
  Do_THD doit(func);

  for (int i = 0; i < NUM_PARTITIONS; i++) {
    MUTEX_LOCK(lock_remove, &LOCK_thd_remove[i]);
    mysql_mutex_lock(&LOCK_thd_list[i]);

    /* Take copy of global_thread_list. */
    THD_array thd_list_copy(thd_list[i]);

    /*
      Allow inserts to global_thread_list. Newly added thd
      will not be accounted for when executing func.
    */
    mysql_mutex_unlock(&LOCK_thd_list[i]);

    /* Execute func for all existing threads. */
    std::for_each(thd_list_copy.begin(), thd_list_copy.end(), doit);

    DEBUG_SYNC_C("inside_do_for_all_thd_copy");
  }
}

void Global_THD_manager::do_for_all_thd(Do_THD_Impl *func) {
  Do_THD doit(func);
  for (int i = 0; i < NUM_PARTITIONS; i++) {
    MUTEX_LOCK(lock, &LOCK_thd_list[i]);
    std::for_each(thd_list[i].begin(), thd_list[i].end(), doit);
  }
}

THD *Global_THD_manager::find_thd(Find_THD_Impl *func) {
  Find_THD find_thd(func);
  for (int i = 0; i < NUM_PARTITIONS; i++) {
    MUTEX_LOCK(lock, &LOCK_thd_list[i]);
    THD_array::const_iterator it =
        std::find_if(thd_list[i].begin(), thd_list[i].end(), find_thd);
    if (it != thd_list[i].end()) return (*it);
  }
  return nullptr;
}

// Optimized version of the above function for when we know
// the thread_id of the THD we are looking for.
THD *Global_THD_manager::find_thd(Find_thd_with_id *func) {
  Find_THD find_thd(func);
  // Since we know the thread_id, we can check the correct
  // partition directly.
  const int partition = thd_partition(func->m_thread_id);
  MUTEX_LOCK(lock, &LOCK_thd_list[partition]);
  THD_array::const_iterator it = std::find_if(
      thd_list[partition].begin(), thd_list[partition].end(), find_thd);
  if (it != thd_list[partition].end()) return (*it);
  return nullptr;
}

void inc_thread_created() {
  Global_THD_manager::get_instance()->inc_thread_created();
}

void thd_lock_thread_count() {
  for (int i = 0; i < Global_THD_manager::NUM_PARTITIONS; i++)
    mysql_mutex_lock(&Global_THD_manager::get_instance()->LOCK_thd_list[i]);
}

void thd_unlock_thread_count() {
  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  for (int i = 0; i < Global_THD_manager::NUM_PARTITIONS; i++) {
    mysql_cond_broadcast(&thd_manager->COND_thd_list[i]);
    mysql_mutex_unlock(&thd_manager->LOCK_thd_list[i]);
  }
}

template <typename T>
class Run_free_function : public Do_THD_Impl {
 public:
  typedef void(do_thd_impl)(THD *, T);

  Run_free_function(do_thd_impl *f, T arg) : m_func(f), m_arg(arg) {}

  virtual void operator()(THD *thd) { (*m_func)(thd, m_arg); }

 private:
  do_thd_impl *m_func;
  T m_arg;
};

void do_for_all_thd(do_thd_impl_uint64 f, uint64 v) {
  Run_free_function<uint64> runner(f, v);
  Global_THD_manager::get_instance()->do_for_all_thd(&runner);
}
