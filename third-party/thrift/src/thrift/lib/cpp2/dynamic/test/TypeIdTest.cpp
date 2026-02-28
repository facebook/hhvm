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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thrift/lib/cpp2/dynamic/TypeId.h>
#include <thrift/lib/cpp2/type/Any.h>

#include <type_traits>

namespace apache::thrift::type_system {

using t = apache::thrift::type_system::TypeIds;

static_assert(std::is_copy_constructible_v<TypeId>);
static_assert(std::is_copy_assignable_v<TypeId>);
static_assert(std::is_move_constructible_v<TypeId>);
static_assert(std::is_move_assignable_v<TypeId>);

TEST(TypeIdTest, PrimitiveEquality) {
  EXPECT_EQ(TypeId(TypeId::Bool()), TypeId::Bool());
  EXPECT_EQ(TypeId(TypeId::I32()), TypeId(TypeId::I32()));
  EXPECT_NE(TypeId(TypeId::Bool()), TypeId::I32());
  EXPECT_NE(TypeId::Any(), TypeId(TypeId::Binary()));
}

TEST(TypeIdTest, CopyAndMove) {
  TypeId original = t::I32;
  TypeId copy = original;
  EXPECT_EQ(copy, original);

  TypeId moved = std::move(original);
  EXPECT_EQ(moved, t::I32);
}

TEST(TypeIdTest, ContainerEquality) {
  EXPECT_EQ(t::list(t::Bool), t::list(t::Bool));
  EXPECT_NE(t::list(t::I16), t::list(t::Bool));

  EXPECT_EQ(t::set(t::set(t::String)), t::set(t::set(t::String)));
  EXPECT_NE(t::set(t::set(t::String)), t::set(t::list(t::String)));

  EXPECT_EQ(
      t::map(t::I32, t::map(t::I32, t::Bool)),
      t::map(t::I32, t::map(t::I32, t::Bool)));
  EXPECT_NE(
      t::map(t::I32, t::map(t::I32, t::Bool)),
      t::map(t::I32, t::map(t::String, t::Bool)));

  EXPECT_NE(t::list(t::Bool), t::Bool);
}

TEST(TypeIdTest, TestKind) {
  EXPECT_TRUE(t::Bool.isBool());
  EXPECT_TRUE(t::Bool.isType<TypeId::Bool>());
  EXPECT_FALSE(t::Bool.isByte());
  EXPECT_FALSE(t::Bool.isType<TypeId::Byte>());

  EXPECT_TRUE(t::Byte.isByte());
  EXPECT_FALSE(t::Byte.isI16());

  EXPECT_TRUE(t::I16.isI16());
  EXPECT_FALSE(t::I16.isI32());

  EXPECT_TRUE(t::I32.isI32());
  EXPECT_FALSE(t::I32.isI64());

  EXPECT_TRUE(t::I64.isI64());
  EXPECT_FALSE(t::I64.isFloat());

  EXPECT_TRUE(t::Float.isFloat());
  EXPECT_FALSE(t::Float.isDouble());

  EXPECT_TRUE(t::Double.isDouble());
  EXPECT_FALSE(t::Double.isString());

  EXPECT_TRUE(t::String.isString());
  EXPECT_FALSE(t::String.isBinary());

  EXPECT_TRUE(t::Binary.isBinary());
  EXPECT_FALSE(t::Binary.isAny());

  EXPECT_TRUE(t::Any.isAny());
  EXPECT_FALSE(t::Any.isBool());

  EXPECT_EQ(
      t::uri("meta.com/thrift/Test").asUri(), Uri("meta.com/thrift/Test"));
  EXPECT_FALSE(t::uri("meta.com/thrift/Test").isList());
  EXPECT_THROW(t::uri("meta.com/thrift/Test").asList(), std::runtime_error);
  EXPECT_THAT(
      [&] { t::uri("meta.com/thrift/Test").asType<TypeId::List>(); },
      testing::ThrowsMessage<std::runtime_error>(
          testing::HasSubstr("userDefinedType")));

  EXPECT_EQ(t::list(t::Bool).asList(), TypeId::List(t::Bool));
  EXPECT_FALSE(t::list(t::Bool).isSet());

  EXPECT_EQ(t::set(t::I32).asSet(), TypeId::Set(t::I32));
  EXPECT_FALSE(t::set(t::I32).isMap());

  EXPECT_EQ(t::map(t::String, t::I64).asMap(), TypeId::Map(t::String, t::I64));
  EXPECT_FALSE(t::map(t::String, t::I64).isList());
}

TEST(TypeIdTest, UriEquality) {
  EXPECT_NE(t::uri("meta.com/thrift/Test"), t::Bool);
  EXPECT_EQ(t::uri("meta.com/thrift/Test"), t::uri("meta.com/thrift/Test"));
  EXPECT_EQ(t::uri("meta.com/thrift/Test"), Uri("meta.com/thrift/Test"));
  EXPECT_NE(t::uri("meta.com/thrift/Test"), Uri("meta.com/thrift/OtherStruct"));
}

TEST(TypeIdTest, ComplexEquality) {
  EXPECT_EQ(
      t::map(t::String, t::list(t::I32)), t::map(t::String, t::list(t::I32)));
  EXPECT_NE(
      t::map(t::String, t::list(t::I32)), t::map(t::String, t::set(t::I32)));
}

TEST(TypeIdTest, Names) {
  EXPECT_EQ(t::I64.name(), "i64");
  EXPECT_EQ(t::Double.name(), "double");
  EXPECT_EQ(t::String.name(), "string");
  EXPECT_EQ(t::set(t::Byte).name(), "set<byte>");
  EXPECT_EQ(t::list(t::Any).name(), "list<any>");
  EXPECT_EQ(t::map(t::String, t::I64).name(), "map<string, i64>");
  EXPECT_EQ(t::set(t::set(t::Binary)).name(), "set<set<binary>>");
  EXPECT_EQ(
      t::map(t::I32, t::list(t::Double)).name(), "map<i32, list<double>>");
  EXPECT_EQ(
      t::uri("example.com/thrift/Example").name(),
      "example.com/thrift/Example");
  EXPECT_EQ(
      t::map(t::I32, t::uri("example.com/thrift/AnotherExample")).name(),
      "map<i32, example.com/thrift/AnotherExample>");
}

TEST(TypeIdTest, TagToTypeId) {
  EXPECT_EQ(t::Bool, tagToTypeId(type::bool_t{}));
  EXPECT_EQ(t::Byte, tagToTypeId(type::byte_t{}));
  EXPECT_EQ(t::I16, tagToTypeId(type::i16_t{}));
  EXPECT_EQ(t::I32, tagToTypeId(type::i32_t{}));
  EXPECT_EQ(t::I64, tagToTypeId(type::i64_t{}));
  EXPECT_EQ(t::Float, tagToTypeId(type::float_t{}));
  EXPECT_EQ(t::Double, tagToTypeId(type::double_t{}));
  EXPECT_EQ(t::String, tagToTypeId(type::string_t{}));
  EXPECT_EQ(t::Binary, tagToTypeId(type::binary_t{}));
  EXPECT_EQ(t::Any, tagToTypeId(type::infer_tag<type::AnyStruct>{}));
  EXPECT_EQ(t::Any, tagToTypeId(type::infer_tag<type::AnyData>{}));
  EXPECT_EQ(t::list(t::Bool), tagToTypeId(type::list<type::bool_t>{}));
  EXPECT_EQ(t::set(t::I32), tagToTypeId(type::set<type::i32_t>{}));
  EXPECT_EQ(
      t::map(t::String, t::I64),
      tagToTypeId(type::map<type::string_t, type::i64_t>{}));
}

TEST(TypeIdTest, Hash) {
  std::unordered_set<TypeId> set;

  auto testFn = [&](auto val) {
    set.insert(val);
    EXPECT_TRUE(set.contains(val));
    set.erase(val);
    EXPECT_FALSE(set.contains(val));
  };

  testFn(t::Bool);
  testFn(t::Byte);
  testFn(t::I16);
  testFn(t::I32);
  testFn(t::I64);
  testFn(t::Float);
  testFn(t::Double);
  testFn(t::String);
  testFn(t::Binary);
  testFn(t::Any);
  testFn(t::list(t::Bool));
  testFn(t::set(t::I32));
  testFn(t::map(t::String, t::I64));
  testFn(t::uri("meta.com/thrift/Test"));
}
} // namespace apache::thrift::type_system
