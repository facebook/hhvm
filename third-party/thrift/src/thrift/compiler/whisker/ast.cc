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

#include <thrift/compiler/whisker/ast.h>
#include <thrift/compiler/whisker/detail/overload.h>
#include <thrift/compiler/whisker/detail/string.h>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <cassert>
#include <iterator>
#include <sstream>

namespace whisker::ast {

namespace {

template <typename T, typename ToStringFunc>
std::string to_joined_string(
    const std::vector<T>& parts, char separator, ToStringFunc&& to_string) {
  assert(!parts.empty());
  std::ostringstream result;
  result << to_string(parts.front());
  for (auto part = std::next(parts.begin()); part != parts.end(); ++part) {
    result << separator << to_string(*part);
  }
  return result.str();
}

} // namespace

std::string text::joined() const {
  std::string result;
  for (const auto& part : parts) {
    result += detail::variant_match(
        part, [](const auto& s) -> std::string_view { return s.value; });
  }
  return result;
}

variable_component::variable_component(
    source_range loc_,
    std::optional<identifier> qualifier_,
    identifier property_)
    : loc(loc_),
      qualifier(std::move(qualifier_)),
      property(std::move(property_)) {
  as_string_ = qualifier.has_value()
      ? fmt::format("{}:{}", qualifier->name, property.name)
      : property.name;
}

const std::string& variable_component::as_string() const {
  return as_string_;
}

std::string variable_lookup::chain_string() const {
  return detail::variant_match(
      chain,
      [](this_ref) -> std::string { return "this"; },
      [](const std::vector<variable_component>& ids) -> std::string {
        return to_joined_string(
            ids, '.', [](const variable_component& id) -> const std::string& {
              return id.as_string();
            });
      });
}

std::string macro_lookup::as_string() const {
  return to_joined_string(
      parts, '/', [](const path_component& component) -> const std::string& {
        return component.value;
      });
}

std::string macro::path_string() const {
  return path.as_string();
}

std::string expression::to_string() const {
  return detail::variant_match(
      which,
      [](const string_literal& s) {
        return fmt::format("\"{}\"", detail::escape(s.text));
      },
      [](const i64_literal& i) { return fmt::format("{}", i.value); },
      [](const null_literal&) -> std::string { return "null"; },
      [](const true_literal&) -> std::string { return "true"; },
      [](const false_literal&) -> std::string { return "false"; },
      [](const variable_lookup& v) { return v.chain_string(); },
      [](const function_call& f) {
        std::string out = fmt::format("({}", f.name());
        for (const auto& arg : f.positional_arguments) {
          fmt::format_to(std::back_inserter(out), " {}", arg.to_string());
        }
        for (const auto& [name, arg] : f.named_arguments) {
          fmt::format_to(
              std::back_inserter(out), " {}={}", name, arg.value->to_string());
        }
        out += ")";
        return out;
      });
}

std::string expression::function_call::name() const {
  return detail::variant_match(
      which,
      [](function_call::builtin_not) -> std::string { return "not"; },
      [](function_call::builtin_and) -> std::string { return "and"; },
      [](function_call::builtin_or) -> std::string { return "or"; },
      [](function_call::builtin_ternary) -> std::string { return "if"; },
      [](const function_call::user_defined& f) -> std::string {
        return f.name.chain_string();
      });
}

std::string_view pragma_statement::to_string() const {
  switch (pragma) {
    case pragma_statement::pragmas::ignore_newlines:
      return "ignore-newlines";
  }
  abort();
}

} // namespace whisker::ast
