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

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/thrift/gen-cpp2/type_system_types.h>

#include <folly/container/F14Map.h>

#include <memory>

// WARNING: This code is highly experimental.
// DO NOT USE for any production code.
namespace apache::thrift::type_system {

class TypeSystemBuilder {
 public:
  void addType(
      Uri,
      SerializableStructDefinition,
      std::optional<SerializableThriftSourceInfo> = std::nullopt);

  void addType(
      Uri,
      SerializableUnionDefinition,
      std::optional<SerializableThriftSourceInfo> = std::nullopt);

  void addType(
      Uri,
      SerializableEnumDefinition,
      std::optional<SerializableThriftSourceInfo> = std::nullopt);

  void addType(
      Uri,
      SerializableOpaqueAliasDefinition,
      std::optional<SerializableThriftSourceInfo> = std::nullopt);

  void addTypes(SerializableTypeSystem);

  std::unique_ptr<TypeSystem> build() &&;

 private:
  struct DefinitionEntry {
    SerializableTypeDefinition definition;
    std::optional<SerializableThriftSourceInfo> sourceInfo;
  };
  folly::F14FastMap<Uri, DefinitionEntry> definitions_;

  void tryEmplace(Uri, DefinitionEntry&&);
};

} // namespace apache::thrift::type_system
