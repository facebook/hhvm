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

#include "hphp/runtime/vm/jit/perf-counters.h"

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

RDS_LOCAL_NO_CHECK(PerfCounters, rl_perf_counters)(PerfCounters{});

#define TPC(n) "jit_" #n,
const char* const kPerfCounterNames[] = {
  TRANS_PERF_COUNTERS
};
#undef TPC
#define TPC(n) StaticString("jit_" #n),
const StaticString s_PerfCounterNames[tpc_num_counters] = {
  TRANS_PERF_COUNTERS
};
#undef TPC

///////////////////////////////////////////////////////////////////////////////

void getPerfCounters(Array& ret) {
  for (auto i = 0; i < tpc_num_counters; ++i) {
    // Until perflab can automatically scale the values we give it to an
    // appropriate range, we have to fudge these numbers so they look more like
    // reasonable hardware counter values.
    ret.set(s_PerfCounterNames[i], rl_perf_counters[i] * 1000);
  }

  for (auto const& pair : Timer::Counters()) {
    if (pair.second.total == 0 && pair.second.count == 0) continue;

    ret.set(String("jit_time_") + pair.first, pair.second.total);
  }
}

///////////////////////////////////////////////////////////////////////////////

}}
