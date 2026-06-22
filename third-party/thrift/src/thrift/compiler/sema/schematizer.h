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

#include <string>
#include <string_view>
#include <unordered_map>

namespace apache::thrift::compiler {

class source_manager;
class t_named;
class t_program;
class t_global_scope;

namespace detail {

class schematizer {
 public:
  enum class value_id : int64_t {};

  struct options {
    bool double_writes : 1; // Legacy copies of data for backward compatiblity.

    bool include_annotations : 1;
    bool include_docs : 1;
    bool include_source_ranges : 1;

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

  explicit schematizer(
      const t_global_scope& global_scope, source_manager& sm, options opts)
      : global_scope_(global_scope), sm_(sm), opts_(opts) {}

  // Gets a universally unique identifier for a definition that is consistent
  // across runs on different including programs.
  std::string identify_definition(const t_named& node);
  int64_t identify_program(const t_program& node);
  static size_t definition_identifier_length();

  // Get the name of the program's schema const.
  static std::string schema_const_name(
      source_manager& sm, const t_program& node);

  const options& opts() const { return opts_; }

 private:
  const t_global_scope& global_scope_;
  source_manager& sm_;
  options opts_;
  std::unordered_map<const t_program*, std::string> program_checksums_;

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

} // namespace detail
} // namespace apache::thrift::compiler
