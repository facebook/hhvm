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

namespace whisker::ast {

std::string lookup_path::as_string(char separator) const {
  assert(!parts.empty());
  std::string result = parts[0].name;
  for (auto part = std::next(parts.begin()); part != parts.end(); ++part) {
    result += separator;
    result += part->name;
  }
  return result;
}

std::string variable_lookup::path_string() const {
  return detail::variant_match(
      path,
      [](this_ref) -> std::string { return "."; },
      [](const lookup_path& path) -> std::string {
        return path.as_string('.');
      });
}

} // namespace whisker::ast
