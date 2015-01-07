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

namespace {

// insert inst after definer
void insertAfter(IRInstruction* definer, IRInstruction* inst) {
  assert(!definer->isBlockEnd());
  Block* block = definer->block();
  auto pos = block->iteratorTo(definer);
  block->insert(++pos, inst);
}

/*
 * Insert a DbgAssertRefCount instruction after each place we produce
 * a refcounted value.  The value must be something we can safely dereference
 * to check the _count field.
 */
void insertRefCountAsserts(IRInstruction& inst, IRUnit& unit) {
  for (SSATmp& dst : inst.dsts()) {
    auto const t = dst.type();
    if (t <= (Type::Counted | Type::StaticStr | Type::StaticArr)) {
      insertAfter(&inst, unit.gen(DbgAssertRefCount, inst.marker(), &dst));
    }
  }
}

void insertStStkAssert(IRInstruction& inst, IRUnit& unit) {
  if (!(inst.src(1)->type() <= Type::Gen)) return;
  auto const addr = unit.gen(
    LdStkAddr,
    inst.marker(),
    Type::PtrToStkGen,
    StackOffset { inst.extra<StStk>()->offset },
    inst.src(0)
  );
  auto const block = inst.block();
  auto pos = block->iteratorTo(&inst);
  ++pos;
  block->insert(pos, addr);
  auto const check = unit.gen(DbgAssertPtr, inst.marker(), addr->dst());
  block->insert(pos, check);
}

void insertCallAssert(IRInstruction& inst, IRUnit& unit) {
  auto const sp = inst.dst();
  auto const addr = unit.gen(
    LdStkAddr,
    inst.marker(),
    Type::PtrToStkGen,
    StackOffset { 0 },
    sp
  );
  insertAfter(&inst, addr);
  auto const check = unit.gen(DbgAssertPtr, inst.marker(), addr->dst());
  insertAfter(addr, check);
}

void insertAsserts(IRUnit& unit) {
  postorderWalk(unit, [&](Block* block) {
    for (auto it = block->begin(); it != block->end();) {
      auto& inst = *it;
      ++it;
      if (inst.op() == StStk) {
        insertStStkAssert(inst, unit);
        continue;
      }
      if (inst.op() == Call) {
        insertCallAssert(inst, unit);
        continue;
      }
      if (!inst.isBlockEnd()) {
        insertRefCountAsserts(inst, unit);
      }
    }
  });
}

}

//////////////////////////////////////////////////////////////////////

void optimize(IRUnit& unit, IRBuilder& irBuilder, TransKind kind) {
  Timer _t(Timer::optimize);

  auto finishPass = [&](const char* msg) {
    if (msg) {
      printUnit(6, unit, folly::format("after {}", msg).str().c_str());
    }
    assert(checkCfg(unit));
    assert(checkTmpsSpanningCalls(unit));
    if (debug) {
      forEachInst(rpoSortCfg(unit), [&](IRInstruction* inst) {
        assert(checkOperandTypes(inst, &unit));
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
    (RuntimeOption::EvalHHIRCse || RuntimeOption::EvalHHIRSimplification);

  auto const hasLoop = RuntimeOption::EvalJitLoops && cfgHasLoop(unit);

  // TODO(#5792564): Guard relaxation doesn't work with loops.
  if (shouldHHIRRelaxGuards() && !hasLoop) {
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

  if (RuntimeOption::EvalHHIRRefcountOpts) {
    optimizeRefcounts(unit, FrameStateMgr{unit.entry()->front().marker()});
    finishPass("refcount opts");
  }

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

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    doPass(insertAsserts, "RefCnt asserts");
  }
}

} }
