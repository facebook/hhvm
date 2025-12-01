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
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/Union.h>

#include <gtest/gtest.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

using namespace apache::thrift;
using namespace apache::thrift::dynamic;
using namespace apache::thrift::type_system;

using def = type_system::TypeSystemBuilder::DefinitionHelper;
using type_system::TypeIds;

class UnionTest : public ::testing::Test {
 protected:
  std::unique_ptr<type_system::TypeSystem> typeSystem;

  UnionTest() {
    type_system::TypeSystemBuilder builder;

    // Create a simple union with three fields:
    // 1: i32 id
    // 2: string name
    // 3: bool active

    builder.addType(
        "test.TestUnion",
        def::Union({
            def::Field(def::Identity(1, "id"), def::Optional, TypeIds::I32),
            def::Field(
                def::Identity(2, "name"), def::Optional, TypeIds::String),
            def::Field(
                def::Identity(3, "active"), def::Optional, TypeIds::Bool),
        }));

    typeSystem = std::move(builder).build();
  }

  const UnionNode& unionNode() {
    return typeSystem->getUserDefinedTypeOrThrow("test.TestUnion").asUnion();
  }
};

TEST_F(UnionTest, EmptyUnion) {
  Union unionValue(unionNode());

  EXPECT_TRUE(unionValue.isEmpty());
  EXPECT_FALSE(unionValue.activeField().valid());
}

TEST_F(UnionTest, SetAndGetFieldByName) {
  Union unionValue(unionNode());

  // Set field by name
  unionValue.setField("id", DynamicValue::makeI32(42));

  EXPECT_FALSE(unionValue.isEmpty());
  EXPECT_TRUE(unionValue.hasField("id"));
  EXPECT_FALSE(unionValue.hasField("name"));
  EXPECT_FALSE(unionValue.hasField("active"));

  auto id = unionValue.getField("id");
  EXPECT_EQ(id.asI32(), 42);
}

TEST_F(UnionTest, SetAndGetFieldById) {
  Union unionValue(unionNode());

  // Set field by ID
  unionValue.setField(
      Union::FieldId{2}, DynamicValue::makeString(String("test")));

  EXPECT_FALSE(unionValue.isEmpty());
  EXPECT_FALSE(unionValue.hasField(Union::FieldId{1}));
  EXPECT_TRUE(unionValue.hasField(Union::FieldId{2}));
  EXPECT_FALSE(unionValue.hasField(Union::FieldId{3}));

  auto name = unionValue.getField(Union::FieldId{2});
  EXPECT_EQ(name.asString().view(), "test");
}

TEST_F(UnionTest, SwitchActiveField) {
  Union unionValue(unionNode());

  // Set first field
  unionValue.setField("id", DynamicValue::makeI32(42));
  EXPECT_TRUE(unionValue.hasField("id"));
  EXPECT_EQ(unionValue.getField("id").asI32(), 42);

  // Switch to different field
  unionValue.setField("name", DynamicValue::makeString(String("hello")));
  EXPECT_FALSE(unionValue.hasField("id"));
  EXPECT_TRUE(unionValue.hasField("name"));
  EXPECT_EQ(unionValue.getField("name").asString().view(), "hello");
}

TEST_F(UnionTest, Clear) {
  Union unionValue(unionNode());

  unionValue.setField("active", DynamicValue::makeBool(true));
  EXPECT_FALSE(unionValue.isEmpty());

  unionValue.clear();
  EXPECT_TRUE(unionValue.isEmpty());
  EXPECT_FALSE(unionValue.hasField("active"));
}

TEST_F(UnionTest, GetInactiveFieldThrows) {
  Union unionValue(unionNode());

  unionValue.setField("id", DynamicValue::makeI32(42));

  EXPECT_THROW(unionValue.getField("name"), std::runtime_error);
  EXPECT_THROW(unionValue.getField("active"), std::runtime_error);
}

TEST_F(UnionTest, GetUnknownFieldThrows) {
  Union unionValue(unionNode());

  EXPECT_THROW(unionValue.getField("unknown"), std::out_of_range);
  EXPECT_THROW(unionValue.getField(Union::FieldId{99}), std::out_of_range);
}

TEST_F(UnionTest, Equality) {
  Union union1(unionNode());
  Union union2(unionNode());

  // Both empty
  EXPECT_EQ(union1, union2);

  // Set same field to same value
  union1.setField("id", DynamicValue::makeI32(42));
  union2.setField("id", DynamicValue::makeI32(42));
  EXPECT_EQ(union1, union2);

  // Different values
  union2.setField("id", DynamicValue::makeI32(100));
  EXPECT_NE(union1, union2);

  // Different active fields
  union2.setField("name", DynamicValue::makeString(String("test")));
  EXPECT_NE(union1, union2);
}

TEST_F(UnionTest, CopyConstructor) {
  Union union1(unionNode());
  union1.setField("name", DynamicValue::makeString(String("original")));

  Union union2(union1);

  EXPECT_EQ(union1, union2);
  EXPECT_TRUE(union2.hasField("name"));
  EXPECT_EQ(union2.getField("name").asString().view(), "original");
}

TEST_F(UnionTest, MoveConstructor) {
  Union union1(unionNode());
  union1.setField("id", DynamicValue::makeI32(42));

  Union union2(std::move(union1));

  EXPECT_TRUE(union2.hasField("id"));
  EXPECT_EQ(union2.getField("id").asI32(), 42);
}

TEST_F(UnionTest, SerializationRoundTripEmpty) {
  // Create an empty union
  Union value(unionNode());
  EXPECT_TRUE(value.isEmpty());

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serialize(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserialize(reader, unionNode(), nullptr);

  // Verify
  EXPECT_EQ(value, deserValue);
  EXPECT_TRUE(deserValue.isEmpty());
}

TEST_F(UnionTest, SerializationRoundTripWithValue) {
  // Create a union with active field
  Union value(unionNode());
  value.setField("name", DynamicValue::makeString(String("test union")));

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serialize(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserialize(reader, unionNode(), nullptr);

  // Verify
  EXPECT_EQ(value, deserValue);
  EXPECT_FALSE(deserValue.isEmpty());
  EXPECT_TRUE(deserValue.hasField("name"));
  EXPECT_EQ(deserValue.getField("name").asString().view(), "test union");
}

TEST_F(UnionTest, SerializationRoundTripBool) {
  Union value(unionNode());
  value.setField("active", DynamicValue::makeBool(true));

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serialize(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserialize(reader, unionNode(), nullptr);

  // Verify
  EXPECT_EQ(value, deserValue);
  EXPECT_TRUE(deserValue.hasField("active"));
  EXPECT_EQ(deserValue.getField("active").asBool(), true);
}

TEST_F(UnionTest, SerializationRoundTripI32) {
  Union value(unionNode());
  value.setField("id", DynamicValue::makeI32(12345));

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serialize(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserialize(reader, unionNode(), nullptr);

  // Verify
  EXPECT_EQ(value, deserValue);
  EXPECT_TRUE(deserValue.hasField("id"));
  EXPECT_EQ(deserValue.getField("id").asI32(), 12345);
}
