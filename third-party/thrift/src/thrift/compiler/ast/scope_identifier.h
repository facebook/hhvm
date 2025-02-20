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

namespace apache::thrift::compiler::scope {

// An identifier without any scope, e.g. `Foo`
struct unscoped_id {
  std::string_view name;
};

// An identifier with a scope, e.g. `foo.Bar`
struct scoped_id {
  std::string_view scope;
  std::string_view name;
};

// A fully qualified enum value identifier, e.g. `foo.Bar.BAZ`
struct enum_id {
  std::string_view scope;
  std::string_view enum_name;
  std::string_view value_name;
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
  identifier(std::string_view scope, std::string_view name)
      : type_(parse(scope, name)) {}
  /* implicit */ identifier(std::string_view id) : type_(parse(id)) {}
  /* implicit */ identifier(const std::string& id) : type_(parse(id)) {}

  bool has_scope() const;
  std::string_view scope() const;
  bool is_scoped_id() const;

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

 private:
  static Type parse(std::string_view scope, std::string_view name);
  static Type parse(std::string_view name);

 private:
  Type type_;
};

} // namespace apache::thrift::compiler::scope
