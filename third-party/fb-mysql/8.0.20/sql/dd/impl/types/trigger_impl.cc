/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/types/trigger_impl.h"

#include <sstream>
#include <string>

#include "lex_string.h"
#include "my_sys.h"
#include "my_user.h"  // parse_user
#include "mysql_com.h"
#include "mysqld_error.h"                  // ER_*
#include "sql/dd/dd_utility.h"             // normalize_string
#include "sql/dd/impl/raw/raw_record.h"    // Raw_record
#include "sql/dd/impl/tables/schemata.h"   // Schemata::name_collation
#include "sql/dd/impl/tables/triggers.h"   // Triggers
#include "sql/dd/impl/transaction_impl.h"  // Open_dictionary_tables_ctx
#include "sql/dd/impl/utils.h"             // is_string_in_lowercase
#include "sql/dd/string_type.h"            // dd::String_type
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/weak_object.h"

using dd::tables::Triggers;

namespace dd {

class Table;

///////////////////////////////////////////////////////////////////////////
// Trigger_impl implementation.
///////////////////////////////////////////////////////////////////////////

Trigger_impl::Trigger_impl()
    : m_event_type(enum_event_type::ET_INSERT),
      m_action_timing(enum_action_timing::AT_BEFORE),
      m_ordinal_position(0),
      m_action_order(0),
      m_sql_mode(0),
      m_table(nullptr),
      m_client_collation_id(INVALID_OBJECT_ID),
      m_connection_collation_id(INVALID_OBJECT_ID),
      m_schema_collation_id(INVALID_OBJECT_ID) {
  m_last_altered = m_created = {0, 0};
}

Trigger_impl::Trigger_impl(Table_impl *table)
    : m_event_type(enum_event_type::ET_INSERT),
      m_action_timing(enum_action_timing::AT_BEFORE),
      m_ordinal_position(0),
      m_sql_mode(0),
      m_table(table),
      m_client_collation_id(INVALID_OBJECT_ID),
      m_connection_collation_id(INVALID_OBJECT_ID),
      m_schema_collation_id(INVALID_OBJECT_ID) {
  m_last_altered = m_created = {0, 0};
}

///////////////////////////////////////////////////////////////////////////

const Table &Trigger_impl::table() const { return *m_table; }

Table &Trigger_impl::table() { return *m_table; }

///////////////////////////////////////////////////////////////////////////

bool Trigger_impl::validate() const {
  if (m_table == nullptr) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Table object for trigger is not set");
    return true;
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////

bool Trigger_impl::restore_attributes(const Raw_record &r) {
  if (check_parent_consistency(m_table,
                               r.read_ref_id(Triggers::FIELD_TABLE_ID)))
    return true;

  // Read id and name.
  restore_id(r, Triggers::FIELD_ID);
  restore_name(r, Triggers::FIELD_NAME);

  // Read enums
  m_event_type =
      static_cast<enum_event_type>(r.read_int(Triggers::FIELD_EVENT_TYPE));
  m_action_timing = static_cast<enum_action_timing>(
      r.read_int(Triggers::FIELD_ACTION_TIMING));
  m_sql_mode = r.read_int(Triggers::FIELD_SQL_MODE);

  // Read numerics
  m_action_order = r.read_uint(Triggers::FIELD_ACTION_ORDER);

  m_created = r.read_timestamp(Triggers::FIELD_CREATED);
  m_last_altered = r.read_timestamp(Triggers::FIELD_LAST_ALTERED);

  // Read references

  m_client_collation_id = r.read_ref_id(Triggers::FIELD_CLIENT_COLLATION_ID);
  m_connection_collation_id =
      r.read_ref_id(Triggers::FIELD_CONNECTION_COLLATION_ID);
  m_schema_collation_id = r.read_ref_id(Triggers::FIELD_SCHEMA_COLLATION_ID);

  // Read strings

  m_action_statement = r.read_str(Triggers::FIELD_ACTION_STATEMENT);
  m_action_statement_utf8 = r.read_str(Triggers::FIELD_ACTION_STATEMENT_UTF8);

  // Read definer user/host
  {
    String_type definer = r.read_str(Triggers::FIELD_DEFINER);

    char user_name_holder[USERNAME_LENGTH + 1];
    LEX_STRING user_name = {user_name_holder, USERNAME_LENGTH};

    char host_name_holder[HOSTNAME_LENGTH + 1];
    LEX_STRING host_name = {host_name_holder, HOSTNAME_LENGTH};

    parse_user(definer.c_str(), definer.length(), user_name.str,
               &user_name.length, host_name.str, &host_name.length);

    m_definer_user.assign(user_name.str, user_name.length);
    m_definer_host.assign(host_name.str, host_name.length);
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Trigger_impl::store_attributes(Raw_record *r) {
  dd::Stringstream_type definer;
  definer << m_definer_user << '@' << m_definer_host;

  return store_id(r, Triggers::FIELD_ID) ||
         store_name(r, Triggers::FIELD_NAME) ||
         r->store(Triggers::FIELD_TABLE_ID, m_table->id()) ||
         r->store(Triggers::FIELD_SCHEMA_ID, schema_id()) ||
         r->store(Triggers::FIELD_EVENT_TYPE, (uint)m_event_type) ||
         r->store(Triggers::FIELD_ACTION_TIMING, (uint)m_action_timing) ||
         r->store(Triggers::FIELD_ACTION_ORDER, m_action_order) ||
         r->store(Triggers::FIELD_ACTION_STATEMENT, m_action_statement) ||
         r->store(Triggers::FIELD_ACTION_STATEMENT_UTF8,
                  m_action_statement_utf8) ||
         r->store(Triggers::FIELD_DEFINER, definer.str()) ||
         r->store(Triggers::FIELD_SQL_MODE, m_sql_mode) ||
         r->store(Triggers::FIELD_CLIENT_COLLATION_ID, m_client_collation_id) ||
         r->store(Triggers::FIELD_CONNECTION_COLLATION_ID,
                  m_connection_collation_id) ||
         r->store(Triggers::FIELD_SCHEMA_COLLATION_ID, m_schema_collation_id) ||
         r->store_timestamp(Triggers::FIELD_CREATED, m_created) ||
         r->store_timestamp(Triggers::FIELD_LAST_ALTERED, m_last_altered);
}

///////////////////////////////////////////////////////////////////////////

void Trigger_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "TRIGGER OBJECT: { "
     << "id: {OID: " << id() << "}; "
     << "m_name: " << name() << "; "
     << "m_schema_id: {OID: " << schema_id() << "}; "
     << "m_table_id: {OID: " << m_table->id() << "}; "
     << "m_event_type: " << (uint)m_event_type << "; "
     << "m_action_timing: " << (uint)m_action_timing << "; "
     << "m_action_order: " << m_action_order << "; "
     << "m_action_statement: " << m_action_statement << "; "
     << "m_action_statement_utf8: " << m_action_statement_utf8 << "; "
     << "m_created: " << m_created.tv_sec << "; "
     << "m_last_altered: " << m_last_altered.tv_sec << "; "
     << "m_sql_mode: " << m_sql_mode << "; "
     << "m_definer_user: " << m_definer_user << "; "
     << "m_definer_host: " << m_definer_host << "; "
     << "m_client_collation_id: " << m_client_collation_id << "; "
     << "m_connection_collation_id: " << m_connection_collation_id << "; "
     << "m_schema_collation_id: " << m_schema_collation_id << "; }";

  outb = ss.str();
}

///////////////////////////////////////////////////////////////////////////

Trigger_impl::Trigger_impl(const Trigger_impl &src, Table_impl *parent)
    : Weak_object(src),
      Entity_object_impl(src),
      m_event_type(src.m_event_type),
      m_action_timing(src.m_action_timing),
      m_ordinal_position(src.m_ordinal_position),
      m_action_order(src.m_action_order),
      m_sql_mode(src.m_sql_mode),
      m_created(src.m_created),
      m_last_altered(src.m_last_altered),
      m_action_statement_utf8(src.m_action_statement_utf8),
      m_action_statement(src.m_action_statement),
      m_definer_user(src.m_definer_user),
      m_definer_host(src.m_definer_host),
      m_table(parent),
      m_client_collation_id(src.m_client_collation_id),
      m_connection_collation_id(src.m_connection_collation_id),
      m_schema_collation_id(src.m_schema_collation_id) {}

///////////////////////////////////////////////////////////////////////////

const Object_table &Trigger_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Trigger_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Triggers>();
}

///////////////////////////////////////////////////////////////////////////

void Trigger::create_mdl_key(const String_type &schema_name,
                             const String_type &name, MDL_key *mdl_key) {
#ifndef DEBUG_OFF
  // Make sure schema name is lowercased when lower_case_table_names == 2.
  if (lower_case_table_names == 2)
    DBUG_ASSERT(is_string_in_lowercase(schema_name,
                                       tables::Schemata::name_collation()));
#endif

  /*
    Normalize the trigger name so that key comparison for case and accent
    insensitive trigger names yields the correct result.
  */
  char normalized_name[NAME_CHAR_LEN * 2];
  size_t len = normalize_string(DD_table::name_collation(), name,
                                normalized_name, sizeof(normalized_name));

  mdl_key->mdl_key_init(MDL_key::TRIGGER, schema_name.c_str(), normalized_name,
                        len, name.c_str());
}

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Trigger::name_collation() {
  return DD_table::name_collation();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
