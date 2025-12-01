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
#include <thrift/lib/cpp2/dynamic/detail/ConcreteMap.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift::dynamic {
namespace {

// Helper to create a map type
inline type_system::TypeRef::Map makeMapType(
    type_system::TypeRef keyType, type_system::TypeRef valueType) {
  static type_system::detail::ContainerTypeCache cache;
  return type_system::TypeRef::Map::of(keyType, valueType, cache);
}

TEST(MapTest, InsertAndGet) {
  auto map = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));

  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.isEmpty());

  map.insert(DynamicValue::makeI32(1), DynamicValue::makeString("one"));
  map.insert(DynamicValue::makeI32(2), DynamicValue::makeString("two"));
  map.insert(DynamicValue::makeI32(3), DynamicValue::makeString("three"));

  EXPECT_EQ(map.size(), 3);
  EXPECT_FALSE(map.isEmpty());

  auto val1 = map.get(DynamicValue::makeI32(1));
  ASSERT_TRUE(val1.has_value());
  EXPECT_EQ(val1->asString().view(), "one");

  auto val2 = map.get(DynamicValue::makeI32(2));
  ASSERT_TRUE(val2.has_value());
  EXPECT_EQ(val2->asString().view(), "two");

  auto val3 = map.get(DynamicValue::makeI32(3));
  ASSERT_TRUE(val3.has_value());
  EXPECT_EQ(val3->asString().view(), "three");

  // Get non-existing key
  auto val4 = map.get(DynamicValue::makeI32(4));
  EXPECT_FALSE(val4.has_value());
}

TEST(MapTest, InsertOrAssign) {
  auto map = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));

  // First insert
  map.insert(DynamicValue::makeI32(1), DynamicValue::makeString("one"));
  EXPECT_EQ(map.size(), 1);

  auto val1 = map.get(DynamicValue::makeI32(1));
  ASSERT_TRUE(val1.has_value());
  EXPECT_EQ(val1->asString().view(), "one");

  // Update existing key
  map.insert(DynamicValue::makeI32(1), DynamicValue::makeString("ONE"));
  EXPECT_EQ(map.size(), 1);

  auto val2 = map.get(DynamicValue::makeI32(1));
  ASSERT_TRUE(val2.has_value());
  EXPECT_EQ(val2->asString().view(), "ONE");
}

TEST(MapTest, Contains) {
  auto map = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));

  map.insert(DynamicValue::makeI32(1), DynamicValue::makeString("one"));
  map.insert(DynamicValue::makeI32(2), DynamicValue::makeString("two"));

  EXPECT_TRUE(map.contains(DynamicValue::makeI32(1)));
  EXPECT_TRUE(map.contains(DynamicValue::makeI32(2)));
  EXPECT_FALSE(map.contains(DynamicValue::makeI32(3)));
}

TEST(MapTest, Erase) {
  auto map = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));

  map.insert(DynamicValue::makeI32(1), DynamicValue::makeString("one"));
  map.insert(DynamicValue::makeI32(2), DynamicValue::makeString("two"));
  map.insert(DynamicValue::makeI32(3), DynamicValue::makeString("three"));

  EXPECT_EQ(map.size(), 3);

  // Erase existing element
  EXPECT_TRUE(map.erase(DynamicValue::makeI32(2)));
  EXPECT_EQ(map.size(), 2);
  EXPECT_FALSE(map.contains(DynamicValue::makeI32(2)));

  // Erase non-existing element
  EXPECT_FALSE(map.erase(DynamicValue::makeI32(4)));
  EXPECT_EQ(map.size(), 2);
}

TEST(MapTest, Clear) {
  auto map = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));

  map.insert(DynamicValue::makeI32(1), DynamicValue::makeString("one"));
  map.insert(DynamicValue::makeI32(2), DynamicValue::makeString("two"));
  map.insert(DynamicValue::makeI32(3), DynamicValue::makeString("three"));

  EXPECT_EQ(map.size(), 3);

  map.clear();

  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.isEmpty());
}

TEST(MapTest, Reserve) {
  auto map = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));

  map.reserve(100);

  // Reserve shouldn't change the size
  EXPECT_EQ(map.size(), 0);

  // But we can still insert
  map.insert(DynamicValue::makeI32(42), DynamicValue::makeString("answer"));
  EXPECT_EQ(map.size(), 1);
}

TEST(MapTest, Equality) {
  auto map1 = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));
  auto map2 = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));

  EXPECT_TRUE(map1 == map2);

  map1.insert(DynamicValue::makeI32(1), DynamicValue::makeString("one"));
  map1.insert(DynamicValue::makeI32(2), DynamicValue::makeString("two"));
  map1.insert(DynamicValue::makeI32(3), DynamicValue::makeString("three"));

  map2.insert(DynamicValue::makeI32(1), DynamicValue::makeString("one"));
  map2.insert(DynamicValue::makeI32(2), DynamicValue::makeString("two"));
  map2.insert(DynamicValue::makeI32(3), DynamicValue::makeString("three"));

  EXPECT_TRUE(map1 == map2);

  map2.insert(DynamicValue::makeI32(4), DynamicValue::makeString("four"));
  EXPECT_FALSE(map1 == map2);

  auto map3 = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));
  EXPECT_FALSE(map1 == map3);

  auto map4 = makeMap(makeMapType(
      type_system::TypeSystem::String(), type_system::TypeSystem::String()));
  EXPECT_FALSE(map3 == map4);
}

TEST(MapTest, TypeMismatchKey) {
  auto map = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));

  // Try to insert with wrong key type
  EXPECT_THROW(
      map.insert(DynamicValue::makeBool(true), DynamicValue::makeString("val")),
      std::runtime_error);

  EXPECT_THROW(
      map.insert(DynamicValue::makeI64(100), DynamicValue::makeString("val")),
      std::runtime_error);
}

TEST(MapTest, TypeMismatchValue) {
  auto map = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));

  // Try to insert with wrong value type
  EXPECT_THROW(
      map.insert(DynamicValue::makeI32(1), DynamicValue::makeBool(true)),
      std::runtime_error);

  EXPECT_THROW(
      map.insert(DynamicValue::makeI32(1), DynamicValue::makeI64(100)),
      std::runtime_error);
}

TEST(MapTest, NumericTypeMaps) {
  // Test i32 -> i64 map
  {
    auto map = makeMap(makeMapType(
        type_system::TypeSystem::I32(), type_system::TypeSystem::I64()));
    map.insert(DynamicValue::makeI32(1), DynamicValue::makeI64(100));
    map.insert(DynamicValue::makeI32(2), DynamicValue::makeI64(200));
    EXPECT_EQ(map.size(), 2);

    auto val1 = map.get(DynamicValue::makeI32(1));
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(val1->asI64(), 100);

    auto val2 = map.get(DynamicValue::makeI32(2));
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(val2->asI64(), 200);
  }

  // Test string -> bool map
  {
    auto map = makeMap(makeMapType(
        type_system::TypeSystem::String(), type_system::TypeSystem::Bool()));
    map.insert(DynamicValue::makeString("yes"), DynamicValue::makeBool(true));
    map.insert(DynamicValue::makeString("no"), DynamicValue::makeBool(false));
    EXPECT_EQ(map.size(), 2);

    auto val1 = map.get(DynamicValue::makeString("yes"));
    ASSERT_TRUE(val1.has_value());
    EXPECT_TRUE(val1->asBool());

    auto val2 = map.get(DynamicValue::makeString("no"));
    ASSERT_TRUE(val2.has_value());
    EXPECT_FALSE(val2->asBool());
  }

  // Test i16 -> double map
  {
    auto map = makeMap(makeMapType(
        type_system::TypeSystem::I16(), type_system::TypeSystem::Double()));
    map.insert(DynamicValue::makeI16(1), DynamicValue::makeDouble(1.5));
    map.insert(DynamicValue::makeI16(2), DynamicValue::makeDouble(2.5));
    EXPECT_EQ(map.size(), 2);

    auto val1 = map.get(DynamicValue::makeI16(1));
    ASSERT_TRUE(val1.has_value());
    EXPECT_DOUBLE_EQ(val1->asDouble(), 1.5);

    auto val2 = map.get(DynamicValue::makeI16(2));
    ASSERT_TRUE(val2.has_value());
    EXPECT_DOUBLE_EQ(val2->asDouble(), 2.5);
  }
}

TEST(MapTest, GetMutableValue) {
  auto map = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::I32()));

  map.insert(DynamicValue::makeI32(1), DynamicValue::makeI32(100));

  // Get mutable reference
  auto val = map.get(DynamicValue::makeI32(1));
  ASSERT_TRUE(val.has_value());
  EXPECT_EQ(val->asI32(), 100);

  // Modify through reference
  val->asI32() = 999;

  // Verify modification
  auto val2 = map.get(DynamicValue::makeI32(1));
  ASSERT_TRUE(val2.has_value());
  EXPECT_EQ(val2->asI32(), 999);
}

TEST(MapTest, ConstGet) {
  auto map = makeMap(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));

  map.insert(DynamicValue::makeI32(1), DynamicValue::makeString("one"));
  map.insert(DynamicValue::makeI32(2), DynamicValue::makeString("two"));

  const auto& constMap = map;

  auto val1 = constMap.get(DynamicValue::makeI32(1));
  ASSERT_TRUE(val1.has_value());
  EXPECT_EQ(val1->asString().view(), "one");

  auto val2 = constMap.get(DynamicValue::makeI32(2));
  ASSERT_TRUE(val2.has_value());
  EXPECT_EQ(val2->asString().view(), "two");

  // Get non-existing key
  auto val3 = constMap.get(DynamicValue::makeI32(3));
  EXPECT_FALSE(val3.has_value());
}

// TODO: Enable serialization tests once serialization is implemented
TEST(MapTest, DISABLED_SerializationRoundTrip) {
  // Create a map type: map<i32, string>
  type_system::TypeRef mapType = type_system::TypeRef(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));

  // Create and populate map
  auto value = DynamicValue::makeDefault(mapType);
  auto& map = value.asMap();
  map.insert(DynamicValue::makeI32(1), DynamicValue::makeString("one"));
  map.insert(DynamicValue::makeI32(2), DynamicValue::makeString("two"));
  map.insert(DynamicValue::makeI32(3), DynamicValue::makeString("three"));

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serializeValue(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserializeValue(reader, mapType);

  // Verify
  EXPECT_EQ(value, deserValue);
  auto& deserMap = deserValue.asMap();
  EXPECT_EQ(deserMap.size(), 3);

  auto val1 = deserMap.get(DynamicValue::makeI32(1));
  ASSERT_TRUE(val1.has_value());
  EXPECT_EQ(val1->asString().view(), "one");

  auto val2 = deserMap.get(DynamicValue::makeI32(2));
  ASSERT_TRUE(val2.has_value());
  EXPECT_EQ(val2->asString().view(), "two");

  auto val3 = deserMap.get(DynamicValue::makeI32(3));
  ASSERT_TRUE(val3.has_value());
  EXPECT_EQ(val3->asString().view(), "three");
}

TEST(MapTest, DISABLED_SerializationInteroperability) {
  // Create map type: map<i32, string>
  auto mapType = makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String());

  // Test 1: Write with DynamicValue, read with materialized type
  {
    // Create and populate map using DynamicValue
    auto value = DynamicValue::makeDefault(type_system::TypeRef(mapType));
    auto& map = value.asMap();
    map.insert(DynamicValue::makeI32(1), DynamicValue::makeString("one"));
    map.insert(DynamicValue::makeI32(2), DynamicValue::makeString("two"));
    map.insert(DynamicValue::makeI32(3), DynamicValue::makeString("three"));

    // Serialize using DynamicValue
    folly::IOBufQueue bufQueue;
    CompactProtocolWriter writer;
    writer.setOutput(&bufQueue);
    serializeValue(writer, value);

    // Deserialize using op::decode with materialized type
    auto buf = bufQueue.move();
    CompactProtocolReader reader;
    reader.setInput(buf.get());

    std::map<int32_t, std::string> result;
    op::decode<type::map<type::i32_t, type::string_t>>(reader, result);

    // Verify
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[1], "one");
    EXPECT_EQ(result[2], "two");
    EXPECT_EQ(result[3], "three");
  }

  // Test 2: Write with materialized type, read with DynamicValue
  {
    // Create materialized type: map<int32_t, string>
    std::map<int32_t, std::string> source = {
        {100, "hundred"}, {200, "two hundred"}, {300, "three hundred"}};

    // Serialize using op::encode
    folly::IOBufQueue bufQueue;
    CompactProtocolWriter writer;
    writer.setOutput(&bufQueue);
    op::encode<type::map<type::i32_t, type::string_t>>(writer, source);

    // Deserialize using DynamicValue
    auto buf = bufQueue.move();
    CompactProtocolReader reader;
    reader.setInput(buf.get());
    auto deserValue = deserializeValue(reader, type_system::TypeRef(mapType));

    // Verify
    auto& deserMap = deserValue.asMap();
    ASSERT_EQ(deserMap.size(), 3);

    auto val1 = deserMap.get(DynamicValue::makeI32(100));
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(val1->asString().view(), "hundred");

    auto val2 = deserMap.get(DynamicValue::makeI32(200));
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(val2->asString().view(), "two hundred");

    auto val3 = deserMap.get(DynamicValue::makeI32(300));
    ASSERT_TRUE(val3.has_value());
    EXPECT_EQ(val3->asString().view(), "three hundred");
  }
}

} // namespace
} // namespace apache::thrift::dynamic
