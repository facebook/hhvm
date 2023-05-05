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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/test/util/gen-cpp2/enum_types.h>

using namespace apache::thrift::test;
using namespace apache::thrift::util;

TEST(EnumUtilsTest, EnumValue) {
  EXPECT_EQ(enumValueOrThrow<MyEnum>("UNKNOWN"), MyEnum::UNKNOWN);
  EXPECT_EQ(enumValueOrThrow<MyEnum>("VALUE"), MyEnum::VALUE);
  EXPECT_EQ(enumValueOrThrow<MyEnum>("FOO"), MyEnum::FOO);
  EXPECT_EQ(enumValueOrThrow<MyEnum>("BAR"), MyEnum::BAR);
  EXPECT_THROW(enumValueOrThrow<MyEnum>("NOTBAR"), std::out_of_range);
  EXPECT_THROW(enumValueOrThrow<MyEnum>("Universe"), std::out_of_range);
}

TEST(EnumUtilsTest, ShortEnumName) {
  EXPECT_STREQ(enumName((MyEnum)0), "UNKNOWN");
  EXPECT_STREQ(enumName((MyEnum)1), "VALUE");
  EXPECT_STREQ(enumName((MyEnum)2), "FOO");
  EXPECT_STREQ(enumName((MyEnum)3), "BAR");
  EXPECT_STREQ(enumName((MyEnum)3, "NOTBAR"), "BAR");
  EXPECT_STREQ(enumName((MyEnum)42, "Universe"), "Universe");
  EXPECT_EQ(enumName((MyEnum)42), nullptr);
  EXPECT_EQ(enumName((MyEnum)-1), nullptr);
}

TEST(EnumUtilsTest, ShortEnumNameSafe) {
  EXPECT_EQ(enumNameSafe((MyEnum)0), "UNKNOWN");
  EXPECT_EQ(enumNameSafe((MyEnum)1), "VALUE");
  EXPECT_EQ(enumNameSafe((MyEnum)2), "FOO");
  EXPECT_EQ(enumNameSafe((MyEnum)3), "BAR");
  EXPECT_EQ(enumNameSafe((MyEnum)42), "42");
  EXPECT_EQ(enumNameSafe((MyEnum)-1), "-1");
}

TEST(EnumUtilsTest, ShortEnumNameOrThrow) {
  EXPECT_STREQ(enumNameOrThrow((MyEnum)0), "UNKNOWN");
  EXPECT_STREQ(enumNameOrThrow((MyEnum)1), "VALUE");
  EXPECT_STREQ(enumNameOrThrow((MyEnum)2), "FOO");
  EXPECT_STREQ(enumNameOrThrow((MyEnum)3), "BAR");
  EXPECT_THROW(enumNameOrThrow(MyEnum(42)), std::out_of_range);
  EXPECT_THROW(enumNameOrThrow(MyEnum(-1)), std::out_of_range);
}

struct MyStruct {
  constexpr static size_t size = 0;
};
TEST(EnumUtilsTest, IsThriftEnum) {
  EXPECT_TRUE(is_thrift_enum_v<MyEnum>);
  enum class CppEnum {};
  enum CEnum {};
  EXPECT_FALSE(is_thrift_enum_v<CppEnum>);
  EXPECT_FALSE(is_thrift_enum_v<CEnum>);
  EXPECT_FALSE(is_thrift_enum_v<MyStruct>);
}
