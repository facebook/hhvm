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

#include <memory>

#include <folly/memory/not_null.h>

#include <thrift/lib/cpp2/dynamic/PruneOptions.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/thrift/gen-cpp2/type_system_types.h>

namespace apache::thrift::type_system {

/**
 * SerializableTypeSystemBuilder provides a builder API for constructing
 * SerializableTypeSystem from the type system with selective definitions.
 * It will also include source information when using `withSourceInfo`.
 *
 * Example:
 *     // Thrift
 *     package "facebook.com/thrift/test"
 *
 *     struct MyStruct {
 *       1: MyUnion field1;
 *     }
 *
 *     union MyUnion {
 *       1: i32 field1;
 *     }
 *
 *     // C++
 *     auto r = SchemaRegistry::get();
 *     auto builder = SerializableTypeSystemBuilder::withoutSourceInfo(r);
 *     // Add both MyStruct and MyUnion definition
 *     builder.addDefinition("facebook.com/thrift/test/MyStruct");
 *     auto sts = std::move(builder).build();
 *     EXPECT_TRUE(sts.types().contains("facebook.com/thrift/test/MyStruct"));
 *     EXPECT_TRUE(sts.types().contains("facebook.com/thrift/test/MyUnion"));
 */
class SerializableTypeSystemBuilder {
 public:
  static SerializableTypeSystemBuilder withSourceInfo(
      const TypeSystem& typeSystem) {
    return SerializableTypeSystemBuilder(typeSystem, true /* withSourceInfo */);
  }
  static SerializableTypeSystemBuilder withoutSourceInfo(
      const TypeSystem& typeSystem) {
    return SerializableTypeSystemBuilder(
        typeSystem, false /* withSourceInfo */);
  }

  /**
   * Add a definition to the type system.
   *
   * Throws:
   *   - InvalidTypeError if the type or dependent types are not defined in
   * the type system.
   */
  void addDefinition(UriView uri);
  std::unique_ptr<SerializableTypeSystem> build() &&;

  /**
   * Builds a SerializableTypeSystem containing only the specified root types
   * and their transitive dependencies from the source TypeSystem.
   *
   * This is a convenience wrapper that creates a TypeSystemBuilder, adds all
   * root definitions, and builds the result in a single call.
   */
  static std::unique_ptr<SerializableTypeSystem> buildPrunedFrom(
      const TypeSystem& source,
      std::span<const UriView> rootUris,
      PruneOptions options = {});

  /**
   * Builds a SerializableTypeSystem containing only the specified root types
   * and their transitive dependencies from the provided TypeSystem.
   *
   * Note: See the URI-based `buildPrunedFrom` overload for more details.
   * This version takes in DefinitionRef objects instead of URIs.
   */
  static std::unique_ptr<SerializableTypeSystem> buildPrunedFrom(
      const TypeSystem& source,
      std::span<const DefinitionRef> rootDefs,
      PruneOptions options = {});

 private:
  SerializableTypeSystemBuilder(
      const TypeSystem& typeSystem, bool withSourceInfo)
      : typeSystem_(&typeSystem), withSourceInfo_(withSourceInfo) {}

  std::vector<SerializableFieldDefinition> toSerializableField(
      std::span<const FieldDefinition> fields);

  bool serializeDefinition(DefinitionRef ref);

  folly::not_null<const TypeSystem*> typeSystem_;
  bool withSourceInfo_;
  SerializableTypeSystem serializableTypeSystem_;
};

} // namespace apache::thrift::type_system
