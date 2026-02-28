/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/mdl_context_backup.h"
#include "mutex_lock.h"              // MUTEX_LOCK
#include "mysql/psi/mysql_memory.h"  // mysql_memory_register
#include "mysqld_error.h"            // ER_*
#include "sql/sql_const.h"           // LONG_TIMEOUT

/**
  Wrapper around MDL_context class which allows to store it in backup
  manager's collection. Also implements MDL_context_owner interface and serves
  as owner for stored MDL_context.
*/

class MDL_context_backup_manager::MDL_context_backup
    : public MDL_context_owner {
 public:
  MDL_context_backup() { m_context.init(this); }

  ~MDL_context_backup() override {
    DBUG_ASSERT(!m_context.has_locks(MDL_EXPLICIT));
    DBUG_ASSERT(!m_context.has_locks(MDL_STATEMENT));

    m_context.release_transactional_locks();
    m_context.destroy();
  }

 public:
  MDL_context *get_context() { return &m_context; }

 private:
  // Begin implementation of MDL_context_owner interface.
  void enter_cond(mysql_cond_t *, mysql_mutex_t *, const PSI_stage_info *,
                  PSI_stage_info *, const char *, const char *, int) override {
    DBUG_TRACE;
    return;
  }

  void exit_cond(const PSI_stage_info *, const char *, const char *,
                 int) override {
    DBUG_TRACE;
    return;
  }

  int is_killed() const override { return false; }

  /**
    @warning Since there is no THD associated with This method returns nullptr
    unconditionally.
   */
  THD *get_thd() override { return nullptr; }

  void notify_shared_lock(MDL_context_owner *, bool) override { return; }

  bool kill_shared_lock(MDL_context_owner *) override { return false; }

  bool notify_hton_pre_acquire_exclusive(const MDL_key *, bool *) override {
    return false;
  }

  void notify_hton_post_release_exclusive(const MDL_key *) override { return; }

  uint get_rand_seed() const override { return 1; }

  bool is_connected() override { return true; }
  // End implementation of MDL_context_owner interface.

 private:
  // MDL_context member to hold locks
  MDL_context m_context;
};

MDL_context_backup_manager *MDL_context_backup_manager::m_single = nullptr;

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_LOCK_mdl_context_backup_manager;
static PSI_mutex_info mdl_context_backup_manager_mutexes[] = {
    {&key_LOCK_mdl_context_backup_manager, "LOCK_mdl_context_backup_manager",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};

static PSI_memory_key key_memory_mdl_context_backup_manager;
static PSI_memory_info mdl_context_backup_manager_memory[] = {
    {&key_memory_mdl_context_backup_manager, "MDL_context_backup_manager", 0, 0,
     PSI_DOCUMENT_ME}};

void MDL_context_backup_manager::init_psi_keys(void) {
  const char *category = "sql";
  int count = array_elements(mdl_context_backup_manager_mutexes);
  mysql_mutex_register(category, mdl_context_backup_manager_mutexes, count);

  count = static_cast<int>(array_elements(mdl_context_backup_manager_memory));
  mysql_memory_register(category, mdl_context_backup_manager_memory, count);
}
#endif /* HAVE_PSI_INTERFACE */

MDL_context_backup_manager::MDL_context_backup_manager(PSI_memory_key key)
    : m_backup_map(
          std::less<MDL_context_backup_key>(),
          Malloc_allocator<std::pair<const MDL_context_backup_key,
                                     std::unique_ptr<MDL_context_backup>>>(
              key)) {
#ifdef HAVE_PSI_INTERFACE
  init_psi_keys();
#endif
  mysql_mutex_init(key_LOCK_mdl_context_backup_manager,
                   &m_LOCK_mdl_context_backup, MY_MUTEX_INIT_SLOW);
}

bool MDL_context_backup_manager::init() {
  DBUG_TRACE;
  m_single = new (std::nothrow)
      MDL_context_backup_manager(key_memory_mdl_context_backup_manager);
  return m_single == nullptr;
}

MDL_context_backup_manager &MDL_context_backup_manager::instance() {
  DBUG_ASSERT(m_single != nullptr);
  return *m_single;
}

void MDL_context_backup_manager::destroy() {
  DBUG_TRACE;
  delete m_single;
  m_single = nullptr;
}

MDL_context_backup_manager::~MDL_context_backup_manager() {
  mysql_mutex_destroy(&m_LOCK_mdl_context_backup);
}

bool MDL_context_backup_manager::check_key_exist(
    const MDL_context_backup_key &key_obj) {
  MUTEX_LOCK(guard, &m_LOCK_mdl_context_backup);
  return m_backup_map.find(key_obj) != m_backup_map.end();
}

bool MDL_context_backup_manager::create_backup(const MDL_context *context,
                                               const uchar *key,
                                               const size_t keylen) {
  DBUG_TRACE;

  bool result = false;
  try {
    MDL_context_backup_key key_obj(key, keylen);

    /*
      Since this method is called as part of THD cleaning up, every XA
      transaction must be present in the m_backup_map only once.
      In other words, it mustn't be present any element for specified xid
      when this method called. Check that this invariant is satisfied.
    */
    DBUG_ASSERT(!check_key_exist(key_obj));

    std::unique_ptr<MDL_context_backup> element(new (std::nothrow)
                                                    MDL_context_backup());

    if (element == nullptr) {
      my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), sizeof(element));
      return true;
    }

    if (element->get_context()->clone_tickets(context, MDL_TRANSACTION))
      return true;

    MUTEX_LOCK(guard, &m_LOCK_mdl_context_backup);
    m_backup_map.emplace(key_obj, std::move(element));
  } catch (std::bad_alloc &ex) {
    result = true;
  }
  return result;
}

bool MDL_context_backup_manager::create_backup(MDL_request_list *mdl_requests,
                                               const uchar *key,
                                               const size_t keylen) {
  DBUG_TRACE;

  bool result = false;
  try {
    MDL_context_backup_key key_obj(key, keylen);
    /*
      Check for presence a record with specified key in the collection of
      MDL_context_backup elements. It is ok to already have a record with
      the same xid value in this collection. Presence of a element with the
      same value of xid at the moment of invocation the method create_backup()
      is stipulated by the fact that ha_recover() can be called twice (and
      hence the same list of prepared XA transactions is processed twice).
      First time it can be called from transaction coordinator (that is either
      from binlog or tc_log_mmap) and second time it is called unconditionally
      from init_server_components(). Therefore presence of an entry in the
      cache at the moment when the method create_backup() is called second time
      shouldn't be considered as error.
    */
    if (check_key_exist(key_obj)) return false;

    std::unique_ptr<MDL_context_backup> element(new (std::nothrow)
                                                    MDL_context_backup());

    if (element == nullptr) {
      my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), sizeof(element));
      return true;
    }

    if (element->get_context()->acquire_locks_nsec(mdl_requests,
                                                   LONG_TIMEOUT_NSEC))
      return true;

    MUTEX_LOCK(guard, &m_LOCK_mdl_context_backup);
    m_backup_map.emplace(key_obj, std::move(element));

  } catch (std::bad_alloc &ex) {
    result = true;
  }

  return result;
}

bool MDL_context_backup_manager::restore_backup(MDL_context *mdl_context,
                                                const uchar *key,
                                                const size_t keylen) {
  bool res = false;
  MDL_context_backup *element;
  DBUG_TRACE;

  MUTEX_LOCK(guard, &m_LOCK_mdl_context_backup);

  auto result = m_backup_map.find(MDL_context_backup_key(key, keylen));
  if (result != m_backup_map.end()) {
    element = result->second.get();
    res = mdl_context->clone_tickets(element->get_context(), MDL_TRANSACTION);
  }

  return res;
}

void MDL_context_backup_manager::delete_backup(const uchar *key,
                                               const size_t keylen) {
  DBUG_TRACE;
  MUTEX_LOCK(guard, &m_LOCK_mdl_context_backup);
  m_backup_map.erase(MDL_context_backup_key(key, keylen));
}
