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
#include "hphp/util/either.h"
#include <gtest/gtest.h>

namespace HPHP {

namespace {
struct A { uintptr_t padding; };
struct B { uintptr_t padding; };
using E = Either<A*,B*>;
}

TEST(Either, Simple) {
  A a;
  B b;
  E ea = &a;
  E eb = &b;
  EXPECT_FALSE(reinterpret_cast<char*>(&a) == reinterpret_cast<char*>(&b));
  EXPECT_TRUE(ea == &a);
  EXPECT_FALSE(ea == &b);
  EXPECT_FALSE(ea == eb);

  auto const wasA = ea.match(
    [] (A*) { return true; },
    [] (B*) { return false; }
  );
  EXPECT_EQ(wasA, true);

  E enull;
  enull.match(
    [] (A*) { EXPECT_TRUE(false); },
    [] (B*) { EXPECT_TRUE(true); }
  );
}

}
