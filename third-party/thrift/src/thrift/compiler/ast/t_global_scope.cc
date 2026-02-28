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

#include <thrift/compiler/ast/scope_identifier.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_global_scope.h>
#include <thrift/compiler/ast/t_program.h>

namespace apache::thrift::compiler {

t_type_ref t_global_scope::ref_type(
    const t_program& program,
    const std::string& name,
    const source_range& range) {
  // Try to resolve the type.
  if (const t_type* type = program.find<t_type>({name, range})) {
    return {*type, range}; // We found the type!
  }

  /*
   Either this type isn't yet declared, or it's never
   declared. Either way allow it and we'll figure it out
   during generation.
  */
  // NOTE(afuller): This assumes that, since the type was referenced by name, it
  // is safe to create a dummy typedef to use as a proxy for the original type.
  // However, this actually breaks dynamic casts.
  // TODO(T244601847): Merge t_placeholder_typedef into t_type_ref and remove
  // const cast.
  auto ph = std::make_unique<t_placeholder_typedef>(&program, name);
  ph->set_src_range(range);
  return add_placeholder_typedef(std::move(ph));
}

const t_named* t_global_scope::add_def_by_uri(const t_named& node) {
  if (!node.uri().empty()) {
    auto result = definitions_by_uri_.emplace(node.uri(), &node);
    if (!result.second) {
      return result.first->second;
    }
  }
  return nullptr;
}

void t_global_scope::add_program(const t_program& program) {
  const auto [it, _] = program_scopes_.try_emplace(
      program.name(), std::vector<const scope::program_scope*>{});
  it->second.push_back(&program.program_scope());
  program_order_.emplace(&program, program_order_.size());
}

const t_global_scope::ProgramScopes& t_global_scope::program_scopes() const {
  return program_scopes_;
}

void t_global_scope::add_definition(
    const t_named& node, std::string_view name, std::string_view value_name) {
  if (value_name != "") {
    // [TEMPORARY] Add global definition for <scope>.<value_name>
    // NOTE: Enum values can't override existing definitions
    all_definitions_.try_emplace(
        global_id{node.program()->name(), value_name, ""}, &node);
  }

  all_definitions_[global_id{node.program()->name(), name, value_name}] = &node;
}

scope::program_scope::ScopePriority t_global_scope::global_priority(
    const t_program& program) const {
  const auto it = program_order_.find(&program);
  if (it == program_order_.end()) {
    // Only the root program should be missing.
    return scope::program_scope::ROOT_PROGRAM_PRIORITY;
  }
  return it->second;
}

} // namespace apache::thrift::compiler
