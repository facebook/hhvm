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
    [&](size_t) { ADD_FAILURE(); }
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

TEST(BitopsTest, findFirst1) {
  std::bitset<64> bitset(1);
  for (int i = 0; i < 63; i++) {
    size_t out = findFirst1(bitset);
    EXPECT_EQ(i, out);
    out = findFirst1(bitset, i, bitset.size());
    EXPECT_EQ(i, out);
    out = findFirst1(bitset, i+1, bitset.size());
    EXPECT_EQ(bitset.size(), out);
    bitset <<= 1;
  }
}

TEST(BitopsTest, findFirst0) {
  std::bitset<64> bitset;
  bitset.set();
  bitset.reset(0);
  for (int i = 0; i < 63; i++) {
    size_t out = findFirst0(bitset);
    EXPECT_EQ(i, out);
    out = findFirst0(bitset, i , bitset.size());
    EXPECT_EQ(i, out);
    out = findFirst0(bitset, i+1, bitset.size());
    EXPECT_EQ(bitset.size(), out);
    bitset.set(i);
    if(i + 1 < bitset.size()) bitset.reset(i+1);
  }
}

TEST(BitopsTest, findLast1) {
  std::bitset<64> bitset(1);
  for (int i = 0; i < 63; i++) {
    size_t out = findLast1(bitset);
    EXPECT_EQ(i, out);
    out = findLast1(bitset, i , bitset.size());
    EXPECT_EQ(i, out);
    out = findLast1(bitset, i+1, bitset.size());
    EXPECT_EQ(bitset.size(), out);
    bitset <<= 1;
  }
}

TEST(BitopsTest, findLast0) {
  std::bitset<64> bitset;
  bitset.set();
  bitset.reset(0);
  for (int i = 0; i < 63; i++) {
    size_t out = findLast0(bitset);
    EXPECT_EQ(i, out);
    out = findLast0(bitset, i , bitset.size());
    EXPECT_EQ(i, out);
    out = findLast0(bitset, i+1, bitset.size());
    EXPECT_EQ(bitset.size(), out);
    bitset.set(i);
    if(i + 1 < bitset.size()) bitset.reset(i+1);
  }
}

TEST(BitopsTest, cornercase) {
  std::bitset<64> bitset;
  bitset.set(0);
  size_t out = findLast1(bitset, 0, 1);
  EXPECT_EQ(0, out);
  out = findFirst0(bitset, 0, 0);
  EXPECT_EQ(0, out);
  out = findFirst1(bitset, 1, 1);
  EXPECT_EQ(1, out);
  out = findLast0(bitset, 2, 2);
  EXPECT_EQ(2, out);
}

TEST(BitopsTest, findLast1_cornercase) {
  std::bitset<64> bitset;
  size_t out = findLast1(bitset, 0, 5);
  EXPECT_EQ(5, out);
  out = findLast1(bitset, 0, 0);
  EXPECT_EQ(0, out);
}


TEST(BitopsTest, fill0){
  std::bitset<64> bitset;
  bitset.set();
  fill0(bitset);
  EXPECT_TRUE(bitset.none());
  bitset.set();
  fill0(bitset, 20, 41);
  for (size_t i = 0; i < bitset.size(); ++i) {
    if (i >= 20 && i <= 40){
      EXPECT_FALSE(bitset.test(i));
    } else {
      EXPECT_TRUE(bitset.test(i));
    }
  }
}

TEST(BitopsTest, fill1){
  std::bitset<64> bitset;
  fill1(bitset);
  EXPECT_TRUE(bitset.all());
  bitset.reset();
  fill1(bitset, 20, 41);
  for (size_t i = 0; i < bitset.size(); ++i) {
    if (i >= 20 && i <= 40){
      EXPECT_TRUE(bitset.test(i));
    }else{
      EXPECT_FALSE(bitset.test(i));
    }
  }
}

}
