/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <gtest/gtest.h>

#include <include/my_rcu_lock.h>
#include <my_systime.h>  // my_sleep
#include <atomic>
#include <thread>

#include "scope_guard.h"

namespace my_rcu_lock_unittest {

class payload_s {
 public:
  payload_s(const char *a, const char *b, const char *c)
      : d1(a), d2(b), d3(c) {}
  payload_s(const payload_s &other)
      : d1(other.d1), d2(other.d2), d3(other.d3) {}
  const char *d1;
  const char *d2;
  const char *d3;
};

typedef MyRcuLock<payload_s> MyRcuLockTest;

class my_rcu_lock_test : public ::testing::Test {
 protected:
  my_rcu_lock_test() {}

  virtual void SetUp() {}

  virtual void TearDown() {}

  static void SetUpTestCase() {
    lock = new MyRcuLockTest(new payload_s("a", "b", "c"));
    reads = 0;
    writes = 0;
  }

  static void TearDownTestCase() { delete lock; }

  static void rcu_reader(size_t reps) {
    for (size_t i = 0; i < reps; i++) {
      {
        MyRcuLockTest::ReadLock rl(lock);
        const payload_s *ptr = rl;
        EXPECT_FALSE(ptr->d1[0] != 'a' || ptr->d2[0] != 'b' ||
                     ptr->d3[0] != 'c');
      }
      reads.fetch_add(1, std::memory_order_relaxed);
    }
  }

  static void rcu_writer(size_t reps, time_t waitms) {
    for (size_t i = 0; i < reps; i++) {
      payload_s *newp = new payload_s("a", "b", "c");

      bool ret = lock->write_wait_and_delete(newp);
      EXPECT_EQ(ret, false);
      /*
        RCU works best with relatively infrequent writes compared to reads.
        Trying to simulate this by spacing out the writes via adding a 100ms
        waits.
        It will work without this too, but will not be testing the code in
        "normal" conditions.
      */
      my_sleep(waitms);

      writes.fetch_add(1, std::memory_order_relaxed);
    }
  }

  static MyRcuLockTest *lock;

  static unsigned char p1[128];
  static std::atomic<long> reads;
  static unsigned char p2[128];
  static std::atomic<long> writes;
  static unsigned char p3[128];
};

MyRcuLockTest *my_rcu_lock_test::lock;
std::atomic<long> my_rcu_lock_test::reads;
std::atomic<long> my_rcu_lock_test::writes;

TEST_F(my_rcu_lock_test, multi_threaded_run) {
  // Capping this at 300 since a std::system_error will be thrown on
  // i686 when creating more than ~400 threads.
  constexpr size_t NUM_READERS = 300;
  std::thread readerts[NUM_READERS];

  constexpr size_t NUM_WRITERS = 10;
  std::thread writerts[NUM_WRITERS];

  {
    auto join_guard = create_scope_guard([&]() {
      // Need to join with those threads already started so that
      // std::terminate is not called in their destructor
      for (auto &rt : readerts) {
        if (rt.joinable()) rt.join();
      }
      for (auto &wt : writerts) {
        if (wt.joinable()) wt.join();
      }
    });

    for (auto &rt : readerts) rt = std::thread(rcu_reader, 100000);
    for (auto &wt : writerts) wt = std::thread(rcu_writer, 5, 100);

    // When leaving this scope the scope guard ensures that we will
    // attempt join every joinable thread
  }

  ASSERT_EQ(reads.load(), 100000 * NUM_READERS);
  ASSERT_EQ(writes.load(), NUM_WRITERS * 5);
}

}  // namespace my_rcu_lock_unittest
