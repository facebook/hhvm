/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "test_util.h"

#include <thread>

double measure_time(std::function<void(void)> f) {
  timespec ts;
  double before;

  clock_gettime(CLOCK_MONOTONIC, &ts);
  before = ts.tv_sec * 1e9 + ts.tv_nsec;

  f();

  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1e9 + ts.tv_nsec - before;
}

double measure_time_concurrent(
    unsigned thread_count,
    std::function<void(unsigned)> f) {
  struct info {
    std::thread thread;
    pthread_rwlock_t* gate;
    unsigned idx;
  } * ti;
  unsigned i;
  pthread_rwlock_t gate;
  timespec ts;
  double before;
  double v;

  pthread_rwlock_init(&gate, nullptr);
  pthread_rwlock_wrlock(&gate);

  // Initialize all threads.
  ti = new info[thread_count];
  for (i = 0; i < thread_count; i++) {
    ti[i].gate = &gate;
    ti[i].idx = i;
    ti[i].thread = std::thread(
        [&](info* t) {
          pthread_rwlock_rdlock(t->gate);
          f(t->idx);
          pthread_rwlock_unlock(t->gate);
        },
        ti + i);
  }

  // Let threads go and wait for them to finish.
  clock_gettime(CLOCK_MONOTONIC, &ts);
  before = ts.tv_sec * 1e9 + ts.tv_nsec;

  pthread_rwlock_unlock(&gate);
  for (i = 0; i < thread_count; i++) {
    ti[i].thread.join();
  }
  clock_gettime(CLOCK_MONOTONIC, &ts);
  v = ts.tv_sec * 1e9 + ts.tv_nsec - before;
  delete[] ti;
  pthread_rwlock_destroy(&gate);
  return v;
}
