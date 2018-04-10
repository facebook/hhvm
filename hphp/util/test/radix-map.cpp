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
#include "hphp/util/radix-map.h"
#include <folly/portability/GTest.h>

namespace HPHP {

TEST(RadixMap, get) {
  RadixMap<char*,2,4> map; // 4-byte alignment, 16 slots per node
  map.insert(nullptr,8); // 4 slots at root level
  EXPECT_EQ(map.get(nullptr), 8);
  EXPECT_EQ(map.get((char*)0), 8); // exact match
  EXPECT_EQ(map.get((char*)1), 0); // same slot but not exact match
  EXPECT_EQ(map.get((char*)3), 0); // same slot but not exact match
  EXPECT_EQ(map.get((char*)4), 0); // in covered slot
  EXPECT_EQ(map.get((char*)8), 0); // past the end
}

TEST(RadixMap, erase) {
  RadixMap<char*,2,4> map; // 4-byte alignment, 16 slots per node
  map.insert(nullptr, 8); // 4 slots at root level
  EXPECT_EQ(map.erase((char*)4), 0); // 0 since entry not found
  EXPECT_EQ(map.get(nullptr), 8);
  EXPECT_EQ(map.erase((char*)3), 0); // 3 is first slot but not exact match
  EXPECT_EQ(map.get(nullptr), 8);
  EXPECT_EQ(map.erase(nullptr), 8);
  EXPECT_EQ(map.get(nullptr), 0);
}

TEST(RadixMap, find) {
  RadixMap<char*,2,4> map; // 4-byte alignment, 16 slots per node
  EXPECT_EQ(map.find(nullptr).ptr, nullptr);
}

TEST(RadixMap, insert_destroy) {
  RadixMap<char*,2,4> map; // 4-byte alignment, 16 slots per node
  map.insert(nullptr, 4);
  map.insert((char*)64, 4); // make it deeper
  EXPECT_EQ(map.countBlocks(), 2);
  map.erase(nullptr);
  map.insert(nullptr, 64); // fill first slot at root level, destroy child
  EXPECT_EQ(map.countBlocks(), 2);
}

TEST(RadixMap, insert_deepen) {
  // tests insert bug found in string_replace_overflow.php
  RadixMap<char*,4,8> map;
  map.insert((char*)0x7fffec0ee000, 0x200000); // slab
  map.insert((char*)0x7fffc7b7f000, 0x14000000); // big string
  EXPECT_EQ(map.countBlocks(), 2);
}

TEST(RadixMap, empty) {
  RadixMap<char*,2,4> map;
  EXPECT_TRUE(map.empty());
  map.insert((char*)4, 4);
  EXPECT_FALSE(map.empty());
  map.erase((char*)4);
  EXPECT_FALSE(map.empty());
  map.clear();
  EXPECT_TRUE(map.empty());
}

}
