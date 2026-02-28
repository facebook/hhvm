#ifndef _EVENT_SCHEDULER_H_
#define _EVENT_SCHEDULER_H_
/* Copyright (c) 2004, 2018, Oracle and/or its affiliates. All rights reserved.

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

/**
  @addtogroup Event_Scheduler
  @{
*/
/**
  @file

  Declarations of the scheduler thread class
  and related functionality.

  This file is internal to Event_Scheduler module. Please do not
  include it directly.  All public declarations of Event_Scheduler
  module are in events.h and event_data_objects.h.
*/

#include <sys/types.h>

#include "my_inttypes.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_stage_bits.h"

class Event_db_repository;
class Event_job_data;
class Event_queue;
class Event_queue_element_for_exec;
class THD;

void pre_init_event_thread(THD *thd);

bool post_init_event_thread(THD *thd);

void deinit_event_thread(THD *thd);

class Event_worker_thread {
 public:
  void run(THD *thd, Event_queue_element_for_exec *event);

 private:
  void print_warnings(THD *thd, Event_job_data *et);
};

class Event_scheduler {
 public:
  Event_scheduler(Event_queue *event_queue_arg);
  ~Event_scheduler();

  /* State changing methods follow */

  bool start(int *err_no);

  bool stop();

  /*
    Need to be public because has to be called from the function
    passed to my_thread_create.
  */
  bool run(THD *thd);

  /* Information retrieving methods follow */
  bool is_running();

  void dump_internal_status();

 private:
  int workers_count();

  /* helper functions */
  bool execute_top(Event_queue_element_for_exec *event_name);

  /* helper functions for working with mutexes & conditionals */
  void lock_data(const char *func, uint line);

  void unlock_data(const char *func, uint line);

  void cond_wait(THD *thd, struct timespec *abstime,
                 const PSI_stage_info *stage, const char *src_func,
                 const char *src_file, uint src_line);

  mysql_mutex_t LOCK_scheduler_state;

  enum enum_state { INITIALIZED = 0, RUNNING, STOPPING };

  /* This is the current status of the life-cycle of the scheduler. */
  enum enum_state state;

  THD *scheduler_thd;

  mysql_cond_t COND_state;

  Event_queue *queue;

  uint mutex_last_locked_at_line;
  uint mutex_last_unlocked_at_line;
  const char *mutex_last_locked_in_func;
  const char *mutex_last_unlocked_in_func;
  bool mutex_scheduler_data_locked;
  bool waiting_on_cond;

  ulonglong started_events;

 private:
  // Disallow copy construction and assignment.
  Event_scheduler(const Event_scheduler &) = delete;
  void operator=(Event_scheduler &) = delete;
};

/**
  @} (End of group Event_Scheduler)
*/

#endif /* _EVENT_SCHEDULER_H_ */
