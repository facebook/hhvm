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

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/thrift/gen-cpp2/type_system_types.h>

namespace apache::thrift::type_system {

/**
 * SerializableTypeSystemBuilder provides a builder API for constructing
 * SerializableTypeSystem from the type system with selective definitions.
 * It will also include source information if SourceIndexedTypeSystem is
 * provided using `withSourceInfo` static method.
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
      const SourceIndexedTypeSystem& typeSystem) {
    return SerializableTypeSystemBuilder(typeSystem, &typeSystem);
  }
  static SerializableTypeSystemBuilder withoutSourceInfo(
      const TypeSystem& typeSystem) {
    return SerializableTypeSystemBuilder(typeSystem, nullptr);
  }

  /**
   * Add a definition to the type system.
   *
   * Throws:
   *   - InvalidTypeError if the type or dependent types are not defined in the
   *     type system.
   */
  void addDefinition(UriView uri);
  std::unique_ptr<SerializableTypeSystem> build() &&;

 private:
  SerializableTypeSystemBuilder(
      const TypeSystem& typeSystem,
      const SourceIndexedTypeSystem* sourceIndexedTypeSystem)
      : typeSystem_(&typeSystem),
        sourceIndexedTypeSystem_(sourceIndexedTypeSystem) {}

  std::vector<SerializableFieldDefinition> toSerializableField(
      folly::span<const FieldDefinition> fields);

  void addDefinition(DefinitionRef ref);
  void addType(TypeRef ref);
  void addAnnotations(const detail::RawAnnotations& annotations);

  folly::not_null<const TypeSystem*> typeSystem_;
  const SourceIndexedTypeSystem* sourceIndexedTypeSystem_;
  SerializableTypeSystem serializableTypeSystem_;
};

} // namespace apache::thrift::type_system
