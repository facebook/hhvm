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

#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/type/Field.h>

#include <folly/portability/GTest.h>

namespace apache::thrift::type {
namespace {

TEST(FieldTest, is_optional_field) {
  using apache::thrift::detail::boxed_value;
  // non-optional
  EXPECT_FALSE(is_optional_field<field_ref<int&>>());
  EXPECT_FALSE(is_optional_field<required_field_ref<int&>>());
  EXPECT_FALSE(is_optional_field<terse_field_ref<int&>>());
  EXPECT_FALSE(is_optional_field<intern_boxed_field_ref<boxed_value<int>&>>());
  EXPECT_FALSE(
      is_optional_field<terse_intern_boxed_field_ref<boxed_value<int>&>>());

  // optional
  EXPECT_TRUE(is_optional_field<optional_field_ref<int&>>());
  EXPECT_TRUE(is_optional_field<optional_boxed_field_ref<int&>>());
  EXPECT_TRUE(is_optional_field<union_field_ref<int&>>());
}

} // namespace
} // namespace apache::thrift::type
