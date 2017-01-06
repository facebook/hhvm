/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/util/bitops.h"
#include <gtest/gtest.h>

namespace HPHP {

TEST(BitopsTest, FfsNonzero) {
  int64_t out;
  int64_t one = 1;
  bool res;
  for (int i = 0; i < 63; i++) {
    res = ffs64(one << i, out);
    EXPECT_TRUE(res);
    EXPECT_EQ(i, out);
  }
}

TEST(BitopsTest, FlsNonzero) {
  int64_t out;
  int64_t one = 1;
  bool res;
  for (int i = 0; i < 63; i++) {
    res = fls64(one << i, out);
    EXPECT_TRUE(res);
    EXPECT_EQ(i, out);
  }
}

TEST(BitopsTest, FfsZero) {
  int64_t zero = 0;
  int64_t out;
  EXPECT_FALSE(ffs64(zero, out));
}

TEST(BitopsTest, FlsZero) {
  int64_t zero = 0;
  int64_t out;
  EXPECT_FALSE(fls64(zero, out));
}

TEST(BitopsTest, BitsetForEach1) {
  std::vector<size_t> indices = {0, 63, 70, 71, 112, 128, 255};
  std::bitset<256> bitset;
  for (auto const& i : indices) {
    bitset.set(i);
  }

  size_t iter = 0;
  bitset_for_each_set(
    bitset,
    [&](size_t i) {
      EXPECT_TRUE(iter < indices.size() &&
                  i == indices[iter]);
      ++iter;
    }
  );
}

TEST(BitopsTest, BitsetForEach2) {
  std::vector<size_t> indices = {90, 107, 115};
  std::bitset<256> bitset;
  for (auto const& i : indices) {
    bitset.set(i);
  }

  size_t iter = 0;
  bitset_for_each_set(
    bitset,
    [&](size_t i) {
      EXPECT_TRUE(iter < indices.size() &&
                  i == indices[iter]);
      ++iter;
    }
  );
}

TEST(BitopsTest, BitsetForEachEmpty) {
  std::bitset<256> bitset;
  bitset_for_each_set(
    bitset,
    [&](size_t) { EXPECT_TRUE(false); }
  );
}

TEST(BitopsTest, BitsetForEachFull) {
  std::bitset<256> bitset;
  bitset.set();

  size_t iter = 0;
  bitset_for_each_set(
    bitset,
    [&](size_t i) {
      EXPECT_TRUE(i < bitset.size() && i == iter);
      ++iter;
    }
  );
}

}
