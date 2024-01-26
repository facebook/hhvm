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

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_interaction.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/source_location.h>

namespace apache {
namespace thrift {
namespace compiler {

/**
 * This represents a scope used for looking up types, services and other
 * definitions. Typically, a scope is associated with a t_program. Scopes are
 * not used to determine code generation, but rather to resolve identifiers at
 * parse time.
 */
class t_scope {
 public:
  void add_definition(std::string name, const t_named* named) {
    definitions_[std::move(name)] = named;
  }

  void add_enum_value(std::string name, const t_const* value);

  // Returns the definition with the given name or nullptr if there is no such
  // definition.
  const t_named* find(std::string_view name) const {
    auto it = definitions_.find(name);
    return it != definitions_.end() ? it->second : nullptr;
  }

  const t_type* find_type(std::string_view name) const {
    return dynamic_cast<const t_type*>(find(name));
  }

  const t_service* find_service(std::string_view name) const {
    return dynamic_cast<const t_service*>(find(name));
  }

  const t_interaction* find_interaction(std::string_view name) const {
    return dynamic_cast<const t_interaction*>(find(name));
  }

  const t_const* find_constant(std::string_view name) const {
    return dynamic_cast<const t_const*>(find(name));
  }

  // Returns an existing def, if one is already registered with the same uri, or
  // nullptr.
  const t_named* add_def(const t_named& node);

  // Returns the definition with the given Thrift URI, or nullptr if there is
  // no such definition.
  const t_named* find_by_uri(std::string_view uri) const {
    auto it = definitions_by_uri_.find(uri);
    return it != definitions_by_uri_.end() ? it->second : nullptr;
  }

  bool is_ambiguous_enum_value(const std::string& enum_value_name) const {
    return redefined_enum_values_.find(enum_value_name) !=
        redefined_enum_values_.end();
  }

  std::string get_fully_qualified_enum_value_names(const std::string& name);

  // Get a (poetically unresolved) reference to given type, declared in the
  // given program.
  t_type_ref ref_type(
      t_program& program, const std::string& name, const source_range& range);

  node_list_view<t_placeholder_typedef> placeholder_typedefs() {
    return placeholder_typedefs_;
  }
  node_list_view<const t_placeholder_typedef> placeholder_typedefs() const {
    return placeholder_typedefs_;
  }

 private:
  t_type_ref add_placeholder_typedef(
      std::unique_ptr<t_placeholder_typedef> ptd) {
    assert(ptd != nullptr);
    auto result = t_type_ref::for_placeholder(*ptd);
    placeholder_typedefs_.emplace_back(std::move(ptd));
    return result;
  }

  node_list<t_placeholder_typedef> placeholder_typedefs_;

  // A map from names to definitions.
  std::map<std::string, const t_named*, std::less<>> definitions_;

  // A map from URIs to definitions.
  std::map<std::string, const t_named*, std::less<>> definitions_by_uri_;

  // Set of enum value names that are redefined and are ambiguous
  // if referred to without the enum name.
  std::unordered_set<std::string> redefined_enum_values_;

  // Map of enum value names to their definition full names.
  std::unordered_map<std::string, std::unordered_set<std::string>> enum_values_;
};

} // namespace compiler
} // namespace thrift
} // namespace apache
