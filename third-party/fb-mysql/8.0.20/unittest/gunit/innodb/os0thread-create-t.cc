/*
   Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <gtest/gtest.h>

#include "os0thread-create.h"

extern uint32_t srv_max_n_threads;

namespace {

struct Global_init {
  Global_init() { srv_max_n_threads = 100; }
};

static Global_init init{};

TEST(CreateDetachedThreadTest, BasicUsage) {
  std::atomic_bool stop{false}, active{false};

  auto f = [&stop, &active](int arg) {
    EXPECT_EQ(42, arg);
    active.store(true);
    while (!stop.load()) {
      UT_RELAX_CPU();
    }
  };

  IB_thread thread;
  EXPECT_FALSE(thread_is_active(thread));

  thread = create_detached_thread(0, std::move(f), 42);
  EXPECT_FALSE(thread_is_active(thread));

  thread.start();
  EXPECT_TRUE(thread_is_active(thread));

  while (!active.load()) {
    UT_RELAX_CPU();
  }

  EXPECT_TRUE(thread_is_active(thread));

  stop.store(true);

  while (thread_is_active(thread)) {
    UT_RELAX_CPU();
  }

  thread = {};

  EXPECT_FALSE(thread_is_active(thread));
  EXPECT_FALSE(os_thread_any_active());
}

TEST(CreateDetachedThreadTest, Move) {
  std::atomic_bool stop{false}, done{false};

  auto f = [&stop, &done](int arg) {
    EXPECT_EQ(42, arg);
    while (!stop.load()) {
      UT_RELAX_CPU();
    }

    done.store(true);
  };

  IB_thread t1, t2;

  t1 = create_detached_thread(0, std::move(f), 42);
  t1.start();
  EXPECT_TRUE(thread_is_active(t1));

  t2 = t1;
  EXPECT_TRUE(thread_is_active(t1));
  EXPECT_TRUE(thread_is_active(t2));

  t2 = std::move(t1);

  EXPECT_FALSE(thread_is_active(t1));
  EXPECT_TRUE(thread_is_active(t2));

  t1 = std::move(t2);

  EXPECT_TRUE(thread_is_active(t1));
  EXPECT_FALSE(thread_is_active(t2));

  t2 = t1;

  EXPECT_TRUE(thread_is_active(t1));
  EXPECT_TRUE(thread_is_active(t2));

  t1 = {};

  EXPECT_FALSE(thread_is_active(t1));
  EXPECT_TRUE(thread_is_active(t2));

  t1 = std::move(t2);
  t2 = {};

  EXPECT_FALSE(thread_is_active(t2));
  EXPECT_TRUE(thread_is_active(t1));

  t1 = {};
  EXPECT_FALSE(thread_is_active(t1));

  EXPECT_FALSE(done.load());
  EXPECT_TRUE(os_thread_any_active());

  stop.store(true);
  while (os_thread_any_active()) {
    UT_RELAX_CPU();
  }
  EXPECT_TRUE(done.load());
}

}  // namespace
