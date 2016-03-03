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

#include "hphp/runtime/vm/jit/perf-counters.h"

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__
// Clang believes that it can force tl_perf_counters into 16-byte alignment,
// and thus emit an inlined version of memcpy later in this file using SSE
// instructions which require  such alignment. It can, in fact, do this --
// except due to what is as far as I can tell a linker bug on OS X, ld doesn't
// actually lay this out with 16 byte alignment, and so the SSE instructions
// crash. In order to work around this, tell clang to force it to only 8 byte
// alignment, which causes it to emit an inlined version of memcpy which does
// not assume 16-byte alignment. (Perversely, it also tickles the ld bug
// differently such that it actually gets 16-byte alignment :\)
alignas(8)
#endif
__thread int64_t tl_perf_counters[tpc_num_counters];

#define TPC(n) "jit_" #n,
const char* const kPerfCounterNames[] = {
  TRANS_PERF_COUNTERS
};
#undef TPC

///////////////////////////////////////////////////////////////////////////////

void getPerfCounters(Array& ret) {
  for (auto i = 0; i < tpc_num_counters; ++i) {
    // Until perflab can automatically scale the values we give it to an
    // appropriate range, we have to fudge these numbers so they look more like
    // reasonable hardware counter values.
    ret.set(String::FromCStr(kPerfCounterNames[i]),
            tl_perf_counters[i] * 1000);
  }

  for (auto const& pair : Timer::Counters()) {
    if (pair.second.total == 0 && pair.second.count == 0) continue;

    ret.set(String("jit_time_") + pair.first, pair.second.total);
  }
}

///////////////////////////////////////////////////////////////////////////////

}}
