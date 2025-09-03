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
#include <tuple>
#include <variant>

#include <thrift/compiler/detail/overload.h>
#include <thrift/compiler/source_location.h>

namespace apache::thrift::compiler::scope {

// An identifier without any scope, e.g. `Foo`
struct unscoped_id {
  source_range loc;
  std::string_view name;

  friend bool operator==(const unscoped_id&, const unscoped_id&);
};

// An identifier with a scope, e.g. `foo.Bar`
struct scoped_id {
  source_range loc;
  std::string_view scope;
  std::string_view name;

  friend bool operator==(const scoped_id&, const scoped_id&);
};

// A fully qualified enum value identifier, e.g. `foo.Bar.BAZ`
struct enum_id {
  source_range loc;
  std::string_view scope;
  std::string_view enum_name;
  std::string_view value_name;

  friend bool operator==(const enum_id&, const enum_id&);
};

struct identifier_hasher;

class identifier {
  friend struct identifier_hasher;

 public:
  constexpr static std::string_view UNUSED = "";

  using Type = std::variant<unscoped_id, scoped_id, enum_id>;
  using Pieces =
      std::tuple<std::string_view, std::string_view, std::string_view>;

  template <typename T>
  /* implicit */ identifier(T ty) : type_(std::move(ty)) {}
  identifier(std::string_view scope, std::string_view name, source_range loc)
      : type_(parse(scope, name, loc)) {}
  identifier(std::string_view id, source_range loc) : type_(parse(id, loc)) {}
  identifier(const std::string& id, source_range loc) : type_(parse(id, loc)) {}

  bool has_scope() const;
  std::string_view scope() const;
  bool is_scoped_id() const;
  source_range src_range() const;

  const enum_id* get_enum_id() const { return std::get_if<enum_id>(&type_); }
  enum_id* get_enum_id() { return std::get_if<enum_id>(&type_); }

  std::string_view get_base_name() const;

  template <class... Funcs>
  decltype(auto) visit(Funcs&&... funcs) const {
    return detail::variant_match(Type{type_}, std::forward<Funcs>(funcs)...);
  }

  Pieces split() const;
  std::pair<std::string_view, std::string_view> unscope() const;

  std::string fmtDebug() const;

  friend bool operator==(const identifier&, const identifier&) = default;

 private:
  static Type parse(
      std::string_view scope, std::string_view name, source_range loc);
  static Type parse(std::string_view name, source_range loc);

 private:
  Type type_;
};

} // namespace apache::thrift::compiler::scope
