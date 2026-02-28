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

#include "sql/dd/impl/types/partition_value_impl.h"

#include <ostream>
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
#include "sql/dd/impl/tables/table_partition_values.h"  // Table_partition_values
#include "sql/dd/impl/transaction_impl.h"      // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/partition_impl.h"  // Partition_impl
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/weak_object.h"

namespace dd {
class Object_key;
class Partition;
class Sdi_rcontext;
class Sdi_wcontext;
class Entity_object_impl;
}  // namespace dd

using dd::tables::Table_partition_values;

namespace dd {

///////////////////////////////////////////////////////////////////////////
// Partition_value_impl implementation.
///////////////////////////////////////////////////////////////////////////

const Partition &Partition_value_impl::partition() const {
  return *m_partition;
}

Partition &Partition_value_impl::partition() { return *m_partition; }

///////////////////////////////////////////////////////////////////////////

bool Partition_value_impl::validate() const {
  if (!m_partition) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No partition object associated.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Partition_value_impl::restore_attributes(const Raw_record &r) {
  // Must resolve ambiguity by static cast.
  if (check_parent_consistency(
          static_cast<Entity_object_impl *>(m_partition),
          r.read_ref_id(Table_partition_values::FIELD_PARTITION_ID)))
    return true;

  m_list_num = r.read_uint(Table_partition_values::FIELD_LIST_NUM);

  m_column_num = r.read_uint(Table_partition_values::FIELD_COLUMN_NUM);

  if (r.is_null(Table_partition_values::FIELD_VALUE_UTF8)) {
    m_null_value = true;
    m_value_utf8.clear();
  } else {
    m_null_value = false;
    m_value_utf8 = r.read_str(Table_partition_values::FIELD_VALUE_UTF8);
  }

  m_max_value = r.read_bool(Table_partition_values::FIELD_MAX_VALUE);

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Partition_value_impl::store_attributes(Raw_record *r) {
  return r->store(Table_partition_values::FIELD_PARTITION_ID,
                  m_partition->id()) ||
         r->store(Table_partition_values::FIELD_LIST_NUM, m_list_num) ||
         r->store(Table_partition_values::FIELD_COLUMN_NUM, m_column_num) ||
         r->store(Table_partition_values::FIELD_VALUE_UTF8, m_value_utf8,
                  m_null_value) ||
         r->store(Table_partition_values::FIELD_MAX_VALUE, m_max_value);
}

///////////////////////////////////////////////////////////////////////////

static_assert(
    Table_partition_values::FIELD_MAX_VALUE == 4,
    "Table_partition_value definition has changed, review (de)ser memfuns!");
void Partition_value_impl::serialize(Sdi_wcontext *, Sdi_writer *w) const {
  w->StartObject();
  write(w, m_max_value, STRING_WITH_LEN("max_value"));
  write(w, m_null_value, STRING_WITH_LEN("null_value"));
  write(w, m_list_num, STRING_WITH_LEN("list_num"));
  write(w, m_column_num, STRING_WITH_LEN("column_num"));
  write(w, m_value_utf8, STRING_WITH_LEN("value_utf8"));
  w->EndObject();
}

///////////////////////////////////////////////////////////////////////////

bool Partition_value_impl::deserialize(Sdi_rcontext *, const RJ_Value &val) {
  read(&m_max_value, val, "max_value");
  read(&m_null_value, val, "null_value");
  read(&m_list_num, val, "list_num");
  read(&m_column_num, val, "column_num");
  read(&m_value_utf8, val, "value_utf8");
  return false;
}

///////////////////////////////////////////////////////////////////////////

void Partition_value_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "PARTITION_VALUE OBJECT: { "
     << "m_partition: {OID: " << m_partition->id() << "}; "
     << "m_list_num: " << m_list_num << "; "
     << "m_column_num: " << m_column_num << "; "
     << "m_value_utf8: " << m_value_utf8 << "; "
     << "m_max_value: " << m_max_value << "; "
     << "m_null_value: " << m_null_value << "; ";

  ss << " }";

  outb = ss.str();
}

///////////////////////////////////////////////////////////////////////////

Object_key *Partition_value_impl::create_primary_key() const {
  return Table_partition_values::create_primary_key(m_partition->id(),
                                                    m_list_num, m_column_num);
}

bool Partition_value_impl::has_new_primary_key() const {
  return m_partition->has_new_primary_key();
}

///////////////////////////////////////////////////////////////////////////

Partition_value_impl::Partition_value_impl(const Partition_value_impl &src,
                                           Partition_impl *parent)
    : Weak_object(src),
      m_max_value(src.m_max_value),
      m_null_value(src.m_null_value),
      m_list_num(src.m_list_num),
      m_column_num(src.m_column_num),
      m_value_utf8(src.m_value_utf8),
      m_partition(parent) {}

///////////////////////////////////////////////////////////////////////////

const Object_table &Partition_value_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Partition_value_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Table_partition_values>();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
