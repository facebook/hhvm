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

namespace HPHP { namespace jit {

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
 *   Jmp t1:Str -> B3
 *
 * B2:
 *   .....
 *   Jmp t2:Int -> B3
 *
 * B3:
 *   t3:Str|Int = DefLabel B1,B2
 *   t5:Cell = Conjure
 *   t6:Cell = Conjure
 *   .....
 *   t7:Int = CheckType<Int> t3:Str|Int -> B5
 *     -> B4
 *
 * Into:
 *
 * B1:
 *   .....
 *   Jmp t1:Str -> B6
 *
 * B2:
 *   .....
 *   Jmp t2:Int -> B7
 *
 * B6:
 *   t8:Str = DefLabel B1
 *   t9:Cell = Conjure
 *   t10:Cell = Conjure
 *   t11:Int = CheckType<Int> t8:Str -> B8
 *     -> B9
 *
 * B7:
 *   t12:Int = DefLabel B2
 *   t13:Cell = Conjure
 *   t14:Cell = Conjure
 *   t15:Int = CheckType<Int> t12:Int -> B10
 *     -> B11
 *
 * B8:
 *   Jmp t8:Str,t9:Cell,t10:Cell -> B12
 *
 * B9:
 *   Jmp t8:Str,t9:Cell,t10:Cell,t11:Int -> B13
 *
 * B10:
 *   Jmp t12:Int,t13:Cell,t14:Cell -> B12
 *
 * B11:
 *   Jmp t12:Int,t13:Cell,t14:Cell,t15:Int -> B13
 *
 * B12:
 *   t16:Str|Int,t17:Cell,t18:Cell = DefLabel B8, B10
 *   Jmp B5
 *
 * B13:
 *   t19:Str|Int,t20:Cell,t21:Cell,t22:Int = DefLabel B9, B11
 *   Jmp B4
 *
 * Then rewrite t3 -> t16, t5 -> t17 ->, t6 -> t18 starting at B5, and
 * rewrite t3 -> t19, t5 -> t20, t6 -> t21, t7 -> t22 starting at B4.
 *
 * Note that the CheckTypes in B6 and B7 can now be optimized away
 * (and the entire CFG simplified).
 */

// Clone the given block (and all instructions within). The block is
// assumed to end in a branch (usually CheckType). The new block's
// next and taken edges are set to two newly created blocks, each of
// which jumps to nextJoin/takenJoin blocks, forwarding the SSATmps
// defined in the new block.
Block* cloneBlock(IRUnit& unit,
                  Block& block,
                  Block& pred,
                  Block& nextJoin,
                  Block& takenJoin) {
  auto const newBlock = unit.defBlock(pred.profCount(), pred.hint());

  jit::fast_map<SSATmp*, SSATmp*> rewrites;
  jit::vector<SSATmp*> newDsts;

  // Walk over the block, cloning each instruction and recording the
  // new SSATmps defined.
  for (auto const& inst : block) {
    auto const newInst = [&] {
      if (inst.is(DefLabel)) {
        auto defLabel = unit.defLabel(inst.numDsts(), newBlock, inst.bcctx());

        auto const& predJmp = pred.back();
        assertx(predJmp.is(Jmp));
        assertx(predJmp.numSrcs() == inst.numDsts());

        for (uint32_t i = 0; i < defLabel->numDsts(); ++i) {
          auto const dst = defLabel->dst(i);
          dst->setType(predJmp.src(i)->type());
        }
        return defLabel;
      }
      auto const cloned = unit.clone(&inst);
      newBlock->append(cloned);
      return cloned;
    }();

    // Rewrite any uses of the old SSATmps with the new SSATmps within
    // the same block.
    for (uint32_t i = 0; i < inst.numSrcs(); ++i) {
      auto const src = inst.src(i);
      auto const it = rewrites.find(src);
      if (it == rewrites.end()) continue;
      newInst->setSrc(i, it->second);
    }

    assertx(inst.numDsts() == newInst->numDsts());
    for (uint32_t i = 0; i < inst.numDsts(); ++i) {
      rewrites.emplace(inst.dst(i), newInst->dst(i));
      newDsts.emplace_back(newInst->dst(i));
    }
  }

  // Define the next/taken blocks, and add placeholder Jmps to them.
  auto const next = unit.defBlock(pred.profCount(), pred.hint());
  auto const taken = unit.defBlock(pred.profCount(), pred.hint());
  next->append(unit.gen(Jmp, newBlock->front().bcctx(), &nextJoin));
  taken->append(unit.gen(Jmp, newBlock->front().bcctx(), &takenJoin));

  // Expand the placeholder Jmps with newly defined SSATmps. There
  // should always be at least one, because the block ends in a
  // CheckType which defines a SSATmp.
  assertx(newDsts.size() > 1);
  for (size_t i = 0; i < newDsts.size(); ++i) {
    unit.expandJmp(&next->back(), newDsts[i]);
    if (i != newDsts.size() - 1) {
      unit.expandJmp(&taken->back(), newDsts[i]);
    }
  }

  retypeDests(&newBlock->back(), &unit);
  newBlock->back().setNext(next);
  newBlock->back().setTaken(taken);
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
      if (phis.count(block)) continue;

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
      for (uint32_t i = 0; i < inst.numSrcs(); ++i) {
        auto const it = rewrites.find(inst.src(i));
        if (it == rewrites.end()) continue;
        inst.setSrc(i, it->second);
      }
    }

    // Schedule each successor for processing, unless it's already been
    // visited.
    block->forEachSucc(
      [&] (Block* succ) {
        if (visited.count(succ)) return;
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

    // Record all SSATmps defined within this block (this will include
    // the SSATmps defined by the DefLabel and CheckType).
    jit::vector<SSATmp*> origDefs;
    auto haveFramePtr = false;
    for (auto const& inst : *block) {
      for (uint32_t i = 0; i < inst.numDsts(); ++i) {
        auto const dst = inst.dst(i);
        // vasm doesn't like it if you phi a FramePtr, so avoid that
        if (dst->isA(TFramePtr)) {
          haveFramePtr = true;
          break;
        }
        origDefs.emplace_back(dst);
      }
      if (haveFramePtr) break;
    }
    if (haveFramePtr) continue;
    // We know we have a DefLabel and CheckType, so we need at least
    // two defs.
    assertx(origDefs.size() > 1);

    FTRACE(4, "Hoisting check-type {} in block B{}\n",
           checkType.toString(), block->id());

    // At this point we know we have a block we're going to hoist:

    // Define "join" blocks, for both the next and taken edge. The
    // CheckType will be hoisted above the DefLabel and this we'll
    // have multiple. They'll all (ultimately) branch to these join
    // points, forwarding the SSATmps defined with the block. The join
    // blocks just serve to be a location for the DefLabel, and then
    // jump to the original CheckType's next and taken blocks.
    auto const nextJoin = unit.defBlock(block->profCount(), block->hint());
    auto const takenJoin = unit.defBlock(block->profCount(), block->hint());

    // Set up the DefLabel state in the join blocks. cloneBlock()
    // below will fill them out.
    unit.defLabel(origDefs.size(), nextJoin, checkType.bcctx());
    unit.defLabel(origDefs.size()-1, takenJoin, checkType.bcctx());
    nextJoin->append(unit.gen(Jmp, checkType.bcctx(), checkType.next()));
    takenJoin->append(unit.gen(Jmp, checkType.bcctx(), checkType.taken()));

    // For each predecessor, clone this block, and set that
    // predecessor (and only that predecessor) to jump to the cloned
    // block. Therefore each predecessor then has its own successor,
    // each with its own CheckType.
    block->forEachPred(
      [&] (Block* pred) {
        auto const cloned =
          cloneBlock(unit, *block, *pred, *nextJoin, *takenJoin);
        pred->back().setTaken(cloned);
      }
    );

    // Update the now fully formed DefLabel's dst types
    retypeDests(&nextJoin->front(), &unit);
    retypeDests(&takenJoin->front(), &unit);

    // Record information to rewrite the uses of the old SSATmps
    // defined in the block with their new ones.
    Rewrite rewrite;
    rewrite.nextStart = checkType.next();
    rewrite.takenStart = checkType.taken();
    rewrite.nextJoin = nextJoin;
    rewrite.takenJoin = takenJoin;
    for (uint32_t i = 0; i < origDefs.size(); ++i) {
      rewrite.next.emplace(origDefs[i], nextJoin->front().dst(i));
    }
    // The -1 is because we want to skip the last CheckType for the
    // taken branch
    for (uint32_t i = 0; i < origDefs.size()-1; ++i) {
      rewrite.taken.emplace(origDefs[i], takenJoin->front().dst(i));
    }
    rewrites.emplace_back(std::move(rewrite));

    // The old block is now unreachable, so unlink it from the CFG.
    checkType.setNext(nullptr);
    checkType.setTaken(nullptr);
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

}}
