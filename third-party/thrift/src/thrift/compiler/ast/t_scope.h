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

class t_const;
class t_service;

/**
 * This represents a scope used for looking up types, services and other AST
 * constructs. Typically, a scope is associated with a t_program. Scopes are not
 * used to determine code generation, but rather to resolve identifiers at parse
 * time.
 */
class t_scope {
 public:
  // Get a (poetically unresolved) reference to given type, declared in the
  // given program.
  t_type_ref ref_type(
      const t_program& program,
      const std::string& name,
      const source_range& range);

  // Returns an existing def, if one is already registered with the same uri, or
  // nullptr.
  const t_named* add_def(const t_named& node);

  // Returns the type or interaction definition with the given name, or nullptr.
  const t_named* find(std::string_view name) {
    return find_or_null(definitions_, name);
  }

  // Returns the definition with the given Thrift URI, or nullptr.
  const t_named* find_by_uri(std::string_view uri) {
    return find_or_null(definitions_by_uri_, uri);
  }

  void add_definition(std::string name, const t_named* named) {
    definitions_[std::move(name)] = named;
  }

  const t_type* find_type(std::string_view name) const {
    return dynamic_cast<const t_type*>(find_or_null(definitions_, name));
  }

  void add_service(std::string name, const t_service* service) {
    services_[std::move(name)] = service;
  }

  const t_service* find_service(std::string_view name) const {
    return find_or_null(services_, name);
  }

  const t_interaction* find_interaction(std::string_view name) const {
    return dynamic_cast<const t_interaction*>(find_or_null(definitions_, name));
  }

  void add_constant(std::string name, const t_const* constant);

  const t_const* find_constant(std::string_view name) const {
    return find_or_null(constants_, name);
  }

  bool is_ambiguous_enum_value(const std::string& enum_value_name) const {
    return redefined_enum_values_.find(enum_value_name) !=
        redefined_enum_values_.end();
  }

  std::string get_fully_qualified_enum_value_names(const std::string& name);

  t_type_ref add_placeholder_typedef(
      std::unique_ptr<t_placeholder_typedef> ptd) {
    assert(ptd != nullptr);
    auto result = t_type_ref::for_placeholder(*ptd);
    placeholder_typedefs_.emplace_back(std::move(ptd));
    return result;
  }
  node_list_view<t_placeholder_typedef> placeholder_typedefs() {
    return placeholder_typedefs_;
  }
  node_list_view<const t_placeholder_typedef> placeholder_typedefs() const {
    return placeholder_typedefs_;
  }

 private:
  node_list<t_placeholder_typedef> placeholder_typedefs_;

  template <typename T>
  using map_type = std::map<std::string, const T*, std::less<>>;

  template <typename T>
  static const T* find_or_null(const map_type<T>& map, std::string_view name) {
    auto it = map.find(name);
    return it != map.end() ? it->second : nullptr;
  }

  // A map from names to type or interaction definitions.
  map_type<t_named> definitions_;

  // Map of names to constants.
  map_type<t_const> constants_;

  // Map of names to services.
  map_type<t_service> services_;

  // Set of enum value names that are redefined and are ambiguous
  // if referred to without the enum name.
  std::unordered_set<std::string> redefined_enum_values_;

  // Map of enum value names to their definition full names.
  std::unordered_map<std::string, std::unordered_set<std::string>> enum_values_;

  // Definitions by uri.
  map_type<t_named> definitions_by_uri_;
};

} // namespace compiler
} // namespace thrift
} // namespace apache
