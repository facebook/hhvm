/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>

#include <gtest/gtest.h>

#include "mcrouter/lib/Crc32HashFunc.h"

using namespace facebook::memcache;

TEST(Crc32Func, basic) {
  Crc32HashFunc func_100(100);
  Crc32HashFunc func_1(1);
  Crc32HashFunc func_max((1 << 30) - 1);
  Crc32HashFunc func_99999(99999);

  // tests with 'sample' as key
  EXPECT_EQ(func_100("sample"), 7);
  EXPECT_EQ(func_1("sample"), 0);

  // tests with empty string as key
  EXPECT_EQ(func_100(""), 0);
  EXPECT_EQ(func_1(""), 0);

  // tests with max pool size
  EXPECT_EQ(func_max(""), 0);
  EXPECT_EQ(func_max("sample"), 822834884);

  std::string test_max_key;

  //-128 .. 127
  for (int i = 0; i < 256; ++i) {
    test_max_key.push_back(i - 128);
  }
  EXPECT_EQ(11697, func_99999(test_max_key));

  // 127 .. -128
  std::reverse(test_max_key.begin(), test_max_key.end());
  EXPECT_EQ(97630, func_99999(test_max_key));
}
