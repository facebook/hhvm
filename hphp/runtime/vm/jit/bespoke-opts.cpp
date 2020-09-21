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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/tracing.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/dce.h"

#include "hphp/util/trace.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

namespace {

TRACE_SET_MOD(hhir_vanilla);

using BlockSequence = req::TinyVector<Block*, 4>;

// Does this instruction check that its input is vanilla *and nothing else*?
bool isVanillaCheck(const IRInstruction& inst) {
  return inst.is(CheckType) &&
         inst.typeParam().arrSpec().vanilla() &&
         inst.src(0)->type() <= inst.typeParam().unspecialize();
}

// Skip over instructions that don't affect our optimization. We might want
// to add in AssertLoc / AssertSrk / AssertType and pure loads here, later.
IRInstruction* firstNontrivialInstruction(Block* block) {
  for (auto& inst : block->instrs()) {
    if (!inst.is(EndGuards)) return &inst;
  }
  always_assert(false);
}

// If there is a unique edge into `block`, returns a sequence of blocks that
// are "nearly-linear control flow" from that edge. That's a fuzzy term; here
// is a precise list of constraints on the output here:
//
//  - All blocks in this list must be dominated by the input edge.
//
//  - Blocks must appear in RPO order - if one of the blocks in the list is
//    dominated by another, the dominator must come earlier in the list.
//
//  - For all blocks `b` in this list except the last one, the successors
//    of `b` must also be in the list.
//
// If it's not possible to find such a sequence, we'll return an empty list.
// It's totally fine for this sequence to end early - we just might miss some
// optimizations if that happens.
//
BlockSequence getSequence(Block* block) {
  BlockSequence result;
  while (block->preds().size() == 1) {
    result.push_back(block);
    auto const next = block->next();
    auto const taken = block->taken();
    if (!next && !taken) return result;
    if (next) {
      if (taken->taken() || next->preds().size() > 1) return result;
      result.push_back(taken);
    }
    block = next ? next : taken;
  }
  return result;
}

std::string repr(const BlockSequence& blocks) {
  std::string result = "{";
  for (auto const block : blocks) {
    if (result.size() > 1) result += ", ";
    result += folly::sformat("B{}", block->id());
  }
  result += "}";
  return result;
}

void redefineValue(const BlockSequence& blocks, SSATmp* prev, SSATmp* next) {
  assertx(prev->type() == next->type());
  if (prev == next) return;
  for (auto const block : blocks) {
    for (auto& inst : block->instrs()) {
      for (auto i = 0; i < inst.numSrcs(); i++) {
        if (inst.src(i) == prev) inst.setSrc(i, next);
      }
    }
  }
}

void optimizeVanillaCheck(IRUnit& unit, Block* block) {
  static const std::unordered_map<Opcode, Opcode> kReplacements = {
    {CheckDictOffset, CheckDictOffsetLA},
    {CheckVecBounds, CheckVecBoundsLA},
  };

  auto const prev = &block->back();
  if (!isVanillaCheck(*prev) || block->next()->preds().size() != 1) return;
  FTRACE(2, "\nB{}: Block ends in vanilla check.\n", block->id());

  auto const next = firstNontrivialInstruction(block->next());
  auto const replace = kReplacements.find(next->op());
  if (replace == kReplacements.end() || next->src(0) != prev->dst()) return;
  FTRACE(2, "  B{}: Next op is {}.\n", block->next()->id(), next->op());

  if (next->is(CheckVecBounds)) {
    auto const src = next->src(1);
    auto const idx = src->hasConstVal(TInt) ? src->intVal() : -1;
    if (!(0 <= idx && idx <= std::numeric_limits<int32_t>::max())) {
      FTRACE(2, "  Skipping CheckVecBounds without known int key.\n");
      return;
    }
  }

  // The principle difficulty with moving the CheckVanilla op around is
  // redefining the SSATmp that used to be the result of the check. Here is
  // our strategy for doing so:
  //
  //   1. We analyze the CFG to look for "nearly-linear control flow" -
  //      see getSequence above for a precise definition.
  //
  //   2. If either sequence dead-ends, we can redefine the SSATmp on that
  //      side and push the the original SSATmp into the other side.
  //
  //   3. If the two sequences meet at a merge point, and no other control
  //      flow meets at that point, then we can redefine the SSATmp on both
  //      sides and phi the new tmps together to define the original SSATmp.
  //
  //   4. Otherwise, if both sequences end in control flow, we bail.
  //
  auto const ns = getSequence(next->next());
  auto const ts = getSequence(next->taken());
  FTRACE(2, "  Sequences: next: {}, taken: {}\n", repr(ns), repr(ts));
  if (ns.empty() || ts.empty()) return;

  auto const nb = ns.back();
  auto const tb = ts.back();
  if (!nb->taken()) {
    FTRACE(2, "  Next side dead-ends -> REWRITE\n");
  } else if (!tb->taken()) {
    FTRACE(2, "  Taken side dead-ends -> REWRITE\n");
  } else if (!nb->next() && !tb->next() && nb->taken() == tb->taken() &&
             nb->taken()->preds().size() == 2) {
    FTRACE(2, "  Found merge point: B{} -> REWRITE\n", nb->taken()->id());
  } else {
    FTRACE(2, "  No merge point -> FAIL\n");
    return;
  }

  // Create a new block, `tmp`, where we CheckVanilla after the replacement
  // check fails. `tmp` can lead to either prev->taken() or next->taken().
  auto const pt = prev->taken();
  auto const nt = next->taken();
  auto const tmp = unit.defBlock(std::max(pt->profCount(), nt->profCount()));
  tmp->setHint(std::max(pt->hint(), nt->hint()));

  auto const src = prev->src(0);
  auto const dst = prev->dst();
  auto const type = prev->typeParam();
  auto const& bcctx = next->bcctx();
  prev->convertToNop();
  prev->become(unit, unit.gen(Jmp, bcctx, next->block()));
  next->block()->preds().back().setTo(nullptr);
  next->setOpcode(replace->second);
  next->setSrc(0, src);
  next->setTaken(tmp);

  // Redefine `dst` on the next and taken side of the replacement check op.
  // We may reuse `dst` on one of the two sides if the other is terminal.
  // If both are terminal, we'll reuse it on the next side.
  auto const reuse_next = !tb->taken();
  auto const reuse_taken = !nb->taken() && !reuse_next;

  auto const ni = reuse_next
    ? unit.gen(dst, AssertType, bcctx, type, src)
    : unit.gen(AssertType, bcctx, type, src);
  auto const ti = reuse_taken
    ? unit.gen(dst, CheckType, bcctx, type, pt, src)
    : unit.gen(CheckType, bcctx, type, pt, src);

  redefineValue(ns, dst, ni->dst());
  redefineValue(ts, dst, ti->dst());
  ns.front()->prepend(ni);
  tmp->prepend(ti);
  ti->setNext(nt);

  // If neither side is terminal, then we need to redefine `dst` as a phi,
  // which happens at the merge point of the two "nearly-linear" sequences.
  if (nb->taken() && tb->taken()) {
    unit.expandJmp(&nb->back(), ni->dst());
    unit.expandJmp(&tb->back(), ti->dst());
    auto const merge = nb->taken();
    if (merge->front().is(DefLabel)) {
      unit.expandLabel(&merge->front(), 1);
    } else {
      unit.defLabel(1, merge, merge->front().bcctx());
    }
    merge->front().dsts().back() = dst;
    dst->setInstruction(&merge->front(), merge->front().numDsts() - 1);
    assertx(ni->dst()->type() == ti->dst()->type());
    dst->setType(ni->dst()->type());
  }
}

}

void optimizeVanillaChecks(IRUnit& unit) {
  FTRACE(1, "optimizeVanillaChecks:\n");
  for (auto& block : rpoSortCfg(unit)) {
    optimizeVanillaCheck(unit, block);
  }
  FTRACE(1, "\n");
}

//////////////////////////////////////////////////////////////////////

}}
