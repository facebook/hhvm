/* Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/types/view_impl.h"

#include <sstream>
#include <string>

#include "lex_string.h"
#include "my_sys.h"
#include "my_user.h"  // parse_user
#include "mysql_com.h"
#include "mysqld_error.h"                           // ER_*
#include "sql/dd/impl/properties_impl.h"            // Properties_impl
#include "sql/dd/impl/raw/raw_record.h"             // Raw_record
#include "sql/dd/impl/tables/columns.h"             // Columns
#include "sql/dd/impl/tables/tables.h"              // Tables
#include "sql/dd/impl/tables/view_routine_usage.h"  // View_routine_usage
#include "sql/dd/impl/tables/view_table_usage.h"    // View_table_usage
#include "sql/dd/impl/transaction_impl.h"         // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/view_routine_impl.h"  // View_routine_impl
#include "sql/dd/impl/types/view_table_impl.h"    // View_table_impl
#include "sql/dd/properties.h"                    // Needed for destructor
#include "sql/dd/string_type.h"                   // dd::String_type
#include "sql/dd/types/column.h"                  // Column
#include "sql/dd/types/view_routine.h"
#include "sql/dd/types/view_table.h"
#include "sql/dd/types/weak_object.h"

using dd::tables::Tables;
using dd::tables::View_routine_usage;
using dd::tables::View_table_usage;

namespace {
enum enum_view_updatable_values { VIEW_NOT_UPDATABLE = 1, VIEW_UPDATABLE };
}

namespace dd {

///////////////////////////////////////////////////////////////////////////
// View_impl implementation.
///////////////////////////////////////////////////////////////////////////

View_impl::View_impl()
    : m_type(enum_table_type::USER_VIEW),
      m_is_updatable(false),
      m_check_option(CO_NONE),
      m_algorithm(VA_UNDEFINED),
      m_security_type(ST_INVOKER),
      m_column_names(),
      m_tables(),
      m_routines(),
      m_client_collation_id(INVALID_OBJECT_ID),
      m_connection_collation_id(INVALID_OBJECT_ID) {}

///////////////////////////////////////////////////////////////////////////

bool View_impl::validate() const {
  if (Abstract_table_impl::validate()) return true;

  if (m_client_collation_id == INVALID_OBJECT_ID) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No client collation object is associated with View.");
    return true;
  }

  if (m_connection_collation_id == INVALID_OBJECT_ID) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Connection collation ID not set.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool View_impl::restore_children(Open_dictionary_tables_ctx *otx) {
  return Abstract_table_impl::restore_children(otx) ||
         m_tables.restore_items(
             this, otx, otx->get_table<View_table>(),
             View_table_usage::create_key_by_view_id(this->id())) ||
         m_routines.restore_items(
             this, otx, otx->get_table<View_routine>(),
             View_routine_usage::create_key_by_view_id(this->id()));
}

///////////////////////////////////////////////////////////////////////////

bool View_impl::store_children(Open_dictionary_tables_ctx *otx) {
  return Abstract_table_impl::store_children(otx) ||
         m_tables.store_items(otx) || m_routines.store_items(otx);
}

///////////////////////////////////////////////////////////////////////////

bool View_impl::drop_children(Open_dictionary_tables_ctx *otx) const {
  return m_routines.drop_items(
             otx, otx->get_table<View_routine>(),
             View_routine_usage::create_key_by_view_id(this->id())) ||
         m_tables.drop_items(
             otx, otx->get_table<View_table>(),
             View_table_usage::create_key_by_view_id(this->id())) ||
         Abstract_table_impl::drop_children(otx);
}

///////////////////////////////////////////////////////////////////////////

void View_impl::remove_children() {
  columns()->remove_all();
  m_column_names.clear();
  m_tables.remove_all();
  m_routines.remove_all();
}

///////////////////////////////////////////////////////////////////////////

bool View_impl::restore_attributes(const Raw_record &r) {
  m_type = (enum_table_type)r.read_int(Tables::FIELD_TYPE);

  if (m_type != enum_table_type::USER_VIEW &&
      m_type != enum_table_type::SYSTEM_VIEW) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Invalid view type found.");
    return true;
  }

  if (Abstract_table_impl::restore_attributes(r)) return true;

  m_definition = r.read_str(Tables::FIELD_VIEW_DEFINITION);
  m_definition_utf8 = r.read_str(Tables::FIELD_VIEW_DEFINITION_UTF8);

  {
    String_type definer = r.read_str(Tables::FIELD_VIEW_DEFINER);

    // NOTE: this is a copy/paste from sp_head::set_definer().

    char user_name_holder[USERNAME_LENGTH + 1];
    LEX_STRING user_name = {user_name_holder, USERNAME_LENGTH};

    char host_name_holder[HOSTNAME_LENGTH + 1];
    LEX_STRING host_name = {host_name_holder, HOSTNAME_LENGTH};

    parse_user(definer.c_str(), definer.length(), user_name.str,
               &user_name.length, host_name.str, &host_name.length);

    m_definer_user.assign(user_name.str, user_name.length);
    m_definer_host.assign(host_name.str, host_name.length);
  }

  m_is_updatable =
      (r.read_int(Tables::FIELD_VIEW_IS_UPDATABLE) == VIEW_UPDATABLE);

  m_check_option =
      (enum_check_option)r.read_int(Tables::FIELD_VIEW_CHECK_OPTION);
  m_security_type =
      (enum_security_type)r.read_int(Tables::FIELD_VIEW_SECURITY_TYPE);
  m_algorithm = (enum_algorithm)r.read_int(Tables::FIELD_VIEW_ALGORITHM);

  m_client_collation_id = r.read_ref_id(Tables::FIELD_VIEW_CLIENT_COLLATION_ID);
  m_connection_collation_id =
      r.read_ref_id(Tables::FIELD_VIEW_CONNECTION_COLLATION_ID);

  m_column_names.insert_values(r.read_str(Tables::FIELD_VIEW_COLUMN_NAMES));

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool View_impl::store_attributes(Raw_record *r) {
  //
  // Store view attributes
  //

  dd::Stringstream_type definer;
  definer << m_definer_user << '@' << m_definer_host;

  return Abstract_table_impl::store_attributes(r) ||
         r->store(Tables::FIELD_TYPE, static_cast<int>(m_type)) ||
         r->store(Tables::FIELD_VIEW_DEFINITION, m_definition) ||
         r->store(Tables::FIELD_VIEW_DEFINITION_UTF8, m_definition_utf8) ||
         r->store(Tables::FIELD_VIEW_CHECK_OPTION, m_check_option) ||
         r->store(Tables::FIELD_VIEW_IS_UPDATABLE,
                  m_is_updatable ? VIEW_UPDATABLE : VIEW_NOT_UPDATABLE) ||
         r->store(Tables::FIELD_VIEW_ALGORITHM, m_algorithm) ||
         r->store(Tables::FIELD_VIEW_SECURITY_TYPE, m_security_type) ||
         r->store(Tables::FIELD_VIEW_DEFINER, definer.str()) ||
         r->store_ref_id(Tables::FIELD_VIEW_CLIENT_COLLATION_ID,
                         m_client_collation_id) ||
         r->store_ref_id(Tables::FIELD_VIEW_CONNECTION_COLLATION_ID,
                         m_connection_collation_id) ||
         r->store(Tables::FIELD_VIEW_COLUMN_NAMES, m_column_names);
}

///////////////////////////////////////////////////////////////////////////

void View_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;

  String_type s;
  Abstract_table_impl::debug_print(s);

  ss << "VIEW OBJECT: { " << s << "m_definition: " << m_definition << "; "
     << "m_definition_utf8: " << m_definition_utf8 << "; "
     << "m_check_option: " << m_check_option << "; "
     << "m_is_updatable: " << (m_is_updatable ? "yes" : "no") << "; "
     << "m_algorithm: " << m_algorithm << "; "
     << "m_security_type: " << m_security_type << "; "
     << "m_definer_user: " << m_definer_user << "; "
     << "m_definer_host: " << m_definer_host << "; "
     << "m_client_collation: {OID: " << m_client_collation_id << "}; "
     << "m_connection_collation: {OID: " << m_connection_collation_id << "}; "
     << "m_column_names: " << m_column_names.raw_string() << "; "
     << "m_tables: " << m_tables.size() << " [ ";

  for (const View_table *f : tables()) {
    String_type ob;
    f->debug_print(ob);
    ss << ob;
  }
  ss << "] ";

  ss << "m_routines:" << m_routines.size() << " [ ";
  for (const View_routine *r : routines()) {
    String_type ob;
    r->debug_print(ob);
    ss << ob;
  }
  ss << "] ";

  ss << " }";

  outb = ss.str();
}

///////////////////////////////////////////////////////////////////////////

View_table *View_impl::add_table() {
  View_table_impl *vt = new (std::nothrow) View_table_impl(this);
  m_tables.push_back(vt);
  return vt;
}

///////////////////////////////////////////////////////////////////////////

View_routine *View_impl::add_routine() {
  View_routine_impl *vr = new (std::nothrow) View_routine_impl(this);
  m_routines.push_back(vr);
  return vr;
}

///////////////////////////////////////////////////////////////////////////

void View_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Tables>();

  otx->register_tables<Column>();
  otx->register_tables<View_table>();
  otx->register_tables<View_routine>();
}

///////////////////////////////////////////////////////////////////////////

View_impl::View_impl(const View_impl &src)
    : Weak_object(src),
      Abstract_table_impl(src),
      m_type(src.m_type),
      m_is_updatable(src.m_is_updatable),
      m_check_option(src.m_check_option),
      m_algorithm(src.m_algorithm),
      m_security_type(src.m_security_type),
      m_definition(src.m_definition),
      m_definition_utf8(src.m_definition_utf8),
      m_definer_user(src.m_definer_user),
      m_definer_host(src.m_definer_host),
      m_column_names(src.m_column_names),
      m_tables(),
      m_routines(),
      m_client_collation_id(src.m_client_collation_id),
      m_connection_collation_id(src.m_connection_collation_id) {
  m_tables.deep_copy(src.m_tables, this);
  m_routines.deep_copy(src.m_routines, this);
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
