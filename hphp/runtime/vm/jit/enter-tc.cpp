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
#include "hphp/runtime/vm/jit/jit-resume-addr.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/runtime.h"

#include "hphp/util/rds-local.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP::jit {

namespace {

ALWAYS_INLINE void preEnter(JitResumeAddr start) {
  assertx(tc::isValidCodeAddress(start.handler));
  assertx(start.arg == nullptr || tc::isValidCodeAddress(start.arg));
  assertx(((uintptr_t)vmsp() & (sizeof(TypedValue) - 1)) == 0);
  assertx(((uintptr_t)vmfp() & (sizeof(TypedValue) - 1)) == 0);

  INC_TPC(enter_tc);
  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    auto const skData = liveSK().toAtomicInt();
    auto const addr = start.arg != nullptr ? start.arg : start.handler;
    Trace::ringbufferEntry(Trace::RBTypeEnterTC, skData, (uint64_t)addr);
  }

  // If stepping in the debugger, the next debugger interrpt check in JIT
  // must bring the execution back to interpreter.
  if (UNLIKELY(RuntimeOption::EnableVSDebugger &&
               !RID().getVSDebugDisablesJit() &&
               !g_context->m_dbgNoBreak &&
               RID().getDebuggerStepIntr())) {
    markFunctionWithDebuggerIntr(vmfp()->func());
  }

  regState() = VMRegState::DIRTY;
}

ALWAYS_INLINE void postExit() {
  regState() = VMRegState::CLEAN;
  assertx(isValidVMStackAddress(vmsp()));

  vmfp() = nullptr;
  vmpc() = nullptr;
}

}

void enterTC(JitResumeAddr start) {
  tracing::BlockNoTrace _{"enter-tc"};

  preEnter(start);
  assertx(rds::local::tcCheck());
  assert_flog(
    tc::isValidCodeAddress(start.handler) &&
    (start.arg == nullptr || tc::isValidCodeAddress(start.arg)),
    "start = {}/{} ; func = {} ({})\n",
    start.handler, start.arg, vmfp()->func(), vmfp()->func()->fullName()
  );
  tc::ustubs().enterTCHelper(
    start.handler, start.arg, rds::tl_base, vmFirstAR());
  postExit();
}

}
