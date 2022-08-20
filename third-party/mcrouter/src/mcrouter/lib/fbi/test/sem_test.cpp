/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <thread>
#include <vector>

#include <glog/logging.h>
#include <gtest/gtest.h>

#include "mcrouter/lib/fbi/counting_sem.h"

static counting_sem_t sem;

void consume(int id, int X) {
  int n = X;
  while (n > 0) {
    unsigned m = counting_sem_lazy_wait(&sem, n);
    n -= m;
    VLOG(1) << "c " << id << " consumed " << m << ", remaining " << n;
  }
  VLOG(1) << "consumer " << id << " done";
}

void consume_nonblocking(int id, int X) {
  int n = X;
  while (n > 0) {
    unsigned m = counting_sem_lazy_nonblocking(&sem, n);
    n -= m;
    VLOG(1) << "c " << id << " consumed " << m << ", remaining " << n;
  }
  VLOG(1) << "consumer " << id << " done";
}

void produce(int id, int Y) {
  for (int i = 0; i < Y; ++i) {
    counting_sem_post(&sem, 1);
  }
  VLOG(1) << "producer " << id << " done";
}

TEST(counting_semaphore, basic) {
  // X producers producing Y items each
  // Y consumer consuming X items each
  // Initial value is INIT,
  // check that we get back the same at the end
  int X = 523, Y = 345, INIT = 3;

  counting_sem_init(&sem, INIT);

  std::vector<std::thread> threads;
  for (int i = 0; i < Y; ++i) {
    threads.push_back(std::thread{&consume, i, X});
  }
  for (int i = 0; i < X; ++i) {
    threads.push_back(std::thread{&produce, i, Y});
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(INIT, counting_sem_value(&sem));
}

TEST(counting_semaphore, nonblocking) {
  // X producers producing Y items each
  // Y consumer consuming X items each
  // Initial value is INIT,
  // check that we get back the same at the end
  int X = 523, Y = 345, INIT = 3;

  counting_sem_init(&sem, INIT);

  std::vector<std::thread> threads;
  for (int i = 0; i < X; ++i) {
    threads.push_back(std::thread{&produce, i, Y});
  }
  for (int i = 0; i < Y; ++i) {
    threads.push_back(std::thread{&consume_nonblocking, i, X});
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(INIT, counting_sem_value(&sem));
}
