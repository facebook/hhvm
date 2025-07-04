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
#include <thrift/test/gen-cpp2/MixinTestStrict_types.h>
#include <thrift/test/gen-cpp2/MixinTest_types.h>

template <class S>
void mixinSimpleTest() {
  S s;
  s.m2()->m1()->field1() = "1";
  s.m2()->field2() = "2";
  s.m3()->field3() = "3";
  s.field4() = "4";
  s.u()->field5_ref() = "5"; // union still requires _ref suffix
  EXPECT_EQ(s.field1().value(), "1");
  EXPECT_EQ(s.field2().value(), "2");
  EXPECT_EQ(s.field3().value(), "3");
  EXPECT_EQ(s.field4().value(), "4");
  EXPECT_EQ(s.field5().value(), "5");
  EXPECT_FALSE(s.field6().has_value());

  EXPECT_EQ(
      *apache::thrift::detail::invoke_reffer<apache::thrift::ident::field1>{}(
          s),
      "1");
  EXPECT_EQ(
      *apache::thrift::detail::invoke_reffer<apache::thrift::ident::field2>{}(
          s),
      "2");
  EXPECT_EQ(
      *apache::thrift::detail::invoke_reffer<apache::thrift::ident::field3>{}(
          s),
      "3");
  EXPECT_EQ(
      *apache::thrift::detail::invoke_reffer<apache::thrift::ident::field4>{}(
          s),
      "4");
  EXPECT_EQ(
      *apache::thrift::detail::invoke_reffer<apache::thrift::ident::field5>{}(
          s),
      "5");

  s.u()->field6_ref() = "6";
  EXPECT_FALSE(s.field5().has_value());
  EXPECT_EQ(s.field6().value(), "6");

  s.field1() = "11";
  s.field2() = "22";
  s.field3() = "33";
  s.field4() = "44";
  s.field5() = "55";

  EXPECT_EQ(s.m2()->m1()->field1().value(), "11");
  EXPECT_EQ(s.m2()->field2().value(), "22");
  EXPECT_EQ(s.m3()->field3().value(), "33");
  EXPECT_EQ(s.field4().value(), "44");
  EXPECT_EQ(s.u()->field5_ref().value(), "55");
  EXPECT_FALSE(s.u()->field6_ref().has_value());

  s.field6() = "66";
  EXPECT_FALSE(s.u()->field5_ref().has_value());
  EXPECT_EQ(s.u()->field6_ref().value(), "66");
}

TEST(Mixin, Simple) {
  mixinSimpleTest<cpp2::Foo>();
  mixinSimpleTest<thrift::test::strict::Foo>();
}

TEST(Mixin, WithRefSuffix) {
  cpp2::Foo s;
  s.m2()->m1()->field1() = "1";
  s.m2()->field2() = "2";
  s.m3()->field3() = "3";
  s.field4() = "4";
  s.u()->field5() = "5";
  EXPECT_EQ(s.field1().value(), "1");
  EXPECT_EQ(s.field2().value(), "2");
  EXPECT_EQ(s.field3().value(), "3");
  EXPECT_EQ(s.field4().value(), "4");
  EXPECT_EQ(s.field5().value(), "5");
  EXPECT_FALSE(s.field6().has_value());
}

TEST(Mixin, Type) {
  cpp2::Foo foo;
  static_assert(std::is_same_v<
                decltype(foo.field1()),
                apache::thrift::field_ref<std::string&>>);
  static_assert(std::is_same_v<
                decltype(std::as_const(foo).field1()),
                apache::thrift::field_ref<const std::string&>>);
  static_assert(std::is_same_v<
                decltype(std::move(foo).field1()),
                apache::thrift::field_ref<std::string&&>>);
  static_assert(std::is_same_v<
                decltype(std::move(std::as_const(foo)).field1()),
                apache::thrift::field_ref<const std::string&>>);
}
