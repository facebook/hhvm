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

#include <optional>
#include <thrift/lib/cpp2/util/gtest/Matcher.h>
#include <thrift/lib/cpp2/util/gtest/test/gen-cpp2/Matcher_types.h>

#include <folly/portability/GTest.h>

// portability/GTest must be imported before any other gtest header
#include <gtest/gtest-spi.h>

using apache::thrift::test::IsThriftUnionWith;
using apache::thrift::test::Person;
using apache::thrift::test::Result;
using apache::thrift::test::SameType;
using apache::thrift::test::ThriftField;
using testing::_;
using testing::Eq;
using testing::Not;
using testing::Optional;

TEST(MatcherTest, ThriftField) {
  auto p = Person();
  p.name() = "Zaphod";
  EXPECT_THAT(p, ThriftField(&Person::name<>, Eq("Zaphod")));
  p.id() = 42;
  EXPECT_THAT(p, ThriftField(&Person::id<>, Eq(42)));
}

TEST(MatcherTestThriftMacher, FieldRef) {
  namespace field = apache::thrift::ident;
  int value = 42;

  auto p = Person();
  p.id() = value;
  EXPECT_THAT(p, ThriftField<field::id>(Eq(value)));
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(p, ThriftField<field::id>(Eq(value + 1))), "");
}

TEST(MatcherTestThriftMacher, OptionalRef) {
  namespace field = apache::thrift::ident;
  std::string value = "Zaphod";
  std::string wrong = "wrong";

  auto p = Person();
  EXPECT_THAT(p, ThriftField<field::name>(Eq(std::nullopt)));
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(p, ThriftField<field::name>(Optional(_))), "");
  // An unset optional_field_ref always compares false to the inner type
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(p, ThriftField<field::name>(wrong)), "");

  p.name() = value;
  EXPECT_THAT(p, ThriftField<field::name>(value));
  EXPECT_THAT(p, ThriftField<field::name>(Optional(Eq(value))));
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(p, ThriftField<field::name>(Eq(wrong))), "");
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(p, ThriftField<field::name>(Optional(Eq(wrong)))), "");
}

TEST(MatcherTest, FiledRefPrintsCorrectly) {
  auto p = Person();
  EXPECT_EQ(testing::PrintToString(p.name_ref()), "empty optional_field_ref");
  p.name_ref() = "Zaphod";
  EXPECT_EQ(
      testing::PrintToString(p.name_ref()),
      "optional_field_ref holding \"Zaphod\"");
  p.id_ref() = 42;
  EXPECT_EQ(testing::PrintToString(p.id_ref()), "field_ref holding 42");
}

TEST(ThriftMacherUnion, MatchesIfActiveMemberIsCorrectAndInnerMatcherMatches) {
  namespace field = apache::thrift::ident;
  auto r = Result();

  int value = 42;
  r.success_ref() = value;
  EXPECT_THAT(r, IsThriftUnionWith<field::success>(value));
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(r, IsThriftUnionWith<field::success>(Not(value))), "");
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(r, IsThriftUnionWith<field::error>(_)), "");

  std::string error = "error";
  r.error_ref() = error;
  EXPECT_THAT(r, IsThriftUnionWith<field::error>(error));
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(r, IsThriftUnionWith<field::error>(Not(error))), "");
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(r, IsThriftUnionWith<field::success>(_)), "");
}

TEST(ThriftMacherUnion, Given_IsUnset_Then_NoTagMatches) {
  namespace field = apache::thrift::ident;

  auto r = Result();
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(r, IsThriftUnionWith<field::success>(_)), "");
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(r, IsThriftUnionWith<field::error>(_)), "");
}

TEST(
    ThriftMacherUnion,
    Given_UnionHasSameTypes_And_IsSet_Then_TagForActiveMemberMatches) {
  namespace field = apache::thrift::ident;

  auto r = SameType();
  r.b_ref() = "b";
  EXPECT_THAT(r, IsThriftUnionWith<field::b>(_));
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(r, IsThriftUnionWith<field::a>(_)), "");
}

TEST(MatcherTestThriftMacher, CanBindToRValueMatcher) {
  namespace field = apache::thrift::ident;
  testing::Matcher<Person&&> _ = ThriftField<field::id>(testing::_);
}

TEST(ThriftMacherUnion, CanBindToRValueMatcher) {
  namespace field = apache::thrift::ident;
  testing::Matcher<SameType&&> _ = IsThriftUnionWith<field::a>(testing::_);
}
