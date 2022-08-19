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

#include "sql/dd/impl/types/column_impl.h"

#include <stddef.h>
#include <memory>
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
#include "mysqld_error.h"                 // ER_*
#include "sql/dd/impl/properties_impl.h"  // Properties_impl
#include "sql/dd/impl/raw/raw_record.h"   // Raw_record
#include "sql/dd/impl/sdi_impl.h"         // sdi read/write functions
#include "sql/dd/impl/tables/column_type_elements.h"  // Column_type_elements
#include "sql/dd/impl/tables/columns.h"               // Colummns
#include "sql/dd/impl/transaction_impl.h"  // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/abstract_table_impl.h"       // Abstract_table_impl
#include "sql/dd/impl/types/column_type_element_impl.h"  // Column_type_element_impl
#include "sql/dd/properties.h"
#include "sql/dd/string_type.h"                // dd::String_type
#include "sql/dd/types/column_type_element.h"  // Column_type_element
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/weak_object.h"

using dd::tables::Column_type_elements;
using dd::tables::Columns;

namespace dd {

class Abstract_table;
class Sdi_rcontext;
class Sdi_wcontext;

static const std::set<String_type> default_valid_option_keys = {
    "column_format", "geom_type",         "interval_count", "not_secondary",
    "storage",       "treat_bit_as_char", "is_array"};

///////////////////////////////////////////////////////////////////////////
// Column_impl implementation.
///////////////////////////////////////////////////////////////////////////

Column_impl::Column_impl()
    : m_type(enum_column_types::LONG),
      m_is_nullable(true),
      m_is_zerofill(false),
      m_is_unsigned(false),
      m_is_auto_increment(false),
      m_is_virtual(false),
      m_hidden(enum_hidden_type::HT_VISIBLE),
      m_ordinal_position(0),
      m_char_length(0),
      m_numeric_precision(0),
      m_numeric_scale(0),
      m_numeric_scale_null(true),
      m_datetime_precision(0),
      m_datetime_precision_null(true),
      m_has_no_default(false),
      m_default_value_null(true),
      m_default_value_utf8_null(true),
      m_options(default_valid_option_keys),
      m_se_private_data(),
      m_table(nullptr),
      m_elements(),
      m_collation_id(INVALID_OBJECT_ID),
      m_is_explicit_collation(false),
      m_column_key(CK_NONE) {}

Column_impl::Column_impl(Abstract_table_impl *table)
    : m_type(enum_column_types::LONG),
      m_is_nullable(true),
      m_is_zerofill(false),
      m_is_unsigned(false),
      m_is_auto_increment(false),
      m_is_virtual(false),
      m_hidden(enum_hidden_type::HT_VISIBLE),
      m_ordinal_position(0),
      m_char_length(0),
      m_numeric_precision(0),
      m_numeric_scale(0),
      m_numeric_scale_null(true),
      m_datetime_precision(0),
      m_datetime_precision_null(true),
      m_has_no_default(false),
      m_default_value_null(true),
      m_default_value_utf8_null(true),
      m_options(default_valid_option_keys),
      m_se_private_data(),
      m_table(table),
      m_elements(),
      m_collation_id(INVALID_OBJECT_ID),
      m_is_explicit_collation(false),
      m_column_key(CK_NONE) {}

Column_impl::~Column_impl() {}

///////////////////////////////////////////////////////////////////////////

const Abstract_table &Column_impl::table() const { return *m_table; }

Abstract_table &Column_impl::table() { return *m_table; }

///////////////////////////////////////////////////////////////////////////

bool Column_impl::validate() const {
  if (!m_table) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Column does not belong to any table.");
    return true;
  }

  if (m_collation_id == INVALID_OBJECT_ID) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Collation ID is not set");
    return true;
  }

  if ((type() == enum_column_types::ENUM ||
       type() == enum_column_types::ENUM) &&
      m_elements.empty()) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "There are no elements supplied.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Column_impl::restore_children(Open_dictionary_tables_ctx *otx) {
  switch (type()) {
    case enum_column_types::ENUM:
    case enum_column_types::SET:
      return m_elements.restore_items(
          this, otx, otx->get_table<Column_type_element>(),
          Column_type_elements::create_key_by_column_id(this->id()));

    default:
      return false;
  }
}

///////////////////////////////////////////////////////////////////////////

bool Column_impl::store_children(Open_dictionary_tables_ctx *otx) {
  return m_elements.store_items(otx);
}

///////////////////////////////////////////////////////////////////////////

bool Column_impl::drop_children(Open_dictionary_tables_ctx *otx) const {
  if (type() == enum_column_types::ENUM || type() == enum_column_types::SET)
    return m_elements.drop_items(
        otx, otx->get_table<Column_type_element>(),
        Column_type_elements::create_key_by_column_id(this->id()));

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Column_impl::restore_attributes(const Raw_record &r) {
  if (check_parent_consistency(m_table, r.read_ref_id(Columns::FIELD_TABLE_ID)))
    return true;

  restore_id(r, Columns::FIELD_ID);
  restore_name(r, Columns::FIELD_NAME);

  m_is_nullable = r.read_bool(Columns::FIELD_IS_NULLABLE);
  m_is_zerofill = r.read_bool(Columns::FIELD_IS_ZEROFILL);
  m_is_unsigned = r.read_bool(Columns::FIELD_IS_UNSIGNED);
  m_is_auto_increment = r.read_bool(Columns::FIELD_IS_AUTO_INCREMENT);
  m_hidden = static_cast<enum_hidden_type>(r.read_int(Columns::FIELD_HIDDEN));

  m_type = (enum_column_types)r.read_int(Columns::FIELD_TYPE);
  m_numeric_precision = r.read_uint(Columns::FIELD_NUMERIC_PRECISION);
  m_numeric_scale_null = r.is_null(Columns::FIELD_NUMERIC_SCALE);
  m_numeric_scale = r.read_uint(Columns::FIELD_NUMERIC_SCALE);
  m_datetime_precision = r.read_uint(Columns::FIELD_DATETIME_PRECISION);
  m_datetime_precision_null = r.is_null(Columns::FIELD_DATETIME_PRECISION);
  m_ordinal_position = r.read_uint(Columns::FIELD_ORDINAL_POSITION);
  m_char_length = r.read_uint(Columns::FIELD_CHAR_LENGTH);

  m_has_no_default = r.read_bool(Columns::FIELD_HAS_NO_DEFAULT);
  m_default_value_null = r.is_null(Columns::FIELD_DEFAULT_VALUE);
  m_default_value = r.read_str(Columns::FIELD_DEFAULT_VALUE, "");
  m_default_value_utf8_null = r.is_null(Columns::FIELD_DEFAULT_VALUE_UTF8);
  m_default_value_utf8 = r.read_str(Columns::FIELD_DEFAULT_VALUE_UTF8, "");
  m_comment = r.read_str(Columns::FIELD_COMMENT);

  m_is_virtual = r.read_bool(Columns::FIELD_IS_VIRTUAL);
  m_generation_expression =
      r.read_str(Columns::FIELD_GENERATION_EXPRESSION, "");
  m_generation_expression_utf8 =
      r.read_str(Columns::FIELD_GENERATION_EXPRESSION_UTF8, "");

  m_collation_id = r.read_ref_id(Columns::FIELD_COLLATION_ID);
  m_is_explicit_collation = r.read_bool(Columns::FIELD_IS_EXPLICIT_COLLATION);

  // Special cases dealing with NULL values for nullable fields

  set_options(r.read_str(Columns::FIELD_OPTIONS, ""));
  set_se_private_data(r.read_str(Columns::FIELD_SE_PRIVATE_DATA, ""));

  set_default_option(r.read_str(Columns::FIELD_DEFAULT_OPTION, ""));
  set_update_option(r.read_str(Columns::FIELD_UPDATE_OPTION, ""));

  m_column_key = (enum_column_key)r.read_int(Columns::FIELD_COLUMN_KEY);

  m_column_type_utf8 = r.read_str(Columns::FIELD_COLUMN_TYPE_UTF8);

  if (!r.is_null(Columns::FIELD_SRS_ID))
    m_srs_id = r.read_uint(Columns::FIELD_SRS_ID);
  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Column_impl::store_attributes(Raw_record *r) {
  //
  // Special cases dealing with NULL values for nullable fields:
  //   - Store NULL in default_option if it is not set.
  //   - Store NULL in update_option if it is not set.
  //   - Store NULL in options if there are no key=value pairs
  //   - Store NULL in se_private_data if there are no key=value pairs
  //

  // TODO-NOW - May be zerofill, unsigned, auto_increment, char_length,
  // numeric_precision, datetime_precision.
  // What value should we store in those columns in case when specific
  // attribute doesn't make sense for the type ? E.g. "unsigned" for
  // VARCHAR column

  return store_id(r, Columns::FIELD_ID) || store_name(r, Columns::FIELD_NAME) ||
         r->store(Columns::FIELD_TABLE_ID, m_table->id()) ||
         r->store(Columns::FIELD_ORDINAL_POSITION, m_ordinal_position) ||
         r->store(Columns::FIELD_TYPE, static_cast<int>(m_type)) ||
         r->store(Columns::FIELD_IS_NULLABLE, m_is_nullable) ||
         r->store(Columns::FIELD_IS_ZEROFILL, m_is_zerofill) ||
         r->store(Columns::FIELD_IS_UNSIGNED, m_is_unsigned) ||
         r->store(Columns::FIELD_CHAR_LENGTH,
                  static_cast<uint>(m_char_length)) ||
         r->store(Columns::FIELD_NUMERIC_PRECISION, m_numeric_precision) ||
         r->store(Columns::FIELD_NUMERIC_SCALE, m_numeric_scale,
                  m_numeric_scale_null) ||
         r->store(Columns::FIELD_DATETIME_PRECISION, m_datetime_precision,
                  m_datetime_precision_null) ||
         r->store_ref_id(Columns::FIELD_COLLATION_ID, m_collation_id) ||
         r->store(Columns::FIELD_IS_EXPLICIT_COLLATION,
                  m_is_explicit_collation) ||
         r->store(Columns::FIELD_HAS_NO_DEFAULT, m_has_no_default) ||
         r->store(Columns::FIELD_DEFAULT_VALUE, m_default_value,
                  m_default_value_null) ||
         r->store(Columns::FIELD_DEFAULT_VALUE_UTF8, m_default_value_utf8,
                  m_default_value_utf8_null) ||
         r->store(Columns::FIELD_DEFAULT_OPTION, m_default_option,
                  m_default_option.empty()) ||
         r->store(Columns::FIELD_UPDATE_OPTION, m_update_option,
                  m_update_option.empty()) ||
         r->store(Columns::FIELD_IS_AUTO_INCREMENT, m_is_auto_increment) ||
         r->store(Columns::FIELD_IS_VIRTUAL, m_is_virtual) ||
         r->store(Columns::FIELD_GENERATION_EXPRESSION, m_generation_expression,
                  m_generation_expression.empty()) ||
         r->store(Columns::FIELD_GENERATION_EXPRESSION_UTF8,
                  m_generation_expression_utf8,
                  m_generation_expression_utf8.empty()) ||
         r->store(Columns::FIELD_COMMENT, m_comment) ||
         r->store(Columns::FIELD_HIDDEN, static_cast<int>(m_hidden)) ||
         r->store(Columns::FIELD_OPTIONS, m_options) ||
         r->store(Columns::FIELD_SE_PRIVATE_DATA, m_se_private_data) ||
         r->store(Columns::FIELD_COLUMN_KEY, m_column_key) ||
         r->store(Columns::FIELD_COLUMN_TYPE_UTF8, m_column_type_utf8) ||
         r->store(Columns::FIELD_SRS_ID,
                  (m_srs_id.has_value() ? m_srs_id.value() : 0),
                  !m_srs_id.has_value());
}

///////////////////////////////////////////////////////////////////////////
static_assert(Columns::FIELD_SE_PRIVATE_DATA == 25,
              "Columns definition has changed, review (de)ser memfuns!");
void Column_impl::serialize(Sdi_wcontext *wctx, Sdi_writer *w) const {
  w->StartObject();
  Entity_object_impl::serialize(wctx, w);
  write_enum(w, m_type, STRING_WITH_LEN("type"));
  write(w, m_is_nullable, STRING_WITH_LEN("is_nullable"));
  write(w, m_is_zerofill, STRING_WITH_LEN("is_zerofill"));
  write(w, m_is_unsigned, STRING_WITH_LEN("is_unsigned"));
  write(w, m_is_auto_increment, STRING_WITH_LEN("is_auto_increment"));
  write(w, m_is_virtual, STRING_WITH_LEN("is_virtual"));
  write_enum(w, m_hidden, STRING_WITH_LEN("hidden"));
  write(w, m_ordinal_position, STRING_WITH_LEN("ordinal_position"));
  write(w, m_char_length, STRING_WITH_LEN("char_length"));
  write(w, m_numeric_precision, STRING_WITH_LEN("numeric_precision"));
  write(w, m_numeric_scale, STRING_WITH_LEN("numeric_scale"));
  write(w, m_numeric_scale_null, STRING_WITH_LEN("numeric_scale_null"));
  write(w, m_datetime_precision, STRING_WITH_LEN("datetime_precision"));
  write(w, m_datetime_precision_null,
        STRING_WITH_LEN("datetime_precision_null"));
  write(w, m_has_no_default, STRING_WITH_LEN("has_no_default"));
  write(w, m_default_value_null, STRING_WITH_LEN("default_value_null"));
  write(w, !m_srs_id.has_value(), STRING_WITH_LEN("srs_id_null"));
  write(w, (m_srs_id.has_value() ? m_srs_id.value() : 0),
        STRING_WITH_LEN("srs_id"));

  // Binary
  write_binary(wctx, w, m_default_value, STRING_WITH_LEN("default_value"));
  write(w, m_default_value_utf8_null,
        STRING_WITH_LEN("default_value_utf8_null"));
  write(w, m_default_value_utf8, STRING_WITH_LEN("default_value_utf8"));
  write(w, m_default_option, STRING_WITH_LEN("default_option"));
  write(w, m_update_option, STRING_WITH_LEN("update_option"));
  write(w, m_comment, STRING_WITH_LEN("comment"));
  write(w, m_generation_expression, STRING_WITH_LEN("generation_expression"));
  write(w, m_generation_expression_utf8,
        STRING_WITH_LEN("generation_expression_utf8"));
  write_properties(w, m_options, STRING_WITH_LEN("options"));
  write_properties(w, m_se_private_data, STRING_WITH_LEN("se_private_data"));
  write_enum(w, m_column_key, STRING_WITH_LEN("column_key"));
  write(w, m_column_type_utf8, STRING_WITH_LEN("column_type_utf8"));
  serialize_each(wctx, w, m_elements, STRING_WITH_LEN("elements"));
  write(w, m_collation_id, STRING_WITH_LEN("collation_id"));
  write(w, m_is_explicit_collation, STRING_WITH_LEN("is_explicit_collation"));
  w->EndObject();
}

///////////////////////////////////////////////////////////////////////////

bool Column_impl::deserialize(Sdi_rcontext *rctx, const RJ_Value &val) {
  Entity_object_impl::deserialize(rctx, val);

  read_enum(&m_type, val, "type");
  read(&m_is_nullable, val, "is_nullable");
  read(&m_is_zerofill, val, "is_zerofill");
  read(&m_is_unsigned, val, "is_unsigned");
  read(&m_is_auto_increment, val, "is_auto_increment");
  read(&m_is_virtual, val, "is_virtual");
  read(&m_ordinal_position, val, "ordinal_position");
  read(&m_char_length, val, "char_length");
  read(&m_numeric_precision, val, "numeric_precision");
  read(&m_numeric_scale, val, "numeric_scale");
  read(&m_numeric_scale_null, val, "numeric_scale_null");
  read(&m_datetime_precision, val, "datetime_precision");
  read(&m_datetime_precision_null, val, "datetime_precision_null");
  read(&m_has_no_default, val, "has_no_default");
  read(&m_default_value_null, val, "default_value_null");
  read_binary(rctx, &m_default_value, val, "default_value");
  read(&m_default_value_utf8_null, val, "default_value_utf8_null");
  read(&m_default_value_utf8, val, "default_value_utf8");
  read(&m_default_option, val, "default_option");
  read(&m_update_option, val, "update_option");
  read(&m_comment, val, "comment");
  read(&m_generation_expression, val, "generation_expression");
  read(&m_generation_expression_utf8, val, "generation_expression_utf8");
  read_properties(&m_options, val, "options");
  read_properties(&m_se_private_data, val, "se_private_data");
  read_enum(&m_column_key, val, "column_key");
  read(&m_column_type_utf8, val, "column_type_utf8");
  read_enum(&m_hidden, val, "hidden");

  bool srs_id_is_null;
  read(&srs_id_is_null, val, "srs_id_null");

  if (!srs_id_is_null) {
    gis::srid_t srs_id;
    read(&srs_id, val, "srs_id");
    m_srs_id = srs_id;
  }

  deserialize_each(
      rctx, [this]() { return add_element(); }, val, "elements");

  read(&m_collation_id, val, "collation_id");
  read(&m_is_explicit_collation, val, "is_explicit_collation");

  track_object(rctx, this);

  return false;
}

///////////////////////////////////////////////////////////////////////////

void Column_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "COLUMN OBJECT: { "
     << "m_id: {OID: " << id() << "}; "
     << "m_table_id: {OID: " << m_table->id() << "}; "
     << "m_name: " << name() << "; "
     << "m_ordinal_position: " << m_ordinal_position << "; "
     << "m_type: " << static_cast<int>(m_type) << "; "
     << "m_is_nullable: " << m_is_nullable << "; "
     << "m_is_zerofill: " << m_is_zerofill << "; "
     << "m_is_unsigned: " << m_is_unsigned << "; "
     << "m_char_length: " << m_char_length << "; "
     << "m_numeric_precision: " << m_numeric_precision << "; "
     << "m_numeric_scale: " << m_numeric_scale << "; "
     << "m_datetime_precision: " << m_datetime_precision << "; "
     << "m_datetime_precision_null: " << m_datetime_precision_null << "; "
     << "m_collation_id: {OID: " << m_collation_id << "}; "
     << "m_is_explicit_collation: " << m_is_explicit_collation << "; "
     << "m_has_no_default: " << m_has_no_default << "; "
     << "m_default_value: <excluded from output>"
     << "; "
     << "m_default_value_utf8: " << m_default_value_utf8 << "; "
     << "m_default_option: " << m_default_option << "; "
     << "m_update_option: " << m_update_option << "; "
     << "m_is_auto_increment: " << m_is_auto_increment << "; "
     << "m_comment: " << m_comment << "; "
     << "m_is_virtual " << m_is_virtual << "; "
     << "m_generation_expression: " << m_generation_expression << "; "
     << "m_generation_expression_utf8: " << m_generation_expression_utf8 << "; "
     << "m_hidden: " << static_cast<int>(m_hidden) << "; "
     << "m_options: " << m_options.raw_string() << "; "
     << "m_column_key: " << m_column_key << "; "
     << "m_column_type_utf8: " << m_column_type_utf8 << "; "
     << "m_srs_id_null: " << !m_srs_id.has_value() << "; ";

  if (m_srs_id.has_value()) ss << "m_srs_id: " << m_srs_id.value() << "; ";

  if (m_type == enum_column_types::ENUM || m_type == enum_column_types::SET) {
    ss << "m_elements: [ ";

    for (const Column_type_element *e : elements()) {
      String_type ob;
      e->debug_print(ob);
      ss << ob;
    }

    ss << " ]";
  }

  ss << " }";

  outb = ss.str();
}

///////////////////////////////////////////////////////////////////////////
// Elements.
///////////////////////////////////////////////////////////////////////////

Column_type_element *Column_impl::add_element() {
  DBUG_ASSERT(type() == enum_column_types::ENUM ||
              type() == enum_column_types::SET);

  Column_type_element_impl *e =
      new (std::nothrow) Column_type_element_impl(this);
  m_elements.push_back(e);
  return e;
}

///////////////////////////////////////////////////////////////////////////

Column_impl::Column_impl(const Column_impl &src, Abstract_table_impl *parent)
    : Weak_object(src),
      Entity_object_impl(src),
      m_type(src.m_type),
      m_is_nullable(src.m_is_nullable),
      m_is_zerofill(src.m_is_zerofill),
      m_is_unsigned(src.m_is_unsigned),
      m_is_auto_increment(src.m_is_auto_increment),
      m_is_virtual(src.m_is_virtual),
      m_hidden(src.m_hidden),
      m_ordinal_position(src.m_ordinal_position),
      m_char_length(src.m_char_length),
      m_numeric_precision(src.m_numeric_precision),
      m_numeric_scale(src.m_numeric_scale),
      m_numeric_scale_null(src.m_numeric_scale_null),
      m_datetime_precision(src.m_datetime_precision),
      m_datetime_precision_null(src.m_datetime_precision_null),
      m_has_no_default(src.m_has_no_default),
      m_default_value_null(src.m_default_value_null),
      m_default_value(src.m_default_value),
      m_default_value_utf8_null(src.m_default_value_utf8_null),
      m_default_value_utf8(src.m_default_value_utf8),
      m_default_option(src.m_default_option),
      m_update_option(src.m_update_option),
      m_comment(src.m_comment),
      m_generation_expression(src.m_generation_expression),
      m_generation_expression_utf8(src.m_generation_expression_utf8),
      m_options(src.m_options),
      m_se_private_data(src.m_se_private_data),
      m_table(parent),
      m_elements(),
      m_column_type_utf8(src.m_column_type_utf8),
      m_collation_id(src.m_collation_id),
      m_is_explicit_collation(src.m_is_explicit_collation),
      m_column_key(src.m_column_key),
      m_srs_id(src.m_srs_id) {
  m_elements.deep_copy(src.m_elements, this);
}

///////////////////////////////////////////////////////////////////////////

const Object_table &Column_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Column_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Columns>();

  otx->register_tables<Column_type_element>();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
