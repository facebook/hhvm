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

#include "hphp/runtime/vm/jit/enter-tc.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/runtime.h"

#include "hphp/util/ringbuffer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace detail {

void enterTC(TCA start, ActRec* stashedAR) {
  if (debug) {
    fflush(stdout);
    fflush(stderr);
  }

  assertx(tc::isValidCodeAddress(start));
  assertx(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
  assertx(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);

  INC_TPC(enter_tc);
  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    auto skData = SrcKey{liveFunc(), vmpc(), liveResumed()}.toAtomicInt();
    Trace::ringbufferEntry(Trace::RBTypeEnterTC, skData, (uint64_t)start);
  }

  tl_regState = VMRegState::DIRTY;
  enterTCImpl(start, stashedAR);
  tl_regState = VMRegState::CLEAN;
  assertx(isValidVMStackAddress(vmsp()));

  vmfp() = nullptr;
}

}}}
