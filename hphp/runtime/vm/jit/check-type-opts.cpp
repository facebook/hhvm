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

#include "hphp/runtime/vm/jit/opt.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/dce.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/timer.h"

#include "hphp/util/dataflow-worklist.h"

namespace HPHP::jit {

TRACE_SET_MOD(hhir_checkTypes);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * This optimization reorders chained, back-to-back CheckType instructions where
 * the second check is for a subtype of the first one.  More specifically, this
 * optimization transforms this pattern:
 *
 *   [block]
 *     tmp1:type1 = CheckType<type1> tmp0 -> taken1  [ct1]
 *   [fallthru1]
 *     tmp2:type2 = CheckType<type2> tmp1 -> taken2  [ct2]
 *   [fallthru2]
 *     ...
 *
 *   vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 *   under the condition that type2 < type1, into the following one:
 *   vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 *
 *   [block]
 *     tmp2:type2 = CheckType<type2> tmp0 -> fallthru1 [ct1]
 *   [fallthru2]
 *     ...
 *
 *   [fallthru1]
 *     tmp1:type1 = CheckType<type1> tmp0 -> taken1  [ct2]
 *   [taken2]
 *
 */
bool reorderCheckTypes(IRUnit& unit) {
  Timer timer(Timer::optimize_reorderCheckTypes, unit.logEntry().get_pointer());
  PassTracer tracer{&unit, Trace::hhir_checkTypes, "reorderCheckTypes"};

  FTRACE(5, "ReorderCheckTypes:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "ReorderCheckTypes:^^^^^^^^^^^^^^^^^^^^^\n"); };

  auto changed = false;

  auto const blocks = rpoSortCfg(unit);
  for (auto& block : blocks) {
    auto& ct1 = block->back();
    if (!ct1.is(CheckType)) continue;
    auto fallthru1 = ct1.next();
    auto& ct2 = fallthru1->front();
    if (!ct2.is(CheckType)) continue;
    if (ct1.dst() != ct2.src(0)) continue;
    auto const type1 = ct1.typeParam();
    auto const type2 = ct2.typeParam();
    if (!(type2 < type1)) continue;

    if (!changed) {
      // This transformation assumes that any use of tmp1 that is
      // dominated by tmp2 actually uses tmp2 (which provides a more
      // refined type) and not tmp1. Thus we run refineTmps to make
      // sure that this invariant applies.
      //
      // This can be expensive, so we defer it until we know we're
      // going to make a change. This does not modify the CFG, so it's
      // safe to do so.
      refineTmps(unit);
    }
    changed = true;

    FTRACE(5, "  - reordering these 2 instructions:\n   - {}\n   - {}\n",
           ct1, ct2);

    auto tmp0 = ct1.src(0);
    auto tmp1 = ct1.dst(0);
    auto tmp2 = ct2.dst(0);
    auto taken1 = ct1.taken();
    auto taken2 = ct2.taken();
    auto fallthru2 = ct2.next();

    // Fix block, ct1 and tmp2.
    ct1.setTaken(fallthru1);
    ct1.setNext(fallthru2);
    ct1.setDst(tmp2);
    ct1.setTypeParam(type2);
    ct1.setSrc(0, tmp0);
    tmp2->setInstruction(&ct1);

    // Fix fallthru1, ct2 and tmp1.
    ct2.setTaken(taken1);
    ct2.setNext(taken2);
    ct2.setDst(tmp1);
    ct2.setTypeParam(type1);
    ct2.setSrc(0, tmp0);
    tmp1->setInstruction(&ct2);
    fallthru1->setProfCount(taken2->profCount());
    fallthru1->setHint(taken2->hint());
  }

  return changed;
}

//////////////////////////////////////////////////////////////////////

/*
 * Check Type Hoisting:
 *
 * This pass attempts to hoist CheckTypes further up the CFG past
 * DefLabels. This will allow the CheckTypes be optimized away (in
 * some cases). It tries to convert the following pattern:
 *
 * B1:
 *   .....
 *   Jmp t1:Str, t123:Cell -> B3
 *1
 * B2:
 *   .....
 *   Jmp t2:Int, t124:Cell -> B3
 *
 * B3:
 *   t3:Str|Int, t125:Cell = DefLabel B1,B2
 *   t5:Cell = Conjure
 *   t4:Str|Int = Passthrough t3:Str|Int
 *   .....
 *   t7:Int = CheckType<Int> t4:Str|Int -> B5
 *     -> B4
 *
 * Into:
 *
 * B1:
 *   .....
 *   t11:Bottom = CheckType<Int> t1:Str -> B6
 *     -> B7
 *
 * B2:
 *   .....
 *   t15:Int = CheckType<Int> t2:Int -> B8
 *     -> B9
 *
 * B6:
 *   Jmp t1:Str, t123:Cell -> B10
 *
 * B7:
 *   Jmp t11:Bottom, t123:Cell -> B11
 *
 * B8:
 *   Jmp t2:Int, t124:Cell -> B10
 *
 * B9:
 *   Jmp t15:Int, t124:Cell -> B11
 *
 * B10:
 *   t16:Str|Int, t126:Cell = DefLabel B6, B8
 *   t17:Cell = Conjure
 *   t18:Str|Int = Passthrough t16:Str|Int
 *   .....
 *   Jmp B5
 *
 * B11:
 *   t22:Int, t127:Cell = DefLabel B7, B9
 *   t20:Cell = Conjure
 *   t21:Int = Passthrough t22:Int
 *   .....
 *   Jmp B4
 *
 * Then rewrite:
 *   Starting at B5 t3 -> t16, t4 -> t18, t5 -> t17 ->, t125 -> t126
 *   Starting at B4 t3 -> t22, t4 -> t21, t5 -> t20, t7 -> t21, t125 -> t127
 *
 * Note that the CheckTypes in B1 and B2 can now be optimized away
 * (and the entire CFG simplified).
 */

// Clone the given block (and all instructions within). The block is assumed to
// end with a CheckType. The new block's CheckType will be replaced with a Jmp
// to dest.  Refuse to clone a block defing a FramePtr as that may lead to a
// frame pointer phi, which vasm does not handle well.
Block* cloneBlock(IRUnit& unit,
                  Block& block,
                  Block& dest,
                  jit::fast_map<SSATmp*, SSATmp*>& rewrites) {
  assertx(block.back().is(CheckType));
  auto const newBlock = unit.defBlock(dest.profCount(), dest.hint());

  // Walk over the block, cloning each instruction and recording the
  // new SSATmps defined.
  for (auto const& inst : block) {
    auto const newInst = [&] {
      if (inst.is(DefLabel)) {
        // We don't setup the types for the DefLabel dests here.  Instead we
        // retype them after hooking up the predecessor Jmps to this block.
        return unit.defLabel(inst.numDsts(), newBlock, inst.bcctx());
      } else if (inst.is(CheckType)) {
        auto const jmp = unit.gen(Jmp, inst.bcctx(), &dest);
        newBlock->append(jmp);
        return jmp;
      }
      auto const cloned = unit.clone(&inst);
      newBlock->append(cloned);
      return cloned;
    }();

    // Rewrite any uses of the old SSATmps with the new SSATmps within
    // the same block.
    for (uint32_t i = 0; i < newInst->numSrcs(); ++i) {
      auto const src = newInst->src(i);
      auto const it = rewrites.find(src);
      if (it == rewrites.end()) continue;
      newInst->setSrc(i, it->second);
    }

    if (inst.is(CheckType)) continue;
    assertx(inst.numDsts() == newInst->numDsts());
    for (uint32_t i = 0; i < inst.numDsts(); ++i) {
      if (inst.dst(i)->isA(TFramePtr)) return nullptr;
      rewrites.emplace(inst.dst(i), newInst->dst(i));
    }
  }
  return newBlock;
}

// Calculate per-block liveness of SSATmps
struct BlockLiveness {
  explicit BlockLiveness(const IRUnit& unit)
    : live{unit.numTmps()}
    , defs{unit.numTmps()}
    , uses{unit.numTmps()} {}
  size_t id;
  boost::dynamic_bitset<> live;
  boost::dynamic_bitset<> defs;
  boost::dynamic_bitset<> uses;
};

// Starting at the given block, rewrite all uses of the rewrite map
// keys to their associated values, inserting DefLabels as
// necessary. Any created DefLabels are stored in the `phi' output
// parameter. No DefLabels are created where the `phi' set says they
// have already been created.
void rewriteUses(IRUnit& unit,
                 Block& start,
                 const jit::fast_map<SSATmp*, SSATmp*>& rewrites,
                 const StateVector<Block, BlockLiveness>& liveness,
                 const IdomVector& idoms,
                 jit::fast_set<Block*>& phis) {
  BlockSet visited;
  jit::stack<Block*> worklist;

  // Visit each block reachable from the start block. For each one,
  // visit each instruction, doing the necessary rewrites. We stop if
  // all possible rewrite instructions are dead.
  visited.emplace(&start);
  worklist.push(&start);

  do {
    auto block = worklist.top();
    worklist.pop();

    // Check if all of the rewrites are dead. If so, we can stop.
    auto const& live = liveness[block].live;
    auto const anyLive = std::any_of(
      rewrites.begin(), rewrites.end(),
      [&] (auto const& rewrite) {
        return live[rewrite.first->id()];
      }
    );
    if (!anyLive) continue;

    if (!dominates(&start, block, idoms)) {
      // This block is not dominated by the start block. We cannot
      // perform the rewrites here because the targets are not
      // available. We need to create a DefLabel. If we already have,
      // stop (nothing more to do).
      if (phis.contains(block)) continue;

      // All of SSATmps in the rewrite map will be inputs to the
      // DefLabel. We want these to be in consistent order, so sort
      // them.
      jit::vector<SSATmp*> sorted;
      for (auto const& rewrite : rewrites) sorted.emplace_back(rewrite.first);
      std::sort(
        sorted.begin(), sorted.end(),
        [&] (SSATmp* s1, SSATmp* s2) { return s1->id() < s2->id(); }
      );

      jit::fast_map<SSATmp*, SSATmp*> newRewrites;
      for (auto const from : sorted) {
        // If it's dead, we can ignore it
        if (!live[from->id()]) continue;

        // Determine what to use for DefLabel inputs. If the pred is
        // dominated by the start, we can use the rewrite map. If not,
        // the DefLabel input should just be the original SSATmp (it
        // might be rewritten by other calls to rewriteUses).
        jit::hash_map<Block*, SSATmp*> inputs;
        block->forEachPred(
          [&] (Block* pred) {
            if (dominates(&start, pred, idoms)) {
              auto const it = rewrites.find(from);
              assertx(it != rewrites.end());
              inputs.emplace(pred, it->second);
            } else {
              inputs.emplace(pred, from);
            }
          }
        );
        // Create the DefLabel
        auto const newTmp = insertPhi(unit, block, inputs);
        newRewrites.emplace(from, newTmp);
      }

      // Record that we created the DefLabel, then recurse to rewrite
      // uses from the created phi. The rewrites are now using the new
      // SSATmps that the DefLabel defines.
      assertx(!newRewrites.empty());
      phis.emplace(block);
      rewriteUses(unit, *block, newRewrites, liveness, idoms, phis);
      continue;
    }

    // This block is dominated by the start block. This means the
    // rewrite targets are available, so just perform the rewrites.
    for (auto& inst : *block) {
      bool changed = false;
      for (uint32_t i = 0; i < inst.numSrcs(); ++i) {
        auto const it = rewrites.find(inst.src(i));
        if (it == rewrites.end()) continue;
        inst.setSrc(i, it->second);
        changed = true;
      }
      if (changed) retypeDests(&inst, &unit);
    }

    // Schedule each successor for processing, unless it's already been
    // visited.
    block->forEachSucc(
      [&] (Block* succ) {
        if (visited.contains(succ)) return;
        visited.emplace(succ);
        worklist.push(succ);
      }
    );
  } while (!worklist.empty());
}

struct Rewrite {
  Block* nextStart;
  Block* takenStart;
  Block* nextJoin;
  Block* takenJoin;
  jit::fast_map<SSATmp*, SSATmp*> next;
  jit::fast_map<SSATmp*, SSATmp*> taken;
};

StateVector<Block, BlockLiveness>
calcLiveness(const IRUnit& unit,
             const BlockList& blocks,
             const jit::vector<Rewrite>& rewrites) {
  StateVector<Block, BlockLiveness> liveness{unit, BlockLiveness{unit}};

  // For rewrites, we need liveness information, so gather it. We only
  // gather it for the SSATmps involved in the rewrite.
  boost::dynamic_bitset<> relevant{unit.numTmps()};
  for (auto const& rewrite : rewrites) {
    for (auto const& p : rewrite.next) {
      liveness[rewrite.nextJoin].defs.set(p.first->id());
      relevant.set(p.first->id());
    }
    for (auto const& p : rewrite.taken) {
      liveness[rewrite.takenJoin].defs.set(p.first->id());
      relevant.set(p.first->id());
    }
  }

  dataflow_worklist<size_t, std::less<size_t>> worklist{blocks.size()};
  for (size_t i = 0; i < blocks.size(); ++i) {
    auto const block = blocks[i];
    auto& state = liveness[block];
    state.id = i;
    worklist.push(i);

    for (auto const& inst : *block) {
      for (auto const src : inst.srcs()) state.uses.set(src->id());
    }
    state.uses &= relevant;
  }

  // Typical liveness dataflow:
  boost::dynamic_bitset<> temp{unit.numTmps()};
  do {
    auto const block = blocks[worklist.pop()];
    auto& state = liveness[block];

    temp.reset();
    block->forEachSucc(
      [&] (Block* succ) { temp |= liveness[succ].live; }
    );
    temp |= state.uses;
    temp -= state.defs;

    if (temp != state.live) {
      block->forEachPred(
        [&] (Block* pred) { worklist.push(liveness[pred].id); }
      );
      std::swap(state.live, temp);
    }
  } while (!worklist.empty());

  return liveness;
}

bool hoistCheckTypes(IRUnit& unit) {
  Timer timer(Timer::optimize_hoistCheckTypes, unit.logEntry().get_pointer());
  PassTracer tracer{&unit, Trace::hhir_checkTypes, "hoistCheckTypes"};

  FTRACE(4, "hoist check types:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(4, "hoist check types:^^^^^^^^^^^^^^^^^^^^^\n"); };

  jit::vector<Rewrite> rewrites;

  // Look for blocks which end in a CheckType and start with a
  // DefLabel. If the CheckType uses a SSATmp defined by that
  // DefLabel, we can hoist the CheckType above the DefLabel.
  for (auto const block : rpoSortCfg(unit)) {
    auto& checkType = block->back();
    if (!checkType.is(CheckType)) continue;

    auto const& defLabel = block->front();
    if (!defLabel.is(DefLabel)) continue;

    if (block->numPreds() <= 1) continue;

    auto const defIdx = defLabel.findDst(canonical(checkType.src(0)));
    if (defIdx == defLabel.numDsts()) continue;

    // Only run this optimization if we know we'll eliminate at least
    // one CheckType.
    auto worthwhile = false;
    block->forEachSrc(
      defIdx,
      [&] (const IRInstruction*, const SSATmp* s) {
        if (worthwhile) return;
        auto const refined = s->type() & checkType.src(0)->type();
        auto const checkTypeParam = checkType.typeParam();
        if (refined <= checkTypeParam || !refined.maybe(checkTypeParam)) {
          worthwhile = true;
        }
      }
    );
    if (!worthwhile) continue;

    // Record information to rewrite the uses of the old SSATmps
    // defined in the block with their new ones in rewrite.
    Rewrite rewrite;
    auto const ctNext = checkType.next();
    auto const ctTaken = checkType.taken();

    auto const nextJoin = cloneBlock(unit, *block, *ctNext, rewrite.next);
    if (!nextJoin) continue;
    auto const takenJoin = cloneBlock(unit, *block, *ctTaken, rewrite.taken);
    if (!takenJoin) continue;

    // At this point we know we have a block we're going to hoist:
    FTRACE(4, "Hoisting check-type {} in block B{}\n",
           checkType.toString(), block->id());

    rewrite.nextStart = ctNext;
    rewrite.takenStart = ctTaken;
    rewrite.nextJoin = nextJoin;
    rewrite.takenJoin = takenJoin;

    // For each predecessor replace its terminal phijmp with a copy of the
    // CheckType we are hoisting, and generate stub blocks for its next and
    // taken edges.  The phijmp we replaced is copied into the stub blocks and
    // retargeted to jump to the join blocks we defined earlier.  In the copied
    // phijumps we modify the src corresponding the the CheckType tmp to the
    // appropriate value (for the next block the checked tmp; for the taken
    // block then unchecked tmp).
    block->forEachPred([&] (Block* pred) {
      auto const next = unit.defBlock(pred->profCount(), pred->hint());
      auto const taken = unit.defBlock(pred->profCount(), pred->hint());

      auto& jmp = pred->back();
      next->append(unit.clone(&jmp));
      taken->append(unit.clone(&jmp));

      next->back().setTaken(nextJoin);
      taken->back().setTaken(takenJoin);

      auto const uncheckedTmp = jmp.src(defIdx);
      jmp.convertToNop();
      auto const newCheckType = unit.clone(&checkType);
      newCheckType->setSrc(0, uncheckedTmp);
      pred->append(newCheckType);
      retypeDests(newCheckType, &unit);

      next->back().setSrc(defIdx, newCheckType->dst());
      taken->back().setSrc(defIdx, uncheckedTmp);

      newCheckType->setNext(next);
      newCheckType->setTaken(taken);
    });

    // While walking the next join block we track any passthrough operations on
    // the result of the CheckType.
    auto checkTypeTmp = nextJoin->front().dst(defIdx);

    // Update the now fully formed DefLabel's dst types and any operations that
    // might depend on them
    for (auto& inst : *nextJoin) {
      retypeDests(&inst, &unit);
      if (inst.isPassthrough() && inst.getPassthroughValue() == checkTypeTmp) {
        assertx(!inst.isControlFlow());
        checkTypeTmp = inst.dst();
      }
    }
    for (auto& inst : *takenJoin) {
      retypeDests(&inst, &unit);
    }

    // Add an extra rewrite for the next block to rewrite the CheckType result.
    rewrite.next.emplace(checkType.dst(), checkTypeTmp);

    // The old block is now unreachable, so unlink it from the CFG.
    checkType.setNext(nullptr);
    checkType.setTaken(nullptr);

    rewrites.emplace_back(std::move(rewrite));
  }

  // Nothing to rewrite, which means we didn't do anything
  if (rewrites.empty()) return false;

  auto const blocks = rpoSortCfg(unit);
  auto const liveness = calcLiveness(unit, blocks, rewrites);

  // Rewrite the uses. The next and taken branches have the created
  // phis in common, as they should not revisit ones the other
  // created.
  auto const idoms = findDominators(unit, blocks, numberBlocks(unit, blocks));
  for (auto const& rewrite : rewrites) {
    jit::fast_set<Block*> phis;
    rewriteUses(unit, *rewrite.nextStart, rewrite.next, liveness, idoms, phis);
    rewriteUses(
      unit, *rewrite.takenStart, rewrite.taken, liveness, idoms, phis
    );
  }

  // At this point the unit is now valid, though might be a bit
  // convoluted. Rely on other passes to clean it up.
  return true;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

bool optimizeCheckTypes(IRUnit& unit) {
  splitCriticalEdges(unit);
  auto changed = reorderCheckTypes(unit);
  changed |= hoistCheckTypes(unit);
  return changed;
}

//////////////////////////////////////////////////////////////////////

}
