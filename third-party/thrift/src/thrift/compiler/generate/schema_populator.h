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
#include <string>
#include <string_view>
#include <utility>

#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/sema/schematizer.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>
#include <thrift/lib/thrift/gen-cpp2/schema_types.h>

namespace apache::thrift::compiler {

class t_const;
class t_enum;
class t_exception;
class t_global_scope;
class t_interaction;
class t_interface;
class t_program;
class t_service;
class t_struct;
class t_structured;
class t_typedef;
class t_union;

namespace detail {

class schema_populator {
 public:
  using intern_func = std::function<schematizer::value_id(
      protocol::Value value, t_program* program)>;

  schema_populator(schematizer& schema_utils, intern_func intern_value)
      : schema_utils_(schema_utils), intern_value_(std::move(intern_value)) {}

  // Creates a generated schema struct describing the argument.
  // https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift
  type::Struct gen_schema(const t_struct& node);
  type::Union gen_schema(const t_union& node);
  type::Exception gen_schema(const t_exception& node);
  type::Interaction gen_schema(const t_interaction& node);
  type::Service gen_schema(const t_service& node);
  type::Const gen_schema(const t_const& node);
  type::Enum gen_schema(const t_enum& node);
  type::Program gen_schema(const t_program& node);
  type::Typedef gen_schema(const t_typedef& node);

  // Creates a schema.Schema describing the argument and all
  // types recursively referenced by it. Calls gen_schema internally.
  type::Schema gen_full_schema(const t_service& node);

 private:
  type::TypeUri type_uri(const t_type& type);
  type::TypeUri type_uri(const t_named& node, bool use_hash);

  type::DefinitionAttrs gen_attrs(
      const t_named& node, const t_program* program);

  type::Type gen_type(
      schema_populator* generator,
      const t_program* program,
      type::DefinitionList* defns_schema,
      const t_type& type);

  type::Type gen_type(const t_type& type, const t_program* program) {
    return gen_type(nullptr, program, nullptr, type);
  }

  type::Fields gen_fields(
      schema_populator* generator,
      const t_program* program,
      type::DefinitionList* defns_schema,
      node_list_view<const t_field> fields);

  type::Functions gen_functions(const t_interface& node);

  const schematizer::options& opts() const { return schema_utils_.opts(); }

  schematizer& schema_utils_;
  intern_func intern_value_;
};

class protocol_value_builder {
 public:
  // Start resolving from a struct definition with the given type
  explicit protocol_value_builder(const t_type& struct_ty);

  // Directly resolves to the type of the underlying ProtocolObject value
  // without external type info.
  [[nodiscard]] static protocol_value_builder as_value_type();

  // Resolves to the inner property of a given type.
  // - For `t_struct` this finds a field with a matching name
  // - For `t_map` this resolves to type of the value
  [[nodiscard]] protocol_value_builder property(const t_const_value& key) const;

  protocol::Value wrap(const t_const_value& val) const;

 private:
  explicit protocol_value_builder();

  // Resolves to the key-type of a map or struct.
  [[nodiscard]] protocol_value_builder key(const t_const_value& key) const;

  // Resolves to the type of the elements in a container.
  // Note: This excludes `map`, which is handled separately for key & value.
  [[nodiscard]] protocol_value_builder container_element(
      const t_const_value& val) const;

  protocol::Value to_labeled_value(const t_const_value& value) const;

 private:
  const t_type* ty_;
};

} // namespace detail
} // namespace apache::thrift::compiler
