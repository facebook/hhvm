/*
   Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/dd_trigger.h"

#include <string.h>

#include "my_alloc.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"                          // my_error, resolve_collation
#include "mysqld_error.h"                    // ER_UNKNOWN_COLLATION
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/string_type.h"
#include "sql/dd/types/table.h"    // dd::Table
#include "sql/dd/types/trigger.h"  // dd::Trigger
#include "sql/dd_table_share.h"    // dd_get_mysql_charset
#include "sql/item_create.h"
#include "sql/key.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_list.h"   // List
#include "sql/sql_servers.h"
#include "sql/strfunc.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/trigger.h"  // Trigger

namespace dd {

/**
  Get DD API value of event type for a trigger.

  @param [in]  new_trigger       pointer to a Trigger object from sql-layer.

  @return Value of enumeration dd::Trigger::enum_event_type
*/

static inline Trigger::enum_event_type get_dd_event_type(
    const ::Trigger *new_trigger) {
  switch (new_trigger->get_event()) {
    case TRG_EVENT_INSERT:
      return Trigger::enum_event_type::ET_INSERT;
    case TRG_EVENT_UPDATE:
      return Trigger::enum_event_type::ET_UPDATE;
    case TRG_EVENT_DELETE:
      return Trigger::enum_event_type::ET_DELETE;
    case TRG_EVENT_MAX:
      break;
  }
  /*
    Since a trigger event is supplied by parser and parser controls that
    a value for trigger event can take only values from the set
    (TRG_EVENT_INSERT, TRG_EVENT_UPDATE, TRG_EVENT_DELETE), it's allowable
    to add an assert to catch violation of this invariant just in case.
  */

  DBUG_ASSERT(false);

  return Trigger::enum_event_type::ET_INSERT;
}

/**
  Get DD API value of action timing for a trigger.

  @param [in]  new_trigger         pointer to a Trigger object from sql-layer.

  @return Value of enumeration Trigger::enum_action_timing
*/

static inline Trigger::enum_action_timing get_dd_action_timing(
    const ::Trigger *new_trigger) {
  switch (new_trigger->get_action_time()) {
    case TRG_ACTION_BEFORE:
      return Trigger::enum_action_timing::AT_BEFORE;
    case TRG_ACTION_AFTER:
      return Trigger::enum_action_timing::AT_AFTER;
    case TRG_ACTION_MAX:
      break;
  }
  /*
    Since a trigger action time is supplied by parser and parser controls
    that a value for trigger action time can take only values from the set
    (TRG_ACTION_BEFORE, TRG_ACTION_AFTER), it's allowable to add an assert
    to catch violation of this invariant just in case.
  */
  DBUG_ASSERT(false);

  return Trigger::enum_action_timing::AT_BEFORE;
}

/**
  Fill in a dd::Trigger object based on a Trigger object supplied by sql-layer.

  @param [in]   new_trigger       Trigger object supplied by sql-layer
  @param [out]  dd_trig_obj       dd::Trigger object to fill in

  @return Operation status
    @retval true   Failure
    @retval false  Success
*/

static bool fill_in_dd_trigger_object(const ::Trigger *new_trigger,
                                      Trigger *dd_trig_obj) {
  dd_trig_obj->set_name(String_type(new_trigger->get_trigger_name().str,
                                    new_trigger->get_trigger_name().length));
  dd_trig_obj->set_definer(String_type(new_trigger->get_definer_user().str,
                                       new_trigger->get_definer_user().length),
                           String_type(new_trigger->get_definer_host().str,
                                       new_trigger->get_definer_host().length));

  dd_trig_obj->set_event_type(get_dd_event_type(new_trigger));
  dd_trig_obj->set_action_timing(get_dd_action_timing(new_trigger));

  dd_trig_obj->set_action_statement(String_type(
      new_trigger->get_definition().str, new_trigger->get_definition().length));

  dd_trig_obj->set_action_statement_utf8(
      String_type(new_trigger->get_definition_utf8().str,
                  new_trigger->get_definition_utf8().length));

  dd_trig_obj->set_sql_mode(new_trigger->get_sql_mode());

  const CHARSET_INFO *collation;

  if (resolve_charset(new_trigger->get_client_cs_name().str,
                      system_charset_info, &collation)) {
    // resolve_charset will not cause an error to be reported if the
    // collation was not found, so we must report error here.
    my_error(ER_UNKNOWN_COLLATION, MYF(0),
             new_trigger->get_client_cs_name().str);
    return true;
  }
  dd_trig_obj->set_client_collation_id(collation->number);

  if (resolve_collation(new_trigger->get_connection_cl_name().str,
                        system_charset_info, &collation)) {
    // resolve_charset will not cause an error to be reported if the
    // collation was not found, so we must report error here.
    my_error(ER_UNKNOWN_COLLATION, MYF(0),
             new_trigger->get_connection_cl_name().str);
    return true;
  }
  dd_trig_obj->set_connection_collation_id(collation->number);

  if (resolve_collation(new_trigger->get_db_cl_name().str, system_charset_info,
                        &collation)) {
    // resolve_charset will not cause an error to be reported if the
    // collation was not found, so we must report error here.
    my_error(ER_UNKNOWN_COLLATION, MYF(0), new_trigger->get_db_cl_name().str);
    return true;
  }
  dd_trig_obj->set_schema_collation_id(collation->number);

  return false;
}

bool create_trigger(THD *thd, const ::Trigger *new_trigger,
                    enum_trigger_order_type ordering_clause,
                    const LEX_CSTRING &referenced_trigger_name) {
  DBUG_TRACE;

  cache::Dictionary_client *dd_client = thd->dd_client();
  cache::Dictionary_client::Auto_releaser releaser(dd_client);

  Table *new_table = nullptr;

  DBUG_EXECUTE_IF("create_trigger_fail", {
    my_error(ER_LOCK_DEADLOCK, MYF(0));
    return true;
  });

  if (dd_client->acquire_for_modification(
          new_trigger->get_db_name().str,
          new_trigger->get_subject_table_name().str, &new_table)) {
    // Error is reported by the dictionary subsystem.
    return true;
  }

  DBUG_ASSERT(new_table != nullptr);

  Trigger *dd_trig_obj;

  if (ordering_clause != TRG_ORDER_NONE) {
    const Trigger *referenced_trg =
        new_table->get_trigger(referenced_trigger_name.str);

    /*
      Checking for presence of a trigger referenced by FOLLOWS/PRECEDES clauses
      is done in Trigger_chain::add_trigger() that is called from
      Table_trigger_dispatcher::create_trigger() before storing trigger
      in the data dictionary. It means that call of dd::Trigger::get_trigger()
      for referenced trigger name must return NOT NULL pointer. Therefore
      it just added assert to check this invariant.
    */
    DBUG_ASSERT(referenced_trg != nullptr);
    if (ordering_clause == TRG_ORDER_FOLLOWS)
      dd_trig_obj = new_table->add_trigger_following(
          referenced_trg, get_dd_action_timing(new_trigger),
          get_dd_event_type(new_trigger));
    else
      dd_trig_obj = new_table->add_trigger_preceding(
          referenced_trg, get_dd_action_timing(new_trigger),
          get_dd_event_type(new_trigger));
  } else
    dd_trig_obj = new_table->add_trigger(get_dd_action_timing(new_trigger),
                                         get_dd_event_type(new_trigger));

  if (dd_trig_obj == nullptr)
    // NOTE: It's expected that an error is reported
    // by the dd::cache::Dictionary_client::add_trigger.
    return true;

  if (fill_in_dd_trigger_object(new_trigger, dd_trig_obj)) return true;

  return dd_client->update(new_table);
}

/**
  Convert event type value from DD presentation to generic SQL presentation.

  @param [in] event_type  Event type value from the Data Dictionary

  @return Event type value as it's presented in generic SQL-layer
*/

static enum_trigger_event_type convert_event_type_from_dd(
    dd::Trigger::enum_event_type event_type) {
  switch (event_type) {
    case dd::Trigger::enum_event_type::ET_INSERT:
      return TRG_EVENT_INSERT;
    case dd::Trigger::enum_event_type::ET_UPDATE:
      return TRG_EVENT_UPDATE;
    case dd::Trigger::enum_event_type::ET_DELETE:
      return TRG_EVENT_DELETE;
  };
  DBUG_ASSERT(false);
  return TRG_EVENT_MAX;
}

/**
  Convert action timing value from DD presentation to generic SQL presentation.

  @param [in] action_timing  Action timing value from the Data Dictionary

  @return Action timing value as it's presented in generic SQL-layer
*/

static enum_trigger_action_time_type convert_action_time_from_dd(
    dd::Trigger::enum_action_timing action_timing) {
  switch (action_timing) {
    case dd::Trigger::enum_action_timing::AT_BEFORE:
      return TRG_ACTION_BEFORE;
    case dd::Trigger::enum_action_timing::AT_AFTER:
      return TRG_ACTION_AFTER;
  }
  DBUG_ASSERT(false);
  return TRG_ACTION_MAX;
}

bool load_triggers(THD *thd, MEM_ROOT *mem_root, const char *schema_name,
                   const char *table_name, const dd::Table &table,
                   List<::Trigger> *triggers) {
  DBUG_TRACE;

  for (const auto &trigger : table.triggers()) {
    LEX_CSTRING db_name_str = {schema_name, strlen(schema_name)};
    LEX_CSTRING subject_table_name = {table_name, strlen(table_name)};
    LEX_CSTRING definition, definition_utf8;

    if (lex_string_strmake(mem_root, &definition,
                           trigger->action_statement().c_str(),
                           trigger->action_statement().length()))
      return true;

    if (lex_string_strmake(mem_root, &definition_utf8,
                           trigger->action_statement_utf8().c_str(),
                           trigger->action_statement_utf8().length()))
      return true;

    LEX_CSTRING definer_user;
    if (lex_string_strmake(mem_root, &definer_user,
                           trigger->definer_user().c_str(),
                           trigger->definer_user().length()))
      return true;

    LEX_CSTRING definer_host;
    if (lex_string_strmake(mem_root, &definer_host,
                           trigger->definer_host().c_str(),
                           trigger->definer_host().length()))
      return true;

    const CHARSET_INFO *client_cs =
        dd_get_mysql_charset(trigger->client_collation_id());
    if (client_cs == nullptr) client_cs = thd->variables.character_set_client;

    const CHARSET_INFO *connection_cs =
        dd_get_mysql_charset(trigger->connection_collation_id());
    if (connection_cs == nullptr)
      connection_cs = thd->variables.collation_connection;

    const CHARSET_INFO *schema_cs =
        dd_get_mysql_charset(trigger->schema_collation_id());
    if (schema_cs == nullptr) schema_cs = thd->variables.collation_database;

    LEX_CSTRING client_cs_name, connection_cl_name, db_cl_name, trigger_name;
    if (lex_string_strmake(mem_root, &client_cs_name, client_cs->csname,
                           strlen(client_cs->csname)) ||
        lex_string_strmake(mem_root, &connection_cl_name, connection_cs->name,
                           strlen(connection_cs->name)) ||
        lex_string_strmake(mem_root, &db_cl_name, schema_cs->name,
                           strlen(schema_cs->name)) ||
        lex_string_strmake(mem_root, &trigger_name, trigger->name().c_str(),
                           trigger->name().length()))
      return true;

    ::Trigger *trigger_to_add = ::Trigger::create_from_dd(
        mem_root, trigger_name, db_name_str, subject_table_name, definition,
        definition_utf8, trigger->sql_mode(), definer_user, definer_host,
        client_cs_name, connection_cl_name, db_cl_name,
        convert_event_type_from_dd(trigger->event_type()),
        convert_action_time_from_dd(trigger->action_timing()),
        trigger->action_order(), trigger->created());

    if (trigger_to_add == nullptr) return true;

    if (triggers->push_back(trigger_to_add, mem_root)) {
      destroy(trigger_to_add);
      return true;
    }
  }

  return false;
}

// Only used by NDB
bool table_has_triggers(THD *thd, const char *schema_name,
                        const char *table_name, bool *table_has_trigger) {
  DBUG_TRACE;

  cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  const Table *table = nullptr;

  if (thd->dd_client()->acquire(schema_name, table_name, &table)) {
    // Error is reported by the dictionary subsystem.
    return true;
  }

  *table_has_trigger = (table != nullptr && table->has_trigger());

  return false;
}

}  // namespace dd
