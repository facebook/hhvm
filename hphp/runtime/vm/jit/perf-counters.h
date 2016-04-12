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

#ifndef incl_HPHP_JIT_PERF_COUNTERS_H_
#define incl_HPHP_JIT_PERF_COUNTERS_H_

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
  TPC(thrift_spec_slow)

#define TPC(n) tpc_ ## n,
enum TransPerfCounter {
  TRANS_PERF_COUNTERS
  tpc_num_counters
};
#undef TPC

#ifdef __APPLE__
// See perf-counters.cpp for an explanation of this hack.
alignas(8)
#endif
extern __thread int64_t tl_perf_counters[tpc_num_counters];
extern const char* const kPerfCounterNames[tpc_num_counters];

#define INC_TPC(n) ++jit::tl_perf_counters[jit::tpc_##n];

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
