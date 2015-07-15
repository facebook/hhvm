/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP { namespace jit { namespace x64 {

//////////////////////////////////////////////////////////////////////

using namespace jit::reg;
using boost::implicit_cast;

TRACE_SET_MOD(ustubs);

//////////////////////////////////////////////////////////////////////

namespace {

TCA emitRetFromInterpretedFrame() {
  Asm a { mcg->code.cold() };
  moveToAlign(mcg->code.cold());
  auto const ret = a.frontier();

  auto const arBase = static_cast<int32_t>(sizeof(ActRec) - sizeof(Cell));
  a.   lea  (rVmSp[-arBase], serviceReqArgRegs[0]);
  a.   movq (rVmFp, serviceReqArgRegs[1]);
  emitServiceReq(mcg->code.cold(), SRFlags::None, folly::none,
    REQ_POST_INTERP_RET);
  return ret;
}

template <bool async>
TCA emitRetFromInterpretedGeneratorFrame() {
  Asm a { mcg->code.cold() };
  moveToAlign(mcg->code.cold());
  auto const ret = a.frontier();
  auto const arOff = BaseGenerator::arOff() -
    (async ? AsyncGenerator::objectOff() : Generator::objectOff());

  // We have to get the Generator object from the current AR's $this, then
  // find where its embedded AR is.
  PhysReg rContAR = serviceReqArgRegs[0];
  a.    loadq  (rVmFp[AROFF(m_this)], rContAR);
  a.    lea    (rContAR[arOff], rContAR);
  a.    movq   (rVmFp, serviceReqArgRegs[1]);
  emitServiceReq(mcg->code.cold(), SRFlags::None, folly::none,
    REQ_POST_INTERP_RET);
  return ret;
}

TCA emitDebuggerRetFromInterpretedFrame() {
  Asm a { mcg->code.cold() };
  moveToAlign(a.code());
  auto const ret = a.frontier();

  auto const rCallee = argNumToRegName[0];
  auto const arBase = static_cast<int32_t>(sizeof(ActRec) - sizeof(Cell));
  a.  lea   (rVmSp[-arBase], rCallee);
  a.  loadl (rCallee[AROFF(m_soff)], eax);
  a.  storel(eax, rVmTl[unwinderDebuggerReturnOffOff()]);
  a.  storeq(rVmSp, rVmTl[unwinderDebuggerReturnSPOff()]);
  a.  call  (TCA(popDebuggerCatch));
  a.  movq  (rVmFp, rdx); // llvm catch traces expect rVmFp to be in rdx.
  a.  jmp   (rax);

  return ret;
}

template <bool async>
TCA emitDebuggerRetFromInterpretedGenFrame() {
  Asm a { mcg->code.cold() };
  moveToAlign(a.code());
  auto const ret = a.frontier();
  auto const arOff = BaseGenerator::arOff() -
    (async ? AsyncGenerator::objectOff() : Generator::objectOff());

  // We have to get the Generator object from the current AR's $this, then
  // find where its embedded AR is.
  PhysReg rContAR = argNumToRegName[0];
  a.  loadq (rVmFp[AROFF(m_this)], rContAR);
  a.  lea   (rContAR[arOff], rContAR);
  a.  loadl (rContAR[AROFF(m_soff)], eax);
  a.  storel(eax, rVmTl[unwinderDebuggerReturnOffOff()]);
  a.  storeq(rVmSp, rVmTl[unwinderDebuggerReturnSPOff()]);
  a.  call  (TCA(popDebuggerCatch));
  a.  movq  (rVmFp, rdx); // llvm catch traces expect rVmFp to be in rdx.
  a.  jmp   (rax);

  return ret;
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
    a.cmpq(rax, *rsp);
    a.je8 (ok);
    a.ud2();
  asm_label(a, ok);
  }

  // Emulate a ret to enterTCExit without actually doing one to avoid
  // unbalancing the return stack buffer. The call from enterTCHelper() that
  // got us into the TC was popped off the RSB by the ret that got us to this
  // stub.
  a.addq(8, rsp);
  a.jmp(TCA(enterTCExit));

  // On a backtrace, gdb tries to locate the calling frame at address
  // returnRIP-1. However, for the first VM frame, there is no code at
  // returnRIP-1, since the AR was set up manually. For this frame,
  // record the tracelet address as starting from this callToExit-1,
  // so gdb does not barf.
  uniqueStubs.callToExit = uniqueStubs.add("callToExit", stub);
}

void emitReturnHelpers(UniqueStubs& us) {
  us.retHelper    = us.add("retHelper", emitRetFromInterpretedFrame());
  us.genRetHelper = us.add("genRetHelper",
                           emitRetFromInterpretedGeneratorFrame<false>());
  us.asyncGenRetHelper = us.add("asyncGenRetHelper",
                           emitRetFromInterpretedGeneratorFrame<true>());
  us.retInlHelper = us.add("retInlHelper", emitRetFromInterpretedFrame());
  us.debuggerRetHelper =
    us.add("debuggerRetHelper", emitDebuggerRetFromInterpretedFrame());
  us.debuggerGenRetHelper = us.add("debuggerGenRetHelper",
    emitDebuggerRetFromInterpretedGenFrame<false>());
  us.debuggerAsyncGenRetHelper = us.add("debuggerAsyncGenRetHelper",
    emitDebuggerRetFromInterpretedGenFrame<true>());
}

void emitResumeInterpHelpers(UniqueStubs& uniqueStubs) {
  Asm a { mcg->code.main() };
  moveToAlign(mcg->code.main());
  Label resumeHelper;
  Label resumeRaw;

  uniqueStubs.interpHelper = a.frontier();
  a.    storeq (argNumToRegName[0], rVmTl[rds::kVmpcOff]);
  uniqueStubs.interpHelperSyncedPC = a.frontier();
  a.    storeq (rVmSp, rVmTl[rds::kVmspOff]);
  a.    storeq (rVmFp, rVmTl[rds::kVmfpOff]);
  a.    movb   (1, rbyte(argNumToRegName[1])); // interpFirst
  a.    jmp8   (resumeHelper);

  uniqueStubs.resumeHelperRet = a.frontier();
  a.    pop   (rVmFp[AROFF(m_savedRip)]);
  uniqueStubs.resumeHelper = a.frontier();
  a.    movb  (0, rbyte(argNumToRegName[1])); // interpFirst
asm_label(a, resumeHelper);
  a.    loadq (rVmTl[rds::kVmfpOff], rVmFp);
  a.    loadq (rip[intptr_t(&mcg)], argNumToRegName[0]);
  a.    call  (TCA(getMethodPtr(&MCGenerator::handleResume)));
asm_label(a, resumeRaw);
  a.    loadq (rVmTl[rds::kVmspOff], rVmSp);
  a.    loadq (rVmTl[rds::kVmfpOff], rVmFp);
  a.    movq  (rVmFp, rdx); // llvm catch traces expect rVmFp to be in rdx.
  a.    jmp   (rax);

  uniqueStubs.add("resumeInterpHelpers", uniqueStubs.interpHelper);

  auto emitInterpOneStub = [&](const Op op) {
    Asm a{mcg->code.cold()};
    moveToAlign(mcg->code.cold());
    auto const start = a.frontier();

    a.  movq(rVmFp, argNumToRegName[0]);
    a.  movq(rVmSp, argNumToRegName[1]);
    a.  movl(r32(rAsm), r32(argNumToRegName[2]));
    a.  call(TCA(interpOneEntryPoints[size_t(op)]));
    a.  testq(rax, rax);
    a.  jnz(resumeRaw);
    a.  jmp(uniqueStubs.resumeHelper);

    uniqueStubs.interpOneCFHelpers[op] = start;
    uniqueStubs.add(
      folly::sformat("interpOneCFHelper-{}", opcodeToName(op)).c_str(),
      start
    );
  };

# define O(name, imm, in, out, flags)                       \
  if (bool((flags) & CF) || bool((flags) & TF)) {           \
    emitInterpOneStub(Op::name);                            \
  }
  OPCODES
# undef O
  // Exit is a very special snowflake: because it can appear in PHP
  // expressions, the emitter pretends that it pushed a value on the eval stack
  // (and iopExit actually does push Null right before throwing). Marking it as
  // TF would mess up any bytecodes that want to consume its output value, so
  // we can't do that. But we also don't want to extend tracelets past it, so
  // the JIT treats it as terminal and uses InterpOneCF to execute it. So,
  // manually make sure we have an interpOneExit stub.
  emitInterpOneStub(Op::Exit);
}

void emitThrowSwitchMode(UniqueStubs& uniqueStubs) {
  Asm a{mcg->code.frozen()};
  moveToAlign(a.code());

  uniqueStubs.throwSwitchMode = a.frontier();
  a.    call(TCA(throwSwitchMode));
  a.    ud2();

  uniqueStubs.add("throwSwitchMode", uniqueStubs.throwSwitchMode);
}

#ifndef USE_GCC_FAST_TLS
void unwindResumeHelper() {
  tl_regState = VMRegState::CLEAN;
  _Unwind_Resume(unwindRdsInfo->exn);
}
#endif

void emitCatchHelper(UniqueStubs& uniqueStubs) {
  Asm a { mcg->code.frozen() };
  moveToAlign(mcg->code.frozen());
  Label debuggerReturn;
  Label resumeCppUnwind;

  uniqueStubs.endCatchHelper = a.frontier();
  a.    cmpq (0, rVmTl[unwinderDebuggerReturnSPOff()]);
  a.    jne8 (debuggerReturn);

  // Normal endCatch situation: call back to tc_unwind_resume, which returns
  // the catch trace (or null) in rax and the new vmfp in rdx.
  a.    movq (rVmFp, argNumToRegName[0]);
  a.    call (TCA(tc_unwind_resume));
  a.    movq (rdx, rVmFp);
  a.    testq(rax, rax);
  a.    jz8  (resumeCppUnwind);
  a.    jmp  (rax);  // rdx is still live if we're going to code from llvm

asm_label(a, resumeCppUnwind);
#ifdef USE_GCC_FAST_TLS
  static_assert(sizeof(tl_regState) == 1,
                "The following store must match the size of tl_regState");
  a.    fs().storeb(static_cast<int32_t>(VMRegState::CLEAN),
                    getTLSPtr(tl_regState).mr());
#endif
  a.    loadq(rVmTl[unwinderExnOff()], argNumToRegName[0]);
#ifdef USE_GCC_FAST_TLS
  a.    call(TCA(_Unwind_Resume));
#else
  a.    call(TCA(unwindResumeHelper));
#endif
  uniqueStubs.endCatchHelperPast = a.frontier();
  a.    ud2();

asm_label(a, debuggerReturn);
  a.    loadq (rVmTl[unwinderDebuggerReturnSPOff()], rVmSp);
  a.    storeq(0, rVmTl[unwinderDebuggerReturnSPOff()]);
  emitServiceReq(a.code(), SRFlags::None, folly::none,
    REQ_POST_DEBUGGER_RET);

  uniqueStubs.add("endCatchHelper", uniqueStubs.endCatchHelper);
}

void emitStackOverflowHelper(UniqueStubs& uniqueStubs) {
  Asm a { mcg->code.cold() };

  moveToAlign(mcg->code.cold());
  uniqueStubs.stackOverflowHelper = a.frontier();
  a.    movq   (rVmFp, argNumToRegName[0]);
  emitCall(a, CppCall::direct(handleStackOverflow), argSet(1));

  uniqueStubs.add("stackOverflowHelper", uniqueStubs.stackOverflowHelper);
}

void emitFreeLocalsHelpers(UniqueStubs& uniqueStubs) {
  Label doRelease;
  Label release;
  Label loopHead;

  auto const rData     = argNumToRegName[0]; // not live coming in, but used
                                             // for destructor calls
  auto const rIter     = argNumToRegName[1]; // live coming in
  auto const rFinished = rdx;
  auto const rType     = rcx;
  int const tvSize     = sizeof(TypedValue);

  auto& cb = mcg->code.hot().available() > 512 ?
    const_cast<CodeBlock&>(mcg->code.hot()) : mcg->code.main();
  Asm a { cb };
  moveToAlign(cb, kNonFallthroughAlign);
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
  mcg->fixupMap().recordFixup(a.frontier(), makeIndirectFixup(3));
  a.    pop     (rFinished);
  a.    pop     (rIter);
  a.    ret     ();

  auto emitDecLocal = [&]() {
    Label skipDecRef;

    emitLoadTVType(a, rIter[TVOFF(m_type)], rType);
    emitCmpTVType(a, KindOfRefCountThreshold, rType);
    a.  jle8   (skipDecRef);
    a.  call   (release);
  asm_label(a, skipDecRef);
  };

  moveToAlign(cb, kJmpTargetAlign);
  uniqueStubs.freeManyLocalsHelper = a.frontier();
  a.    lea    (rVmFp[-(jit::kNumFreeLocalsHelpers * sizeof(Cell))],
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
  const Vreg rData{argNumToRegName[0]};
  const Vreg rType{argNumToRegName[1]};

  auto doStub = [&](Type ty) {
    assert(ty.maybe(TCounted));
    auto const start = cold.frontier();
    Vauto vasm{cold};
    auto& v = vasm.main();

    auto destroy = [&](Vout& v) {
      auto toSave = kGPCallerSaved;
      PhysRegSaverStub save{v, toSave};

      assert(!ty.isKnownDataType() && ty.maybe(TCounted));
      // We've saved all caller-saved registers so it's ok to use them as
      // scratch, including rType. This duplicates work done in
      // lookupDestructor() but we have to be careful to not use arbitrary
      // Vregs for anything other than status flags in this stub.
      v << movzbq{rType, rType};
      v << shrli{kShiftDataTypeToDestrIndex, rType, rType, v.makeReg()};
      auto const dtor_table =
        safe_cast<int>(reinterpret_cast<intptr_t>(g_destructors));
      v << callm{Vptr{Vreg{}, rType, 8, dtor_table}, argSet(1)};
      v << syncpoint{makeIndirectFixup(save.dwordsPushed())};
    };

    emitDecRefWork(v, v, rData, destroy, false);

    v << ret{};
    return start;
  };

  us.genDecRefHelper = us.add("genDecRefHelper", doStub(TGen));
}

void emitFuncPrologueRedispatch(UniqueStubs& uniqueStubs) {
  auto& cb = mcg->code.hot().available() > 512 ?
    const_cast<CodeBlock&>(mcg->code.hot()) : mcg->code.main();
  Asm a { cb };

  moveToAlign(cb);
  uniqueStubs.funcPrologueRedispatch = a.frontier();

  assertx(kScratchCrossTraceRegs.contains(rax));
  assertx(kScratchCrossTraceRegs.contains(rdx));
  assertx(kScratchCrossTraceRegs.contains(rcx));

  Label actualDispatch;
  Label numParamsCheck;

  // rax := called func
  // edx := num passed parameters
  // ecx := num declared parameters
  a.    loadq  (rVmFp[AROFF(m_func)], rax);
  a.    loadl  (rVmFp[AROFF(m_numArgsAndFlags)], edx);
  a.    andl   (ActRec::kNumArgsMask, edx);
  a.    loadl  (rax[Func::paramCountsOff()], ecx);
  // see Func::finishedEmittingParams and Func::numParams for rationale
  a.    shrl   (0x1, ecx);

  // If we passed more args than declared, jump to the numParamsCheck.
  a.    cmpl   (edx, ecx);
  a.    jl8    (numParamsCheck);

asm_label(a, actualDispatch);
  a.    loadq  (rax[rdx*8 + Func::prologueTableOff()], rax);
  a.    jmp    (rax);
  a.    ud2    ();

  // Hmm, more parameters passed than the function expected. Did we
  // pass kNumFixedPrologues or more? If not, %rdx is still a
  // perfectly legitimate index into the func prologue table.
asm_label(a, numParamsCheck);
  a.    cmpl   (kNumFixedPrologues, edx);
  a.    jl8    (actualDispatch);

  // Too many gosh-darned parameters passed. Go to numExpected + 1, which
  // is always a "too many params" entry point.
  a.    loadq  (rax[rcx*8 + Func::prologueTableOff() + sizeof(TCA)], rax);
  a.    jmp    (rax);
  a.    ud2    ();

  uniqueStubs.add("funcPrologueRedispatch", uniqueStubs.funcPrologueRedispatch);
}

void emitFCallArrayHelper(UniqueStubs& uniqueStubs) {
  auto& cb = mcg->code.hot().available() > 512 ?
    const_cast<CodeBlock&>(mcg->code.hot()) : mcg->code.main();
  Asm a { cb };

  moveToAlign(cb, kNonFallthroughAlign);
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
   * rVmFp and rVmTl, so we're allowed to use any other regs without saving
   * them.
   */

  Label noCallee;

  auto const rPCOff  = argNumToRegName[0];
  auto const rPCNext = argNumToRegName[1];
  auto const rBC     = r13;

  a.    storeq (rVmFp, rVmTl[rds::kVmfpOff]);
  a.    storeq (rVmSp, rVmTl[rds::kVmspOff]);

  // rBC := fp -> m_func -> m_unit -> m_bc
  a.    loadq  (rVmFp[AROFF(m_func)], rBC);
  a.    loadq  (rBC[Func::unitOff()], rBC);
  a.    loadq  (rBC[Unit::bcOff()],   rBC);
  // Convert offsets into PC's and sync the PC
  a.    addq   (rBC,    rPCOff);
  a.    storeq (rPCOff, rVmTl[rds::kVmpcOff]);
  a.    addq   (rBC,    rPCNext);

  a.    subq   (8, rsp);  // stack parity

  a.    movq   (rPCNext, argNumToRegName[0]);
  a.    call   (TCA(&doFCallArrayTC));

  a.    loadq  (rVmTl[rds::kVmspOff], rVmSp);

  a.    testb  (rbyte(rax), rbyte(rax));
  a.    jz8    (noCallee);

  a.    addq   (8, rsp);
  a.    loadq  (rVmTl[rds::kVmfpOff], rVmFp);
  a.    pop    (rVmFp[AROFF(m_savedRip)]);
  a.    loadq  (rVmFp[AROFF(m_func)], rax);
  a.    loadq  (rax[Func::funcBodyOff()], rax);
  a.    jmp    (rax);
  a.    ud2    ();

asm_label(a, noCallee);
  a.    addq   (8, rsp);
  a.    ret    ();

  uniqueStubs.add("fcallArrayHelper", uniqueStubs.fcallArrayHelper);
}

//////////////////////////////////////////////////////////////////////

void emitFCallHelperThunk(UniqueStubs& uniqueStubs) {
  TCA (*helper)(ActRec*) = &fcallHelper;
  Asm a { mcg->code.cold() };

  moveToAlign(mcg->code.cold());
  uniqueStubs.fcallHelperThunk = a.frontier();

  Label skip;

  // fcallHelper may call doFCall. doFCall changes the return ip
  // pointed to by r15 so that it points to MCGenerator::m_retHelper,
  // which does a REQ_POST_INTERP_RET service request. So we need to
  // to pop the return address into r15 + m_savedRip before calling
  // fcallHelper, and then push it back from r15 + m_savedRip after
  // fcallHelper returns in case it has changed it.
  a.    pop    (rVmFp[AROFF(m_savedRip)]);
  a.    movq   (rVmFp, argNumToRegName[0]); // live for helper calls below
  emitCall(a, CppCall::direct(helper), argSet(1));
  if (debug) {
    a.  movq   (0x1, rVmSp);  // "assert" nothing reads it
  }
  a.    testq  (rax, rax);
  a.    js8    (skip);
  a.    push   (rVmFp[AROFF(m_savedRip)]);
  a.    jmp    (rax);
  a.    ud2    ();

asm_label(a, skip);
  a.    neg    (rax);
  a.    loadq  (rVmTl[rds::kVmfpOff], rVmFp);
  a.    loadq  (rVmTl[rds::kVmspOff], rVmSp);
  a.    jmp    (rax);
  a.    ud2    ();

  uniqueStubs.add("fcallHelperThunk", uniqueStubs.fcallHelperThunk);
}

void emitFuncBodyHelperThunk(UniqueStubs& uniqueStubs) {
  TCA (*helper)(ActRec*) = &funcBodyHelper;
  Asm a { mcg->code.cold() };

  moveToAlign(mcg->code.cold());
  uniqueStubs.funcBodyHelperThunk = a.frontier();

  // This helper is called via a direct jump from the TC (from
  // fcallArrayHelper). So the stack parity is already correct.
  a.    movq   (rVmFp, argNumToRegName[0]);
  emitCall(a, CppCall::direct(helper), argSet(1));
  a.    jmp    (rax);
  a.    ud2    ();

  uniqueStubs.add("funcBodyHelperThunk", uniqueStubs.funcBodyHelperThunk);
}

void emitFunctionEnterHelper(UniqueStubs& uniqueStubs) {
  bool (*helper)(const ActRec*, int) = &EventHook::onFunctionCall;
  Asm a { mcg->code.cold() };

  moveToAlign(mcg->code.cold());
  uniqueStubs.functionEnterHelper = a.frontier();

  Label skip;

  PhysReg ar = argNumToRegName[0];

  a.   movq    (rVmFp, ar);
  a.   push    (rVmFp);
  a.   movq    (rsp, rVmFp);
  a.   push    (ar[AROFF(m_savedRip)]);
  a.   push    (ar[AROFF(m_sfp)]);
  a.   movq    (EventHook::NormalFunc, argNumToRegName[1]);
  emitCall(a, CppCall::direct(helper), argSet(2));
  uniqueStubs.functionEnterHelperReturn = a.frontier();
  a.   testb   (al, al);
  a.   je8     (skip);
  a.   addq    (16, rsp);
  a.   pop     (rVmFp);
  a.   ret     ();

asm_label(a, skip);
  // The event hook has already cleaned up the stack/actrec so that we're ready
  // to continue from the original call site.  Just need to grab the fp/rip
  // from the original frame, and sync rVmSp to the execution-context's copy.
  a.   pop     (rVmFp);
  a.   pop     (rsi);
  a.   addq    (16, rsp); // drop our call frame
  a.   loadq   (rVmTl[rds::kVmspOff], rVmSp);
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
  a.    subq   (8, rsp);  // align native stack
  a.    movq   (rVmFp, argNumToRegName[0]);
  emitCall(a, CppCall::direct(handlePossibleStackOverflow), argSet(1));
  a.    addq   (8, rsp);
  a.    jmp    (uniqueStubs.functionEnterHelper);
  a.    ud2    ();

  uniqueStubs.add("functionSurprisedOrStackOverflow",
                  uniqueStubs.functionSurprisedOrStackOverflow);
}

void emitBindCallStubs(UniqueStubs& uniqueStubs) {
  auto emitStub = [](bool immutable) {
    auto& cb = mcg->code.cold();
    auto const start = cb.frontier();
    Asm a{cb};
    a.  loadq  (rip[intptr_t(&mcg)], argNumToRegName[0]);
    a.  loadq  (*rsp, argNumToRegName[1]); // reconstruct toSmash from savedRip
    a.  subq   (kCallLen, argNumToRegName[1]);
    a.  movq   (rVmFp, argNumToRegName[2]);
    a.  movb   (immutable, rbyte(argNumToRegName[3]));
    a.  subq   (8, rsp); // align stack
    a.  call   (TCA(getMethodPtr(&MCGenerator::handleBindCall)));
    a.  addq   (8, rsp);
    a.  jmp    (rax);
    return start;
  };

  uniqueStubs.bindCallStub = emitStub(false);
  uniqueStubs.add("bindCallStub", uniqueStubs.bindCallStub);
  uniqueStubs.immutableBindCallStub = emitStub(true);
  uniqueStubs.add("immutableBindCallStub", uniqueStubs.immutableBindCallStub);
}

}

//////////////////////////////////////////////////////////////////////

UniqueStubs emitUniqueStubs() {
  UniqueStubs us;
  auto functions = {
    emitCallToExit,
    emitReturnHelpers,
    emitResumeInterpHelpers,
    emitThrowSwitchMode,
    emitCatchHelper,
    emitStackOverflowHelper,
    emitFreeLocalsHelpers,
    emitDecRefHelper,
    emitFuncPrologueRedispatch,
    emitFCallArrayHelper,
    emitFCallHelperThunk,
    emitFuncBodyHelperThunk,
    emitFunctionEnterHelper,
    emitFunctionSurprisedOrStackOverflow,
    emitBindCallStubs,
  };
  for (auto& f : functions) f(us);
  return us;
}

//////////////////////////////////////////////////////////////////////

}}}
