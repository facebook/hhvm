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

/*
 * sql_manager.cc
 * This thread manages various maintenance tasks.
 *
 *   o Flushing the tables every flush_time seconds.
 */

#include "sql/sql_manager.h"

#include <errno.h>
#include <sys/types.h>
#include <time.h>

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_systime.h"
#include "my_thread.h"  // my_thread_t
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/log.h"
#include "sql/mysqld.h"    // flush_time
#include "sql/sql_base.h"  // tdc_flush_unused_tables
#include "sql/sql_class.h"

static bool volatile manager_thread_in_use;
static bool abort_manager;

my_thread_t manager_thread;
mysql_mutex_t LOCK_manager;
mysql_cond_t COND_manager;

extern "C" {
static void *handle_manager(void *arg MY_ATTRIBUTE((unused))) {
  int error = 0;
  struct timespec abstime;
  bool reset_flush_time = true;

  my_thread_init();
  {
    DBUG_TRACE;

    THD *thd = new THD;
    thd->set_new_thread_id();
    thd->thread_stack = (char *)&thd;
    thd->store_globals();

    manager_thread = my_thread_self();
    manager_thread_in_use = true;

    for (;;) {
      mysql_mutex_lock(&LOCK_manager);
      /* XXX: This will need to be made more general to handle different
       * polling needs. */
      if (flush_time) {
        if (reset_flush_time) {
          set_timespec(&abstime, flush_time);
          reset_flush_time = false;
        }
        while ((!error || error == EINTR) && !abort_manager)
          error = mysql_cond_timedwait(&COND_manager, &LOCK_manager, &abstime);
      } else {
        while ((!error || error == EINTR) && !abort_manager)
          error = mysql_cond_wait(&COND_manager, &LOCK_manager);
      }
      mysql_mutex_unlock(&LOCK_manager);

      if (abort_manager) break;

      if (is_timeout(error)) {
        tdc_flush_unused_tables();
        error = 0;
        reset_flush_time = true;
      }
    }
    manager_thread_in_use = false;
    thd->release_resources();
    delete thd;
  }  // Can't use DBUG_RETURN after my_thread_end
  my_thread_end();
  return (nullptr);
}
}  // extern "C"

/* Start handle manager thread */
void start_handle_manager() {
  DBUG_TRACE;
  abort_manager = false;
  if (flush_time && flush_time != ~(ulong)0L) {
    my_thread_handle hThread;
    int error;
    if ((error =
             mysql_thread_create(key_thread_handle_manager, &hThread,
                                 &connection_attrib, handle_manager, nullptr)))
      LogErr(WARNING_LEVEL, ER_CANT_CREATE_HANDLE_MGR_THREAD, error);
  }
}

/* Initiate shutdown of handle manager thread */
void stop_handle_manager() {
  DBUG_TRACE;
  abort_manager = true;
  mysql_mutex_lock(&LOCK_manager);
  if (manager_thread_in_use) {
    DBUG_PRINT("quit", ("initiate shutdown of handle manager thread: 0x%lx",
                        (ulong)manager_thread));
    mysql_cond_signal(&COND_manager);
  }
  mysql_mutex_unlock(&LOCK_manager);
}
