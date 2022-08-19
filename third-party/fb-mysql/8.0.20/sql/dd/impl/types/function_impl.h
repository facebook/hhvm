/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__FUNCTION_IMPL_INCLUDED
#define DD__FUNCTION_IMPL_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <new>

#include "my_inttypes.h"
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/entity_object_impl.h"
#include "sql/dd/impl/types/routine_impl.h"  // dd::Routine_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/column.h"
#include "sql/dd/types/function.h"  // dd::Function
#include "sql/dd/types/routine.h"
#include "sql/dd/types/view.h"

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Open_dictionary_tables_ctx;
class Parameter;
class Weak_object;
class Object_table;

class Function_impl : public Routine_impl, public Function {
 public:
  Function_impl();

  virtual ~Function_impl() {}

  virtual bool update_routine_name_key(Name_key *key, Object_id schema_id,
                                       const String_type &name) const;

 public:
  virtual bool validate() const;

  virtual bool restore_attributes(const Raw_record &r);

  virtual bool store_attributes(Raw_record *r);

  virtual void debug_print(String_type &outb) const;

 public:
  /////////////////////////////////////////////////////////////////////////
  // result data type.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_column_types result_data_type() const {
    return m_result_data_type;
  }

  virtual void set_result_data_type(enum_column_types result_data_type) {
    m_result_data_type = result_data_type;
  }

  virtual bool is_result_data_type_null() const {
    return m_result_data_type_null;
  }

  virtual void set_result_data_type_null(bool is_null) {
    m_result_data_type_null = is_null;
  }

  /////////////////////////////////////////////////////////////////////////
  // Result display type
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &result_data_type_utf8() const {
    return m_result_data_type_utf8;
  }

  virtual void set_result_data_type_utf8(
      const String_type &result_data_type_utf8) {
    m_result_data_type_utf8 = result_data_type_utf8;
  }

  /////////////////////////////////////////////////////////////////////////
  // result_is_zerofill.
  /////////////////////////////////////////////////////////////////////////

  virtual bool result_is_zerofill() const { return m_result_is_zerofill; }

  virtual void set_result_zerofill(bool zerofill) {
    m_result_is_zerofill = zerofill;
  }

  /////////////////////////////////////////////////////////////////////////
  // result_is_unsigned.
  /////////////////////////////////////////////////////////////////////////

  virtual bool result_is_unsigned() const { return m_result_is_unsigned; }

  virtual void set_result_unsigned(bool unsigned_flag) {
    m_result_is_unsigned = unsigned_flag;
  }

  /////////////////////////////////////////////////////////////////////////
  // result_char_length.
  /////////////////////////////////////////////////////////////////////////

  virtual size_t result_char_length() const { return m_result_char_length; }

  virtual void set_result_char_length(size_t char_length) {
    m_result_char_length = char_length;
  }

  /////////////////////////////////////////////////////////////////////////
  // result_numeric_precision.
  /////////////////////////////////////////////////////////////////////////

  virtual uint result_numeric_precision() const {
    return m_result_numeric_precision;
  }

  virtual void set_result_numeric_precision(uint result_numeric_precision) {
    m_result_numeric_precision_null = false;
    m_result_numeric_precision = result_numeric_precision;
  }

  virtual void set_result_numeric_precision_null(bool is_null) {
    m_result_numeric_precision_null = is_null;
  }

  virtual bool is_result_numeric_precision_null() const {
    return m_result_numeric_precision_null;
  }

  /////////////////////////////////////////////////////////////////////////
  // result_numeric_scale.
  /////////////////////////////////////////////////////////////////////////

  virtual uint result_numeric_scale() const { return m_result_numeric_scale; }

  virtual void set_result_numeric_scale(uint result_numeric_scale) {
    m_result_numeric_scale_null = false;
    m_result_numeric_scale = result_numeric_scale;
  }

  virtual void set_result_numeric_scale_null(bool is_null) {
    m_result_numeric_scale_null = is_null;
  }

  virtual bool is_result_numeric_scale_null() const {
    return m_result_numeric_scale_null;
  }

  /////////////////////////////////////////////////////////////////////////
  // result_datetime_precision.
  /////////////////////////////////////////////////////////////////////////

  virtual uint result_datetime_precision() const {
    return m_result_datetime_precision;
  }

  virtual void set_result_datetime_precision(uint result_datetime_precision) {
    m_result_datetime_precision_null = false;
    m_result_datetime_precision = result_datetime_precision;
  }

  virtual void set_result_datetime_precision_null(bool is_null) {
    m_result_datetime_precision_null = is_null;
  }

  virtual bool is_result_datetime_precision_null() const {
    return m_result_datetime_precision_null;
  }

  /////////////////////////////////////////////////////////////////////////
  // result_collation.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id result_collation_id() const {
    return m_result_collation_id;
  }

  virtual void set_result_collation_id(Object_id result_collation_id) {
    m_result_collation_id = result_collation_id;
  }

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
  virtual const Object_table &object_table() const {
    return Routine_impl::object_table();
  }
  virtual Object_id schema_id() const { return Routine_impl::schema_id(); }
  virtual void set_schema_id(Object_id schema_id) {
    Routine_impl::set_schema_id(schema_id);
  }
  virtual enum_routine_type type() const { return Routine_impl::type(); }
  virtual const String_type &definition() const {
    return Routine_impl::definition();
  }
  virtual void set_definition(const String_type &definition) {
    Routine_impl::set_definition(definition);
  }
  virtual const String_type &definition_utf8() const {
    return Routine_impl::definition_utf8();
  }
  virtual void set_definition_utf8(const String_type &definition_utf8) {
    Routine_impl::set_definition_utf8(definition_utf8);
  }
  virtual const String_type &parameter_str() const {
    return Routine_impl::parameter_str();
  }
  virtual void set_parameter_str(const String_type &parameter_str) {
    Routine_impl::set_parameter_str(parameter_str);
  }
  virtual bool is_deterministic() const {
    return Routine_impl::is_deterministic();
  }
  virtual void set_deterministic(bool deterministic) {
    Routine_impl::set_deterministic(deterministic);
  }
  virtual enum_sql_data_access sql_data_access() const {
    return Routine_impl::sql_data_access();
  }
  virtual void set_sql_data_access(enum_sql_data_access sda) {
    Routine_impl::set_sql_data_access(sda);
  }
  virtual View::enum_security_type security_type() const {
    return Routine_impl::security_type();
  }
  virtual void set_security_type(View::enum_security_type security_type) {
    Routine_impl::set_security_type(security_type);
  }
  virtual ulonglong sql_mode() const { return Routine_impl::sql_mode(); }
  virtual void set_sql_mode(ulonglong sm) { Routine_impl::set_sql_mode(sm); }
  virtual const String_type &definer_user() const {
    return Routine_impl::definer_user();
  }
  virtual const String_type &definer_host() const {
    return Routine_impl::definer_host();
  }
  virtual void set_definer(const String_type &username,
                           const String_type &hostname) {
    Routine_impl::set_definer(username, hostname);
  }
  virtual Object_id client_collation_id() const {
    return Routine_impl::client_collation_id();
  }
  virtual void set_client_collation_id(Object_id client_collation_id) {
    Routine_impl::set_client_collation_id(client_collation_id);
  }
  virtual Object_id connection_collation_id() const {
    return Routine_impl::connection_collation_id();
  }
  virtual void set_connection_collation_id(Object_id connection_collation_id) {
    Routine_impl::set_connection_collation_id(connection_collation_id);
  }
  virtual Object_id schema_collation_id() const {
    return Routine_impl::schema_collation_id();
  }
  virtual void set_schema_collation_id(Object_id schema_collation_id) {
    Routine_impl::set_schema_collation_id(schema_collation_id);
  }
  virtual ulonglong created(bool convert_time) const {
    return Routine_impl::created(convert_time);
  }
  virtual void set_created(ulonglong created) {
    Routine_impl::set_created(created);
  }
  virtual ulonglong last_altered(bool convert_time) const {
    return Routine_impl::last_altered(convert_time);
  }
  virtual void set_last_altered(ulonglong last_altered) {
    Routine_impl::set_last_altered(last_altered);
  }
  virtual const String_type &comment() const { return Routine_impl::comment(); }
  virtual void set_comment(const String_type &comment) {
    Routine_impl::set_comment(comment);
  }
  virtual Parameter *add_parameter() { return Routine_impl::add_parameter(); }
  virtual const Parameter_collection &parameters() const {
    return Routine_impl::parameters();
  }
  virtual bool update_name_key(Name_key *key) const {
    return Function::update_name_key(key);
  }

 private:
  enum_column_types m_result_data_type;
  String_type m_result_data_type_utf8;

  bool m_result_data_type_null;
  bool m_result_is_zerofill;
  bool m_result_is_unsigned;

  bool m_result_numeric_precision_null;
  bool m_result_numeric_scale_null;
  bool m_result_datetime_precision_null;

  uint m_result_numeric_precision;
  uint m_result_numeric_scale;
  uint m_result_datetime_precision;

  size_t m_result_char_length;

  Object_id m_result_collation_id;

  Function_impl(const Function_impl &src);
  Function_impl *clone() const { return new Function_impl(*this); }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__FUNCTION_IMPL_INCLUDED
