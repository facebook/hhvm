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

#include "hphp/runtime/vm/jit/enter-tc.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/call-flags.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/util/ringbuffer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit {

namespace {

ALWAYS_INLINE void preEnter(TCA start) {
  if (debug) {
    fflush(stdout);
    fflush(stderr);
  }

  assertx(tc::isValidCodeAddress(start));
  assertx(((uintptr_t)vmsp() & (sizeof(TypedValue) - 1)) == 0);
  assertx(((uintptr_t)vmfp() & (sizeof(TypedValue) - 1)) == 0);

  INC_TPC(enter_tc);
  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    auto const skData = liveSK().toAtomicInt();
    Trace::ringbufferEntry(Trace::RBTypeEnterTC, skData, (uint64_t)start);
  }

  tl_regState = VMRegState::DIRTY;
}

ALWAYS_INLINE void postExit() {
  tl_regState = VMRegState::CLEAN;
  assertx(isValidVMStackAddress(vmsp()));

  vmfp() = nullptr;
}

}

void enterTC(TCA start) {
  tracing::BlockNoTrace _{"enter-tc"};

  preEnter(start);
  assert_flog(tc::isValidCodeAddress(start), "start = {} ; func = {} ({})\n",
              start, vmfp()->func(), vmfp()->func()->fullName());
  auto& regs = vmRegsUnsafe();
  tc::ustubs().enterTCHelper(start, regs.fp, rds::tl_base, regs.stack.top(),
                             vmFirstAR());
  postExit();
}

}}
