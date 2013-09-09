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

using namespace Transl; // XXX remove
using namespace Transl::reg;
using boost::implicit_cast;

TRACE_SET_MOD(ustubs);

//////////////////////////////////////////////////////////////////////

namespace {

// Utility for logging stubs addresses during startup and registering
// the gdb symbols.  It's often useful to know where they were when
// debugging.
TCA add(const char* name, TCA start) {
  auto const inAStubs = start > tx64->stubsCode.base();
  UNUSED auto const stop = inAStubs ? tx64->stubsCode.frontier()
                                    : tx64->mainCode.frontier();

  FTRACE(1, "unique stub: {} {} -- {:4} bytes: {}\n",
         inAStubs ? "astubs @ "
                  : "  ahot @  ",
         static_cast<void*>(start),
         static_cast<size_t>(stop - start),
         name);

  ONTRACE(2,
    [&]{
      Disasm dasm(Disasm::Options().indent(4));
      std::ostringstream os;
      dasm.disasm(os, start, stop);
      FTRACE(2, "{}\n", os.str());
    }()
  );

  tx64->recordGdbStub(
    tx64->codeBlockFor(start),
    start,
    strdup(folly::format("HHVM::{}", name).str().c_str())
  );
  return start;
}

TCA emitRetFromInterpretedFrame() {
  Asm a { tx64->stubsCode };
  moveToAlign(a);
  auto const ret = a.frontier();

  auto const arBase = static_cast<int32_t>(sizeof(ActRec) - sizeof(Cell));
  a.   lea  (rVmSp[-arBase], serviceReqArgRegs[0]);
  a.   movq (rVmFp, serviceReqArgRegs[1]);
  emitServiceReq(a, SRFlags::JmpInsteadOfRet, REQ_POST_INTERP_RET);
  return ret;
}

TCA emitRetFromInterpretedGeneratorFrame() {
  Asm a { tx64->stubsCode };
  moveToAlign(a);
  auto const ret = a.frontier();

  // We have to get the Continuation object from the current AR's $this, then
  // find where its embedded AR is.
  PhysReg rContAR = serviceReqArgRegs[0];
  a.    loadq  (rVmFp[AROFF(m_this)], rContAR);
  a.    loadq  (rContAR[CONTOFF(m_arPtr)], rContAR);
  a.    movq   (rVmFp, serviceReqArgRegs[1]);
  emitServiceReq(a, SRFlags::JmpInsteadOfRet, REQ_POST_INTERP_RET);
  return ret;
}

template<int Arity>
TCA emitNAryStub(Asm& a, CppCall c) {
  static_assert(Arity < kNumRegisterArgs, "");

  // The callNAryStub has already saved these regs on a.
  RegSet alreadySaved;
  for (size_t i = 0; i < Arity; ++i) {
    alreadySaved |= RegSet(argNumToRegName[i]);
  }

  /*
   * We've made a call instruction, and pushed Arity args on the
   * stack.  So the stack address will be odd coming into the stub if
   * Arity + 1 (for the call) is odd.  We need to correct for this
   * when saving other registers below to keep SSE-friendly alignment
   * of the stack.
   */
  const int Parity = (Arity + 1) % 2;

  // These dtor stubs are meant to be called with the call
  // instruction, unlike most translator code.
  moveToAlign(a);
  TCA start = a.frontier();
  /*
   * Preserve most caller-saved regs. The calling code has already
   * preserved regs in `alreadySaved'; we push the rest of the caller
   * saved regs and rbp.  It should take 9 qwords in total, and the
   * incoming call instruction made it 10.  This is an even number of
   * pushes, so we preserve the SSE-friendliness of our execution
   * environment (without real intervention from PhysRegSaverParity).
   *
   * Note that we don't need to clean all registers because the only
   * reason we could need those locations written back is if stack
   * unwinding were to happen.  These stubs can re-enter due to user
   * destructors, but exceptions are not allowed to propagate out of
   * those, so it's not a problem.
   */
  a.    push (rbp); // {
  a.    movq (rsp, rbp);
  {
    RegSet s = kGPCallerSaved - alreadySaved;
    PhysRegSaverParity rs(Parity, a, s);
    emitCall(a, c);
  }
  a.    pop  (rbp);  // }
  a.    ret  ();
  return start;
}

TCA emitUnaryStub(Asm& a, CppCall c) {
  return emitNAryStub<1>(a, c);
}

//////////////////////////////////////////////////////////////////////

void emitCallToExit(UniqueStubs& uniqueStubs) {
  Asm a { tx64->stubsCode };

  // Emit a byte of padding. This is a kind of hacky way to avoid
  // hitting an assert in recordGdbStub when we call it with stub - 1
  // as the start address.
  a.emitNop(1);
  auto const stub = emitServiceReq(
    a,
    SRFlags::Align | SRFlags::JmpInsteadOfRet,
    REQ_EXIT
  );

  // On a backtrace, gdb tries to locate the calling frame at address
  // returnRIP-1. However, for the first VM frame, there is no code at
  // returnRIP-1, since the AR was set up manually. For this frame,
  // record the tracelet address as starting from this callToExit-1,
  // so gdb does not barf.
  uniqueStubs.callToExit = add("callToExit", stub);
}

void emitReturnHelpers(UniqueStubs& us) {
  us.retHelper    = add("retHelper", emitRetFromInterpretedFrame());
  us.genRetHelper = add("genRetHelper",
                        emitRetFromInterpretedGeneratorFrame());
  us.retInlHelper = add("retInlHelper", emitRetFromInterpretedFrame());
}

void emitResumeHelpers(UniqueStubs& uniqueStubs) {
  Asm a { tx64->stubsCode };
  moveToAlign(a);

  auto const fpOff = offsetof(VMExecutionContext, m_fp);
  auto const spOff = offsetof(VMExecutionContext, m_stack) +
                       Stack::topOfStackOffset();

  uniqueStubs.resumeHelperRet = a.frontier();
  a.    pop   (rStashedAR[AROFF(m_savedRip)]);
  uniqueStubs.resumeHelper = a.frontier();
  emitGetGContext(a, rax);
  a.   loadq  (rax[fpOff], rVmFp);
  a.   loadq  (rax[spOff], rVmSp);
  emitServiceReq(a, REQ_RESUME);

  add("resumeHelpers", uniqueStubs.resumeHelper);
}

void emitDefClsHelper(UniqueStubs& uniqueStubs) {
  Asm a { tx64->stubsCode };
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

  add("defClsHelper", uniqueStubs.defClsHelper);
}

void emitStackOverflowHelper(UniqueStubs& uniqueStubs) {
  Asm a { tx64->stubsCode };

  moveToAlign(a);
  uniqueStubs.stackOverflowHelper = a.frontier();

  // We are called from emitStackCheck, with the new stack frame in
  // rStashedAR. Get the caller's PC into rdi and save it off.
  a.    loadq  (rVmFp[AROFF(m_func)], rax);
  a.    loadl  (rStashedAR[AROFF(m_soff)], edi);
  a.    loadq  (rax[Func::sharedOffset()], rax);
  a.    loadl  (rax[Func::sharedBaseOffset()], eax);
  a.    addl   (eax, edi);
  emitEagerVMRegSave(a, RegSaveFlags::SaveFP | RegSaveFlags::SavePC);
  emitServiceReq(a, REQ_STACK_OVERFLOW);

  add("stackOverflowHelper", uniqueStubs.stackOverflowHelper);
}

void emitDtorStubs(UniqueStubs& uniqueStubs) {
  Asm a { tx64->stubsCode };

  auto const strDtor = add("dtorStr", emitUnaryStub(
    a, CppCall(getMethodPtr(&StringData::release))
  ));
  auto const arrDtor = add("dtorArr", emitUnaryStub(
    a, CppCall(getVTableOffset(&HphpArray::release))
  ));
  auto const objDtor = add("dtorObj", emitUnaryStub(
    a, CppCall(getMethodPtr(&ObjectData::release))
  ));
  auto const resDtor = add("dtorRes", emitUnaryStub(
    a, CppCall(getMethodPtr(&ResourceData::release))
  ));
  auto const refDtor = add("dtorRef", emitUnaryStub(
    a, CppCall(implicit_cast<void*>(getMethodPtr(&RefData::release)))
  ));

  uniqueStubs.dtorStubs[0] = nullptr;
  uniqueStubs.dtorStubs[typeToDestrIndex(BitwiseKindOfString)] = strDtor;
  uniqueStubs.dtorStubs[typeToDestrIndex(KindOfArray)]         = arrDtor;
  uniqueStubs.dtorStubs[typeToDestrIndex(KindOfObject)]        = objDtor;
  uniqueStubs.dtorStubs[typeToDestrIndex(KindOfResource)]      = resDtor;
  uniqueStubs.dtorStubs[typeToDestrIndex(KindOfRef)]           = refDtor;
}

void emitGenericDecRefHelpers(UniqueStubs& uniqueStubs) {
  Label release;

  Asm a { tx64->mainCode };
  moveToAlign(a, kNonFallthroughAlign);

  // dtorGenericStub just takes a pointer to the TypedValue in rdi.
  uniqueStubs.irPopRHelper = a.frontier();
  // popR: Move top-of-stack pointer to rdi
  emitMovRegReg(a, rVmSp, rdi);
  // fall through
  uniqueStubs.dtorGenericStub = a.frontier();
  emitLoadTVType(a, rdi[TVOFF(m_type)], r32(rAsm));
  a.    loadq  (rdi[TVOFF(m_data)], rdi);
  // Fall through to the regs stub.

  /*
   * Custom calling convention: m_type goes in rAsm, m_data in
   * rdi.  We don't ever store program locations in rAsm, so the
   * caller didn't need to spill anything.  The assembler sometimes
   * uses rAsm, but we know the stub won't need to and it makes it
   * possible to share the code for both decref helpers.
   */
  uniqueStubs.dtorGenericStubRegs = a.frontier();
  a.    cmpl   (RefCountStaticValue, rdi[FAST_REFCOUNT_OFFSET]);
  jccBlock<CC_Z>(a, [&] {
    a.  decl   (rdi[FAST_REFCOUNT_OFFSET]);
    release.jcc8(a, CC_Z);
  });
  a.    ret    ();

asm_label(a, release);
  {
    PhysRegSaver prs(a, kGPCallerSaved - RegSet(rdi));
    callDestructor(a, rAsm, rax);
    tx64->fixupMap().recordIndirectFixup(
      a.frontier(),
      prs.rspTotalAdjustmentRegs()
    );
  }
  a.    ret    ();

  add("genericDtors", uniqueStubs.irPopRHelper);
}

void emitFreeLocalsHelpers(UniqueStubs& uniqueStubs) {
  Label doRelease;
  Label release;
  Label loopHead;

  /*
   * Note: the IR currently requires that we preserve r13/r14 across
   * calls to these free locals helpers.
   */
  static_assert(rVmSp == rbx, "");
  auto const rIter     = rbx;
  auto const rFinished = r15;
  auto const rType     = esi;
  auto const rData     = rdi;

  Asm a { tx64->mainCode };
  moveToAlign(a, kNonFallthroughAlign);

asm_label(a, release);
  a.    loadq  (rIter[TVOFF(m_data)], rData);
  a.    cmpl   (RefCountStaticValue, rData[FAST_REFCOUNT_OFFSET]);
  jccBlock<CC_Z>(a, [&] {
    a.  decl   (rData[FAST_REFCOUNT_OFFSET]);
    a.  jz8    (doRelease);
  });
  a.    ret    ();
asm_label(a, doRelease);
  jumpDestructor(a, PhysReg(rType), rax);

  moveToAlign(a, kJmpTargetAlign);
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
  a.    ret    (8);

  add("freeLocalsHelpers", uniqueStubs.freeManyLocalsHelper);
}

void emitFuncPrologueRedispatch(UniqueStubs& uniqueStubs) {
  Asm a { tx64->mainCode };

  moveToAlign(a);
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

  add("funcPrologueRedispatch", uniqueStubs.funcPrologueRedispatch);
}

void emitFCallArrayHelper(UniqueStubs& uniqueStubs) {
  Asm a { tx64->mainCode };

  moveToAlign(a, kNonFallthroughAlign);
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

  add("fcallArrayHelper", uniqueStubs.fcallArrayHelper);
}

//////////////////////////////////////////////////////////////////////

void emitFCallHelperThunk(UniqueStubs& uniqueStubs) {
  TCA (*helper)(ActRec*) = &fcallHelper;
  Asm a { tx64->stubsCode };

  moveToAlign(a);
  uniqueStubs.fcallHelperThunk = a.frontier();

  Label popAndXchg;

  // fcallHelper is used for prologues, and (in the case of
  // closures) for dispatch to the function body. In the first
  // case, there's a call, in the second, there's a jmp.
  // We can differentiate by comparing r15 and rVmFp
  a.    movq   (rStashedAR, argNumToRegName[0]);
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
  a.    xchgq  (rStashedAR, rVmFp);
  a.    pop    (rVmFp[AROFF(m_savedRip)]);
  emitCall(a, CppCall(helper));
  a.    push   (rVmFp[AROFF(m_savedRip)]);
  a.    xchgq  (rStashedAR, rVmFp);
  a.    jmp    (rax);
  a.    ud2    ();

  add("fcallHelperThunk", uniqueStubs.fcallHelperThunk);
}

void emitFuncBodyHelperThunk(UniqueStubs& uniqueStubs) {
  TCA (*helper)(ActRec*) = &funcBodyHelper;
  Asm a { tx64->stubsCode };

  moveToAlign(a);
  uniqueStubs.funcBodyHelperThunk = a.frontier();

  // This helper is called via a direct jump from the TC (from
  // fcallArrayHelper). So the stack parity is already correct.
  a.    movq   (rVmFp, argNumToRegName[0]);
  emitCall(a, CppCall(helper));
  a.    jmp    (rax);
  a.    ud2    ();

  add("funcBodyHelperThunk", uniqueStubs.funcBodyHelperThunk);
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
    emitDtorStubs,
    emitGenericDecRefHelpers,
    emitFreeLocalsHelpers,
    emitFuncPrologueRedispatch,
    emitFCallArrayHelper,
    emitFCallHelperThunk,
    emitFuncBodyHelperThunk,
  };
  for (auto& f : functions) f(us);
  return us;
}

//////////////////////////////////////////////////////////////////////

}}}

