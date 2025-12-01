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

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/Serialization.h>
#include <thrift/lib/cpp2/dynamic/Struct.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift::dynamic {
namespace {

using def = type_system::TypeSystemBuilder::DefinitionHelper;
using type_system::TypeIds;

// Helper struct to hold TypeSystem and provide access to StructNode
struct StructTest : ::testing::Test {
  std::unique_ptr<type_system::TypeSystem> typeSystem;

  StructTest() {
    type_system::TypeSystemBuilder builder;

    // Create a simple struct with three fields:
    // 1: i32 id
    // 2: i64 count
    // 3: bool active (optional)

    builder.addType(
        "test.TestStruct",
        def::Struct({
            def::Field(
                def::Identity(1, "id"), def::AlwaysPresent, TypeIds::I32),
            def::Field(
                def::Identity(2, "count"), def::AlwaysPresent, TypeIds::I64),
            def::Field(
                def::Identity(3, "active"), def::Optional, TypeIds::Bool),
        }));

    typeSystem = std::move(builder).build();
  }

  const type_system::StructNode& structNode() {
    return typeSystem->getUserDefinedTypeOrThrow("test.TestStruct").asStruct();
  }
};

TEST_F(StructTest, CreateEmpty) {
  auto structValue = makeStruct(structNode());

  EXPECT_TRUE(
      structValue.type().isEqualIdentityTo(type_system::TypeRef(structNode())));
}

TEST_F(StructTest, SetAndGetField) {
  auto structValue = makeStruct(structNode());

  // Test by name
  structValue.setField("id", DynamicValue::makeI32(42));
  auto id = structValue.getField("id");
  ASSERT_TRUE(id.has_value());
  EXPECT_EQ(id->asI32(), 42);

  // Test by ID
  structValue.setField(FieldId{2}, DynamicValue::makeI64(100));
  auto count = structValue.getField(FieldId{2});
  ASSERT_TRUE(count.has_value());
  EXPECT_EQ(count->asI64(), 100);
}

TEST_F(StructTest, HasField) {
  auto structValue = makeStruct(structNode());

  // Initially, all optional fields should be null
  EXPECT_TRUE(structValue.hasField("id"));
  EXPECT_FALSE(structValue.hasField("active"));

  // Test by ID
  EXPECT_TRUE(structValue.hasField(FieldId{1}));
  EXPECT_FALSE(structValue.hasField(FieldId{3}));

  // After setting a field
  structValue.setField("active", DynamicValue::makeBool(true));
  EXPECT_TRUE(structValue.hasField("id"));
  EXPECT_TRUE(structValue.hasField("active"));

  // Test getField returns empty optional for unset optional field
  auto unsetField = makeStruct(structNode()).getField("active");
  EXPECT_FALSE(unsetField.has_value());

  // Test getField returns value for set optional field
  auto setField = structValue.getField("active");
  ASSERT_TRUE(setField.has_value());
  EXPECT_TRUE(setField->asBool());
}

TEST_F(StructTest, FieldNotFound) {
  auto structValue = makeStruct(structNode());

  EXPECT_THROW(structValue.getField("nonexistent"), std::out_of_range);
  EXPECT_THROW(structValue.getField(FieldId{999}), std::out_of_range);
  EXPECT_THROW(
      structValue.setField("nonexistent", DynamicValue::makeI32(1)),
      std::out_of_range);
}

TEST_F(StructTest, TypeMismatch) {
  auto structValue = makeStruct(structNode());

  // Try to set an i32 field with a bool value
  EXPECT_THROW(
      structValue.setField("id", DynamicValue::makeBool(true)),
      std::runtime_error);
}

TEST_F(StructTest, Equality) {
  auto struct1 = makeStruct(structNode());
  struct1.setField("id", DynamicValue::makeI32(42));

  auto struct2 = makeStruct(structNode());
  struct2.setField("id", DynamicValue::makeI32(42));

  EXPECT_TRUE(struct1 == struct2);

  // Change a field
  struct2.setField("id", DynamicValue::makeI32(99));
  EXPECT_FALSE(struct1 == struct2);
}

TEST_F(StructTest, Clone) {
  auto struct1 = makeStruct(structNode());
  struct1.setField("id", DynamicValue::makeI32(42));

  // Clone through copy constructor
  auto struct2 = struct1;

  // They should be equal
  EXPECT_TRUE(struct1 == struct2);

  // Modify the clone
  struct2.setField("id", DynamicValue::makeI32(99));

  // Original should be unchanged
  EXPECT_EQ(struct1.getField("id")->asI32(), 42);
  EXPECT_EQ(struct2.getField("id")->asI32(), 99);
}

TEST_F(StructTest, SerializationRoundTrip) {
  // Create a DynamicValue with a struct
  auto value = DynamicValue::makeDefault(type_system::TypeRef(structNode()));
  auto& structValue = value.asStruct();
  structValue.setField("id", DynamicValue::makeI32(42));
  structValue.setField("count", DynamicValue::makeI64(1000));
  structValue.setField("active", DynamicValue::makeBool(true));

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serializeValue(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue =
      deserializeValue(reader, type_system::TypeRef(structNode()));

  // Verify
  EXPECT_EQ(value, deserValue);
  auto& deserStruct = deserValue.asStruct();
  EXPECT_EQ(deserStruct.getField("id")->asI32(), 42);
  EXPECT_EQ(deserStruct.getField("count")->asI64(), 1000);
  EXPECT_EQ(deserStruct.getField("active")->asBool(), true);
}

TEST_F(StructTest, SerializationWithNullFields) {
  // Create a DynamicValue with a struct with some null fields
  auto value = DynamicValue::makeDefault(type_system::TypeRef(structNode()));
  auto& structValue = value.asStruct();
  structValue.setField("id", DynamicValue::makeI32(99));
  // count and active fields are left null

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serializeValue(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue =
      deserializeValue(reader, type_system::TypeRef(structNode()));

  // Verify
  auto& deserStruct = deserValue.asStruct();
  EXPECT_EQ(deserStruct.getField("id")->asI32(), 99);
  EXPECT_TRUE(deserStruct.hasField("count"));
  EXPECT_EQ(deserStruct.getField("count")->asI64(), 0);
  EXPECT_FALSE(deserStruct.hasField("active"));
  EXPECT_FALSE(deserStruct.getField("active").has_value());
}

TEST_F(StructTest, ClearOptionalField) {
  auto structValue = makeStruct(structNode());

  // Set an optional field
  structValue.setField("active", DynamicValue::makeBool(true));
  EXPECT_TRUE(structValue.hasField("active"));
  ASSERT_TRUE(structValue.getField("active").has_value());
  EXPECT_TRUE(structValue.getField("active")->asBool());

  // Clear the optional field by name
  structValue.clearOptionalField("active");
  EXPECT_FALSE(structValue.hasField("active"));
  EXPECT_FALSE(structValue.getField("active").has_value());

  // Set it again
  structValue.setField("active", DynamicValue::makeBool(false));
  EXPECT_TRUE(structValue.hasField("active"));

  // Clear by field ID
  structValue.clearOptionalField(FieldId{3});
  EXPECT_FALSE(structValue.hasField("active"));

  // Try to clear a non-optional field - should throw
  EXPECT_THROW(structValue.clearOptionalField("id"), std::runtime_error);
  EXPECT_THROW(structValue.clearOptionalField(FieldId{1}), std::runtime_error);

  // Try to clear a nonexistent field - should throw
  EXPECT_THROW(
      structValue.clearOptionalField("nonexistent"), std::out_of_range);
  EXPECT_THROW(structValue.clearOptionalField(FieldId{999}), std::out_of_range);
}

} // namespace
} // namespace apache::thrift::dynamic
