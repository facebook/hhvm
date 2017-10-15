/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2016 Facebook, Inc. (http://www.facebook.com)          |
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
#include "hphp/util/thread-local.h"

#include <folly/portability/GTest.h>
#include <thread>
#include <atomic>

namespace HPHP {

static std::atomic<int> s_destructs;

struct Foo {
  explicit Foo() : m_bar(42) {}
  ~Foo() {
    EXPECT_EQ(m_bar, 87);
    ++s_destructs;
  }
  int m_bar;
};

static THREAD_LOCAL_FLAT(Foo, s_foo);

TEST(ThreadLocalFlat, OnThreadExit) {
  s_destructs = 0;
  std::thread thread([&]() {
      EXPECT_TRUE(s_foo.isNull());
      Foo* foo = s_foo.getCheck();
      EXPECT_EQ(foo->m_bar, 42);
      EXPECT_FALSE(s_foo.isNull());
      EXPECT_EQ(foo, s_foo.getCheck());
      EXPECT_EQ(foo, s_foo.getNoCheck());
      EXPECT_EQ(s_destructs, 0);
      foo->m_bar = 87;
    });
  EXPECT_EQ(s_destructs, 0);
  thread.join();
  EXPECT_EQ(s_destructs, 1);
}

// Manual re-construction of the instance.
TEST(ThreadLocalFlat, Recreate) {
  s_destructs = 0;
  std::thread thread([&]() {
      EXPECT_TRUE(s_foo.isNull());
      Foo* foo = s_foo.getCheck();
      EXPECT_FALSE(s_foo.isNull());
      foo->m_bar = 87;
      s_foo.destroy();
      EXPECT_TRUE(s_foo.isNull());
      foo = s_foo.getCheck();
      EXPECT_FALSE(s_foo.isNull());
      EXPECT_EQ(foo->m_bar, 42);
      EXPECT_EQ(s_destructs, 1);
      foo->m_bar = 87;
    });
  EXPECT_LE(s_destructs, 1);
  thread.join();
  EXPECT_EQ(s_destructs, 2);
}

}
