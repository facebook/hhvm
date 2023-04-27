/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/types/view_table_impl.h"

#include <ostream>
#include <string>

#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"                         // ER_*
#include "sql/dd/impl/raw/raw_record.h"           // Raw_record
#include "sql/dd/impl/tables/view_table_usage.h"  // View_table_usage
#include "sql/dd/impl/transaction_impl.h"         // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/view_impl.h"          // View_impl
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/weak_object.h"

namespace dd {
class Object_key;
}  // namespace dd

using dd::tables::View_table_usage;

namespace dd {

///////////////////////////////////////////////////////////////////////////
// View_table_impl implementation.
///////////////////////////////////////////////////////////////////////////

View_table_impl::View_table_impl() {}

View_table_impl::View_table_impl(View_impl *view) : m_view(view) {}

///////////////////////////////////////////////////////////////////////////

const View &View_table_impl::view() const { return *m_view; }

View &View_table_impl::view() { return *m_view; }

///////////////////////////////////////////////////////////////////////////

bool View_table_impl::validate() const {
  if (!m_view) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No view is associated with this view table object.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool View_table_impl::restore_attributes(const Raw_record &r) {
  if (check_parent_consistency(m_view,
                               r.read_ref_id(View_table_usage::FIELD_VIEW_ID)))
    return true;

  m_table_catalog = r.read_str(View_table_usage::FIELD_TABLE_CATALOG);
  m_table_schema = r.read_str(View_table_usage::FIELD_TABLE_SCHEMA);
  m_table_name = r.read_str(View_table_usage::FIELD_TABLE_NAME);

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool View_table_impl::store_attributes(Raw_record *r) {
  return r->store(View_table_usage::FIELD_VIEW_ID, m_view->id()) ||
         r->store(View_table_usage::FIELD_TABLE_CATALOG, m_table_catalog) ||
         r->store(View_table_usage::FIELD_TABLE_SCHEMA, m_table_schema) ||
         r->store(View_table_usage::FIELD_TABLE_NAME, m_table_name);
}

///////////////////////////////////////////////////////////////////////////

void View_table_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "VIEW TABLE OBJECT: { "
     << "m_view: {OID: " << m_view->id() << "}; "
     << "m_table_catalog: " << m_table_catalog << "; "
     << "m_table_schema: " << m_table_schema << "; "
     << "m_table_name: " << m_table_name;

  outb = ss.str();
}

///////////////////////////////////////////////////////////////////////////

Object_key *View_table_impl::create_primary_key() const {
  return View_table_usage::create_primary_key(m_view->id(), m_table_catalog,
                                              m_table_schema, m_table_name);
}

bool View_table_impl::has_new_primary_key() const {
  return m_view->has_new_primary_key();
}

///////////////////////////////////////////////////////////////////////////

View_table_impl::View_table_impl(const View_table_impl &src, View_impl *parent)
    : Weak_object(src),
      m_table_catalog(src.m_table_catalog),
      m_table_schema(src.m_table_schema),
      m_table_name(src.m_table_name),
      m_view(parent) {}

///////////////////////////////////////////////////////////////////////////

const Object_table &View_table_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void View_table_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<View_table_usage>();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
