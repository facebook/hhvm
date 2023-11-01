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

namespace apache {
namespace thrift {
namespace compiler {
namespace go {

class codegen_data {
 public:
  // the import path for the supporting library
  std::string thrift_lib_import =
      "github.com/facebook/fbthrift/thrift/lib/go/thrift";
  // the import path for the supporting metadata library
  std::string thrift_metadata_import =
      "github.com/facebook/fbthrift/thrift/lib/thrift/metadata";
  // package name override (otherwise inferred from thrift file by default)
  std::string package_override;

  // whether to generate code compatible with the old Go generator
  // (to make the migration easier)
  bool compat = true;
  // whether to generate "legacy" getters which do not properly support optional
  // fields (to make the migration easier)
  bool compat_getters = true;
  // whether to generate "legacy" setters which do not properly support optional
  // fields (to make the migration easier)
  bool compat_setters = true;
  // whether to generate Thrift metadata
  bool gen_metadata = true;

  // Records field names for every struct in the program.
  // This is needed to resolve some edge case name collisions.
  std::map<std::string, std::set<std::string>> struct_to_field_names = {};
  // Req/Resp structs are internal and must be unexported (i.e. lowercase)
  // This set will help us track these srtucts by name.
  std::set<std::string> req_resp_struct_names;
  // Mapping of service name to a vector of req/resp structs for that service.
  std::map<std::string, std::vector<t_struct*>> service_to_req_resp_structs =
      {};

  void set_current_program(const t_program* program);

  void compute_go_package_aliases();
  void compute_struct_to_field_names();
  void compute_service_to_req_resp_structs();

  bool is_current_program(const t_program* program);

  std::string get_go_package_alias(const t_program* program);
  std::string go_package_alias_prefix(const t_program* program);

 private:
  std::string make_go_package_name_unique(const std::string& name);

  // The current program being generated.
  const t_program* current_program_;
  // Key: package name according to Thrift.
  // Value: package name to use in generated code.
  std::map<std::string, std::string> go_package_map_;
  // A map for keeping track and resolving package name collisions.
  std::map<std::string, int32_t> go_package_name_collisions_ = {
      {"thrift", 0},
      {"context", 0},
      {"fmt", 0},
      {"strings", 0},
      {"sync", 0},
      {"metadata", 0},
  };
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

bool is_func_go_supported(const t_function* func);
bool is_go_reserved_word(const std::string& value);

bool is_type_go_struct(const t_type* type);
bool is_type_go_nilable(const t_type* type);
bool is_type_go_comparable(
    const t_type* type, std::map<std::string, int> visited_type_names = {});
bool is_type_metadata_primitive(const t_type* type);

std::string get_go_field_name(const t_field* field);
std::string get_go_func_name(const t_function* func);

std::set<std::string> get_struct_go_field_names(const t_structured* tstruct);

std::vector<t_struct*> get_service_req_resp_structs(const t_service* service);

const std::string* get_go_name_annotation(const t_named* node);
const std::string* get_go_tag_annotation(const t_named* node);

} // namespace go
} // namespace compiler
} // namespace thrift
} // namespace apache
