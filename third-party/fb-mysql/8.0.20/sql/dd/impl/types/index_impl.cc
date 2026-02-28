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

#include "sql/dd/impl/types/index_impl.h"

#include <stddef.h>
#include <set>
#include <sstream>
#include <string>

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "m_string.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"                           // ER_*
#include "sql/dd/impl/properties_impl.h"            // Properties_impl
#include "sql/dd/impl/raw/raw_record.h"             // Raw_record
#include "sql/dd/impl/sdi_impl.h"                   // sdi read/write functions
#include "sql/dd/impl/tables/index_column_usage.h"  // Index_column_usage
#include "sql/dd/impl/tables/indexes.h"             // Indexes
#include "sql/dd/impl/transaction_impl.h"          // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/index_element_impl.h"  // Index_element_impl
#include "sql/dd/impl/types/table_impl.h"          // Table_impl
#include "sql/dd/properties.h"
#include "sql/dd/string_type.h"  // dd::String_type
#include "sql/dd/types/column.h"
#include "sql/dd/types/index_element.h"
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/weak_object.h"
#include "sql/field.h"

using dd::tables::Index_column_usage;
using dd::tables::Indexes;

namespace dd {

class Sdi_rcontext;
class Sdi_wcontext;
class Table;

static const std::set<String_type> default_valid_option_keys = {
    "block_size", "flags", "parser_name"};

///////////////////////////////////////////////////////////////////////////
// Index_impl implementation.
///////////////////////////////////////////////////////////////////////////

Index_impl::Index_impl()
    : m_hidden(false),
      m_is_generated(false),
      m_ordinal_position(0),
      m_options(default_valid_option_keys),
      m_se_private_data(),
      m_type(IT_MULTIPLE),
      m_algorithm(IA_BTREE),
      m_is_algorithm_explicit(false),
      m_is_visible(true),
      m_table(nullptr),
      m_elements(),
      m_tablespace_id(INVALID_OBJECT_ID) {}

Index_impl::Index_impl(Table_impl *table)
    : m_hidden(false),
      m_is_generated(false),
      m_ordinal_position(0),
      m_options(default_valid_option_keys),
      m_se_private_data(),
      m_type(IT_MULTIPLE),
      m_algorithm(IA_BTREE),
      m_is_algorithm_explicit(false),
      m_is_visible(true),
      m_table(table),
      m_elements(),
      m_tablespace_id(INVALID_OBJECT_ID) {}

Index_impl::~Index_impl() {}

///////////////////////////////////////////////////////////////////////////

const Table &Index_impl::table() const { return *m_table; }

Table &Index_impl::table() { return *m_table; }

///////////////////////////////////////////////////////////////////////////

bool Index_impl::validate() const {
  if (!m_table) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No table object associated with this index.");
    return true;
  }
  if (m_engine.empty()) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Engine name is not set.");
    return true;
  }

  if (m_elements.empty()) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "The index has no elements.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Index_impl::restore_children(Open_dictionary_tables_ctx *otx) {
  return m_elements.restore_items(
      // Column will be resolved in restore_attributes() called from
      // Collection::restore_items().
      this, otx, otx->get_table<Index_element>(),
      Index_column_usage::create_key_by_index_id(this->id()));
}

///////////////////////////////////////////////////////////////////////////

bool Index_impl::store_children(Open_dictionary_tables_ctx *otx) {
  return m_elements.store_items(otx);
}

///////////////////////////////////////////////////////////////////////////

bool Index_impl::drop_children(Open_dictionary_tables_ctx *otx) const {
  return m_elements.drop_items(
      otx, otx->get_table<Index_element>(),
      Index_column_usage::create_key_by_index_id(this->id()));
}

///////////////////////////////////////////////////////////////////////////

bool Index_impl::restore_attributes(const Raw_record &r) {
  if (check_parent_consistency(m_table, r.read_ref_id(Indexes::FIELD_TABLE_ID)))
    return true;

  restore_id(r, Indexes::FIELD_ID);
  restore_name(r, Indexes::FIELD_NAME);

  m_hidden = r.read_bool(Indexes::FIELD_HIDDEN);
  m_is_generated = r.read_bool(Indexes::FIELD_IS_GENERATED);
  m_ordinal_position = r.read_uint(Indexes::FIELD_ORDINAL_POSITION);
  m_comment = r.read_str(Indexes::FIELD_COMMENT);

  m_type = (enum_index_type)r.read_int(Indexes::FIELD_TYPE);
  m_algorithm = (enum_index_algorithm)r.read_int(Indexes::FIELD_ALGORITHM);
  m_is_algorithm_explicit = r.read_bool(Indexes::FIELD_IS_ALGORITHM_EXPLICIT);
  m_is_visible = r.read_bool(Indexes::FIELD_IS_VISIBLE);

  m_tablespace_id = r.read_ref_id(Indexes::FIELD_TABLESPACE_ID);

  set_options(r.read_str(Indexes::FIELD_OPTIONS, ""));
  set_se_private_data(r.read_str(Indexes::FIELD_SE_PRIVATE_DATA, ""));

  m_engine = r.read_str(Indexes::FIELD_ENGINE);

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Index_impl::store_attributes(Raw_record *r) {
  //
  // Special cases dealing with NULL values for nullable fields
  //   - Store NULL if tablespace id is not set
  //     Eg: A non-innodb table may not have tablespace
  //   - Store NULL if se_private_id is not set
  //     Eg: A non-innodb table may not have se_private_id
  //   - Store NULL in options if there are no key=value pairs
  //   - Store NULL in se_private_data if there are no key=value pairs

  return store_id(r, Indexes::FIELD_ID) || store_name(r, Indexes::FIELD_NAME) ||
         r->store(Indexes::FIELD_TABLE_ID, m_table->id()) ||
         r->store(Indexes::FIELD_TYPE, m_type) ||
         r->store(Indexes::FIELD_ALGORITHM, m_algorithm) ||
         r->store(Indexes::FIELD_IS_ALGORITHM_EXPLICIT,
                  m_is_algorithm_explicit) ||
         r->store(Indexes::FIELD_IS_VISIBLE, m_is_visible) ||
         r->store(Indexes::FIELD_IS_GENERATED, m_is_generated) ||
         r->store(Indexes::FIELD_HIDDEN, m_hidden) ||
         r->store(Indexes::FIELD_ORDINAL_POSITION, m_ordinal_position) ||
         r->store(Indexes::FIELD_COMMENT, m_comment) ||
         r->store(Indexes::FIELD_OPTIONS, m_options) ||
         r->store(Indexes::FIELD_SE_PRIVATE_DATA, m_se_private_data) ||
         r->store_ref_id(Indexes::FIELD_TABLESPACE_ID, m_tablespace_id) ||
         r->store(Indexes::FIELD_ENGINE, m_engine);
}

///////////////////////////////////////////////////////////////////////////
static_assert(Indexes::FIELD_ENGINE == 14,
              "Indexes definition has changed, review (de)ser memfuns!");
void Index_impl::serialize(Sdi_wcontext *wctx, Sdi_writer *w) const {
  w->StartObject();
  Entity_object_impl::serialize(wctx, w);

  write(w, m_hidden, STRING_WITH_LEN("hidden"));
  write(w, m_is_generated, STRING_WITH_LEN("is_generated"));
  write(w, m_ordinal_position, STRING_WITH_LEN("ordinal_position"));
  write(w, m_comment, STRING_WITH_LEN("comment"));

  write_properties(w, m_options, STRING_WITH_LEN("options"));
  write_properties(w, m_se_private_data, STRING_WITH_LEN("se_private_data"));
  write_enum(w, m_type, STRING_WITH_LEN("type"));
  write_enum(w, m_algorithm, STRING_WITH_LEN("algorithm"));
  write(w, m_is_algorithm_explicit, STRING_WITH_LEN("is_algorithm_explicit"));
  write(w, m_is_visible, STRING_WITH_LEN("is_visible"));
  write(w, m_engine, STRING_WITH_LEN("engine"));

  serialize_each(wctx, w, m_elements, STRING_WITH_LEN("elements"));

  serialize_tablespace_ref(wctx, w, m_tablespace_id,
                           STRING_WITH_LEN("tablespace_ref"));
  w->EndObject();
}

///////////////////////////////////////////////////////////////////////////

bool Index_impl::deserialize(Sdi_rcontext *rctx, const RJ_Value &val) {
  Entity_object_impl::deserialize(rctx, val);

  read(&m_hidden, val, "hidden");
  read(&m_is_generated, val, "is_generated");
  read(&m_ordinal_position, val, "ordinal_position");
  read(&m_comment, val, "comment");
  read_properties(&m_options, val, "options");
  read_properties(&m_se_private_data, val, "se_private_data");
  read_enum(&m_type, val, "type");
  read_enum(&m_algorithm, val, "algorithm");
  read(&m_is_algorithm_explicit, val, "is_algorithm_explicit");
  read(&m_is_visible, val, "is_visible");
  read(&m_engine, val, "engine");

  deserialize_each(
      rctx, [this]() { return add_element(nullptr); }, val, "elements");

  if (deserialize_tablespace_ref(rctx, &m_tablespace_id, val,
                                 "tablespace_name")) {
    return true;
  }

  track_object(rctx, this);

  return false;
}

///////////////////////////////////////////////////////////////////////////

void Index_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "INDEX OBJECT: { "
     << "id: {OID: " << id() << "}; "
     << "m_table: {OID: " << m_table->id() << "}; "
     << "m_name: " << name() << "; "
     << "m_type: " << m_type << "; "
     << "m_algorithm: " << m_algorithm << "; "
     << "m_is_algorithm_explicit: " << m_is_algorithm_explicit << "; "
     << "m_is_visible: " << m_is_visible << "; "
     << "m_is_generated: " << m_is_generated << "; "
     << "m_comment: " << m_comment << "; "
     << "m_hidden: " << m_hidden << "; "
     << "m_ordinal_position: " << m_ordinal_position << "; "
     << "m_options " << m_options.raw_string() << "; "
     << "m_se_private_data " << m_se_private_data.raw_string() << "; "
     << "m_tablespace {OID: " << m_tablespace_id << "}; "
     << "m_engine: " << m_engine << "; "
     << "m_elements: " << m_elements.size() << " [ ";

  {
    for (const Index_element *c : elements()) {
      String_type ob;
      c->debug_print(ob);
      ss << ob;
    }
  }
  ss << "] ";

  ss << " }";

  outb = ss.str();
}

/////////////////////////////////////////////////////////////////////////

Index_element *Index_impl::add_element(Column *c) {
  Index_element_impl *e = new (std::nothrow) Index_element_impl(this, c);
  m_elements.push_back(e);
  return e;
}

//////////////////////////////////////////////////////////////////////////

/**
  Check if index represents candidate key.

  @note This function is in sync with how we evaluate TABLE_SHARE::primary_key.
*/

bool Index_impl::is_candidate_key() const {
  if (type() != Index::IT_PRIMARY && type() != Index::IT_UNIQUE) return false;

  for (const Index_element *idx_elem_obj : elements()) {
    // Skip hidden index elements
    if (idx_elem_obj->is_hidden()) continue;

    if (idx_elem_obj->column().is_nullable()) return false;

    if (idx_elem_obj->column().is_virtual()) return false;

    if (idx_elem_obj->column().type() == enum_column_types::GEOMETRY)
      return false;

    /*
      Probably we should adjust is_prefix() to take these two scenarios
      into account. But this also means that we probably need avoid
      setting HA_PART_KEY_SEG in them.
    */

    if ((idx_elem_obj->column().type() == enum_column_types::TINY_BLOB &&
         idx_elem_obj->length() == 255) ||
        (idx_elem_obj->column().type() == enum_column_types::BLOB &&
         idx_elem_obj->length() == 65535) ||
        (idx_elem_obj->column().type() == enum_column_types::MEDIUM_BLOB &&
         idx_elem_obj->length() == (1 << 24) - 1) ||
        (idx_elem_obj->column().type() == enum_column_types::LONG_BLOB &&
         idx_elem_obj->length() == (1LL << 32) - 1))
      continue;

    if (idx_elem_obj->is_prefix()) return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////

Index_impl::Index_impl(const Index_impl &src, Table_impl *parent)
    : Weak_object(src),
      Entity_object_impl(src),
      m_hidden(src.m_hidden),
      m_is_generated(src.m_is_generated),
      m_ordinal_position(src.m_ordinal_position),
      m_comment(src.m_comment),
      m_options(src.m_options),
      m_se_private_data(src.m_se_private_data),
      m_type(src.m_type),
      m_algorithm(src.m_algorithm),
      m_is_algorithm_explicit(src.m_is_algorithm_explicit),
      m_is_visible(src.m_is_visible),
      m_engine(src.m_engine),
      m_table(parent),
      m_elements(),
      m_tablespace_id(src.m_tablespace_id) {
  m_elements.deep_copy(src.m_elements, this);
}

///////////////////////////////////////////////////////////////////////////

const Object_table &Index_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Index_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Indexes>();

  otx->register_tables<Index_element>();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
