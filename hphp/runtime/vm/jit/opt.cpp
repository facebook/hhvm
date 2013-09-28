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

#include "hphp/runtime/vm/jit/opt.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/trace-builder.h"

namespace HPHP {
namespace JIT {

// insert inst after the point dst is defined
static void insertAfter(IRInstruction* definer, IRInstruction* inst) {
  assert(!definer->isBlockEnd());
  Block* block = definer->block();
  auto pos = block->iteratorTo(definer);
  if (pos->op() == DefLabel) {
    ++pos;
  }
  block->insert(++pos, inst);
}

/*
 * Insert a DbgAssertRefCount instruction after each place we produce
 * a refcounted value.  The value must be something we can safely dereference
 * to check the _count field.
 */
static void insertRefCountAsserts(IRInstruction& inst, IRUnit& unit) {
  for (SSATmp& dst : inst.dsts()) {
    Type t = dst.type();
    if (t.subtypeOf(Type::Counted | Type::StaticStr | Type::StaticArr)) {
      insertAfter(&inst, unit.gen(DbgAssertRefCount, inst.marker(), &dst));
    }
  }
}

/*
 * Insert a DbgAssertTv instruction for each stack location stored to by
 * a SpillStack instruction.
 */
static void insertSpillStackAsserts(IRInstruction& inst, IRUnit& unit) {
  SSATmp* sp = inst.dst();
  auto const vals = inst.srcs().subpiece(2);
  auto* block = inst.block();
  auto pos = block->iteratorTo(&inst); ++pos;
  for (unsigned i = 0, n = vals.size(); i < n; ++i) {
    Type t = vals[i]->type();
    if (t.subtypeOf(Type::Gen)) {
      IRInstruction* addr = unit.gen(LdStackAddr,
                                     inst.marker(),
                                     Type::PtrToGen,
                                     StackOffset(i),
                                     sp);
      block->insert(pos, addr);
      IRInstruction* check = unit.gen(DbgAssertPtr, inst.marker(), addr->dst());
      block->insert(pos, check);
    }
  }
}

/*
 * Insert asserts at various points in the IR.
 * TODO: t2137231 Insert DbgAssertPtr at points that use or produces a GenPtr
 */
static void insertAsserts(IRTrace* trace, IRUnit& unit) {
  forEachTraceBlock(trace, [&](Block* block) {
    for (auto it = block->begin(), end = block->end(); it != end; ) {
      IRInstruction& inst = *it;
      ++it;
      if (inst.op() == SpillStack) {
        insertSpillStackAsserts(inst, unit);
        continue;
      }
      if (inst.op() == Call) {
        SSATmp* sp = inst.dst();
        IRInstruction* addr = unit.gen(LdStackAddr,
                                       inst.marker(),
                                       Type::PtrToGen,
                                       StackOffset(0),
                                       sp);
        insertAfter(&inst, addr);
        insertAfter(addr, unit.gen(DbgAssertPtr, inst.marker(), addr->dst()));
        continue;
      }
      if (!inst.isBlockEnd()) insertRefCountAsserts(inst, unit);
    }
  });
}

void optimizeTrace(IRTrace* trace, TraceBuilder& traceBuilder) {
  auto& unit = traceBuilder.unit();

  auto finishPass = [&](const char* msg) {
    dumpTrace(6, trace, folly::format("after {}", msg).str().c_str());
    assert(checkCfg(trace, unit));
    assert(checkTmpsSpanningCalls(trace, unit));
    if (debug) forEachTraceInst(trace, assertOperandTypes);
  };

  auto doPass = [&](void (*fn)(IRTrace*, IRUnit&),
                    const char* msg) {
    fn(trace, unit);
    finishPass(msg);
  };

  auto dce = [&](const char* which) {
    if (!RuntimeOption::EvalHHIRDeadCodeElim) return;
    eliminateDeadCode(trace, unit);
    finishPass(folly::format("{} DCE", which).str().c_str());
  };

  if (RuntimeOption::EvalHHIRRelaxGuards) {
    auto changed = relaxGuards(trace, unit, *traceBuilder.guards());
    if (changed) finishPass("guard relaxation");
  }

  dce("initial");

  if (RuntimeOption::EvalHHIRPredictionOpts) {
    doPass(optimizePredictions, "prediction opts");
  }

  if (RuntimeOption::EvalHHIRExtraOptPass
      && (RuntimeOption::EvalHHIRCse
          || RuntimeOption::EvalHHIRSimplification)) {
    traceBuilder.reoptimize();
    finishPass("reoptimize");
    // Cleanup any dead code left around by CSE/Simplification
    // Ideally, this would be controlled by a flag returned
    // by optimzeTrace indicating whether DCE is necessary
    dce("reoptimize");
  }

  if (RuntimeOption::EvalHHIRJumpOpts) {
    doPass(optimizeJumps, "jumpopts");
    dce("jump opts");
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    doPass(insertAsserts, "RefCnt asserts");
  }
}

} }
