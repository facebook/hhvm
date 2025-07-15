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
#include <utility>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/program_scope.h>
#include <thrift/compiler/ast/scope_identifier.h>
#include <thrift/compiler/ast/t_interaction.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/detail/overload.h>
#include <thrift/compiler/source_location.h>

namespace apache::thrift::compiler {

/**
 * This represents a global-level scope, i.e. a scope shared between all
 * programs parsed by a single compiler invocation. The global scope is used for
 * looking up types, services and other definitions that cannot be resolved
 * directly via the relevant program they're used in, e.g. lookups via Uri.
 * While every program has access to the global scope, only the root t_program
 * creates an instance of it. Note: Scopes are not used to determine code
 * generation, but rather to resolve identifiers at parse time.
 */
class t_global_scope {
 public:
  using ProgramScopes = std::
      unordered_map<std::string_view, std::vector<const scope::program_scope*>>;

  struct global_id {
    std::string_view scope;
    std::string_view name;
    std::string_view value_name; // Only for enum values

    bool operator==(const global_id& other) const {
      return scope == other.scope && name == other.name &&
          value_name == other.value_name;
    }
  };

  struct global_id_hasher {
    size_t operator()(const global_id& id) const {
      return std::hash<std::string_view>{}(id.scope) ^
          std::hash<std::string_view>{}(id.name) ^
          std::hash<std::string_view>{}(id.value_name);
    }
  };

  // Returns an existing def, if one is already registered with the same uri, or
  // nullptr.
  const t_named* add_def_by_uri(const t_named& node);

  // [TEMPORARY] Adds a definition to the global scope. This is used to retain
  // the "old" behavior of globally resolving identifiers.
  void add_definition(
      const t_named& node,
      std::string_view name,
      std::string_view value_name = "");

  // Adds a scope via program/name alias to the global scope
  void add_program(const t_program& program);

  // Returns the global priority of the given program.
  // The priority is the order in which the program was added to the global
  // scope.
  scope::program_scope::ScopePriority global_priority(
      const t_program& program) const;

  // Returns the definition with the given Thrift URI, or nullptr if there is
  // no such definition.
  const t_named* find_by_uri(std::string_view uri) const {
    auto it = definitions_by_uri_.find(uri);
    return it != definitions_by_uri_.end() ? it->second : nullptr;
  }

  // Get a (poetically unresolved) reference to given type, declared in the
  // given program.
  t_type_ref ref_type(
      const t_program& program,
      const std::string& name,
      const source_range& range);

  node_list_view<t_placeholder_typedef> placeholder_typedefs() {
    return placeholder_typedefs_;
  }
  node_list_view<const t_placeholder_typedef> placeholder_typedefs() const {
    return placeholder_typedefs_;
  }

  const ProgramScopes& program_scopes() const;

  template <typename Node = t_named>
  const Node* find(scope::identifier id) const {
    const auto [scope, name, value_name] = id.split();
    const global_id gid{scope, name, value_name};
    auto it = all_definitions_.find(gid);
    if (it == all_definitions_.end()) {
      return nullptr;
    }
    return dynamic_cast<const Node*>(it->second);
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

  // A mapping from scope names/alias to a collection of definitions.
  ProgramScopes program_scopes_;

  // [TEMPORARY Global ordering of programs in the order they are added.
  // This is used to allow scope resolution to happen in the same order as
  // the all_definitions_ below.
  std::unordered_map<const t_program*, size_t> program_order_;

  // [TEMPORARY] A global list of definitions in order of their instantiation.
  // This is used to retain the "old" behavior of globally resolving identifiers
  // in the order they were defined.
  std::unordered_map<global_id, const t_named*, global_id_hasher>
      all_definitions_;

  // A map from URIs to definitions.
  std::map<std::string, const t_named*, std::less<>> definitions_by_uri_;
};

} // namespace apache::thrift::compiler
