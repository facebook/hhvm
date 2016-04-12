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

#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/simplify.h"

#include "hphp/util/trace.h"

#include <folly/Optional.h>

#include <iterator>

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir_phi);

namespace {

/*
 * If every value in values is equivalent and safe to sink through a phi,
 * return a model instruction to be copied. Otherwise, return nullptr.
 *
 * Currently only supports a very limited set of instructions to simplify the
 * decision for when it's safe to clone/sink them:
 * - No memory effects (read or write).
 * - One SSATmp src, which must be either a constant or the globally-available
     FramePtr/StkPtr.
 */
const IRInstruction* findSinkablePhiSrc(
  const jit::flat_set<SSATmp*>& values
) {
  const IRInstruction* inst = nullptr;

  for (auto val : values) {
    if (!val->inst()->is(LdLocAddr, LdStkAddr, LdMIStateAddr)) {
      return nullptr;
    }
    assertx(val->inst()->numSrcs() == 1);

    if (inst == nullptr) {
      inst = val->inst();
      continue;
    }

    auto newInst = val->inst();
    if (newInst->op() != inst->op() ||
        newInst->hasExtra() != inst->hasExtra() ||
        (newInst->hasExtra() &&
         !equalsExtra(newInst->op(), newInst->rawExtra(), inst->rawExtra())) ||
        newInst->src(0) != inst->src(0)) {
      return nullptr;
    }
  }

  return inst;
}

}

/*
 * Perform a few simple optimizations on DefLabels:
 * - If a phi's dest has a type that represents a constant value, replace the
 *   phi with a Mov from a DefConst (to be copy-propagated by a later pass).
 * - If the preds provide only one unique value other than the dest of the phi
     itself, replace the phi with a Mov of that unique value.
 * - If all preds provide an equivalent value, insert a copy of the instruction
 *   defining that value after the DefLabel, replacing the phi. This is only
 *   done for a limited whitelist of instructions that are safe and cheap to
 *   sink.
 */
void optimizePhis(IRUnit& unit) {
  auto changed = false;
  PassTracer pt{&unit, TRACEMOD, "optimizePhis", &changed};

  bool repeat;
  jit::flat_set<SSATmp*> values;
  auto processBlock = [&](Block* b) {
    auto& label = b->front();
    if (!label.is(DefLabel)) return;

    for (unsigned i = 0; i < label.numDsts(); ++i) {
      values.clear();
      b->forEachSrc(i, [&](IRInstruction* jmp, SSATmp*) {
        copyProp(jmp);
        values.insert(jmp->src(i));
      });

      auto const phiDest = label.dst(i);
      IRInstruction* newInst = nullptr;
      if (phiDest->hasConstVal() ||
          phiDest->type().subtypeOfAny(TUninit, TInitNull, TNullptr)) {
        newInst =
          unit.gen(phiDest, Mov, label.marker(), unit.cns(phiDest->type()));
      } else if (values.size() == 1 ||
                 (values.size() == 2 && values.count(phiDest))) {
        values.erase(phiDest);
        // This is safe without any extra dominator checks because we know that
        // there are no preds that don't have the value available.
        newInst = unit.gen(phiDest, Mov, label.marker(), *values.begin());
      } else if (auto sinkInst = findSinkablePhiSrc(values)) {
        // As long as DefInlineFP exists, FramePtr SSATmps aren't truly
        // SSA. We have to make sure the live FramePtr at the point of the
        // DefLabel is the same as the one from the LdLocAddr, if that's the
        // instruction we're trying to sink.
        if (sinkInst->is(LdLocAddr) &&
            sinkInst->marker().fp() != label.marker().fp()) {
          continue;
        }
        newInst = unit.clone(sinkInst, phiDest);
        newInst->marker() = label.marker();
      }

      if (newInst != nullptr) {
        deletePhiDest(&label, i);
        b->insert(std::next(b->iteratorTo(&label)), newInst);
        changed = repeat = true;
      }
    }

    if (label.numDsts() == 0) b->erase(&label);
  };

  do {
    repeat = false;
    postorderWalk(unit, processBlock);
  } while (repeat);
}

}}
