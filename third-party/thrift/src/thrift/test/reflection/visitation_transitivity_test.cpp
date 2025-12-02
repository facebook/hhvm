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

#include <thrift/test/reflection/gen-cpp2/reflection_visitation.h>

#include <gtest/gtest.h>
#include <folly/Overload.h>

using namespace apache::thrift;
using namespace std;

namespace test_cpp2 {
namespace cpp_reflection {
TEST(DepCStruct, TestTransitivity) {
  dep_C_struct s;
  s.i_c() = 10;
  s.d()->i_d() = 20;
  for_each_field(s, [](auto&&, auto&& ref) {
    folly::overload(
        [](int32_t i) { EXPECT_EQ(i, 10); },
        [](dep_D_struct d) {
          for_each_field(d, [](auto&&, auto&& ref) { EXPECT_EQ(*ref, 20); });
        })(*ref);
  });
}
TEST(union1, TestUnion) {
  union1 s;
  s.us() = "foo";
  visit_union(s, [](auto&&, auto&& value) {
    folly::overload(
        [](string& s) { EXPECT_EQ(s, "foo"); },
        [](auto&&) { EXPECT_TRUE(false); })(value);
  });
}
} // namespace cpp_reflection
} // namespace test_cpp2
