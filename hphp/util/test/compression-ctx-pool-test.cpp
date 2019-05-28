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

#include <thread>

#include <gtest/gtest.h>

#include "hphp/util/compression-ctx-pool.h"

using namespace testing;

namespace HPHP {

namespace {

std::atomic<size_t> numFoos{0};

class Foo {};

Foo* fooCreator() {
  numFoos++;
  return new Foo();
}

void fooDeleter(Foo* f) {
  delete f;
}

using Pool = CompressionContextPool<Foo, fooCreator, fooDeleter>;

} // anonymous namespace

class CompressionContextPoolTest : public testing::Test {
 protected:
  void SetUp() override {
    pool_ = std::make_unique<Pool>();
  }

  void TearDown() override {
    pool_.reset();
  }

  std::unique_ptr<Pool> pool_;
};

TEST_F(CompressionContextPoolTest, testGet) {
  auto ptr = pool_->get();
  EXPECT_TRUE(ptr);
}

TEST_F(CompressionContextPoolTest, testSame) {
  Pool::Object* tmp;
  {
    auto ptr = pool_->get();
    tmp = ptr.get();
  }
  {
    auto ptr = pool_->get();
    EXPECT_EQ(tmp, ptr.get());
  }
}

TEST_F(CompressionContextPoolTest, testDifferent) {
  auto ptr1 = pool_->get();
  auto ptr2 = pool_->get();
  EXPECT_NE(ptr1.get(), ptr2.get());
}

TEST_F(CompressionContextPoolTest, testLifo) {
  auto ptr1 = pool_->get();
  auto ptr2 = pool_->get();
  auto ptr3 = pool_->get();
  auto t1 = ptr1.get();
  auto t2 = ptr2.get();
  auto t3 = ptr3.get();
  EXPECT_NE(t1, t2);
  EXPECT_NE(t1, t3);
  EXPECT_NE(t2, t3);

  ptr3.reset();
  ptr2.reset();
  ptr1.reset();

  ptr1 = pool_->get();
  EXPECT_EQ(ptr1.get(), t1);
  ptr1.reset();

  ptr1 = pool_->get();
  EXPECT_EQ(ptr1.get(), t1);
  ptr2 = pool_->get();
  EXPECT_EQ(ptr2.get(), t2);
  ptr1.reset();
  ptr2.reset();

  ptr1 = pool_->get();
  EXPECT_EQ(ptr1.get(), t2);
  ptr2 = pool_->get();
  EXPECT_EQ(ptr2.get(), t1);
  ptr3 = pool_->get();
  EXPECT_EQ(ptr3.get(), t3);
}

TEST_F(CompressionContextPoolTest, testMultithread) {
  constexpr size_t numThreads = 64;
  constexpr size_t numIters = 1 << 14;
  std::vector<std::thread> ts;
  for (size_t i = 0; i < numThreads; i++) {
    ts.emplace_back([& pool = *pool_]() {
      for (size_t n = 0; n < numIters; n++) {
        auto ref = pool.get();
        ref.get();
        ref.reset();
      }
    });
  }

  for (auto& t : ts) {
    t.join();
  }

  EXPECT_LE(numFoos.load(), numThreads);
}

} // namespace HPHP
