/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/irgen-func-prologue.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"
#include "hphp/util/growable-vector.h"
#include "hphp/util/immed.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

TransContext prologue_context(TransID transID,
                              TransKind kind,
                              const Func* func,
                              Offset entry) {
  return TransContext(
    transID,
    kind,
    TransFlags{},
    SrcKey{func, entry, SrcKey::PrologueTag{}},
    FPInvOffset{func->numSlotsInFrame()},
    0
  );
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

TCA genFuncPrologue(TransID transID, TransKind kind, Func* func, int argc,
                    CodeCache::View code, CGMeta& fixups) {
  auto context = prologue_context(transID, kind, func,
                                  func->getEntryForNumArgs(argc));
  IRUnit unit{context};
  irgen::IRGS env{unit, nullptr};

  irgen::emitFuncPrologue(env, argc, transID);
  irgen::sealUnit(env);

  printUnit(2, unit, "After initial prologue generation");

  auto vunit = irlower::lowerUnit(env.unit, CodeKind::Prologue);
  emitVunit(*vunit, env.unit, code, fixups);

  // In order to find the start of the (post guard) prologue after
  // possibly relocating the code, we add a watchpoint that points to
  // &unit.prologueStart. In some situations (eg tc-relocate) we will
  // relocate the code again - but at that point, unit has gone (and
  // tc-relocate tracks the start of the prologue for itself). So we
  // need to remove it here, to prevent wild writes to dead stack
  // locations.
  auto it = std::find_if(fixups.watchpoints.begin(), fixups.watchpoints.end(),
                         [&] (TCA* p) { return p == &unit.prologueStart; });
  assertx(it != fixups.watchpoints.end());
  fixups.watchpoints.erase(it);

  return unit.prologueStart;
}

TCA genFuncBodyDispatch(Func* func, const DVFuncletsVec& dvs,
                        TransKind kind, CodeCache::View code) {
  auto context = prologue_context(kInvalidTransID, kind, func, func->base());
  IRUnit unit{context};
  irgen::IRGS env{unit, nullptr};

  irgen::emitFuncBodyDispatch(env, dvs);
  irgen::sealUnit(env);

  CGMeta fixups;
  auto vunit = irlower::lowerUnit(env.unit, CodeKind::Prologue);

  auto& main = code.main();
  auto const start = main.frontier();

  emitVunit(*vunit, env.unit, code, fixups);

  if (RuntimeOption::EvalPerfRelocate) {
    GrowableVector<IncomingBranch> ibs;
    auto& frozen = code.frozen();
    tc::recordPerfRelocMap(start, main.frontier(),
                           frozen.frontier(), frozen.frontier(),
                           context.srcKey(), 0, ibs, fixups);
  }
  fixups.process(nullptr);

  return start;
}

///////////////////////////////////////////////////////////////////////////////

}}
