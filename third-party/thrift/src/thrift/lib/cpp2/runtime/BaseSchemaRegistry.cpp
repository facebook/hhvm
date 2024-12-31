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

#include <thrift/lib/cpp2/runtime/BaseSchemaRegistry.h>

#include <folly/Indestructible.h>

namespace apache::thrift {

void BaseSchemaRegistry::registerSchema(
    std::string_view name, std::string_view data, std::string_view path) {
  if (accessed_) {
    throw std::runtime_error("Schemas accessed before registration complete.");
  }
  if (auto it = rawSchemas_.find(name); it != rawSchemas_.end()) {
    if (it->second.path != path) { // Needed to support dynamic linking
      throw std::runtime_error(fmt::format(
          "Checksum collision between {} and {}. Make any change to either file's content (e.g.. whitespace/comment) to fix it.",
          it->second.path,
          path));
    }
    return;
  }
  rawSchemas_[name] = {data, path};
}

BaseSchemaRegistry& BaseSchemaRegistry::get() {
  static folly::Indestructible<BaseSchemaRegistry> self;
  return *self;
}

} // namespace apache::thrift
