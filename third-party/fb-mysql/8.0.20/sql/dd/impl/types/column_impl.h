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

#ifndef DD__COLUMN_IMPL_INCLUDED
#define DD__COLUMN_IMPL_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <memory>  // std::unique_ptr
#include <new>

#include "my_dbug.h"
#include "nullable.h"
#include "sql/dd/impl/properties_impl.h"           // Properties_impl
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/properties.h"
#include "sql/dd/sdi_fwd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/column.h"               // dd::Column
#include "sql/dd/types/column_type_element.h"  // IWYU pragma: keep
#include "sql/gis/srid.h"                      // gis::srid_t

using Mysql::Nullable;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Abstract_table;
class Abstract_table_impl;
class Object_table;
class Open_dictionary_tables_ctx;
class Properties;
class Raw_record;
class Sdi_rcontext;
class Sdi_wcontext;
class Weak_object;

///////////////////////////////////////////////////////////////////////////

class Column_impl : public Entity_object_impl, public Column {
 public:
  Column_impl();

  Column_impl(Abstract_table_impl *table);

  Column_impl(const Column_impl &src, Abstract_table_impl *parent);

  virtual ~Column_impl();

 public:
  virtual const Object_table &object_table() const;

  static void register_tables(Open_dictionary_tables_ctx *otx);

  virtual bool validate() const;

  virtual bool restore_children(Open_dictionary_tables_ctx *otx);

  virtual bool store_children(Open_dictionary_tables_ctx *otx);

  virtual bool drop_children(Open_dictionary_tables_ctx *otx) const;

  virtual bool restore_attributes(const Raw_record &r);

  virtual bool store_attributes(Raw_record *r);

  void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const;

  bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val);

  void debug_print(String_type &outb) const;

  void set_ordinal_position(uint ordinal_position) {
    m_ordinal_position = ordinal_position;
  }

 public:
  /////////////////////////////////////////////////////////////////////////
  // table.
  /////////////////////////////////////////////////////////////////////////

  virtual const Abstract_table &table() const;

  virtual Abstract_table &table();

  /////////////////////////////////////////////////////////////////////////
  // type.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_column_types type() const { return m_type; }

  virtual void set_type(enum_column_types type) { m_type = type; }

  /////////////////////////////////////////////////////////////////////////
  // collation.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id collation_id() const { return m_collation_id; }

  virtual void set_collation_id(Object_id collation_id) {
    m_collation_id = collation_id;
  }

  virtual void set_is_explicit_collation(bool is_explicit_collation) {
    m_is_explicit_collation = is_explicit_collation;
  }

  virtual bool is_explicit_collation() const { return m_is_explicit_collation; }

  /////////////////////////////////////////////////////////////////////////
  // nullable.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_nullable() const { return m_is_nullable; }

  virtual void set_nullable(bool nullable) { m_is_nullable = nullable; }

  /////////////////////////////////////////////////////////////////////////
  // is_zerofill.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_zerofill() const { return m_is_zerofill; }

  virtual void set_zerofill(bool zerofill) { m_is_zerofill = zerofill; }

  /////////////////////////////////////////////////////////////////////////
  // is_unsigned.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_unsigned() const { return m_is_unsigned; }

  virtual void set_unsigned(bool unsigned_flag) {
    m_is_unsigned = unsigned_flag;
  }

  /////////////////////////////////////////////////////////////////////////
  // auto increment.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_auto_increment() const { return m_is_auto_increment; }

  virtual void set_auto_increment(bool auto_increment) {
    m_is_auto_increment = auto_increment;
  }

  /////////////////////////////////////////////////////////////////////////
  // ordinal_position.
  /////////////////////////////////////////////////////////////////////////

  virtual uint ordinal_position() const { return m_ordinal_position; }

  /////////////////////////////////////////////////////////////////////////
  // char_length.
  /////////////////////////////////////////////////////////////////////////

  virtual size_t char_length() const { return m_char_length; }

  virtual void set_char_length(size_t char_length) {
    m_char_length = char_length;
  }

  /////////////////////////////////////////////////////////////////////////
  // numeric_precision.
  /////////////////////////////////////////////////////////////////////////

  virtual uint numeric_precision() const { return m_numeric_precision; }

  virtual void set_numeric_precision(uint numeric_precision) {
    m_numeric_precision = numeric_precision;
  }

  /////////////////////////////////////////////////////////////////////////
  // numeric_scale.
  /////////////////////////////////////////////////////////////////////////

  virtual uint numeric_scale() const { return m_numeric_scale; }

  virtual void set_numeric_scale(uint numeric_scale) {
    m_numeric_scale_null = false;
    m_numeric_scale = numeric_scale;
  }

  virtual void set_numeric_scale_null(bool is_null) {
    m_numeric_scale_null = is_null;
  }

  virtual bool is_numeric_scale_null() const { return m_numeric_scale_null; }

  /////////////////////////////////////////////////////////////////////////
  // datetime_precision.
  /////////////////////////////////////////////////////////////////////////

  virtual uint datetime_precision() const { return m_datetime_precision; }

  virtual void set_datetime_precision(uint datetime_precision) {
    m_datetime_precision_null = false;
    m_datetime_precision = datetime_precision;
  }

  virtual void set_datetime_precision_null(bool is_null) {
    m_datetime_precision_null = is_null;
  }

  virtual bool is_datetime_precision_null() const {
    return m_datetime_precision_null;
  }

  /////////////////////////////////////////////////////////////////////////
  // has_no_default.
  /////////////////////////////////////////////////////////////////////////

  virtual bool has_no_default() const { return m_has_no_default; }

  virtual void set_has_no_default(bool has_no_default) {
    m_has_no_default = has_no_default;
  }

  /////////////////////////////////////////////////////////////////////////
  // default_value (binary).
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &default_value() const { return m_default_value; }

  virtual void set_default_value(const String_type &default_value) {
    m_default_value_null = false;
    m_default_value = default_value;
  }

  virtual void set_default_value_null(bool is_null) {
    m_default_value_null = is_null;
  }

  virtual bool is_default_value_null() const { return m_default_value_null; }

  /////////////////////////////////////////////////////////////////////////
  // default_value_utf8
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &default_value_utf8() const {
    return m_default_value_utf8;
  }

  virtual void set_default_value_utf8(const String_type &default_value_utf8) {
    m_default_value_utf8_null = false;
    m_default_value_utf8 = default_value_utf8;
  }

  virtual void set_default_value_utf8_null(bool is_null) {
    m_default_value_utf8_null = is_null;
  }

  /* purecov: begin deadcode */
  virtual bool is_default_value_utf8_null() const {
    return m_default_value_utf8_null;
  }
  /* purecov: end */

  /////////////////////////////////////////////////////////////////////////
  // is virtual ?
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_virtual() const { return m_is_virtual; }

  virtual void set_virtual(bool is_virtual) { m_is_virtual = is_virtual; }

  /////////////////////////////////////////////////////////////////////////
  // generation_expression (binary).
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &generation_expression() const {
    return m_generation_expression;
  }

  virtual void set_generation_expression(
      const String_type &generation_expression) {
    m_generation_expression = generation_expression;
  }

  virtual bool is_generation_expression_null() const {
    return m_generation_expression.empty();
  }

  /////////////////////////////////////////////////////////////////////////
  // generation_expression_utf8
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &generation_expression_utf8() const {
    return m_generation_expression_utf8;
  }

  virtual void set_generation_expression_utf8(
      const String_type &generation_expression_utf8) {
    m_generation_expression_utf8 = generation_expression_utf8;
  }

  virtual bool is_generation_expression_utf8_null() const {
    return m_generation_expression_utf8.empty();
  }

  /////////////////////////////////////////////////////////////////////////
  // default_option.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &default_option() const { return m_default_option; }

  virtual void set_default_option(const String_type &default_option) {
    m_default_option = default_option;
  }

  /////////////////////////////////////////////////////////////////////////
  // update_option.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &update_option() const { return m_update_option; }

  virtual void set_update_option(const String_type &update_option) {
    m_update_option = update_option;
  }

  /////////////////////////////////////////////////////////////////////////
  // Comment.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const { return m_comment; }

  virtual void set_comment(const String_type &comment) { m_comment = comment; }

  /////////////////////////////////////////////////////////////////////////
  // Hidden.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_hidden_type hidden() const { return m_hidden; }

  virtual void set_hidden(enum_hidden_type hidden) { m_hidden = hidden; }

  /////////////////////////////////////////////////////////////////////////
  // Options.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &options() const { return m_options; }

  virtual Properties &options() { return m_options; }

  virtual bool set_options(const String_type &options_raw) {
    return m_options.insert_values(options_raw);
  }

  /////////////////////////////////////////////////////////////////////////
  // se_private_data.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &se_private_data() const {
    return m_se_private_data;
  }

  virtual Properties &se_private_data() { return m_se_private_data; }

  virtual bool set_se_private_data(const Properties &se_private_data) {
    return m_se_private_data.insert_values(se_private_data);
  }

  virtual bool set_se_private_data(const String_type &se_private_data_raw) {
    return m_se_private_data.insert_values(se_private_data_raw);
  }

  /////////////////////////////////////////////////////////////////////////
  // Column key type.
  /////////////////////////////////////////////////////////////////////////

  virtual void set_column_key(enum_column_key column_key) {
    m_column_key = column_key;
  }

  virtual enum_column_key column_key() const { return m_column_key; }

  /////////////////////////////////////////////////////////////////////////
  // Spatial reference system ID
  /////////////////////////////////////////////////////////////////////////
  virtual void set_srs_id(Nullable<gis::srid_t> srs_id) { m_srs_id = srs_id; }

  virtual Nullable<gis::srid_t> srs_id() const { return m_srs_id; }

  /////////////////////////////////////////////////////////////////////////
  // Elements.
  /////////////////////////////////////////////////////////////////////////

  virtual Column_type_element *add_element();

  virtual const Column_type_element_collection &elements() const {
    DBUG_ASSERT(type() == enum_column_types::ENUM ||
                type() == enum_column_types::SET);
    return m_elements;
  }

  /////////////////////////////////////////////////////////////////////////
  // Column display type
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &column_type_utf8() const {
    return m_column_type_utf8;
  }

  virtual void set_column_type_utf8(const String_type &column_type_utf8) {
    m_column_type_utf8 = column_type_utf8;
  }

  virtual size_t elements_count() const { return m_elements.size(); }

  // Fix "inherits ... via dominance" warnings
  virtual Entity_object_impl *impl() { return Entity_object_impl::impl(); }
  virtual const Entity_object_impl *impl() const {
    return Entity_object_impl::impl();
  }
  virtual Object_id id() const { return Entity_object_impl::id(); }
  virtual bool is_persistent() const {
    return Entity_object_impl::is_persistent();
  }
  virtual const String_type &name() const { return Entity_object_impl::name(); }
  virtual void set_name(const String_type &name) {
    Entity_object_impl::set_name(name);
  }

  virtual bool is_array() const {
    // Is this a typed array field?
    if (options().exists("is_array")) {
      bool is_array;
      if (!options().get("is_array", &is_array)) return is_array;
    }

    return false;
  }

 public:
  static Column_impl *restore_item(Abstract_table_impl *table) {
    return new (std::nothrow) Column_impl(table);
  }

  static Column_impl *clone(const Column_impl &other,
                            Abstract_table_impl *table) {
    return new (std::nothrow) Column_impl(other, table);
  }

  Column_impl *clone(Abstract_table_impl *parent) const {
    return new Column_impl(*this, parent);
  }

 private:
  // Fields.

  enum_column_types m_type;

  bool m_is_nullable;
  bool m_is_zerofill;
  bool m_is_unsigned;
  bool m_is_auto_increment;
  bool m_is_virtual;
  enum_hidden_type m_hidden;

  uint m_ordinal_position;
  size_t m_char_length;
  uint m_numeric_precision;
  uint m_numeric_scale;
  bool m_numeric_scale_null;
  uint m_datetime_precision;
  uint m_datetime_precision_null;

  bool m_has_no_default;

  bool m_default_value_null;
  String_type m_default_value;
  bool m_default_value_utf8_null;
  String_type m_default_value_utf8;

  String_type m_default_option;
  String_type m_update_option;
  String_type m_comment;

  String_type m_generation_expression;
  String_type m_generation_expression_utf8;

  Properties_impl m_options;
  Properties_impl m_se_private_data;

  // References to tightly-coupled objects.

  Abstract_table_impl *m_table;

  Column_type_element_collection m_elements;

  String_type m_column_type_utf8;

  // References to loosely-coupled objects.

  Object_id m_collation_id;
  bool m_is_explicit_collation;

  // TODO-WIKI21 should the columns.name be defined utf8_general_cs ?
  // instead of utf8_general_ci.

  enum_column_key m_column_key;

  Nullable<gis::srid_t> m_srs_id;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__COLUMN_IMPL_INCLUDED
