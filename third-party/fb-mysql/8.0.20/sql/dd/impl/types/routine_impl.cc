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

#include "sql/dd/impl/types/routine_impl.h"

#include <new>
#include <sstream>
#include <string>

#include "lex_string.h"
#include "my_sys.h"
#include "my_user.h"  // parse_user
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/dd/dd_utility.h"                 // normalize_string()
#include "sql/dd/impl/raw/raw_record.h"        // Raw_record
#include "sql/dd/impl/tables/parameters.h"     // Parameters
#include "sql/dd/impl/tables/routines.h"       // Routines
#include "sql/dd/impl/tables/schemata.h"       // Schemata::name_collation
#include "sql/dd/impl/transaction_impl.h"      // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/parameter_impl.h"  // Parameter_impl
#include "sql/dd/impl/utils.h"                 // is_string_in_lowercase
#include "sql/dd/string_type.h"                // dd::String_type
#include "sql/dd/types/parameter.h"
#include "sql/dd/types/weak_object.h"

using dd::tables::Parameters;
using dd::tables::Routines;

namespace dd {

///////////////////////////////////////////////////////////////////////////
// Routine_impl implementation.
///////////////////////////////////////////////////////////////////////////

Routine_impl::Routine_impl()
    : m_routine_type(RT_PROCEDURE),
      m_sql_data_access(SDA_CONTAINS_SQL),
      m_security_type(View::ST_INVOKER),
      m_is_deterministic(false),
      m_sql_mode(0),
      m_created(0),
      m_last_altered(0),
      m_parameters(),
      m_schema_id(INVALID_OBJECT_ID),
      m_client_collation_id(INVALID_OBJECT_ID),
      m_connection_collation_id(INVALID_OBJECT_ID),
      m_schema_collation_id(INVALID_OBJECT_ID) {}

Routine_impl::~Routine_impl() {}

///////////////////////////////////////////////////////////////////////////

bool Routine_impl::validate() const {
  if (schema_id() == INVALID_OBJECT_ID) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Schema ID is not set");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Routine_impl::restore_children(Open_dictionary_tables_ctx *otx) {
  return m_parameters.restore_items(
      this, otx, otx->get_table<Parameter>(),
      Parameters::create_key_by_routine_id(this->id()));
}

///////////////////////////////////////////////////////////////////////////

bool Routine_impl::store_children(Open_dictionary_tables_ctx *otx) {
  return m_parameters.store_items(otx);
}

///////////////////////////////////////////////////////////////////////////

bool Routine_impl::drop_children(Open_dictionary_tables_ctx *otx) const {
  return m_parameters.drop_items(
      otx, otx->get_table<Parameter>(),
      Parameters::create_key_by_routine_id(this->id()));
}

/////////////////////////////////////////////////////////////////////////

bool Routine_impl::restore_attributes(const Raw_record &r) {
  // Read id and name.
  restore_id(r, Routines::FIELD_ID);
  restore_name(r, Routines::FIELD_NAME);

  // Read enums
  m_routine_type = (enum_routine_type)r.read_int(Routines::FIELD_TYPE);

  m_sql_data_access =
      (enum_sql_data_access)r.read_int(Routines::FIELD_SQL_DATA_ACCESS);

  m_security_type =
      (View::enum_security_type)r.read_int(Routines::FIELD_SECURITY_TYPE);

  // Read booleans
  m_is_deterministic = r.read_bool(Routines::FIELD_IS_DETERMINISTIC);

  // Read ulonglong
  m_sql_mode = r.read_int(Routines::FIELD_SQL_MODE);
  m_created = r.read_int(Routines::FIELD_CREATED);
  m_last_altered = r.read_int(Routines::FIELD_LAST_ALTERED);

  // Read references
  m_schema_id = r.read_ref_id(Routines::FIELD_SCHEMA_ID);
  m_client_collation_id = r.read_ref_id(Routines::FIELD_CLIENT_COLLATION_ID);
  m_connection_collation_id =
      r.read_ref_id(Routines::FIELD_CONNECTION_COLLATION_ID);
  m_schema_collation_id = r.read_ref_id(Routines::FIELD_SCHEMA_COLLATION_ID);

  // Read strings
  m_definition = r.read_str(Routines::FIELD_DEFINITION);
  m_definition_utf8 = r.read_str(Routines::FIELD_DEFINITION_UTF8);
  m_parameter_str = r.read_str(Routines::FIELD_PARAMETER_STR);
  m_comment = r.read_str(Routines::FIELD_COMMENT);

  // Read definer user/host
  {
    String_type definer = r.read_str(Routines::FIELD_DEFINER);

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

bool Routine_impl::store_attributes(Raw_record *r) {
  dd::Stringstream_type definer;
  definer << m_definer_user << '@' << m_definer_host;

  return store_id(r, Routines::FIELD_ID) ||
         store_name(r, Routines::FIELD_NAME) ||
         r->store(Routines::FIELD_SCHEMA_ID, m_schema_id) ||
         r->store(Routines::FIELD_NAME, name()) ||
         r->store(Routines::FIELD_TYPE, m_routine_type) ||
         r->store(Routines::FIELD_DEFINITION, m_definition) ||
         r->store(Routines::FIELD_DEFINITION_UTF8, m_definition_utf8) ||
         r->store(Routines::FIELD_PARAMETER_STR, m_parameter_str) ||
         r->store(Routines::FIELD_IS_DETERMINISTIC, m_is_deterministic) ||
         r->store(Routines::FIELD_SQL_DATA_ACCESS, m_sql_data_access) ||
         r->store(Routines::FIELD_SECURITY_TYPE, m_security_type) ||
         r->store(Routines::FIELD_DEFINER, definer.str()) ||
         r->store(Routines::FIELD_SQL_MODE, m_sql_mode) ||
         r->store(Routines::FIELD_CLIENT_COLLATION_ID, m_client_collation_id) ||
         r->store(Routines::FIELD_CONNECTION_COLLATION_ID,
                  m_connection_collation_id) ||
         r->store(Routines::FIELD_SCHEMA_COLLATION_ID, m_schema_collation_id) ||
         r->store(Routines::FIELD_CREATED, m_created) ||
         r->store(Routines::FIELD_LAST_ALTERED, m_last_altered) ||
         r->store(Routines::FIELD_COMMENT, m_comment, m_comment.empty());
}

///////////////////////////////////////////////////////////////////////////

bool Routine::update_id_key(Id_key *key, Object_id id) {
  key->update(id);
  return false;
}

///////////////////////////////////////////////////////////////////////////

void Routine_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "id: {OID: " << id() << "}; "
     << "m_name: " << name() << "; "
     << "m_routine_type: " << m_routine_type << "; "
     << "m_sql_data_access: " << m_sql_data_access << "; "
     << "m_security_type: " << m_security_type << "; "
     << "m_is_deterministic: " << m_is_deterministic << "; "
     << "m_sql_mode: " << m_sql_mode << "; "
     << "m_created: " << m_created << "; "
     << "m_last_altered: " << m_last_altered << "; "
     << "m_definition: " << m_definition << "; "
     << "m_definition_utf8: " << m_definition_utf8 << "; "
     << "m_parameter_str: " << m_parameter_str << "; "
     << "m_definer_user: " << m_definer_user << "; "
     << "m_definer_host: " << m_definer_host << "; "
     << "m_comment: " << m_comment << "; "
     << "m_schema_id: {OID: " << m_schema_id << "}; "
     << "m_client_collation_id: " << m_client_collation_id << "; "
     << "m_connection_collation_id: " << m_connection_collation_id << "; "
     << "m_schema_collation_id: " << m_schema_collation_id << "; "
     << "m_parameters: " << m_parameters.size() << " [ ";

  for (const Parameter *f : parameters()) {
    String_type ob;
    f->debug_print(ob);
    ss << ob;
  }

  ss << "] ";

  outb = ss.str();
}

///////////////////////////////////////////////////////////////////////////

Parameter *Routine_impl::add_parameter() {
  Parameter_impl *p = new (std::nothrow) Parameter_impl(this);
  m_parameters.push_back(p);
  return p;
}

///////////////////////////////////////////////////////////////////////////

const Object_table &Routine_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Routine_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Routines>();

  otx->register_tables<Parameter>();
}

///////////////////////////////////////////////////////////////////////////

Routine_impl::Routine_impl(const Routine_impl &src)
    : Weak_object(src),
      Entity_object_impl(src),
      m_routine_type(src.m_routine_type),
      m_sql_data_access(src.m_sql_data_access),
      m_security_type(src.m_security_type),
      m_is_deterministic(src.m_is_deterministic),
      m_sql_mode(src.m_sql_mode),
      m_created(src.m_created),
      m_last_altered(src.m_last_altered),
      m_definition(src.m_definition),
      m_definition_utf8(src.m_definition_utf8),
      m_parameter_str(src.m_parameter_str),
      m_definer_user(src.m_definer_user),
      m_definer_host(src.m_definer_host),
      m_comment(src.m_comment),
      m_parameters(),
      m_schema_id(src.m_schema_id),
      m_client_collation_id(src.m_client_collation_id),
      m_connection_collation_id(src.m_connection_collation_id),
      m_schema_collation_id(src.m_schema_collation_id) {
  m_parameters.deep_copy(src.m_parameters, this);
}

///////////////////////////////////////////////////////////////////////////

void Routine::create_mdl_key(enum_routine_type type,
                             const String_type &schema_name,
                             const String_type &name, MDL_key *mdl_key) {
#ifndef DEBUG_OFF
  // Make sure schema name is lowercased when lower_case_table_names == 2.
  if (lower_case_table_names == 2)
    DBUG_ASSERT(is_string_in_lowercase(schema_name,
                                       tables::Schemata::name_collation()));
  DBUG_EXECUTE_IF("simulate_lctn_two_case_for_schema_case_compare", {
    DBUG_ASSERT(
        (lower_case_table_names == 2) ||
        is_string_in_lowercase(schema_name, &my_charset_utf8_tolower_ci));
  });
#endif

  /*
    Normalize the routine name so that key comparison for case and accent
    insensitive routine names yields the correct result.
  */
  char normalized_name[NAME_CHAR_LEN * 2];
  size_t len = normalize_string(DD_table::name_collation(), name,
                                normalized_name, sizeof(normalized_name));

  mdl_key->mdl_key_init(
      type == RT_FUNCTION ? MDL_key::FUNCTION : MDL_key::PROCEDURE,
      schema_name.c_str(), normalized_name, len, name.c_str());
}

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Routine::name_collation() {
  return DD_table::name_collation();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
