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

#include <folly/gen/ParallelMap.h>

#include <vector>

#include <glog/logging.h>

#include <folly/Memory.h>
#include <folly/gen/Base.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/GTest.h>

using namespace folly::gen;

TEST(Pmap, InfiniteEquivalent) {
  // apply
  {
    // clang-format off
    auto mapResult
      = seq(1)
      | map([](int x) { return x * x; })
      | until([](int x) { return x > 1000 * 1000; })
      | as<std::vector<int>>();

    auto pmapResult
      = seq(1)
      | pmap([](int x) { return x * x; }, 4)
      | until([](int x) { return x > 1000 * 1000; })
      | as<std::vector<int>>();
    // clang-format on

    EXPECT_EQ(pmapResult, mapResult);
  }

  // foreach
  {
    // clang-format off
    auto mapResult
      = seq(1, 10)
      | map([](int x) { return x * x; })
      | as<std::vector<int>>();

    auto pmapResult
      = seq(1, 10)
      | pmap([](int x) { return x * x; }, 4)
      | as<std::vector<int>>();
    // clang-format on

    EXPECT_EQ(pmapResult, mapResult);
  }
}

TEST(Pmap, Empty) {
  // apply
  {
    // clang-format off
    auto mapResult
      = seq(1)
      | map([](int x) { return x * x; })
      | until([](int) { return true; })
      | as<std::vector<int>>();

    auto pmapResult
      = seq(1)
      | pmap([](int x) { return x * x; }, 4)
      | until([](int) { return true; })
      | as<std::vector<int>>();
    // clang-format on

    EXPECT_EQ(mapResult.size(), 0);
    EXPECT_EQ(pmapResult, mapResult);
  }

  // foreach
  {
    // clang-format off
    auto mapResult
      = empty<int>()
      | map([](int x) { return x * x; })
      | as<std::vector<int>>();

    auto pmapResult
      = empty<int>()
      | pmap([](int x) { return x * x; }, 4)
      | as<std::vector<int>>();
    // clang-format on

    EXPECT_EQ(mapResult.size(), 0);
    EXPECT_EQ(pmapResult, mapResult);
  }
}

TEST(Pmap, Rvalues) {
  // apply
  {
    // clang-format off
    auto mapResult
        = seq(1)
        | map([](int x) { return std::make_unique<int>(x); })
        | map([](std::unique_ptr<int> x) {
            return std::make_unique<int>(*x * *x); })
        | map([](std::unique_ptr<int> x) { return *x; })
        | take(1000)
        | sum;

    auto pmapResult
        = seq(1)
        | pmap([](int x) { return std::make_unique<int>(x); })
        | pmap([](std::unique_ptr<int> x) {
            return std::make_unique<int>(*x * *x); })
        | pmap([](std::unique_ptr<int> x) { return *x; })
        | take(1000)
        | sum;
    // clang-format on

    EXPECT_EQ(pmapResult, mapResult);
  }

  // foreach
  {
    // clang-format off
    auto mapResult
        = seq(1, 1000)
        | map([](int x) { return std::make_unique<int>(x); })
        | map([](std::unique_ptr<int> x) {
            return std::make_unique<int>(*x * *x); })
        | map([](std::unique_ptr<int> x) { return *x; })
        | sum;

    auto pmapResult
        = seq(1, 1000)
        | pmap([](int x) { return std::make_unique<int>(x); })
        | pmap([](std::unique_ptr<int> x) {
            return std::make_unique<int>(*x * *x); })
        | pmap([](std::unique_ptr<int> x) { return *x; })
        | sum;
    // clang-format on

    EXPECT_EQ(pmapResult, mapResult);
  }
}

TEST(Pmap, Exception) {
#if __GNUC__ == 7 && __GNUC_MINOR__ == 5 && !__clang__
  LOG(INFO) << "some versions of gcc miscompile the code below without this";
#endif

  // Exception from source
  EXPECT_THROW(
      just("a") | eachTo<int>() | pmap(To<float>()) | sum, std::runtime_error);

  // Exception from predicate
  EXPECT_THROW(just("b") | pmap(To<int>()) | sum, std::runtime_error);

  // Exception from downstream
  EXPECT_THROW(
      just("c") | pmap(To<std::string>()) | eachTo<int>() | sum,
      std::runtime_error);
}
