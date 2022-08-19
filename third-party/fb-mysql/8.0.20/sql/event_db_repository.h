#ifndef _EVENT_DB_REPOSITORY_H_
#define _EVENT_DB_REPOSITORY_H_

/*
   Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "lex_string.h"
#include "my_inttypes.h"

class Event_basic;
class Event_parse_data;
class THD;

typedef long my_time_t;

namespace dd {
class Schema;
}

/*
  Fields in mysql.event table in 5.7. This enum is used to
  read and update mysql.events dictionary table during upgrade
  scenario.

  Note:  This enum should not be used for other purpose
         as it will be removed eventually.
*/
enum enum_events_table_field {
  ET_FIELD_DB = 0,
  ET_FIELD_NAME,
  ET_FIELD_BODY,
  ET_FIELD_DEFINER,
  ET_FIELD_EXECUTE_AT,
  ET_FIELD_INTERVAL_EXPR,
  ET_FIELD_TRANSIENT_INTERVAL,
  ET_FIELD_CREATED,
  ET_FIELD_MODIFIED,
  ET_FIELD_LAST_EXECUTED,
  ET_FIELD_STARTS,
  ET_FIELD_ENDS,
  ET_FIELD_STATUS,
  ET_FIELD_ON_COMPLETION,
  ET_FIELD_SQL_MODE,
  ET_FIELD_COMMENT,
  ET_FIELD_ORIGINATOR,
  ET_FIELD_TIME_ZONE,
  ET_FIELD_CHARACTER_SET_CLIENT,
  ET_FIELD_COLLATION_CONNECTION,
  ET_FIELD_DB_COLLATION,
  ET_FIELD_BODY_UTF8,
  ET_FIELD_COUNT
};

/**
  @addtogroup Event_Scheduler
  @{

  @file event_db_repository.h

  Data Dictionary related operations of Event Scheduler.

  This is a private header file of Events module. Please do not include it
  directly. All public declarations of Events module should be stored in
  events.h and event_data_objects.h.
*/

class Event_db_repository {
  Event_db_repository() {}

 public:
  static bool create_event(THD *thd, Event_parse_data *parse_data,
                           bool create_if_not, bool *event_already_exists);

  static bool update_event(THD *thd, Event_parse_data *parse_data,
                           const LEX_CSTRING *new_dbname,
                           const LEX_CSTRING *new_name);

  static bool drop_event(THD *thd, LEX_CSTRING db, LEX_CSTRING name,
                         bool drop_if_exists, bool *event_exists);

  static bool drop_schema_events(THD *thd, const dd::Schema &schema);

  static bool load_named_event(THD *thd, LEX_CSTRING dbname, LEX_CSTRING name,
                               Event_basic *et);

  static bool update_timing_fields_for_event(THD *thd,
                                             LEX_CSTRING event_db_name,
                                             LEX_CSTRING event_name,
                                             my_time_t last_executed,
                                             ulonglong status);

  // Disallow copy construction and assignment.
  Event_db_repository(const Event_db_repository &) = delete;
  void operator=(Event_db_repository &) = delete;
};

/**
  @} (End of group Event_Scheduler)
*/
#endif /* _EVENT_DB_REPOSITORY_H_ */
