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
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/translator-x64.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"

namespace HPHP { namespace JIT { namespace ARM {

namespace {

using namespace vixl;

void emitCallToExit(UniqueStubs& us) {
  MacroAssembler a { tx64->stubsCode };

  a.   Nop   ();
  us.callToExit = a.frontier();
  a.   Brk   (0);

  us.add("callToExit", us.callToExit);
}

void emitReturnHelpers(UniqueStubs& us) {
  MacroAssembler a { tx64->stubsCode };

  us.retHelper = us.genRetHelper = us.retInlHelper = a.frontier();
  a.   Brk   (0);

  us.add("retHelper", us.retHelper);
  us.add("genRetHelper", us.genRetHelper);
  us.add("retInlHelper", us.retInlHelper);
}

void emitResumeHelpers(UniqueStubs& us) {
  MacroAssembler a { tx64->stubsCode };

  auto const fpOff = offsetof(VMExecutionContext, m_fp);
  auto const spOff = offsetof(VMExecutionContext, m_stack) +
                       Stack::topOfStackOffset();

  us.resumeHelperRet = a.frontier();
  a.   Ldr   (rAsm, MemOperand(vixl::sp, 8, PostIndex));
  a.   Str   (rAsm, rStashedAR[AROFF(m_savedRip)]);
  us.resumeHelper = a.frontier();
  a.   Ldr   (rVmFp, rGContextReg[fpOff]);
  a.   Ldr   (rVmSp, rGContextReg[spOff]);

  emitServiceReq(tx64->stubsCode, REQ_RESUME);

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
  MacroAssembler a { tx64->stubsCode };

  us.defClsHelper = a.frontier();
  a.   Brk   (0);

  us.add("defClsHelper", us.defClsHelper);
}

void emitDtorStubs(UniqueStubs& us) {
}

void emitGenericDecRefHelpers(UniqueStubs& us) {
  MacroAssembler a { tx64->mainCode };

  us.irPopRHelper = a.frontier();
  a.   Brk   (0);

  us.add("genericDtors", us.irPopRHelper);
}

void emitFreeLocalsHelpers(UniqueStubs& us) {
  MacroAssembler a { tx64->mainCode };

  us.freeManyLocalsHelper = a.frontier();
  a.   Brk   (0);

  us.add("freeManyLocalsHelper", us.freeManyLocalsHelper);
}

void emitFuncPrologueRedispatch(UniqueStubs& us) {
  MacroAssembler a { tx64->stubsCode };

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
  TCA (*helper)(ActRec*) = &fcallHelper;
  MacroAssembler a { tx64->stubsCode };

  us.fcallHelperThunk = a.frontier();
  vixl::Label popAndXchg;

  a.   Mov   (argReg(0), rStashedAR);
  a.   Cmp   (rVmFp, rStashedAR);
  a.   B     (&popAndXchg, vixl::ne);
  emitCall(a, CppCall(helper));
  a.   Br    (rReturnReg);

  a.   bind  (&popAndXchg);
  emitXorSwap(a, rStashedAR, rVmFp);
  // Pop return address into ActRec.
  a.   Ldr   (rAsm, vixl::sp[0]);
  a.   Str   (rAsm, rVmFp[AROFF(m_savedRip)]);
  emitCall(a, CppCall(helper));
  // Put return address back on stack.
  a.   Ldr   (rAsm, rVmFp[AROFF(m_savedRip)]);
  a.   Str   (rAsm, vixl::sp[0]);
  emitXorSwap(a, rStashedAR, rVmFp);
  a.   Br    (rReturnReg);

  us.add("fcallHelperThunk", us.fcallHelperThunk);
}

void emitFuncBodyHelperThunk(UniqueStubs& us) {
  TCA (*helper)(ActRec*) = &funcBodyHelper;
  MacroAssembler a { tx64->stubsCode };

  us.funcBodyHelperThunk = a.frontier();
  a.   Mov   (argReg(0), rVmFp);
  a.   Mov   (rHostCallReg, reinterpret_cast<intptr_t>(helper));
  a.   Push  (rLinkReg, rVmFp);
  a.   HostCall(1);
  a.   Pop   (rVmFp, rLinkReg);
  a.   Br    (rReturnReg);

  us.add("funcBodyHelperThunk", us.funcBodyHelperThunk);
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

}}}
