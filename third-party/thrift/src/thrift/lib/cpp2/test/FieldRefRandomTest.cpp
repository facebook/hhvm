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

#include <optional>
#include <random>
#include <string>
#include <thrift/lib/cpp2/FieldRef.h>

#include <gtest/gtest.h>

#include <folly/container/Array.h>
#include <thrift/lib/cpp2/BadFieldAccess.h>

using apache::thrift::optional_field_ref;
using namespace std;

void callRandomMethod(
    mt19937& rng, optional_field_ref<string&>& a, optional<string>& b) {
  auto arg = to_string(rng());
  auto methods = folly::make_array<function<void()>>(
      [&] { EXPECT_EQ(bool(a), bool(b)); },
      [&] { EXPECT_EQ(a.has_value(), b.has_value()); },
      [&] { EXPECT_EQ(a.value_or(arg), b.value_or(arg)); },
      [&] { EXPECT_EQ(a.emplace(arg), b.emplace(arg)); },
      [&] { a = arg, b = arg; },
      [&] { a.reset(), b.reset(); },
      [&] {
        if (a) {
          EXPECT_EQ(a.value(), b.value());
        } else {
          EXPECT_THROW(a.value(), apache::thrift::bad_field_access);
          EXPECT_THROW(b.value(), bad_optional_access);
        }
      },
      [&] {
        if (a) {
          EXPECT_EQ(*a, *b);
        } else {
          EXPECT_THROW(*a, apache::thrift::bad_field_access);
          // *b never throws
        }
      });

  // Choose a random method and call it
  methods[rng() % methods.size()]();
}

void comparison(
    mt19937& rng,
    const optional_field_ref<string&>& a1,
    const optional_field_ref<string&>& a2,
    const optional<string>& b1,
    const optional<string>& b2) {
  auto arg = to_string(rng());
  auto methods = folly::make_array<function<void()>>(
      [&] { EXPECT_EQ(a1 == a2, b1 == b2); },
      [&] { EXPECT_EQ(a1 != a2, b1 != b2); },
      [&] { EXPECT_EQ(a1 >= a2, b1 >= b2); },
      [&] { EXPECT_EQ(a1 <= a2, b1 <= b2); },
      [&] { EXPECT_EQ(a1 > a2, b1 > b2); },
      [&] { EXPECT_EQ(a1 < a2, b1 < b2); },
      [&] { EXPECT_EQ(a1 == arg, b1 == arg); },
      [&] { EXPECT_EQ(a1 != arg, b1 != arg); },
      [&] { EXPECT_EQ(a1 >= arg, b1 >= arg); },
      [&] { EXPECT_EQ(a1 <= arg, b1 <= arg); },
      [&] { EXPECT_EQ(a1 > arg, b1 > arg); },
      [&] { EXPECT_EQ(a1 < arg, b1 < arg); },
      [&] { EXPECT_EQ(arg == a2, arg == b2); },
      [&] { EXPECT_EQ(arg != a2, arg != b2); },
      [&] { EXPECT_EQ(arg >= a2, arg >= b2); },
      [&] { EXPECT_EQ(arg <= a2, arg <= b2); },
      [&] { EXPECT_EQ(arg > a2, arg > b2); },
      [&] { EXPECT_EQ(arg < a2, arg < b2); });

  // Choose a comparison and call it
  methods[rng() % methods.size()]();
}

class RandomTestWithSeed : public testing::TestWithParam<int> {};

TEST_P(RandomTestWithSeed, test) {
  mt19937 rng(GetParam());

  string p1, p2;
  uint8_t q1 = 0, q2 = 0;

  // Defining 2 variables since we want to test comparison operators
  auto a1 = apache::thrift::detail::make_optional_field_ref(p1, q1);
  auto a2 = apache::thrift::detail::make_optional_field_ref(p2, q2);

  optional<string> b1, b2;

  for (int i = 0; i < 100'000; i++) {
    comparison(rng, a1, a2, b1, b2);
    callRandomMethod(rng, a1, b1);
    callRandomMethod(rng, a2, b2);
  }
}

INSTANTIATE_TEST_CASE_P(
    RandomTest,
    RandomTestWithSeed,
    testing::Range(0, folly::kIsDebug ? 16 : 256));
