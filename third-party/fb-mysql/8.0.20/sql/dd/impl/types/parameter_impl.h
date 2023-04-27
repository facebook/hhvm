/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__PARAMETER_IMPL_INCLUDED
#define DD__PARAMETER_IMPL_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <memory>  // std::unique_ptr
#include <new>

#include "my_dbug.h"
#include "sql/dd/impl/properties_impl.h"  // dd::Properties_imp
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/column.h"
#include "sql/dd/types/parameter.h"               // dd::Parameter
#include "sql/dd/types/parameter_type_element.h"  // IWYU pragma: keep

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Object_table;
class Open_dictionary_tables_ctx;
class Routine;
class Routine_impl;
class Weak_object;

///////////////////////////////////////////////////////////////////////////

class Parameter_impl : public Entity_object_impl, public Parameter {
 public:
  Parameter_impl();

  Parameter_impl(Routine_impl *routine);

  Parameter_impl(const Parameter_impl &src, Routine_impl *parent);

  virtual ~Parameter_impl() {}

 public:
  virtual const Object_table &object_table() const;

  virtual bool validate() const;

  virtual bool restore_children(Open_dictionary_tables_ctx *otx);

  virtual bool store_children(Open_dictionary_tables_ctx *otx);

  virtual bool drop_children(Open_dictionary_tables_ctx *otx) const;

  virtual bool store_attributes(Raw_record *r);

  virtual bool restore_attributes(const Raw_record &r);

  virtual void debug_print(String_type &outb) const;

  void set_ordinal_position(uint ordinal_position) {
    m_ordinal_position = ordinal_position;
  }

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  /////////////////////////////////////////////////////////////////////////
  // Name is nullable in case of function return type.
  /////////////////////////////////////////////////////////////////////////

  virtual void set_name_null(bool is_null) { m_is_name_null = is_null; }

  virtual bool is_name_null() const { return m_is_name_null; }

  /////////////////////////////////////////////////////////////////////////
  // ordinal_position.
  /////////////////////////////////////////////////////////////////////////

  virtual uint ordinal_position() const { return m_ordinal_position; }

  /////////////////////////////////////////////////////////////////////////
  // parameter_mode.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_parameter_mode mode() const { return m_parameter_mode; }

  virtual void set_mode(enum_parameter_mode mode) { m_parameter_mode = mode; }

  virtual void set_parameter_mode_null(bool is_null) {
    m_parameter_mode_null = is_null;
  }

  virtual bool is_parameter_mode_null() const { return m_parameter_mode_null; }

  /////////////////////////////////////////////////////////////////////////
  // parameter_mode.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_column_types data_type() const { return m_data_type; }

  virtual void set_data_type(enum_column_types type) { m_data_type = type; }

  /////////////////////////////////////////////////////////////////////////
  // display type
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &data_type_utf8() const { return m_data_type_utf8; }

  virtual void set_data_type_utf8(const String_type &data_type_utf8) {
    m_data_type_utf8 = data_type_utf8;
  }

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
    m_numeric_precision_null = false;
    m_numeric_precision = numeric_precision;
  }

  virtual void set_numeric_precision_null(bool is_null) {
    m_numeric_precision_null = is_null;
  }

  virtual bool is_numeric_precision_null() const {
    return m_numeric_precision_null;
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
  // collation.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id collation_id() const { return m_collation_id; }

  virtual void set_collation_id(Object_id collation_id) {
    m_collation_id = collation_id;
  }

  /////////////////////////////////////////////////////////////////////////
  // Options.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &options() const { return m_options; }

  virtual Properties &options() { return m_options; }

  virtual bool set_options(const String_type &options_raw) {
    return m_options.insert_values(options_raw);
  }

  /////////////////////////////////////////////////////////////////////////
  // routine.
  /////////////////////////////////////////////////////////////////////////

  virtual const Routine &routine() const;

  virtual Routine &routine();

  /////////////////////////////////////////////////////////////////////////
  // Elements.
  /////////////////////////////////////////////////////////////////////////

  virtual Parameter_type_element *add_element();

  virtual const Parameter_type_element_collection &elements() const {
    DBUG_ASSERT(data_type() == enum_column_types::ENUM ||
                data_type() == enum_column_types::SET);
    return m_elements;
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

 public:
  static Parameter_impl *restore_item(Routine_impl *routine) {
    return new (std::nothrow) Parameter_impl(routine);
  }

  static Parameter_impl *clone(const Parameter_impl &other,
                               Routine_impl *routine) {
    return new (std::nothrow) Parameter_impl(other, routine);
  }

 private:
  // Fields
  bool m_is_name_null;

  enum_parameter_mode m_parameter_mode;
  bool m_parameter_mode_null;
  enum_column_types m_data_type;
  String_type m_data_type_utf8;

  bool m_is_zerofill;
  bool m_is_unsigned;

  uint m_ordinal_position;
  size_t m_char_length;
  uint m_numeric_precision;
  bool m_numeric_precision_null;
  uint m_numeric_scale;
  bool m_numeric_scale_null;
  uint m_datetime_precision;
  bool m_datetime_precision_null;

  Parameter_type_element_collection m_elements;

  Properties_impl m_options;

  // References to other tightly coupled objects
  Routine_impl *m_routine;

  // References to loosely-coupled objects.

  Object_id m_collation_id;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__PARAMETER_IMPL_INCLUDED
