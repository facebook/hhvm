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

#include <unordered_map>
#include <unordered_set>

#include <thrift/compiler/ast/scope_identifier.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_enum_value.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_node.h>

namespace apache::thrift::compiler::scope {

// Indexes all definitions within a t_program
class program_scope {
  // A program identifier is a either:
  // - A type definition e.g. `Foo` (encoded as [Foo, nullptr])
  // - An enum value e.g. `Foo.Bar` (encoded as [Foo, Bar])
  struct id {
    std::string_view definition_name;
    std::optional<std::string_view> subdefinition_name;

    bool operator==(const id& other) const {
      return definition_name == other.definition_name &&
          subdefinition_name == other.subdefinition_name;
    }
  };

  struct id_hasher {
    size_t operator()(const id& id) const {
      return std::hash<std::string_view>{}(id.definition_name) ^
          std::hash<std::string_view>{}(
                 id.subdefinition_name.value_or(scope::identifier::UNUSED));
    }
  };

 public:
  using Definitions = std::unordered_map<id, const t_named*, id_hasher>;
  using ScopePriority = std::uint64_t;

  constexpr static ScopePriority ROOT_PROGRAM_PRIORITY =
      std::numeric_limits<ScopePriority>::max() - 1;
  constexpr static ScopePriority ALIAS_PRIORITY =
      std::numeric_limits<ScopePriority>::max();

  // Find a definition by either:
  // * one-part identifier, e.g. `Foo`
  // * two-part identifier, e.g. `Foo.Bar`
  //   * This is used for enum values, e.g. enum Foo { Bar = 1 }
  template <typename Node = t_named>
  const Node* find_by_name(
      std::string_view definition_name,
      std::optional<std::string_view> subdefinition_name = std::nullopt) const {
    auto it = definitions_.find(id{definition_name, subdefinition_name});
    if (it == definitions_.end()) {
      return nullptr;
    }
    return dynamic_cast<const Node*>(it->second);
  }

  void add_definition(scope::identifier ident, const t_named* named) {
    const auto [definition_name, subdefinition_name] = ident.unscope();
    std::optional<std::string_view> subdef_name =
        subdefinition_name == scope::identifier::UNUSED
        ? std::optional<std::string_view>{std::nullopt}
        : subdefinition_name;
    definitions_.insert_or_assign(id{definition_name, subdef_name}, named);

    if (const enum_id* enum_id_ = ident.get_enum_id()) {
      // Allow access to the enum values using `<scope>.<value>` identifiers
      //
      // Note: These definitions are not allowed to redefine existing
      // definitions, to prevent e.g. enum variants overriding types
      // TODO: T212609345 Remove the ability to access unscoped enum
      auto [_, inserted] =
          definitions_.emplace(id{enum_id_->value_name, std::nullopt}, named);

      const auto* constant = dynamic_cast<const t_const*>(named);
      assert(constant->value()->is_enum());
      const std::string& enum_value_name =
          constant->value()->get_enum_value()->name();

      // DEVNOTE: UNKNOWN is excluded from this check because it is widely used
      // & would lead to a lot of false positives. This (likely) relied on the
      // assumption that UNKNOWN has value 0, which is not guaranteed. This
      // should be removed once enum values cannot no longer be accessed without
      // the enum name.
      if (!inserted && enum_value_name != "UNKNOWN") {
        redefined_enum_values_.insert(enum_value_name);
      }
    }
  }

  bool is_ambiguous_enum_value(const std::string_view enum_value_name) const {
    return redefined_enum_values_.count(enum_value_name);
  }

  std::vector<std::string> find_ambiguous_enum_values(
      std::string_view name) const {
    std::unordered_set<std::string> ambiguous_values;
    for (const auto& [id, _] : definitions_) {
      if (id.subdefinition_name && id.subdefinition_name.value() == name) {
        ambiguous_values.insert(
            fmt::format(
                "{}.{}", id.definition_name, id.subdefinition_name.value()));
      }
    }

    return {ambiguous_values.begin(), ambiguous_values.end()};
  }

 private:
  Definitions definitions_;

  // List of enum values that have multiple definitions when used without the
  // enum name, e.g. `enum scope.Foo { One = 1, Two = 2 }` & `enum scope.Bar {
  // Two = 22, Three = 33 }` `scope.Two` is ambiguous to use without specifying
  // either `Bar.Two` or `Foo.Two`
  // TODO: T212609345 Remove the ability to access unscoped enum
  std::unordered_set<std::string_view> redefined_enum_values_;
};

} // namespace apache::thrift::compiler::scope
