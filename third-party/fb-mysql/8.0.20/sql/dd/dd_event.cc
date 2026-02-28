/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/dd_event.h"

#include <memory>
#include <string>

#include "lex_string.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysqld_error.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/impl/utils.h"
#include "sql/dd/types/schema.h"
#include "sql/event_parse_data.h"  // Event_parse_data
#include "sql/log.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_connect.h"
#include "sql/sql_db.h"  // get_default_db_collation
#include "sql/sql_lex.h"
#include "sql/system_variables.h"
#include "sql/tztime.h"  // Time_zone
#include "sql_string.h"

namespace dd {

static const char *failsafe_object = "Event status option";

int get_old_status(Event::enum_event_status event_status) {
  switch (event_status) {
    case Event::ES_ENABLED:
      return Event_parse_data::ENABLED;
    case Event::ES_DISABLED:
      return Event_parse_data::DISABLED;
    case Event::ES_SLAVESIDE_DISABLED:
      return Event_parse_data::SLAVESIDE_DISABLED;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, failsafe_object);
  DBUG_ASSERT(false);

  return Event_parse_data::DISABLED;
  /* purecov: end deadcode */
}

/**
  Convert legacy event status to dd::Event::enum_event_status.

  @param  event_status  Legacy event_status

  @returns dd::Event::enum_event_status value corressponding to
           legacy event_status.
*/

static Event::enum_event_status get_enum_event_status(int event_status) {
  switch (event_status) {
    case Event_parse_data::ENABLED:
      return Event::ES_ENABLED;
    case Event_parse_data::DISABLED:
      return Event::ES_DISABLED;
    case Event_parse_data::SLAVESIDE_DISABLED:
      return Event::ES_SLAVESIDE_DISABLED;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, failsafe_object);
  DBUG_ASSERT(false);

  return Event::ES_DISABLED;
  /* purecov: end deadcode */
}

int get_old_on_completion(Event::enum_on_completion on_completion) {
  switch (on_completion) {
    case Event::OC_DROP:
      return Event_parse_data::ON_COMPLETION_DROP;
    case Event::OC_PRESERVE:
      return Event_parse_data::ON_COMPLETION_PRESERVE;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, failsafe_object);
  DBUG_ASSERT(false);

  return Event_parse_data::ON_COMPLETION_DROP;
  /* purecov: end deadcode */
}

/**
  Convert legacy event on completion behavior to dd::Event::enum_on_compeltion.

  @param  on_completion  Legacy on completion behaviour value

  @returns dd::Event::enum_on_compeltion corressponding to legacy
           event on completion value.
*/

static Event::enum_on_completion get_on_completion(int on_completion) {
  switch (on_completion) {
    case Event_parse_data::ON_COMPLETION_DEFAULT:
    case Event_parse_data::ON_COMPLETION_DROP:
      return Event::OC_DROP;
    case Event_parse_data::ON_COMPLETION_PRESERVE:
      return Event::OC_PRESERVE;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, failsafe_object);
  DBUG_ASSERT(false);

  return Event::OC_DROP;
  /* purecov: end deadcode */
}

interval_type get_old_interval_type(Event::enum_interval_field interval_field) {
  switch (interval_field) {
    case Event::IF_YEAR:
      return INTERVAL_YEAR;
    case Event::IF_QUARTER:
      return INTERVAL_QUARTER;
    case Event::IF_MONTH:
      return INTERVAL_MONTH;
    case Event::IF_WEEK:
      return INTERVAL_WEEK;
    case Event::IF_DAY:
      return INTERVAL_DAY;
    case Event::IF_HOUR:
      return INTERVAL_HOUR;
    case Event::IF_MINUTE:
      return INTERVAL_MINUTE;
    case Event::IF_SECOND:
      return INTERVAL_SECOND;
    case Event::IF_MICROSECOND:
      return INTERVAL_MICROSECOND;
    case Event::IF_YEAR_MONTH:
      return INTERVAL_YEAR_MONTH;
    case Event::IF_DAY_HOUR:
      return INTERVAL_DAY_HOUR;
    case Event::IF_DAY_MINUTE:
      return INTERVAL_DAY_MINUTE;
    case Event::IF_DAY_SECOND:
      return INTERVAL_DAY_SECOND;
    case Event::IF_HOUR_MINUTE:
      return INTERVAL_HOUR_MINUTE;
    case Event::IF_HOUR_SECOND:
      return INTERVAL_HOUR_SECOND;
    case Event::IF_MINUTE_SECOND:
      return INTERVAL_MINUTE_SECOND;
    case Event::IF_DAY_MICROSECOND:
      return INTERVAL_DAY_MICROSECOND;
    case Event::IF_HOUR_MICROSECOND:
      return INTERVAL_HOUR_MICROSECOND;
    case Event::IF_MINUTE_MICROSECOND:
      return INTERVAL_MINUTE_MICROSECOND;
    case Event::IF_SECOND_MICROSECOND:
      return INTERVAL_SECOND_MICROSECOND;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, failsafe_object);
  DBUG_ASSERT(false);

  return INTERVAL_YEAR;
  /* purecov: end deadcode */
}

/**
  Convert legacy interval type value to dd::Event::enum_interval_field.

  @param  interval_type_val  Interval type value.

  @returns dd::Event::enum_interval_field corressponding to legacy
           interval type value.
*/

static Event::enum_interval_field get_enum_interval_field(
    interval_type interval_type_val) {
  switch (interval_type_val) {
    case INTERVAL_YEAR:
      return Event::IF_YEAR;
    case INTERVAL_QUARTER:
      return Event::IF_QUARTER;
    case INTERVAL_MONTH:
      return Event::IF_MONTH;
    case INTERVAL_WEEK:
      return Event::IF_WEEK;
    case INTERVAL_DAY:
      return Event::IF_DAY;
    case INTERVAL_HOUR:
      return Event::IF_HOUR;
    case INTERVAL_MINUTE:
      return Event::IF_MINUTE;
    case INTERVAL_SECOND:
      return Event::IF_SECOND;
    case INTERVAL_MICROSECOND:
      return Event::IF_MICROSECOND;
    case INTERVAL_YEAR_MONTH:
      return Event::IF_YEAR_MONTH;
    case INTERVAL_DAY_HOUR:
      return Event::IF_DAY_HOUR;
    case INTERVAL_DAY_MINUTE:
      return Event::IF_DAY_MINUTE;
    case INTERVAL_DAY_SECOND:
      return Event::IF_DAY_SECOND;
    case INTERVAL_HOUR_MINUTE:
      return Event::IF_HOUR_MINUTE;
    case INTERVAL_HOUR_SECOND:
      return Event::IF_HOUR_SECOND;
    case INTERVAL_MINUTE_SECOND:
      return Event::IF_MINUTE_SECOND;
    case INTERVAL_DAY_MICROSECOND:
      return Event::IF_DAY_MICROSECOND;
    case INTERVAL_HOUR_MICROSECOND:
      return Event::IF_HOUR_MICROSECOND;
    case INTERVAL_MINUTE_MICROSECOND:
      return Event::IF_MINUTE_MICROSECOND;
    case INTERVAL_SECOND_MICROSECOND:
      return Event::IF_SECOND_MICROSECOND;
    case INTERVAL_LAST:
      DBUG_ASSERT(false);
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, failsafe_object);
  DBUG_ASSERT(false);

  return Event::IF_YEAR;
  /* purecov: end deadcode */
}

/**
  Set Event attributes.

  @param    thd               THD context.
  @param    schema            Schema containing the event.
  @param    event             Pointer to Event Object.
  @param    event_name        Event name.
  @param    event_body        Event body.
  @param    event_body_utf8   Event body utf8.
  @param    definer           Definer of event.
  @param    event_data        Parsed Event Data.
  @param    is_update         true if existing Event objects attributes set
                              else false.
*/

static void set_event_attributes(THD *thd, const dd::Schema &schema,
                                 Event *event, const String_type &event_name,
                                 const String_type &event_body,
                                 const String_type &event_body_utf8,
                                 const LEX_USER *definer,
                                 Event_parse_data *event_data, bool is_update) {
  // Set Event name and definer.
  event->set_name(event_name);
  event->set_definer(definer->user.str, definer->host.str);

  // Set last altered time.
  event->set_last_altered(
      dd::my_time_t_to_ull_datetime(thd->query_start_in_secs()));

  // Set Event on completion and status.
  event->set_on_completion(get_on_completion(event_data->on_completion));
  if (!is_update || event_data->status_changed)
    event->set_event_status(get_enum_event_status(event_data->status));
  event->set_originator(event_data->originator);

  // Set Event definition attributes.
  if (event_data->body_changed) {
    event->set_sql_mode(thd->variables.sql_mode);
    event->set_definition_utf8(event_body_utf8);
    event->set_definition(event_body);
  }

  // Set Event scheduling attributes.
  if (event_data->expression) {
    const String *tz_name = thd->variables.time_zone->get_name();
    if (!is_update || !event_data->starts_null)
      event->set_time_zone(tz_name->ptr());

    event->set_interval_value_null(false);
    event->set_interval_value(event_data->expression);
    event->set_interval_field_null(false);
    event->set_interval_field(get_enum_interval_field(event_data->interval));

    event->set_execute_at_null(true);
    if (!event_data->starts_null) {
      event->set_starts_null(false);
      event->set_starts(event_data->starts);
    } else
      event->set_starts_null(true);

    if (!event_data->ends_null) {
      event->set_ends_null(false);
      event->set_ends(event_data->ends);
    } else
      event->set_ends_null(true);
  } else if (event_data->execute_at) {
    const String *tz_name = thd->variables.time_zone->get_name();
    event->set_time_zone(tz_name->ptr());
    event->set_interval_value_null(true);
    event->set_interval_field_null(true);
    event->set_starts_null(true);
    event->set_ends_null(true);
    event->set_execute_at_null(false);
    event->set_execute_at(event_data->execute_at);
  } else
    DBUG_ASSERT(is_update);

  if (event_data->comment.str != nullptr)
    event->set_comment(String_type(event_data->comment.str));

  // Set collation relate attributes.
  event->set_client_collation_id(thd->variables.character_set_client->number);
  event->set_connection_collation_id(
      thd->variables.collation_connection->number);

  const CHARSET_INFO *db_cl = nullptr;
  if (get_default_db_collation(schema, &db_cl)) {
    DBUG_PRINT("error", ("get_default_db_collation failed."));
    // Obtain collation from thd and proceed.
    thd->clear_error();
  }

  db_cl = db_cl ? db_cl : thd->collation();
  event->set_schema_collation_id(db_cl->number);
}

bool create_event(THD *thd, const Schema &schema, const String_type &event_name,
                  const String_type &event_body,
                  const String_type &event_body_utf8, const LEX_USER *definer,
                  Event_parse_data *event_data) {
  DBUG_TRACE;

  std::unique_ptr<dd::Event> event(schema.create_event(thd));

  // Set Event attributes.
  set_event_attributes(thd, schema, event.get(), event_name, event_body,
                       event_body_utf8, definer, event_data, false);

  return thd->dd_client()->store(event.get());
}

bool update_event(THD *thd, Event *event, const dd::Schema &schema,
                  const dd::Schema *new_schema,
                  const String_type &new_event_name,
                  const String_type &new_event_body,
                  const String_type &new_event_body_utf8,
                  const LEX_USER *definer, Event_parse_data *event_data) {
  DBUG_TRACE;

  // Check whether alter event was given dates that are in the past.
  if (event_data->check_dates(thd, static_cast<int>(event->on_completion())))
    return true;

  // Update Schema Id if there is a dbname change.
  if (new_schema != nullptr) event->set_schema_id(new_schema->id());

  // Set the altered event attributes.
  set_event_attributes(
      thd, (new_schema != nullptr) ? *new_schema : schema, event,
      new_event_name != "" ? new_event_name : event->name(), new_event_body,
      new_event_body_utf8, definer, event_data, true);

  return thd->dd_client()->update(event);
}

bool update_event_time_and_status(THD *thd, Event *event,
                                  my_time_t last_executed, ulonglong status) {
  DBUG_TRACE;

  event->set_event_status_null(false);
  event->set_event_status(get_enum_event_status(status));
  event->set_last_executed_null(false);
  event->set_last_executed(last_executed);

  return thd->dd_client()->update(event);
}

}  // namespace dd
