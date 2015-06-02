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

#include "hphp/runtime/vm/jit/mutation.h"

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/analysis.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

namespace {

//////////////////////////////////////////////////////////////////////

using DomChildren = StateVector<Block,BlockList>;
DomChildren findDomChildren(const IRUnit& unit,
                            const IdomVector& idoms,
                            const BlockList& blocks) {
  auto ret = DomChildren(unit, BlockList{});
  for (auto& block : blocks) {
    if (auto const idom = idoms[block]) {
      ret[idom].push_back(block);
    }
  }
  return ret;
}

bool retypeDst(IRInstruction* inst, int num) {
  auto const ssa = inst->dst(num);
  auto const oldType = ssa->type();

  /*
   * The type of a tmp defined by DefLabel is the union of the types of the
   * tmps at each incoming Jmp.
   */
  if (inst->op() == DefLabel) {
    auto type = TBottom;
    inst->block()->forEachSrc(num, [&] (IRInstruction*, SSATmp* tmp) {
      type = type | tmp->type();
    });
    ssa->setType(type);
    return type != oldType;
  }

  auto const newType = outputType(inst, num);
  ssa->setType(newType);

  return newType != oldType;
}

struct RefineTmpsRec {
  explicit RefineTmpsRec(IRUnit& unit,
                         const IdomVector* idoms,
                         const BlockList& rpoBlocks)
    : state{unit.numTmps()}
    , idoms(*idoms)
    , dchildren{findDomChildren(unit, *idoms, rpoBlocks)}
  {}

  void go(Block* blk) {
    TRACE_SET_MOD(hhir_refineTmps);

    auto saved_state = folly::Optional<sparse_idptr_map<SSATmp,SSATmp*>>{};

    FTRACE(3, "B{}\n", blk->id());
    for (auto& inst : blk->instrs()) {
      FTRACE(3, "  {}\n", inst);
      for (auto srcID = uint32_t{0}; srcID < inst.numSrcs(); ++srcID) {
        auto const src = inst.src(srcID);
        if (auto const replace = find_replacement(src, blk)) {
          FTRACE(1, "    rewrite {} -> {} in {}\n", *src, *replace, inst);
          inst.setSrc(srcID, replace);
          if (inst.hasDst()) needsReflow = true;
        }
      }

      if (isCallOp(inst.op()) && !state.empty()) {
        if (!saved_state) saved_state = state;
        state.clear();
        continue;
      }

      if (inst.is(CheckType, AssertType)) {
        if (!saved_state) saved_state = state;
        auto const dst = inst.dst();
        auto const src = inst.src(0);
        state[src] = dst;
        continue;
      }
    }

    for (auto& child : dchildren[blk]) go(child);
    if (saved_state) state.swap(*saved_state);
  }

  SSATmp* find_replacement(SSATmp* origSrc, Block* blk) {
    if (!state.contains(origSrc)) return nullptr;
    auto cand = state[origSrc];
    if (is_tmp_usable(idoms, cand, blk)) return cand;
    while (cand->inst()->isPassthrough()) {
      cand = cand->inst()->getPassthroughValue();
      if (cand == origSrc || !cand) return nullptr;
      if (is_tmp_usable(idoms, cand, blk)) return cand;
    }
    return nullptr;
  }

  sparse_idptr_map<SSATmp,SSATmp*> state;
  const IdomVector& idoms;
  const DomChildren dchildren;
  bool needsReflow{false};
};

//////////////////////////////////////////////////////////////////////

}

void cloneToBlock(const BlockList& rpoBlocks,
                  IRUnit& unit,
                  Block::iterator const first,
                  Block::iterator const last,
                  Block* const target) {
  StateVector<SSATmp,SSATmp*> rewriteMap(unit, nullptr);

  auto rewriteSources = [&] (IRInstruction* inst) {
    for (int i = 0; i < inst->numSrcs(); ++i) {
      if (auto newTmp = rewriteMap[inst->src(i)]) {
        FTRACE(5, "  rewrite: {} -> {}\n",
               inst->src(i)->toString(),
               newTmp->toString());
        inst->setSrc(i, newTmp);
      }
    }
  };

  auto targetIt = target->skipHeader();
  for (auto it = first; it != last; ++it) {
    assertx(!it->isControlFlow());

    FTRACE(5, "cloneToBlock({}): {}\n", target->id(), it->toString());
    auto const newInst = unit.clone(&*it);

    if (auto const numDests = newInst->numDsts()) {
      for (int i = 0; i < numDests; ++i) {
        FTRACE(5, "  add rewrite: {} -> {}\n",
               it->dst(i)->toString(),
               newInst->dst(i)->toString());
        rewriteMap[it->dst(i)] = newInst->dst(i);
      }
    }

    target->insert(targetIt, newInst);
    targetIt = ++target->iteratorTo(newInst);
  }

  postorderWalk(
    unit,
    [&](Block* block) {
      FTRACE(5, "cloneToBlock: rewriting block {}\n", block->id());
      for (auto& inst : *block) {
        FTRACE(5, " rewriting {}\n", inst.toString());
        rewriteSources(&inst);
      }
    },
    target
  );
}

void moveToBlock(Block::iterator const first,
                 Block::iterator const last,
                 Block* const target) {
  if (first == last) return;

  auto const srcBlock = first->block();

  auto targetIt = target->skipHeader();
  for (auto it = first; it != last;) {
    auto const inst = &*it;
    assertx(!inst->isControlFlow());

    FTRACE(5, "moveToBlock({}): {}\n",
           target->id(),
           inst->toString());

    it = srcBlock->erase(it);
    target->insert(targetIt, inst);
    targetIt = ++target->iteratorTo(inst);
  }
}

bool retypeDests(IRInstruction* inst, const IRUnit* unit) {
  auto changed = false;
  for (auto i = uint32_t{0}; i < inst->numDsts(); ++i) {
    DEBUG_ONLY auto const oldType = inst->dst(i)->type();
    if (retypeDst(inst, i)) {
      changed = true;
      ITRACE(5, "reflowTypes: retyped {} in {}\n", oldType.toString(),
             inst->toString());
    }
  }
  return changed;
}

/*
 * Algorithm for reflow:
 * 1. for each block in reverse postorder:
 * 2.   compute dest types of each instruction in forwards order
 * 3.   if the block ends with a jmp that passes types to a label,
 *      and the jmp is a loop edge,
 *      and any passed types cause the target label's type to widen,
 *      then set again=true
 * 4. if again==true, goto step 1
 */
void reflowTypes(IRUnit& unit) {
  auto const blocks = rpoSortCfg(unit);
  auto const ids    = numberBlocks(unit, blocks);

  auto isBackEdge = [&] (Block* from, Block* to) {
    return ids[from] > ids[to];
  };

  for (bool again = true; again;) {
    again = false;
    for (auto* block : blocks) {
      FTRACE(5, "reflowTypes: visiting block {}\n", block->id());
      for (auto& inst : *block) retypeDests(&inst, &unit);
      auto& jmp = block->back();
      auto n = jmp.numSrcs();
      if (!again && jmp.is(Jmp) && n > 0 && isBackEdge(block, jmp.taken())) {
        // if we pass a widening type to a label, loop again.
        auto srcs = jmp.srcs();
        auto dsts = jmp.taken()->front().dsts();
        for (unsigned i = 0; i < n; ++i) {
          if (srcs[i]->type() <= dsts[i]->type()) continue;
          again = true;
          break;
        }
      }
    }
  }
}

void insertNegativeAssertTypes(IRUnit& unit, const BlockList& blocks) {
  for (auto& blk : blocks) {
    auto const& inst = blk->back();
    if (!inst.is(CheckType)) continue;

    // Note that we can't assert the type if this isn't the only predecessor,
    // but also, it's currently not the case that it would ever be useful to do
    // so.  If a taken edge coming out of a CheckType is critical, even if we
    // split it so we could insert the AssertType, there would be no use of the
    // tmp in the edge-splitting-block to rewrite to the dst of the AssertType.
    if (inst.taken()->numPreds() != 1) continue;

    auto const checkTy = inst.typeParam();
    auto const srcTy   = inst.src(0)->type();
    auto const takenTy = negativeCheckType(srcTy, checkTy);
    if (takenTy < srcTy) {
      inst.taken()->prepend(
        unit.gen(AssertType, inst.marker(), takenTy, inst.src(0))
      );
    }
  }
}

void refineTmps(IRUnit& unit,
                const BlockList& rpoBlocks,
                const IdomVector& idoms) {
  TRACE_SET_MOD(hhir_refineTmps);
  PassTracer tracer{&unit, Trace::hhir_refineTmps, "refineTmps"};
  RefineTmpsRec refiner{unit, &idoms, rpoBlocks};
  refiner.go(unit.entry());
  if (refiner.needsReflow) reflowTypes(unit);
}

SSATmp* insertPhi(IRUnit& unit, Block* blk,
                  const jit::vector<SSATmp*>& inputs) {
  assert(blk->numPreds() > 1);
  auto label = &blk->front();
  if (!label->is(DefLabel)) {
    label = unit.defLabel(1, label->marker());
    blk->insert(blk->begin(), label);
  } else {
    for (auto d = label->numDsts(); d--; ) {
      auto result = label->dst(d);
      uint32_t i = 0;
      blk->forEachPred([&](Block* pred) {
          if (result) {
            auto& jmp = pred->back();
            if (jmp.src(d) != inputs[i++]) {
              result = nullptr;
            }
          }
        });
      if (result) return result;
    }
    unit.expandLabel(label, 1);
  }

  uint32_t i = 0;
  blk->forEachPred([&](Block* pred) {
      unit.expandJmp(&pred->back(), inputs[i++]);
    });
  retypeDests(label, &unit);
  return label->dst(label->numDsts() - 1);
}

//////////////////////////////////////////////////////////////////////

}}
