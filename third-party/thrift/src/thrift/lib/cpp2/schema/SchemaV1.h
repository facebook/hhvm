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

#include <folly/Portability.h>

#include <thrift/lib/thrift/gen-cpp2/schema_types.h>
#define THRIFT_SCHEMA_AVAILABLE

namespace apache::thrift::schema {
struct DefinitionsSchema {
  // Which definitions are of interest (e.g. the root service).
  // Thrift-generated handlers only have a single entry in this vector,
  // but multiplexed and custom handlers may have several.
  std::vector<type::DefinitionKey> definitions;
  // The schema to look those definitions up in.
  type::Schema schema;
};
} // namespace apache::thrift::schema
