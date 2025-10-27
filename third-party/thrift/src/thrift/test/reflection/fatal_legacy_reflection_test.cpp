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

#include <thrift/lib/cpp2/reflection/legacy_reflection.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/Traits.h>

#include <thrift/lib/thrift/gen-cpp2/reflection_fatal_types.h>
#include <thrift/test/reflection/gen-cpp2/fatal_legacy_reflection_fatal_types.h>
#include <thrift/test/reflection/gen-cpp2/fatal_legacy_reflection_types.h>

using namespace apache::thrift;
using namespace apache::thrift::test;

TEST(FatalLegacyReflectionTest, name) {
  const auto actual = legacy_reflection<SampleStruct>::name();
  EXPECT_EQ("struct fatal_legacy_reflection.SampleStruct", actual);
}

TEST(FatalLegacyReflectionTest, schema) {
  using type = SampleStruct;
  ASSERT_FALSE(
      std::is_unsigned<
          folly::remove_cvref_t<decltype(*type().i16_field())>>::value)
      << "sanity";
  ASSERT_TRUE(
      std::is_unsigned<
          folly::remove_cvref_t<decltype(*type().ui16_field())>>::value)
      << "sanity";
  constexpr auto name = "struct fatal_legacy_reflection.SampleStruct";
  auto schema = legacy_reflection<type>::schema();
  EXPECT_THAT(*schema.names(), testing::Contains(testing::Key(name)));
}
