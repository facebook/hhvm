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

#include <folly/Function.h>
#include <folly/SharedMutex.h>
#include <folly/container/F14Map.h>

namespace apache::thrift {

class BaseSchemaRegistry {
 public:
  // Access the global registry.
  static BaseSchemaRegistry& get();

  void registerSchema(
      std::string_view name, std::string_view data, std::string_view path);

  using Callback = folly::Function<void(std::string_view) const>;

 private:
  struct RawSchema {
    std::string_view data;
    std::string_view path;
  };
  folly::F14FastMap<std::string_view, RawSchema> rawSchemas_;
  bool accessed_;
  Callback insertCallback_;
  folly::SharedMutex mutex_;

  friend class SchemaRegistry;
};

} // namespace apache::thrift
