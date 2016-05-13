/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TSC_H_
#define incl_HPHP_TSC_H_

#include <folly/portability/Asm.h>

#include "hphp/util/assertions.h"
#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace HPHP {

/*
 * Return the underlying machine cycle counter. While this is slightly
 * non-portable in theory, all the CPUs you're likely to care about support
 * it in some way or another.
 */
inline uint64_t cpuCycles() {
#ifdef __x86_64__
  uint64_t lo, hi;
  asm volatile("rdtsc" : "=a"((lo)),"=d"(hi));
  return lo | (hi << 32);
#elif __powerpc64__
  // This returns a time-base
  uint64_t tb;
  asm volatile("mfspr %0, 268" : "=r" (tb));
  return tb;
#elif _MSC_VER
  return (uint64_t)__rdtsc();
#elif __aarch64__
  // FIXME: This returns the virtual timer which is not exactly
  // the core cycles but has a different frequency.
  uint64_t tb;
  asm volatile("mrs %0, cntvct_el0" : "=r" (tb));
  return tb;
#else
  not_implemented();
#endif
}

inline void cpuRelax() {
  folly::asm_volatile_pause();
}

inline void cycleDelay(uint32_t numCycles) {
  auto start = cpuCycles();
  do {
    if (numCycles > 100) cpuRelax();
  } while (cpuCycles() - start < numCycles);
}

}

#endif
