/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/util.h"
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

TEST(BitopsTest, NextPow32Bit) {
  static uint32_t values[] = { 1, 2, 3, 63, 64, 65 };
  for(auto v : values) {
    auto next = HPHP::Util::nextPower2(v);
    EXPECT_TRUE(next > 0 && (next & (next - 1)) == 0);
    EXPECT_LE(v, next);
    EXPECT_LT(next / 2, v);
  }
}


TEST(BitopsTest, NextPow64Bit) {
  static uint64_t values[] = {
      1, 2, 3, 63, 64, 65, uint64_t(-1) / 2, (uint64_t(1) << 40) + 1
  };
  for(auto v : values) {
    auto next = HPHP::Util::nextPower2(v);
    EXPECT_TRUE(next > 0 && (next & (next - 1)) == 0);
    EXPECT_LE(v, next);
    EXPECT_LT(next / 2, v);
  }
}

}
