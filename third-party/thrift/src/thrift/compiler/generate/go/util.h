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

#include <set>
#include <string>

#include <thrift/compiler/ast/t_program.h>

namespace apache::thrift::compiler::go {

class codegen_data {
 public:
  // the import path for the supporting library
  std::string thrift_lib_import =
      "github.com/facebook/fbthrift/thrift/lib/go/thrift/types";
  // the import path for the supporting metadata library
  std::string thrift_metadata_import =
      "github.com/facebook/fbthrift/thrift/lib/thrift/metadata";
  // package name override (otherwise inferred from thrift file by default)
  std::string package_override;

  // whether to generate code compatible with the old Go generator
  // (to make the migration easier)
  bool compat = true;
  // whether to generate "legacy" setters which do not properly support optional
  // fields (to make the migration easier)
  bool compat_setters = true;
  // whether to generate Thrift metadata
  bool gen_metadata = true;
  // whether to generate DefaultGet method
  bool gen_default_get = false;
  // whether to use reflect codec
  bool use_reflect_codec = false;

  // Records field names for every structured definition in the program.
  // This is needed to resolve some edge case name collisions.
  std::map<const t_structured*, std::set<std::string>> struct_to_field_names =
      {};
  /**
   * Req/resp structs in the program.
   *
   * TODO(T244354071): This construct is the source of a memory leak. Refactor
   * codegen to work without ephemeral structs OR replace this with
   * node_list_view<t_struct> to manage lifetimes.
   *
   * `make_func_req_resp_structs` creates ephemeral structs using `new t_struct`
   * which are put into this vector. When this vector is destructed, all
   * references to those objects are dead and the memory has leaked.
   * As of writing, this is INTENTIONAL. The ephemeral structs are created with
   * (non-unique) unique_ptrs to field instances which are part of the
   * non-ephemeral AST. If this vector managed lifetimes correctly and
   * destructed those structs (and thus their list of unique_ptr fields), those
   * copied field pointers would be double-freed (once as part of the ephemeral
   * request/response struct, once as part of the original unique_ptr it was
   * extracted from).
   *
   * Thrift AST nodes are non-copyable and non-movable. The correct way to
   * handle this is to refactor code generation to only use the original
   * functions' params/throws AST nodes to generate the desired output, rather
   * than creating ephemeral AST nodes.
   */
  std::vector<const t_struct*> req_resp_structs = {};
  // A vector of types for which we need to generate metadata.
  // Order matters here - items later in the list may have a dependency
  // on items earlier in the list. This ensures that the Go code can
  // successfully build when generated based on the items in this list.
  std::vector<const t_type*> thrift_metadata_types = {};

  void set_current_program(const t_program* program);

  void compute_go_package_aliases();
  void compute_struct_to_field_names();
  void compute_req_resp_structs();
  void compute_thrift_metadata_types();

  bool is_current_program(const t_program* program);

  std::string_view maybe_munge_ident_and_cache(
      const t_named* named, bool exported = true, bool compact = true);

  std::string get_go_package_alias(const t_program* program);
  std::string go_package_alias_prefix(const t_program* program);

 private:
  std::string make_go_package_name_unique(const std::string& name);
  void add_to_thrift_metadata_types(
      const t_type* type, std::set<std::string>& visited_type_names);

  // The current program being generated.
  const t_program* current_program_;
  // Key: package name according to Thrift.
  // Value: package name to use in generated code.
  std::map<std::string, std::string> go_package_map_;
  // A map for keeping track and resolving package name collisions.
  std::map<std::string, int32_t> go_package_name_collisions_ = {
      {"thrift", 0},
      {"context", 0},
      {"errors", 0},
      {"fmt", 0},
      {"sync", 0},
      {"metadata", 0},
      {"maps", 0},
      {"reflect", 0},
  };

  struct go_munged_names_cache_key_ {
    using self = go_munged_names_cache_key_;

    std::string_view named{};
    bool exported{};
    bool compact{};

    auto as_tuple() const noexcept {
      return std::tuple{named, exported, compact};
    }

    friend bool operator<(self const& a, self const& b) noexcept {
      return a.as_tuple() < b.as_tuple();
    }
  };
  struct go_munged_names_cache_entry_ {
    std::string_view view;
    std::string ownership;
  };
  std::map<go_munged_names_cache_key_, go_munged_names_cache_entry_>
      go_munged_names_cache_;
};

// Name of the field of the response helper struct where
// the return value is stored (if function call is not void).
extern const std::string DEFAULT_RETVAL_FIELD_NAME;

// e.g. very.good.package
std::string get_go_package_name(
    const t_program* program, std::string name_override = "");
// e.g. very/good/package
std::string get_go_package_dir(
    const t_program* program, std::string name_override = "");
// e.g. package
std::string get_go_package_base_name(
    const t_program* program, std::string name_override = "");

std::string munge_ident(
    const std::string& ident, bool exported = true, bool compat = true);
std::string quote(const std::string& data);

std::string snakecase(const std::string& name);

bool is_func_go_client_supported(const t_function* func);
bool is_func_go_server_supported(const t_function* func);
bool is_go_reserved_word(const std::string& value);

bool is_type_go_struct(const t_type* type);
bool is_type_go_nilable(const t_type* type);
bool is_type_go_comparable(
    const t_type* type, std::map<std::string, int> visited_type_names = {});
bool is_type_metadata_primitive(const t_type* type);

std::string get_go_field_name(const t_field* field);
std::string get_go_func_name(const t_function* func);

std::set<std::string> get_struct_go_field_names(const t_structured* tstruct);

void make_func_req_resp_structs(
    const t_function* func,
    const std::string& prefix,
    std::vector<const t_struct*>& req_resp_structs);

const std::string* get_go_name_annotation(const t_named* node);
const std::string* get_go_tag_annotation(const t_named* node);

int get_field_size(const t_field* field, bool is_inside_union);
void optimize_fields_layout(std::vector<const t_field*>& fields, bool is_union);

} // namespace apache::thrift::compiler::go
