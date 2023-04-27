/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>

#include <gtest/gtest.h>

#include <folly/Conv.h>

#include "mcrouter/lib/Ch3HashFunc.h"

using namespace facebook::memcache;

TEST(Ch3Func, basic) {
  Ch3HashFunc func_100(100);
  Ch3HashFunc func_1(1);
  Ch3HashFunc func_max(furc_maximum_pool_size());
  Ch3HashFunc func_99999(99999);

  // tests with 'sample' as key
  EXPECT_TRUE(func_100("sample") == 97);
  EXPECT_TRUE(func_1("sample") == 0);

  // tests with empty string as key
  EXPECT_TRUE(func_100("") == 72);
  EXPECT_TRUE(func_1("") == 0);

  // tests with max pool size
  EXPECT_TRUE(func_max("") == 6173600);
  EXPECT_TRUE(func_max("sample") == 5167780);

  std::string test_max_key;

  //-128 .. 127
  for (int i = 0; i < 256; ++i) {
    test_max_key.push_back(i - 128);
  }
  EXPECT_TRUE(31015 == func_99999(test_max_key));

  // 127 .. -128
  std::reverse(test_max_key.begin(), test_max_key.end());
  EXPECT_TRUE(67101 == func_99999(test_max_key));
}

TEST(Ch3Func, ch3_3) {
  Ch3HashFunc ch3_func_3(3);
  std::vector<size_t> ch3_counts(3, 0);

  for (size_t i = 0; i < 1000; ++i) {
    auto key = folly::to<std::string>(i);
    ++ch3_counts[ch3_func_3(key)];
  }

  EXPECT_TRUE(std::vector<size_t>({307, 342, 351}) == ch3_counts);
}

TEST(Ch3HashFunc, ch3_10) {
  Ch3HashFunc ch3_func_10(10);
  std::vector<size_t> ch3_counts(10, 0);

  for (size_t i = 0; i < 10000; ++i) {
    auto key = folly::to<std::string>(i);
    ++ch3_counts[ch3_func_10(key)];
  }

  EXPECT_TRUE(
      std::vector<size_t>(
          {995, 955, 1046, 968, 1032, 972, 1016, 1038, 1010, 968}) ==
      ch3_counts);
}
