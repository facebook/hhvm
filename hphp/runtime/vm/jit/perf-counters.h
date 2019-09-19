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

#ifndef incl_HPHP_JIT_PERF_COUNTERS_H_
#define incl_HPHP_JIT_PERF_COUNTERS_H_

#include "hphp/util/rds-local.h"

#include <cstdint>

namespace HPHP {

struct Array;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

#define TRANS_PERF_COUNTERS \
  TPC(translate) \
  TPC(retranslate) \
  TPC(interp_bb) \
  TPC(interp_bb_force) \
  TPC(interp_instr) \
  TPC(interp_one) \
  TPC(max_trans) \
  TPC(enter_tc) \
  TPC(service_req) \
  TPC(unser_prop_slow) \
  TPC(unser_prop_fast) \
  TPC(thrift_read_slow) \
  TPC(thrift_write_slow) \
  TPC(thrift_spec_slow) \
  TPC(unit_load)

#define TPC(n) tpc_ ## n,
enum TransPerfCounter {
  TRANS_PERF_COUNTERS
  tpc_num_counters
};
#undef TPC

#ifdef __APPLE__
// Clang believes that it can force perf_counters into 16-byte alignment,
// and thus emit an inlined version of memcpy later in this file using SSE
// instructions which require  such alignment. It can, in fact, do this --
// except due to what is as far as I can tell a linker bug on OS X, ld doesn't
// actually lay this out with 16 byte alignment, and so the SSE instructions
// crash. In order to work around this, tell clang to force it to only 8 byte
// alignment, which causes it to emit an inlined version of memcpy which does
// not assume 16-byte alignment. (Perversely, it also tickles the ld bug
// differently such that it actually gets 16-byte alignment :\)
struct alignas(8) PerfCounters : std::array<int64_t, tpc_num_counters> {};
#else
using PerfCounters = std::array<int64_t, tpc_num_counters>;
#endif
extern RDS_LOCAL_NO_CHECK(PerfCounters, rl_perf_counters);
extern const char* const kPerfCounterNames[tpc_num_counters];

#define INC_TPC(n) ++jit::rl_perf_counters[jit::tpc_##n];

/*
 * Add name-to-perf-counter pairs to `ret', in the same format as hardware
 * counters.
 *
 * This is the format expected by the PHP function hphp_get_hardware_counter().
 */
void getPerfCounters(Array& ret);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
