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

#include <thrift/compiler/test/fixtures/default_values/src/gen-cpp2/module_types.h>

namespace facebook::thrift::compiler::test::fixtures::default_values {

TEST(DefaultValuesCpp2Test, IntrinsicDefaultValues) {
  StructWithNoCustomDefaultValues s;
  EXPECT_EQ(s.unqualified_integer().value(), 0);
  EXPECT_FALSE(s.optional_integer().has_value());
  EXPECT_EQ(s.required_integer().value(), 0);

  const TrivialStruct& unqualifiedStruct = s.unqualified_struct().value();
  EXPECT_EQ(unqualifiedStruct.int_value().value(), 0);

  EXPECT_FALSE(s.optional_struct().has_value());

  const TrivialStruct& requiredStruct = s.required_struct().value();
  EXPECT_EQ(requiredStruct.int_value().value(), 0);
}

TEST(DefaultValuesCpp2Test, CustomDefaultValues) {
  StructWithCustomDefaultValues s;
  EXPECT_EQ(s.unqualified_integer().value(), 42);
  EXPECT_FALSE(s.optional_integer().has_value());
  EXPECT_EQ(s.required_integer().value(), 44);

  const TrivialStruct& unqualifiedStruct = s.unqualified_struct().value();
  EXPECT_EQ(unqualifiedStruct.int_value().value(), 123);

  EXPECT_FALSE(s.optional_struct().has_value());

  const TrivialStruct& requiredStruct = s.required_struct().value();
  EXPECT_EQ(requiredStruct.int_value().value(), 789);
}

} // namespace facebook::thrift::compiler::test::fixtures::default_values
