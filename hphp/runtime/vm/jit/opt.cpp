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
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/timer.h"

#include "hphp/util/trace.h"

#include <boost/dynamic_bitset.hpp>

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

void optimize(IRUnit& unit, IRBuilder& irBuilder, TransKind kind) {
  Timer _t(Timer::optimize);

  auto const finishPass = [&] (const char* msg) {
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

  auto const doPass = [&] (void (*fn)(IRUnit&), const char* msg = nullptr) {
    fn(unit);
    finishPass(msg);
  };

  auto const dce = [&] (const char* which) {
    if (!RuntimeOption::EvalHHIRDeadCodeElim) return;
    eliminateDeadCode(unit);
    finishPass(folly::format("{} DCE", which).str().c_str());
  };

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

    if (RuntimeOption::EvalHHIRSimplification) {
      doPass(simplify, "guard relaxation simplify");
      doPass(cleanCfg);
    }
  }

  dce("initial");

  if (RuntimeOption::EvalHHIRPredictionOpts) {
    doPass(optimizePredictions, "prediction opts");
  }

  if (RuntimeOption::EvalHHIRSimplification) {
    doPass(simplify, "simplify");
    dce("simplify");
    doPass(cleanCfg);
  }

  if (RuntimeOption::EvalHHIRGlobalValueNumbering) {
    doPass(gvn);
    dce("gvn");
  }

  if (kind != TransKind::Profile && RuntimeOption::EvalHHIRMemoryOpts) {
    doPass(optimizeLoads);
    dce("loadelim");
  }

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

//////////////////////////////////////////////////////////////////////

}}
