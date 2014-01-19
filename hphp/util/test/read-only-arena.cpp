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
#include "hphp/util/read-only-arena.h"
#include <gtest/gtest.h>

#include <vector>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

TEST(ReadOnlyArena, simpleTest) {
  ReadOnlyArena arena(4096 * 10);

  const char foo[] = "abc";
  auto strP = static_cast<const char*>(
    arena.allocate(foo, sizeof foo)
  );

  EXPECT_EQ(strcmp(strP, foo), 0);

  const char someJunk[] = "whatevs";

  auto strP2 = static_cast<const char*>(
    arena.allocate(someJunk, sizeof someJunk)
  );

  EXPECT_EQ(strcmp(strP2, someJunk), 0);
}

//////////////////////////////////////////////////////////////////////

}
