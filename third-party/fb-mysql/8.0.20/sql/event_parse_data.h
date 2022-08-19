/*
   Copyright (c) 2008, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _EVENT_PARSE_DATA_H_
#define _EVENT_PARSE_DATA_H_

#include <stddef.h>

#include "lex_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_time.h"  // interval_type

class Item;
class THD;
class sp_name;

#define EVEX_GET_FIELD_FAILED -2
#define EVEX_BAD_PARAMS -5
#define EVEX_MICROSECOND_UNSUP -6
#define EVEX_MAX_INTERVAL_VALUE 1000000000L

class Event_parse_data {
 public:
  /*
    ENABLED = feature can function normally (is turned on)
    SLAVESIDE_DISABLED = feature is turned off on slave
    DISABLED = feature is turned off
  */
  enum enum_status { ENABLED = 1, DISABLED, SLAVESIDE_DISABLED };

  enum enum_on_completion {
    /*
      On CREATE EVENT, DROP is the DEFAULT as per the docs.
      On ALTER  EVENT, "no change" is the DEFAULT.
    */
    ON_COMPLETION_DEFAULT = 0,
    ON_COMPLETION_DROP,
    ON_COMPLETION_PRESERVE
  };

  int on_completion;
  int status;
  bool status_changed;
  longlong originator;
  /*
    do_not_create will be set if STARTS time is in the past and
    on_completion == ON_COMPLETION_DROP.
  */
  bool do_not_create;

  bool body_changed;

  LEX_CSTRING dbname;
  LEX_CSTRING name;
  LEX_STRING definer;  // combination of user and host
  LEX_STRING comment;

  Item *item_starts;
  Item *item_ends;
  Item *item_execute_at;

  my_time_t starts;
  my_time_t ends;
  my_time_t execute_at;
  bool starts_null;
  bool ends_null;
  bool execute_at_null;

  sp_name *identifier;
  Item *item_expression;
  longlong expression;
  interval_type interval;

  bool check_parse_data(THD *thd);

  bool check_dates(THD *thd, int previous_on_completion);

  Event_parse_data()
      : on_completion(Event_parse_data::ON_COMPLETION_DEFAULT),
        status(Event_parse_data::ENABLED),
        status_changed(false),
        do_not_create(false),
        body_changed(false),
        item_starts(nullptr),
        item_ends(nullptr),
        item_execute_at(nullptr),
        starts_null(true),
        ends_null(true),
        execute_at_null(true),
        item_expression(nullptr),
        expression(0) {
    DBUG_TRACE;

    /* Actually in the parser STARTS is always set */
    starts = ends = execute_at = 0;

    comment.str = nullptr;
    comment.length = 0;

    return;
  }

  ~Event_parse_data() {}

 private:
  void init_definer(THD *thd);

  void init_name(THD *thd, sp_name *spn);

  int init_execute_at(THD *thd);

  int init_interval(THD *thd);

  int init_starts(THD *thd);

  int init_ends(THD *thd);

  void report_bad_value(THD *thd, const char *item_name, Item *bad_item);

  void check_if_in_the_past(THD *thd, my_time_t ltime_utc);

  Event_parse_data(const Event_parse_data &); /* Prevent use of these */
  void check_originator_id(THD *thd);
  void operator=(Event_parse_data &);
};
#endif
