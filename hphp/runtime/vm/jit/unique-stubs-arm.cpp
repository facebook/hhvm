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

#include "hphp/util/data-block.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

namespace HPHP { namespace JIT { namespace ARM {

namespace {

using namespace vixl;

void emitCallToExit(UniqueStubs& us) {
  MacroAssembler a { tx64->mainCode };

  a.   Nop   ();
  us.callToExit = a.frontier();
  a.   Brk   (0);

  us.add("callToExit", us.callToExit);
}

void emitReturnHelpers(UniqueStubs& us) {
  MacroAssembler a { tx64->mainCode };

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
  MacroAssembler a { tx64->mainCode };

  auto const fpOff = offsetof(VMExecutionContext, m_fp);
  auto const spOff = offsetof(VMExecutionContext, m_stack) +
                       Stack::topOfStackOffset();

  us.resumeHelperRet = a.frontier();
  a.   Ldr   (rAsm, MemOperand(vixl::sp, 8, PostIndex));
  a.   Str   (rAsm, rStashedAR[AROFF(m_savedRip)]);
  us.resumeHelper = a.frontier();
  a.   Ldr   (rVmFp, rGContextReg[fpOff]);
  a.   Ldr   (rVmSp, rGContextReg[spOff]);

  emitServiceReq(tx64->mainCode, REQ_RESUME);

  us.add("resumeHelper", us.resumeHelper);
  us.add("resumeHelperRet", us.resumeHelperRet);
}

void emitStackOverflowHelper(UniqueStubs& us) {
  MacroAssembler a { tx64->stubsCode };

  us.stackOverflowHelper = a.frontier();
  a.   Brk   (0);

  us.add("stackOverflowHelper", us.stackOverflowHelper);
}


void emitDefClsHelper(UniqueStubs& us) {
  MacroAssembler a { tx64->mainCode };

  us.defClsHelper = a.frontier();
  a.   Brk   (0);

  us.add("defClsHelper", us.defClsHelper);
}

void emitFreeLocalsHelpers(UniqueStubs& us) {
  MacroAssembler a { tx64->mainCode };

  us.freeManyLocalsHelper = a.frontier();
  a.   Brk   (0);

  us.add("freeManyLocalsHelper", us.freeManyLocalsHelper);
}

void emitFuncPrologueRedispatch(UniqueStubs& us) {
  MacroAssembler a { tx64->mainCode };

  us.funcPrologueRedispatch = a.frontier();
  a.   Brk   (0);

  us.add("funcPrologueRedispatch", us.funcPrologueRedispatch);
}

void emitFCallArrayHelper(UniqueStubs& us) {
  MacroAssembler a { tx64->mainCode };

  us.fcallArrayHelper = a.frontier();
  a.   Brk   (0);

  us.add("fcallArrayHelper", us.fcallArrayHelper);
}

void emitFCallHelperThunk(UniqueStubs& us) {
  TCA (*helper)(ActRec*, void*) = &fcallHelper;
  MacroAssembler a { tx64->mainCode };

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
  a.   Ldr   (rVmFp, rGContextReg[offsetof(VMExecutionContext, m_fp)]);
  a.   Ldr   (rVmSp, rGContextReg[offsetof(VMExecutionContext, m_stack) +
                                  Stack::topOfStackOffset()]);

  a.   bind  (&jmpRet);

  a.   Br    (rReturnReg);

  us.add("fcallHelperThunk", us.fcallHelperThunk);
}

void emitFuncBodyHelperThunk(UniqueStubs& us) {
  TCA (*helper)(ActRec*, void*) = &funcBodyHelper;
  MacroAssembler a { tx64->mainCode };

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
  MacroAssembler a { tx64->mainCode };

  us.functionEnterHelper = a.frontier();

  vixl::Label skip;

  auto ar = argReg(0);

  a.   Push    (rLinkReg, rVmFp);
  a.   Mov     (rVmFp, vixl::sp);
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

  a.   Pop     (rVmFp, rLinkReg, rAsm, rAsm2);
  a.   Ldr     (rVmSp, rGContextReg[offsetof(VMExecutionContext, m_stack) +
                                    Stack::topOfStackOffset()]);
  a.   Br      (rLinkReg);

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
