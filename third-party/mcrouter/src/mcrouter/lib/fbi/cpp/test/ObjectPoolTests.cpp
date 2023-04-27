/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "mcrouter/lib/fbi/cpp/ObjectPool.h"

using namespace facebook::memcache;

template <typename T>
struct TestAllocator : public std::allocator<T> {
 public:
  using size_type = std::size_t;
  using pointer = T*;
  using const_pointer = const T*;
  using value_type = T;

  pointer allocate(size_type n) {
    TestAllocator<T>::nAllocations++;
    return std::allocator<T>::allocate(n);
  }

  void deallocate(pointer p, size_type n) {
    TestAllocator<T>::nDeAllocations++;
    std::allocator<T>::deallocate(p, n);
  }

  static int nAllocations;
  static int nDeAllocations;
};

template <typename T>
int TestAllocator<T>::nAllocations;
template <typename T>
int TestAllocator<T>::nDeAllocations;

struct TestType {
  TestType() : TestType{0, 0} {}
  TestType(int a, int b) {
    TestType::nConstructed++;
    m1 = a;
    m2 = b;
  }
  ~TestType() {
    TestType::nDestructed++;
  }
  int m1, m2;
  static int nConstructed;
  static int nDestructed;
};

int TestType::nConstructed;
int TestType::nDestructed;

class TestException {};

struct TestTypeThrowing {
  TestTypeThrowing(int val, bool throwEx) {
    if (throwEx) {
      throw TestException();
    }
    mval = val;
  }
  int mval;
};

TEST(ObjectPool, Basic) {
  auto* pool = new ObjectPool<TestType, TestAllocator<TestType>>(2);

  auto checkCounters = [=](int nc, int nd, int na, int nda) {
    EXPECT_EQ(TestType::nConstructed, nc);
    EXPECT_EQ(TestType::nDestructed, nd);
    EXPECT_EQ(TestAllocator<TestType>::nAllocations, na);
    EXPECT_EQ(TestAllocator<TestType>::nDeAllocations, nda);
  };

  auto* vala = pool->alloc();
  EXPECT_TRUE(vala != nullptr);
  EXPECT_EQ(vala->m1, 0);
  EXPECT_EQ(vala->m2, 0);
  checkCounters(1, 0, 1, 0);

  auto* valb = pool->alloc(2, 3);
  EXPECT_TRUE(valb != nullptr);
  EXPECT_EQ(valb->m1, 2);
  EXPECT_EQ(valb->m2, 3);
  checkCounters(2, 0, 2, 0);

  pool->free(vala);
  checkCounters(2, 1, 2, 0);

  vala = pool->alloc(4, 5);
  EXPECT_TRUE(vala != nullptr);
  EXPECT_EQ(vala->m1, 4);
  EXPECT_EQ(vala->m2, 5);
  checkCounters(3, 1, 2, 0);

  auto* valc = pool->alloc(6, 7);
  EXPECT_TRUE(valc != nullptr);
  EXPECT_EQ(valc->m1, 6);
  EXPECT_EQ(valc->m2, 7);
  checkCounters(4, 1, 3, 0);

  pool->free(vala);
  pool->free(valb);
  pool->free(valc);
  checkCounters(4, 4, 3, 1);

  delete pool;
  checkCounters(4, 4, 3, 3);
}

TEST(ObjectPool, ThrowingType) {
  ObjectPool<TestTypeThrowing, TestAllocator<TestTypeThrowing>> pool(1);

  EXPECT_THROW(pool.alloc(3, true), TestException);
  auto* ptr = pool.alloc(5, false);
  EXPECT_EQ(ptr->mval, 5);
  pool.free(ptr);

  EXPECT_EQ(TestAllocator<TestTypeThrowing>::nAllocations, 1);
  EXPECT_EQ(TestAllocator<TestTypeThrowing>::nDeAllocations, 0);
}
