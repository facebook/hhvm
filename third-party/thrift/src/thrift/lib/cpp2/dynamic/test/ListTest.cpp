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
#include <thrift/lib/cpp2/dynamic/detail/ConcreteList.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift::dynamic {
namespace {

// Helper to create a list type
inline type_system::TypeRef::List makeListType(
    type_system::TypeRef elementType) {
  static type_system::detail::ContainerTypeCache cache;
  return type_system::TypeRef::List::of(elementType, cache);
}

TEST(ListTest, CreateAndSize) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));
  EXPECT_EQ(list.size(), 0);
  EXPECT_TRUE(list.isEmpty());
}

TEST(ListTest, AppendAndAccess) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));

  list.push_back(DynamicValue::makeI32(10));
  list.push_back(DynamicValue::makeI32(20));
  list.push_back(DynamicValue::makeI32(30));

  EXPECT_EQ(list.size(), 3);
  EXPECT_FALSE(list.isEmpty());

  EXPECT_EQ(list[0].asI32(), 10);
  EXPECT_EQ(list[1].asI32(), 20);
  EXPECT_EQ(list[2].asI32(), 30);
}

TEST(ListTest, SubscriptOperator) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));

  list.push_back(DynamicValue::makeI32(100));
  list.push_back(DynamicValue::makeI32(200));

  EXPECT_EQ(list[0].asI32(), 100);
  EXPECT_EQ(list[1].asI32(), 200);
}

TEST(ListTest, Set) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));

  list.push_back(DynamicValue::makeI32(1));
  list.push_back(DynamicValue::makeI32(2));
  list.push_back(DynamicValue::makeI32(3));

  list.set(1, DynamicValue::makeI32(99));

  EXPECT_EQ(list[0].asI32(), 1);
  EXPECT_EQ(list[1].asI32(), 99);
  EXPECT_EQ(list[2].asI32(), 3);
}

TEST(ListTest, Prepend) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));

  list.push_back(DynamicValue::makeI32(10));
  list.push_front(DynamicValue::makeI32(20));
  list.push_front(DynamicValue::makeI32(30));

  EXPECT_EQ(list.size(), 3);
  EXPECT_EQ(list[0].asI32(), 30);
  EXPECT_EQ(list[1].asI32(), 20);
  EXPECT_EQ(list[2].asI32(), 10);
}

TEST(ListTest, InsertAtIndex) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));

  list.push_back(DynamicValue::makeI32(1));
  list.push_back(DynamicValue::makeI32(3));

  list.insertAtIndex(1, DynamicValue::makeI32(2));

  EXPECT_EQ(list.size(), 3);
  EXPECT_EQ(list[0].asI32(), 1);
  EXPECT_EQ(list[1].asI32(), 2);
  EXPECT_EQ(list[2].asI32(), 3);
}

TEST(ListTest, Fill) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));

  list.push_back(DynamicValue::makeI32(1));
  list.push_back(DynamicValue::makeI32(2));

  list.fill(5, DynamicValue::makeI32(42));

  EXPECT_EQ(list.size(), 5);
  for (size_t i = 0; i < 5; ++i) {
    EXPECT_EQ(list[i].asI32(), 42);
  }
}

TEST(ListTest, Extend) {
  auto list1 = makeList(makeListType(type_system::TypeSystem::I32()));
  auto list2 = makeList(makeListType(type_system::TypeSystem::I32()));

  list1.push_back(DynamicValue::makeI32(1));
  list1.push_back(DynamicValue::makeI32(2));

  list2.push_back(DynamicValue::makeI32(3));
  list2.push_back(DynamicValue::makeI32(4));

  list1.extend(list2);

  EXPECT_EQ(list1.size(), 4);
  EXPECT_EQ(list1[0].asI32(), 1);
  EXPECT_EQ(list1[1].asI32(), 2);
  EXPECT_EQ(list1[2].asI32(), 3);
  EXPECT_EQ(list1[3].asI32(), 4);
}

TEST(ListTest, Slice) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));

  list.push_back(DynamicValue::makeI32(1));
  list.push_back(DynamicValue::makeI32(2));
  list.push_back(DynamicValue::makeI32(3));

  // Basic slice
  {
    auto sliced = list.slice(0, 2);
    EXPECT_EQ(sliced.size(), 2);
    EXPECT_EQ(sliced[0].asI32(), 1);
    EXPECT_EQ(sliced[1].asI32(), 2);
  }

  // Empty slice
  {
    auto sliced = list.slice(1, 1);
    EXPECT_EQ(sliced.size(), 0);
    EXPECT_TRUE(sliced.isEmpty());
  }

  // Full slice
  {
    auto sliced = list.slice(0, 3);
    EXPECT_EQ(sliced.size(), 3);
    EXPECT_EQ(sliced[0].asI32(), 1);
    EXPECT_EQ(sliced[1].asI32(), 2);
    EXPECT_EQ(sliced[2].asI32(), 3);
  }

  // Out of range slices
  EXPECT_THROW(list.slice(0, 10), std::out_of_range);
  EXPECT_THROW(list.slice(10, 20), std::out_of_range);
  EXPECT_THROW(list.slice(2, 1), std::out_of_range);
}

TEST(ListTest, Clear) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));

  list.push_back(DynamicValue::makeI32(1));
  list.push_back(DynamicValue::makeI32(2));
  list.push_back(DynamicValue::makeI32(3));

  EXPECT_EQ(list.size(), 3);

  list.clear();

  EXPECT_EQ(list.size(), 0);
  EXPECT_TRUE(list.isEmpty());
}

TEST(ListTest, Reserve) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));

  list.reserve(100);

  // Reserve shouldn't change the size
  EXPECT_EQ(list.size(), 0);

  // But we can still append
  list.push_back(DynamicValue::makeI32(42));
  EXPECT_EQ(list.size(), 1);
}

TEST(ListTest, Equality) {
  auto list1 = makeList(makeListType(type_system::TypeSystem::I32()));
  auto list2 = makeList(makeListType(type_system::TypeSystem::I32()));

  list1.push_back(DynamicValue::makeI32(1));
  list1.push_back(DynamicValue::makeI32(2));
  list1.push_back(DynamicValue::makeI32(3));

  list2.push_back(DynamicValue::makeI32(1));
  list2.push_back(DynamicValue::makeI32(2));
  list2.push_back(DynamicValue::makeI32(3));

  EXPECT_TRUE(list1 == list2);

  list2.set(1, DynamicValue::makeI32(99));
  EXPECT_FALSE(list1 == list2);
}

TEST(ListTest, TypeMismatch) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));

  // Try to append a bool value to an i32 list
  EXPECT_THROW(
      list.push_back(DynamicValue::makeBool(true)), std::runtime_error);

  // Try to set with wrong type
  list.push_back(DynamicValue::makeI32(42));
  EXPECT_THROW(list.set(0, DynamicValue::makeBool(false)), std::runtime_error);

  // Try to prepend with wrong type
  EXPECT_THROW(list.push_front(DynamicValue::makeI64(100)), std::runtime_error);

  // Try to insertAtIndex with wrong type
  EXPECT_THROW(
      list.insertAtIndex(0, DynamicValue::makeDouble(3.14)),
      std::runtime_error);
}

TEST(ListTest, NumericTypeLists) {
  // Test bool list
  {
    auto list = makeList(makeListType(type_system::TypeSystem::Bool()));
    list.push_back(DynamicValue::makeBool(true));
    list.push_back(DynamicValue::makeBool(false));
    EXPECT_EQ(list.size(), 2);
    EXPECT_TRUE(list[0].asBool());
    EXPECT_FALSE(list[1].asBool());
  }

  // Test byte list
  {
    auto list = makeList(makeListType(type_system::TypeSystem::Byte()));
    list.push_back(DynamicValue::makeByte(10));
    list.push_back(DynamicValue::makeByte(20));
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list[0].asByte(), 10);
    EXPECT_EQ(list[1].asByte(), 20);
  }

  // Test i16 list
  {
    auto list = makeList(makeListType(type_system::TypeSystem::I16()));
    list.push_back(DynamicValue::makeI16(1000));
    list.push_back(DynamicValue::makeI16(2000));
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list[0].asI16(), 1000);
    EXPECT_EQ(list[1].asI16(), 2000);
  }

  // Test i64 list
  {
    auto list = makeList(makeListType(type_system::TypeSystem::I64()));
    list.push_back(DynamicValue::makeI64(1000000000LL));
    list.push_back(DynamicValue::makeI64(2000000000LL));
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list[0].asI64(), 1000000000LL);
    EXPECT_EQ(list[1].asI64(), 2000000000LL);
  }

  // Test float list
  {
    auto list = makeList(makeListType(type_system::TypeSystem::Float()));
    list.push_back(DynamicValue::makeFloat(1.5f));
    list.push_back(DynamicValue::makeFloat(2.5f));
    EXPECT_EQ(list.size(), 2);
    EXPECT_FLOAT_EQ(list[0].asFloat(), 1.5f);
    EXPECT_FLOAT_EQ(list[1].asFloat(), 2.5f);
  }

  // Test double list
  {
    auto list = makeList(makeListType(type_system::TypeSystem::Double()));
    list.push_back(DynamicValue::makeDouble(1.5));
    list.push_back(DynamicValue::makeDouble(2.5));
    EXPECT_EQ(list.size(), 2);
    EXPECT_DOUBLE_EQ(list[0].asDouble(), 1.5);
    EXPECT_DOUBLE_EQ(list[1].asDouble(), 2.5);
  }
}

TEST(ListTest, OutOfRange) {
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));

  list.push_back(DynamicValue::makeI32(1));

  // Test out of range access
  EXPECT_THROW(list[1], std::out_of_range);
  EXPECT_THROW(list[100], std::out_of_range);

  // Test out of range set
  EXPECT_THROW(list.set(1, DynamicValue::makeI32(2)), std::out_of_range);
  EXPECT_THROW(list.set(100, DynamicValue::makeI32(2)), std::out_of_range);

  // Test out of range insert: index == size is valid for insert
  EXPECT_NO_THROW(list.insertAtIndex(1, DynamicValue::makeI32(2)));

  // But beyond size is not
  EXPECT_THROW(
      list.insertAtIndex(100, DynamicValue::makeI32(3)), std::out_of_range);
}

// Serialization tests using list<list<i32>>

TEST(ListTest, SerializationRoundTrip) {
  // Create inner list type: list<i32>
  type_system::TypeRef innerListType =
      type_system::TypeRef(makeListType(type_system::TypeSystem::I32()));

  // Create outer list type: list<list<i32>>
  type_system::TypeRef outerListType =
      type_system::TypeRef(makeListType(innerListType));

  // Create inner lists as DynamicValues
  auto innerValue1 = DynamicValue::makeDefault(innerListType);
  auto& innerList1 = innerValue1.asList();
  innerList1.push_back(DynamicValue::makeI32(1));
  innerList1.push_back(DynamicValue::makeI32(2));
  innerList1.push_back(DynamicValue::makeI32(3));

  auto innerValue2 = DynamicValue::makeDefault(innerListType);
  auto& innerList2 = innerValue2.asList();
  innerList2.push_back(DynamicValue::makeI32(10));
  innerList2.push_back(DynamicValue::makeI32(20));

  auto innerValue3 = DynamicValue::makeDefault(innerListType);
  auto& innerList3 = innerValue3.asList();
  innerList3.push_back(DynamicValue::makeI32(100));
  innerList3.push_back(DynamicValue::makeI32(200));
  innerList3.push_back(DynamicValue::makeI32(300));
  innerList3.push_back(DynamicValue::makeI32(400));

  // Create outer list as DynamicValue
  auto value = DynamicValue::makeDefault(outerListType);
  auto& outerList = value.asList();
  outerList.push_back(std::move(innerValue1));
  outerList.push_back(std::move(innerValue2));
  outerList.push_back(std::move(innerValue3));

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serializeValue(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserializeValue(reader, outerListType);

  // Verify
  EXPECT_EQ(value, deserValue);
  auto& deserOuterList = deserValue.asList();
  EXPECT_EQ(deserOuterList.size(), 3);

  // Check first inner list
  auto deserInnerList1Value = deserOuterList[0];
  auto& deserInnerList1 = deserInnerList1Value.asList();
  EXPECT_EQ(deserInnerList1.size(), 3);
  EXPECT_EQ(deserInnerList1[0].asI32(), 1);
  EXPECT_EQ(deserInnerList1[1].asI32(), 2);
  EXPECT_EQ(deserInnerList1[2].asI32(), 3);

  // Check second inner list
  auto deserInnerList2Value = deserOuterList[1];
  auto& deserInnerList2 = deserInnerList2Value.asList();
  EXPECT_EQ(deserInnerList2.size(), 2);
  EXPECT_EQ(deserInnerList2[0].asI32(), 10);
  EXPECT_EQ(deserInnerList2[1].asI32(), 20);

  // Check third inner list
  auto deserInnerList3Value = deserOuterList[2];
  auto& deserInnerList3 = deserInnerList3Value.asList();
  EXPECT_EQ(deserInnerList3.size(), 4);
  EXPECT_EQ(deserInnerList3[0].asI32(), 100);
  EXPECT_EQ(deserInnerList3[1].asI32(), 200);
  EXPECT_EQ(deserInnerList3[2].asI32(), 300);
  EXPECT_EQ(deserInnerList3[3].asI32(), 400);
}

TEST(ListTest, SerializationInteroperability) {
  // Create inner list type: list<i32>
  auto innerListType = makeListType(type_system::TypeSystem::I32());

  // Create outer list type: list<list<i32>>
  auto outerListType = makeListType(type_system::TypeRef(innerListType));

  // Test 1: Write with DynamicValue, read with materialized type
  {
    // Create and populate lists using DynamicValue
    auto innerValue1 =
        DynamicValue::makeDefault(type_system::TypeRef(innerListType));
    auto& innerList1 = innerValue1.asList();
    innerList1.push_back(DynamicValue::makeI32(1));
    innerList1.push_back(DynamicValue::makeI32(2));

    auto innerValue2 =
        DynamicValue::makeDefault(type_system::TypeRef(innerListType));
    auto& innerList2 = innerValue2.asList();
    innerList2.push_back(DynamicValue::makeI32(10));
    innerList2.push_back(DynamicValue::makeI32(20));
    innerList2.push_back(DynamicValue::makeI32(30));

    auto value = DynamicValue::makeDefault(type_system::TypeRef(outerListType));
    auto& outerList = value.asList();
    outerList.push_back(std::move(innerValue1));
    outerList.push_back(std::move(innerValue2));

    // Serialize using DynamicValue
    folly::IOBufQueue bufQueue;
    CompactProtocolWriter writer;
    writer.setOutput(&bufQueue);
    serializeValue(writer, value);

    // Deserialize using op::decode with materialized type
    auto buf = bufQueue.move();
    CompactProtocolReader reader;
    reader.setInput(buf.get());

    std::vector<std::vector<int32_t>> result;
    op::decode<type::list<type::list<type::i32_t>>>(reader, result);

    // Verify
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0].size(), 2);
    EXPECT_EQ(result[0][0], 1);
    EXPECT_EQ(result[0][1], 2);
    ASSERT_EQ(result[1].size(), 3);
    EXPECT_EQ(result[1][0], 10);
    EXPECT_EQ(result[1][1], 20);
    EXPECT_EQ(result[1][2], 30);
  }

  // Test 2: Write with materialized type, read with DynamicValue
  {
    // Create materialized type: vector<vector<int32_t>>
    std::vector<std::vector<int32_t>> source = {{100, 200, 300}, {400, 500}};

    // Serialize using op::encode
    folly::IOBufQueue bufQueue;
    CompactProtocolWriter writer;
    writer.setOutput(&bufQueue);
    op::encode<type::list<type::list<type::i32_t>>>(writer, source);

    // Deserialize using DynamicValue
    auto buf = bufQueue.move();
    CompactProtocolReader reader;
    reader.setInput(buf.get());
    auto deserValue =
        deserializeValue(reader, type_system::TypeRef(outerListType));

    // Verify
    auto& deserOuterList = deserValue.asList();
    ASSERT_EQ(deserOuterList.size(), 2);

    auto deserInnerList1Value = deserOuterList[0];
    auto& deserInnerList1 = deserInnerList1Value.asList();
    ASSERT_EQ(deserInnerList1.size(), 3);
    EXPECT_EQ(deserInnerList1[0].asI32(), 100);
    EXPECT_EQ(deserInnerList1[1].asI32(), 200);
    EXPECT_EQ(deserInnerList1[2].asI32(), 300);

    auto deserInnerList2Value = deserOuterList[1];
    auto& deserInnerList2 = deserInnerList2Value.asList();
    ASSERT_EQ(deserInnerList2.size(), 2);
    EXPECT_EQ(deserInnerList2[0].asI32(), 400);
    EXPECT_EQ(deserInnerList2[1].asI32(), 500);
  }
}

} // namespace
} // namespace apache::thrift::dynamic
