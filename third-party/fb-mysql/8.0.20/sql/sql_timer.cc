/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_timer.h"

#include <stddef.h>
#include <atomic>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "my_timer.h"  // my_timer_t
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "sql/mysqld.h"              // key_thd_timer_mutex
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/psi_memory_key.h"      // key_memory_thd_timer
#include "sql/sql_class.h"           // THD
#include "thr_mutex.h"

/**
  Cast a member of a structure to the structure that contains it.

  @param  ptr     Pointer to the member.
  @param  type    Type of the structure that contains the member.
  @param  member  Name of the member within the structure.
*/
#define my_container_of(ptr, type, member) \
  ((type *)((char *)ptr - offsetof(type, member)))

struct THD_timer_info {
  my_thread_id thread_id;
  my_timer_t timer;
  mysql_mutex_t mutex;
  bool destroy;
};

static void timer_callback(my_timer_t *);

/**
  Allocate and initialize a thread timer object.

  @return NULL on failure.
*/

static THD_timer_info *thd_timer_create(void) {
  THD_timer_info *thd_timer;
  DBUG_TRACE;

  thd_timer = (THD_timer_info *)my_malloc(key_memory_thd_timer,
                                          sizeof(THD_timer_info), MYF(MY_WME));

  if (thd_timer == nullptr) return nullptr;

  thd_timer->thread_id = 0;
  mysql_mutex_init(key_thd_timer_mutex, &thd_timer->mutex, MY_MUTEX_INIT_FAST);
  thd_timer->destroy = false;
  thd_timer->timer.notify_function = timer_callback;

  if (DBUG_EVALUATE_IF("thd_timer_create_failure", 0, 1) &&
      !my_timer_create(&thd_timer->timer))
    return thd_timer;

  mysql_mutex_destroy(&thd_timer->mutex);
  my_free(thd_timer);

  return nullptr;
}

/**
  Notify a thread (session) that its timer has expired.

  @param  thd_timer   Thread timer object.

  @return true if the object should be destroyed.
*/

static bool timer_notify(THD_timer_info *thd_timer) {
  Find_thd_with_id find_thd_with_id(thd_timer->thread_id);
  THD *thd = Global_THD_manager::get_instance()->find_thd(&find_thd_with_id);

  DBUG_ASSERT(!thd_timer->destroy || !thd_timer->thread_id);
  /*
    Statement might have finished while the timer notification
    was being delivered. If this is the case, the timer object
    was detached (orphaned) and has no associated session (thd).
  */
  if (thd) {
    /* process only if thread is not already undergoing any kill connection. */
    if (thd->killed != THD::KILL_CONNECTION) {
      thd->awake(THD::KILL_TIMEOUT);
    }
    mysql_mutex_unlock(&thd->LOCK_thd_data);
  }

  /* Mark the object as unreachable. */
  thd_timer->thread_id = 0;

  return thd_timer->destroy;
}

/**
  Timer expiration notification callback.

  @param  timer   Timer (mysys) object.

  @note Invoked in a separate thread of control.
*/

static void timer_callback(my_timer_t *timer) {
  bool destroy;
  THD_timer_info *thd_timer;

  thd_timer = my_container_of(timer, THD_timer_info, timer);

  mysql_mutex_lock(&thd_timer->mutex);
  destroy = timer_notify(thd_timer);
  mysql_mutex_unlock(&thd_timer->mutex);

  if (destroy) thd_timer_destroy(thd_timer);
}

/**
  Set the time until the currently running statement is aborted.

  @param  thd         Thread (session) context.
  @param  thd_timer   Thread timer object.
  @param  time        Length of time, in milliseconds, until the currently
                      running statement is aborted.

  @return NULL on failure.
*/

THD_timer_info *thd_timer_set(THD *thd, THD_timer_info *thd_timer,
                              unsigned long time) {
  DBUG_TRACE;

  /* Create a new thread timer object if one was not provided. */
  if (thd_timer == nullptr && (thd_timer = thd_timer_create()) == nullptr)
    return nullptr;

  DBUG_ASSERT(!thd_timer->destroy && !thd_timer->thread_id);

  /* Mark the notification as pending. */
  thd_timer->thread_id = thd->thread_id();

  /* Arm the timer. */
  if (DBUG_EVALUATE_IF("thd_timer_set_failure", 0, 1) &&
      !my_timer_set(&thd_timer->timer, time))
    return thd_timer;

  /* Dispose of the (cached) timer object. */
  thd_timer_destroy(thd_timer);

  return nullptr;
}

/**
  Reap a (possibly) pending timer object.

  @param  thd_timer   Thread timer object.
  @param  pending

  @return true if the timer object is unreachable.
*/

static bool reap_timer(THD_timer_info *thd_timer, bool pending) {
  /* Cannot be tagged for destruction. */
  DBUG_ASSERT(!thd_timer->destroy);

  /* If not pending, timer hasn't fired. */
  DBUG_ASSERT(pending || thd_timer->thread_id);

  /*
    The timer object can be reused if the timer was stopped before
    expiring. Otherwise, the timer notification function might be
    executing asynchronously in the context of a separate thread.
  */
  bool unreachable = pending ? thd_timer->thread_id == 0 : true;

  thd_timer->thread_id = 0;

  return unreachable;
}

/**
  Deactivate the given timer.

  @param  thd_timer   Thread timer object.

  @return NULL if the timer object was orphaned.
          Otherwise, the given timer object is returned.
*/

THD_timer_info *thd_timer_reset(THD_timer_info *thd_timer) {
  bool unreachable;
  int status, state;
  DBUG_TRACE;

  status = my_timer_cancel(&thd_timer->timer, &state);

  /*
    If the notification function cannot possibly run anymore, cache
    the timer object as there are no outstanding references to it.
  */
  mysql_mutex_lock(&thd_timer->mutex);
  unreachable = reap_timer(thd_timer, status ? true : !state);
  thd_timer->destroy = !unreachable;
  mysql_mutex_unlock(&thd_timer->mutex);

  return unreachable ? thd_timer : nullptr;
}

/**
  Release resources allocated for a thread timer.

  @param  thd_timer   Thread timer object.
*/

void thd_timer_destroy(THD_timer_info *thd_timer) {
  DBUG_TRACE;

  my_timer_delete(&thd_timer->timer);
  mysql_mutex_destroy(&thd_timer->mutex);
  my_free(thd_timer);
}
