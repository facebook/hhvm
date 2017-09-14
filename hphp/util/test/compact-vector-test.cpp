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
#include "hphp/util/compact-vector.h"
#include <folly/portability/GTest.h>

namespace HPHP {

namespace {

struct A {
  A() { self = this; }
  A(const A& o) { self = this; }
  A(A&&o) noexcept { self = this; }
  A& operator=(const A& o) { self = this; return *this; }
  A& operator=(A&& o) { self = this; return *this; }

  bool valid() const { return self == this; }
private:
  A* self;
};

}

TEST(CompactVector, Simple) {
  CompactVector<A> av;

  auto valid = [&] {
    for (auto const& a : av) {
      if (!a.valid()) return false;
    }
    return true;
  };

  EXPECT_EQ(valid(), true);
  av.resize(10);
  EXPECT_EQ(valid(), true);
  av.resize(100);
  EXPECT_EQ(valid(), true);
  av.erase(av.begin());
  EXPECT_EQ(valid(), true);
  av.erase(av.begin() + 10, av.begin() + 20);
  EXPECT_EQ(valid(), true);
  EXPECT_EQ(av.size(), 89);
  av.erase(av.begin() + 50, av.end());
  EXPECT_EQ(valid(), true);
  EXPECT_EQ(av.size(), 50);
}

}
