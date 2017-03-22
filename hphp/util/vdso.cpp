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

#include "hphp/util/vdso.h"

#ifdef FACEBOOK
#include "common/time/ClockGettimeNS.h" // nolint
#endif

#include <folly/ClockGettimeWrappers.h>

namespace HPHP { namespace vdso {

int clock_gettime(clockid_t clock, timespec* ts) {
#ifdef FACEBOOK
  uint64_t time;
  constexpr uint64_t sec_to_ns = 1000000000;

  if (clock == CLOCK_THREAD_CPUTIME_ID &&
      !fb_perf_get_thread_cputime_ns(&time)) {
    ts->tv_sec = time / sec_to_ns;
    ts->tv_nsec = time % sec_to_ns;
    return 0;
  }
#endif
  return folly::chrono::clock_gettime(clock, ts);
}

int64_t clock_gettime_ns(clockid_t clock) {
#ifdef FACEBOOK
  uint64_t time;

  if (clock == CLOCK_THREAD_CPUTIME_ID &&
      !fb_perf_get_thread_cputime_ns(&time)) {
    return time;
  }
#endif
  return folly::chrono::clock_gettime_ns(clock);
}

}}
