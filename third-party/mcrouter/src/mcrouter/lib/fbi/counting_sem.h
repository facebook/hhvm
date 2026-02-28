/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

/**
 * A counting semaphore.
 *
 * counting_sem_lazy_wait(sem, n) is best effort - it will be non-blocking if
 * possible and the return value will be in the range [1, n], giving the actual
 * value the semaphore was decremented for.
 *
 * Example use:
 *
 *   counting_sem_init(&sem, initial_value);
 *   ...
 *   while (n > 0) {
 *     int m = counting_sem_lazy_wait(&sem, n);
 *     [Do something with m items]
 *     n -= m;
 *   }
 *   ...
 *   counting_sem_post(&sem, n);
 *
 *
 * Implementation details
 *
 * The different states of sem->cnt are:
 *   -1:    Semaphore value is 0 and there's at least one waiter
 *   0:     Semaphore value is 0 and there are no waiters
 *   > 0:   Semaphore value is positive
 *
 * We need a state distinct from '0' to avoid calling wake
 * in the uncontented case.
 */
#include <stdint.h>
#include <atomic>

struct counting_sem_t {
  /**
   * Semaphore value.
   * -1 means "the value is 0 and there's a thread waiting".
   */
  std::atomic<int32_t> cnt{};
};

/**
 * Initialize the semaphore.
 * If val < 0, semaphore will be initialized with 0.
 */
void counting_sem_init(counting_sem_t* sem, int32_t val);

/**
 * Returns current semaphore value.
 */
int32_t counting_sem_value(counting_sem_t* sem);

/**
 * Will only wait if the semaphore value is at zero.
 * Will return a positive integer <= n giving the actual value
 * the semaphore was decremented for.
 *
 * If n <= 0, returns 0 immediately.
 */
int32_t counting_sem_lazy_wait(counting_sem_t* sem, int32_t n);

/**
 * Returns 0 if counting_sem_lazy_wait would block, <= n otherwise
 */
int32_t counting_sem_lazy_nonblocking(counting_sem_t* sem, int32_t n);

/**
 * Increments the semaphore by n.
 * Does nothing if n <= 0.
 */
void counting_sem_post(counting_sem_t* sem, int32_t n);
