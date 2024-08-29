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

#include <cassert>
#include <iterator>
#include <utility>

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
      [](const std::vector<identifier>& chain) -> std::string {
        return to_joined_string(
            chain, '.', [](const identifier& id) -> const std::string& {
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

} // namespace whisker::ast
