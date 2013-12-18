/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/translator-x64-internal.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP { namespace JIT { namespace X64 {

//////////////////////////////////////////////////////////////////////

using namespace JIT::reg;
using boost::implicit_cast;

TRACE_SET_MOD(ustubs);

//////////////////////////////////////////////////////////////////////

namespace {

TCA emitRetFromInterpretedFrame() {
  Asm a { tx64->stubsCode };
  moveToAlign(tx64->stubsCode);
  auto const ret = a.frontier();

  auto const arBase = static_cast<int32_t>(sizeof(ActRec) - sizeof(Cell));
  a.   lea  (rVmSp[-arBase], serviceReqArgRegs[0]);
  a.   movq (rVmFp, serviceReqArgRegs[1]);
  emitServiceReq(tx64->stubsCode, SRFlags::JmpInsteadOfRet,
                 REQ_POST_INTERP_RET);
  return ret;
}

TCA emitRetFromInterpretedGeneratorFrame() {
  Asm a { tx64->stubsCode };
  moveToAlign(tx64->stubsCode);
  auto const ret = a.frontier();

  // We have to get the Continuation object from the current AR's $this, then
  // find where its embedded AR is.
  PhysReg rContAR = serviceReqArgRegs[0];
  a.    loadq  (rVmFp[AROFF(m_this)], rContAR);
  a.    lea  (rContAR[c_Continuation::getArOffset()], rContAR);
  a.    movq   (rVmFp, serviceReqArgRegs[1]);
  emitServiceReq(tx64->stubsCode, SRFlags::JmpInsteadOfRet,
                 REQ_POST_INTERP_RET);
  return ret;
}

//////////////////////////////////////////////////////////////////////

void emitCallToExit(UniqueStubs& uniqueStubs) {
  Asm a { tx64->mainCode };

  // Emit a byte of padding. This is a kind of hacky way to avoid
  // hitting an assert in recordGdbStub when we call it with stub - 1
  // as the start address.
  a.emitNop(1);
  auto const stub = emitServiceReq(
    tx64->mainCode,
    SRFlags::Align | SRFlags::JmpInsteadOfRet,
    REQ_EXIT
  );

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
                           emitRetFromInterpretedGeneratorFrame());
  us.retInlHelper = us.add("retInlHelper", emitRetFromInterpretedFrame());
}

void emitResumeHelpers(UniqueStubs& uniqueStubs) {
  Asm a { tx64->mainCode };
  moveToAlign(tx64->mainCode);

  auto const fpOff = offsetof(VMExecutionContext, m_fp);
  auto const spOff = offsetof(VMExecutionContext, m_stack) +
                       Stack::topOfStackOffset();

  uniqueStubs.resumeHelperRet = a.frontier();
  a.    pop   (rStashedAR[AROFF(m_savedRip)]);
  uniqueStubs.resumeHelper = a.frontier();
  emitGetGContext(a, rax);
  a.   loadq  (rax[fpOff], rVmFp);
  a.   loadq  (rax[spOff], rVmSp);
  emitServiceReq(tx64->mainCode, REQ_RESUME);

  uniqueStubs.add("resumeHelpers", uniqueStubs.resumeHelper);
}

void emitDefClsHelper(UniqueStubs& uniqueStubs) {
  Asm a { tx64->mainCode };
  uniqueStubs.defClsHelper = a.frontier();

  void (*helper)(PreClass*) = defClsHelper;
  PhysReg rEC = argNumToRegName[2];
  emitGetGContext(a, rEC);
  a.   storeq (rVmFp, rEC[offsetof(VMExecutionContext, m_fp)]);
  a.   storeq (argNumToRegName[1],
                  rEC[offsetof(VMExecutionContext, m_pc)]);
  a.   storeq (rax, rEC[offsetof(VMExecutionContext, m_stack) +
                    Stack::topOfStackOffset()]);
  a.   jmp    (TCA(helper));

  uniqueStubs.add("defClsHelper", uniqueStubs.defClsHelper);
}

void emitStackOverflowHelper(UniqueStubs& uniqueStubs) {
  Asm a { tx64->stubsCode };

  moveToAlign(tx64->stubsCode);
  uniqueStubs.stackOverflowHelper = a.frontier();

  // We are called from emitStackCheck, with the new stack frame in
  // rStashedAR. Get the caller's PC into rdi and save it off.
  a.    loadq  (rVmFp[AROFF(m_func)], rax);
  a.    loadl  (rStashedAR[AROFF(m_soff)], edi);
  a.    loadq  (rax[Func::sharedOffset()], rax);
  a.    loadl  (rax[Func::sharedBaseOffset()], eax);
  a.    addl   (eax, edi);
  emitEagerVMRegSave(a, RegSaveFlags::SaveFP | RegSaveFlags::SavePC);
  emitServiceReq(tx64->stubsCode, REQ_STACK_OVERFLOW);

  uniqueStubs.add("stackOverflowHelper", uniqueStubs.stackOverflowHelper);
}

void emitFreeLocalsHelpers(UniqueStubs& uniqueStubs) {
  Label doRelease;
  Label release;
  Label loopHead;

  /*
   * Note: the IR currently requires that we preserve r13/r14 across
   * calls to these free locals helpers.  These helpers assume the
   * stack is balanced (rsp%16 == 0) on entry, unlike normal ABI calls
   * where the stack was balanced before the call, and now has the
   * return address on the stack (rsp%16 == 8).
   */
  static_assert(rVmSp == rbx, "");
  auto const rIter     = rbx;
  auto const rFinished = r15;
  auto const rType     = esi;
  auto const rData     = rdi;

  Asm a { tx64->mainCode };
  moveToAlign(tx64->mainCode, kNonFallthroughAlign);

asm_label(a, release);
  a.    loadq  (rIter[TVOFF(m_data)], rData);
  a.    cmpl   (1, rData[FAST_REFCOUNT_OFFSET]);
  jccBlock<CC_L>(a, [&] {
    a.  jz8    (doRelease);
    a.  decl   (rData[FAST_REFCOUNT_OFFSET]);
  });
  a.    ret    ();
asm_label(a, doRelease);
  jumpDestructor(a, PhysReg(rType), rax);

  moveToAlign(tx64->mainCode, kJmpTargetAlign);
  uniqueStubs.freeManyLocalsHelper = a.frontier();
  a.    lea    (rVmFp[-(JIT::kNumFreeLocalsHelpers * sizeof(Cell))],
                rFinished);

  auto emitDecLocal = [&] {
    Label skipDecRef;

    emitLoadTVType(a, rIter[TVOFF(m_type)], rType);
    emitCmpTVType(a, KindOfRefCountThreshold, rType);
    a.  jle8   (skipDecRef);
    a.  call   (release);
    tx64->fixupMap().recordIndirectFixup(a.frontier(), 0);
  asm_label(a, skipDecRef);
  };

  // Loop for the first few locals, but unroll the final
  // kNumFreeLocalsHelpers.
asm_label(a, loopHead);
  emitDecLocal();
  a.    addq   (sizeof(TypedValue), rIter);
  a.    cmpq   (rIter, rFinished);
  a.    jnz8   (loopHead);

  for (int i = 0; i < kNumFreeLocalsHelpers; ++i) {
    uniqueStubs.freeLocalsHelpers[kNumFreeLocalsHelpers - i - 1] = a.frontier();
    emitDecLocal();
    if (i != kNumFreeLocalsHelpers - 1) {
      a.addq   (sizeof(TypedValue), rIter);
    }
  }

  a.    addq   (AROFF(m_r) + sizeof(TypedValue), rVmSp);
  a.    ret    ();

  uniqueStubs.add("freeLocalsHelpers", uniqueStubs.freeManyLocalsHelper);
}

void emitFuncPrologueRedispatch(UniqueStubs& uniqueStubs) {
  Asm a { tx64->mainCode };

  moveToAlign(tx64->mainCode);
  uniqueStubs.funcPrologueRedispatch = a.frontier();

  assert(kScratchCrossTraceRegs.contains(rax));
  assert(kScratchCrossTraceRegs.contains(rdx));
  assert(kScratchCrossTraceRegs.contains(rcx));

  Label actualDispatch;
  Label numParamsCheck;

  // rax := called func
  // edx := num passed parameters
  // ecx := num declared parameters
  a.    loadq  (rStashedAR[AROFF(m_func)], rax);
  a.    loadl  (rStashedAR[AROFF(m_numArgsAndCtorFlag)], edx);
  a.    andl   (0x7fffffff, edx);
  a.    loadl  (rax[Func::numParamsOff()], ecx);

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
  Asm a { tx64->mainCode };

  moveToAlign(tx64->mainCode, kNonFallthroughAlign);
  uniqueStubs.fcallArrayHelper = a.frontier();

  /*
   * When translating FCallArray, we have a pre-live ActRec on the
   * stack and an Array of the parameters.  This stub uses the
   * interpreter functions to enter the ActRec, but those functions
   * may also tell us not to run it.  We reach this stub using a call
   * instruction from the TC.
   *
   * In the case we're told to run it, we pop the return IP from the
   * call and put it in the pre-live ActRec, link it as the frame
   * pointer, and then jump directly to the prologue for the function
   * being called.  This is done to keep the return stack buffer
   * balanced with the call to this stub.  If we're told not to
   * (e.g. the call_user_func_array was intercepted), we just return
   * to our caller in the TC after re-loading the VM regs (the
   * interpreter will have popped the pre-live ActRec for us).
   *
   * NOTE: we're assuming we don't need to save registers, because we
   * don't ever have live registers across php-level calls.
   */

  Label noCallee;

  auto const rPCOff  = argNumToRegName[0];
  auto const rPCNext = argNumToRegName[1];
  auto const rBC     = r13;
  auto const rEC     = r15;

  auto const spOff = offsetof(VMExecutionContext, m_stack) +
                       Stack::topOfStackOffset();
  auto const fpOff = offsetof(VMExecutionContext, m_fp);
  auto const pcOff = offsetof(VMExecutionContext, m_pc);

  emitGetGContext(a, rEC);
  a.    storeq (rVmFp, rEC[fpOff]);
  a.    storeq (rVmSp, rEC[spOff]);

  // rBC := fp -> m_func -> m_unit -> m_bc
  a.    loadq  (rVmFp[AROFF(m_func)], rBC);
  a.    loadq  (rBC[Func::unitOff()], rBC);
  a.    loadq  (rBC[Unit::bcOff()],   rBC);
  // Convert offsets into PC's and sync the PC
  a.    addq   (rBC,    rPCOff);
  a.    storeq (rPCOff, rEC[pcOff]);
  a.    addq   (rBC,    rPCNext);

  a.    subq   (8, rsp);  // stack parity

  a.    movq   (rEC, argNumToRegName[0]);
  assert(rPCNext == argNumToRegName[1]);
  a.    call   (TCA(getMethodPtr(&VMExecutionContext::doFCallArrayTC)));

  a.    loadq  (rEC[spOff], rVmSp);

  a.    testq  (rax, rax);
  a.    jz8    (noCallee);

  a.    addq   (8, rsp);
  a.    loadq  (rEC[fpOff], rVmFp);
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
  TCA (*helper)(ActRec*, void*) = &fcallHelper;
  Asm a { tx64->mainCode };

  moveToAlign(tx64->mainCode);
  uniqueStubs.fcallHelperThunk = a.frontier();

  Label popAndXchg, skip;

  // fcallHelper is used for prologues, and (in the case of
  // closures) for dispatch to the function body. In the first
  // case, there's a call, in the second, there's a jmp.
  // We can differentiate by comparing r15 and rVmFp
  a.    movq   (rStashedAR, argNumToRegName[0]);
  a.    movq   (rVmSp, argNumToRegName[1]);
  a.    cmpq   (rStashedAR, rVmFp);
  a.    jne8   (popAndXchg);
  emitCall(a, CppCall(helper));
  a.    jmp    (rax);
  // The ud2 is a hint to the processor that the fall-through path of the
  // indirect jump (which it statically predicts as most likely) is not
  // possible.
  a.    ud2    ();

  // fcallHelper may call doFCall. doFCall changes the return ip
  // pointed to by r15 so that it points to TranslatorX64::m_retHelper,
  // which does a REQ_POST_INTERP_RET service request. So we need to
  // to pop the return address into r15 + m_savedRip before calling
  // fcallHelper, and then push it back from r15 + m_savedRip after
  // fcallHelper returns in case it has changed it.
asm_label(a, popAndXchg);
  // There is a brief span from enterTCAtPrologue until the function
  // is entered where rbp is *below* the new actrec, and is missing
  // a number of c++ frames. The new actrec is linked onto the c++
  // frames, however, so switch it into rbp in case fcallHelper throws.
  a.    pop    (rStashedAR[AROFF(m_savedRip)]);
  a.    xchgq  (rStashedAR, rVmFp);
  emitCall(a, CppCall(helper));
  a.    testq  (rax, rax);
  a.    js8    (skip);
  a.    xchgq  (rStashedAR, rVmFp);
  a.    push   (rStashedAR[AROFF(m_savedRip)]);
  a.    jmp    (rax);
  a.    ud2    ();

asm_label(a, skip);
  emitGetGContext(a, rdi);
  a.    neg    (rax);
  a.    loadq  (rdi[offsetof(VMExecutionContext, m_fp)], rVmFp);
  a.    loadq  (rdi[offsetof(VMExecutionContext, m_stack) +
                    Stack::topOfStackOffset()], rVmSp);
  a.    jmp    (rax);
  a.    ud2    ();

  uniqueStubs.add("fcallHelperThunk", uniqueStubs.fcallHelperThunk);
}

void emitFuncBodyHelperThunk(UniqueStubs& uniqueStubs) {
  TCA (*helper)(ActRec*,void*) = &funcBodyHelper;
  Asm a { tx64->mainCode };

  moveToAlign(tx64->mainCode);
  uniqueStubs.funcBodyHelperThunk = a.frontier();

  // This helper is called via a direct jump from the TC (from
  // fcallArrayHelper). So the stack parity is already correct.
  a.    movq   (rVmFp, argNumToRegName[0]);
  a.    movq   (rVmSp, argNumToRegName[1]);
  emitCall(a, CppCall(helper));
  a.    jmp    (rax);
  a.    ud2    ();

  uniqueStubs.add("funcBodyHelperThunk", uniqueStubs.funcBodyHelperThunk);
}

void emitFunctionEnterHelper(UniqueStubs& uniqueStubs) {
  bool (*helper)(const ActRec*, int) = &EventHook::onFunctionEnter;
  Asm a { tx64->mainCode };

  moveToAlign(tx64->mainCode);
  uniqueStubs.functionEnterHelper = a.frontier();

  Label skip;

  PhysReg ar = argNumToRegName[0];

  a.   push    (rVmFp);
  a.   movq    (rsp, rVmFp);
  a.   push    (ar[AROFF(m_savedRip)]);
  a.   push    (ar[AROFF(m_savedRbp)]);
  a.   movq    (EventHook::NormalFunc, argNumToRegName[1]);
  emitCall(a, CppCall(helper));
  a.   testb   (al, al);
  a.   je8     (skip);
  a.   addq    (16, rsp);
  a.   pop     (rVmFp);
  a.   ret     ();
asm_label(a, skip);
// The event hook has already cleaned up the stack/actrec
// so that we're ready to continue from the original call
// site.
// Just need to grab the fp/rip from the original frame,
// and sync rVmSp to the execution-context's copy.
  a.   pop     (rVmFp);
  a.   pop     (rsi);
  a.   addq    (16, rsp); // drop our call frame
  emitGetGContext(a, rax);
  a.   loadq   (rax[offsetof(VMExecutionContext, m_stack) +
                    Stack::topOfStackOffset()], rVmSp);
  a.   jmp     (rsi);
  a.   ud2     ();

  uniqueStubs.add("functionEnterHelper", uniqueStubs.functionEnterHelper);
}

}

//////////////////////////////////////////////////////////////////////

UniqueStubs emitUniqueStubs() {
  UniqueStubs us;
  auto functions = {
    emitCallToExit,
    emitReturnHelpers,
    emitResumeHelpers,
    emitStackOverflowHelper,
    emitDefClsHelper,
    emitFreeLocalsHelpers,
    emitFuncPrologueRedispatch,
    emitFCallArrayHelper,
    emitFCallHelperThunk,
    emitFuncBodyHelperThunk,
    emitFunctionEnterHelper,
  };
  for (auto& f : functions) f(us);
  return us;
}

//////////////////////////////////////////////////////////////////////

}}}

