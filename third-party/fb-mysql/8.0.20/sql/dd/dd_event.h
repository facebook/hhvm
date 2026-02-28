/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_EVENT_INCLUDED
#define DD_EVENT_INCLUDED

#include "my_inttypes.h"
#include "my_time.h"             // interval_type
#include "sql/dd/string_type.h"  // dd::String_type
#include "sql/dd/types/event.h"  // dd::Event::enum_event_status

class Event_parse_data;
class THD;

struct LEX_USER;

using sql_mode_t = ulonglong;

namespace dd {
class Schema;

/**
   Convert new DD Event::enum_event_status to status type used in
   server code.

   @param event_status status of type enum_event_status.

   @returns an int indicating status of event used in server code.
*/
int get_old_status(Event::enum_event_status event_status);

/**
   Convert new DD Event::enum_on_completion to completion type used in
   server code.

   @param on_completion on_completion of type enum_on_completion.

   @returns an int indicating on_completion of event used in server code.
*/
int get_old_on_completion(Event::enum_on_completion on_completion);

/**
   Convert new DD interval_field type to type interval_type used in
   server code.

   @param interval_field interval_field of type enum_interval_field.

   @returns an value of type interval type indicating interval type values
            used in server code.
*/
interval_type get_old_interval_type(Event::enum_interval_field interval_field);

/**
   Create an event object and commit it to DD Table Events.

   @param thd              Thread handle
   @param schema           Schema object.
   @param event_name       Event name
   @param event_body       Event body.
   @param event_body_utf8  Event body in utf8 format.
   @param definer          Definer of the event.
   @param event_data       Event information obtained from parser.

   @retval true  Event creation failed.
   @retval false Event creation succeeded.
*/
bool create_event(THD *thd, const Schema &schema, const String_type &event_name,
                  const String_type &event_body,
                  const String_type &event_body_utf8, const LEX_USER *definer,
                  Event_parse_data *event_data);

/**
  Create or update a event object and commit it to
  DD Table Events.

  @param thd                 Thread handle
  @param event               Event to update.
  @param schema              Schema currently containing the event.
  @param new_schema          New Schema or nullptr if the schema does not
  change.
  @param new_event_name      Updated Event name.
  @param new_event_body      Updated Event body.
  @param new_event_body_utf8 Updated Event body in utf8 format.
  @param definer             Definer of the event.
  @param event_data          Event information obtained from parser.

  @retval true  Event updation failed.
  @retval false Event updation succeeded.
*/
bool update_event(THD *thd, Event *event, const dd::Schema &schema,
                  const dd::Schema *new_schema,
                  const String_type &new_event_name,
                  const String_type &new_event_body,
                  const String_type &new_event_body_utf8,
                  const LEX_USER *definer, Event_parse_data *event_data);

/**
  Update time related fields of Event object.

  @param thd               Thread handle
  @param event             Event to update.
  @param last_executed     Time the event was last executed.
  @param status            Event status.

  @retval true  true if update failed.
  @retval false false if update succeeded.
*/
bool update_event_time_and_status(THD *thd, Event *event,
                                  my_time_t last_executed, ulonglong status);

}  // namespace dd
#endif  // DD_EVENT_INCLUDED
