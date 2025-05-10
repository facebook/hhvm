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

#include <thrift/test/gen-cpp2/field_ref_codegen_types.h>

#include <gtest/gtest.h>

using namespace apache::thrift::test;

TEST(field_ref_codegen_test, optional_getter) {
  test_struct s;
  apache::thrift::optional_field_ref<int64_t&> ref = s.foo();
  EXPECT_FALSE(ref.has_value());
  ref = 42;
  EXPECT_TRUE(ref.has_value());
  EXPECT_EQ(*ref, 42);
}

TEST(field_ref_codegen_test, getter) {
  test_struct s;
  apache::thrift::field_ref<int64_t&> ref = s.bar();
  EXPECT_FALSE(ref.is_set());
  ref = 42;
  EXPECT_TRUE(ref.is_set());
  EXPECT_EQ(*ref, 42);
}
