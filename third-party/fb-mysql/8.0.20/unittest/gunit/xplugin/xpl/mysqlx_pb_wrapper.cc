/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "unittest/gunit/xplugin/xpl/mysqlx_pb_wrapper.h"

#include <utility>

#include "plugin/x/src/helper/to_string.h"

namespace xpl {
namespace test {

Identifier::Identifier(const std::string &name,
                       const std::string &schema_name) {
  if (name.empty() == false) m_base.set_name(name);

  if (schema_name.empty() == false) m_base.set_schema_name(schema_name);
}

Column_identifier::Column_identifier(const std::string &name,
                                     const std::string &table_name,
                                     const std::string &schema_name) {
  if (name.empty() == false) m_base.set_name(name);

  if (table_name.empty() == false) m_base.set_table_name(table_name);

  if (schema_name.empty() == false) m_base.set_schema_name(schema_name);
}

Column_identifier::Column_identifier(const Document_path &path,
                                     const std::string &name,
                                     const std::string &table_name,
                                     const std::string &schema_name) {
  if (!name.empty()) m_base.set_name(name);

  if (!table_name.empty()) m_base.set_table_name(table_name);

  if (!schema_name.empty()) m_base.set_schema_name(schema_name);

  m_base.mutable_document_path()->CopyFrom(path);
}

Scalar::Scalar(const int value) {
  m_base.set_type(Mysqlx::Datatypes::Scalar_Type_V_SINT);
  m_base.set_v_signed_int(value);
}

Scalar::Scalar(const unsigned int value) {
  m_base.set_type(Mysqlx::Datatypes::Scalar_Type_V_UINT);
  m_base.set_v_unsigned_int(value);
}

Scalar::Scalar(const bool value) {
  m_base.set_type(Mysqlx::Datatypes::Scalar_Type_V_BOOL);
  m_base.set_v_bool(value);
}

Scalar::Scalar(const float value) {
  m_base.set_type(Mysqlx::Datatypes::Scalar_Type_V_FLOAT);
  m_base.set_v_float(value);
}

Scalar::Scalar(const double value) {
  m_base.set_type(Mysqlx::Datatypes::Scalar_Type_V_DOUBLE);
  m_base.set_v_double(value);
}

Scalar::Scalar(const Scalar::Octets &value) {
  m_base.set_type(Mysqlx::Datatypes::Scalar_Type_V_OCTETS);
  m_base.mutable_v_octets()->CopyFrom(value);
}

Scalar::Scalar(const Scalar::String &value) {
  m_base.set_type(Mysqlx::Datatypes::Scalar_Type_V_STRING);
  m_base.mutable_v_string()->CopyFrom(value);
}

Scalar::Scalar(const char *value) {
  m_base.set_type(Mysqlx::Datatypes::Scalar_Type_V_STRING);
  *m_base.mutable_v_string()->mutable_value() = value;
}

Scalar::Scalar(Scalar::Null) {
  m_base.set_type(Mysqlx::Datatypes::Scalar_Type_V_NULL);
}

Scalar::String::String(const std::string &value) { m_base.set_value(value); }

Scalar::Octets::Octets(const std::string &value, const Content_type type) {
  m_base.set_value(value);
  m_base.set_content_type(static_cast<uint32_t>(type));
}

Any::Any(const Scalar &scalar) {
  m_base.set_type(Mysqlx::Datatypes::Any_Type_SCALAR);
  m_base.mutable_scalar()->CopyFrom(scalar);
}

Any::Any(const Object &obj) {
  m_base.set_type(Mysqlx::Datatypes::Any_Type_OBJECT);
  m_base.mutable_obj()->CopyFrom(obj);
}

Any::Any(const Array &array) {
  m_base.set_type(Mysqlx::Datatypes::Any_Type_ARRAY);
  m_base.mutable_array()->CopyFrom(array);
}

Expr::Expr(const Scalar &value) {
  m_base.set_type(Mysqlx::Expr::Expr_Type_LITERAL);
  m_base.mutable_literal()->CopyFrom(value);
}

Expr::Expr(const Operator &oper) {
  m_base.set_type(Mysqlx::Expr::Expr_Type_OPERATOR);
  m_base.mutable_operator_()->CopyFrom(oper);
}

Expr::Expr(const Function_call &func) {
  m_base.set_type(Mysqlx::Expr::Expr_Type_FUNC_CALL);
  m_base.mutable_function_call()->CopyFrom(func);
}

Expr::Expr(const Column_identifier &id) {
  m_base.set_type(Mysqlx::Expr::Expr_Type_IDENT);
  m_base.mutable_identifier()->CopyFrom(id);
}

Expr::Expr(const Object &obj) {
  m_base.set_type(Mysqlx::Expr::Expr_Type_OBJECT);
  m_base.mutable_object()->CopyFrom(obj);
}

Expr::Expr(const Array &arr) {
  m_base.set_type(Mysqlx::Expr::Expr_Type_ARRAY);
  m_base.mutable_array()->CopyFrom(arr);
}

Expr::Expr(const Placeholder &ph) {
  m_base.set_type(Mysqlx::Expr::Expr_Type_PLACEHOLDER);
  m_base.set_position(ph.value);
}

Expr::Expr(const Variable &var) {
  m_base.set_type(Mysqlx::Expr::Expr_Type_VARIABLE);
  m_base.set_variable(var.value);
}

Any::Object::Object(const std::initializer_list<Any::Object::Fld> &list) {
  for (const Fld &f : list) {
    Mysqlx::Datatypes::Object_ObjectField *item = m_base.add_fld();
    item->set_key(f.key);
    item->mutable_value()->CopyFrom(f.value);
  }
}

Any::Object::Object(const std::string &key, Any *value) {
  Mysqlx::Datatypes::Object_ObjectField *item = m_base.add_fld();
  item->set_key(key);
  if (value) item->mutable_value()->CopyFrom(*value);
}

Object::Object(const std::string &key, Expr *value) {
  Mysqlx::Expr::Object_ObjectField *item = m_base.add_fld();
  item->set_key(key);
  if (value) item->mutable_value()->CopyFrom(*value);
}

Object::Object(const std::initializer_list<Fld> &list) {
  for (const Fld &f : list) {
    Mysqlx::Expr::Object_ObjectField *item = m_base.add_fld();
    item->set_key(f.key);
    item->mutable_value()->CopyFrom(f.value);
  }
}

Column::Column(const std::string &name, const std::string &alias) {
  if (!name.empty()) m_base.set_name(name);
  if (!alias.empty()) m_base.set_alias(alias);
}

Column::Column(const Document_path &path, const std::string &name,
               const std::string &alias) {
  m_base.mutable_document_path()->CopyFrom(path);
  if (!name.empty()) m_base.set_name(name);
  if (!alias.empty()) m_base.set_alias(alias);
}

Collection::Collection(const std::string &name, const std::string &schema) {
  if (!name.empty()) m_base.set_name(name);
  if (!schema.empty()) m_base.set_schema(schema);
}

Projection::Projection(const Expr &source, const std::string &alias) {
  m_base.mutable_source()->CopyFrom(source);
  if (!alias.empty()) m_base.set_alias(alias);
}

Order::Order(const Expr &expr, const Direction dir) {
  m_base.mutable_expr()->CopyFrom(expr);
  m_base.set_direction(dir);
}

Limit::Limit(const uint64_t row_count, const uint64_t offset) {
  if (row_count > 0) m_base.set_row_count(row_count);
  if (offset > 0) m_base.set_offset(offset);
}

Limit_expr::Limit_expr(const Expr &row_count) {
  m_base.mutable_row_count()->CopyFrom(row_count);
}

Limit_expr::Limit_expr(const Expr &row_count, const Expr &offset) {
  m_base.mutable_row_count()->CopyFrom(row_count);
  m_base.mutable_offset()->CopyFrom(offset);
}

Update_operation::Update_operation(const Update_type &update_type,
                                   const Column_identifier &source,
                                   const Expr &value) {
  m_base.set_operation(update_type);
  m_base.mutable_source()->CopyFrom(source);
  m_base.mutable_value()->CopyFrom(value);
}

Update_operation::Update_operation(const Update_type &update_type,
                                   const Document_path &source,
                                   const Expr &value) {
  m_base.set_operation(update_type);
  m_base.mutable_source()->CopyFrom(Column_identifier(source));
  m_base.mutable_value()->CopyFrom(value);
}

Update_operation::Update_operation(const Update_type &update_type,
                                   const Document_path &source) {
  m_base.set_operation(update_type);
  m_base.mutable_source()->CopyFrom(Column_identifier(source));
}

Update_operation::Update_operation(const Update_type &update_type,
                                   const Column_identifier &source) {
  m_base.set_operation(update_type);
  m_base.mutable_source()->CopyFrom(source);
}

}  // namespace test
}  // namespace xpl
