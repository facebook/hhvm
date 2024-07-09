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

#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>

#include <folly/Indestructible.h>

namespace apache::thrift {
#ifdef FBTHRIFT_HAS_SCHEMA
void SchemaRegistry::registerSchema(std::string_view data) {
  auto schema = CompactSerializer::deserialize<type::Schema>(data);
  auto id = *schema.programs()[0].id();
  if (auto it = getSchemas().find(id); it != getSchemas().end()) {
    if (it->second.programs()[0].path() != schema.programs()[0].path()) {
      throw std::runtime_error(fmt::format(
          "Duplicate schema id for {} vs {}",
          *schema.programs()[0].path(),
          *it->second.programs()[0].path()));
    }
    return;
  }
  getSchemas()[id] = std::move(schema);
}

folly::F14FastMap<type::ProgramId, type::Schema>& SchemaRegistry::getSchemas() {
  static folly::Indestructible<folly::F14FastMap<type::ProgramId, type::Schema>>
      schemas;
  return *schemas;
}

#else
void SchemaRegistry::registerSchema(std::string_view) {}
#endif
} // namespace apache::thrift
