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

#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/test/reflection/gen-cpp2/reflection_types.h>

#include <gtest/gtest.h>
#include <folly/Overload.h>

namespace test_cpp2::cpp_reflection {
namespace {

TEST(struct1, ModifyField) {
  struct1 s;
  s.field4().emplace().set_us("foo");
  s.field5().emplace().set_us_2("bar");
  auto run = folly::overload(
      [](union1& ref) {
        EXPECT_EQ(ref.get_us(), "foo");
        ref.set_ui(20);
      },
      [](auto&) { EXPECT_TRUE(false) << "type mismatch"; });

  // Visit field with ID 16 (field4)
  apache::thrift::op::invoke_by_field_id<struct1>(
      apache::thrift::FieldId{16},
      [&](auto id) {
        auto ref = apache::thrift::op::get<decltype(id)>(s);
        EXPECT_TRUE(ref.has_value());
        run(*ref);
      },
      [&]() { FAIL() << "Invalid field ID"; });
  EXPECT_EQ(s.field4()->ui(), 20);
  EXPECT_EQ(s.field5()->us_2(), "bar");

  // Test invalid field ID - should call the fallback handler
  EXPECT_THROW(
      apache::thrift::op::invoke_by_field_id<struct1>(
          static_cast<apache::thrift::FieldId>(123456),
          [&](auto) {},
          [&]() { throw std::out_of_range("Invalid thrift id"); }),
      std::out_of_range);
}
} // namespace
} // namespace test_cpp2::cpp_reflection
