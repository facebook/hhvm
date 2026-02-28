#ifndef _EVENT_H_
#define _EVENT_H_
/* Copyright (c) 2004, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @defgroup Event_Scheduler Event Scheduler
  @ingroup Runtime_Environment
  @{

  @file sql/events.h

  A public interface of Events_Scheduler module.
*/

#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "my_inttypes.h"
#include "my_psi_config.h"
#include "my_time.h" /* interval_type */
#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/components/services/psi_thread_bits.h"
#include "mysql/psi/psi_memory.h"  // PSI_memory_key

class Event_db_repository;
class Event_parse_data;
class Event_queue;
class Event_scheduler;
class String;
class THD;

namespace dd {
class Schema;
}

struct CHARSET_INFO;

#ifdef HAVE_PSI_INTERFACE
extern PSI_mutex_key key_event_scheduler_LOCK_scheduler_state;
extern PSI_cond_key key_event_scheduler_COND_state;
extern PSI_thread_key key_thread_event_scheduler, key_thread_event_worker;
#endif /* HAVE_PSI_INTERFACE */

extern PSI_memory_key key_memory_event_basic_root;

/* Always defined, for SHOW PROCESSLIST. */
extern PSI_stage_info stage_waiting_on_empty_queue;
extern PSI_stage_info stage_waiting_for_next_activation;
extern PSI_stage_info stage_waiting_for_scheduler_to_stop;

int sortcmp_lex_string(LEX_CSTRING s, LEX_CSTRING t, CHARSET_INFO *cs);

/**
  @brief A facade to the functionality of the Event Scheduler.

  Every public operation against the scheduler has to be executed via the
  interface provided by a static method of this class. No instance of this
  class is ever created and it has no non-static data members.

  The life cycle of the Events module is the following:

  At server start up:
     init_mutexes() -> init()
  When the server is running:
     create_event(), drop_event(), start_or_stop_event_scheduler(), etc
  At shutdown:
     deinit(), destroy_mutexes().

  The peculiar initialization and shutdown cycle is an adaptation to the
  outside server startup/shutdown framework and mimics the rest of MySQL
  subsystems (ACL, time zone tables, etc).
*/

class Events {
 public:
  /*
    the following block is to support --event-scheduler command line option
    and the @@global.event_scheduler SQL variable.
    See sys_var.cc
  */
  enum enum_opt_event_scheduler { EVENTS_OFF, EVENTS_ON, EVENTS_DISABLED };
  /* Protected using LOCK_global_system_variables only. */
  static ulong opt_event_scheduler;
  static bool start(int *err_no);
  static bool stop();

  static bool init(bool opt_noacl);

  static void deinit();

  static void init_mutexes();

  static bool create_event(THD *thd, Event_parse_data *parse_data,
                           bool if_exists);

  static bool update_event(THD *thd, Event_parse_data *parse_data,
                           const LEX_CSTRING *new_dbname,
                           const LEX_CSTRING *new_name);

  static bool drop_event(THD *thd, LEX_CSTRING dbname, LEX_CSTRING name,
                         bool if_exists);

  static bool lock_schema_events(THD *thd, const dd::Schema &schema);

  static bool drop_schema_events(THD *thd, const dd::Schema &schema);

  static bool show_create_event(THD *thd, LEX_CSTRING dbname, LEX_CSTRING name);

  /* Needed for both SHOW CREATE EVENT and INFORMATION_SCHEMA */
  static int reconstruct_interval_expression(String *buf,
                                             interval_type interval,
                                             longlong expression);

  static void dump_internal_status();

  Events(const Events &) = delete;
  void operator=(Events &) = delete;

 private:
  static Event_queue *event_queue;
  static Event_scheduler *scheduler;
};

/**
  @} (end of group Event Scheduler)
*/

#endif /* _EVENT_H_ */
