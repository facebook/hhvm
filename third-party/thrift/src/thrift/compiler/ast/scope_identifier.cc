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

#include <cassert>
#include <fmt/core.h>

#include <thrift/compiler/ast/scope_identifier.h>

namespace apache::thrift::compiler::scope {

identifier::Type identifier::parse(
    std::string_view scope, std::string_view name, source_range loc) {
  const auto pos = name.find_first_of('.');
  if (pos == std::string_view::npos) {
    return scoped_id{.loc = loc, .scope = scope, .name = name};
  }

  std::string_view enum_name = name.substr(0, pos);
  std::string_view value_name = name.substr(pos + 1);

  return enum_id{
      .loc = loc,
      .scope = scope,
      .enum_name = enum_name,
      .value_name = value_name};
}

identifier::Type identifier::parse(std::string_view name, source_range loc) {
  auto pos = name.find_first_of('.');
  if (pos == std::string_view::npos) {
    return unscoped_id{.loc = loc, .name = name};
  }

  std::string_view scope = name.substr(0, pos);
  std::string_view name_or_enum = name.substr(pos + 1);

  auto second_dot = name_or_enum.find_first_of('.');
  if (second_dot == std::string_view::npos) {
    return scoped_id{.loc = loc, .scope = scope, .name = name_or_enum};
  }

  return enum_id{
      loc,
      scope,
      name_or_enum.substr(0, second_dot),
      name_or_enum.substr(second_dot + 1)};
}

bool identifier::has_scope() const {
  return !std::holds_alternative<unscoped_id>(type_);
}

bool identifier::is_scoped_id() const {
  return std::holds_alternative<scoped_id>(type_);
}

source_range identifier::src_range() const {
  return visit(
      [](const unscoped_id& id) { return id.loc; },
      [&](const scoped_id& id) { return id.loc; },
      [&](const enum_id& id) { return id.loc; });
}

std::string_view identifier::get_base_name() const {
  return visit(
      [](const unscoped_id& id) { return id.name; },
      [&](const scoped_id& id) { return id.name; },
      [&](const enum_id& id) { return id.value_name; });
}

std::string identifier::fmtDebug() const {
  return visit(
      [](const unscoped_id& id) { return fmt::format("{}", id.name); },
      [&](const scoped_id& id) {
        return fmt::format("{}.{}", id.scope, id.name);
      },
      [&](const enum_id& id) {
        return fmt::format("{}.{}.{}", id.scope, id.enum_name, id.value_name);
      });
}

std::string_view identifier::scope() const {
  return visit(
      [&](const scoped_id& id) { return id.scope; },
      [&](const enum_id& id) { return id.scope; },
      [](const unscoped_id&) -> std::string_view {
        assert(false && "unscoped_id does not have a scope");
        return {};
      });
}

std::pair<std::string_view, std::string_view> identifier::unscope() const {
  using PairId = std::pair<std::string_view, std::string_view>;

  return visit(
      [](scope::unscoped_id&& unscoped) -> PairId {
        return std::make_pair(unscoped.name, UNUSED);
      },
      [](scope::scoped_id&& scoped) -> PairId {
        return std::make_pair(scoped.name, UNUSED);
      },
      [](scope::enum_id&& enum_id) -> PairId {
        return std::make_pair(enum_id.enum_name, enum_id.value_name);
      });
}

identifier::Pieces identifier::split() const {
  return visit(
      [](const unscoped_id& id) { return Pieces{UNUSED, id.name, UNUSED}; },
      [&](const scoped_id& id) { return Pieces{id.scope, id.name, UNUSED}; },
      [&](const enum_id& id) {
        return Pieces{id.scope, id.enum_name, id.value_name};
      });
}

bool operator==(const unscoped_id& lhs, const unscoped_id& rhs) {
  return lhs.name == rhs.name;
}

bool operator==(const scoped_id& lhs, const scoped_id& rhs) {
  return lhs.scope == rhs.scope && lhs.name == rhs.name;
}

bool operator==(const enum_id& lhs, const enum_id& rhs) {
  return lhs.scope == rhs.scope && lhs.enum_name == rhs.enum_name &&
      lhs.value_name == rhs.value_name;
}

} // namespace apache::thrift::compiler::scope
