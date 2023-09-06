/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <functional>
#include <memory>

#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_typedef.h>

namespace apache {
namespace thrift {
namespace compiler {
class schematizer {
 public:
  using InternFunc = std::function<t_program::value_id(
      std::unique_ptr<t_const_value> val, t_program* program)>;

  explicit schematizer(
      const t_program_bundle* bundle,
      InternFunc intern_value = &default_intern_value)
      : bundle_(bundle), intern_value_(std::move(intern_value)) {}

  // Creates a constant of type schema.Struct describing the argument.
  // https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift
  std::unique_ptr<t_const_value> gen_schema(const t_structured& node);
  std::unique_ptr<t_const_value> gen_schema(const t_service& node);
  std::unique_ptr<t_const_value> gen_schema(const t_const& node);
  std::unique_ptr<t_const_value> gen_schema(const t_enum& node);
  std::unique_ptr<t_const_value> gen_schema(const t_program& node);
  std::unique_ptr<t_const_value> gen_schema(const t_typedef& node);

  // Creates a constant of type schema.Schema describing the argument and all
  // types recursively referenced by it. Calls gen_schema internally.
  std::unique_ptr<t_const_value> gen_full_schema(const t_service& node);

 private:
  const t_program_bundle* bundle_;
  InternFunc intern_value_;

  static t_program::value_id default_intern_value(
      std::unique_ptr<t_const_value> val, t_program* program);

  t_type_ref stdType(std::string_view uri);
  std::unique_ptr<t_const_value> typeUri(const t_type& type);
  void add_definition(
      t_const_value& schema,
      const t_named& node,
      const t_program* program,
      schematizer::InternFunc& intern_value);
  std::unique_ptr<t_const_value> gen_type(
      schematizer* generator,
      const t_program* program,
      t_const_value* defns_schema,
      const t_type& type);
  std::unique_ptr<t_const_value> gen_type(
      const t_type& type, const t_program* program) {
    return gen_type(nullptr, program, nullptr, type);
  }
  void add_fields(
      schematizer* generator,
      const t_program* program,
      t_const_value* defns_schema,
      t_const_value& schema,
      const std::string& fields_name,
      node_list_view<const t_field> fields,
      schematizer::InternFunc& intern_value);
};

// Turns a t_const_value holding some value V into a t_const_value holding a
// protocol.Value representing V.
// Currently only uses bool/i64/double/string/list/map.
// TODO: allow increasing type fidelity.
std::unique_ptr<t_const_value> wrap_with_protocol_value(
    const t_const_value& value, t_type_ref ttype);
} // namespace compiler
} // namespace thrift
} // namespace apache
