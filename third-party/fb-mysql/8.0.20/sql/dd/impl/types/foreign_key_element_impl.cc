/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/types/foreign_key_element_impl.h"

#include <sstream>
#include <string>

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "m_string.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"                // ER_*
#include "sql/dd/impl/raw/raw_record.h"  // Raw_record
#include "sql/dd/impl/sdi_impl.h"        // sdi read/write functions
#include "sql/dd/impl/tables/foreign_key_column_usage.h"  // Foreign_key_column_usage
#include "sql/dd/impl/transaction_impl.h"        // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/foreign_key_impl.h"  // Foreign_key_impl
#include "sql/dd/impl/types/table_impl.h"        // Table_impl
#include "sql/dd/string_type.h"                  // dd::String_type
#include "sql/dd/types/column.h"                 // Column
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/weak_object.h"

namespace dd {
class Entity_object_impl;
}  // namespace dd

using dd::tables::Foreign_key_column_usage;

namespace dd {

class Foreign_key;
class Object_key;
class Sdi_rcontext;
class Sdi_wcontext;

///////////////////////////////////////////////////////////////////////////
// Foreign_key_element_impl implementation.
///////////////////////////////////////////////////////////////////////////

const Foreign_key &Foreign_key_element_impl::foreign_key() const {
  return *m_foreign_key;
}

Foreign_key &Foreign_key_element_impl::foreign_key() { return *m_foreign_key; }

///////////////////////////////////////////////////////////////////////////

bool Foreign_key_element_impl::validate() const {
  if (!m_foreign_key) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No foreign key associated with this element.");
    return true;
  }

  if (!m_column) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No Column is associated with this key element.");
    return true;
  }

  if (m_referenced_column_name.empty()) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Referenced column name is not set.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Foreign_key_element_impl::restore_attributes(const Raw_record &r) {
  // Must resolve ambiguity by static cast.
  if (check_parent_consistency(
          static_cast<Entity_object_impl *>(m_foreign_key),
          r.read_ref_id(Foreign_key_column_usage::FIELD_FOREIGN_KEY_ID)))
    return true;

  m_ordinal_position =
      r.read_uint(Foreign_key_column_usage::FIELD_ORDINAL_POSITION);

  m_referenced_column_name =
      r.read_str(Foreign_key_column_usage::FIELD_REFERENCED_COLUMN_NAME);

  m_column = m_foreign_key->table_impl().get_column(
      r.read_ref_id(Foreign_key_column_usage::FIELD_COLUMN_ID));

  return (m_column == nullptr);
}

///////////////////////////////////////////////////////////////////////////

bool Foreign_key_element_impl::store_attributes(Raw_record *r) {
  return r->store(Foreign_key_column_usage::FIELD_ORDINAL_POSITION,
                  m_ordinal_position) ||
         r->store(Foreign_key_column_usage::FIELD_FOREIGN_KEY_ID,
                  m_foreign_key->id()) ||
         r->store(Foreign_key_column_usage::FIELD_COLUMN_ID, m_column->id()) ||
         r->store(Foreign_key_column_usage::FIELD_REFERENCED_COLUMN_NAME,
                  m_referenced_column_name);
}

///////////////////////////////////////////////////////////////////////////

static_assert(
    Foreign_key_column_usage::FIELD_REFERENCED_COLUMN_NAME == 3,
    "Foreign_key_column_usage definition has changed, review (de)ser memfuns!");
void Foreign_key_element_impl::serialize(Sdi_wcontext *, Sdi_writer *w) const {
  w->StartObject();
  write_opx_reference(w, m_column, STRING_WITH_LEN("column_opx"));
  write(w, m_ordinal_position, STRING_WITH_LEN("ordinal_position"));
  write(w, m_referenced_column_name, STRING_WITH_LEN("referenced_column_name"));
  w->EndObject();
}

///////////////////////////////////////////////////////////////////////////

bool Foreign_key_element_impl::deserialize(Sdi_rcontext *rctx,
                                           const RJ_Value &val) {
  read_opx_reference(rctx, &m_column, val, "column_opx");
  read(&m_ordinal_position, val, "ordinal_position");
  read(&m_referenced_column_name, val, "referenced_column_name");
  return false;
}

///////////////////////////////////////////////////////////////////////////
/* purecov: begin inspected */
void Foreign_key_element_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "FOREIGN_KEY_ELEMENT OBJECT: { "
     << "m_foreign_key: {OID: " << m_foreign_key->id() << "}; "
     << "m_column: {OID: " << m_column->id() << "}; "
     << "m_referenced_column_name: " << m_referenced_column_name << "; ";

  ss << " }";

  outb = ss.str();
}
/* purecov: end */

///////////////////////////////////////////////////////////////////////////

Object_key *Foreign_key_element_impl::create_primary_key() const {
  return Foreign_key_column_usage::create_primary_key(m_foreign_key->id(),
                                                      m_ordinal_position);
}

bool Foreign_key_element_impl::has_new_primary_key() const {
  return m_foreign_key->has_new_primary_key();
}

///////////////////////////////////////////////////////////////////////////

Foreign_key_element_impl *Foreign_key_element_impl::clone(
    const Foreign_key_element_impl &other, Foreign_key_impl *fk) {
  return new Foreign_key_element_impl(
      other, fk, fk->table_impl().get_column(other.column().name()));
}

Foreign_key_element_impl::Foreign_key_element_impl(
    const Foreign_key_element_impl &src, Foreign_key_impl *parent,
    Column *column)
    : Weak_object(src),
      m_foreign_key(parent),
      m_column(column),
      m_ordinal_position(src.m_ordinal_position),
      m_referenced_column_name(src.m_referenced_column_name) {}

///////////////////////////////////////////////////////////////////////////

const Object_table &Foreign_key_element_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Foreign_key_element_impl::register_tables(
    Open_dictionary_tables_ctx *otx) {
  otx->add_table<Foreign_key_column_usage>();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
