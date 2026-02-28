/*
   Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "sql/conn_handler/connection_handler_manager.h"

#include <new>

#include "my_dbug.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_thd_wait.h"
#include "mysqld_error.h"                              // ER_*
#include "sql/conn_handler/channel_info.h"             // Channel_info
#include "sql/conn_handler/connection_handler_impl.h"  // Per_thread_connection_handler
#include "sql/conn_handler/plugin_connection_handler.h"  // Plugin_connection_handler
#include "sql/current_thd.h"
#include "sql/mysqld.h"        // max_connections
#include "sql/sql_callback.h"  // MYSQL_CALLBACK
#include "thr_lock.h"
#include "thr_mutex.h"

struct Connection_handler_functions;

// Initialize static members
uint Connection_handler_manager::connection_count = 0;
ulong Connection_handler_manager::max_used_connections = 0;
ulong Connection_handler_manager::max_used_connections_time = 0;
THD_event_functions *Connection_handler_manager::event_functions = nullptr;
THD_event_functions *Connection_handler_manager::saved_event_functions =
    nullptr;
mysql_mutex_t Connection_handler_manager::LOCK_connection_count;
mysql_cond_t Connection_handler_manager::COND_connection_count;
Connection_handler_manager *Connection_handler_manager::m_instance = nullptr;
ulong Connection_handler_manager::thread_handling =
    SCHEDULER_ONE_THREAD_PER_CONNECTION;
uint Connection_handler_manager::max_threads = 0;

// conn_handler is reused between set/reset calls. A thread could already
// be in process_new_connection and load m_connection_handler so deleting
// it would cause a segfault.
static Plugin_connection_handler plugin_conn_handler;

/**
  Helper functions to allow mysys to call the thread scheduler when
  waiting for locks.
*/

static void scheduler_wait_lock_begin() {
  MYSQL_CALLBACK(Connection_handler_manager::event_functions, thd_wait_begin,
                 (current_thd, THD_WAIT_TABLE_LOCK));
}

static void scheduler_wait_lock_end() {
  MYSQL_CALLBACK(Connection_handler_manager::event_functions, thd_wait_end,
                 (current_thd));
}

static void scheduler_wait_sync_begin() {
  MYSQL_CALLBACK(Connection_handler_manager::event_functions, thd_wait_begin,
                 (current_thd, THD_WAIT_SYNC));
}

static void scheduler_wait_sync_end() {
  MYSQL_CALLBACK(Connection_handler_manager::event_functions, thd_wait_end,
                 (current_thd));
}

bool Connection_handler_manager::valid_connection_count() {
  bool connection_accepted = true;
  mysql_mutex_lock(&LOCK_connection_count);
  if (connection_count > max_connections) {
    connection_accepted = false;
    m_connection_errors_max_connection++;
  }
  mysql_mutex_unlock(&LOCK_connection_count);
  return connection_accepted;
}

bool Connection_handler_manager::check_and_incr_conn_count(
    bool is_admin_connection) {
  bool connection_accepted = true;
  mysql_mutex_lock(&LOCK_connection_count);
  /*
    Here we allow max_connections + 1 clients to connect
    (by checking before we increment by 1).

    The last connection is reserved for SUPER users. This is
    checked later during authentication where valid_connection_count()
    is called for non-SUPER users only.
  */
  if (connection_count > max_connections && !is_admin_connection) {
    connection_accepted = false;
    m_connection_errors_max_connection++;
  } else {
    ++connection_count;

    if (connection_count > max_used_connections) {
      max_used_connections = connection_count;
      max_used_connections_time = (ulong)my_time(0);
    }
  }
  mysql_mutex_unlock(&LOCK_connection_count);
  return connection_accepted;
}

void Connection_handler_manager::dec_connection_count() {
  mysql_mutex_lock(&LOCK_connection_count);
  connection_count--;
  /*
    Notify shutdown thread when last connection is done with its job
  */
  if (connection_count == 0) mysql_cond_signal(&COND_connection_count);
  mysql_mutex_unlock(&LOCK_connection_count);
}

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_LOCK_connection_count;

static PSI_mutex_info all_conn_manager_mutexes[] = {
    {&key_LOCK_connection_count, "LOCK_connection_count", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};

static PSI_cond_key key_COND_connection_count;

static PSI_cond_info all_conn_manager_conds[] = {
    {&key_COND_connection_count, "COND_connection_count", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};
#endif

bool Connection_handler_manager::init() {
  /*
    This is a static member function.
    Per_thread_connection_handler's static members need to be initialized
    even if One_thread_connection_handler is used instead.
  */
  Per_thread_connection_handler::init();

  Connection_handler *connection_handler = nullptr;
  switch (Connection_handler_manager::thread_handling) {
    case SCHEDULER_ONE_THREAD_PER_CONNECTION:
      connection_handler = new (std::nothrow) Per_thread_connection_handler();
      break;
    case SCHEDULER_NO_THREADS:
      connection_handler = new (std::nothrow) One_thread_connection_handler();
      break;
    default:
      DBUG_ASSERT(false);
  }

  if (connection_handler == nullptr) {
    // This is a static member function.
    Per_thread_connection_handler::destroy();
    return true;
  }

  m_instance =
      new (std::nothrow) Connection_handler_manager(connection_handler);

  if (m_instance == nullptr) {
    delete connection_handler;
    // This is a static member function.
    Per_thread_connection_handler::destroy();
    return true;
  }

#ifdef HAVE_PSI_INTERFACE
  int count = static_cast<int>(array_elements(all_conn_manager_mutexes));
  mysql_mutex_register("sql", all_conn_manager_mutexes, count);

  count = static_cast<int>(array_elements(all_conn_manager_conds));
  mysql_cond_register("sql", all_conn_manager_conds, count);
#endif

  mysql_mutex_init(key_LOCK_connection_count, &LOCK_connection_count,
                   MY_MUTEX_INIT_FAST);

  mysql_cond_init(key_COND_connection_count, &COND_connection_count);

  max_threads = connection_handler->get_max_threads();

  // Init common callback functions.
  thr_set_lock_wait_callback(scheduler_wait_lock_begin,
                             scheduler_wait_lock_end);
  thr_set_sync_wait_callback(scheduler_wait_sync_begin,
                             scheduler_wait_sync_end);
  return false;
}

void Connection_handler_manager::wait_till_no_connection() {
  mysql_mutex_lock(&LOCK_connection_count);
  while (connection_count > 0) {
    mysql_cond_wait(&COND_connection_count, &LOCK_connection_count);
  }
  mysql_mutex_unlock(&LOCK_connection_count);
}

void Connection_handler_manager::destroy_instance() {
  Per_thread_connection_handler::destroy();

  if (m_instance != nullptr) {
    delete m_instance;
    m_instance = nullptr;
    mysql_mutex_destroy(&LOCK_connection_count);
    mysql_cond_destroy(&COND_connection_count);
  }
}

void Connection_handler_manager::reset_max_used_connections() {
  mysql_mutex_lock(&LOCK_connection_count);
  max_used_connections = connection_count;
  max_used_connections_time = (ulong)my_time(0);
  mysql_mutex_unlock(&LOCK_connection_count);
}

void Connection_handler_manager::load_connection_handler(
    Connection_handler *conn_handler) {
  // We don't support loading more than one dynamic connection handler
  DBUG_ASSERT(Connection_handler_manager::thread_handling !=
              SCHEDULER_TYPES_COUNT);
  m_saved_connection_handler = m_connection_handler;
  m_saved_thread_handling = Connection_handler_manager::thread_handling;
  m_connection_handler = conn_handler;
  Connection_handler_manager::thread_handling = SCHEDULER_TYPES_COUNT;
  max_threads = m_connection_handler->get_max_threads();
}

bool Connection_handler_manager::unload_connection_handler() {
  DBUG_ASSERT(m_saved_connection_handler != nullptr);
  if (m_saved_connection_handler == nullptr) return true;
  m_connection_handler = m_saved_connection_handler;
  Connection_handler_manager::thread_handling = m_saved_thread_handling;
  m_saved_connection_handler = nullptr;
  m_saved_thread_handling = 0;
  max_threads = m_connection_handler->get_max_threads();
  return false;
}

void Connection_handler_manager::process_new_connection(
    Channel_info *channel_info) {
  if (connection_events_loop_aborted() ||
      !check_and_incr_conn_count(channel_info->is_admin_connection())) {
    channel_info->send_error_and_close_channel(ER_CON_COUNT_ERROR, 0, true);
    delete channel_info;
    return;
  }

  if (m_connection_handler->add_connection(channel_info)) {
    inc_aborted_connects();
    delete channel_info;
  }
}

THD *create_thd(Channel_info *channel_info) {
  THD *thd = channel_info->create_thd();
  if (thd == nullptr)
    channel_info->send_error_and_close_channel(ER_OUT_OF_RESOURCES, 0, false);

  return thd;
}

void destroy_channel_info(Channel_info *channel_info) { delete channel_info; }

void dec_connection_count() {
  Connection_handler_manager::dec_connection_count();
}

void increment_aborted_connects() {
  Connection_handler_manager::get_instance()->inc_aborted_connects();
}

int my_connection_handler_set(Connection_handler_functions *chf,
                              THD_event_functions *tef) {
  DBUG_ASSERT(chf != nullptr && tef != nullptr);
  if (chf == nullptr || tef == nullptr) return 1;

  plugin_conn_handler.set_functions(chf);
  Connection_handler_manager::get_instance()->load_connection_handler(
      &plugin_conn_handler);
  Connection_handler_manager::saved_event_functions =
      Connection_handler_manager::event_functions;
  Connection_handler_manager::event_functions = tef;
  return 0;
}

int my_connection_handler_reset() {
  Connection_handler_manager::event_functions =
      Connection_handler_manager::saved_event_functions;
  bool error =
      Connection_handler_manager::get_instance()->unload_connection_handler();
  if (!error) {
    plugin_conn_handler.end_functions();
  }
  return error;
}
