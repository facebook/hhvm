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

#include <thrift/compiler/ast/t_program.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace go {

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

std::string make_unique_name(
    std::map<std::string, int32_t>& name_collisions, const std::string& name);

bool is_func_go_supported(const t_function* func);
bool is_go_reserved_word(const std::string& value);

bool is_type_nilable(const t_type* type);
bool is_type_go_struct(const t_type* type);
bool is_type_go_comparable(
    const t_type* type, std::map<std::string, int> visited_type_names = {});

std::string get_go_field_name(const t_field* field);
std::string get_go_func_name(const t_function* func);

std::set<std::string> get_struct_go_field_names(const t_struct* struct_);

std::vector<t_struct*> get_service_req_resp_structs(const t_service* service);

const std::string* get_go_name_annotation(const t_named* node);
const std::string* get_go_tag_annotation(const t_named* node);

} // namespace go
} // namespace compiler
} // namespace thrift
} // namespace apache
