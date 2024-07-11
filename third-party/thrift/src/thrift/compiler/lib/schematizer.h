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
#include <unordered_map>

#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_scope.h>
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

  enum class included_data : uint16_t {
    DoubleWrites = 1, // Legacy copies of data for backcompat
    Annotations = 2,
    Docs = 4,
    SourceRanges = 8,
  };

  struct included_data_set {
    bool test(included_data item) const {
      return data & static_cast<storage>(item);
    }
    void set(included_data item) { data |= static_cast<storage>(item); }
    void reset(included_data item) { data &= ~static_cast<storage>(item); }

    // Needed to work around https://github.com/llvm/llvm-project/issues/36032
    included_data_set() : data(static_cast<storage>(-1)) {}

   private:
    using storage = std::underlying_type_t<included_data>;
    storage data;
  };

  struct options {
    included_data_set include;
    InternFunc intern_value;
    bool use_hash = false; // Uses typeHashPrefixSha2_256 in typeUri and
                           // definitionKey instead of definitionId.
    bool include_generated_ = false;
    bool source_ranges_ = false;
    bool only_root_program_ = false;
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
  std::string_view program_checksum(const t_program& program);
};

// Turns a t_const_value holding some value V into a t_const_value holding a
// protocol.Value representing V.
// Currently only uses bool/i64/double/string/list/map.
// TODO: allow increasing type fidelity.
std::unique_ptr<t_const_value> wrap_with_protocol_value(
    const t_const_value& value, t_type_ref ttype);

// Tag for obtaining a compact-encoded schema for the root program via pluggable
// function.
struct GetSchemaTag {
  static std::string defaultImpl(
      schematizer::options& /* schema_opts */,
      source_manager& /* source_mgr */,
      const t_program& /* root_program */) {
    return {};
  }
};
} // namespace compiler
} // namespace thrift
} // namespace apache
