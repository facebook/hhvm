/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/unique-stubs-x64.h"

#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/event-hook.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit { namespace x64 {

///////////////////////////////////////////////////////////////////////////////

using namespace jit::reg;

TRACE_SET_MOD(ustubs);

///////////////////////////////////////////////////////////////////////////////

static void alignJmpTarget(CodeBlock& cb) {
  align(cb, Alignment::JmpTarget, AlignContext::Dead);
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFunctionEnterHelper(CodeBlock& cb, UniqueStubs& us) {
  bool (*helper)(const ActRec*, int) = &EventHook::onFunctionCall;
  Asm a { cb };

  alignJmpTarget(cb);
  auto const start = a.frontier();

  Label skip;

  PhysReg ar = rarg(0);

  a.   movq    (rvmfp(), ar);
  a.   push    (rvmfp());
  a.   movq    (rsp(), rvmfp());
  a.   push    (ar[AROFF(m_savedRip)]);
  a.   push    (ar[AROFF(m_sfp)]);
  a.   movq    (EventHook::NormalFunc, rarg(1));
  emitCall(a, CppCall::direct(helper), arg_regs(2));
  us.functionEnterHelperReturn = a.frontier();
  a.   testb   (al, al);
  a.   je8     (skip);
  a.   addq    (16, rsp());
  a.   pop     (rvmfp());
  a.   ret     ();

asm_label(a, skip);
  // The event hook has already cleaned up the stack/actrec so that we're ready
  // to continue from the original call site.  Just need to grab the fp/rip
  // from the original frame, and sync rvmsp() to the execution-context's copy.
  a.   pop     (rvmfp());
  a.   pop     (rsi);
  a.   addq    (16, rsp()); // drop our call frame
  a.   loadq   (rvmtl()[rds::kVmspOff], rvmsp());
  a.   jmp     (rsi);
  a.   ud2     ();

  return start;
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFreeLocalsHelpers(CodeBlock& cb, UniqueStubs& us) {
  Label doRelease;
  Label release;
  Label loopHead;

  auto const rData     = rarg(0); // not live coming in, but used
                                             // for destructor calls
  auto const rIter     = rarg(1); // live coming in
  auto const rFinished = rdx;
  auto const rType     = ecx;
  int const tvSize     = sizeof(TypedValue);

  Asm a { cb };
  align(cb, Alignment::CacheLine, AlignContext::Dead);
  auto const start = a.frontier();

asm_label(a, release);
  a.    loadq  (rIter[TVOFF(m_data)], rData);
  a.    cmpl   (1, rData[FAST_REFCOUNT_OFFSET]);
  jccBlock<CC_L>(a, [&] {
    a.  jz8    (doRelease);
    a.  decl   (rData[FAST_REFCOUNT_OFFSET]);
  });
  a.    ret    ();
asm_label(a, doRelease);
  a.    push    (rIter);
  a.    push    (rFinished);
  a.    call    (lookupDestructor(a, PhysReg(rType)));
  // Three quads between where %rsp is now and the saved RIP of the call into
  // the stub: two from the pushes above, and one for the saved RIP of the call
  // to `release' done below (e.g., in emitDecLocal).
  mcg->fixupMap().recordFixup(a.frontier(), makeIndirectFixup(3));
  a.    pop     (rFinished);
  a.    pop     (rIter);
  a.    ret     ();

  auto emitDecLocal = [&]() {
    Label skipDecRef;

    // Zero-extend the type while loading so it can be used as an array index
    // to lookupDestructor() above.
    emitLoadTVType(a, rIter[TVOFF(m_type)], rType);
    emitCmpTVType(a, KindOfRefCountThreshold, rbyte(rType));
    a.  jle8   (skipDecRef);
    a.  call   (release);
  asm_label(a, skipDecRef);
  };

  alignJmpTarget(cb);
  us.freeManyLocalsHelper = a.frontier();
  a.    lea    (rvmfp()[-(jit::kNumFreeLocalsHelpers * sizeof(Cell))],
                rFinished);

  // Loop for the first few locals, but unroll the final kNumFreeLocalsHelpers.
asm_label(a, loopHead);
  emitDecLocal();
  a.    addq   (tvSize, rIter);
  a.    cmpq   (rIter, rFinished);
  a.    jnz8   (loopHead);

  for (int i = 0; i < kNumFreeLocalsHelpers; ++i) {
    us.freeLocalsHelpers[kNumFreeLocalsHelpers - i - 1] = a.frontier();
    emitDecLocal();
    if (i != kNumFreeLocalsHelpers - 1) {
      a.addq   (tvSize, rIter);
    }
  }

  a.    ret    ();

  // Keep me small!
  always_assert(Stats::enabled() ||
                (a.frontier() - start <= 4 * kX64CacheLineSize));

  return start;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" void enterTCExit();

TCA emitCallToExit(CodeBlock& cb) {
  Asm a { cb };

  // Emit a byte of padding. This is a kind of hacky way to avoid
  // hitting an assert in recordGdbStub when we call it with stub - 1
  // as the start address.
  a.emitNop(1);
  auto const start = a.frontier();
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    Label ok;
    a.emitImmReg(uintptr_t(enterTCExit), rax);
    a.cmpq(rax, *rsp());
    a.je8 (ok);
    a.ud2();
  asm_label(a, ok);
  }

  // Emulate a ret to enterTCExit without actually doing one to avoid
  // unbalancing the return stack buffer. The call from enterTCHelper() that
  // got us into the TC was popped off the RSB by the ret that got us to this
  // stub.
  a.addq(8, rsp());
  a.jmp(TCA(enterTCExit));

  // On a backtrace, gdb tries to locate the calling frame at address
  // returnRIP-1. However, for the first VM frame, there is no code at
  // returnRIP-1, since the AR was set up manually. For this frame,
  // record the tracelet address as starting from this callToExit-1,
  // so gdb does not barf.
  return start;
}

TCA emitEndCatchHelper(CodeBlock& cb, UniqueStubs& us) {
  Asm a { cb };
  alignJmpTarget(cb);
  Label debuggerReturn;
  Label resumeCppUnwind;

  auto const start = a.frontier();
  a.    cmpq (0, rvmtl()[unwinderDebuggerReturnSPOff()]);
  a.    jne8 (debuggerReturn);

  // Normal endCatch situation: call back to tc_unwind_resume, which returns
  // the catch trace (or null) in rax and the new vmfp in rdx.
  a.    movq (rvmfp(), rarg(0));
  a.    call (TCA(tc_unwind_resume));
  a.    movq (rdx, rvmfp());
  a.    testq(rax, rax);
  a.    jz8  (resumeCppUnwind);
  a.    jmp  (rax);  // rdx is still live if we're going to code from llvm

asm_label(a, resumeCppUnwind);
  static_assert(sizeof(tl_regState) == 1,
                "The following store must match the size of tl_regState");
  auto vptr = emitTLSAddr(a, tl_regState, rax);
  Vasm::prefix(a, vptr).
        storeb(static_cast<int32_t>(VMRegState::CLEAN), vptr.mr());
  a.    loadq(rvmtl()[unwinderExnOff()], rarg(0));
  emitCall(a, TCA(_Unwind_Resume), arg_regs(1));
  us.endCatchHelperPast = a.frontier();
  a.    ud2();

asm_label(a, debuggerReturn);
  a.    loadq (rvmtl()[unwinderDebuggerReturnSPOff()], rvmsp());
  a.    storeq(0, rvmtl()[unwinderDebuggerReturnSPOff()]);
  svcreq::emit_persistent(a.code(), folly::none, REQ_POST_DEBUGGER_RET);

  return start;
}

///////////////////////////////////////////////////////////////////////////////

}}}
