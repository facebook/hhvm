/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/test/reflection/gen-cpp2/reflection_fatal_types.h>

#include <thrift/lib/cpp2/reflection/helpers.h>

#include <thrift/lib/cpp2/reflection/internal/test_helpers.h>

#include <folly/portability/GTest.h>

namespace test_cpp2 {
namespace cpp_reflection {

TEST(reflection_helpers, get_struct_member_by_name) {
  using traits = apache::thrift::reflect_struct<struct1>;

  EXPECT_SAME<
      traits::member::field0,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field0::name,
          fatal::get_type::name>>();
  EXPECT_SAME<
      traits::member::field1,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field1::name,
          fatal::get_type::name>>();
  EXPECT_SAME<
      traits::member::field2,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field2::name,
          fatal::get_type::name>>();
  EXPECT_SAME<
      traits::member::field3,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field3::name,
          fatal::get_type::name>>();
  EXPECT_SAME<
      traits::member::field4,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field4::name,
          fatal::get_type::name>>();
  EXPECT_SAME<
      traits::member::field5,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field5::name,
          fatal::get_type::name>>();

  EXPECT_SAME<
      traits::member::field0,
      apache::thrift::
          get_struct_member_by_name<struct1, traits::member::field0::name>>();
  EXPECT_SAME<
      traits::member::field1,
      apache::thrift::
          get_struct_member_by_name<struct1, traits::member::field1::name>>();
  EXPECT_SAME<
      traits::member::field2,
      apache::thrift::
          get_struct_member_by_name<struct1, traits::member::field2::name>>();
  EXPECT_SAME<
      traits::member::field3,
      apache::thrift::
          get_struct_member_by_name<struct1, traits::member::field3::name>>();
  EXPECT_SAME<
      traits::member::field4,
      apache::thrift::
          get_struct_member_by_name<struct1, traits::member::field4::name>>();
  EXPECT_SAME<
      traits::member::field5,
      apache::thrift::
          get_struct_member_by_name<struct1, traits::member::field5::name>>();
}

TEST(reflection_helpers, get_struct_member_by_id) {
  using traits = apache::thrift::reflect_struct<struct1>;

  EXPECT_SAME<
      traits::member::field0,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field0::id,
          fatal::get_type::id>>();
  EXPECT_SAME<
      traits::member::field1,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field1::id,
          fatal::get_type::id>>();
  EXPECT_SAME<
      traits::member::field2,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field2::id,
          fatal::get_type::id>>();
  EXPECT_SAME<
      traits::member::field3,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field3::id,
          fatal::get_type::id>>();
  EXPECT_SAME<
      traits::member::field4,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field4::id,
          fatal::get_type::id>>();
  EXPECT_SAME<
      traits::member::field5,
      apache::thrift::get_struct_member_by<
          struct1,
          traits::member::field5::id,
          fatal::get_type::id>>();

  EXPECT_SAME<
      traits::member::field0,
      apache::thrift::get_struct_member_by_id<
          struct1,
          traits::member::field0::id::value>>();
  EXPECT_SAME<
      traits::member::field1,
      apache::thrift::get_struct_member_by_id<
          struct1,
          traits::member::field1::id::value>>();
  EXPECT_SAME<
      traits::member::field2,
      apache::thrift::get_struct_member_by_id<
          struct1,
          traits::member::field2::id::value>>();
  EXPECT_SAME<
      traits::member::field3,
      apache::thrift::get_struct_member_by_id<
          struct1,
          traits::member::field3::id::value>>();
  EXPECT_SAME<
      traits::member::field4,
      apache::thrift::get_struct_member_by_id<
          struct1,
          traits::member::field4::id::value>>();
  EXPECT_SAME<
      traits::member::field5,
      apache::thrift::get_struct_member_by_id<
          struct1,
          traits::member::field5::id::value>>();
}

} // namespace cpp_reflection
} // namespace test_cpp2
