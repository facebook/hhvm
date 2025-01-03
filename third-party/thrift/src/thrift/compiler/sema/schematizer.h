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
#include <unordered_map>

#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_field.h>

namespace apache::thrift::compiler {

class t_enum;
class t_program;
class t_scope;
class t_service;
class t_structured;
class t_typedef;

namespace detail {

class protocol_value_builder;

class schematizer {
 public:
  enum class value_id : int64_t {};

  using intern_func = std::function<value_id(
      std::unique_ptr<t_const_value> val, t_program* program)>;

  struct options {
    bool double_writes : 1; // Legacy copies of data for backward compatiblity.

    bool include_annotations : 1;
    bool include_docs : 1;
    bool include_source_ranges : 1;

    intern_func intern_value;
    bool use_hash = false; // Uses typeHashPrefixSha2_256 in typeUri and
                           // definitionKey instead of definitionId.
    bool include_generated_ = false;
    bool source_ranges_ = false;
    bool only_root_program_ = false;

    options()
        : double_writes(true),
          include_annotations(true),
          include_docs(true),
          include_source_ranges(true) {}
  };

  explicit schematizer(const t_scope& scope, source_manager& sm, options opts)
      : scope_(scope), sm_(sm), opts_(std::move(opts)) {}

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

  // Gets a universally unique identifier for a definition that is consistent
  // across runs on different including programs.
  std::string identify_definition(const t_named& node);
  int64_t identify_program(const t_program& node);

  // Get the name of the program's schema const.
  static std::string name_schema(source_manager& sm, const t_program& node);

 private:
  const t_scope& scope_;
  source_manager& sm_;
  options opts_;
  std::unordered_map<const t_program*, std::string> program_checksums_;

  t_type_ref std_type(std::string_view uri);
  std::unique_ptr<t_const_value> type_uri(const t_type& type);

  void add_definition(
      t_const_value& schema,
      const t_named& node,
      const t_program* program,
      intern_func& intern_value);

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
      intern_func& intern_value);

  std::string_view program_checksum(const t_program& program);
};

// Tag for obtaining a compact-encoded schema for the root program via a
// pluggable function.
struct get_schema_tag {
  static std::string default_impl(
      schematizer::options& /* schema_opts */,
      source_manager& /* source_mgr */,
      const t_program& /* root_program */) {
    return {};
  }
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

  std::unique_ptr<t_const_value> wrap(
      const t_const_value& val, t_type_ref ttype) const;

 private:
  explicit protocol_value_builder();

  // Resolves to the key-type of a map or struct.
  [[nodiscard]] protocol_value_builder key(const t_const_value& key) const;

  // Resolves to the type of the elements in a container.
  // Note: This excludes `map`, which is handled separately for key & value.
  [[nodiscard]] protocol_value_builder container_element(
      const t_const_value& val) const;

  // Generates a self-describing value pair for a given `t_const_value`, e.g.
  // String("i64Value") => I64(42)
  // String("stringValue") => String("hello")
  std::pair<std::unique_ptr<t_const_value>, std::unique_ptr<t_const_value>>
  to_labeled_value(const t_const_value& value) const;

 private:
  const t_type* ty_;
};

} // namespace detail
} // namespace apache::thrift::compiler
