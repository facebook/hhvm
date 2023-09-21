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

#include <thrift/compiler/ast/t_node.h>

namespace apache {
namespace thrift {
namespace compiler {

const std::string t_node::kEmptyString;

const std::string* t_node::find_annotation_or_null(
    const std::vector<std::string_view>& names) const {
  for (std::string_view name : names) {
    auto itr = annotations_.find(name);
    if (itr != annotations_.end()) {
      return &itr->second.value;
    }
  }
  return nullptr;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
