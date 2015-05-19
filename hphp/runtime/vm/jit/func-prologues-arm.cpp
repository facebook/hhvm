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
#include "hphp/runtime/vm/jit/func-prologues-arm.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/back-end.h"
#include "hphp/runtime/vm/jit/service-requests-arm.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP { namespace jit { namespace arm {


//////////////////////////////////////////////////////////////////////

namespace {

SrcKey emitPrologueWork(Func* func, int nPassed) {
  not_implemented();
}

//////////////////////////////////////////////////////////////////////
// ARM-only prologue runtime helpers

const StaticString s_call("__call");
const StaticString s_callStatic("__callStatic");

//////////////////////////////////////////////////////////////////////

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) {
  auto& mainCode = mcg->code.main();
  auto& frozenCode = mcg->code.frozen();
  vixl::MacroAssembler a { mainCode };
  vixl::MacroAssembler afrozen { frozenCode };
  TCA start = mainCode.frontier();
  a.   Ldr   (rAsm.W(), rVmFp[AROFF(m_numArgsAndFlags)]);
  for (auto i = 0; i < dvs.size(); ++i) {
    a. Cmp   (rAsm.W(), dvs[i].first);
    emitBindJ(mainCode, frozenCode, CC_LE, SrcKey(func, dvs[i].second, false));
  }
  emitBindJ(mainCode, frozenCode, CC_None, SrcKey(func, func->base(), false));
  mcg->cgFixups().process(nullptr);
  return start;
}

SrcKey emitFuncPrologue(TransID transID, Func* func, int argc, TCA& start) {
  vixl::MacroAssembler a { mcg->code.main() };
  vixl::Label veryStart;
  a.bind(&veryStart);

  a.    Brk   (0);

  SrcKey skFuncBody = emitPrologueWork(func, argc);

  return skFuncBody;
}

}}}
