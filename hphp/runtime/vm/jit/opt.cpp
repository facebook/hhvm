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

#include "hphp/runtime/vm/jit/opt.h"

#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/dce.h"

#include "hphp/util/trace.h"

#include <boost/dynamic_bitset.hpp>

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

enum class DCE { None, Minimal, Full };

template<class PassFN>
void doPass(IRUnit& unit, PassFN fn, DCE dce) {
  fn(unit);
  switch (dce) {
  case DCE::Minimal:  mandatoryDCE(unit); break;
  case DCE::Full:     fullDCE(unit); // fallthrough
  case DCE::None:     assertx(checkEverything(unit)); break;
  }
}

void removeExitPlaceholders(IRUnit& unit) {
  for (auto& block : rpoSortCfg(unit)) {
    if (block->back().is(ExitPlaceholder)) {
      unit.replace(&block->back(), Jmp, block->next());
    }
  }
}

//////////////////////////////////////////////////////////////////////

}

void optimize(IRUnit& unit, IRBuilder& irBuilder, TransKind kind) {
  Timer timer(Timer::optimize);

  assertx(checkEverything(unit));

  auto const hasLoop = RuntimeOption::EvalJitLoops && cfgHasLoop(unit);

  if (shouldHHIRRelaxGuards() && !hasLoop) {
    Timer _t(Timer::optimize_relaxGuards);
    const bool simple = kind == TransKind::Profile;
    RelaxGuardsFlags flags = (RelaxGuardsFlags)
      (RelaxReflow | (simple ? RelaxSimple : RelaxNormal));
    auto changed = relaxGuards(unit, *irBuilder.guards(), flags);
    if (changed) {
      printUnit(6, unit, "after guard relaxation");
      mandatoryDCE(unit);  // relaxGuards can leave unreachable preds.
    }

    if (RuntimeOption::EvalHHIRSimplification) {
      doPass(unit, simplifyPass, DCE::Minimal);
      doPass(unit, cleanCfg, DCE::None);
    }
  }

  fullDCE(unit);
  printUnit(6, unit, " after initial DCE ");
  assertx(checkEverything(unit));

  if (RuntimeOption::EvalHHIRTypeCheckHoisting) {
    doPass(unit, hoistTypeChecks, DCE::Minimal);
  }
  doPass(unit, removeExitPlaceholders, DCE::Minimal);

  if (RuntimeOption::EvalHHIRPredictionOpts) {
    doPass(unit, optimizePredictions, DCE::None);
  }

  if (RuntimeOption::EvalHHIRSimplification) {
    doPass(unit, simplifyPass, DCE::Full);
    doPass(unit, cleanCfg, DCE::None);
  }

  if (RuntimeOption::EvalHHIRGlobalValueNumbering) {
    doPass(unit, gvn, DCE::Full);
  }

  if (kind != TransKind::Profile && RuntimeOption::EvalHHIRMemoryOpts) {
    doPass(unit, optimizeLoads, DCE::Full);
  }

  if (kind != TransKind::Profile && RuntimeOption::EvalHHIRMemoryOpts) {
    doPass(unit, optimizeStores, DCE::Full);
  }

  if (kind != TransKind::Profile && RuntimeOption::EvalHHIRRefcountOpts) {
    doPass(unit, optimizeRefcounts2, DCE::Full);
  }

  if (RuntimeOption::EvalHHIRLICM) {
    if (kind != TransKind::Profile && hasLoop) {
      // The clean pass is just to stress lack of pre_headers for now, since
      // LICM is a disabled prototype pass.
      doPass(unit, cleanCfg, DCE::None);
      doPass(unit, optimizeLoopInvariantCode, DCE::Minimal);
    }
    doPass(unit, removeExitPlaceholders, DCE::Full);
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    doPass(unit, insertAsserts, DCE::None);
  }

  // Perform a final clean pass to collapse any critical edges that were
  // split.
  doPass(unit, cleanCfg, DCE::None);
}

//////////////////////////////////////////////////////////////////////

}}
