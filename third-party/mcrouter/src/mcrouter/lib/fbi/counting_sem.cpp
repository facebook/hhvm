/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "counting_sem.h"

#include <algorithm>
#include <atomic>

#include <limits.h>
#include <linux/futex.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "mcrouter/lib/fbi/util.h"

#define fbi_futex_wait(p, val) \
  syscall(SYS_futex, (p), FUTEX_WAIT | FUTEX_PRIVATE_FLAG, (val), NULL, NULL, 0)

#define fbi_futex_wake(p, n) \
  syscall(SYS_futex, (p), FUTEX_WAKE | FUTEX_PRIVATE_FLAG, (n), NULL, NULL, 0)

void counting_sem_init(counting_sem_t* sem, int32_t val) {
  sem->cnt.store(std::max(val, 0), std::memory_order_release);
}

int32_t counting_sem_value(counting_sem_t* sem) {
  int32_t cnt = sem->cnt.load(std::memory_order_acquire);
  return std::max(cnt, 0);
}

template <bool nonblocking>
static int32_t counting_sem_lazy_helper(counting_sem_t* sem, int32_t n) {
  int32_t latest, attempt, next;

  if (n <= 0) {
    return 0;
  }

  /*
   * Non-blocking case: semaphore value is positive.
   * Decrement it by at most n and return right away.
   */
  latest = sem->cnt.load(std::memory_order_acquire);
  while (latest > 0) {
    attempt = std::min(n, latest);
    if (sem->cnt.compare_exchange_strong(
            latest, latest - attempt, std::memory_order_acq_rel)) {
      return attempt;
    }
  }

  if (nonblocking) {
    return 0;
  }

  /*
   * Otherwise we have to wait and try again.
   */
  do {
    /* Wait loop */
    do {
      /*
       * Change 0 into -1.  Note we must do this check
       * every loop iteration due to the following scenario:
       * This thread sets -1.  Before we called wait, another thread
       * posts() and yet another thread waits() so the counter is back to 0.
       */
      if (latest == 0) {
        sem->cnt.compare_exchange_strong(latest, -1, std::memory_order_acq_rel);
      }

      if (latest <= 0) {
        /*
         * Either we saw a 0 (and we set it to -1) or we saw a -1.
         * Wait if it's still a -1.
         */
        fbi_futex_wait(&sem->cnt, -1);
        latest = sem->cnt.load(std::memory_order_acquire);
      }
    } while (latest <= 0);

    /* latest > 0 due to loop above, so attempt is always positive */
    attempt = std::min(n, latest);
    next = latest - attempt;

    /*
     * Other threads might already be waiting.
     * We can't set this to 0 here, or post() will never wake them up.
     */
    if (next == 0) {
      next = -1;
    }
  } while (!sem->cnt.compare_exchange_strong(
      latest, next, std::memory_order_acq_rel));

  if (next > 0) {
    /*
     * The semaphore value is still positive.
     * We must wake here in case other threads are waiting.
     */
    fbi_futex_wake(&sem->cnt, 1);
  }

  return attempt;
}

int32_t counting_sem_lazy_wait(counting_sem_t* sem, int32_t n) {
  return counting_sem_lazy_helper<false>(sem, n);
}

int32_t counting_sem_lazy_nonblocking(counting_sem_t* sem, int32_t n) {
  return counting_sem_lazy_helper<true>(sem, n);
}

void counting_sem_post(counting_sem_t* sem, int32_t n) {
  int32_t latest, base, next;

  if (n <= 0) {
    return;
  }

  latest = sem->cnt.load(std::memory_order_acquire);
  do {
    base = std::max(latest, 0);
    next = base + std::min(n, INT32_MAX - base);
  } while (!sem->cnt.compare_exchange_strong(
      latest, next, std::memory_order_acq_rel));

  if (latest < 0) {
    /* If we went out of the negative state, we need to wake a thread up. */
    fbi_futex_wake(&sem->cnt, 1);
  }
}
