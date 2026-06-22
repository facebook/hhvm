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

#include <gtest/gtest.h>

#include <folly/Utility.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/enum_types.h>

using apache::thrift::TEnumTraits;
using namespace apache::thrift::util;
using namespace cpp2;

using MyEnum2Types = ::testing::
    Types<MyEnum2, MyEnum2I8, MyEnum2U8, MyEnum2I16, MyEnum2U16, MyEnum2U32>;

template <typename T>
class MyEnum2Test : public ::testing::Test {};
TYPED_TEST_SUITE(MyEnum2Test, MyEnum2Types);

TYPED_TEST(MyEnum2Test, IsThriftEnum) {
  static_assert(std::is_enum_v<TypeParam>);
  static_assert(is_thrift_enum_v<TypeParam>);
}

TYPED_TEST(MyEnum2Test, Values) {
  EXPECT_EQ(TEnumTraits<TypeParam>::min(), TypeParam::ME2_0);
  EXPECT_EQ(folly::to_underlying(TypeParam::ME2_0), 0);
  EXPECT_EQ(folly::to_underlying(TypeParam::ME2_1), 1);
  EXPECT_EQ(folly::to_underlying(TypeParam::ME2_2), 2);
  EXPECT_EQ(TEnumTraits<TypeParam>::max(), TypeParam::ME2_2);
}

TYPED_TEST(MyEnum2Test, Names) {
  EXPECT_STREQ(enumName(TypeParam::ME2_0), "ME2_0");
  EXPECT_STREQ(enumName(TypeParam::ME2_1), "ME2_1");
  EXPECT_STREQ(enumName(TypeParam::ME2_2), "ME2_2");
  EXPECT_EQ(enumName(static_cast<TypeParam>(-10)), (const char*)nullptr);
  EXPECT_STREQ(enumName(static_cast<TypeParam>(-10), "foo"), "foo");
}

TYPED_TEST(MyEnum2Test, EnumValueOrThrow) {
  EXPECT_EQ(enumValueOrThrow<TypeParam>("ME2_0"), TypeParam::ME2_0);
  EXPECT_EQ(enumValueOrThrow<TypeParam>("ME2_1"), TypeParam::ME2_1);
  EXPECT_EQ(enumValueOrThrow<TypeParam>("ME2_2"), TypeParam::ME2_2);
  EXPECT_THROW(enumValueOrThrow<TypeParam>("INVALID"), std::out_of_range);
}

TYPED_TEST(MyEnum2Test, EnumNameSafe) {
  EXPECT_EQ(enumNameSafe(TypeParam::ME2_0), "ME2_0");
  EXPECT_EQ(enumNameSafe(TypeParam::ME2_1), "ME2_1");
  EXPECT_EQ(enumNameSafe(TypeParam::ME2_2), "ME2_2");
  EXPECT_EQ(enumNameSafe(static_cast<TypeParam>(10)), "10");
}

TYPED_TEST(MyEnum2Test, EnumNameOrThrow) {
  EXPECT_STREQ(enumNameOrThrow(TypeParam::ME2_0), "ME2_0");
  EXPECT_STREQ(enumNameOrThrow(TypeParam::ME2_1), "ME2_1");
  EXPECT_STREQ(enumNameOrThrow(TypeParam::ME2_2), "ME2_2");
  EXPECT_THROW(enumNameOrThrow(static_cast<TypeParam>(-10)), std::out_of_range);
}

TYPED_TEST(MyEnum2Test, TryParseEnum) {
  TypeParam e;
  EXPECT_TRUE(tryParseEnum("ME2_2", &e));
  EXPECT_EQ(e, TypeParam::ME2_2);
  EXPECT_FALSE(tryParseEnum("INVALID_ENUM_NAME", &e));
}

TYPED_TEST(MyEnum2Test, TryGetEnumName) {
  auto name = tryGetEnumName(TypeParam::ME2_2);
  ASSERT_TRUE(name.has_value());
  EXPECT_EQ(*name, "ME2_2");
  EXPECT_FALSE(tryGetEnumName(static_cast<TypeParam>(-10)).has_value());
}

TYPED_TEST(MyEnum2Test, HashSpecialization) {
  using U = std::underlying_type_t<TypeParam>;
  EXPECT_EQ((std::hash<U>()(0)), (std::hash<TypeParam>()(TypeParam::ME2_0)));
  EXPECT_NE((std::hash<U>()(0)), (std::hash<TypeParam>()(TypeParam::ME2_1)));
}

TYPED_TEST(MyEnum2Test, EqualToSpecialization) {
  std::equal_to<TypeParam> eq;
  EXPECT_TRUE(eq(TypeParam::ME2_0, TypeParam::ME2_0));
  EXPECT_FALSE(eq(TypeParam::ME2_0, TypeParam::ME2_1));
}

template <typename E, typename = void>
constexpr bool t_enum_trait_has_min_v = false;
template <typename E>
constexpr bool
    t_enum_trait_has_min_v<E, folly::void_t<decltype(TEnumTraits<E>::min())>> =
        true;

template <typename E, typename = void>
constexpr bool t_enum_trait_has_max_v = false;
template <typename E>
constexpr bool
    t_enum_trait_has_max_v<E, folly::void_t<decltype(TEnumTraits<E>::max())>> =
        true;

TEST(EnumTestCpp2, Enum) {
  // Check that all the enum values match what we expect
  EXPECT_TRUE(TEnumTraits<MyEnum1>::min() == MyEnum1::ME1_0);
  EXPECT_EQ(int(MyEnum1::ME1_0), 0);
  EXPECT_EQ(int(MyEnum1::ME1_1), 1);
  EXPECT_EQ(int(MyEnum1::ME1_2), 2);
  EXPECT_EQ(int(MyEnum1::ME1_3), 3);
  EXPECT_EQ(int(MyEnum1::ME1_5), 5);
  EXPECT_EQ(int(MyEnum1::ME1_6), 6);
  EXPECT_TRUE(TEnumTraits<MyEnum1>::max() == MyEnum1::ME1_6);

  EXPECT_TRUE(TEnumTraits<MyEnum3>::min() == MyEnum3::ME3_N2);
  EXPECT_EQ(int(MyEnum3::ME3_0), 0);
  EXPECT_EQ(int(MyEnum3::ME3_1), 1);
  EXPECT_EQ(int(MyEnum3::ME3_N2), -2);
  EXPECT_EQ(int(MyEnum3::ME3_N1), -1);
  EXPECT_EQ(int(MyEnum3::ME3_9), 9);
  EXPECT_EQ(int(MyEnum3::ME3_10), 10);
  EXPECT_TRUE(TEnumTraits<MyEnum3>::max() == MyEnum3::ME3_10);

  EXPECT_TRUE(TEnumTraits<MyEnum4>::min() == MyEnum4::ME4_A);
  EXPECT_EQ(int(MyEnum4::ME4_A), 0x7ffffffd);
  EXPECT_EQ(int(MyEnum4::ME4_B), 0x7ffffffe);
  EXPECT_EQ(int(MyEnum4::ME4_C), 0x7fffffff);
  EXPECT_TRUE(TEnumTraits<MyEnum4>::max() == MyEnum4::ME4_C);

  EXPECT_TRUE(TEnumTraits<MyUnion::Type>::min() == MyUnion::Type::first_field);
  EXPECT_EQ(int(MyUnion::Type::first_field), 1);
  EXPECT_EQ(int(MyUnion::Type::i32_field), 2);
  EXPECT_EQ(int(MyUnion::Type::last_field), 5);
  EXPECT_TRUE(TEnumTraits<MyUnion::Type>::max() == MyUnion::Type::last_field);

  EXPECT_FALSE(t_enum_trait_has_max_v<EmptyUnion::Type>);
  EXPECT_FALSE(t_enum_trait_has_min_v<EmptyUnion::Type>);
  EXPECT_FALSE(t_enum_trait_has_max_v<EmptyEnum>);
  EXPECT_FALSE(t_enum_trait_has_min_v<EmptyEnum>);
}

TEST(EnumTestCpp2, Constant) {
  MyStruct ms;
  EXPECT_TRUE(*ms.me2_2() == MyEnum2::ME2_2);
  EXPECT_TRUE(*ms.me3_n2() == MyEnum3::ME3_N2);
}

TEST(EnumTestCpp2, Names) {
  EXPECT_EQ(enumName(MyEnum3::ME3_1), std::string{"ME3_1"});
}

TEST(EnumTestCpp2, Parse) {
  MyEnum3 e3;
  EXPECT_TRUE(tryParseEnum("ME3_N2", &e3));
  EXPECT_EQ((int)MyEnum3::ME3_N2, (int)e3);
  EXPECT_FALSE(tryParseEnum("BAR_ME3_N2", &e3));
}

TEST(EnumTestCpp2, Unscoped) {
  using MyEnumUnscopedUnderlying = std::underlying_type_t<MyEnumUnscoped>;
  EXPECT_TRUE((std::is_same<MyEnumUnscopedUnderlying, int>::value));
  MyEnumUnscoped value = {};
  EXPECT_TRUE(tryParseEnum("MEU_A", &value));
  EXPECT_EQ(MyEnumUnscoped::MEU_A, value) << "unscoped usage";
  EXPECT_EQ(int(MyEnumUnscoped::MEU_A), value) << "implicit conversion";
}

TEST(EnumTestCpp2, ForwardReference) {
  MyStructWithForwardRefEnum obj;
  EXPECT_EQ(MyForwardRefEnum::NONZERO, *obj.a());
  EXPECT_EQ(MyForwardRefEnum::NONZERO, *obj.b());
}

TEST(EnumTestCpp2, Invalid) {
  MyStruct ms;
  ms.me1_t1() = static_cast<MyEnum1>(42); // out of range
  ms.me1_nodefault() = static_cast<MyEnum1>(42);
  ms.me1_optional() = static_cast<MyEnum1>(42);
  auto str = apache::thrift::CompactSerializer::serialize<std::string>(ms);
  auto ms2 = apache::thrift::CompactSerializer::deserialize<MyStruct>(str);
  EXPECT_EQ(*ms2.me1_t1(), static_cast<MyEnum1>(42));
  EXPECT_EQ(*ms2.me1_nodefault(), static_cast<MyEnum1>(42));
  EXPECT_TRUE(ms2.me1_optional().has_value());
  EXPECT_EQ(*ms2.me1_optional(), static_cast<MyEnum1>(42));
}

namespace {

enum NonThriftEnum {};

struct HasType {
  using type = void;
};

template <typename T, typename = folly::void_t<>>
struct has_type_member : std::false_type {};

template <typename T>
struct has_type_member<T, folly::void_t<typename T::type>> : std::true_type {};

} // namespace

TEST(EnumTestCpp2, NonThriftEnumTraits) {
  EXPECT_TRUE(has_type_member<HasType>::value);
  EXPECT_FALSE(has_type_member<TEnumTraits<NonThriftEnum>>::value);
  EXPECT_TRUE(has_type_member<TEnumTraits<MyEnum1>>::value);
}

TEST(EnumTestCpp2, UnderlyingType) {
  EXPECT_TRUE((std::is_same_v<std::underlying_type_t<I8>, std::int8_t>));
  EXPECT_TRUE((std::is_same_v<std::underlying_type_t<U8>, std::uint8_t>));
  EXPECT_TRUE((std::is_same_v<std::underlying_type_t<I16>, std::int16_t>));
  EXPECT_TRUE((std::is_same_v<std::underlying_type_t<U16>, std::uint16_t>));
  EXPECT_TRUE((std::is_same_v<std::underlying_type_t<U32>, std::uint32_t>));
}

TEST(EnumTestCpp2, Uri) {
  EXPECT_EQ(apache::thrift::uri<MyEnum2>(), "facebook.com/thrift/test/MyEnum2");
}
