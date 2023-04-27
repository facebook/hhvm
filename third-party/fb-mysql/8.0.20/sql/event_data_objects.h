#ifndef _EVENT_DATA_OBJECTS_H_
#define _EVENT_DATA_OBJECTS_H_
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
  @addtogroup Event_Scheduler
  @{

  @file event_data_objects.h
*/

#include <sys/types.h>

#include "lex_string.h"
#include "my_alloc.h"  // MEM_ROOT
#include "my_inttypes.h"
#include "my_psi_config.h"
#include "my_time.h"  // interval_type
#include "mysql/components/services/psi_statement_bits.h"

class String;
class THD;
class Time_zone;

typedef ulonglong sql_mode_t;
namespace dd {
class Event;
}

void init_scheduler_psi_keys(void);

class Event_queue_element_for_exec {
 public:
  Event_queue_element_for_exec() {}
  ~Event_queue_element_for_exec();

  bool init(LEX_CSTRING dbname, LEX_CSTRING name);

  LEX_CSTRING dbname;
  LEX_CSTRING name;
  bool dropped;
  THD *thd;

  void claim_memory_ownership();

  /* Prevent use of these */
  Event_queue_element_for_exec(const Event_queue_element_for_exec &) = delete;
  void operator=(Event_queue_element_for_exec &) = delete;

#ifdef HAVE_PSI_INTERFACE
  PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

class Event_basic {
 protected:
  MEM_ROOT mem_root;

 public:
  LEX_CSTRING m_schema_name;
  LEX_CSTRING m_event_name;
  LEX_CSTRING m_definer;

  Time_zone *m_time_zone;
  Event_basic();
  virtual ~Event_basic();
  virtual bool fill_event_info(THD *thd, const dd::Event &ev_obj,
                               const char *dbname) = 0;
};

class Event_queue_element : public Event_basic {
 public:
  int m_on_completion;
  int m_status;
  longlong m_originator;

  my_time_t m_last_executed;
  my_time_t m_execute_at;
  my_time_t m_starts;
  my_time_t m_ends;
  bool m_starts_null;
  bool m_ends_null;
  bool m_execute_at_null;

  longlong m_expression;
  interval_type m_interval;

  bool m_dropped;

  uint m_execution_count;

  Event_queue_element();
  virtual ~Event_queue_element();
  virtual bool fill_event_info(THD *thd, const dd::Event &event,
                               const char *dbname);

  bool compute_next_execution_time(THD *thd);

  void mark_last_executed(THD *thd);
};

class Event_timed : public Event_queue_element {
 public:
  LEX_STRING m_definition;

  LEX_CSTRING m_definer_user;
  LEX_CSTRING m_definer_host;

  LEX_STRING m_comment;

  ulonglong m_created;
  ulonglong m_modified;

  sql_mode_t m_sql_mode;

  class Stored_program_creation_ctx *m_creation_ctx;
  LEX_STRING m_definition_utf8;
  Event_timed();
  virtual ~Event_timed();

  void init();

  virtual bool fill_event_info(THD *thd, const dd::Event &event,
                               const char *schema_name);

  int get_create_event(const THD *thd, String *buf);

  Event_timed(const Event_timed &) = delete;
  void operator=(Event_timed &) = delete;
};

class Event_job_data : public Event_basic {
 public:
  LEX_STRING m_definition;
  LEX_CSTRING m_definer_user;
  LEX_CSTRING m_definer_host;

  sql_mode_t m_sql_mode;

  class Stored_program_creation_ctx *m_creation_ctx;

  Event_job_data();

  bool execute(THD *thd, bool drop);

  Event_job_data(const Event_job_data &) = delete;
  void operator=(Event_job_data &) = delete;

 private:
  virtual bool fill_event_info(THD *thd, const dd::Event &event,
                               const char *schema_name);
  bool construct_sp_sql(THD *thd, String *sp_sql);
};

/**
  Build an SQL drop event string.

  @param[in]     thd         Thread handle
  @param[in,out] sp_sql      Pointer to String object where the SQL query will
                             be stored
  @param[in]     db_name     The schema name
  @param[in]     event_name  The event name

  @retval        false       The drop event SQL query is built
  @retval        true        Otherwise
*/
bool construct_drop_event_sql(THD *thd, String *sp_sql, LEX_CSTRING db_name,
                              LEX_CSTRING event_name);

/* Compares only the schema part of the identifier */
bool event_basic_db_equal(LEX_CSTRING db, Event_basic *et);

/* Compares the whole identifier*/
bool event_basic_identifier_equal(LEX_CSTRING db, LEX_CSTRING name,
                                  Event_basic *b);

/**
  @} (End of group Event_Scheduler)
*/

#endif /* _EVENT_DATA_OBJECTS_H_ */
