/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Clocks.h"
#include <sys/time.h>

namespace facebook::memcache::cycles {

uint64_t getCpuCycles() noexcept {
#if defined(__x86_64__)
  uint64_t hi;
  uint64_t lo;
  __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return (hi << 32) | lo;
#elif defined(__i386__)
  uint64_t val;
  __asm__ volatile("rdtsc" : "=A"(val));
  return val;
#elif defined(__powerpc__) && \
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
  uint64_t val;
  val = __builtin_ppc_get_timebase();
  return val;
#elif defined(__aarch64__)
  uint64_t cval;
  asm volatile("mrs %0, cntvct_el0" : "=r"(cval));
  return cval;
#elif defined(__ARM_ARCH)
#if (__ARM_ARCH >= 6)
  uint32_t pmccntr;
  uint32_t pmuseren;
  uint32_t pmcntenset;
  asm volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
  if (pmuseren & 1) {
    asm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
    if (pmcntenset & 0x80000000ul) {
      asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
      return static_cast<uint64_t>(pmccntr) * 64;
    }
  }
#endif
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
#else
#error Unsupported CPU. Consider implementing your own Clock.
#endif
}

} // namespace facebook::memcache::cycles
