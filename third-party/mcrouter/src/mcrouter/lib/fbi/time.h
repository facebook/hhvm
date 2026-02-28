/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

/** timeval inline functions */
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>

typedef struct timeval timeval_t;

static inline void timeval_add(timeval_t* a, const timeval_t* b) {
  a->tv_sec += b->tv_sec;
  a->tv_usec += b->tv_usec;
  if (a->tv_usec > 1000000) {
    a->tv_sec += 1;
    a->tv_usec -= 1000000;
  }
}

static inline void timeval_sub(timeval_t* a, const timeval_t* b) {
  if (b->tv_usec > a->tv_usec) {
    a->tv_sec--;
    a->tv_usec += 1000000;
  }
  a->tv_sec -= b->tv_sec;
  a->tv_usec -= b->tv_usec;
}

static inline int timeval_cmp(const timeval_t* a, const timeval_t* b) {
  return a->tv_sec > b->tv_sec ? 1
      : a->tv_sec < b->tv_sec  ? -1
                               : a->tv_usec - b->tv_usec;
}

static inline int timeval_lt(const timeval_t* a, const timeval_t* b) {
  return timeval_cmp(a, b) < 0;
}

static inline int timeval_le(const timeval_t* a, const timeval_t* b) {
  return timeval_cmp(a, b) <= 0;
}

static inline int timeval_eq(const timeval_t* a, const timeval_t* b) {
  return timeval_cmp(a, b) == 0;
}

static inline int timeval_gt(const timeval_t* a, const timeval_t* b) {
  return timeval_cmp(a, b) > 0;
}

static inline int timeval_ge(const timeval_t* a, const timeval_t* b) {
  return timeval_cmp(a, b) >= 0;
}

static inline const timeval_t* timeval_min(
    const timeval_t* a,
    const timeval_t* b) {
  return timeval_cmp(a, b) <= 0 ? a : b;
}

static inline uint32_t timeval_ms(const timeval_t* t) {
  return (t->tv_sec * 1000) + (t->tv_usec + 500) / 1000;
}

static inline uint64_t timeval_us(const timeval_t* t) {
  return (t->tv_sec * 1000000) + t->tv_usec;
}

static inline timeval_t ms_to_timeval(const uint32_t ms) {
  timeval_t t;
  t.tv_sec = ms / 1000;
  t.tv_usec = (ms % 1000) * 1000;

  return t;
}
