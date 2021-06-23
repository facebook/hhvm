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

#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/simple-propagation.h"
#include "hphp/runtime/vm/jit/timer.h"

#include "hphp/util/trace.h"

#include <iterator>

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir_phi);

namespace {

struct SinkableInst {
  explicit operator bool() const { return inst; }

  const IRInstruction* inst;
  bool is_lval;
};

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
SinkableInst findSinkablePhiSrc(
  const jit::flat_set<SSATmp*>& values
) {
  const IRInstruction* inst = nullptr;
  auto is_lval = false;

  for (auto val : values) {
    if (inst && val->inst()->is(ConvPtrToLval) != is_lval) return {};

    if (val->inst()->is(ConvPtrToLval)) {
      val = val->inst()->src(0);
      is_lval = true;
    }
    if (!val->inst()->is(LdLocAddr, LdStkAddr, LdMIStateAddr)) {
      return {};
    }
    assertx(val->inst()->numSrcs() <= 1);

    if (inst == nullptr) {
      inst = val->inst();
      continue;
    }

    auto newInst = val->inst();
    if (newInst->op() != inst->op() ||
        newInst->hasExtra() != inst->hasExtra() ||
        (newInst->hasExtra() &&
         !equalsExtra(newInst->op(), newInst->rawExtra(), inst->rawExtra())) ||
        newInst->numSrcs() != inst->numSrcs() ||
        (newInst->numSrcs() && newInst->src(0) != inst->src(0))) {
      return {};
    }
  }

  return {inst, is_lval};
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
bool optimizePhis(IRUnit& unit) {
  auto changed = false;
  PassTracer pt{&unit, TRACEMOD, "optimizePhis", &changed};
  Timer t(Timer::optimize_phis, unit.logEntry().get_pointer());

  bool repeat;
  jit::flat_set<SSATmp*> values;
  jit::flat_set<SSATmp*> foldable_labels;
  auto processBlock = [&](Block* b) {
    auto it = b->begin();
    auto& label = *it;
    if (!label.is(DefLabel)) return;
    // Look for blocks of the form t = DefLabel; Jmp[N]Zero t
    // We're interested in the case that all inputs are constants,
    // so that we can redirect the phijmps.
    auto maybe_foldable = (label.numDsts() == 1 &&
                           ++it != b->end() &&
                           it->is(JmpZero, JmpNZero) &&
                           it->src(0) == label.dst(0));

    for (unsigned i = 0; i < label.numDsts(); ++i) {
      auto insert_after = b->iteratorTo(&label);
      values.clear();
      b->forEachSrc(i, [&](IRInstruction* jmp, SSATmp*) {
        copyProp(jmp);
        auto const src = jmp->src(i);
        values.insert(src);
        if (maybe_foldable &&
            !src->type().hasConstVal(TBool) &&
            !src->type().hasConstVal(TInt)) {
          maybe_foldable = false;
        }
      });

      auto const phiDest = label.dst(i);
      IRInstruction* newInst = nullptr;
      if (phiDest->type().admitsSingleVal()) {
        newInst =
          unit.gen(phiDest, Mov, label.bcctx(), unit.cns(phiDest->type()));
      } else if (values.size() == 1 ||
                 (values.size() == 2 && values.count(phiDest))) {
        values.erase(phiDest);
        // This is safe without any extra dominator checks because we know that
        // there are no preds that don't have the value available.
        newInst = unit.gen(phiDest, Mov, label.bcctx(), *values.begin());
      } else if (auto sink = findSinkablePhiSrc(values)) {
        // As long as DefInlineFP exists, FramePtr SSATmps aren't truly
        // SSA. We have to make sure the live FramePtr at the point of the
        // DefLabel is the same as the one from the LdLocAddr, if that's the
        // instruction we're trying to sink.
        if (sink.inst->is(LdLocAddr) &&
            sink.inst->marker().fp() != label.marker().fp()) {
          continue;
        }

        newInst = unit.clone(sink.inst, sink.is_lval ? nullptr : phiDest);
        newInst->marker() = label.marker();
        if (sink.is_lval) {
          b->insert(std::next(insert_after), newInst);
          insert_after++;
          newInst =
            unit.gen(phiDest, ConvPtrToLval, label.bcctx(), newInst->dst());
        }
      }

      if (newInst != nullptr) {
        deletePhiDest(&label, i);
        b->insert(std::next(insert_after), newInst);
        changed = repeat = true;
      } else if (maybe_foldable) {
        foldable_labels.insert(phiDest);
      }
    }

    if (label.numDsts() == 0) b->erase(&label);
  };

  // We've found a candidate set of DefLabels whose jmps can simply
  // be redirected; but if the DefLabel's output is used beyoud its block
  // we can't do the transformation.
  auto checkFoldable = [&](Block* b) {
    for (auto& i : *b) {
      for (auto src : i.srcs()) {
        auto it = foldable_labels.find(src);
        if (it != foldable_labels.end()) {
          if (src->inst()->block() != b) {
            foldable_labels.erase(it);
          }
        }
      }
    }
  };

  auto fixFoldable = [&](Block* b) {
    auto it = b->begin();
    auto& label = *it++;
    assertx(label.is(DefLabel));
    assertx(label.numDsts() == 1 &&
            it != b->end() &&
            it->is(JmpZero, JmpNZero) &&
            it->src(0) == label.dst(0));

    jit::vector<std::pair<IRInstruction*,Block*>> jmps;
    b->forEachSrc(0, [&](IRInstruction* jmp, SSATmp*) {
      auto const src = jmp->src(0);
      bool flag;
      if (src->type().hasConstVal(TBool)) {
        flag = src->type().boolVal();
      } else {
        assertx(src->type().hasConstVal(TInt));
        flag = src->type().intVal();
      }
      auto const target = flag == it->is(JmpNZero) ? it->taken() : it->next();
      assertx(target);
      jmps.emplace_back(jmp, target);
    });

    deletePhiDest(&label, 0);
    assertx(label.numDsts() == 0);
    b->erase(&label);

    for (auto const& jmp : jmps) {
      jmp.first->setTaken(jmp.second);
    }
  };

  do {
    repeat = false;
    foldable_labels.clear();
    postorderWalk(unit, processBlock);
    if (foldable_labels.size()) {
      postorderWalk(unit, checkFoldable);
      if (foldable_labels.size()) {
        for (auto src : foldable_labels) {
          fixFoldable(src->inst()->block());
        }
        repeat = changed = true;
      }
    }
  } while (repeat);

  return false;
}

}}
