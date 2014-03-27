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
#include "hphp/util/tls-pod-bag.h"
#include <gtest/gtest.h>

#include <memory>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

struct Ty {
  uint64_t foo;
  uint64_t bar;
};

using Vec = TlsPodBag<Ty,std::allocator<Ty>>;

}

//////////////////////////////////////////////////////////////////////

TEST(TlsPodBag, Simple) {
  static __thread Vec v = {};
  EXPECT_TRUE(v.empty());

  constexpr auto kCount = 15;

  for (auto i = uint32_t{1}; i < 1 + kCount; ++i) {
    v.find_unpopulated()->foo = i;
  }
  EXPECT_FALSE(v.empty());
  EXPECT_EQ(v.population(), kCount);

  v.release_if([&] (const Ty& t) { return t.foo == 12; });
  EXPECT_EQ(v.population(), kCount - 1);

  v.find_unpopulated()->foo = 42; // should reuse 12 slot

  auto counter = uint32_t{1};
  v.for_each([&] (const Ty& t) {
    if (counter > kCount) {
      // This is an unpopulated slot that may occur from capacity
      // being larger.
      EXPECT_EQ(t.foo, 0);
      return;
    }
    if (counter == 12) {
      EXPECT_EQ(t.foo, 42);
    } else {
      EXPECT_EQ(t.foo, counter);
    }
    ++counter;
  });

  v.visit_to_remove([&] (const Ty& t) {
    EXPECT_EQ(t.foo, 1);
  });
  EXPECT_EQ(v.population(), kCount - 1);

  v.release_if([&] (const Ty& t) { return t.foo != 0; });
  EXPECT_TRUE(v.empty());
}

TEST(TlsPodBag, More) {
  static __thread Vec v = {};

  constexpr auto kBase = 1024;
  constexpr auto kTop = 1024;

  for (auto count = 0; count < 5; ++count) {
    for (auto i = kBase; i < kBase + kTop; ++i) {
      v.find_unpopulated()->foo = i;
    }
    v.release_if([&] (const Ty& t) {
      return t.foo >= kBase + 100 && t.foo <= kTop - 100;
    });
  }
  v.release_if([&] (const Ty& t) { return t.foo != 0; });
}

//////////////////////////////////////////////////////////////////////

}
