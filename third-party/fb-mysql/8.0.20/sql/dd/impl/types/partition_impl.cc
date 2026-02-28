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

#include "sql/dd/impl/types/partition_impl.h"

#include <stddef.h>
#include <set>
#include <sstream>
#include <string>

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"                         // ER_*
#include "sql/dd/impl/raw/raw_record.h"           // Raw_record
#include "sql/dd/impl/sdi_impl.h"                 // sdi read/write functions
#include "sql/dd/impl/tables/index_partitions.h"  // Index_partitions
#include "sql/dd/impl/tables/table_partition_values.h"  // Table_partition_values
#include "sql/dd/impl/tables/table_partitions.h"        // Table_partitions
#include "sql/dd/impl/transaction_impl.h"  // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/partition_index_impl.h"  // Partition_index_impl
#include "sql/dd/impl/types/partition_value_impl.h"  // Partition_value_impl
#include "sql/dd/impl/types/table_impl.h"            // Table_impl
#include "sql/dd/string_type.h"                      // dd::String_type
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/partition_index.h"
#include "sql/dd/types/partition_value.h"
#include "sql/dd/types/weak_object.h"

using dd::tables::Index_partitions;
using dd::tables::Table_partition_values;
using dd::tables::Table_partitions;

namespace dd {

class Index;
class Sdi_rcontext;
class Sdi_wcontext;

static const std::set<String_type> default_valid_option_keys = {
    "data_file_name", "explicit_tablespace", "index_file_name", "max_rows",
    "min_rows",       "nodegroup_id",        "tablespace"};

///////////////////////////////////////////////////////////////////////////
// Partition_impl implementation.
///////////////////////////////////////////////////////////////////////////

Partition_impl::Partition_impl()
    : m_parent_partition_id(INVALID_OBJECT_ID),
      m_number(-1),
      m_se_private_id(INVALID_OBJECT_ID),
      m_options(default_valid_option_keys),
      m_se_private_data(),
      m_table(nullptr),
      m_parent(nullptr),
      m_values(),
      m_indexes(),
      m_subpartitions(),
      m_tablespace_id(INVALID_OBJECT_ID) {}

Partition_impl::Partition_impl(Table_impl *table)
    : m_parent_partition_id(INVALID_OBJECT_ID),
      m_number(-1),
      m_se_private_id((ulonglong)-1),
      m_options(default_valid_option_keys),
      m_se_private_data(),
      m_table(table),
      m_parent(nullptr),
      m_values(),
      m_indexes(),
      m_subpartitions(),
      m_tablespace_id(INVALID_OBJECT_ID) {
  if (m_table->subpartition_type() == Table::ST_NONE)
    m_table->add_leaf_partition(this);
}

Partition_impl::Partition_impl(Table_impl *table, Partition_impl *parent)
    : m_parent_partition_id(INVALID_OBJECT_ID),
      m_number(-1),
      m_se_private_id((ulonglong)-1),
      m_options(default_valid_option_keys),
      m_se_private_data(),
      m_table(table),
      m_parent(parent),
      m_values(),
      m_indexes(),
      m_subpartitions(),
      m_tablespace_id(INVALID_OBJECT_ID) {
  m_table->add_leaf_partition(this);
}

Partition_impl::~Partition_impl() {}

///////////////////////////////////////////////////////////////////////////

const Table &Partition_impl::table() const { return *m_table; }

Table &Partition_impl::table() { return *m_table; }

///////////////////////////////////////////////////////////////////////////

bool Partition_impl::validate() const {
  if (!m_table) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No table object associated with this partition.");
    return true;
  }

  if (m_engine.empty()) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Engine name is not set.");
    return true;
  }

  // Partition values only relevant for LIST and RANGE partitioning,
  // not for KEY and HASH, so no validation on m_values.

  if ((m_parent_partition_id == dd::INVALID_OBJECT_ID && m_parent) ||
      (m_parent_partition_id != dd::INVALID_OBJECT_ID && !m_parent)) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Partition parent_partition_id not set.");
    return true;
  }

  if (m_number == (uint)-1) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Partition number not set.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Partition_impl::restore_children(Open_dictionary_tables_ctx *otx) {
  return m_values.restore_items(
             this, otx, otx->get_table<Partition_value>(),
             Table_partition_values::create_key_by_partition_id(this->id()),
             Partition_value_order_comparator()) ||
         m_indexes.restore_items(
             // Index will be resolved in restore_attributes()
             // called from Collection::restore_items().
             this, otx, otx->get_table<Partition_index>(),
             Index_partitions::create_key_by_partition_id(this->id()),
             Partition_index_order_comparator()) ||
         m_subpartitions.restore_items(
             this, otx, otx->get_table<Partition>(),
             Table_partitions::create_key_by_parent_partition_id(
                 this->table().id(), this->id()),
             Partition_order_comparator());
}

///////////////////////////////////////////////////////////////////////////

bool Partition_impl::store_children(Open_dictionary_tables_ctx *otx) {
  for (Partition *part : m_subpartitions) part->set_parent_partition_id(id());

  return m_values.store_items(otx) || m_indexes.store_items(otx) ||
         m_subpartitions.store_items(otx);
}

///////////////////////////////////////////////////////////////////////////

bool Partition_impl::drop_children(Open_dictionary_tables_ctx *otx) const {
  return m_values.drop_items(
             otx, otx->get_table<Partition_value>(),
             Table_partition_values::create_key_by_partition_id(this->id())) ||
         m_indexes.drop_items(
             otx, otx->get_table<Partition_index>(),
             Index_partitions::create_key_by_partition_id(this->id())) ||
         m_subpartitions.drop_items(
             otx, otx->get_table<Partition>(),
             Table_partitions::create_key_by_parent_partition_id(
                 this->table().id(), this->id()));
}

///////////////////////////////////////////////////////////////////////////

bool Partition_impl::restore_attributes(const Raw_record &r) {
  if (check_parent_consistency(m_table,
                               r.read_ref_id(Table_partitions::FIELD_TABLE_ID)))
    return true;

  restore_id(r, Table_partitions::FIELD_ID);
  restore_name(r, Table_partitions::FIELD_NAME);

  m_parent_partition_id = r.read_uint(
      Table_partitions::FIELD_PARENT_PARTITION_ID, dd::INVALID_OBJECT_ID);

  m_number = r.read_uint(Table_partitions::FIELD_NUMBER);

  m_description_utf8 = r.read_str(Table_partitions::FIELD_DESCRIPTION_UTF8);
  m_engine = r.read_str(Table_partitions::FIELD_ENGINE);
  m_comment = r.read_str(Table_partitions::FIELD_COMMENT);

  m_tablespace_id = r.read_ref_id(Table_partitions::FIELD_TABLESPACE_ID);

  m_se_private_id = r.read_uint(Table_partitions::FIELD_SE_PRIVATE_ID, -1);

  set_options(r.read_str(Table_partitions::FIELD_OPTIONS, ""));
  set_se_private_data(r.read_str(Table_partitions::FIELD_SE_PRIVATE_DATA, ""));

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Partition_impl::store_attributes(Raw_record *r) {
  return store_id(r, Table_partitions::FIELD_ID) ||
         store_name(r, Table_partitions::FIELD_NAME) ||
         r->store(Table_partitions::FIELD_TABLE_ID, m_table->id()) ||
         r->store(Table_partitions::FIELD_PARENT_PARTITION_ID,
                  m_parent_partition_id,
                  m_parent_partition_id == dd::INVALID_OBJECT_ID) ||
         r->store(Table_partitions::FIELD_NUMBER, m_number) ||
         r->store(Table_partitions::FIELD_DESCRIPTION_UTF8, m_description_utf8,
                  m_description_utf8.empty()) ||
         r->store(Table_partitions::FIELD_ENGINE, m_engine) ||
         r->store(Table_partitions::FIELD_COMMENT, m_comment) ||
         r->store(Table_partitions::FIELD_OPTIONS, m_options) ||
         r->store(Table_partitions::FIELD_SE_PRIVATE_DATA, m_se_private_data) ||
         r->store(Table_partitions::FIELD_SE_PRIVATE_ID, m_se_private_id,
                  m_se_private_id == INVALID_OBJECT_ID) ||
         r->store_ref_id(Table_partitions::FIELD_TABLESPACE_ID,
                         m_tablespace_id);
}

///////////////////////////////////////////////////////////////////////////
static_assert(
    Table_partitions::FIELD_TABLESPACE_ID == 11,
    "Table_partitions definition has changed. Review (de)ser memfuns!");
void Partition_impl::serialize(Sdi_wcontext *wctx, Sdi_writer *w) const {
  w->StartObject();
  Entity_object_impl::serialize(wctx, w);
  write(w, m_parent_partition_id, STRING_WITH_LEN("parent_partition_id"));
  write(w, m_number, STRING_WITH_LEN("number"));
  write(w, m_se_private_id, STRING_WITH_LEN("se_private_id"));
  write(w, m_description_utf8, STRING_WITH_LEN("description_utf8"));
  write(w, m_engine, STRING_WITH_LEN("engine"));
  write(w, m_comment, STRING_WITH_LEN("comment"));
  write_properties(w, m_options, STRING_WITH_LEN("options"));
  write_properties(w, m_se_private_data, STRING_WITH_LEN("se_private_data"));
  serialize_each(wctx, w, m_values, STRING_WITH_LEN("values"));
  serialize_each(wctx, w, m_indexes, STRING_WITH_LEN("indexes"));
  serialize_each(wctx, w, m_subpartitions, STRING_WITH_LEN("subpartitions"));
  serialize_tablespace_ref(wctx, w, m_tablespace_id,
                           STRING_WITH_LEN("tablespace_ref"));
  w->EndObject();
}

///////////////////////////////////////////////////////////////////////////

bool Partition_impl::deserialize(Sdi_rcontext *rctx, const RJ_Value &val) {
  Entity_object_impl::deserialize(rctx, val);
  read(&m_parent_partition_id, val, "parent_partition_id");
  read(&m_number, val, "number");
  read(&m_se_private_id, val, "se_private_id");
  read(&m_description_utf8, val, "description_utf8");
  read(&m_engine, val, "engine");
  read(&m_comment, val, "comment");
  read_properties(&m_options, val, "options");
  read_properties(&m_se_private_data, val, "se_private_data");
  deserialize_each(
      rctx, [this]() { return add_value(); }, val, "values");
  deserialize_each(
      rctx, [this]() { return add_index(nullptr); }, val, "indexes");
  deserialize_each(
      rctx, [this]() { return add_subpartition(); }, val, "subpartitions");

  return deserialize_tablespace_ref(rctx, &m_tablespace_id, val,
                                    "tablespace_ref");
}

///////////////////////////////////////////////////////////////////////////

void Partition_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "Partition OBJECT: { "
     << "m_id: {OID: " << id() << "}; "
     << "m_table: {OID: " << m_table->id() << "}; "
     << "m_name: " << name() << "; "
     << "m_parent_partition_id: " << m_parent_partition_id << "; "
     << "m_number: " << m_number << "; "
     << "m_description_utf8: " << m_description_utf8 << "; "
     << "m_engine: " << m_engine << "; "
     << "m_comment: " << m_comment << "; "
     << "m_options " << m_options.raw_string() << "; "
     << "m_se_private_data " << m_se_private_data.raw_string() << "; "
     << "m_se_private_id: {OID: " << m_se_private_id << "}; "
     << "m_tablespace: {OID: " << m_tablespace_id << "}; "
     << "m_values: " << m_values.size() << " [ ";

  {
    for (const Partition_value *c : values()) {
      String_type ob;
      c->debug_print(ob);
      ss << ob;
    }
  }
  ss << "] ";

  ss << "m_indexes: " << m_indexes.size() << " [ ";

  {
    for (const Partition_index *i : indexes()) {
      String_type ob;
      i->debug_print(ob);
      ss << ob;
    }
  }
  ss << "] ";

  ss << "m_subpartitions: " << m_subpartitions.size() << " [ ";

  {
    for (const Partition *i : subpartitions()) {
      String_type ob;
      i->debug_print(ob);
      ss << ob;
    }
  }
  ss << "] ";

  ss << " }";

  outb = ss.str();
}

/////////////////////////////////////////////////////////////////////////

Partition_value *Partition_impl::add_value() {
  Partition_value_impl *e = new (std::nothrow) Partition_value_impl(this);
  m_values.push_back(e);
  return e;
}

///////////////////////////////////////////////////////////////////////////

Partition_index *Partition_impl::add_index(Index *idx) {
  Partition_index_impl *e = new (std::nothrow) Partition_index_impl(this, idx);
  m_indexes.push_back(e);
  return e;
}

///////////////////////////////////////////////////////////////////////////

Partition *Partition_impl::add_subpartition() {
  /// Support just one level of sub partitions.
  DBUG_ASSERT(!parent());

  Partition_impl *p =
      new (std::nothrow) Partition_impl(&this->table_impl(), this);
  p->set_parent(this);
  m_subpartitions.push_back(p);
  return p;
}

///////////////////////////////////////////////////////////////////////////

Partition_impl::Partition_impl(const Partition_impl &src, Table_impl *parent)
    : Weak_object(src),
      Entity_object_impl(src),
      m_parent_partition_id(src.m_parent_partition_id),
      m_number(src.m_number),
      m_se_private_id(src.m_se_private_id),
      m_description_utf8(src.m_description_utf8),
      m_engine(src.m_engine),
      m_comment(src.m_comment),
      m_options(src.m_options),
      m_se_private_data(src.m_se_private_data),
      m_table(parent),
      m_parent(src.parent() ? parent->get_partition(src.parent()->name())
                            : nullptr),
      m_values(),
      m_indexes(),
      m_subpartitions(),
      m_tablespace_id(src.m_tablespace_id) {
  m_values.deep_copy(src.m_values, this);
  m_indexes.deep_copy(src.m_indexes, this);
  m_subpartitions.deep_copy(src.m_subpartitions, this);

  if (m_table->subpartition_type() == Table::ST_NONE)
    m_table->add_leaf_partition(this);
}

///////////////////////////////////////////////////////////////////////////

Partition_impl::Partition_impl(const Partition_impl &src, Partition_impl *part)
    : Weak_object(src),
      Entity_object_impl(src),
      m_parent_partition_id(src.m_parent_partition_id),
      m_number(src.m_number),
      m_se_private_id(src.m_se_private_id),
      m_description_utf8(src.m_description_utf8),
      m_engine(src.m_engine),
      m_comment(src.m_comment),
      m_options(src.m_options),
      m_se_private_data(src.m_se_private_data),
      m_table(&part->table_impl()),
      m_parent(part),
      m_values(),
      m_indexes(),
      m_subpartitions(),
      m_tablespace_id(src.m_tablespace_id) {
  m_values.deep_copy(src.m_values, this);
  m_indexes.deep_copy(src.m_indexes, this);

  m_table->add_leaf_partition(this);
}

///////////////////////////////////////////////////////////////////////////

const Object_table &Partition_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Partition_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Table_partitions>();

  otx->register_tables<Partition_value>();
  otx->register_tables<Partition_index>();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
