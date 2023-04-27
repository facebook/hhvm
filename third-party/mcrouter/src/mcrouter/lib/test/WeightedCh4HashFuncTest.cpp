/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include <folly/Conv.h>

#include "mcrouter/lib/Ch3HashFunc.h"
#include "mcrouter/lib/WeightedCh4HashFunc.h"

using namespace facebook::memcache;

/* Just test the same behaviour as Ch3 for weights = 1.0 */
TEST(WeightedCh4HashFunc, basic) {
  WeightedCh4HashFunc func_100(std::vector<double>(100, 1.0));
  WeightedCh4HashFunc func_1({1.0});
  WeightedCh4HashFunc func_max(
      std::vector<double>(furc_maximum_pool_size(), 1.0));
  WeightedCh4HashFunc func_99999(std::vector<double>(99999, 1.0));

  EXPECT_EQ(97, func_100("sample"));
  EXPECT_EQ(0, func_1("sample"));

  EXPECT_EQ(72, func_100(""));
  EXPECT_EQ(0, func_1(""));

  EXPECT_EQ(6173600, func_max(""));
  EXPECT_EQ(5167780, func_max("sample"));

  std::string test_max_key;

  //-128 .. 127
  for (int i = 0; i < 256; ++i) {
    test_max_key.push_back(i - 128);
  }
  EXPECT_EQ(31015, func_99999(test_max_key));

  // 127 .. -128
  std::reverse(test_max_key.begin(), test_max_key.end());
  EXPECT_EQ(67101, func_99999(test_max_key));
}

/* Zero weight -> will give up and return a valid index */
TEST(WeightedCh4HashFunc, zeroWeight) {
  WeightedCh4HashFunc func_1({0.0});
  WeightedCh4HashFunc func_100(std::vector<double>(100, 0.0));

  EXPECT_EQ(0, func_1("sample"));
  EXPECT_EQ(0, func_1(""));

  EXPECT_EQ(12, func_100("sample"));
  EXPECT_EQ(69, func_100(""));
}

/* Compare ch3 and wch4 */
TEST(WeightedCh4HashFunc, reducedWeight) {
  Ch3HashFunc ch3_func_3(3);
  WeightedCh4HashFunc wch4_func_3({1.0, 1.0, 0.7});

  std::vector<size_t> ch3_counts(3, 0);
  std::vector<size_t> wch4_counts(3, 0);

  for (size_t i = 0; i < 1000; ++i) {
    auto key = folly::to<std::string>(i);
    auto ch3_i = ch3_func_3(key);
    auto wch4_i = wch4_func_3(key);
    if (ch3_i != 2) {
      /* hosts with weight 1.0 still get all of their traffic unchanged */
      EXPECT_EQ(ch3_i, wch4_i);
    }

    ++ch3_counts[ch3_func_3(key)];
    ++wch4_counts[wch4_func_3(key)];
  }

  /* Note reduced weight for the 3rd box */
  EXPECT_EQ(std::vector<size_t>({307, 342, 351}), ch3_counts);
  EXPECT_EQ(std::vector<size_t>({352, 378, 270}), wch4_counts);
}

/* Compare ch3 and wch4 */
TEST(WeightedCh4HashFunc, randomWeights) {
  Ch3HashFunc ch3_func_10(10);
  WeightedCh4HashFunc wch4_func_10(
      {0.429, 0.541, 0.117, 0.998, 0.283, 0.065, 0.109, 0.042, 0.676, 0.943});

  std::vector<size_t> ch3_counts(10, 0);
  std::vector<size_t> wch4_counts(10, 0);

  for (size_t i = 0; i < 10000; ++i) {
    auto key = folly::to<std::string>(i);

    ++ch3_counts[ch3_func_10(key)];
    ++wch4_counts[wch4_func_10(key)];
  }

  EXPECT_EQ(
      std::vector<size_t>(
          {995, 955, 1046, 968, 1032, 972, 1016, 1038, 1010, 968}),
      ch3_counts);
  EXPECT_EQ(
      std::vector<size_t>(
          {963, 1226, 305, 2371, 700, 162, 250, 105, 1615, 2303}),
      wch4_counts);
}
