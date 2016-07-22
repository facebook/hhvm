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

#include "hphp/util/vdso.h"

#ifdef FACEBOOK
#include "common/time/ClockGettimeNS.h" // nolint
#endif

#ifndef _MSC_VER
#define _GNU_SOURCE 1
#include <dlfcn.h>
#endif

#include <cstring>

namespace HPHP {

namespace {
///////////////////////////////////////////////////////////////////////////////

struct Vdso {
  Vdso() {
#ifndef _MSC_VER
    auto constexpr flags = RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD;
    auto handle = dlopen("linux-vdso.so.1", flags);
    if (handle == nullptr) {
      return;
    }

    SCOPE_EXIT {
      dlclose(handle);
    };

    clock_gettime_ns = reinterpret_cast<decltype(clock_gettime_ns)>(
      dlsym(handle, "__vdso_clock_gettime_ns")
    );
    clock_gettime = reinterpret_cast<decltype(clock_gettime)>(
      dlsym(handle, "__vdso_clock_gettime")
    );
#endif
  }

  ALWAYS_INLINE int64_t clockGetTimeNS(clockid_t clk) {
#ifdef FACEBOOK
    uint64_t time;

    if (clk == CLOCK_THREAD_CPUTIME_ID &&
        !fb_perf_get_thread_cputime_ns(&time)) {
      return time;
    }
#endif
    return clock_gettime_ns ? clock_gettime_ns(clk) : -1;
  }

  ALWAYS_INLINE int64_t clockGetTime(clockid_t clk, timespec* ts) {
#ifdef FACEBOOK
    uint64_t time;
    constexpr uint64_t sec_to_ns = 1000000000;

    if (clk == CLOCK_THREAD_CPUTIME_ID &&
        !fb_perf_get_thread_cputime_ns(&time)) {
      ts->tv_sec = time / sec_to_ns;
      ts->tv_nsec = time % sec_to_ns;
      return 0;
    }
#endif
    if (clock_gettime) {
      memset(ts, 0, sizeof(*ts));
      return clock_gettime(clk, ts);
    }
    return -1;
  }

  //////////////////////////////////////////////////////////////////////////////

  int (*clock_gettime)(clockid_t, timespec*){nullptr};
  int64_t (*clock_gettime_ns)(clockid_t){nullptr};
};

Vdso s_vdso;

////////////////////////////////////////////////////////////////////////////////
}

namespace vdso {

int clock_gettime(clockid_t clock, timespec* ts) {
  return s_vdso.clockGetTime(clock, ts);
}

int64_t clock_gettime_ns(clockid_t clock) {
  return s_vdso.clockGetTimeNS(clock);
}

}

///////////////////////////////////////////////////////////////////////////////
}
