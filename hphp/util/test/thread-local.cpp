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

#include <thread>
#include <gtest/gtest.h>

namespace HPHP {

static bool s_exited = false;

struct Foo {
  explicit Foo(int bar) : m_bar(bar) {}
  ~Foo() { EXPECT_EQ(m_bar, 87); }
  static void Create(void* storage) { new (storage) Foo(42); }
  static void Delete(Foo* foo) { foo->~Foo(); }
  static void OnThreadExit(Foo* foo) {
    foo->~Foo();
    s_exited = true;
  }
  int m_bar;
};

using TLSFoo = ThreadLocalSingleton<Foo>;
static TLSFoo s_fooInitDummy;

TEST(ThreadLocalSingleton, OnThreadExit) {
  s_exited = false;
  std::thread thread([&]() {
      EXPECT_TRUE(TLSFoo::isNull());
      Foo* foo = TLSFoo::getCheck();
      EXPECT_EQ(foo->m_bar, 42);
      EXPECT_FALSE(TLSFoo::isNull());
      EXPECT_EQ(foo, TLSFoo::getCheck());
      EXPECT_EQ(foo, TLSFoo::getNoCheck());
      EXPECT_FALSE(s_exited);
      foo->m_bar = 87;
    });
  EXPECT_FALSE(s_exited);
  thread.join();
  EXPECT_TRUE(s_exited);
}

// Manual re-construction of the instance.
TEST(ThreadLocalSingleton, Recreate) {
  s_exited = false;
  std::thread thread([&]() {
      EXPECT_TRUE(TLSFoo::isNull());
      Foo* foo = TLSFoo::getCheck();
      EXPECT_FALSE(TLSFoo::isNull());
      foo->m_bar = 87;
      TLSFoo::destroy();
      EXPECT_TRUE(TLSFoo::isNull());
      foo = TLSFoo::getCheck();
      EXPECT_FALSE(TLSFoo::isNull());
      EXPECT_EQ(foo->m_bar, 42);
      EXPECT_FALSE(s_exited);
      foo->m_bar = 87;
    });
  EXPECT_FALSE(s_exited);
  thread.join();
  EXPECT_TRUE(s_exited);
}

}
