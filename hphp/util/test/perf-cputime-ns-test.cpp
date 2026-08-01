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

#ifndef HHVM_FACEBOOK

#include "hphp/util/perf-cputime-ns.h"

#include <cmath>
#include <ctime>
#include <cstdint>

#include <folly/ClockGettimeWrappers.h>
#include <folly/portability/GTest.h>

#include "hphp/util/portability.h"

namespace HPHP {
namespace {

///////////////////////////////////////////////////////////////////////////////

// Burn a measurable amount of thread CPU time so the two clocks have something
// to advance through.  Marked volatile so the compiler can't optimize it away.
NEVER_INLINE int64_t burnCpu(int64_t reps) {
  volatile int64_t acc = 0;
  for (int64_t i = 0; i < reps; ++i) {
    acc += i * 2654435761ull + 1;
  }
  return acc;
}

int64_t follyThreadCpuNs() {
  return folly::chrono::clock_gettime_ns(CLOCK_THREAD_CPUTIME_ID);
}

///////////////////////////////////////////////////////////////////////////////

// The perf-based reader projects the kernel's last on-CPU snapshot forward with
// the cycle counter, while folly issues the clock_gettime() syscall.  The two
// observe the same monotonically increasing thread clock at slightly different
// instants, so a perf read sandwiched between two folly reads must fall within
// the bracket they form (plus a tolerance for scheduling/measurement noise).
TEST(PerfCpuTimeNs, BracketedByFolly) {
  uint64_t perf;
  if (fb_perf_get_thread_cputime_ns(&perf)) {
    GTEST_SKIP() << "perf thread cputime unavailable on this platform/kernel";
  }

  // 50ms of slack: generous enough to absorb a preemption between reads, tight
  // enough to catch a clock that is wildly wrong (e.g. wall time vs CPU time).
  constexpr int64_t kToleranceNs = 50'000'000;

  for (int i = 0; i < 100; ++i) {
    burnCpu(200000);

    auto const before = follyThreadCpuNs();
    ASSERT_EQ(0, fb_perf_get_thread_cputime_ns(&perf));
    auto const after = follyThreadCpuNs();

    // Sanity: folly's own clock must not go backwards across the perf read.
    ASSERT_LE(before, after);

    auto const p = static_cast<int64_t>(perf);
    EXPECT_GE(p, before - kToleranceNs)
      << "perf read fell below the bracket at iteration " << i;
    EXPECT_LE(p, after + kToleranceNs)
      << "perf read rose above the bracket at iteration " << i;
  }
}

// Successive perf reads must be monotonically non-decreasing: it is a clock.
TEST(PerfCpuTimeNs, Monotonic) {
  uint64_t prev;
  if (fb_perf_get_thread_cputime_ns(&prev)) {
    GTEST_SKIP() << "perf thread cputime unavailable on this platform/kernel";
  }

  for (int i = 0; i < 1000; ++i) {
    burnCpu(50000);
    uint64_t cur;
    ASSERT_EQ(0, fb_perf_get_thread_cputime_ns(&cur));
    EXPECT_GE(cur, prev) << "perf clock moved backwards at iteration " << i;
    prev = cur;
  }
}

// Over a fixed chunk of work the perf and folly clocks should report a similar
// elapsed CPU time -- the deltas should agree closely, which they will not if
// one of them is actually measuring something else (e.g. wall-clock time).
TEST(PerfCpuTimeNs, DeltaAgreesWithFolly) {
  uint64_t perfStart;
  if (fb_perf_get_thread_cputime_ns(&perfStart)) {
    GTEST_SKIP() << "perf thread cputime unavailable on this platform/kernel";
  }
  auto const follyStart = follyThreadCpuNs();

  // Enough work that the elapsed time dominates per-read measurement noise.
  for (int i = 0; i < 50; ++i) burnCpu(500000);

  uint64_t perfEnd;
  ASSERT_EQ(0, fb_perf_get_thread_cputime_ns(&perfEnd));
  auto const follyEnd = follyThreadCpuNs();

  auto const perfDelta = static_cast<int64_t>(perfEnd - perfStart);
  auto const follyDelta = follyEnd - follyStart;

  ASSERT_GT(perfDelta, 0);
  ASSERT_GT(follyDelta, 0);

  // The two elapsed measurements should be within 25% of each other.
  auto const diff = std::abs(perfDelta - follyDelta);
  auto const larger = std::max(perfDelta, follyDelta);
  EXPECT_LT(diff, larger / 4)
    << "perf delta " << perfDelta << "ns disagrees with folly delta "
    << follyDelta << "ns";
}

///////////////////////////////////////////////////////////////////////////////

}
}

#endif // !HHVM_FACEBOOK
