/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/func-prologue.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-func-prologue.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/code-cache.h"
#include "hphp/util/data-block.h"
#include "hphp/util/growable-vector.h"
#include "hphp/util/immed.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

TransContext prologue_context(TransID transID,
                              const Func* func,
                              Offset entry) {
  return TransContext(
    transID,
    SrcKey{func, entry, SrcKey::PrologueTag{}},
    FPInvOffset{func->numSlotsInFrame()}
  );
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

TCA genFuncPrologue(TransID transID, Func* func, int argc) {
  auto context = prologue_context(transID, func,
                                  func->getEntryForNumArgs(argc));
  IRGS env{context, TransFlags{}};

  auto& cb = mcg->code.main();

  // Dump the func guard in the TC before anything else.
  emitFuncGuard(func, cb);
  auto const start = cb.frontier();

  irgen::emitFuncPrologue(env, argc, transID);
  irgen::sealUnit(env);

  irlower::genCode(env.unit, CodeKind::CrossTrace);

  return start;
}

TCA genFuncBodyDispatch(Func* func, const DVFuncletsVec& dvs) {
  auto context = prologue_context(kInvalidTransID, func, func->base());
  IRGS env{context, TransFlags{}};

  assertx(mcg->cgFixups().empty());

  auto& main = mcg->code.main();
  auto& frozen = mcg->code.frozen();

  auto const start = main.frontier();

  irgen::emitFuncBodyDispatch(env, dvs);
  irgen::sealUnit(env);

  irlower::genCode(env.unit, CodeKind::CrossTrace);

  if (RuntimeOption::EvalPerfRelocate) {
    GrowableVector<IncomingBranch> ibs;
    recordPerfRelocMap(start, main.frontier(),
                       frozen.frontier(), frozen.frontier(),
                       SrcKey { func, dvs[0].second, false },
                       0, ibs, mcg->cgFixups());
  }
  mcg->cgFixups().process(nullptr);

  return start;
}

///////////////////////////////////////////////////////////////////////////////

}}
