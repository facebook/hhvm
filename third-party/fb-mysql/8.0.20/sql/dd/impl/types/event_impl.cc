/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/types/event_impl.h"

#include <sstream>
#include <string>

#include "lex_string.h"
#include "my_sys.h"
#include "my_user.h"  // parse_user
#include "mysql_com.h"
#include "mysqld_error.h"                  // ER_*
#include "sql/dd/dd_utility.h"             // normalize_string()
#include "sql/dd/impl/raw/raw_record.h"    // Raw_record
#include "sql/dd/impl/tables/events.h"     // Events
#include "sql/dd/impl/tables/schemata.h"   // Schemata::name_collation
#include "sql/dd/impl/transaction_impl.h"  // Open_dictionary_tables_ctx
#include "sql/dd/impl/utils.h"             // is_string_in_lowercase
#include "sql/dd/string_type.h"            // dd::String_type
#include "sql/dd/types/weak_object.h"

using dd::tables::Events;

namespace dd {

///////////////////////////////////////////////////////////////////////////
// Event_impl implementation.
///////////////////////////////////////////////////////////////////////////

Event_impl::Event_impl()
    : m_interval_field(IF_YEAR),
      m_event_status(ES_DISABLED),
      m_on_completion(OC_DROP),
      m_sql_mode(0),
      m_created(0),
      m_last_altered(0),
      m_originator(0),
      m_interval_value(0),
      m_execute_at(0),
      m_starts(0),
      m_ends(0),
      m_last_executed(0),
      m_is_execute_at_null(false),
      m_is_interval_value_null(false),
      m_is_interval_field_null(false),
      m_is_starts_null(false),
      m_is_ends_null(false),
      m_is_event_status_null(false),
      m_is_last_executed_null(true),
      m_schema_id(INVALID_OBJECT_ID),
      m_client_collation_id(INVALID_OBJECT_ID),
      m_connection_collation_id(INVALID_OBJECT_ID),
      m_schema_collation_id(INVALID_OBJECT_ID) {}

///////////////////////////////////////////////////////////////////////////

bool Event_impl::validate() const {
  if (schema_id() == INVALID_OBJECT_ID) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Schema ID is not set");
    return true;
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////

bool Event_impl::restore_attributes(const Raw_record &r) {
  // Read id and name.
  restore_id(r, Events::FIELD_ID);
  restore_name(r, Events::FIELD_NAME);

  // Read enums
  m_interval_field =
      (enum_interval_field)r.read_int(Events::FIELD_INTERVAL_FIELD);
  m_is_interval_field_null = r.is_null(Events::FIELD_INTERVAL_FIELD);

  m_event_status = (enum_event_status)r.read_int(Events::FIELD_STATUS);
  m_is_event_status_null = r.is_null(Events::FIELD_STATUS);

  m_on_completion = (enum_on_completion)r.read_int(Events::FIELD_ON_COMPLETION);

  // Read ulonglong
  m_sql_mode = r.read_int(Events::FIELD_SQL_MODE);
  m_created = r.read_int(Events::FIELD_CREATED);
  m_last_altered = r.read_int(Events::FIELD_LAST_ALTERED);
  m_originator = r.read_int(Events::FIELD_ORIGINATOR);
  m_interval_value = r.read_int(Events::FIELD_INTERVAL_VALUE);
  m_is_interval_value_null = r.is_null(Events::FIELD_INTERVAL_VALUE);

  // Read time
  m_execute_at = r.read_time(Events::FIELD_EXECUTE_AT);
  m_is_execute_at_null = r.is_null(Events::FIELD_EXECUTE_AT);

  m_starts = r.read_time(Events::FIELD_STARTS);
  m_is_starts_null = r.is_null(Events::FIELD_STARTS);

  m_ends = r.read_time(Events::FIELD_ENDS);
  m_is_ends_null = r.is_null(Events::FIELD_ENDS);

  m_last_executed = r.read_time(Events::FIELD_LAST_EXECUTED);
  m_is_last_executed_null = r.is_null(Events::FIELD_LAST_EXECUTED);

  // Read strings
  m_time_zone = r.read_str(Events::FIELD_TIME_ZONE);
  m_definition = r.read_str(Events::FIELD_DEFINITION);
  m_definition_utf8 = r.read_str(Events::FIELD_DEFINITION_UTF8);
  m_comment = r.read_str(Events::FIELD_COMMENT);

  // Read definer user/host
  {
    String_type definer = r.read_str(Events::FIELD_DEFINER);

    char user_name_holder[USERNAME_LENGTH + 1];
    LEX_STRING user_name = {user_name_holder, USERNAME_LENGTH};

    char host_name_holder[HOSTNAME_LENGTH + 1];
    LEX_STRING host_name = {host_name_holder, HOSTNAME_LENGTH};

    parse_user(definer.c_str(), definer.length(), user_name.str,
               &user_name.length, host_name.str, &host_name.length);

    m_definer_user.assign(user_name.str, user_name.length);
    m_definer_host.assign(host_name.str, host_name.length);
  }

  // Read references
  m_schema_id = r.read_ref_id(Events::FIELD_SCHEMA_ID);
  m_client_collation_id = r.read_ref_id(Events::FIELD_CLIENT_COLLATION_ID);
  m_connection_collation_id =
      r.read_ref_id(Events::FIELD_CONNECTION_COLLATION_ID);
  m_schema_collation_id = r.read_ref_id(Events::FIELD_SCHEMA_COLLATION_ID);

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Event_impl::store_attributes(Raw_record *r) {
  dd::Stringstream_type definer;
  definer << m_definer_user << '@' << m_definer_host;

  return store_id(r, Events::FIELD_ID) || store_name(r, Events::FIELD_NAME) ||
         r->store(Events::FIELD_NAME, name()) ||
         r->store(Events::FIELD_INTERVAL_FIELD, m_interval_field,
                  m_is_interval_field_null) ||
         r->store(Events::FIELD_STATUS, m_event_status,
                  m_is_event_status_null) ||
         r->store(Events::FIELD_ON_COMPLETION, m_on_completion) ||
         r->store(Events::FIELD_SQL_MODE, m_sql_mode) ||
         r->store(Events::FIELD_CREATED, m_created) ||
         r->store(Events::FIELD_LAST_ALTERED, m_last_altered) ||
         r->store(Events::FIELD_ORIGINATOR, m_originator) ||
         r->store(Events::FIELD_INTERVAL_VALUE, m_interval_value,
                  m_is_interval_value_null) ||
         r->store(Events::FIELD_TIME_ZONE, m_time_zone) ||
         r->store_time(Events::FIELD_EXECUTE_AT, m_execute_at,
                       m_is_execute_at_null) ||
         r->store_time(Events::FIELD_STARTS, m_starts, m_is_starts_null) ||
         r->store_time(Events::FIELD_ENDS, m_ends, m_is_ends_null) ||
         r->store_time(Events::FIELD_LAST_EXECUTED, m_last_executed,
                       m_is_last_executed_null) ||
         r->store(Events::FIELD_DEFINITION, m_definition) ||
         r->store(Events::FIELD_DEFINITION_UTF8, m_definition_utf8) ||
         r->store(Events::FIELD_DEFINER, definer.str()) ||
         r->store(Events::FIELD_COMMENT, m_comment) ||
         r->store(Events::FIELD_SCHEMA_ID, m_schema_id) ||
         r->store(Events::FIELD_CLIENT_COLLATION_ID, m_client_collation_id) ||
         r->store(Events::FIELD_CONNECTION_COLLATION_ID,
                  m_connection_collation_id) ||
         r->store(Events::FIELD_SCHEMA_COLLATION_ID, m_schema_collation_id);
}

///////////////////////////////////////////////////////////////////////////

bool Event::update_id_key(Id_key *key, Object_id id) {
  key->update(id);
  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Event::update_name_key(Name_key *key, Object_id schema_id,
                            const String_type &name) {
  return Events::update_object_key(key, schema_id, name);
}

///////////////////////////////////////////////////////////////////////////

void Event_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "id: {OID: " << id() << "}; "
     << "m_name: " << name() << "; "
     << "m_interval_field: " << m_interval_field << "; "
     << "m_is_interval_field_null: " << m_is_interval_field_null << "; "
     << "m_event_status: " << m_event_status << "; "
     << "m_is_event_status_null: " << m_is_event_status_null << ";"
     << "m_on_completion: " << m_on_completion << "; "
     << "m_sql_mode: " << m_sql_mode << "; "
     << "m_created: " << m_created << "; "
     << "m_last_altered: " << m_last_altered << "; "
     << "m_originator: " << m_originator << "; "
     << "m_interval_value: " << m_interval_value << "; "
     << "m_is_interval_value_null: " << m_is_interval_value_null << "; "
     << "m_execute_at: " << m_execute_at << "; "
     << "m_is_execute_at_null: " << m_is_execute_at_null << "; "
     << "m_starts: " << m_starts << "; "
     << "m_is_starts_null: " << m_is_starts_null << "; "
     << "m_ends: " << m_ends << "; "
     << "m_is_ends_null: " << m_is_ends_null << "; "
     << "m_last_executed: " << m_last_executed << "; "
     << "m_is_last_executed_null: " << m_is_last_executed_null << "; "
     << "m_time_zone: " << m_time_zone << "; "
     << "m_definition: " << m_definition << "; "
     << "m_definition_utf8: " << m_definition_utf8 << "; "
     << "m_definer_user: " << m_definer_user << "; "
     << "m_definer_host: " << m_definer_host << "; "
     << "m_comment: " << m_comment << "; "
     << "m_schema_id: {OID: " << m_schema_id << "}; "
     << "m_client_collation_id: " << m_client_collation_id << "; "
     << "m_connection_collation_id: " << m_connection_collation_id << "; "
     << "m_schema_collation_id: " << m_schema_collation_id << "; ]";

  outb = ss.str();
}

///////////////////////////////////////////////////////////////////////////

const Object_table &Event_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Event_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Events>();
}

///////////////////////////////////////////////////////////////////////////

Event_impl::Event_impl(const Event_impl &src)
    : Weak_object(src),
      Entity_object_impl(src),
      m_interval_field(src.m_interval_field),
      m_event_status(src.m_event_status),
      m_on_completion(src.m_on_completion),
      m_sql_mode(src.m_sql_mode),
      m_created(src.m_created),
      m_last_altered(src.m_last_altered),
      m_originator(src.m_originator),
      m_interval_value(src.m_interval_value),
      m_execute_at(src.m_execute_at),
      m_starts(src.m_starts),
      m_ends(src.m_ends),
      m_last_executed(src.m_last_executed),
      m_is_execute_at_null(src.m_is_execute_at_null),
      m_is_interval_value_null(src.m_is_interval_value_null),
      m_is_interval_field_null(src.m_is_interval_field_null),
      m_is_starts_null(src.m_is_starts_null),
      m_is_ends_null(src.m_is_ends_null),
      m_is_event_status_null(src.m_is_event_status_null),
      m_is_last_executed_null(src.m_is_last_executed_null),
      m_time_zone(src.m_time_zone),
      m_definition(src.m_definition),
      m_definition_utf8(src.m_definition_utf8),
      m_definer_user(src.m_definer_user),
      m_definer_host(src.m_definer_host),
      m_comment(src.m_comment),
      m_schema_id(src.m_schema_id),
      m_client_collation_id(src.m_client_collation_id),
      m_connection_collation_id(src.m_connection_collation_id),
      m_schema_collation_id(src.m_schema_collation_id) {}

///////////////////////////////////////////////////////////////////////////

void Event::create_mdl_key(const String_type &schema_name,
                           const String_type &name, MDL_key *mdl_key) {
#ifndef DEBUG_OFF
  // Make sure schema name is lowercased when lower_case_table_names == 2.
  if (lower_case_table_names == 2)
    DBUG_ASSERT(is_string_in_lowercase(schema_name,
                                       tables::Schemata::name_collation()));
#endif

  /*
    Normalize the event name so that key comparison for case and accent
    insensitive event names yields the correct result.
  */
  char normalized_name[NAME_CHAR_LEN * 2];
  size_t len = normalize_string(DD_table::name_collation(), name,
                                normalized_name, sizeof(normalized_name));

  mdl_key->mdl_key_init(MDL_key::EVENT, schema_name.c_str(), normalized_name,
                        len, name.c_str());
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
