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

#include <thrift/lib/cpp2/protocol/SchemaValidator.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/dynamic/TypeId.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

using namespace apache::thrift;
using namespace apache::thrift::type_system;

// Test outcome: Maybe - blob conforms to schema
TEST(SchemaValidatorTest, ValidBlobConformsToSchema) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  // Define a simple struct: struct Person { 1: string name; 2: i32 age; }
  builder.addType(
      "facebook.com/thrift/test/Person",
      def::Struct({
          def::Field(def::Identity(1, "name"), def::Optional, TypeIds::String),
          def::Field(def::Identity(2, "age"), def::Optional, TypeIds::I32),
      }));

  auto typeSystem = std::move(builder).build();
  auto personType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Person"));
  ASSERT_TRUE(personType.has_value());
  auto typeRef = TypeRef::fromDefinition(*personType);

  // Serialize a conforming struct
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("name", protocol::TType::T_STRING, 1);
  writer.writeString("Alice");
  writer.writeFieldEnd();
  writer.writeFieldBegin("age", protocol::TType::T_I32, 2);
  writer.writeI32(30);
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  // Validate - should return Maybe
  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::Maybe);
}

// Test outcome: MaybeWithUnknownFields - blob has extra fields
TEST(SchemaValidatorTest, BlobWithUnknownFields) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  // Define a struct with only field 1: struct Person { 1: string name; }
  builder.addType(
      "facebook.com/thrift/test/Person",
      def::Struct({
          def::Field(def::Identity(1, "name"), def::Optional, TypeIds::String),
      }));

  auto typeSystem = std::move(builder).build();
  auto personType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Person"));
  ASSERT_TRUE(personType.has_value());
  auto typeRef = TypeRef::fromDefinition(*personType);

  // Serialize a struct with an extra field (field 2)
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("name", protocol::TType::T_STRING, 1);
  writer.writeString("Alice");
  writer.writeFieldEnd();
  writer.writeFieldBegin("age", protocol::TType::T_I32, 2); // Unknown field
  writer.writeI32(30);
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  // Validate - should return MaybeWithUnknownFields
  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::MaybeWithUnknownFields);
}

// Test outcome: No - type mismatch
TEST(SchemaValidatorTest, TypeMismatchDetected) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  // Define a struct: struct Person { 1: string name; 2: i32 age; }
  builder.addType(
      "facebook.com/thrift/test/Person",
      def::Struct({
          def::Field(def::Identity(1, "name"), def::Optional, TypeIds::String),
          def::Field(def::Identity(2, "age"), def::Optional, TypeIds::I32),
      }));

  auto typeSystem = std::move(builder).build();
  auto personType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Person"));
  ASSERT_TRUE(personType.has_value());
  auto typeRef = TypeRef::fromDefinition(*personType);

  // Serialize with wrong type for field 2 (bool instead of i32)
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("name", protocol::TType::T_STRING, 1);
  writer.writeString("Alice");
  writer.writeFieldEnd();
  writer.writeFieldBegin("age", protocol::TType::T_BOOL, 2); // Wrong type!
  writer.writeBool(true);
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  // Validate - should return No
  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::No);
}

// Test nested structures - conforming
TEST(SchemaValidatorTest, NestedStructureConforming) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  // Define nested structs
  builder.addType(
      "facebook.com/thrift/test/Address",
      def::Struct({
          def::Field(def::Identity(1, "city"), def::Optional, TypeIds::String),
          def::Field(def::Identity(2, "zip"), def::Optional, TypeIds::I32),
      }));

  builder.addType(
      "facebook.com/thrift/test/Person",
      def::Struct({
          def::Field(def::Identity(1, "name"), def::Optional, TypeIds::String),
          def::Field(
              def::Identity(2, "address"),
              def::Optional,
              TypeIds::uri("facebook.com/thrift/test/Address")),
      }));

  auto typeSystem = std::move(builder).build();
  auto personType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Person"));
  ASSERT_TRUE(personType.has_value());
  auto typeRef = TypeRef::fromDefinition(*personType);

  // Serialize nested structure
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr); // Person
  writer.writeFieldBegin("name", protocol::TType::T_STRING, 1);
  writer.writeString("Alice");
  writer.writeFieldEnd();
  writer.writeFieldBegin("address", protocol::TType::T_STRUCT, 2);
  writer.writeStructBegin(nullptr); // Address
  writer.writeFieldBegin("city", protocol::TType::T_STRING, 1);
  writer.writeString("Seattle");
  writer.writeFieldEnd();
  writer.writeFieldBegin("zip", protocol::TType::T_I32, 2);
  writer.writeI32(98101);
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  // Validate - should return Maybe
  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::Maybe);
}

// Test nested structures with unknown field in nested struct
TEST(SchemaValidatorTest, NestedStructureWithUnknownField) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  // Define nested structs - Address only has field 1
  builder.addType(
      "facebook.com/thrift/test/Address",
      def::Struct({
          def::Field(def::Identity(1, "city"), def::Optional, TypeIds::String),
      }));

  builder.addType(
      "facebook.com/thrift/test/Person",
      def::Struct({
          def::Field(def::Identity(1, "name"), def::Optional, TypeIds::String),
          def::Field(
              def::Identity(2, "address"),
              def::Optional,
              TypeIds::uri("facebook.com/thrift/test/Address")),
      }));

  auto typeSystem = std::move(builder).build();
  auto personType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Person"));
  ASSERT_TRUE(personType.has_value());
  auto typeRef = TypeRef::fromDefinition(*personType);

  // Serialize nested structure with unknown field in nested struct
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr); // Person
  writer.writeFieldBegin("name", protocol::TType::T_STRING, 1);
  writer.writeString("Alice");
  writer.writeFieldEnd();
  writer.writeFieldBegin("address", protocol::TType::T_STRUCT, 2);
  writer.writeStructBegin(nullptr); // Address
  writer.writeFieldBegin("city", protocol::TType::T_STRING, 1);
  writer.writeString("Seattle");
  writer.writeFieldEnd();
  writer.writeFieldBegin("zip", protocol::TType::T_I32, 2); // Unknown!
  writer.writeI32(98101);
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  // Validate - should return MaybeWithUnknownFields
  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::MaybeWithUnknownFields);
}

// Test container types - list
TEST(SchemaValidatorTest, ListTypeConforming) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  builder.addType(
      "facebook.com/thrift/test/Numbers",
      def::Struct({
          def::Field(
              def::Identity(1, "values"),
              def::Optional,
              TypeIds::list(TypeIds::I32)),
      }));

  auto typeSystem = std::move(builder).build();
  auto numbersType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Numbers"));
  ASSERT_TRUE(numbersType.has_value());
  auto typeRef = TypeRef::fromDefinition(*numbersType);

  // Serialize struct with list
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("values", protocol::TType::T_LIST, 1);
  writer.writeListBegin(protocol::TType::T_I32, 3);
  writer.writeI32(1);
  writer.writeI32(2);
  writer.writeI32(3);
  writer.writeListEnd();
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  // Validate - should return Maybe
  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::Maybe);
}

// Test container with wrong element type
TEST(SchemaValidatorTest, ListTypeWithWrongElementType) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  builder.addType(
      "facebook.com/thrift/test/Numbers",
      def::Struct({
          def::Field(
              def::Identity(1, "values"),
              def::Optional,
              TypeIds::list(TypeIds::I32)),
      }));

  auto typeSystem = std::move(builder).build();
  auto numbersType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Numbers"));
  ASSERT_TRUE(numbersType.has_value());
  auto typeRef = TypeRef::fromDefinition(*numbersType);

  // Serialize struct with list of wrong type (strings instead of i32)
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("values", protocol::TType::T_LIST, 1);
  writer.writeListBegin(protocol::TType::T_STRING, 2); // Wrong type!
  writer.writeString("one");
  writer.writeString("two");
  writer.writeListEnd();
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  // Validate - should return No
  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::No);
}

// Test primitive type validation
TEST(SchemaValidatorTest, PrimitiveTypeConforming) {
  auto typeRef = TypeSystem::I32();

  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeI32(42);
  auto serialized = queue.move();

  // Should succeed for primitive types
  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::Maybe);
}

// Test union types
TEST(SchemaValidatorTest, UnionTypeConforming) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  // Define a union: union Result { 1: string success; 2: i32 errorCode; }
  builder.addType(
      "facebook.com/thrift/test/Result",
      def::Union({
          def::Field(
              def::Identity(1, "success"), def::Optional, TypeIds::String),
          def::Field(
              def::Identity(2, "errorCode"), def::Optional, TypeIds::I32),
      }));

  builder.addType(
      "facebook.com/thrift/test/Response",
      def::Struct({
          def::Field(
              def::Identity(1, "result"),
              def::Optional,
              TypeIds::uri("facebook.com/thrift/test/Result")),
      }));

  auto typeSystem = std::move(builder).build();
  auto responseType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Response"));
  ASSERT_TRUE(responseType.has_value());
  auto typeRef = TypeRef::fromDefinition(*responseType);

  // Serialize struct containing union
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr); // Response
  writer.writeFieldBegin("result", protocol::TType::T_STRUCT, 1);
  writer.writeStructBegin(nullptr); // Result (union)
  writer.writeFieldBegin("success", protocol::TType::T_STRING, 1);
  writer.writeString("OK");
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::Maybe);
}

// Test set container type
TEST(SchemaValidatorTest, SetTypeConforming) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  builder.addType(
      "facebook.com/thrift/test/Tags",
      def::Struct({
          def::Field(
              def::Identity(1, "tags"),
              def::Optional,
              TypeIds::set(TypeIds::String)),
      }));

  auto typeSystem = std::move(builder).build();
  auto tagsType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Tags"));
  ASSERT_TRUE(tagsType.has_value());
  auto typeRef = TypeRef::fromDefinition(*tagsType);

  // Serialize struct with set
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("tags", protocol::TType::T_SET, 1);
  writer.writeSetBegin(protocol::TType::T_STRING, 2);
  writer.writeString("tag1");
  writer.writeString("tag2");
  writer.writeSetEnd();
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::Maybe);
}

// Test map container type
TEST(SchemaValidatorTest, MapTypeConforming) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  builder.addType(
      "facebook.com/thrift/test/Metadata",
      def::Struct({
          def::Field(
              def::Identity(1, "data"),
              def::Optional,
              TypeIds::map(TypeIds::String, TypeIds::I32)),
      }));

  auto typeSystem = std::move(builder).build();
  auto metadataType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Metadata"));
  ASSERT_TRUE(metadataType.has_value());
  auto typeRef = TypeRef::fromDefinition(*metadataType);

  // Serialize struct with map
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("data", protocol::TType::T_MAP, 1);
  writer.writeMapBegin(protocol::TType::T_STRING, protocol::TType::T_I32, 2);
  writer.writeString("key1");
  writer.writeI32(100);
  writer.writeString("key2");
  writer.writeI32(200);
  writer.writeMapEnd();
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::Maybe);
}

// Test enum type
TEST(SchemaValidatorTest, EnumTypeConforming) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  // Define an enum: enum Status { PENDING = 0, ACTIVE = 1, DONE = 2 }
  builder.addType(
      "facebook.com/thrift/test/Status",
      def::Enum({
          def::EnumValue("PENDING", 0),
          def::EnumValue("ACTIVE", 1),
          def::EnumValue("DONE", 2),
      }));

  builder.addType(
      "facebook.com/thrift/test/Task",
      def::Struct({
          def::Field(def::Identity(1, "name"), def::Optional, TypeIds::String),
          def::Field(
              def::Identity(2, "status"),
              def::Optional,
              TypeIds::uri("facebook.com/thrift/test/Status")),
      }));

  auto typeSystem = std::move(builder).build();
  auto taskType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Task"));
  ASSERT_TRUE(taskType.has_value());
  auto typeRef = TypeRef::fromDefinition(*taskType);

  // Serialize struct with enum field (enums are i32)
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("name", protocol::TType::T_STRING, 1);
  writer.writeString("My Task");
  writer.writeFieldEnd();
  writer.writeFieldBegin("status", protocol::TType::T_I32, 2);
  writer.writeI32(1); // ACTIVE
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::Maybe);
}

// Test Any type
TEST(SchemaValidatorTest, AnyTypeConforming) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  builder.addType(
      "facebook.com/thrift/test/Container",
      def::Struct({
          def::Field(def::Identity(1, "payload"), def::Optional, TypeIds::Any),
      }));

  auto typeSystem = std::move(builder).build();
  auto containerType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Container"));
  ASSERT_TRUE(containerType.has_value());
  auto typeRef = TypeRef::fromDefinition(*containerType);

  // Serialize struct with Any field (Any is serialized as a struct)
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr); // Container
  writer.writeFieldBegin("payload", protocol::TType::T_STRUCT, 1);
  writer.writeStructBegin(nullptr); // Any payload (some struct)
  writer.writeFieldBegin("data", protocol::TType::T_STRING, 1);
  writer.writeString("arbitrary data");
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::Maybe);
}

// Test opaque alias type
TEST(SchemaValidatorTest, OpaqueAliasTypeConforming) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  // Define an opaque alias: typedef i64 Timestamp
  builder.addType(
      "facebook.com/thrift/test/Timestamp", def::OpaqueAlias(TypeIds::I64));

  builder.addType(
      "facebook.com/thrift/test/Event",
      def::Struct({
          def::Field(def::Identity(1, "name"), def::Optional, TypeIds::String),
          def::Field(
              def::Identity(2, "timestamp"),
              def::Optional,
              TypeIds::uri("facebook.com/thrift/test/Timestamp")),
      }));

  auto typeSystem = std::move(builder).build();
  auto eventType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Event"));
  ASSERT_TRUE(eventType.has_value());
  auto typeRef = TypeRef::fromDefinition(*eventType);

  // Serialize struct with opaque alias field (should be i64)
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("name", protocol::TType::T_STRING, 1);
  writer.writeString("Event1");
  writer.writeFieldEnd();
  writer.writeFieldBegin("timestamp", protocol::TType::T_I64, 2);
  writer.writeI64(1234567890123);
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::Maybe);
}

// Test opaque alias with wrong underlying type
TEST(SchemaValidatorTest, OpaqueAliasTypeMismatch) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  // Define an opaque alias: typedef i64 Timestamp
  builder.addType(
      "facebook.com/thrift/test/Timestamp", def::OpaqueAlias(TypeIds::I64));

  builder.addType(
      "facebook.com/thrift/test/Event",
      def::Struct({
          def::Field(
              def::Identity(1, "timestamp"),
              def::Optional,
              TypeIds::uri("facebook.com/thrift/test/Timestamp")),
      }));

  auto typeSystem = std::move(builder).build();
  auto eventType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Event"));
  ASSERT_TRUE(eventType.has_value());
  auto typeRef = TypeRef::fromDefinition(*eventType);

  // Serialize with wrong type (string instead of i64)
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("timestamp", protocol::TType::T_STRING, 1); // Wrong!
  writer.writeString("not a timestamp");
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto serialized = queue.move();

  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::No);
}

// Test blob not fully consumed - i64 serialized, validated as i32
TEST(SchemaValidatorTest, BlobNotFullyConsumed_I64AsI32) {
  auto typeRef = TypeSystem::I32();

  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  // Write an i64 value (8 bytes in binary, variable in compact)
  writer.writeI64(0x123456789ABCDEF0LL);
  auto serialized = queue.move();

  // Validating as i32 should fail because the blob won't be fully consumed
  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::No);
}

// Test blob not fully consumed - extra data after struct
TEST(SchemaValidatorTest, BlobNotFullyConsumed_ExtraDataAfterStruct) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  builder.addType(
      "facebook.com/thrift/test/Simple",
      def::Struct({
          def::Field(def::Identity(1, "value"), def::Optional, TypeIds::I32),
      }));

  auto typeSystem = std::move(builder).build();
  auto simpleType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Simple"));
  ASSERT_TRUE(simpleType.has_value());
  auto typeRef = TypeRef::fromDefinition(*simpleType);

  // Serialize a valid struct followed by extra data
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("value", protocol::TType::T_I32, 1);
  writer.writeI32(42);
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  // Write extra garbage data
  writer.writeI32(999);
  auto serialized = queue.move();

  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::No);
}

// Test blob not fully consumed - extra data after list
TEST(SchemaValidatorTest, BlobNotFullyConsumed_ExtraDataAfterList) {
  using def = TypeSystemBuilder::DefinitionHelper;
  TypeSystemBuilder builder;

  builder.addType(
      "facebook.com/thrift/test/Numbers",
      def::Struct({
          def::Field(
              def::Identity(1, "values"),
              def::Optional,
              TypeIds::list(TypeIds::I32)),
      }));

  auto typeSystem = std::move(builder).build();
  auto numbersType =
      typeSystem->getUserDefinedType(Uri("facebook.com/thrift/test/Numbers"));
  ASSERT_TRUE(numbersType.has_value());
  auto typeRef = TypeRef::fromDefinition(*numbersType);

  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin(nullptr);
  writer.writeFieldBegin("values", protocol::TType::T_LIST, 1);
  writer.writeListBegin(protocol::TType::T_I32, 2);
  writer.writeI32(1);
  writer.writeI32(2);
  writer.writeListEnd();
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  // Write extra garbage data
  writer.writeI32(999);
  auto serialized = queue.move();

  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::No);
}

// Test string validation
TEST(SchemaValidatorTest, StringTypeConforming) {
  auto typeRef = TypeSystem::String();

  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeString("hello world");
  auto serialized = queue.move();

  auto result = validateBlob<CompactProtocolReader>(*serialized, typeRef);
  EXPECT_EQ(result, SchemaValidationResult::Maybe);
}
