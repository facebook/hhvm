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

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

void optimize(IRUnit& unit, IRBuilder& irBuilder, TransKind kind) {
  Timer _t(Timer::optimize);

  auto finishPass = [&](const char* msg) {
    if (msg) {
      printUnit(6, unit, folly::format("after {}", msg).str().c_str());
    }
    assertx(checkCfg(unit));
    assertx(checkTmpsSpanningCalls(unit));
    if (debug) {
      forEachInst(rpoSortCfg(unit), [&](IRInstruction* inst) {
        assertx(checkOperandTypes(inst, &unit));
      });
    }
  };

  auto doPass = [&](void (*fn)(IRUnit&), const char* msg = nullptr) {
    fn(unit);
    finishPass(msg);
  };

  auto dce = [&](const char* which) {
    if (!RuntimeOption::EvalHHIRDeadCodeElim) return;
    eliminateDeadCode(unit);
    finishPass(folly::format("{} DCE", which).str().c_str());
  };

  auto const doReoptimize = RuntimeOption::EvalHHIRExtraOptPass &&
                            RuntimeOption::EvalHHIRSimplification;

  auto const hasLoop = RuntimeOption::EvalJitLoops && cfgHasLoop(unit);

  auto const traceMode = kind != TransKind::Optimize ||
                         RuntimeOption::EvalJitPGORegionSelector == "hottrace";

  // TODO (#5792564): Guard relaxation doesn't work with loops.
  // TODO (#6599498): Guard relaxation is broken in wholecfg mode.
  if (shouldHHIRRelaxGuards() && !hasLoop && traceMode) {
    Timer _t(Timer::optimize_relaxGuards);
    const bool simple = kind == TransKind::Profile &&
                        (RuntimeOption::EvalJitRegionSelector == "tracelet" ||
                         RuntimeOption::EvalJitRegionSelector == "method");
    RelaxGuardsFlags flags = (RelaxGuardsFlags)
      (RelaxReflow | (simple ? RelaxSimple : RelaxNormal));
    auto changed = relaxGuards(unit, *irBuilder.guards(), flags);
    if (changed) finishPass("guard relaxation");

    if (doReoptimize) {
      irBuilder.reoptimize();
      finishPass("guard relaxation reoptimize");
    }
  }

  // This is vestigial (it removes some instructions needed by the old refcount
  // opts pass), and will be removed soon.
  eliminateTakes(unit);

  dce("initial");

  if (RuntimeOption::EvalHHIRPredictionOpts) {
    doPass(optimizePredictions, "prediction opts");
  }

  if (doReoptimize) {
    irBuilder.reoptimize();
    finishPass("reoptimize");
    dce("reoptimize");
  }

  if (RuntimeOption::EvalHHIRGlobalValueNumbering) {
    doPass(gvn);
    dce("gvn");
  }

  if (kind != TransKind::Profile && RuntimeOption::EvalHHIRMemoryOpts) {
    doPass(optimizeLoads);
    dce("loadelim");
  }

  /*
   * Note: doing this pass this late might not be ideal, in particular because
   * we've already turned some StLoc instructions into StLocNT.
   *
   * But right now there are assumptions preventing us from doing it before
   * refcount opts.  (Refcount opts needs to see all the StLocs explicitly
   * because it makes assumptions about whether references are consumed based
   * on that.)
   */
  if (kind != TransKind::Profile && RuntimeOption::EvalHHIRMemoryOpts) {
    doPass(optimizeStores);
    dce("storeelim");
  }

  if (kind != TransKind::Profile && RuntimeOption::EvalHHIRRefcountOpts) {
    doPass(optimizeRefcounts2);
    dce("refcount");
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    doPass(insertAsserts);
  }
}

} }
