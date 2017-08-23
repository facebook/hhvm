/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/util/hardware-counter.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <cstring>

#include "hphp/util/hash.h"

namespace HPHP {

NEVER_INLINE
int64_t hash_alot(int64_t seed, int64_t reps) {
  int64_t h = seed;
  while (reps--) {
    h += hash_int64_fallback(h);
  }
  return h;
}

TEST(HardwareCounterTest, InstructionCount) {
  const bool excludeKernel = true;
  HardwareCounter::Init(true, "", false, excludeKernel, -1);
  HardwareCounter::s_counter.getCheck();
  auto begin = HardwareCounter::GetInstructionCount();
  const int64_t reps = 100000000;
  int64_t res = hash_alot(42, reps);
  auto mid = HardwareCounter::GetInstructionCount();
  res = hash_alot(res, reps);
  auto end = HardwareCounter::GetInstructionCount();
  auto d0 = mid - begin;
  auto d1 = end - mid;
  EXPECT_GE(d1, reps);  // Sanity check.
  auto dd = llabs(d1 - d0);
  double relativeError = static_cast<double>(dd) / d0;
  EXPECT_LE(relativeError, 1e-6);
  EXPECT_NE(0, res);  // Use 'res'.
}

}
