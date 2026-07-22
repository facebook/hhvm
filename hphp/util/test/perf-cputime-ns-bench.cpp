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

#include "hphp/util/perf-cputime-ns.h"

#include <cstdint>

#include <folly/Benchmark.h>
#include <folly/BenchmarkUtil.h>
#include <folly/ClockGettimeWrappers.h>
#include <folly/init/Init.h>

BENCHMARK(folly_clock_gettime_ns, n) {
  for (unsigned i = 0; i < n; ++i) {
    auto ns = folly::chrono::clock_gettime_ns(CLOCK_THREAD_CPUTIME_ID);
    folly::doNotOptimizeAway(ns);
  }
}

BENCHMARK_RELATIVE(perf_get_thread_cputime_ns, n) {
  uint64_t ns;
  for (unsigned i = 0; i < n; ++i) {
    fb_perf_get_thread_cputime_ns(&ns);
    folly::doNotOptimizeAway(ns);
  }
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
