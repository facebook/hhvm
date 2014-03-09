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

#include "hphp/util/data-block.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

namespace HPHP { namespace JIT { namespace ARM {

namespace {

using namespace vixl;

void emitCallToExit(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };

  a.   Nop   ();
  us.callToExit = a.frontier();
  emitServiceReq(
    mcg->code.main(),
    SRFlags::Align | SRFlags::JmpInsteadOfRet,
    REQ_EXIT
  );

  us.add("callToExit", us.callToExit);
}

void emitReturnHelpers(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };

  us.retHelper = a.frontier();
  a.   Brk   (0);

  us.retInlHelper = a.frontier();
  a.   Brk   (0);

  us.genRetHelper = a.frontier();
  a.   Brk   (0);

  us.add("retHelper", us.retHelper);
  us.add("genRetHelper", us.genRetHelper);
  us.add("retInlHelper", us.retInlHelper);
}

void emitResumeHelpers(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };

  auto const fpOff = offsetof(ExecutionContext, m_fp);
  auto const spOff = offsetof(ExecutionContext, m_stack) +
                       Stack::topOfStackOffset();

  us.resumeHelperRet = a.frontier();
  a.   Str   (vixl::x30, rStashedAR[AROFF(m_savedRip)]);
  us.resumeHelper = a.frontier();
  a.   Ldr   (rVmFp, rGContextReg[fpOff]);
  a.   Ldr   (rVmSp, rGContextReg[spOff]);

  emitServiceReq(mcg->code.main(), REQ_RESUME);

  us.add("resumeHelper", us.resumeHelper);
  us.add("resumeHelperRet", us.resumeHelperRet);
}

void emitStackOverflowHelper(UniqueStubs& us) {
  MacroAssembler a { mcg->code.stubs() };

  us.stackOverflowHelper = a.frontier();
  a.  Ldr  (rAsm, rVmFp[AROFF(m_func)]);
  a.  Ldr  (rAsm2.W(), rStashedAR[AROFF(m_soff)]);
  a.  Ldr  (rAsm, rAsm[Func::sharedOffset()]);
  a.  Ldr  (rAsm.W(), rAsm[Func::sharedBaseOffset()]);
  // The VM-reg-save helper will read the current BC offset out of argReg(0).
  a.  Add  (argReg(0).W(), rAsm.W(), rAsm2.W());

  emitEagerVMRegSave(a, RegSaveFlags::SaveFP | RegSaveFlags::SavePC);
  emitServiceReq(mcg->code.stubs(), REQ_STACK_OVERFLOW);

  us.add("stackOverflowHelper", us.stackOverflowHelper);
}


void emitDefClsHelper(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };

  us.defClsHelper = a.frontier();
  a.   Brk   (0);

  us.add("defClsHelper", us.defClsHelper);
}

void emitFreeLocalsHelpers(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };

  us.freeManyLocalsHelper = a.frontier();
  a.   Brk   (0);

  us.add("freeManyLocalsHelper", us.freeManyLocalsHelper);
}

void emitFuncPrologueRedispatch(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };
  vixl::Label actualDispatch;
  vixl::Label numParamsCheck;

  us.funcPrologueRedispatch = a.frontier();

  // Using x0, x1 and x2 as scratch registers here. This is happening between
  // compilation units -- trying to get into a func prologue, to be precise --
  // so there are no live registers.

  a.  Ldr  (x0, rStashedAR[AROFF(m_func)]);
  a.  Ldr  (w1, rStashedAR[AROFF(m_numArgsAndGenCtorFlags)]);
  a.  And  (w1, w1, 0x7fffffff);
  a.  Ldr  (w2, x0[Func::numParamsOff()]);

  // If we passed more args than declared, jump to the numParamsCheck.
  a.  Cmp  (w2, w1);
  a.  B    (&numParamsCheck, lt);

  a.  bind (&actualDispatch);
  // Need to load x0[w1 * 8 + Func::prologueTableOff()]. On x64 there's an
  // addressing mode for this. Here, there's only base+imm and base+reg. So we
  // add the immediate to the base first, then use base+reg with scaling.
  a.  Add  (x0, x0, Func::prologueTableOff());
  // What this is saying: base is x0, index is w1 (with Unsigned eXTension to 64
  // bits), scaled by 2^3.
  a.  Ldr  (x0, MemOperand(x0, w1, UXTW, 3));
  a.  Br   (x0);
  a.  Brk  (0);

  a.  bind (&numParamsCheck);
  a.  Cmp  (w1, kNumFixedPrologues);
  a.  B    (&actualDispatch, lt);

  // Too many parameters.
  a.  Add  (x0, x0, Func::prologueTableOff() + sizeof(TCA));
  a.  Ldr  (x0, MemOperand(x0, w2, UXTW, 3));
  a.  Br   (x0);
  a.  Brk  (0);

  us.add("funcPrologueRedispatch", us.funcPrologueRedispatch);
}

void emitFCallArrayHelper(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };

  us.fcallArrayHelper = a.frontier();
  a.   Brk   (0);

  us.add("fcallArrayHelper", us.fcallArrayHelper);
}

void emitFCallHelperThunk(UniqueStubs& us) {
  TCA (*helper)(ActRec*, void*) = &fcallHelper;
  MacroAssembler a { mcg->code.main() };

  us.fcallHelperThunk = a.frontier();
  vixl::Label popAndXchg, jmpRet;

  a.   Mov   (argReg(0), rStashedAR);
  a.   Mov   (argReg(1), rVmSp);
  a.   Cmp   (rVmFp, rStashedAR);
  a.   B     (&popAndXchg, vixl::ne);
  emitCall(a, CppCall(helper));
  a.   Br    (rReturnReg);

  a.   bind  (&popAndXchg);
  emitXorSwap(a, rStashedAR, rVmFp);
  // Put return address into ActRec.
  a.   Str   (rLinkReg, rVmFp[AROFF(m_savedRip)]);
  emitCall(a, CppCall(helper));
  // Put return address back in the link register.
  a.   Ldr   (rLinkReg, rVmFp[AROFF(m_savedRip)]);
  emitXorSwap(a, rStashedAR, rVmFp);
  a.   Cmp   (rReturnReg, 0);
  a.   B     (&jmpRet, vixl::gt);
  a.   Neg   (rReturnReg, rReturnReg);
  a.   Ldr   (rVmFp, rGContextReg[offsetof(ExecutionContext, m_fp)]);
  a.   Ldr   (rVmSp, rGContextReg[offsetof(ExecutionContext, m_stack) +
                                  Stack::topOfStackOffset()]);

  a.   bind  (&jmpRet);

  a.   Br    (rReturnReg);

  us.add("fcallHelperThunk", us.fcallHelperThunk);
}

void emitFuncBodyHelperThunk(UniqueStubs& us) {
  TCA (*helper)(ActRec*, void*) = &funcBodyHelper;
  MacroAssembler a { mcg->code.main() };

  us.funcBodyHelperThunk = a.frontier();
  a.   Mov   (argReg(0), rVmFp);
  a.   Mov   (argReg(1), rVmSp);
  a.   Mov   (rHostCallReg, reinterpret_cast<intptr_t>(helper));
  a.   Push  (rLinkReg, rVmFp);
  a.   HostCall(2);
  a.   Pop   (rVmFp, rLinkReg);
  a.   Br    (rReturnReg);

  us.add("funcBodyHelperThunk", us.funcBodyHelperThunk);
}

void emitFunctionEnterHelper(UniqueStubs& us) {
  bool (*helper)(const ActRec*, int) = &EventHook::onFunctionEnter;
  MacroAssembler a { mcg->code.main() };

  us.functionEnterHelper = a.frontier();

  vixl::Label skip;

  auto ar = argReg(0);

  a.   Push    (rLinkReg, rVmFp);
  a.   Mov     (rVmFp, vixl::sp);
  // rAsm2 gets the savedRbp, rAsm gets the savedRip.
  a.   Ldp     (rAsm2, rAsm, ar[AROFF(m_savedRbp)]);
  static_assert(AROFF(m_savedRbp) + 8 == AROFF(m_savedRip),
                "m_savedRbp must precede m_savedRip");
  a.   Push    (rAsm, rAsm2);
  a.   Mov     (argReg(1), EventHook::NormalFunc);
  a.   Mov     (rHostCallReg, reinterpret_cast<intptr_t>(helper));
  a.   HostCall(2);
  a.   Cbz     (rReturnReg, &skip);
  a.   Mov     (vixl::sp, rVmFp);
  a.   Pop     (rVmFp, rLinkReg);
  a.   Ret     (rLinkReg);

  a.   bind    (&skip);

  // Tricky. The last two things we pushed were the saved fp and return TCA from
  // the function we were supposed to enter. Since we're now "returning" from
  // that function, restore that fp and jump to that return TCA. Below that on
  // the stack are that function's *caller's* saved fp and return TCA. We can
  // ignore the saved fp, but we have to restore the return TCA into x30.
  auto rIgnored = rAsm2;
  a.   Pop     (rVmFp, rAsm);
  a.   Pop     (rIgnored, rLinkReg);
  a.   Ldr     (rVmSp, rGContextReg[offsetof(ExecutionContext, m_stack) +
                                    Stack::topOfStackOffset()]);
  a.   Br      (rAsm);

  us.add("functionEnterHelper", us.functionEnterHelper);
}

} // anonymous namespace

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

}}}
