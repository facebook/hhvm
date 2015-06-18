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
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

namespace HPHP { namespace jit { namespace arm {

namespace {

using namespace vixl;

void emitCallToExit(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };

  a.   Nop   ();
  us.callToExit = a.frontier();
  a.   Br    (rLinkReg);
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

  us.asyncGenRetHelper = a.frontier();
  a.   Brk   (0);

  us.add("retHelper", us.retHelper);
  us.add("genRetHelper", us.genRetHelper);
  us.add("asyncGenRetHelper", us.asyncGenRetHelper);
  us.add("retInlHelper", us.retInlHelper);
}

void emitResumeHelpers(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };

  us.resumeHelperRet = a.frontier();
  a.   Brk   (0);

  not_implemented();

  us.add("resumeHelper", us.resumeHelper);
  us.add("resumeHelperRet", us.resumeHelperRet);
}

void emitStackOverflowHelper(UniqueStubs& us) {
  MacroAssembler a { mcg->code.cold() };

  us.stackOverflowHelper = a.frontier();
  a.   Brk   (0);

  not_implemented();

  us.add("stackOverflowHelper", us.stackOverflowHelper);
}

void emitFreeLocalsHelpers(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };

  us.freeManyLocalsHelper = a.frontier();
  a.   Brk   (0);

  us.add("freeManyLocalsHelper", us.freeManyLocalsHelper);
}

void emitFuncPrologueRedispatch(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };

  a.  Brk  (0);
  not_implemented();

  us.add("funcPrologueRedispatch", us.funcPrologueRedispatch);
}

void emitFCallArrayHelper(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };

  us.fcallArrayHelper = a.frontier();
  a.   Brk   (0);

  us.add("fcallArrayHelper", us.fcallArrayHelper);
}

void emitFCallHelperThunk(UniqueStubs& us) {
  MacroAssembler a { mcg->code.main() };
  us.fcallHelperThunk = a.frontier();
  a.   Brk   (0);
  us.add("fcallHelperThunk", us.fcallHelperThunk);
}

void emitFuncBodyHelperThunk(UniqueStubs& us) {
  TCA (*helper)(ActRec*) = &funcBodyHelper;
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
  bool (*helper)(const ActRec*, int) = &EventHook::onFunctionCall;
  MacroAssembler a { mcg->code.main() };

  us.functionEnterHelper = a.frontier();

  vixl::Label skip;

  auto ar = argReg(0);

  a.   Push    (rLinkReg, rVmFp);
  a.   Mov     (rVmFp, vixl::sp);
  // rAsm2 gets the savedRbp, rAsm gets the savedRip.
  a.   Ldp     (rAsm2, rAsm, ar[AROFF(m_sfp)]);
  static_assert(AROFF(m_sfp) + 8 == AROFF(m_savedRip),
                "m_sfp must precede m_savedRip");
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
  a.   Ldr     (rVmSp, rVmTl[rds::kVmspOff]);
  a.   Br      (rAsm);

  us.add("functionEnterHelper", us.functionEnterHelper);
}

void emitBindCallStubs(UniqueStubs& uniqueStubs) {
  for (int i = 0; i < 2; i++) {
    auto& cb = mcg->code.cold();
    if (!i) {
      uniqueStubs.bindCallStub = cb.frontier();
    } else {
      uniqueStubs.immutableBindCallStub = cb.frontier();
    }
    not_implemented();
  }
  uniqueStubs.add("bindCallStub", uniqueStubs.bindCallStub);
  uniqueStubs.add("immutableBindCallStub", uniqueStubs.immutableBindCallStub);
}

} // anonymous namespace

UniqueStubs emitUniqueStubs() {
  UniqueStubs us;
  auto functions = {
    emitCallToExit,
    emitReturnHelpers,
    emitResumeHelpers,
    emitStackOverflowHelper,
    emitFreeLocalsHelpers,
    emitFuncPrologueRedispatch,
    emitFCallArrayHelper,
    emitFCallHelperThunk,
    emitFuncBodyHelperThunk,
    emitFunctionEnterHelper,
    emitBindCallStubs,
  };
  for (auto& f : functions) f(us);
  return us;
}

}}}
