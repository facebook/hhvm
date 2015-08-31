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
#include "hphp/runtime/vm/jit/unique-stubs.h"

#include <boost/implicit_cast.hpp>
#include <sstream>

#include "hphp/util/abi-cxx.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/disasm.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP { namespace jit { namespace x64 {

//////////////////////////////////////////////////////////////////////

using namespace jit::reg;
using boost::implicit_cast;

TRACE_SET_MOD(ustubs);

//////////////////////////////////////////////////////////////////////

namespace {

void moveToAlign(CodeBlock& cb) {
  align(cb, Alignment::JmpTarget, AlignContext::Dead);
}

//////////////////////////////////////////////////////////////////////

extern "C" void enterTCExit();

void emitCallToExit(UniqueStubs& uniqueStubs) {
  Asm a { mcg->code.main() };

  // Emit a byte of padding. This is a kind of hacky way to avoid
  // hitting an assert in recordGdbStub when we call it with stub - 1
  // as the start address.
  a.emitNop(1);
  auto const stub = a.frontier();
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
  uniqueStubs.callToExit = uniqueStubs.add("callToExit", stub);
}

void emitThrowSwitchMode(UniqueStubs& uniqueStubs) {
  Asm a{mcg->code.frozen()};
  moveToAlign(a.code());

  uniqueStubs.throwSwitchMode = a.frontier();
  a.    call(TCA(throwSwitchMode));
  a.    ud2();

  uniqueStubs.add("throwSwitchMode", uniqueStubs.throwSwitchMode);
}

void emitCatchHelper(UniqueStubs& uniqueStubs) {
  Asm a { mcg->code.frozen() };
  moveToAlign(mcg->code.frozen());
  Label debuggerReturn;
  Label resumeCppUnwind;

  uniqueStubs.endCatchHelper = a.frontier();
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
  uniqueStubs.endCatchHelperPast = a.frontier();
  a.    ud2();

asm_label(a, debuggerReturn);
  a.    loadq (rvmtl()[unwinderDebuggerReturnSPOff()], rvmsp());
  a.    storeq(0, rvmtl()[unwinderDebuggerReturnSPOff()]);
  svcreq::emit_persistent(a.code(), folly::none, REQ_POST_DEBUGGER_RET);

  uniqueStubs.add("endCatchHelper", uniqueStubs.endCatchHelper);
}

void emitFreeLocalsHelpers(UniqueStubs& uniqueStubs) {
  Label doRelease;
  Label release;
  Label loopHead;

  auto const rData     = rarg(0); // not live coming in, but used
                                             // for destructor calls
  auto const rIter     = rarg(1); // live coming in
  auto const rFinished = rdx;
  auto const rType     = ecx;
  int const tvSize     = sizeof(TypedValue);

  auto& cb = mcg->code.hot().available() > 512 ?
    const_cast<CodeBlock&>(mcg->code.hot()) : mcg->code.main();
  Asm a { cb };
  align(cb, Alignment::CacheLine, AlignContext::Dead);
  auto stubBegin = a.frontier();

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

  moveToAlign(cb);
  uniqueStubs.freeManyLocalsHelper = a.frontier();
  a.    lea    (rvmfp()[-(jit::kNumFreeLocalsHelpers * sizeof(Cell))],
                rFinished);

  // Loop for the first few locals, but unroll the final kNumFreeLocalsHelpers.
asm_label(a, loopHead);
  emitDecLocal();
  a.    addq   (tvSize, rIter);
  a.    cmpq   (rIter, rFinished);
  a.    jnz8   (loopHead);

  for (int i = 0; i < kNumFreeLocalsHelpers; ++i) {
    uniqueStubs.freeLocalsHelpers[kNumFreeLocalsHelpers - i - 1] = a.frontier();
    emitDecLocal();
    if (i != kNumFreeLocalsHelpers - 1) {
      a.addq   (tvSize, rIter);
    }
  }

  a.    ret    ();

  // Keep me small!
  always_assert(Stats::enabled() ||
                (a.frontier() - stubBegin <= 4 * kX64CacheLineSize));

  uniqueStubs.add("freeLocalsHelpers", stubBegin);
}

void emitDecRefHelper(UniqueStubs& us) {
  auto& cold = mcg->code.cold();
  const Vreg rData{rarg(0)};
  const Vreg rType{rarg(1)};

  auto doStub = [&](Type ty) {
    assert(ty.maybe(TCounted));
    auto const start = cold.frontier();
    Vauto vasm{cold};
    auto& v = vasm.main();

    auto destroy = [&](Vout& v) {
      auto const toSave = abi().gpUnreserved - abi().calleeSaved;
      PhysRegSaver save{v, toSave, false /* aligned */};

      assert(!ty.isKnownDataType() && ty.maybe(TCounted));
      // We've saved all caller-saved registers so it's ok to use them as
      // scratch, including rType. This duplicates work done in
      // lookupDestructor() but we have to be careful to not use arbitrary
      // Vregs for anything other than status flags in this stub.
      v << movzbq{rType, rType};
      v << shrli{kShiftDataTypeToDestrIndex, rType, rType, v.makeReg()};
      auto const dtor_table =
        safe_cast<int>(reinterpret_cast<intptr_t>(g_destructors));
      v << callm{Vptr{Vreg{}, rType, 8, dtor_table}, arg_regs(1)};
      v << syncpoint{makeIndirectFixup(save.dwordsPushed())};
    };

    emitDecRefWork(v, v, rData, destroy, false);

    v << ret{};
    return start;
  };

  us.decRefGeneric = us.add("decRefGeneric", doStub(TGen));
}

void emitFCallArrayHelper(UniqueStubs& uniqueStubs) {
  auto& cb = mcg->code.hot().available() > 512 ?
    const_cast<CodeBlock&>(mcg->code.hot()) : mcg->code.main();
  Asm a { cb };

  align(cb, Alignment::CacheLine, AlignContext::Dead);
  uniqueStubs.fcallArrayHelper = a.frontier();

  /*
   * When translating FCallArray, we have a pre-live ActRec on the stack and an
   * Array of the parameters.  This stub uses the interpreter functions to
   * enter the ActRec, but those functions may also tell us not to run it.  We
   * reach this stub using a call instruction from the TC.
   *
   * In the case we're told to run it, we pop the return IP from the call and
   * put it in the pre-live ActRec, link it as the frame pointer, and then jump
   * directly to the prologue for the function being called.  This is done to
   * keep the return stack buffer balanced with the call to this stub.  If
   * we're told not to (e.g. the call_user_func_array was intercepted), we just
   * return to our caller in the TC after re-loading the VM regs (the
   * interpreter will have popped the pre-live ActRec for us).
   *
   * NOTE: Our ABI for php-level calls only has two callee-saved registers:
   * rvmfp() and rvmtl(), so we're allowed to use any other regs without saving
   * them.
   */

  Label noCallee;

  auto const rPCOff  = rarg(0);
  auto const rPCNext = rarg(1);
  auto const rBC     = r13;

  a.    storeq (rvmfp(), rvmtl()[rds::kVmfpOff]);
  a.    storeq (rvmsp(), rvmtl()[rds::kVmspOff]);

  // rBC := fp -> m_func -> m_unit -> m_bc
  a.    loadq  (rvmfp()[AROFF(m_func)], rBC);
  a.    loadq  (rBC[Func::unitOff()], rBC);
  a.    loadq  (rBC[Unit::bcOff()],   rBC);
  // Convert offsets into PC's and sync the PC
  a.    addq   (rBC,    rPCOff);
  a.    storeq (rPCOff, rvmtl()[rds::kVmpcOff]);
  a.    addq   (rBC,    rPCNext);

  a.    subq   (8, rsp());  // stack parity

  a.    movq   (rPCNext, rarg(0));
  a.    call   (TCA(&doFCallArrayTC));

  a.    loadq  (rvmtl()[rds::kVmspOff], rvmsp());

  a.    testb  (rbyte(rax), rbyte(rax));
  a.    jz8    (noCallee);

  a.    addq   (8, rsp());
  a.    loadq  (rvmtl()[rds::kVmfpOff], rvmfp());
  a.    pop    (rvmfp()[AROFF(m_savedRip)]);
  a.    loadq  (rvmfp()[AROFF(m_func)], rax);
  a.    loadq  (rax[Func::funcBodyOff()], rax);
  a.    jmp    (rax);
  a.    ud2    ();

asm_label(a, noCallee);
  a.    addq   (8, rsp());
  a.    ret    ();

  uniqueStubs.add("fcallArrayHelper", uniqueStubs.fcallArrayHelper);
}

//////////////////////////////////////////////////////////////////////

void emitFunctionEnterHelper(UniqueStubs& uniqueStubs) {
  bool (*helper)(const ActRec*, int) = &EventHook::onFunctionCall;
  Asm a { mcg->code.cold() };

  moveToAlign(mcg->code.cold());
  uniqueStubs.functionEnterHelper = a.frontier();

  Label skip;

  PhysReg ar = rarg(0);

  a.   movq    (rvmfp(), ar);
  a.   push    (rvmfp());
  a.   movq    (rsp(), rvmfp());
  a.   push    (ar[AROFF(m_savedRip)]);
  a.   push    (ar[AROFF(m_sfp)]);
  a.   movq    (EventHook::NormalFunc, rarg(1));
  emitCall(a, CppCall::direct(helper), arg_regs(2));
  uniqueStubs.functionEnterHelperReturn = a.frontier();
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

  uniqueStubs.add("functionEnterHelper", uniqueStubs.functionEnterHelper);
}

void emitFunctionSurprisedOrStackOverflow(UniqueStubs& uniqueStubs) {
  Asm a { mcg->code.cold() };

  moveToAlign(mcg->code.main());
  uniqueStubs.functionSurprisedOrStackOverflow = a.frontier();

  /*
   * We might be here because of a stack overflow, or because of a real
   * surprise, or because of a spurious wake up where we raced with a
   * background thread clearing surprise flags.
   *
   * We need to verify whether it is a stack overflow, because the handling of
   * that is different.  However, if it was a spurious wake up it's fine to
   * just pretend we had a real surprise---the surprise handler rechecks all
   * the flags and clears them as necessary.  It will set the stack top trigger
   * back if no flags are actually set.
   */

  // If handlePossibleStackOverflow returns, it was not a stack overflow, so we
  // need to go through event hook processing.
  a.    subq   (8, rsp());  // align native stack
  a.    movq   (rvmfp(), rarg(0));
  emitCall(a, CppCall::direct(handlePossibleStackOverflow), arg_regs(1));
  a.    addq   (8, rsp());
  a.    jmp    (uniqueStubs.functionEnterHelper);
  a.    ud2    ();

  uniqueStubs.add("functionSurprisedOrStackOverflow",
                  uniqueStubs.functionSurprisedOrStackOverflow);
}

}

//////////////////////////////////////////////////////////////////////

void emitUniqueStubs(UniqueStubs& us) {
  auto functions = {
    emitCallToExit,
    emitThrowSwitchMode,
    emitCatchHelper,
    emitFreeLocalsHelpers,
    emitDecRefHelper,
    emitFCallArrayHelper,
    emitFunctionEnterHelper,
    emitFunctionSurprisedOrStackOverflow,
  };
  for (auto& f : functions) f(us);
}

//////////////////////////////////////////////////////////////////////

}}}
