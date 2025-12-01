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

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/dynamic/Serialization.h>
#include <thrift/lib/cpp2/dynamic/detail/ConcreteSet.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift::dynamic {
namespace {

// Helper to create a set type
inline type_system::TypeRef::Set makeSetType(type_system::TypeRef elementType) {
  static type_system::detail::ContainerTypeCache cache;
  return type_system::TypeRef::Set::of(elementType, cache);
}

TEST(SetTest, InsertAndContains) {
  auto set = makeSet(makeSetType(type_system::TypeSystem::I32()));

  EXPECT_EQ(set.size(), 0);
  EXPECT_TRUE(set.isEmpty());

  EXPECT_TRUE(set.insert(DynamicValue::makeI32(10)));
  EXPECT_TRUE(set.insert(DynamicValue::makeI32(20)));
  EXPECT_TRUE(set.insert(DynamicValue::makeI32(30)));

  EXPECT_EQ(set.size(), 3);
  EXPECT_FALSE(set.isEmpty());

  EXPECT_TRUE(set.contains(DynamicValue::makeI32(10)));
  EXPECT_TRUE(set.contains(DynamicValue::makeI32(20)));
  EXPECT_TRUE(set.contains(DynamicValue::makeI32(30)));
  EXPECT_FALSE(set.contains(DynamicValue::makeI32(40)));
}

TEST(SetTest, InsertDuplicate) {
  auto set = makeSet(makeSetType(type_system::TypeSystem::I32()));

  // First insert returns true
  EXPECT_TRUE(set.insert(DynamicValue::makeI32(10)));
  EXPECT_EQ(set.size(), 1);

  // Duplicate insert returns false
  EXPECT_FALSE(set.insert(DynamicValue::makeI32(10)));
  EXPECT_EQ(set.size(), 1);
}

TEST(SetTest, Erase) {
  auto set = makeSet(makeSetType(type_system::TypeSystem::I32()));

  set.insert(DynamicValue::makeI32(10));
  set.insert(DynamicValue::makeI32(20));
  set.insert(DynamicValue::makeI32(30));

  EXPECT_EQ(set.size(), 3);

  // Erase existing element
  EXPECT_TRUE(set.erase(DynamicValue::makeI32(20)));
  EXPECT_EQ(set.size(), 2);
  EXPECT_FALSE(set.contains(DynamicValue::makeI32(20)));

  // Erase non-existing element
  EXPECT_FALSE(set.erase(DynamicValue::makeI32(40)));
  EXPECT_EQ(set.size(), 2);
}

TEST(SetTest, Clear) {
  auto set = makeSet(makeSetType(type_system::TypeSystem::I32()));

  set.insert(DynamicValue::makeI32(1));
  set.insert(DynamicValue::makeI32(2));
  set.insert(DynamicValue::makeI32(3));

  EXPECT_EQ(set.size(), 3);

  set.clear();

  EXPECT_EQ(set.size(), 0);
  EXPECT_TRUE(set.isEmpty());
}

TEST(SetTest, Reserve) {
  auto set = makeSet(makeSetType(type_system::TypeSystem::I32()));

  set.reserve(100);

  // Reserve shouldn't change the size
  EXPECT_EQ(set.size(), 0);

  // But we can still insert
  set.insert(DynamicValue::makeI32(42));
  EXPECT_EQ(set.size(), 1);
}

TEST(SetTest, Equality) {
  auto set1 = makeSet(makeSetType(type_system::TypeSystem::I32()));
  auto set2 = makeSet(makeSetType(type_system::TypeSystem::I32()));

  EXPECT_TRUE(set1 == set2);

  set1.insert(DynamicValue::makeI32(1));
  set1.insert(DynamicValue::makeI32(2));
  set1.insert(DynamicValue::makeI32(3));

  set2.insert(DynamicValue::makeI32(1));
  set2.insert(DynamicValue::makeI32(2));
  set2.insert(DynamicValue::makeI32(3));

  EXPECT_TRUE(set1 == set2);

  set2.insert(DynamicValue::makeI32(4));
  EXPECT_FALSE(set1 == set2);

  auto set3 = makeSet(makeSetType(type_system::TypeSystem::I32()));
  EXPECT_FALSE(set1 == set3);

  auto set4 = makeSet(makeSetType(type_system::TypeSystem::String()));
  EXPECT_FALSE(set3 == set4);
}

TEST(SetTest, TypeMismatch) {
  auto set = makeSet(makeSetType(type_system::TypeSystem::I32()));

  // Try to insert a bool value to an i32 set
  EXPECT_THROW(set.insert(DynamicValue::makeBool(true)), std::runtime_error);

  // Try to insert with wrong type
  EXPECT_THROW(set.insert(DynamicValue::makeI64(100)), std::runtime_error);

  // Try to insert with wrong type
  EXPECT_THROW(set.insert(DynamicValue::makeDouble(3.14)), std::runtime_error);
}

// TODO: Enable serialization tests once serialization is implemented
// TEST(SetTest, SerializationRoundTrip) {

TEST(SetTest, DISABLED_SerializationRoundTrip) {
  // Create a set type: set<i32>
  type_system::TypeRef setType =
      type_system::TypeRef(makeSetType(type_system::TypeSystem::I32()));

  // Create and populate set
  auto value = DynamicValue::makeDefault(setType);
  auto& set = value.asSet();
  set.insert(DynamicValue::makeI32(1));
  set.insert(DynamicValue::makeI32(2));
  set.insert(DynamicValue::makeI32(3));

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serializeValue(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserializeValue(reader, setType);

  // Verify
  EXPECT_EQ(value, deserValue);
  auto& deserSet = deserValue.asSet();
  EXPECT_EQ(deserSet.size(), 3);
  EXPECT_TRUE(deserSet.contains(DynamicValue::makeI32(1)));
  EXPECT_TRUE(deserSet.contains(DynamicValue::makeI32(2)));
  EXPECT_TRUE(deserSet.contains(DynamicValue::makeI32(3)));
}

TEST(SetTest, DISABLED_SerializationInteroperability) {
  // Create set type: set<i32>
  auto setType = makeSetType(type_system::TypeSystem::I32());

  // Test 1: Write with DynamicValue, read with materialized type
  {
    // Create and populate set using DynamicValue
    auto value = DynamicValue::makeDefault(type_system::TypeRef(setType));
    auto& set = value.asSet();
    set.insert(DynamicValue::makeI32(1));
    set.insert(DynamicValue::makeI32(2));
    set.insert(DynamicValue::makeI32(3));

    // Serialize using DynamicValue
    folly::IOBufQueue bufQueue;
    CompactProtocolWriter writer;
    writer.setOutput(&bufQueue);
    serializeValue(writer, value);

    // Deserialize using op::decode with materialized type
    auto buf = bufQueue.move();
    CompactProtocolReader reader;
    reader.setInput(buf.get());

    std::set<int32_t> result;
    op::decode<type::set<type::i32_t>>(reader, result);

    // Verify
    ASSERT_EQ(result.size(), 3);
    EXPECT_TRUE(result.count(1) > 0);
    EXPECT_TRUE(result.count(2) > 0);
    EXPECT_TRUE(result.count(3) > 0);
  }

  // Test 2: Write with materialized type, read with DynamicValue
  {
    // Create materialized type: set<int32_t>
    std::set<int32_t> source = {100, 200, 300};

    // Serialize using op::encode
    folly::IOBufQueue bufQueue;
    CompactProtocolWriter writer;
    writer.setOutput(&bufQueue);
    op::encode<type::set<type::i32_t>>(writer, source);

    // Deserialize using DynamicValue
    auto buf = bufQueue.move();
    CompactProtocolReader reader;
    reader.setInput(buf.get());
    auto deserValue = deserializeValue(reader, type_system::TypeRef(setType));

    // Verify
    auto& deserSet = deserValue.asSet();
    ASSERT_EQ(deserSet.size(), 3);
    EXPECT_TRUE(deserSet.contains(DynamicValue::makeI32(100)));
    EXPECT_TRUE(deserSet.contains(DynamicValue::makeI32(200)));
    EXPECT_TRUE(deserSet.contains(DynamicValue::makeI32(300)));
  }
}

TEST(SetTest, Iteration) {
  auto set = makeSet(makeSetType(type_system::TypeSystem::I32()));

  set.insert(DynamicValue::makeI32(1));
  set.insert(DynamicValue::makeI32(2));
  set.insert(DynamicValue::makeI32(3));

  // Test iteration using range-for
  std::set<int32_t> values;
  for (auto ref : set) {
    values.insert(ref.asI32());
  }

  ASSERT_EQ(values.size(), 3);
  EXPECT_TRUE(values.count(1) > 0);
  EXPECT_TRUE(values.count(2) > 0);
  EXPECT_TRUE(values.count(3) > 0);
}

TEST(SetTest, IterationEmpty) {
  auto set = makeSet(makeSetType(type_system::TypeSystem::I32()));

  // Test iteration over empty set
  int count = 0;
  for (auto ref : set) {
    (void)ref;
    count++;
  }

  EXPECT_EQ(count, 0);
  EXPECT_EQ(set.begin(), set.end());
}

} // namespace
} // namespace apache::thrift::dynamic
