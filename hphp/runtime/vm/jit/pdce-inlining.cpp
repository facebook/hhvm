/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

/*
Partial DCE for DefInlineFP/InlineReturn Instructions

This module implements a pass to sink DefInlineFP instructions off the critical
path allowing us to eliminate InlineReturn instructions. These optimizations
differ enough from both DCE and PDCE to require a separate pass.

The primary motivation for this pass is the observation that the only time that
an inlined callee's frame must be completely materialized on the stack is when
exiting from the compilation unit while still inside of the callee. This can
occur during a side exit when we encounter a ReqRetranslate at which point the
translator will need to create a new translation for the callee, or at the end
of a catch trace, where the unwinder will have to walk the callee frame and
inspect its live ActRec.

In particular notice that any instructions which may raise, as well as Call,
CallArray, and CallBuiltin all require catch traces. Many of these instructions
can be found within very simple callees and without such a pass would require
that the callee's frame be stored to the stack and the frame pointer be updated
to reference it.

== Overview

This module performs two passes over the compilation unit. The analyze pass
does not mutate the unit, while the optimize pass iteratively removes nested
frames.

Both passes are idempotent and can be run independent of other modules. As with
other optimizations a mandatory DCE must be performed after the module is run.
The optimize pass requires that critical edges be split prior to processing the
unit.

== Analyze pass

The analyze pass walks the compilation unit collecting a list of potentially
sinkable DefInlineFP/InlineReturn pairs. The pass operates on the assumption
that InlineReturn tN will exist iff tN = DefInlineFP has not been sunk.

In addition to collecting pairs of DefInlineFP/InlineReturn a list of all
terminal blocks associated with a particular target DefInlineFP is collected,
as well as a list of all instructions within the callee that depend on the
frame directly or indirectly.

From this assumption only DefInlineFP's associated with an InlineReturn are
considered, and terminal blocks are associated with the most deeply nested
DefInlineFP that is paired with an InlineReturn.

== Optimize pass

The optimize pass is run on each DefInlineFP/InlineReturn pair identified
during the analyze pass. The pass begins by identifying the main-trace or
callee main blocks and a set of blocks referred to as exit-heads.

=== Callee main blocks

Callee main blocks are those blocks from which the InlineReturn are reachable,
which are dominated by DefInlineFP. The callee main blocks are therefore
identified by walking the graph backwards from the InlineReturn and selecting
each block encountered, terminating when the DefInlineFP is encountered.

=== Side exits

Side exiting blocks are those blocks which are dominated by DefInlineFP but
cannot reach the InlineReturn. From any such block control flow will ultimately
leave the compilation unit while still within the inlined callee (exiting the
callee within the compilation unit will always occur at the InlineReturn).

Notice that any use of an inlined frame not occurring within the callee main
blocks is necessarily on a side exit.

=== Exit heads

Side exit blocks with a predecessor on in a callee main block are defined here
as exit-heads. These are blocks were we will need to introduce a new
DefInlineFP so that the frame is properly setup for the exit trace, and the
frames on the stack are properly linked when leaving the compilation unit.

By splitting critical edges we ensure that it's impossible to have an exit head
that also has a side exiting predecessor. This is important because while we
cannot both define a new frame with DefInlineFP and phi it with an incoming
frame from another side exiting block in the same block.

Notice that any callee main block must have a callee main block successor or be
the terminal callee main block. Also note that any exit head must have a callee
main block predecessor. Therefore, any exit-head with a predecessor side exit
will have at least on critical edge (M2->E2) in this case.

 DefInlineFP
      .
      .
      v
      M1 -> E1
      .     .
      .     .
      v     V
      M2 -> E2 -> T
      .
      .
      v
 InlineReturn

Splitting critical edges allows us to be certain that any exit head will have
only callee main block predecessors, making it safe to add DefInlineFP to the
start of every exit-head.

=== Filter callees

After determining the callee main blocks the optimize pass will process every
use of the callee frame pointer to determine if the use is compatible with
sinking the DefInlineFP into exit-heads. Any use occuring within an exit-block
is accepted as these uses will have access to the sunk frame pointer. Uses on
the main block that can either be transformed to stack relative uses or
adjusted to use the parent frame pointer (generally with some additional fixup
                                          in any associated catch traces) are
also accepted.

All pure memory access and pointer logic can be transformed, in particular:
LdLoc, StLoc, LdLocAddr, CheckLoc, HintLocInner, and AssertLoc.

Currently only EagerSyncVMRegs, CallBuiltin, and Call can be adjusted to use
the parent frame. For calls only those calls which are known to not access the
caller frame can be modified in this manner. All C++ builtins can do this but
many hot functions do not. A whitelist exists for builtin functions which are
safe to call without a valid caller frame.

A special list of instructions known to access the current frame without
explicitly depending on it are also blacklisted.

=== Optimize callees

Callees not rejected by the filter are processed in three phases. First new
DefInlineFP instructions are inserted at exit heads and all exit blocks are
updated to use these new frame pointers, inserting phi nodes where necessary.
Simultaneously, BC markers in these exit blocks are updated to reference the
new FP.

Next, callee main blocks are processed to transform frame relative access into
stack relative access and adjust certain instructions to use the parent frame.

Lastly, BC markers are updated in all callee blocks. Markers in callee main
blocks are modified to reference the parent FP, sp offsets are recomputed
relative to this FP, and the fixup source key is set to the call source key
from the caller. Adjusting the FP and offset is critical to prevent the callee
stack from being smashed when reentering the VM, while changing the fixup
source key is done to preserve invariants in the fixup map.

=== Catch traces

An additional EagerSyncVMRegs is inserted following EndCatch in catch traces to
ensure that the callee frame is visited by the unwinder.
*/

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/dce.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit {
namespace {

TRACE_SET_MOD(pdce_inline);

using InstructionList = std::vector<IRInstruction*>;
using InstructionSet = std::unordered_set<IRInstruction*>;
using FPUseMap = std::unordered_map<SSATmp*, InstructionSet>;
using FPMap = std::unordered_map<Block*, SSATmp*>;

struct InlineAnalysis {
  IRUnit* unit;
  /*
   * This list is ordered so that for nested inlining the most nested return
   * appears first. This is important because the list is processed in order
   * and eliminating a nested inlined return may allow us to eliminate outer
   * layers of nesting.
   */
  InstructionList inlineReturns;
  /*
   * Every reachable use of a particular inlined fp in the unit.
   */
  FPUseMap fpUses;
  /*
   * Map fp -> set, where all blocks in set exit the unit while still within the
   * the inlined region for fp.
   */
  std::unordered_map<SSATmp*, BlockSet> exitBlocks;
};

struct OptimizeContext {
  OptimizeContext(IRUnit* unit, SSATmp* fp, IRInstruction* ret, FPUseMap* uses)
    : unit(unit)
    , deadFp(fp)
    , inlineReturn(ret)
    , fpUses(uses)
  {}

  IRUnit* unit;
  /*
   * The FP from the DefInlineFP we're seeking to remove.
   */
  SSATmp* deadFp;

  /*
   * The InlineReturn instruction that defines this region. Note that while
   * this pass may insert multiple DefInlineFP instructions for a particular
   * inlined region it will never insert new InlineReturn instructions.
   */
  IRInstruction* inlineReturn;

  /*
   * Every reachable use of a particular inlined fp in the unit.
   */
  FPUseMap* fpUses;

  /*
   * All blocks within the inlined callee from which the InlineReturn is
   * reachable. For this inlined region to be a viable target for this
   * optimization the InlineReturn should be the only use of DefInlineFP within
   * these blocks.
   */
  BlockSet mainBlocks;

  /*
   * Tracks the inlined FP currently in use for each block within the inlined
   * region.
   */
  FPMap fpMap;
};

////////////////////////////////////////////////////////////////////////////////

struct BlockInfo {
  Block* block;
  unsigned depth;
  IRInstruction* fpInst;
};

/*
 * These instructions all read from the ActRec. In particular all three can call
 * through to VerifyParamFail, which can read locals from the frame, check the
 * weak types flag on the ActRec; The other two can also check is_callable which
 * may read the context from the frame.
 *
 * TODO(#9876771): these should be cleaned up.
 */
bool isDangerousActRecInst(IRInstruction& inst) {
  return inst.is(VerifyParamCls, VerifyParamCallable,
                 VerifyParamFail, VerifyRetFail);
}

/*
 * Initialize InlineAnalysis from an IRUnit. Critical edges should be split for
 * the unit before passing it to analyze.
 */
InlineAnalysis analyze(IRUnit& unit) {
  InlineAnalysis ia;
  BlockSet seen;
  std::deque<BlockInfo> workQ = {
    BlockInfo{ unit.entry(), unsigned{0}, nullptr }
  };
  seen.insert(unit.entry());

  // Exit blocks are all associated with FPs that are defined on the main trace
  // as part of a DefInlineFP/InlineReturn pair (mainFPs)
  std::unordered_set<SSATmp*> mainFPs;

  auto addFPUse = [&] (IRInstruction& inst, SSATmp* use) {
    auto it = ia.fpUses.find(use);
    if (it != ia.fpUses.end()) it->second.insert(&inst);
  };

  while (!workQ.empty()) {
    auto info = workQ.front();
    workQ.pop_front();

    auto fpInst = info.fpInst;
    auto depth = info.depth;
    for (auto& inst : *info.block) {
      if (inst.is(InlineReturn)) {
        assertx(depth && fpInst->is(DefInlineFP));
        assertx(fpInst == inst.src(0)->inst());
        depth--;
        fpInst = fpInst->src(1)->inst();
        ia.inlineReturns.push_back(&inst);
        mainFPs.insert(inst.src(0));
        ITRACE(2, "Found InlineReturn (depth = {}, fp = {}): {}\n",
               depth, inst, *fpInst->dst());
      } else if (inst.is(DefFP)) {
        assertx(!depth && !fpInst);
        fpInst = &inst;
        ITRACE(3, "Found DefFP: {}\n", inst);
      } else if (isDangerousActRecInst(inst)) {
        addFPUse(inst, fpInst->dst());
      } else if (inst.is(DefInlineFP)) {
        depth++;
        fpInst = &inst;
        ia.fpUses.emplace(inst.dst(), InstructionSet{});
        ia.exitBlocks.emplace(inst.dst(), BlockSet{});
        addFPUse(inst, inst.src(1));
        ITRACE(2, "Found DefInlineFP (depth = {}): {}\n", depth, inst);
      } else {
        for (auto src : inst.srcs()) {
          addFPUse(inst, src);
        }
      }
    }

    if (auto next = info.block->next()) {
      auto it = seen.insert(next);
      if (it.second) workQ.emplace_back(BlockInfo{next, depth, fpInst});
    }

    if (auto taken = info.block->taken()) {
      auto it = seen.insert(taken);
      if (it.second) workQ.emplace_back(BlockInfo{taken, depth, fpInst});
    }

    if (!info.block->isExit()) continue;

    if (depth) {
      assertx(ia.exitBlocks.count(fpInst->dst()));
      ia.exitBlocks[fpInst->dst()].insert(info.block);
      ITRACE(2, "Found side exit: BlockId = {}\n", info.block->id());
    }
  }

  // If an exit is associated with an FP that was already sunk walk up the chain
  // to the first FP that wasn't sunk and associate it there.
  for (auto& exit : ia.exitBlocks) {
    auto fp = exit.first;
    while (!mainFPs.count(fp) && fp->inst()->is(DefInlineFP)) {
      fp = fp->inst()->src(1);
    }
    if (fp == exit.first || !mainFPs.count(fp)) continue;
    ia.exitBlocks[fp].insert(exit.second.begin(), exit.second.end());
    exit.second.clear();
  }

  ia.unit = &unit;
  return ia;
}

////////////////////////////////////////////////////////////////////////////////

void replace(IRInstruction& inst, SSATmp* oldSrc, SSATmp* newSrc) {
  for (int i = 0; i < inst.numSrcs(); ++i) {
    if (inst.src(i) == oldSrc) {
      ITRACE(5, "Replacing use (oldFp = {}, newFp = {}); {}\n",
             *oldSrc, *newSrc, inst);
      inst.setSrc(i, newSrc);
    }
  }
}

void replace(Block* block, SSATmp* oldSrc, SSATmp* newSrc, FPUseMap& map) {
  ITRACE(4, "replace(): oldFp = {}, newFp = {}\n", *oldSrc, *newSrc);
  Trace::Indent _i;

  if (oldSrc == newSrc) return;

  for (auto& inst : *block) {
    replace(inst, oldSrc, newSrc);
    // Intentionally only adjusting the fp here, as the stack offsets should be
    // unchanged in places where we've sunk the DefInlineFP
    if (inst.marker().fp() == oldSrc) {
      inst.marker() = inst.marker().adjustFP(newSrc);
    }
    if (map.count(oldSrc)) map[oldSrc].erase(&inst);
  }
}

////////////////////////////////////////////////////////////////////////////////

/*
 * Memory instructions which can be made stack relative if DefInlineFP is pushed
 * past them.
 */
bool canConvertToStack(IRInstruction& inst) {
  return inst.is(LdLoc, StLoc, CheckLoc, AssertLoc, HintLocInner, LdLocAddr);
}

/*
 * Instructions which require a FramePtr for chaining but will accept a parent
 * FramePtr. (NB: these instructions will likely require a DefInlineFP in their
 * catch blocks to ensure that the callee is visited by the unwinder).
 */
bool canAdjustFrame(IRInstruction& inst) {
  switch (inst.op()) {
  case EagerSyncVMRegs: return true;
  /*
   * Some builtin functions modify locals, read the varenv, or inspect the
   * caller frame (e.g. for the value of m_thisOrClass). If these functions
   * are called we cannot elide the inlined frame.
   *
   * TODO(#9876778): we should be able to support CallArray here as well.
   */
  case CallBuiltin: {
    auto data = inst.extra<CallBuiltin>();
    return !data->destroyLocals && !data->needsCallerFrame;
  }
  case Call: {
    auto data = inst.extra<Call>();
    return !data->destroyLocals && !data->needsCallerFrame;
  }
  default: break;
  }
  return false;
}

/*
 * While it's possible we pushed the DefInlineFP into the catch trace it
 * will be after the BeginCatch. The marker for BeginCatch must match the
 * marker on inst.
 */
void updateCatchMarker(IRInstruction& inst) {
  if (inst.taken() && inst.taken()->isCatch()) {
    ITRACE(4, "Updating catch marker: {}", inst);
    auto it = inst.taken()->skipHeader();
    assertx(it != inst.taken()->begin());
    (--it)->marker() = inst.marker();
  }
}

SrcKey findCallSK(IRInstruction& inst) {
  assertx(inst.is(DefInlineFP));
  auto it = inst.block()->iteratorTo(&inst);
  assertx(it != inst.block()->begin());
  auto& prev = *--it;
  assertx(prev.is(BeginInlining));
  return prev.marker().sk();
}

void adjustFrame(IRUnit& unit,
                 IRInstruction& inst,
                 SSATmp* oldFp,
                 SSATmp* newFp) {
  assertx(canAdjustFrame(inst));

  replace(inst, oldFp, newFp);
  if (!inst.is(Call)) return;
  /*
   * At various points we may walk the rbp chain to do such things as generate
   * backtraces. Occasionally we look at m_soff and expect it to refer to
   * an offset in m_sfp->m_func, which won't be the case if we've dropped frames
   * for inlining. To avoid this problem we update the return BC offset to refer
   * to the outer frame and fix it up in the catch trace when we restore the
   * rbp chain.
   */
  auto extra = inst.extra<Call>();
  auto const after = extra->after;
  auto const sk = findCallSK(*oldFp->inst());
  extra->after = sk.advanced().offset() - sk.func()->base();

  auto catchBlock = inst.taken();
  assertx(catchBlock && catchBlock->isCatch());

  auto it = catchBlock->skipHeader();
  IRInstruction* def = nullptr;
  while (it->is(DefInlineFP)) {
    def = &*(it++);
  }
  assertx(def);
  assertx(it != catchBlock->end());

  /*
   * We've already inserted the requisite sync here, and have reencountered the
   * Call while sinking a parent frame.
   */
  if (it->is(SyncReturnBC)) {
    assertx(
      it->extra<SyncReturnBC>()->spOffset == extra->spOffset + extra->numParams
    );
    return;
  }

  auto syncInst = unit.gen(
    SyncReturnBC,
    it->bcctx(),
    SyncReturnBCData{after, extra->spOffset + extra->numParams},
    inst.src(0),
    def->dst()
  );
  catchBlock->insert(it, syncInst);
}

bool isCallCatch(Block* block) {
  assertx(block->back().is(EndCatch));
  std::deque<Block*> workQ {block};
  while (!workQ.empty()) {
    block = workQ.front();
    workQ.pop_front();
    if (block->isCatch()) break;
    for (auto& pred : block->preds()) {
      workQ.emplace_back(pred.inst()->block());
    }
  }
  for (auto& pred : block->preds()) {
    if (pred.inst()->is(Call, CallArray)) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

/*
 * We define the main trace for a DefInlineFP-InlineReturn pair as the set of
 * blocks on any path from DefInlineFP to InlineReturn.
 */
BlockSet findMainBlocks(Block* enterBlock, Block* exitBlock) {
  BlockSet mainBlocks;
  std::deque<Block*> workQ = {exitBlock};

  mainBlocks.insert(exitBlock);
  if (exitBlock == enterBlock) return mainBlocks;

  mainBlocks.insert(enterBlock);
  while (!workQ.empty()) {
    auto block = workQ.front();
    workQ.pop_front();

    block->forEachPred([&] (Block* p) {
      auto it = mainBlocks.insert(p);
      if (it.second) workQ.push_back(p);
    });
  }

  return mainBlocks;
}

/*
 * Exit heads are non-main-blocks with at least one main-block predecessor. Note
 * that because we split critical edges some special properties apply to these
 * blocks, notably:
 *
 * 1) Must have main-block predecessor
 * 2) Must not be a main-block
 *
 * Because InlineReturn must be reachable from all main-blocks,
 * 3) predecessor must have a main-block successor
 * 4) exit head can only have a single predecessor (the main-block) as the
 *    main-block must have another successor.
 */
BlockList findExitHeads(OptimizeContext& ctx, BlockSet& exits) {
  std::deque<Block*> workQ(exits.begin(), exits.end());
  BlockSet seen;
  seen.insert(exits.begin(), exits.end());

  BlockList heads;
  while (!workQ.empty()) {
    auto block = workQ.front();
    workQ.pop_front();

    bool anyIsMain = false;
    block->forEachPred([&] (Block* p) {
      if (ctx.mainBlocks.count(p)) {
        anyIsMain = true;
        return;
      }
      auto it = seen.insert(p);
      if (it.second) workQ.push_back(p);
    });

    if (anyIsMain) {
      heads.push_back(block);
      ITRACE(3, "Found exit head: BlockId = {}\n", block->id());
    }
  }
  return heads;
}

void recordNewUse(OptimizeContext& ctx, IRInstruction* inst, SSATmp* use) {
  auto it = ctx.fpUses->find(use);
  if (it != ctx.fpUses->end()) it->second.insert(inst);
}

void insertDefInlineFPs(OptimizeContext& ctx, BlockList& heads) {
  for (auto block : heads) {
    assertx(block->numPreds() == 1);
    auto newDef = ctx.unit->clone(ctx.deadFp->inst());
    auto newFp  = newDef->dst();
    block->prepend(newDef);
    replace(block, ctx.deadFp, newFp, *ctx.fpUses);
    ctx.fpMap[block] = newFp;
    recordNewUse(ctx, newDef, newDef->src(1));
    ITRACE(3, "Initializing exit head (block id = {}): {}\n",
           block->id(), *newDef);
  }
}

/*
 * Update block succ to either use the FP from pred, phi the FP from pred with
 * FPs of other predecessors, or define a new FP. This function ensures that if
 * two exit heads have a common successor we will phi their frame pointers.
 */
bool process(OptimizeContext& ctx, Block* pred, Block* succ,
             SSATmp* predOldFp) {
  ITRACE(3, "process(): pred = {}, succ = {}\n", pred->id(), succ->id());
  Trace::Indent _i;

  assertx(ctx.fpMap.count(pred));
  auto predCurFp = ctx.fpMap[pred];
  always_assert(predCurFp->type() == TFramePtr);

  // The map does not contain succ, we have not processed it before
  if (!ctx.fpMap.count(succ)) {
    ctx.fpMap[succ] = predCurFp;
    replace(succ, ctx.deadFp, predCurFp, *ctx.fpUses);
    return true;
  }

  // We processed this succ and its fp is already the same as pred
  auto curFp = ctx.fpMap[succ];
  always_assert(curFp->type() == TFramePtr);
  if (curFp == predCurFp) {
    ITRACE(3, "succ fp matches pred fp, bailing (fp = {})\n", *predCurFp);
    return false;
  }

  // The FP in succ came directly from pred, but pred now has a new FP, simply
  // update all instances of predOldFp with predCurFp
  if (curFp == predOldFp) {
    ctx.fpMap[succ] = predCurFp;
    replace(succ, curFp, predCurFp, *ctx.fpUses);
    return true;
  }

  // Multiple preds define different FPs which will require a DefLabel to phi
  // them in succ
  auto const label = [&] {
    if (curFp->inst()->block() == succ) {
      assertx(curFp->inst()->is(DefLabel));
      ITRACE(4, "succ has DefLabel: {}\n", *curFp->inst());
      always_assert(succ->front().dst(succ->front().numDsts() - 1) == curFp);
      return &succ->front();
    } else if (succ->front().is(DefLabel)) {
      // succ already has a label but it cannot contain an inlined fp because
      // if it did we would have stored it in the fpMap
      ctx.unit->expandLabel(&succ->front(), 1);
      ITRACE(4, "Expanding succ DefLabel: {}\n", succ->front());
      return &succ->front();
    }
    // Don't put DefLabel in a block with a BeginCatch. Splitting critical
    // edges should hoist BeginCatch for us.
    assertx(!succ->isCatch());
    auto ret = ctx.unit->defLabel(1, succ->front().bcctx());
    succ->insert(succ->begin(), ret);
    ITRACE(4, "Creating succ DefLabel: {}\n", succ->front());
    return ret;
  }();

  auto const phiIdx = label->numDsts() - 1;
  auto newFp = label->dst(phiIdx);
  ctx.fpMap[succ] = newFp;

  ITRACE(3, "pred fp: {}, succ old fp: {}, succ new fp: {}\n",
         *predCurFp, *curFp, *newFp);

  replace(succ, curFp, newFp, *ctx.fpUses);

  ITRACE(3, "Updating pred jmps\n");
  {
    Trace::Indent _i;
    succ->forEachPred([&] (Block* p) {
      auto& jmp = p->back();
      auto updateFp = ctx.fpMap.count(p) ? ctx.fpMap[p] : ctx.deadFp;

      if (jmp.numSrcs() > phiIdx) {
        always_assert(jmp.src(phiIdx) == updateFp);
        return;
      }

      ITRACE(4, "Found pred to update (block id: {}): {}\n",
             p->id(), p->back());
      ctx.unit->expandJmp(&jmp, updateFp);
      always_assert(jmp.numSrcs() == phiIdx + 1);
    });
  }

  retypeDests(label, ctx.unit);

  // If we didn't change the fp for this block we don't need to re-process it's
  // successors.
  ITRACE(3, "succ fp was {}\n", curFp != newFp ? "changed" : "unchanged");
  return curFp != newFp;
}

void transformUses(OptimizeContext& ctx, InstructionSet& uses) {
  auto fp = ctx.deadFp;
  auto def = fp->inst();
  auto parentFp = ctx.deadFp->inst()->src(1);

  auto it = uses.begin();
  while (it != uses.end()) {
    auto inst = *it;
    auto block = inst->block();
    /*
     * We may have previously sunk a child DefInlineFP into an exit block, so
     * make sure to sink the parent here.
     */
    if (inst->is(DefInlineFP)) {
      assertx(ctx.mainBlocks.count(block) == 0);
      auto newDef = ctx.unit->clone(def);
      auto newFp  = newDef->dst();
      block->prepend(newDef);

      ITRACE(3, "Updating child DefInlineFP (fp = {}): {}\n", *newFp, *inst);
      inst->setSrc(1, newFp);
      recordNewUse(ctx, newDef, parentFp);
    } else if (canConvertToStack(*inst)) {
      assertx(ctx.mainBlocks.count(block) != 0);
      ITRACE(3, "Converting to stack relative instruction: {}\n", *inst);
      convertToStackInst(*ctx.unit, *inst);
    } else if (canAdjustFrame(*inst)) {
      assertx(ctx.mainBlocks.count(block) != 0);
      ITRACE(3, "Using parent frame for instruction: {}\n", *inst);
      adjustFrame(*ctx.unit, *inst, fp, parentFp);
      recordNewUse(ctx, inst, parentFp);
    } else if (isDangerousActRecInst(*inst)) {
      // It's okay to have these in side exits
      always_assert(ctx.mainBlocks.count(block) == 0);
    } else {
      /*
       * All uses that weren't dealt with in side exiting traces need to be
       * handled specially.
       */
      always_assert(false);
    }
    it = uses.erase(it);
  }
}

void adjustBCMarkers(OptimizeContext& ctx) {
  auto fp  = ctx.deadFp;
  auto def = fp->inst();
  auto parentFp = def->src(1);
  auto parent   = parentFp->inst();

  /*
   * We're going to pretend this instruction occured in the caller, so
   * update its marker to use the same SrcKey as the call.
   *
   * This is particularly important for syncing vm regs where we read the
   * marker to eagerly sync vmpc().
   */
  auto const callSK = findCallSK(*def);
  auto const curSpOff = def->extra<DefInlineFP>()->spOffset.offset;
  int32_t spAdjust;
  if (parent->is(DefInlineFP)) {
    auto parentSpOff = parent->extra<DefInlineFP>()->spOffset.offset;
    spAdjust = parentSpOff - curSpOff;
  } else {
    assertx(parent->is(DefFP));
    auto spOff = ctx.unit->mainSP()->inst()->extra<DefSP>()->offset.offset;
    spAdjust = spOff - curSpOff;
  }

  // We need to fix up the main trace-- if a function can reenter we need to be
  // sure that we've adjusted its BC marker so that it doesn't stomp on the
  // stack
  for (auto block : ctx.mainBlocks) {
    for (auto& inst : *block) {
      /*
       * Don't replace markers that are still valid.
       *
       * It may seem like it isn't possible to have markers in the mainBlock
       * that don't belong to fp since we must have already elided any other
       * DefInlineFP's in these blocks or they would prevent us from eliding
       * fp. Notice that the block containing InlineReturn may have other
       * instructions whose marker does not match fp as InlineReturn is
       * non-terminal.
       */
      if (inst.marker().fp() != fp) continue;
      /*
       * The arithmetic here has to be correct-- some of these instructions may
       * throw (we may have pushed the DefInlineFP into their catch blocks), so
       * there can't be unexpected gaps in the stack while unwinding.
       *
       * Update the SrcKeys used in fixup maps to be relative to the Func* in
       * the live ActRec.
       */
      auto newOff = inst.marker().spOff() + spAdjust;
      ITRACE(4, "Updating marker (old spOff = {}, new spOff = {}): {}\n",
             inst.marker().spOff().offset, newOff.offset, inst);

      inst.marker() = inst.marker().adjustFP(parentFp)
                                   .adjustSP(newOff)
                                   .adjustFixupSK(callSK);
      updateCatchMarker(inst);
    }
  }
}

void syncCatchTraces(OptimizeContext& ctx, BlockSet& exitBlocks) {
  for (auto block : exitBlocks) {
    /*
     * Catch blocks are special: we need to sync vmfp() and vmsp() so that the
     * unwinder knows to free the inlined frame. If the catch block is from a
     * call we don't do this because vmfp()/vmsp() should be further up the
     * stack. These instructions will emit a special SyncReturnBC instead to
     * fixup the call frame to contain the inlined frame.
     *
     * Note: when unwinding from an exception the callee may not be the first
     * AR on the stack, however, with the exception of Call, and CallArray, the
     * next frame will always be native. This means that the callee will always
     * be the first frame in its nested VM. The unwinder will unwind through
     * VMs nested under this one first, making it safe to assume that we can
     * sync the VM registers here.
     */
    if (block->back().is(EndCatch) && !isCallCatch(block)) {
      auto& endCatch = block->back();
      auto sync = ctx.unit->gen(
        EagerSyncVMRegs,
        endCatch.bcctx(),
        *endCatch.extra<EndCatch>(),
        endCatch.src(0),
        endCatch.src(1)
      );
      block->insert(--block->end(), sync);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool optimize(InlineAnalysis& env, IRInstruction* inlineReturn) {
  ITRACE(2, "optimize(): InlineReturn = {}\n", *inlineReturn);
  Trace::Indent _i;

  assertx(inlineReturn->is(InlineReturn));

  auto fp = inlineReturn->src(0);
  assertx(env.fpUses.count(fp));

  auto def = fp->inst();
  assertx(def->is(DefInlineFP));

  OptimizeContext ctx {env.unit, fp, inlineReturn, &env.fpUses};
  ctx.mainBlocks = findMainBlocks(def->block(), inlineReturn->block());

  // Check if this callee is a candidate for DefInlineFP sinking
  auto& uses = env.fpUses[fp];
  auto hasMainUse = std::any_of(
    uses.begin(),
    uses.end(),
    [&] (IRInstruction* inst) { return ctx.mainBlocks.count(inst->block()) &&
                                       !canConvertToStack(*inst) &&
                                       !canAdjustFrame(*inst); }
  );
  if (hasMainUse) {
    ITRACE(2, "skipping unsuitable InlineReturn (uses = {})\n", uses.size());
    return false;
  }

  auto heads = findExitHeads(ctx, env.exitBlocks[fp]);

  // We won't need to create more DefInlineFP's after this, and if they're
  // completely unused in any of these traces DCE will clean it up.
  insertDefInlineFPs(ctx, heads);

  // Update FP's in all blocks reachable from the exit heads
  std::deque<std::pair<Block*,SSATmp*>> workQ;
  for (auto h : heads) {
    workQ.push_back(std::make_pair(h, ctx.deadFp));
  }

  while (!workQ.empty()) {
    auto info = workQ.front();
    auto block = info.first;
    auto oldFp = info.second;
    workQ.pop_front();
    block->forEachSucc([&] (Block* succ) {
      auto prev = ctx.fpMap.count(succ) ? ctx.fpMap[succ] : ctx.deadFp;
      if (process(ctx, block, succ, oldFp)) {
        workQ.push_back(std::make_pair(succ, prev));
      }
    });
  }

  // Remaining references to the FP must be from nested DefInlineFP instructions
  // that were already moved off the main execution path
  transformUses(ctx, uses);

  // Update BC markers on the main trace to use the parentFP, parentFP relative
  // offsets, and the call SrcKey
  adjustBCMarkers(ctx);

  // Insert EagerSyncVMRegs at the end of catch traces
  syncCatchTraces(ctx, env.exitBlocks[fp]);

  // We need to reprocess these exits if we end up pushing parentFp into its
  // respective exit traces.
  auto const parentFp = def->src(1);
  if (env.exitBlocks.count(parentFp)) {
    env.exitBlocks[parentFp].insert(
      env.exitBlocks[fp].begin(),
      env.exitBlocks[fp].end()
    );
  }
  env.exitBlocks[fp].clear();

  ITRACE(2, "Done transforming uses.\n");
  return true;
}
}

////////////////////////////////////////////////////////////////////////////////

void optimizeInlineReturns(IRUnit& unit) {
  Timer timer(Timer::partial_dce_DefInlineFP, unit.logEntry().get_pointer());

  ITRACE(1, "optimize_inline_returns()\n");
  ITRACE(2, "splitting critical edges\n");
  splitCriticalEdges(unit);

  auto ia = analyze(unit);
  for (auto iret : ia.inlineReturns) {
    auto def = iret->src(0)->inst();
    if (!optimize(ia, iret)) continue;

    /*
     * Killing the fp if there are still uses would be --bad--
     */
    always_assert(ia.fpUses[def->dst()].size() == 0);

    /*
     * If we're in a nested inlined call and we just eliminated a DefInlineFP
     * we can eliminate it from the use map which may allow us to optimize its
     * parent.
     *
     * Note that order is important here, we process calls from most to least
     * nested so that we can expose as many such opportunities as possible.
     */
    if (ia.fpUses.count(def->src(1))) {
      ITRACE(2, "Removing use of {} from dead instruction {}\n",
             *def->src(1), *def);
      assertx(ia.fpUses[def->src(1)].count(def));
      ia.fpUses[def->src(1)].erase(def);
    }

    ITRACE(2, "Replace InlineReturn: {}\n", *iret);
    convertToInlineReturnNoFrame(unit, *iret);

    ITRACE(2, "Removing dead DefInlineFP: {}\n", *def);
    def->block()->erase(def);
  }
}

}}
