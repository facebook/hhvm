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

#include <string_view>

#include <folly/container/F14Map.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#if __has_include(<thrift/lib/thrift/gen-cpp2/schema_types.h>)
#define FBTHRIFT_HAS_SCHEMA
#include <thrift/lib/thrift/gen-cpp2/schema_types.h>
#endif

namespace apache::thrift {

class SchemaRegistry {
 public:
  static void registerSchema(std::string_view data);

#ifdef FBTHRIFT_HAS_SCHEMA
  struct Iter {
    auto begin() const { return SchemaRegistry::getSchemas().begin(); }
    auto end() const { return SchemaRegistry::getSchemas().end(); }
  };
  static Iter iter() { return {}; }

 private:
  static folly::F14FastMap<type::ProgramId, type::Schema>& getSchemas();
#endif
};

} // namespace apache::thrift
