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
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/align-x64.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

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
  alignJmpTarget(cb);

  auto const start = vwrap(cb, [&] (Vout& v) {
    auto const ar = v.makeReg();

    v << copy{rvmfp(), ar};

    // Set up the call frame for the stub.  We can't skip this like we do in
    // other stubs because we need the return IP for this frame in the %rbp
    // chain, in order to find the proper fixup for the VMRegAnchor in the
    // intercept handler.
    v << push{rvmfp()};
    v << copy{rsp(), rvmfp()};

    // When we call the event hook, it might tell us to skip the callee
    // (because of fb_intercept).  If that happens, we need to return to the
    // caller, but the handler will have already popped the callee's frame.
    // So, we need to save these values for later.
    v << pushm{ar[AROFF(m_savedRip)]};
    v << pushm{ar[AROFF(m_sfp)]};

    v << copy2{ar, v.cns(EventHook::NormalFunc), rarg(0), rarg(1)};

    bool (*hook)(const ActRec*, int) = &EventHook::onFunctionCall;
    v << call{TCA(hook)};
  });

  us.functionEnterHelperReturn = vwrap2(cb, [&] (Vout& v, Vout& vcold) {
    auto const sf = v.makeReg();
    v << testb{rret(), rret(), sf};

    unlikelyIfThen(v, vcold, CC_Z, sf, [&] (Vout& v) {
      auto const saved_rip = v.makeReg();

      // The event hook has already cleaned up the stack and popped the
      // callee's frame, so we're ready to continue from the original call
      // site.  We just need to grab the fp/rip of the original frame that we
      // saved earlier, and sync rvmsp().
      v << pop{rvmfp()};
      v << pop{saved_rip};

      // Drop our call frame.
      v << addqi{16, rsp(), rsp(), v.makeReg()};

      // Sync vmsp and return to the caller.  This unbalances the return stack
      // buffer, but if we're intercepting, we probably don't care.
      v << load{rvmtl()[rds::kVmspOff], rvmsp()};
      v << jmpr{saved_rip};
    });

    // Skip past the stuff we saved for the intercept case.
    v << addqi{16, rsp(), rsp(), v.makeReg()};

    // Execute a leave, returning us to the callee's prologue.
    v << pop{rvmfp()};
    v << ret{};
  });

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
  auto const udrspo = rvmtl()[unwinderDebuggerReturnSPOff()];

  auto const debuggerReturn = vwrap(cb, [&] (Vout& v) {
    v << load{udrspo, rvmsp()};
    v << storeqi{0, udrspo};
  });
  svcreq::emit_persistent(cb, folly::none, REQ_POST_DEBUGGER_RET);

  auto const resumeCPPUnwind = vwrap(cb, [] (Vout& v) {
    static_assert(sizeof(tl_regState) == 1,
                  "The following store must match the size of tl_regState.");
    auto const regstate = emitTLSAddr(v, tl_regState, v.makeReg());
    v << storebi{static_cast<int32_t>(VMRegState::CLEAN), regstate};

    v << load{rvmtl()[unwinderExnOff()], rarg(0)};
    v << call{TCA(_Unwind_Resume), arg_regs(1)};
  });
  us.endCatchHelperPast = cb.frontier();
  vwrap(cb, [] (Vout& v) { v << ud2{}; });

  alignJmpTarget(cb);

  return vwrap(cb, [&] (Vout& v) {
    auto const done1 = v.makeBlock();
    auto const sf1 = v.makeReg();

    v << cmpqim{0, udrspo, sf1};
    v << jcci{CC_NE, sf1, done1, debuggerReturn};
    v = done1;

    // Normal end catch situation: call back to tc_unwind_resume, which returns
    // the catch trace (or null) in %rax, and the new vmfp in %rdx.
    v << copy{rvmfp(), rarg(0)};
    v << call{TCA(tc_unwind_resume)};
    v << copy{reg::rdx, rvmfp()};

    auto const done2 = v.makeBlock();
    auto const sf2 = v.makeReg();

    v << testq{reg::rax, reg::rax, sf2};
    v << jcci{CC_Z, sf2, done2, resumeCPPUnwind};
    v = done2;

    // We need to do a syncForLLVMCatch(), but vmfp is already in rdx.
    v << jmpr{reg::rax};
  });
}

///////////////////////////////////////////////////////////////////////////////

}}}
