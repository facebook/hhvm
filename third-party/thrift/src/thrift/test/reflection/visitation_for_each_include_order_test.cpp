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

// Test whether it works if we include for_each.h first.
#include <thrift/lib/cpp2/visitation/for_each.h>

#include <thrift/test/reflection/gen-cpp2/reflection_for_each_field.h>

#include <gtest/gtest.h>

using namespace apache::thrift;
using namespace test_cpp2::cpp_reflection;

TEST(structA, test) {
  structA s;
  s.a() = 1;
  s.b() = "1";
  for_each_field(s, [](auto&&, auto ref) {
    EXPECT_EQ(folly::to<std::string>(*ref), "1");
    ref = folly::to<typename decltype(ref)::value_type>(2);
  });
  EXPECT_EQ(s.a(), 2);
  EXPECT_EQ(s.b(), "2");
}
