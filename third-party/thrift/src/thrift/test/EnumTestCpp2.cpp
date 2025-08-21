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

#include <unordered_set>

#include <gtest/gtest.h>

#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/enum_types.h>

using apache::thrift::TEnumTraits;
using namespace apache::thrift::util;
using namespace cpp2;

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

TEST(EnumTestCpp2, test_enum) {
  // Check that all the enum values match what we expect
  EXPECT_TRUE(TEnumTraits<MyEnum1>::min() == MyEnum1::ME1_0);
  EXPECT_EQ(int(MyEnum1::ME1_0), 0);
  EXPECT_EQ(int(MyEnum1::ME1_1), 1);
  EXPECT_EQ(int(MyEnum1::ME1_2), 2);
  EXPECT_EQ(int(MyEnum1::ME1_3), 3);
  EXPECT_EQ(int(MyEnum1::ME1_5), 5);
  EXPECT_EQ(int(MyEnum1::ME1_6), 6);
  EXPECT_TRUE(TEnumTraits<MyEnum1>::max() == MyEnum1::ME1_6);

  EXPECT_TRUE(TEnumTraits<MyEnum2>::min() == MyEnum2::ME2_0);
  EXPECT_EQ(int(MyEnum2::ME2_0), 0);
  EXPECT_EQ(int(MyEnum2::ME2_1), 1);
  EXPECT_EQ(int(MyEnum2::ME2_2), 2);
  EXPECT_TRUE(TEnumTraits<MyEnum2>::max() == MyEnum2::ME2_2);

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

TEST(EnumTestCpp2, test_enum_constant) {
  MyStruct ms;
  EXPECT_TRUE(*ms.me2_2() == MyEnum2::ME2_2);
  EXPECT_TRUE(*ms.me3_n2() == MyEnum3::ME3_N2);
}

TEST(EnumTestCpp2, test_enum_names) {
  EXPECT_EQ(enumName(MyEnum3::ME3_1), std::string{"ME3_1"});
  EXPECT_EQ(enumName(MyEnum2::ME2_2), std::string{"ME2_2"});
  EXPECT_EQ(enumName(static_cast<MyEnum2>(-10)), (const char*)nullptr);
  EXPECT_EQ(enumName(static_cast<MyEnum2>(-10), "foo"), "foo");
}

TEST(EnumTestCpp2, test_enum_parse) {
  MyEnum2 e2;
  MyEnum3 e3;

  EXPECT_TRUE(tryParseEnum("ME2_2", &e2));
  EXPECT_EQ((int)MyEnum2::ME2_2, (int)e2);
  EXPECT_TRUE(tryParseEnum("ME3_N2", &e3));
  EXPECT_EQ((int)MyEnum3::ME3_N2, (int)e3);

  EXPECT_FALSE(tryParseEnum("FOO_ME2_0", &e2));
  EXPECT_FALSE(tryParseEnum("BAR_ME3_N2", &e3));
}

TEST(EnumTestCpp2, test_unordered_set) {
  std::unordered_set<MyEnum2> stuff;
  stuff.insert(MyEnum2::ME2_0);
  EXPECT_TRUE(stuff.contains(MyEnum2::ME2_0));
  EXPECT_FALSE(stuff.contains(MyEnum2::ME2_1));
}

TEST(EnumTestCpp2, test_hash_specialization) {
  EXPECT_EQ((std::hash<int>()(0)), (std::hash<MyEnum2>()(MyEnum2::ME2_0)));
  EXPECT_NE((std::hash<int>()(0)), (std::hash<MyEnum2>()(MyEnum2::ME2_1)));
}

TEST(EnumTestCpp2, test_equal_to_specialization) {
  EXPECT_TRUE((std::equal_to<MyEnum2>()(MyEnum2::ME2_0, MyEnum2::ME2_0)));
  EXPECT_FALSE((std::equal_to<MyEnum2>()(MyEnum2::ME2_0, MyEnum2::ME2_1)));
}

TEST(EnumTestCpp2, test_unscoped) {
  using MyEnumUnscopedUnderlying =
      typename std::underlying_type<MyEnumUnscoped>::type;
  EXPECT_TRUE((std::is_same<MyEnumUnscopedUnderlying, int>::value));
  MyEnumUnscoped value = {};
  EXPECT_TRUE(tryParseEnum("MEU_A", &value));
  EXPECT_EQ(MyEnumUnscoped::MEU_A, value) << "unscoped usage";
  EXPECT_EQ(int(MyEnumUnscoped::MEU_A), value) << "implicit conversion";
}

TEST(EnumTestCpp2, test_enum_forward_reference) {
  MyStructWithForwardRefEnum obj;
  EXPECT_EQ(MyForwardRefEnum::NONZERO, *obj.a());
  EXPECT_EQ(MyForwardRefEnum::NONZERO, *obj.b());
}

TEST(EnumTestCpp2, test_enum_invalid) {
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

TEST(EnumTestCpp2, test_non_thrift_enum_trait) {
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

TEST(EnumTestCpp2, uri) {
  EXPECT_EQ(apache::thrift::uri<MyEnum2>(), "facebook.com/thrift/test/MyEnum2");
}
