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
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/test/reflection/gen-cpp2/troublesome_types.h>

namespace apache::thrift::test::reflection {
namespace {

// Test that types with "troublesome" names (names that could conflict with
// internal reflection names) work correctly with modern always-on reflection.
TEST(TroublesomeTest, ModernReflection) {
  // Verify struct names
  EXPECT_EQ(op::get_class_name_v<strings>, "strings");
  EXPECT_EQ(op::get_class_name_v<structs>, "structs");
  EXPECT_EQ(op::get_class_name_v<name>, "name");
  EXPECT_EQ(op::get_class_name_v<members>, "members");

  // Verify union names
  EXPECT_EQ(op::get_class_name_v<unions>, "unions");
  EXPECT_EQ(op::get_class_name_v<member>, "member");

  // Verify exception names
  EXPECT_EQ(op::get_class_name_v<exceptions>, "exceptions");
}

} // namespace
} // namespace apache::thrift::test::reflection
