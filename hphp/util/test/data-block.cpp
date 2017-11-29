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
#include "hphp/util/data-block.h"
#include <folly/portability/GTest.h>

namespace HPHP {

TEST(DataBlock, Basic) {
  DataBlock d;
  auto s = (ConstCodeAddress)0x100;
  auto e = (ConstCodeAddress)0x200;
  d.init((Address)s, e - s, "test");
  EXPECT_FALSE(d.contains(s-1, s));
  EXPECT_FALSE(d.contains(s-1, s+1));
  EXPECT_TRUE(d.contains(s, s));
  EXPECT_TRUE(d.contains(s, s+1));
  EXPECT_TRUE(d.contains(s, e));
  EXPECT_TRUE(d.contains(e-1, e));
  EXPECT_TRUE(d.contains(e, e));
  EXPECT_FALSE(d.contains(e-1, e+1));
  EXPECT_FALSE(d.contains(e, e+1));
  EXPECT_FALSE(d.contains(e, e-1));
  EXPECT_FALSE(d.contains(e, s));
}

}
