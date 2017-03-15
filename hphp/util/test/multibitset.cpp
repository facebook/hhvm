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

#include "hphp/util/multibitset.h"

#include <gtest/gtest.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

TEST(MultiBitSet, Basic) {
  auto mbs = multibitset<3>(100);

  EXPECT_EQ(mbs[0], 0);
  EXPECT_EQ(mbs[41], 0);
  EXPECT_EQ(mbs[42], 0);
  EXPECT_EQ(mbs[43], 0);

  mbs[0] = 2;
  mbs[42] = 7;
  mbs[41] = 6;
  mbs[43] = 5;

  EXPECT_EQ(mbs[0], 2);
  EXPECT_EQ(mbs[41], 6);
  EXPECT_EQ(mbs[42], 7);
  EXPECT_EQ(mbs[43], 5);

  EXPECT_EQ(mbs.ffs(), 0);
  EXPECT_EQ(mbs.ffs(0), 0);
  EXPECT_EQ(mbs.ffs(1), 41);
  EXPECT_EQ(mbs.ffs(41), 41);
  EXPECT_EQ(mbs.ffs(42), 42);
  EXPECT_EQ(mbs.ffs(44), multibitset<3>::npos);

  EXPECT_EQ(mbs.fls(0), 0);
  EXPECT_EQ(mbs.fls(1), 0);
  EXPECT_EQ(mbs.fls(41), 41);
  EXPECT_EQ(mbs.fls(42), 42);
  EXPECT_EQ(mbs.fls(44), 43);
  EXPECT_EQ(mbs.fls(), 43);
  mbs[0] = 0;
  EXPECT_EQ(mbs.fls(1), multibitset<3>::npos);

  mbs.resize(43);
  EXPECT_EQ(mbs.size(), 43);
  EXPECT_EQ(mbs[0], 0);
  EXPECT_EQ(mbs[41], 6);
  EXPECT_EQ(mbs[42], 7);

  mbs.resize(123);
  mbs.reset();

  mbs[119] = 1;
  mbs.resize(119);
  EXPECT_EQ(mbs.ffs(), multibitset<3>::npos);
}

TEST(MultiBitSet, Chunked) {
  auto mbs = chunked_multibitset<3>(42);

  EXPECT_EQ(mbs[1], 0);
  EXPECT_EQ(mbs[23], 0);
  EXPECT_EQ(mbs[60], 0);
  EXPECT_EQ(mbs[120], 0);

  mbs[1] = 2;
  mbs[23] = 7;
  EXPECT_EQ(mbs[1], 2);
  EXPECT_EQ(mbs[23], 7);

  mbs[60] = 6;
  EXPECT_EQ(mbs[1], 2);
  EXPECT_EQ(mbs[23], 7);
  EXPECT_EQ(mbs[60], 6);

  mbs[120] = 5;
  EXPECT_EQ(mbs[1], 2);
  EXPECT_EQ(mbs[23], 7);
  EXPECT_EQ(mbs[60], 6);
  EXPECT_EQ(mbs[120], 5);
  EXPECT_EQ(mbs[39], 0);

  EXPECT_EQ(mbs.ffs(), 1);
  EXPECT_EQ(mbs.ffs(0), 1);
  EXPECT_EQ(mbs.ffs(1), 1);
  EXPECT_EQ(mbs.ffs(2), 23);
  EXPECT_EQ(mbs.ffs(23), 23);
  EXPECT_EQ(mbs.ffs(24), 60);
  EXPECT_EQ(mbs.ffs(60), 60);
  EXPECT_EQ(mbs.ffs(61), 120);
  EXPECT_EQ(mbs.ffs(120), 120);
  EXPECT_EQ(mbs.ffs(121), chunked_multibitset<3>::npos);
  EXPECT_EQ(mbs.ffs(10000), chunked_multibitset<3>::npos);

  EXPECT_EQ(mbs.fls(0), chunked_multibitset<3>::npos);
  EXPECT_EQ(mbs.fls(1), 1);
  EXPECT_EQ(mbs.fls(2), 1);
  EXPECT_EQ(mbs.fls(23), 23);
  EXPECT_EQ(mbs.fls(24), 23);
  EXPECT_EQ(mbs.fls(60), 60);
  EXPECT_EQ(mbs.fls(61), 60);
  EXPECT_EQ(mbs.fls(120), 120);
  EXPECT_EQ(mbs.fls(121), 120);
  EXPECT_EQ(mbs.fls(10000), 120);
  EXPECT_EQ(mbs.fls(), 120);

  mbs.reset();

  mbs[1] = 2;
  mbs[23] = 7;
  // skip a chunk
  mbs[120] = 5;

  EXPECT_EQ(mbs.ffs(23), 23);
  EXPECT_EQ(mbs.ffs(24), 120);
  EXPECT_EQ(mbs.ffs(60), 120);
  EXPECT_EQ(mbs.ffs(119), 120);
  EXPECT_EQ(mbs.ffs(120), 120);

  EXPECT_EQ(mbs.fls(23), 23);
  EXPECT_EQ(mbs.fls(24), 23);
  EXPECT_EQ(mbs.fls(60), 23);
  EXPECT_EQ(mbs.fls(119), 23);
  EXPECT_EQ(mbs.fls(120), 120);

  mbs.reset();

  // This shouldn't take forever.
  mbs[1ull << 62] = 4;
  EXPECT_EQ(mbs.ffs(), 1ull << 62);
}

///////////////////////////////////////////////////////////////////////////////

}
