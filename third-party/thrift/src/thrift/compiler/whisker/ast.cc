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

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <cassert>
#include <iterator>

namespace whisker::ast {

namespace {

template <typename T, typename ToStringFunc>
std::string to_joined_string(
    const std::vector<T>& parts, char separator, ToStringFunc&& to_string) {
  assert(!parts.empty());
  std::string result = to_string(parts.front());
  for (auto part = std::next(parts.begin()); part != parts.end(); ++part) {
    result += separator;
    result += to_string(*part);
  }
  return result;
}

} // namespace

std::string variable_lookup::chain_string() const {
  return detail::variant_match(
      chain,
      [](this_ref) -> std::string { return "."; },
      [](const std::vector<identifier>& ids) -> std::string {
        return to_joined_string(
            ids, '.', [](const identifier& id) -> const std::string& {
              return id.name;
            });
      });
}

std::string partial_lookup::as_string() const {
  return to_joined_string(
      parts, '/', [](const path_component& component) -> const std::string& {
        return component.value;
      });
}

std::string partial_apply::path_string() const {
  return path.as_string();
}

std::string expression::to_string() const {
  return detail::variant_match(
      content,
      [](const variable_lookup& v) { return v.chain_string(); },
      [](const function_call& f) {
        std::string out = fmt::format("({}", f.name());
        for (const auto& arg : f.args) {
          fmt::format_to(std::back_inserter(out), " {}", arg.to_string());
        }
        out += ")";
        return out;
      });
}

std::string_view expression::function_call::name() const {
  return detail::variant_match(
      which,
      [&](function_call::not_tag) { return "not"; },
      [&](function_call::and_tag) { return "and"; },
      [&](function_call::or_tag) { return "or"; });
}

std::string_view pragma_statement::to_string() const {
  switch (pragma) {
    case pragma_statement::pragmas::single_line:
      return "single-line";
  }
}

} // namespace whisker::ast
