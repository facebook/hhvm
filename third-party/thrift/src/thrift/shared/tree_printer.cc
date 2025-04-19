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

#include <thrift/shared/detail/string.h>
#include <thrift/shared/tree_printer.h>

#include <algorithm>

namespace apache::thrift::tree_printer {

std::string escape(std::string_view str) {
  return detail::escape(str);
}

std::ostream& operator<<(
    std::ostream& out, const scope::nesting_context& self) {
  using kind = scope::nesting_context::kind;
  if (self.parent_ == nullptr) {
    // depth of 0 has no output
    assert(self.kind_ == kind::node);
    return out;
  }

  std::string buffer;
  if (self.kind_ == kind::node) {
    buffer += "-|";
  } else {
    buffer += "-`";
  }
  for (auto parent = self.parent_; parent != nullptr;
       parent = parent->parent_) {
    if (parent->parent_ == nullptr) {
      // depth of 0 has no output
      break;
    }
    if (parent->kind_ == kind::node) {
      buffer += " |";
    } else {
      buffer += "  ";
    }
  }

  std::reverse(buffer.begin(), buffer.end());
  return out << buffer;
}

} // namespace apache::thrift::tree_printer
