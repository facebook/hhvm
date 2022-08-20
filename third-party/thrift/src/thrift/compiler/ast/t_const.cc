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

#include <algorithm>

#include <fmt/core.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_const_value.h>

namespace apache {
namespace thrift {
namespace compiler {

const t_const_value& t_const::get_value_from_structured_annotation(
    const char* key) const {
  const t_const_value* value =
      get_value_from_structured_annotation_or_null(key);

  if (!value) {
    throw std::runtime_error(fmt::format("key `{}` not found.", key));
  }
  return *value;
}

const t_const_value* t_const::get_value_from_structured_annotation_or_null(
    const char* key) const {
  const auto& annotations = value()->get_map();
  const auto it = std::find_if(
      annotations.begin(), annotations.end(), [key](const auto& item) {
        return item.first->get_string() == key;
      });
  return (it != annotations.end()) ? it->second : nullptr;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
