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

#include <thrift/lib/cpp2/protocol/Serializer.h>

#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
#include <folly/test/JsonTestUtil.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/TableBasedSerializerImpl.h>
#include <thrift/test/tablebased/gen-cpp2/frozen_tablebased_types.h>
#include <thrift/test/tablebased/gen-cpp2/frozen_types.h>
#include <thrift/test/tablebased/gen-cpp2/thrift_tablebased_types.h>
#include <thrift/test/tablebased/gen-cpp2/thrift_tablebased_types_custom_protocol.h>
#include <thrift/test/tablebased/gen-cpp2/thrift_types.h>

using apache::thrift::BinarySerializer;
using apache::thrift::CompactSerializer;
using apache::thrift::SimpleJSONSerializer;
using namespace facebook::thrift::test;
namespace tablebased = facebook::thrift::test::tablebased;

namespace {
// This will actually fail if fields with larger ids are listed first in the
// thrift struct because old serialization code will serialize by IDL order,
// while new serialization code will serialize by field id order. The resulting
// difference in order will consequently change the bytes serialized.
#define EXPECT_SERIALIZED_DATA_EQ(Serializer, expected, result) \
  do {                                                          \
    if (std::is_same_v<Serializer, SimpleJSONSerializer>) {     \
      FOLLY_EXPECT_JSON_EQ(expected, result);                   \
    } else {                                                    \
      EXPECT_EQ(expected, result);                              \
    }                                                           \
  } while (false)

// Tests that table based serialization matches the output of original
// serialization. Tests that table based deserialization works with original
// serialized bytes.
#define EXPECT_COMPATIBLE_PROTOCOL_IMPL(                                     \
    object, tableBasedObject, Serializer, shouldSkipEqualityForUnionWithRef) \
  do {                                                                       \
    std::string originalBytes =                                              \
        Serializer::template serialize<std::string>(object);                 \
    auto tableBasedObjectFromOriginalBytes =                                 \
        Serializer::template deserialize<decltype(tableBasedObject)>(        \
            originalBytes);                                                  \
    std::string tableBasedBytes =                                            \
        Serializer::template serialize<std::string>(tableBasedObject);       \
    if (!shouldSkipEqualityForUnionWithRef) {                                \
      EXPECT_EQ(tableBasedObject, tableBasedObjectFromOriginalBytes);        \
    }                                                                        \
    EXPECT_SERIALIZED_DATA_EQ(Serializer, originalBytes, tableBasedBytes);   \
  } while (false)

#define EXPECT_COMPATIBLE_PROTOCOL(object, tableBasedObject, Serializer) \
  EXPECT_COMPATIBLE_PROTOCOL_IMPL(object, tableBasedObject, Serializer, false)

#define EXPECT_COMPATIBLE_PROTOCOL_UNION_REF( \
    object, tableBasedObject, Serializer)     \
  EXPECT_COMPATIBLE_PROTOCOL_IMPL(object, tableBasedObject, Serializer, true)

template <typename Type>
Type makeStructWithIncludeLike() {
  auto obj = Type();
  obj.field_ref() = {};
  return obj;
}

template <typename Type>
Type makeFrozenStructBLike() {
  auto obj = Type();
  obj.fieldA_ref() = 2000;
  return obj;
}

template <typename Type>
Type makeFrozenStructALike() {
  auto obj = Type();
  obj.fieldA_ref() = 2000;
  return obj;
}

template <typename Type>
Type makeStructBLike() {
  auto obj = Type();
  obj.i64_field_ref() = 2000;
  obj.iobufptr_field_ref() = folly::IOBuf::copyBuffer("testBuffer");

  obj.list_field_ref() = std::make_shared<std::vector<int64_t>>();
  obj.list_field_ref()->emplace_back(9000);
  obj.list_field_ref()->emplace_back(8000);

  obj.i32_field_ref() = 1000;
  obj.i16_field_ref() = 20;
  obj.byte_field_ref() = 16;
  obj.bool_field_ref() = true;
  obj.set_field_ref() = std::set{1, 2, 3};
  obj.iobuf_field_ref() = "testBuffer";
  obj.double_field_ref() = 1.0;
  obj.float_field_ref() = 2.0;
  return obj;
}

template <typename Type>
Type makeStructALike() {
  auto obj = Type();
  obj.list_field_ref() = {"first", "second"};
  obj.map_field_ref() = {{"first", 1}, {"second", 2}};
  obj.opt_str_field_ref() = "yo";
  obj.i64_field_ref() = 123;
  obj.str_field_ref() = "unqualified";
  obj.bin_field_ref()->push_back('\0');
  using Struct = std::remove_reference_t<decltype(*obj.struct_field_ref())>;
  obj.struct_field_ref() = makeStructBLike<Struct>();
  using Enum = std::remove_reference_t<decltype(*obj.enum_field_ref())>;
  obj.enum_field_ref() = Enum::A;
  return obj;
}

template <typename Type>
Type makeStructWithRefLike() {
  auto obj = Type();
  using Struct =
      std::remove_reference_t<decltype(*obj.shared_struct_field_ref())>;
  obj.shared_struct_field_ref() = std::make_shared<std::add_const_t<Struct>>(
      makeStructBLike<std::remove_const_t<Struct>>());
  std::vector<std::string> tmp = {"test1", "test2"};
  obj.shared_list_field_ref() =
      std::make_shared<const std::vector<std::string>>(std::move(tmp));
  obj.shared_i16_field_ref() = std::make_shared<const std::int16_t>(1000);
  obj.unique_i32_field_ref() = std::make_unique<std::int32_t>(5000);
  return obj;
}
} // namespace

using Protocols =
    ::testing::Types<CompactSerializer, SimpleJSONSerializer, BinarySerializer>;

template <typename Serializer>
class MultiProtocolTest : public ::testing::Test {};
TYPED_TEST_CASE(MultiProtocolTest, Protocols);

TYPED_TEST(MultiProtocolTest, EmptyFrozenStructA) {
  EXPECT_COMPATIBLE_PROTOCOL(
      FrozenStructA(), tablebased::FrozenStructA(), TypeParam);
}

TYPED_TEST(MultiProtocolTest, FrozenStructA) {
  FrozenStructA oldObject = makeFrozenStructALike<FrozenStructA>();
  tablebased::FrozenStructA newObject =
      makeFrozenStructALike<tablebased::FrozenStructA>();
  EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
}

TYPED_TEST(MultiProtocolTest, EmptyFrozenStructB) {
  EXPECT_COMPATIBLE_PROTOCOL(
      FrozenStructB(), tablebased::FrozenStructA(), TypeParam);
}

TYPED_TEST(MultiProtocolTest, FrozenStructB) {
  FrozenStructB oldObject = makeFrozenStructBLike<FrozenStructB>();
  tablebased::FrozenStructB newObject =
      makeFrozenStructBLike<tablebased::FrozenStructB>();
  EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
}

TYPED_TEST(MultiProtocolTest, EmptyStructA) {
  EXPECT_COMPATIBLE_PROTOCOL(StructA(), tablebased::StructA(), TypeParam);
}

TYPED_TEST(MultiProtocolTest, StructA) {
  StructA oldObject = makeStructALike<StructA>();
  tablebased::StructA newObject = makeStructALike<tablebased::StructA>();
  EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
}

TYPED_TEST(MultiProtocolTest, EmptyStructWithRef) {
  EXPECT_COMPATIBLE_PROTOCOL(
      StructWithRef(), tablebased::StructWithRef(), TypeParam);
}

TYPED_TEST(MultiProtocolTest, StructWithRef) {
  auto oldObject = makeStructWithRefLike<StructWithRef>();
  auto newObject = makeStructWithRefLike<tablebased::StructWithRef>();
  EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
}

TYPED_TEST(MultiProtocolTest, EmptyStructWithInclude) {
  EXPECT_COMPATIBLE_PROTOCOL(
      StructWithInclude(), tablebased::StructWithInclude(), TypeParam);
}

TYPED_TEST(MultiProtocolTest, StructWithInclude) {
  auto oldObject = makeStructWithIncludeLike<StructWithInclude>();
  auto newObject = makeStructWithIncludeLike<tablebased::StructWithInclude>();
  EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
}

TYPED_TEST(MultiProtocolTest, EmptyUnion) {
  EXPECT_COMPATIBLE_PROTOCOL(Union(), tablebased::Union(), TypeParam);
}

TYPED_TEST(MultiProtocolTest, Union) {
  {
    Union oldObject;
    oldObject.a_field() = makeStructALike<StructA>();
    tablebased::Union newObject;
    newObject.a_field() = makeStructALike<tablebased::StructA>();
    EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
  }
  {
    Union oldObject;
    oldObject.b_field() = makeStructBLike<StructB>();
    tablebased::Union newObject;
    newObject.b_field() = makeStructBLike<tablebased::StructB>();
    EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
  }
  {
    Union oldObject;
    oldObject.str_field() = "test";
    tablebased::Union newObject;
    newObject.str_field() = "test";
    EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
  }
}

TYPED_TEST(MultiProtocolTest, EmptyUnionWithRef) {
  EXPECT_COMPATIBLE_PROTOCOL(
      UnionWithRef(), tablebased::UnionWithRef(), TypeParam);
}

TYPED_TEST(MultiProtocolTest, UnionWithRef) {
  {
    UnionWithRef oldObject;
    oldObject.set_simple_field(makeStructBLike<StructB>());
    tablebased::UnionWithRef newObject;
    newObject.set_simple_field(makeStructBLike<tablebased::StructB>());
    EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
  }
  {
    UnionWithRef oldObject;
    oldObject.set_unique_field();
    {
      auto& ptr = oldObject.get_unique_field();
      const_cast<std::unique_ptr<StructA>&>(ptr) =
          std::unique_ptr<StructA>(nullptr);
    }
    tablebased::UnionWithRef newObject;
    newObject.set_unique_field();
    {
      auto& ptr = newObject.get_unique_field();
      const_cast<std::unique_ptr<tablebased::StructA>&>(ptr) =
          std::unique_ptr<tablebased::StructA>(nullptr);
    }
    EXPECT_COMPATIBLE_PROTOCOL_UNION_REF(oldObject, newObject, TypeParam);
    oldObject.set_unique_field(makeStructALike<StructA>());
    newObject.set_unique_field(makeStructALike<tablebased::StructA>());
    EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
  }
  {
    UnionWithRef oldObject;
    oldObject.set_shared_field();
    {
      auto& ptr = oldObject.get_shared_field();
      const_cast<std::shared_ptr<StructA>&>(ptr) =
          std::shared_ptr<StructA>(nullptr);
    }
    tablebased::UnionWithRef newObject;
    newObject.set_shared_field();
    {
      auto& ptr = newObject.get_shared_field();
      const_cast<std::shared_ptr<tablebased::StructA>&>(ptr) =
          std::shared_ptr<tablebased::StructA>(nullptr);
    }
    EXPECT_COMPATIBLE_PROTOCOL_UNION_REF(oldObject, newObject, TypeParam);
    oldObject.set_shared_field(makeStructALike<StructA>());
    newObject.set_shared_field(makeStructALike<tablebased::StructA>());
    EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
  }
  {
    UnionWithRef oldObject;
    oldObject.set_shared_const_field();
    {
      auto& ptr = oldObject.get_shared_const_field();
      const_cast<std::shared_ptr<const StructA>&>(ptr) =
          std::shared_ptr<const StructA>(nullptr);
    }
    tablebased::UnionWithRef newObject;
    newObject.set_shared_const_field();
    {
      auto& ptr = newObject.get_shared_const_field();
      const_cast<std::shared_ptr<const tablebased::StructA>&>(ptr) =
          std::shared_ptr<const tablebased::StructA>(nullptr);
    }
    EXPECT_COMPATIBLE_PROTOCOL_UNION_REF(oldObject, newObject, TypeParam);
    oldObject.set_shared_const_field(makeStructALike<StructA>());
    newObject.set_shared_const_field(makeStructALike<tablebased::StructA>());
    EXPECT_COMPATIBLE_PROTOCOL(oldObject, newObject, TypeParam);
  }
}

TYPED_TEST(MultiProtocolTest, DirtyReadIntoContainer) {
  auto dirty = tablebased::StructA();
  dirty.list_field() = {"should be cleared"};
  auto filled = makeStructALike<tablebased::StructA>();
  auto serialized = TypeParam::template serialize<std::string>(filled);
  TypeParam::deserialize(serialized, dirty);
  EXPECT_EQ(filled.list_field(), dirty.list_field());
}

TYPED_TEST(MultiProtocolTest, ReadingUnqualifiedFieldShouldSetIsset) {
  auto obj = makeStructALike<tablebased::StructA>();
  auto deserialized = TypeParam::template deserialize<tablebased::StructA>(
      TypeParam::template serialize<std::string>(obj));
  EXPECT_TRUE(deserialized.str_field_ref().is_set());
  EXPECT_EQ(deserialized.str_field_ref(), "unqualified");
}

TEST(SerializerTest, UnionValueOffsetIsZero) {
  tablebased::Union u;
  u.set_str_field("test");
  EXPECT_EQ(static_cast<void*>(&u), &*u.str_field());

  u.set_a_field({});
  EXPECT_EQ(static_cast<void*>(&u), &*u.a_field());

  u.set_b_field({});
  EXPECT_EQ(static_cast<void*>(&u), &*u.b_field());
}

TEST(SerializerTest, DuplicateUnionData) {
  // Test that we can handle invalid serialized input with duplicate and
  // incomplete union data.
  const char data[] =
      "\x0c" // type = TType::T_STRUCT
      "\x00\x01" // fieldId = 1 (union_field)
      "\x0b" // type = TType::T_STRING
      "\x00\x01" // fieldId = 1 (string_field)
      "\x00\x00\x00\x00" // size = 0
      "\x00" // end of union_field

      "\x0c" // type = TType::T_STRUCT
      "\x00\x01" // fieldId = 1 (union_field)
      "\x13" // type = TType::T_FLOAT
      "\x00\x02"; // fieldId = 2 (float_field), value is missing

  EXPECT_THROW(
      BinarySerializer::deserialize<tablebased::TestStructWithUnion>(
          folly::StringPiece(data, sizeof(data))),
      std::out_of_range);
}

TEST(SerializerTest, DebugProtocol) {
  auto s = apache::thrift::debugString(tablebased::StructA());
  EXPECT_NE(s.find("StructA"), std::string::npos);
}
