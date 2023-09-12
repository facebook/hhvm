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

#include <folly/container/Foreach.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/VirtualProtocol.h>
#include <thrift/lib/cpp2/protocol/detail/index.h>
#include <thrift/test/lazy_deserialization/MemberAccessor.h>
#include <thrift/test/lazy_deserialization/common.h>
#include <thrift/test/lazy_deserialization/gen-cpp2/deprecated_terse_writes_types.h>
#include <thrift/test/lazy_deserialization/gen-cpp2/deprecated_terse_writes_types_custom_protocol.h>
#include <thrift/test/lazy_deserialization/gen-cpp2/simple_types.h>
#include <thrift/test/lazy_deserialization/gen-cpp2/simple_types_custom_protocol.h>

namespace apache::thrift::test {

TYPED_TEST(LazyDeserialization, Copyable) {
  auto base = this->genLazyStruct();
  auto bar = base; // copy constructor
  typename TypeParam::LazyStruct baz;
  baz = base; // copy assignment

  EXPECT_EQ(bar.field1_ref(), baz.field1_ref());
  EXPECT_EQ(bar.field2_ref(), baz.field2_ref());
  EXPECT_EQ(bar.field3_ref(), baz.field3_ref());
  EXPECT_EQ(bar.field4_ref(), baz.field4_ref());

  auto foo = this->genStruct();

  EXPECT_EQ(foo.field1_ref(), bar.field1_ref());
  EXPECT_EQ(foo.field2_ref(), bar.field2_ref());
  EXPECT_EQ(foo.field3_ref(), bar.field3_ref());
  EXPECT_EQ(foo.field4_ref(), bar.field4_ref());
}

TYPED_TEST(LazyDeserialization, Moveable) {
  auto temp = this->genLazyStruct();
  auto bar = std::move(temp); // move constructor
  typename TypeParam::LazyStruct baz;
  baz = this->genLazyStruct(); // move assignment

  EXPECT_EQ(bar.field1_ref(), baz.field1_ref());
  EXPECT_EQ(bar.field2_ref(), baz.field2_ref());
  EXPECT_EQ(bar.field3_ref(), baz.field3_ref());
  EXPECT_EQ(bar.field4_ref(), baz.field4_ref());

  auto foo = this->genStruct();

  EXPECT_EQ(foo.field1_ref(), bar.field1_ref());
  EXPECT_EQ(foo.field2_ref(), bar.field2_ref());
  EXPECT_EQ(foo.field3_ref(), bar.field3_ref());
  EXPECT_EQ(foo.field4_ref(), bar.field4_ref());
}

TYPED_TEST(LazyDeserialization, Swap) {
  auto temp = this->genLazyStruct();
  typename TypeParam::LazyStruct bar, baz;
  swap(bar, temp);

  EXPECT_EQ(temp.field1_ref(), baz.field1_ref());
  EXPECT_EQ(temp.field2_ref(), baz.field2_ref());
  EXPECT_EQ(temp.field3_ref(), baz.field3_ref());
  EXPECT_EQ(temp.field4_ref(), baz.field4_ref());

  baz = this->genLazyStruct();

  EXPECT_EQ(bar.field1_ref(), baz.field1_ref());
  EXPECT_EQ(bar.field2_ref(), baz.field2_ref());
  EXPECT_EQ(bar.field3_ref(), baz.field3_ref());
  EXPECT_EQ(bar.field4_ref(), baz.field4_ref());
}

template <typename T>
class Serialization : public testing::Test {};

using Serializers = ::testing::Types<CompactSerializer, BinarySerializer>;
TYPED_TEST_SUITE(Serialization, Serializers);

TYPED_TEST(LazyDeserialization, FooToLazyFoo) {
  using LazyStruct = typename TypeParam::LazyStruct;

  auto foo = this->genStruct();
  auto s = this->serialize(foo);
  auto lazyFoo = this->template deserialize<LazyStruct>(s);

  EXPECT_EQ(foo.field1_ref(), lazyFoo.field1_ref());
  EXPECT_EQ(foo.field2_ref(), lazyFoo.field2_ref());
  EXPECT_EQ(foo.field3_ref(), lazyFoo.field3_ref());
  EXPECT_EQ(foo.field4_ref(), lazyFoo.field4_ref());
}

TYPED_TEST(LazyDeserialization, LazyFooToFoo) {
  using Struct = typename TypeParam::Struct;

  auto lazyFoo = this->genLazyStruct();
  auto s = this->serialize(lazyFoo);
  auto foo = this->template deserialize<Struct>(s);

  EXPECT_EQ(foo.field1_ref(), lazyFoo.field1_ref());
  EXPECT_EQ(foo.field2_ref(), lazyFoo.field2_ref());
  EXPECT_EQ(foo.field3_ref(), lazyFoo.field3_ref());
  EXPECT_EQ(foo.field4_ref(), lazyFoo.field4_ref());
}

FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field1, LazyFoo, field1);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field2, LazyFoo, field2);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field3, LazyFoo, field3);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field4, LazyFoo, field4);

FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field1, OptionalLazyFoo, field1);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field2, OptionalLazyFoo, field2);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field3, OptionalLazyFoo, field3);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field4, OptionalLazyFoo, field4);

FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field1, TerseLazyFoo, field1);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field2, TerseLazyFoo, field2);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field3, TerseLazyFoo, field3);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field4, TerseLazyFoo, field4);

FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field1, TerseOptionalLazyFoo, field1);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field2, TerseOptionalLazyFoo, field2);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field3, TerseOptionalLazyFoo, field3);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field4, TerseOptionalLazyFoo, field4);

FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field1, FooNoChecksum, field1);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field2, FooNoChecksum, field2);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field3, FooNoChecksum, field3);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field4, FooNoChecksum, field4);

FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field1, LazyFooNoChecksum, field1);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field2, LazyFooNoChecksum, field2);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field3, LazyFooNoChecksum, field3);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field4, LazyFooNoChecksum, field4);

FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field1, LazyCppRef, field1);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field2, LazyCppRef, field2);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field3, LazyCppRef, field3);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field4, LazyCppRef, field4);

TYPED_TEST(LazyDeserialization, CheckDataMember) {
  using LazyStruct = typename TypeParam::LazyStruct;

  auto foo = this->genLazyStruct();
  auto s = this->serialize(foo);
  auto lazyFoo = this->template deserialize<LazyStruct>(s);

  EXPECT_EQ(get_field1(lazyFoo), foo.field1_ref());
  EXPECT_EQ(get_field2(lazyFoo), foo.field2_ref());
  EXPECT_TRUE(get_field3(lazyFoo).empty());
  EXPECT_TRUE(get_field4(lazyFoo).empty());

  EXPECT_EQ(lazyFoo.field1_ref(), foo.field1_ref());
  EXPECT_EQ(lazyFoo.field2_ref(), foo.field2_ref());
  EXPECT_EQ(lazyFoo.field3_ref(), foo.field3_ref());
  EXPECT_EQ(lazyFoo.field4_ref(), foo.field4_ref());

  EXPECT_EQ(get_field1(lazyFoo), foo.field1_ref());
  EXPECT_EQ(get_field2(lazyFoo), foo.field2_ref());
  EXPECT_EQ(get_field3(lazyFoo), foo.field3_ref());
  EXPECT_EQ(get_field4(lazyFoo), foo.field4_ref());
}

// Lazy deserialization does not support virtual protocol, but deserialization
// of a struct with lazy fields should still be possible.
TYPED_TEST(LazyDeserialization, VirtualProtocol) {
  using LazyStruct = typename TypeParam::LazyStruct;
  using Serializer = typename TypeParam::Serializer;
  using VirtualProtocolReader =
      VirtualReader<typename Serializer::ProtocolReader>;

  auto foo = this->genLazyStruct();
  auto s = this->serialize(foo);
  auto buf = folly::IOBuf::wrapBuffer(folly::StringPiece(s));

  // Test both concrete and abstract readers.
  const auto test = [&](auto&& reader) {
    LazyStruct lazyFoo;
    reader.setInput(buf.get());
    lazyFoo.read(&reader);
    EXPECT_EQ(foo, lazyFoo);
  };

  test(VirtualProtocolReader{});
  test(static_cast<VirtualReaderBase&&>(VirtualProtocolReader{}));
}

TYPED_TEST(Serialization, CppRef) {
  {
    LazyCppRef foo;
    foo.field1_ref() = std::make_unique<std::vector<int32_t>>(10, 10);
    foo.field2_ref() = std::make_shared<std::vector<int32_t>>(20, 20);
    foo.field3_ref() = std::make_shared<std::vector<int32_t>>(30, 30);
    foo.field4_ref() = std::make_unique<std::vector<int32_t>>(40, 40);
    auto s = TypeParam::template serialize<std::string>(foo);
    auto bar = TypeParam::template deserialize<LazyCppRef>(s);
    EXPECT_EQ(*bar.field1_ref(), std::vector<int32_t>(10, 10));
    EXPECT_EQ(*bar.field2_ref(), std::vector<int32_t>(20, 20));
    EXPECT_EQ(*bar.field3_ref(), std::vector<int32_t>(30, 30));
    EXPECT_EQ(*bar.field4_ref(), std::vector<int32_t>(40, 40));
  }
  {
    LazyCppRef foo;
    auto s = TypeParam::template serialize<std::string>(foo);
    auto bar = TypeParam::template deserialize<LazyCppRef>(s);
    EXPECT_FALSE(bar.field1_ref());
    EXPECT_FALSE(bar.field2_ref());
    EXPECT_FALSE(bar.field3_ref());
    EXPECT_TRUE(bar.field4_ref()); // non-optional field always has value
  }
}

TYPED_TEST(Serialization, LazyDeserializeCppRef) {
  LazyCppRef foo;
  foo.field1_ref() = std::make_unique<std::vector<int32_t>>(10, 10);
  foo.field2_ref() = std::make_shared<std::vector<int32_t>>(20, 20);
  foo.field3_ref() = std::make_shared<std::vector<int32_t>>(30, 30);
  foo.field4_ref() = std::make_unique<std::vector<int32_t>>(40, 40);
  auto bar = TypeParam::template deserialize<LazyCppRef>(
      TypeParam::template serialize<std::string>(foo));
  auto baz = TypeParam::template deserialize<LazyCppRef>(
      TypeParam::template serialize<std::string>(bar));

  // all fields are not deserialized
  EXPECT_FALSE(get_field1(bar));
  EXPECT_FALSE(get_field2(bar));
  EXPECT_FALSE(get_field3(bar));
  EXPECT_TRUE(get_field4(bar)->empty()); // non-optional field always has value
  EXPECT_FALSE(get_field1(baz));
  EXPECT_FALSE(get_field2(baz));
  EXPECT_FALSE(get_field3(baz));
  EXPECT_TRUE(get_field4(baz)->empty());

  // access all fields
  EXPECT_EQ(*bar.field1_ref(), std::vector<int32_t>(10, 10));
  EXPECT_EQ(*bar.field2_ref(), std::vector<int32_t>(20, 20));
  EXPECT_EQ(*bar.field3_ref(), std::vector<int32_t>(30, 30));
  EXPECT_EQ(*bar.field4_ref(), std::vector<int32_t>(40, 40));
  EXPECT_EQ(*baz.field1_ref(), std::vector<int32_t>(10, 10));
  EXPECT_EQ(*baz.field2_ref(), std::vector<int32_t>(20, 20));
  EXPECT_EQ(*baz.field3_ref(), std::vector<int32_t>(30, 30));
  EXPECT_EQ(*baz.field4_ref(), std::vector<int32_t>(40, 40));

  // now all fields are deserialized
  EXPECT_TRUE(get_field1(bar));
  EXPECT_TRUE(get_field2(bar));
  EXPECT_TRUE(get_field3(bar));
  EXPECT_EQ(*get_field4(bar), std::vector<int32_t>(40, 40));
  EXPECT_TRUE(get_field1(baz));
  EXPECT_TRUE(get_field2(baz));
  EXPECT_TRUE(get_field3(baz));
  EXPECT_EQ(*get_field4(baz), std::vector<int32_t>(40, 40));
}

TYPED_TEST(LazyDeserialization, Comparison) {
  using LazyStruct = typename TypeParam::LazyStruct;

  {
    auto foo1 = this->genLazyStruct();
    auto s = this->serialize(foo1);
    auto foo2 = this->template deserialize<LazyStruct>(s);

    EXPECT_FALSE(get_field1(foo1).empty());
    EXPECT_FALSE(get_field2(foo1).empty());
    EXPECT_FALSE(get_field3(foo1).empty());
    EXPECT_FALSE(get_field4(foo1).empty());

    // field3 and field4 are lazy field, thus they are empty
    EXPECT_FALSE(get_field1(foo2).empty());
    EXPECT_FALSE(get_field2(foo2).empty());
    EXPECT_TRUE(get_field3(foo2).empty());
    EXPECT_TRUE(get_field4(foo2).empty());

    EXPECT_EQ(foo1, foo2);
    foo1.field4_ref()->clear();
    EXPECT_NE(foo1, foo2);
    foo2.field4_ref()->clear();
    EXPECT_EQ(foo1, foo2);
  }

  {
    auto foo1 = this->genLazyStruct();
    auto s = this->serialize(foo1);
    auto foo2 = this->template deserialize<LazyStruct>(s);

    foo1.field4_ref()->clear();
    EXPECT_LT(foo1, foo2);
  }

  {
    auto foo1 = this->genLazyStruct();
    foo1.field1_ref()->clear();
    foo1.field2_ref()->clear();
    // Only lazy fields left set.

    auto s = this->serialize(foo1);
    auto foo2 = this->template deserialize<LazyStruct>(s);

    foo2.field4_ref()->clear();
    EXPECT_LT(foo2, foo1);

    foo2 = this->template deserialize<LazyStruct>(s);
    foo2 = {};
    EXPECT_LT(foo2, foo1);
  }
}

// The intention of this unit-test is testing if serialized data doesn't have
// index, we want to ensure it can be still deserialized to struct with lazy
// field with behavior that identical to non-lazy field. i.e. deserialization
// should overwrite fields and clear IOBuf. This can happen during schema
// evolution when user turned off lazy deserialization only on writer side.
TYPED_TEST(LazyDeserialization, DeserializeLazyStructWithoutIndex) {
  using Struct = typename TypeParam::Struct;
  using LazyStruct = typename TypeParam::LazyStruct;

  Struct foo;
  foo.field1_ref().emplace();
  foo.field2_ref().emplace();
  foo.field3_ref().emplace();
  foo.field4_ref().emplace();

  // We need first deserialization since even though `this->genLazyStruct()`
  // returns lazy struct, all lazy fields are already deserialized.
  LazyStruct lazyFoo;
  this->deserialize(this->serialize(this->genLazyStruct()), lazyFoo);
  this->deserialize(this->serialize(foo), lazyFoo);

  if (std::is_same_v<Struct, TerseFoo>) {
    // If struct enabled terse writes, we won't serialize fields that has
    // default value, thus we won't change lazyFoo after deserialization
    EXPECT_EQ(lazyFoo, this->genLazyStruct());
  } else {
    EXPECT_TRUE(lazyFoo.field1_ref()->empty());
    EXPECT_TRUE(lazyFoo.field2_ref()->empty());
    EXPECT_TRUE(lazyFoo.field3_ref()->empty());
    EXPECT_TRUE(lazyFoo.field4_ref()->empty());
  }
}

TYPED_TEST(
    LazyDeserialization, DeserializeLazyStructWithoutIndexOrTerseWrites) {
  using LazyStruct = typename TypeParam::LazyStruct;

  Foo foo;
  LazyStruct lazyFoo;
  this->deserialize(this->serialize(this->genLazyStruct()), lazyFoo);
  this->deserialize(this->serialize(foo), lazyFoo);

  EXPECT_TRUE(lazyFoo.field1_ref()->empty());
  EXPECT_TRUE(lazyFoo.field2_ref()->empty());
  EXPECT_TRUE(lazyFoo.field3_ref()->empty());
  EXPECT_TRUE(lazyFoo.field4_ref()->empty());
}

TYPED_TEST(LazyDeserialization, OptionalField) {
  using Struct = typename TypeParam::Struct;
  using LazyStruct = typename TypeParam::LazyStruct;

  auto s = this->serialize(this->genStruct());
  auto foo = this->template deserialize<Struct>(s);
  auto lazyFoo = this->template deserialize<LazyStruct>(s);

  EXPECT_EQ(foo.field1_ref(), lazyFoo.field1_ref());
  EXPECT_EQ(foo.field2_ref(), lazyFoo.field2_ref());
  EXPECT_EQ(foo.field3_ref(), lazyFoo.field3_ref());
  EXPECT_EQ(foo.field4_ref(), lazyFoo.field4_ref());
}

TYPED_TEST(LazyDeserialization, ReserializeLazyField) {
  using LazyStruct = typename TypeParam::LazyStruct;

  auto foo1 = this->template deserialize<LazyStruct>(
      this->serialize(this->genLazyStruct()));
  auto foo2 = this->template deserialize<LazyStruct>(this->serialize(foo1));

  EXPECT_EQ(foo1.field1_ref(), foo2.field1_ref());
  EXPECT_EQ(foo1.field2_ref(), foo2.field2_ref());
  EXPECT_EQ(foo1.field3_ref(), foo2.field3_ref());
  EXPECT_EQ(foo1.field4_ref(), foo2.field4_ref());
}

TYPED_TEST(LazyDeserialization, SupportedToUnsupportedProtocol) {
  using LazyStruct = typename TypeParam::LazyStruct;

  auto foo1 = this->template deserialize<LazyStruct>(
      this->serialize(this->genLazyStruct()));

  EXPECT_FALSE(get_field1(foo1).empty());
  EXPECT_FALSE(get_field2(foo1).empty());
  EXPECT_TRUE(get_field3(foo1).empty());
  EXPECT_TRUE(get_field4(foo1).empty());

  // Simple JSON doesn't support lazy deserialization
  // All fields will be deserialized
  SimpleJSONSerializer::deserialize(
      SimpleJSONSerializer::serialize<std::string>(Empty{}), foo1);

  EXPECT_FALSE(get_field1(foo1).empty());
  EXPECT_FALSE(get_field2(foo1).empty());
  EXPECT_FALSE(get_field3(foo1).empty());
  EXPECT_FALSE(get_field4(foo1).empty());

  auto foo2 = this->genLazyStruct();

  EXPECT_EQ(foo1.field1_ref(), foo2.field1_ref());
  EXPECT_EQ(foo1.field2_ref(), foo2.field2_ref());
  EXPECT_EQ(foo1.field3_ref(), foo2.field3_ref());
  EXPECT_EQ(foo1.field4_ref(), foo2.field4_ref());
}

TYPED_TEST(LazyDeserialization, ReserializeSameStruct) {
  using LazyStruct = typename TypeParam::LazyStruct;

  auto foo1 = this->template deserialize<LazyStruct>(
      this->serialize(this->genLazyStruct()));

  EXPECT_FALSE(get_field1(foo1).empty());
  EXPECT_FALSE(get_field2(foo1).empty());
  EXPECT_TRUE(get_field3(foo1).empty());
  EXPECT_TRUE(get_field4(foo1).empty());

  // Lazy fields remain undeserialized
  this->deserialize(this->serialize(OptionalFoo{}), foo1);

  EXPECT_FALSE(get_field1(foo1).empty());
  EXPECT_FALSE(get_field2(foo1).empty());
  EXPECT_TRUE(get_field3(foo1).empty());
  EXPECT_TRUE(get_field4(foo1).empty());

  auto foo2 = this->genLazyStruct();

  EXPECT_EQ(foo1.field1_ref(), foo2.field1_ref());
  EXPECT_EQ(foo1.field2_ref(), foo2.field2_ref());
  EXPECT_EQ(foo1.field3_ref(), foo2.field3_ref());
  EXPECT_EQ(foo1.field4_ref(), foo2.field4_ref());
}

TYPED_TEST(LazyDeserialization, DeserializationWithDifferentProtocol) {
  using Serializer = typename TypeParam::Serializer;
  using LazyStruct = typename TypeParam::LazyStruct;
  using Serializer2 = std::conditional_t<
      std::is_same_v<Serializer, CompactSerializer>,
      BinarySerializer,
      CompactSerializer>;

  auto foo = this->template deserialize<LazyStruct>(
      this->serialize(this->genLazyStruct()));

  EXPECT_FALSE(get_field1(foo).empty());
  EXPECT_FALSE(get_field2(foo).empty());
  EXPECT_TRUE(get_field3(foo).empty());
  EXPECT_TRUE(get_field4(foo).empty());

  // Deserialize with same protocol, all fields are untouched
  Serializer::deserialize(
      Serializer::template serialize<std::string>(Empty{}), foo);

  EXPECT_FALSE(get_field1(foo).empty());
  EXPECT_FALSE(get_field2(foo).empty());
  EXPECT_TRUE(get_field3(foo).empty());
  EXPECT_TRUE(get_field4(foo).empty());

  // Deserialize with different protocol, all fields are deserialized
  Serializer2::deserialize(
      Serializer2::template serialize<std::string>(Empty{}), foo);

  EXPECT_FALSE(get_field1(foo).empty());
  EXPECT_FALSE(get_field2(foo).empty());
  EXPECT_FALSE(get_field3(foo).empty());
  EXPECT_FALSE(get_field4(foo).empty());

  EXPECT_EQ(foo, this->genLazyStruct());
}

TEST(Serialization, SerializeWithDifferentProtocolSimple) {
  auto foo = CompactSerializer::deserialize<LazyFoo>(
      CompactSerializer::serialize<std::string>(gen<LazyFoo>()));

  auto foo1 = BinarySerializer::deserialize<LazyFoo>(
      BinarySerializer::serialize<std::string>(foo));

  EXPECT_EQ(foo1, gen<LazyFoo>());
}

TYPED_TEST(LazyDeserialization, SerializationWithSameProtocol) {
  using Serializer = typename TypeParam::Serializer;
  using LazyStruct = typename TypeParam::LazyStruct;

  auto foo = Serializer::template deserialize<LazyStruct>(
      Serializer::template serialize<std::string>(gen<LazyStruct>()));

  auto foo1 = Serializer::template deserialize<LazyStruct>(
      Serializer::template serialize<std::string>(foo));

  // Serialize with same protocol, lazy fields won't be deserialized
  EXPECT_FALSE(get_field1(foo).empty());
  EXPECT_FALSE(get_field2(foo).empty());
  EXPECT_TRUE(get_field3(foo).empty());
  EXPECT_TRUE(get_field4(foo).empty());

  EXPECT_EQ(foo1, gen<LazyStruct>());
}

TYPED_TEST(LazyDeserialization, SerializationWithDifferentProtocol) {
  using Serializer = typename TypeParam::Serializer;
  using LazyStruct = typename TypeParam::LazyStruct;
  using Serializer2 = std::conditional_t<
      std::is_same_v<Serializer, CompactSerializer>,
      BinarySerializer,
      CompactSerializer>;

  auto foo = Serializer::template deserialize<LazyStruct>(
      Serializer::template serialize<std::string>(gen<LazyStruct>()));

  auto foo1 = Serializer2::template deserialize<LazyStruct>(
      Serializer2::template serialize<std::string>(foo));

  EXPECT_FALSE(get_field1(foo).empty());
  EXPECT_FALSE(get_field2(foo).empty());
  EXPECT_FALSE(get_field3(foo).empty());
  EXPECT_FALSE(get_field4(foo).empty());

  EXPECT_EQ(foo1, gen<LazyStruct>());
}

constexpr int kThreadCount = 100;
TYPED_TEST(LazyDeserialization, MultithreadAccess) {
  using LazyStruct = typename TypeParam::LazyStruct;

  auto bar = this->genLazyStruct();
  auto readonlyAccesses = std::make_tuple(
      [&bar](auto& foo) {
        EXPECT_EQ(foo.field1_ref(), bar.field1_ref());
        EXPECT_EQ(foo.field2_ref(), bar.field2_ref());
        EXPECT_EQ(foo.field3_ref(), bar.field3_ref());
        EXPECT_EQ(foo.field4_ref(), bar.field4_ref());
      },
      [&bar](auto& foo) { EXPECT_EQ(foo, bar); },
      [](auto& foo) { LazyStruct baz{foo}; },
      [](auto& foo) {
        LazyStruct baz;
        baz = foo;
      },
      [](auto& foo) { LazyStruct baz{std::move(std::as_const(foo))}; },
      [](auto& foo) {
        LazyStruct baz;
        baz = std::move(std::as_const(foo));
      });

  folly::for_each(readonlyAccesses, [&](auto func) {
    for (int n = 0; n < 2; n++) {
      const bool castToConst = (n == 0);
      auto foo = this->template deserialize<LazyStruct>(
          this->serialize(this->genLazyStruct()));
      std::array<std::thread, kThreadCount> threads;
      for (auto& i : threads) {
        i = std::thread([&] {
          if (castToConst) {
            func(std::as_const(foo));
          } else {
            func(foo);
          }
        });
      }
      for (auto& i : threads) {
        i.join();
      }
      EXPECT_EQ(foo, bar);
    }
  });
}

TYPED_TEST(LazyDeserialization, TestGFlags) {
  using LazyStruct = typename TypeParam::LazyStruct;
  gflags::FlagSaver saver;

  FLAGS_thrift_enable_lazy_deserialization = false;

  auto foo = this->genLazyStruct();
  auto s = this->serialize(foo);
  auto lazyFoo = this->template deserialize<LazyStruct>(s);

  EXPECT_EQ(get_field1(lazyFoo), foo.field1_ref());
  EXPECT_EQ(get_field2(lazyFoo), foo.field2_ref());
  EXPECT_FALSE(get_field3(lazyFoo).empty());
  EXPECT_FALSE(get_field4(lazyFoo).empty());
}

} // namespace apache::thrift::test
