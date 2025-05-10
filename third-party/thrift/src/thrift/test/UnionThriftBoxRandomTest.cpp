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

#include <random>
#include <string>
#include <utility>

#include <gtest/gtest.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <thrift/test/gen-cpp2/UnionThriftBoxRandomTest_types.h>

constexpr int kIterationCount = folly::kIsDebug ? 100'000 : 1'000'000;

namespace apache::thrift::test {

class RandomTestWithSeed : public testing::TestWithParam<int> {};
TEST_P(RandomTestWithSeed, test) {
  std::mt19937 rng;
  rng.seed(GetParam());
  Ref a;
  NonRef b;

  for (int i = 0; i < kIterationCount; i++) {
    auto s = std::to_string(rng() % 100);
    std::vector<std::function<void()>> methods = {
        [&] {
          a.field_1_ref() = s;
          b.field_1_ref() = s;
        },
        [&] {
          a.field_2_ref() = s;
          b.field_2_ref() = s;
        },
        [&] {
          a.field_3_ref().emplace(s);
          b.field_3_ref() = s;
        },
        [&] {
          a.field_4_ref() = s;
          b.field_4_ref() = s;
        },
        [&] {
          a.field_1_ref().emplace(s);
          b.field_1_ref().emplace(s);
        },
        [&] {
          a.field_2_ref().emplace(s);
          b.field_2_ref().emplace(s);
        },
        [&] {
          a.field_3_ref().emplace(s);
          b.field_3_ref().emplace(s);
        },
        [&] {
          a.field_4_ref().emplace(s);
          b.field_4_ref().emplace(s);
        },
        [&] {
          a.field_1_ref().emplace();
          b.field_1_ref().emplace();
        },
        [&] {
          a.field_2_ref().emplace();
          b.field_2_ref().emplace();
        },
        [&] {
          a.field_3_ref().emplace();
          b.field_3_ref().emplace();
        },
        [&] {
          a.field_4_ref().emplace();
          b.field_4_ref().emplace();
        },
    };

    // Choose a random method and call it
    methods[rng() % methods.size()]();

    auto expectEq = [](const auto& a, const auto& b) {
      EXPECT_EQ(bool(a), bool(b));
      if (a && b) {
        EXPECT_EQ(*a, *b);
      }
    };

    EXPECT_EQ(
        folly::to_underlying(a.getType()), folly::to_underlying(b.getType()));
    expectEq(a.field_1_ref(), b.field_1_ref());
    expectEq(a.field_2_ref(), b.field_2_ref());
    expectEq(a.field_3_ref(), b.field_3_ref());
    expectEq(a.field_4_ref(), b.field_4_ref());
  }
}

INSTANTIATE_TEST_CASE_P(
    RandomTest,
    RandomTestWithSeed,
    testing::Range(0, folly::kIsDebug ? 10 : 1000));

} // namespace apache::thrift::test
