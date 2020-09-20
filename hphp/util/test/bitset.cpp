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

#include <gtest/gtest.h>

#include "hphp/util/bitset.h"

namespace HPHP { namespace jit {

TEST(BitSet, 32) {
  using ts = BitSet<32>;
  ts one = ts::bit<5>();
  ts two = ts::bit<9>() | one;

  // Basic printing.
  EXPECT_EQ("0x0000000000000020", one.hexStr());
  EXPECT_EQ("0x0000000000000220", two.hexStr());

  // Shifting.
  EXPECT_EQ("0x0000000000000220", (two >> 0).hexStr());
  EXPECT_EQ("0x0000000000000022", (two >> 4).hexStr());
  EXPECT_EQ("0x0000000000000002", (two >> 8).hexStr());
  EXPECT_EQ("0x0000000000000000", (two >> 10).hexStr());
  EXPECT_EQ("0x0000000000000000", (two >> 32).hexStr());
  EXPECT_EQ("0x0000000000000000", (two >> 33).hexStr());
  EXPECT_EQ("0x0000000000000220", (two << 0).hexStr());
  EXPECT_EQ("0x0000000000002200", (two << 4).hexStr());
  EXPECT_EQ("0x0000000000022000", (two << 8).hexStr());
  EXPECT_EQ("0x0000000020000000", (two << 24).hexStr());
  EXPECT_EQ("0x0000000000000000", (two << 27).hexStr());
  EXPECT_EQ("0x0000000000000000", (two << 32).hexStr());
  EXPECT_EQ("0x0000000000000000", (two << 33).hexStr());

  // ~
  EXPECT_EQ("0x00000000fffffddf", (~two).hexStr());

  // &, |
  EXPECT_EQ(one, one & two);
  EXPECT_EQ(two, one | two);
  EXPECT_EQ(one << 4, two & (one << 4));

  // Equality.
  EXPECT_TRUE(one != two);
  EXPECT_FALSE(one == two);
  EXPECT_TRUE(two == two);
  EXPECT_FALSE(two != two);

  // &=, |=
  ts tmp = one;
  tmp |= ts::bit<9>();
  EXPECT_EQ(two, tmp);
  tmp &= ts::bit<5>();
  EXPECT_EQ(one & two, tmp);

  // hash()
  EXPECT_NE(one.hash(), two.hash());

  // count()
  EXPECT_EQ(1, one.count());
  EXPECT_EQ(2, two.count());
}

TEST(BitSet, 64) {
  using ts = BitSet<64>;
  ts one = ts::bit<5>();
  ts two = ts::bit<9>() | one;

  // Basic printing.
  EXPECT_EQ("0x0000000000000020", one.hexStr());
  EXPECT_EQ("0x0000000000000220", two.hexStr());

  // Shifting.
  EXPECT_EQ("0x0000000000000220", (two >> 0).hexStr());
  EXPECT_EQ("0x0000000000000002", (two >> 8).hexStr());
  EXPECT_EQ("0x0000000000000000", (two >> 10).hexStr());
  EXPECT_EQ("0x0000000000000000", (two >> 64).hexStr());
  EXPECT_EQ("0x0000000000000000", (two >> 65).hexStr());
  EXPECT_EQ("0x0000000000000220", (two << 0).hexStr());
  EXPECT_EQ("0x0000000220000000", (two << 24).hexStr());
  EXPECT_EQ("0x2000000000000000", (two << 56).hexStr());
  EXPECT_EQ("0x0000000000000000", (two << 59).hexStr());
  EXPECT_EQ("0x0000000000000000", (two << 64).hexStr());
  EXPECT_EQ("0x0000000000000000", (two << 65).hexStr());

  // ~
  EXPECT_EQ("0xfffffffffffffddf", (~two).hexStr());

  // &, |
  EXPECT_EQ(one, one & two);
  EXPECT_EQ(two, one | two);
  EXPECT_EQ(one << 4, two & (one << 4));

  // Equality.
  EXPECT_TRUE(one != two);
  EXPECT_FALSE(one == two);
  EXPECT_TRUE(two == two);
  EXPECT_FALSE(two != two);

  // hash()
  EXPECT_NE(one.hash(), two.hash());

  // count()
  EXPECT_EQ(1, one.count());
  EXPECT_EQ(2, two.count());
}

TEST(BitSet, 65) {
  using ts = BitSet<65>;
  ts one = ts::bit<5>();
  ts two = ts::bit<9>() | one;
  ts three = ts::bit<64>() | two;

  // Basic printing.
  EXPECT_EQ("0x00000000000000000000000000000020", one.hexStr());
  EXPECT_EQ("0x00000000000000000000000000000220", two.hexStr());
  EXPECT_EQ("0x00000000000000010000000000000220", three.hexStr());

  // Shifting.
  EXPECT_EQ("0x00000000000000010000000000000220", (three >> 0).hexStr());
  EXPECT_EQ("0x00000000000000000100000000000002", (three >> 8).hexStr());
  EXPECT_EQ("0x00000000000000000000000000000001", (three >> 64).hexStr());
  EXPECT_EQ("0x00000000000000000000000000000000", (three >> 65).hexStr());
  EXPECT_EQ("0x00000000000000000000000000000000", (three >> 66).hexStr());
  EXPECT_EQ("0x00000000000000010000000000000220", (three << 0).hexStr());
  EXPECT_EQ("0x00000000000000002000000000000000", (three << 56).hexStr());
  EXPECT_EQ("0x00000000000000010000000000000000", (three << 59).hexStr());
  EXPECT_EQ("0x00000000000000000000000000000000", (three << 64).hexStr());
  EXPECT_EQ("0x00000000000000000000000000000000", (three << 65).hexStr());

  // ~
  EXPECT_EQ("0x0000000000000001fffffffffffffddf", (~two).hexStr());
  EXPECT_EQ("0x0000000000000000fffffffffffffddf", (~three).hexStr());

  // &, |
  EXPECT_EQ(two, two & three);
  EXPECT_EQ(three, two | three);
  EXPECT_EQ(ts::bit<64>(), three & ts::bit<64>());

  // Equality.
  EXPECT_TRUE(one != two);
  EXPECT_FALSE(one == two);
  EXPECT_TRUE(two == two);
  EXPECT_FALSE(two != two);
  EXPECT_TRUE(three != two);
  EXPECT_FALSE(three == two);
  EXPECT_TRUE(three == three);
  EXPECT_FALSE(three != three);

  // hash()
  EXPECT_NE(one.hash(), two.hash());
  EXPECT_NE(three.hash(), two.hash());
  EXPECT_NE(ts::bit<64>().hash(), ts{}.hash());
  EXPECT_NE(ts::bit<64>().hash(), ts::bit<0>().hash());

  // count()
  EXPECT_EQ(1, one.count());
  EXPECT_EQ(2, two.count());
  EXPECT_EQ(3, three.count());
}

}}
