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
CallUnpack, and CallBuiltin all require catch traces. Many of these instructions
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
the main block that can either be transformed to stack relative uses or adjusted
to use the parent frame pointer (generally with some additional fixup in any
associated catch traces) are also accepted.

All pure memory access and pointer logic can be transformed, in
particular: LdLoc, StLoc, LdLocAddr, CheckLoc, and AssertLoc.

Currently only EagerSyncVMRegs, CallBuiltin, and Call can be adjusted
to use the parent frame.

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

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/dce.h"
#include "hphp/runtime/vm/jit/id-set.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit {
namespace {

TRACE_SET_MOD(pdce_inline);

using SSATmpSet = IdSet<SSATmp>;
using InstructionList = jit::vector<IRInstruction*>;
using InstructionSet = jit::fast_set<IRInstruction*>;
using FPUseMap = jit::fast_map<SSATmp*, InstructionSet>;
using FPMap = jit::fast_map<Block*, SSATmp*>;

struct BlockCmp {
  bool operator()(const Block* a, const Block* b) const {
    if (a->id() == b->id()) return false;
    return a->id() < b->id();
  }
};

using OrderedBlockSet = std::set<Block*, BlockCmp>;

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
  jit::fast_map<SSATmp*, OrderedBlockSet> exitBlocks;
  /*
   * Cache of a SSATmp (defined by a DefLabel), to the "resolved"
   * operands of that DefLabel.
   *
   * NB: This has to be a hash_map because we rely on iterator
   * stability during insertions.
   */
  jit::hash_map<SSATmp*, SSATmpSet> defLabelCache;
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
  SSATmpSet possibleFps;
};

/*
 * The Verify* instructions all read from the ActRec. In particular all can call
 * through to VerifyParamFail, which can read locals from the frame, check the
 * weak types flag on the ActRec; The other two can also check is_callable which
 * may read the context from the frame.
 *
 * InitThrowableFileAndLine reads the current ActRec, and asserts
 * that it's a builtin frame.
 *
 * TODO(#9876771): these should be cleaned up.
 */
bool isDangerousActRecInst(IRInstruction& inst) {
  if (debug && inst.is(InitThrowableFileAndLine)) return true;
  return inst.is(VerifyParamCls, VerifyParamCallable,
                 VerifyParamFail, VerifyRetFail,
                 VerifyReifiedLocalType, VerifyReifiedReturnType);
}

/*
 * Given a FramePtr defined by a DefLabel, return the "resolved"
 * operands of that DefLabel. That is, the set of all SSA tmps defined by
 * a DefFP, DefFuncEntryFP or DefInlineFP (not a DefLabel) that ultimately feed
 * into that DefLabel (perhaps through intermediate DefLabels).
 */
void resolveDefLabel(InlineAnalysis& ia,
                     SSATmpSet& resolved,
                     SSATmp* tmp,
                     SSATmpSet& visited) {
  assertx(tmp->isA(TFramePtr));

  auto const defLabel = tmp->inst();
  assertx(defLabel->is(DefLabel));

  // We can generate self-referential DefLabels because of
  // loops. Ignore that part.
  if (visited[tmp]) return;
  visited.add(tmp);

  auto const dests = defLabel->dsts();
  auto const destIdx =
    std::find(dests.begin(), dests.end(), tmp) - dests.begin();
  assertx(destIdx >= 0 && destIdx < defLabel->numDsts());

  // Visit every operand of this DefLabel. If that comes from a
  // DefLabel, recurse. Otherwise add it to the set.
  defLabel->block()->forEachSrc(
    destIdx,
    [&] (const IRInstruction*, SSATmp* src) {
      if (src->inst()->is(DefLabel)) {
        resolveDefLabel(ia, resolved, src, visited);
        return;
      }
      if (src->inst()->is(DefFP, DefFuncEntryFP, DefInlineFP)) {
        resolved.add(src);
      }
    }
  );
}

/*
 * If 'tmp' is defined by a DefLabel, resolve its operands (using
 * resolveDefLabel) and call 'f' on each operand. Otherwise if 'tmp' is defined
 * by DefFP, DefFuncEntryFP or DefInlineFP, call 'f' with it. This is used to
 * canonicalize a frame pointer to a set of actual non-Phi'd frame pointers.
 */
template <typename F>
void resolve(InlineAnalysis& ia, SSATmp* tmp, F&& f) {
  if (!tmp->isA(TFramePtr)) return;
  if (tmp->inst()->is(DefLabel)) {
    // Utilize the cache to avoid doing redundant walks.
    auto it = ia.defLabelCache.find(tmp);
    if (it == ia.defLabelCache.end()) {
      SSATmpSet visited;
      it = ia.defLabelCache.emplace(tmp, SSATmpSet{}).first;
      resolveDefLabel(ia, it->second, tmp, visited);
    }
    assertx(!it->second.none());
    // NB: It should be okay if the callback calls back into resolve()
    // and mutates the cache because defLabelCache preserves iterators
    // during insertion.
    it->second.forEach(
      [&] (size_t id) { f(ia.unit->findSSATmp(id)); }
    );
    return;
  }
  if (tmp->inst()->is(DefFP, DefFuncEntryFP, DefInlineFP)) f(tmp);
}

/*
 * Like resolve(), but for a set of SSATmps.
 */
template <typename F>
void resolveAll(InlineAnalysis& ia, const SSATmpSet& tmps, F&& f) {
  tmps.forEach(
    [&] (size_t id) {
      resolve(ia, ia.unit->findSSATmp(id), std::forward<F>(f));
    }
  );
};

/*
 * Initialize InlineAnalysis from an IRUnit. Critical edges should be split for
 * the unit before passing it to analyze.
 */
InlineAnalysis analyze(IRUnit& unit) {
  InlineAnalysis ia;
  ia.unit = &unit;

  BlockSet seen;
  std::deque<BlockInfo> workQ = {
    BlockInfo{ unit.entry(), unsigned{0} }
  };
  seen.insert(unit.entry());

  // We need to store the exit information and process it when we're
  // done with the blocks. This is so we can guarantee we've processed
  // every block before trying to resolve.
  jit::vector<std::pair<Block*, SSATmpSet>> exits;

  // Exit blocks are all associated with FPs that are defined on the main trace
  // as part of a DefInlineFP/InlineReturn pair (mainFPs)
  SSATmpSet mainFPs;

  auto const addFPUse = [&] (IRInstruction& inst, SSATmp* use) {
    resolve(
      ia, use,
      [&] (SSATmp* s) {
        auto it = ia.fpUses.find(s);
        if (it != ia.fpUses.end()) it->second.insert(&inst);
      }
    );
  };

  auto const DEBUG_ONLY showSet = [&] (const SSATmpSet& set) {
    auto first = true;
    std::string out;
    set.forEach(
      [&] (uint32_t id) {
        out += folly::sformat(
          "{}{}", first ? "" : ", ",
          *unit.findSSATmp(id)
        );
        first = false;
      }
    );
    return folly::sformat("{{{}}}", out);
  };

  while (!workQ.empty()) {
    auto info = workQ.front();
    workQ.pop_front();

    auto possibleFps = std::move(info.possibleFps);
    auto depth = info.depth;
    for (auto& inst : *info.block) {
      if (inst.is(InlineReturn)) {
        assertx(depth);

        SSATmpSet prevFps;
        resolveAll(
          ia, possibleFps,
          [&] (SSATmp* s) {
            assertx(s->inst()->is(DefInlineFP));
            prevFps.add(s->inst()->src(1));
          }
        );

        depth--;
        possibleFps = std::move(prevFps);
        ia.inlineReturns.push_back(&inst);
        mainFPs.add(inst.src(0));
        ITRACE(2, "Found InlineReturn (depth = {}, fp = {}): {}\n",
               depth, inst, showSet(possibleFps));
      } else if (inst.is(InlineSuspend)) {
        assertx(depth);

        addFPUse(inst, inst.src(0));

        // Note that even though this block isn't necessarily an exit, we treat
        // it as though it were an exit block for this FP, as we will need to
        // sink the DefInlineFP to here.
        SSATmpSet prevFps;
        resolveAll(
          ia, possibleFps,
          [&] (SSATmp* s) {
            assertx(s->inst()->is(DefInlineFP));
            prevFps.add(s->inst()->src(1));
            ia.exitBlocks[s].insert(info.block);
          }
        );

        depth--;
        possibleFps = std::move(prevFps);
        ITRACE(2, "Found InlineSuspend (depth = {}, fp = {}): {}\n",
               depth, inst, showSet(possibleFps));
      } else if (inst.is(DefFP, DefFuncEntryFP)) {
        assertx(!depth && possibleFps.none());
        possibleFps.add(inst.dst());
        ITRACE(3, "Found DefFP/DefFuncEntryFP: {}\n", inst);
      } else if (isDangerousActRecInst(inst)) {
        possibleFps.forEach(
          [&] (size_t id) { addFPUse(inst, unit.findSSATmp(id)); }
        );
      } else if (inst.is(DefInlineFP)) {
        depth++;
        possibleFps = SSATmpSet{inst.dst()};
        ia.fpUses.emplace(inst.dst(), InstructionSet{});
        ia.exitBlocks.emplace(inst.dst(), OrderedBlockSet{});
        addFPUse(inst, inst.src(1));
        ITRACE(2, "Found DefInlineFP (depth = {}): {}\n", depth, inst);
      } else {
        for (auto src : inst.srcs()) addFPUse(inst, src);

        // If this DefLabel defines a FramePtr, we'll set it to the
        // current FP (resolve will peek through the DefLabel when
        // necessary).
        if (inst.is(DefLabel)) {
          auto const numDsts = inst.numDsts();
          for (size_t i = 0; i < numDsts; ++i) {
            auto dst = inst.dst(i);
            if (!dst->isA(TFramePtr)) continue;

            // We consider the dst the frame pointer only if one of
            // the inputs to the DefLabel is a current frame pointer.
            auto const found = inst.block()->findSrc(
              i, [&] (SSATmp* src) { return possibleFps[src]; }
            );
            if (found) {
              possibleFps = SSATmpSet{dst};
              break;
            }
          }
        }
      }
    }

    if (info.block->isExit()) {
      if (depth) {
        exits.emplace_back(info.block, possibleFps);
        ITRACE(2, "Found side exit: BlockId = {}\n", info.block->id());
      }
    }

    if (auto taken = info.block->taken()) {
      auto it = seen.insert(taken);
      if (it.second) workQ.emplace_back(BlockInfo{taken, depth, possibleFps});
    }

    if (auto next = info.block->next()) {
      auto it = seen.insert(next);
      if (it.second) {
        workQ.emplace_back(BlockInfo{next, depth, std::move(possibleFps)});
      }
    }
  }

  // Now that we've visited every block we can process the exits.
  for (auto const& exit : exits) {
    resolveAll(
      ia, exit.second,
      [&] (SSATmp* s) {
        assertx(ia.exitBlocks.count(s));
        ia.exitBlocks[s].insert(exit.first);
      }
    );
  }

  for (auto& exit : ia.exitBlocks) {
    auto fp = exit.first;
    while (!mainFPs[fp] && fp->inst()->is(DefInlineFP)) {
      fp = fp->inst()->src(1);
    }
    if (fp == exit.first || !mainFPs[fp]) continue;
    ia.exitBlocks[fp].insert(exit.second.begin(), exit.second.end());
    exit.second.clear();
  }

  return ia;
}

////////////////////////////////////////////////////////////////////////////////

bool replaceFP(IRInstruction& inst, SSATmp* oldSrc, SSATmp* newSrc) {
  bool replaced = false;
  for (int i = 0; i < inst.numSrcs(); ++i) {
    if (inst.src(i) == oldSrc) {
      ITRACE(5, "Replacing use (oldFp = {}, newFp = {}); {}\n",
             *oldSrc, *newSrc, inst);
      inst.setSrc(i, newSrc);
      replaced = true;
    }
  }
  return replaced;
}

/**
 * Replace uses of DefInlineFP in the block. Returns true iff this block returns
 * from the inlined callee into the caller.
 */
bool replaceFP(Block* block, SSATmp* oldSrc, SSATmp* newSrc, FPUseMap& map) {
  ITRACE(4, "replaceFP(): oldFp = {}, newFp = {}\n", *oldSrc, *newSrc);
  Trace::Indent _i;

  assertx(oldSrc != newSrc);

  for (auto& inst : *block) {
    auto const replaced = replaceFP(inst, oldSrc, newSrc);
    // Intentionally only adjusting the fp here, as the stack offsets should be
    // unchanged in places where we've sunk the DefInlineFP
    if (inst.marker().fp() == oldSrc) {
      inst.marker() = inst.marker().adjustFP(newSrc);
    }
    if (map.count(oldSrc)) map[oldSrc].erase(&inst);
    assertx(!(replaced && inst.is(InlineReturn)));
    if (replaced && inst.is(InlineSuspend)) return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

/*
 * Memory instructions which can be made stack relative if DefInlineFP is pushed
 * past them.
 */
bool canConvertToStack(IRInstruction& inst) {
  if (inst.is(MemoGetStaticCache, MemoSetStaticCache,
              MemoGetLSBCache, MemoSetLSBCache,
              MemoGetInstanceCache, MemoSetInstanceCache)) {
    return inst.src(0)->isA(TFramePtr);
  }
  if (inst.is(StLoc)) {
    auto const id = inst.marker().func()->lookupVarId(s_86metadata.get());
    return inst.extra<StLoc>()->locId != id;
  }
  return inst.is(LdLoc, CheckLoc, AssertLoc, LdLocAddr);
}

/*
 * Instructions which require a FramePtr for chaining but will accept a parent
 * FramePtr. (NB: these instructions will likely require a DefInlineFP in their
 * catch blocks to ensure that the callee is visited by the unwinder).
 */
bool canAdjustFrame(IRInstruction& inst) {
  return inst.is(Call, CallBuiltin, EagerSyncVMRegs);
}

/*
 * While it's possible we pushed the DefInlineFP into the catch trace it
 * will be after the BeginCatch. The marker for BeginCatch must match the
 * marker on inst.
 */
void updateCatchMarker(
    IRInstruction& inst,
    folly::Optional<std::map<uint32_t, std::vector<std::string>>>& traces) {
  if (inst.taken() && inst.taken()->isCatch()) {
    if (traces) {
      (*traces)[inst.block()->id()].push_back(
        folly::sformat("Updating catch marker: {}\n", inst)
      );
    }
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

  replaceFP(inst, oldFp, newFp);
  if (!inst.is(Call)) return;
  /*
   * At various points we may walk the rbp chain to do such things as generate
   * backtraces. Occasionally we look at m_callOff and expect it to refer to
   * an offset in m_sfp->m_func, which won't be the case if we've dropped frames
   * for inlining. To avoid this problem we update the call BC offset to refer
   * to the outer frame and fix it up in the catch trace when we restore the
   * rbp chain.
   */
  auto extra = inst.extra<Call>();
  auto const callOffset = extra->callOffset;
  auto const sk = findCallSK(*oldFp->inst());
  extra->callOffset = sk.offset() - sk.func()->base();

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
      it->extra<SyncReturnBC>()->spOffset ==
        extra->spOffset + extra->numInputs()
    );
    return;
  }

  auto syncInst = unit.gen(
    SyncReturnBC,
    it->bcctx(),
    SyncReturnBCData{callOffset, extra->spOffset + extra->numInputs()},
    inst.src(0),
    def->dst()
  );
  catchBlock->insert(it, syncInst);
}

bool isCallCatch(Block* block) {
  assertx(block->back().is(EndCatch));
  return
    block->back().extra<EndCatch>()->mode == EndCatchData::CatchMode::CallCatch;
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
BlockList findExitHeads(OptimizeContext& ctx, OrderedBlockSet& exits) {
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

void recordNewUse(InlineAnalysis& ia,
                  OptimizeContext& ctx,
                  IRInstruction* inst,
                  SSATmp* use) {
  resolve(
    ia, use,
    [&] (SSATmp* s) {
      auto it = ctx.fpUses->find(s);
      if (it != ctx.fpUses->end()) it->second.insert(inst);
    }
  );
}

bool insertDefInlineFP(InlineAnalysis& ia, OptimizeContext& ctx, Block* block) {
  assertx(block->numPreds() == 1);
  assertx(ctx.deadFp->inst()->is(DefInlineFP));
  auto newDef = ctx.unit->clone(ctx.deadFp->inst());
  if (block->isCatch()) {
    newDef->extra<DefInlineFP>()->syncVmfp = true;
    // Update DefInlineFP's marker to be BeginCatch's marker so that we can
    // later use it to correctly calculate PC
    newDef->marker() = block->catchMarker();
  }
  auto newFp  = newDef->dst();
  block->prepend(newDef);
  auto const inlineExit = replaceFP(block, ctx.deadFp, newFp, *ctx.fpUses);
  ctx.fpMap[block] = newFp;
  recordNewUse(ia, ctx, newDef, newDef->src(1));
  ITRACE(3, "Initializing exit head (block id = {}): {}\n",
         block->id(), *newDef);
  return !inlineExit;
}

/*
 * Update block to use the FP from a dominating block.  If there is no
 * dominating block that defines an FP we need to PHI together the FPs
 * from our predecessors.  In this case if either of our predecessors
 * is unrpocessed we can reschedule to be processed after they get
 * processed.
 */
bool process(OptimizeContext& ctx, const IdomVector& idoms, Block* block) {
  ITRACE(3, "process(): block = {}\n", block->id());
  Trace::Indent _i;

  // We only need to process blocks once.
  if (ctx.fpMap.count(block)) return false;

  auto const rewriteFPs = [&] (SSATmp* fp) {
    always_assert(fp->type() == TFramePtr);
    ctx.fpMap[block] = fp;
    // We only rewrite the FP once, so we know the old fp is ctx.DeadFp.
    return !replaceFP(block, ctx.deadFp, fp, *ctx.fpUses);
  };

  if (block->numPreds() == 1) {
    // Fast path for single predecessor.
    // A single predecessor must dominate the block.
    auto const pred = block->preds().front().from();
    assertx(ctx.fpMap.count(pred));
    auto predFp = ctx.fpMap[pred];
    return rewriteFPs(predFp);
  }

  // Walk the dominators of the block and look for an FP to use.
  for (auto dom = block; dom != nullptr; dom = idoms[dom]) {
    if (ctx.fpMap.count(dom)) {
      return rewriteFPs(ctx.fpMap[dom]);
    }
  }

  // No dominators of the block had a usable FP.  We have to create a phi.
  if (debug) {
    block->forEachPred([&] (Block* pred) {
      // We queue blocks in RPO order.  If a pred is not processed yet something
      // is wrong.  Perhaps a block representing the head of a loop is not
      // dominated by a single FP def.
      assertx(ctx.fpMap.count(pred) || cfgHasLoop(*ctx.unit));
    });
  }

  // Multiple preds define different FPs which will require a DefLabel to phi
  // them in block
  auto const label = [&] {
    if (block->front().is(DefLabel)) {
      // block already has a label but it cannot contain an inlined fp because
      // if it did we would have had to have already processed it (breaking
      // our invariant of processing blocks only once).
      ctx.unit->expandLabel(&block->front(), 1);
      ITRACE(4, "Expanding block DefLabel: {}\n", block->front());
      return &block->front();
    }
    // Don't put DefLabel in a block with a BeginCatch. Splitting critical
    // edges should hoist BeginCatch for us.
    assertx(!block->isCatch());
    auto result = ctx.unit->defLabel(1, block, block->front().bcctx());
    ITRACE(4, "Creating block DefLabel: {}\n", result);
    return result;
  }();

  auto const phiIdx = label->numDsts() - 1;
  auto newFp = label->dst(phiIdx);
  ITRACE(3, "block new new fp: {}\n", *newFp);

  ITRACE(3, "Updating pred jmps\n");
  {
    Trace::Indent _i2;
    block->forEachPred([&] (Block* p) {
      auto& jmp = p->back();
      // There are no critical edges so p must have a single successor.
      assertx(p->numSuccs() == 1);
      assertx(jmp.numSrcs() == phiIdx);

      /*
       * When a block needs a Phi node we to extend jumps from all predecessor
       * blocks with their updated fp values. In cases where the cfg contains a
       * cycle our dataflow analysis may encounter blocks with unprocessed
       * predecessors requiring Phi nodes. In those instances we extend the jump
       * with ctx.deadFp, so that it can be updated when the node is processed.
       */
      auto updateFp = [&] {
        auto const it = ctx.fpMap.find(p);
        if (it != ctx.fpMap.end()) return it->second;
        return ctx.deadFp;
      }();

      ITRACE(4, "Found pred to update (block id: {}): {}\n",
             p->id(), p->back());
      ctx.unit->expandJmp(&jmp, updateFp);
      always_assert(jmp.numSrcs() == phiIdx + 1);
    });
  }
  retypeDests(label, ctx.unit);
  return rewriteFPs(newFp);
}

void transformUses(InlineAnalysis& ia,
                   OptimizeContext& ctx,
                   InstructionSet& uses,
                   bool& reflow) {
  if (!uses.size()) return;

  auto fp = ctx.deadFp;
  auto def = fp->inst();
  auto parentFp = ctx.deadFp->inst()->src(1);

  jit::vector<IRInstruction*> vuses(uses.begin(), uses.end());
  std::sort(vuses.begin(), vuses.end(),
            [] (IRInstruction* a, IRInstruction* b) {
              if (a->block() != b->block()) {
                return a->block()->id() < b->block()->id();
              }
              if (a->id() == b->id()) return false;
              return a->id() < b->id();
            });
  for (auto inst : vuses) {
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
      recordNewUse(ia, ctx, newDef, parentFp);
    } else if (canConvertToStack(*inst)) {
      assertx(ctx.mainBlocks.count(block) != 0);
      ITRACE(3, "Converting to stack relative instruction: {}\n", *inst);
      convertToStackInst(*ctx.unit, *inst);

      // We may have change the types of some pointers
      reflow = true;
    } else if (canAdjustFrame(*inst)) {
      assertx(ctx.mainBlocks.count(block) != 0);
      ITRACE(3, "Using parent frame for instruction: {}\n", *inst);
      adjustFrame(*ctx.unit, *inst, fp, parentFp);
      recordNewUse(ia, ctx, inst, parentFp);
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
  }
  uses.clear();
}

} // namespace

/*
 * Calculate the stack pointer offset corresponding to the given frame pointer.
 *
 * Normally this is straight-forward, but if the parent is DefLabel,
 * we may have do a recursive walk.
 */
folly::Optional<int32_t> findSPOffset(const IRUnit& unit,
                                      const SSATmp* fp,
                                      SSATmpSet& visited) {
  assertx(fp->isA(TFramePtr));
  auto const inst = fp->inst();

  if (inst->is(DefInlineFP)) {
    return inst->extra<DefInlineFP>()->spOffset.offset;
  }
  if (inst->is(DefFP, DefFuncEntryFP)) {
    auto const defSP = unit.mainSP()->inst();
    assertx(defSP->is(DefFrameRelSP, DefRegSP));
    return defSP->extra<FPInvOffsetData>()->offset.offset;
  }

  assertx(inst->is(DefLabel));

  // We can encounter self-referential DefLabels because of loops, so
  // ignore those.
  if (visited[fp]) return folly::none;
  visited.add(fp);

  auto const dests = inst->dsts();
  auto const destIdx =
    std::find(dests.begin(), dests.end(), fp) - dests.begin();
  assertx(destIdx >= 0 && destIdx < inst->numDsts());

  // Right now all inputs to the DefLabel should ultimately have the
  // same offset. So, just recurse down an arbitrary source and use
  // the offset from that. In debug builds, we'll visit all the
  // sources and assert that the offsets are all the same.
  folly::Optional<int32_t> spOff;
  inst->block()->forEachSrc(
    destIdx,
    [&] (const IRInstruction*, const SSATmp* tmp) {
      if (!spOff) {
        spOff = findSPOffset(unit, tmp, visited);
        return;
      }
      if (!debug) return;
      auto const off = findSPOffset(unit, tmp, visited);
      always_assert(!off || *off == *spOff);
    }
  );

  return spOff;
}

namespace {

void adjustBCMarkers(OptimizeContext& ctx) {
  auto fp  = ctx.deadFp;
  auto def = fp->inst();
  auto parentFp = def->src(1);

  auto const spAdjust = [&] {
    assertx(def->is(DefInlineFP));
    auto const curSpOff = def->extra<DefInlineFP>()->spOffset.offset;
    SSATmpSet visited;
    auto const spOff = findSPOffset(*ctx.unit, parentFp, visited);
    assertx(spOff);
    return *spOff - curSpOff;
  }();

  /*
   * We're going to pretend this instruction occured in the caller, so
   * update its marker to use the same SrcKey as the call.
   *
   * This is particularly important for syncing vm regs where we read the
   * marker to eagerly sync vmpc().
   */
  auto const callSK = findCallSK(*def);

  // if we're tracing, use a map to make sure the output is produced in a
  // deterministic order.
  folly::Optional<std::map<uint32_t, std::vector<std::string>>> traces;
  if (Trace::moduleEnabled(Trace::pdce_inline, 4)) traces.emplace();

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
      if (traces) {
        (*traces)[block->id()].push_back(
          folly::sformat(
            "Updating marker (old spOff = {}, new spOff = {}): {}\n",
            inst.marker().spOff().offset, newOff.offset, inst));
      }

      inst.marker() = inst.marker().adjustFP(parentFp)
                                   .adjustSP(newOff)
                                   .adjustFixupSK(callSK);
      updateCatchMarker(inst, traces);
    }
  }

  if (traces) {
    for (auto const& t : *traces) {
      for (auto const DEBUG_ONLY& s : t.second) {
        ITRACE(4, "{}", s);
      }
    }
  }
}

void syncCatchTraces(OptimizeContext& ctx, OrderedBlockSet& exitBlocks) {
  for (auto block : exitBlocks) {
    /*
     * Catch blocks are special: we need to sync vmfp() and vmsp() so that the
     * unwinder knows to free the inlined frame. If the catch block is from a
     * call we don't do this because vmfp()/vmsp() should be further up the
     * stack. These instructions will emit a special SyncReturnBC instead to
     * fixup the call frame to contain the inlined frame.
     *
     * Note: when unwinding from an exception the callee may not be the first
     * AR on the stack, however, with the exception of Call, and CallUnpack, the
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
        IRSPRelOffsetData { endCatch.extra<EndCatch>()->offset },
        endCatch.src(0),
        endCatch.src(1)
      );
      block->insert(--block->end(), sync);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool optimize(InlineAnalysis& env, IRInstruction* inlineReturn, bool& reflow) {
  ITRACE(2, "optimize(): InlineReturn = {}\n", *inlineReturn);
  Trace::Indent _i;

  assertx(inlineReturn->is(InlineReturn));

  auto fp = inlineReturn->src(0);
  auto def = fp->inst();
  if (def->is(DefLabel)) {
    // Removing DefLabel defined FramePtrs will require better analysis
    ITRACE(2, "skipping unsuitable InlineReturn (defined by DefLabel)\n");
    return false;
  }

  assertx(env.fpUses.count(fp));
  assertx(def->is(DefInlineFP));

  OptimizeContext ctx {env.unit, fp, inlineReturn, &env.fpUses};
  ctx.mainBlocks = findMainBlocks(def->block(), inlineReturn->block());

  // Check if this callee is a candidate for DefInlineFP sinking
  auto& uses = env.fpUses[fp];
  auto const hasMainUse = [&](IRInstruction* inst) {
    return ctx.mainBlocks.count(inst->block()) &&
           !canConvertToStack(*inst) &&
           !canAdjustFrame(*inst);
  };
  auto numMainUses = std::count_if(uses.begin(), uses.end(), hasMainUse);
  if (numMainUses > 0) {
    ITRACE(2, "skipping unsuitable InlineReturn (uses = {})\n", numMainUses);
    if (Trace::moduleEnabled(Trace::pdce_inline, 3)) {
      std::vector<std::string> tmp;
      for (auto const inst : uses) {
        if (hasMainUse(inst)) tmp.push_back(inst->toString());
      }
      std::sort(tmp.begin(), tmp.end());
      for (auto const DEBUG_ONLY& s : tmp) ITRACE(3, "  use: {}\n", s);
    }
    return false;
  }

  // We use a dataflow queue based on block RPO id to schedule updating the FP
  // uses.  We do this to ensure predecessors of a block are processed before
  // the block itself.  This makes it easy to handle cases where PHIs of FPs
  // are necessary (we can simply phi whatever FPs were used in the
  // predecessors together).  Furthermore we can identify badly formed input
  // easily when there is no dominating block with an FP available for use, and
  // one of the predecessors has not already been adjusted.
  auto const rpoBlocks = rpoSortCfg(*env.unit);
  auto const rpoIDs = numberBlocks(*env.unit, rpoBlocks);
  dataflow_worklist<uint32_t> incompleteQ(rpoBlocks.size());

  // Update FP's in all blocks reachable from the exit heads
  auto heads = findExitHeads(ctx, env.exitBlocks[fp]);
  for (auto h : heads) {
    // We won't need to create more DefInlineFP's after this, and if they're
    // completely unused in any of these traces DCE will clean it up.
    if (insertDefInlineFP(env, ctx, h)) {
      h->forEachSucc([&] (Block* succ) {
        incompleteQ.push(rpoIDs[succ]);
      });
    }
  }

  auto const idoms = findDominators(*env.unit, rpoBlocks, rpoIDs);
  while (!incompleteQ.empty()) {
    auto block = rpoBlocks[incompleteQ.pop()];
    if (process(ctx, idoms, block)) {
      block->forEachSucc([&] (Block* succ) {
        incompleteQ.push(rpoIDs[succ]);
      });
    }
  }

  // Remaining references to the FP must be from nested DefInlineFP instructions
  // that were already moved off the main execution path
  transformUses(env, ctx, uses, reflow);

  // Update BC markers on the main trace to use the parentFP, parentFP relative
  // offsets, and the call SrcKey
  adjustBCMarkers(ctx);

  // Insert EagerSyncVMRegs at the end of catch traces
  syncCatchTraces(ctx, env.exitBlocks[fp]);

  // We need to reprocess these exits if we end up pushing parentFp into its
  // respective exit traces.
  resolve(
    env, def->src(1),
    [&] (SSATmp* s) {
      if (env.exitBlocks.count(s)) {
        env.exitBlocks[s].insert(
          env.exitBlocks[fp].begin(),
          env.exitBlocks[fp].end()
        );
      }
    }
  );
  env.exitBlocks[fp].clear();

  ITRACE(2, "Done transforming uses.\n");
  return true;
}
}

////////////////////////////////////////////////////////////////////////////////

void optimizeInlineReturns(IRUnit& unit) {
  PassTracer tracer{&unit, Trace::pdce_inline, "optimizeInlineReturns"};
  Timer timer(Timer::partial_dce_DefInlineFP, unit.logEntry().get_pointer());

  ITRACE(1, "optimize_inline_returns()\n");
  ITRACE(2, "splitting critical edges\n");
  splitCriticalEdges(unit);

  bool reflow = false;
  auto ia = analyze(unit);
  for (auto iret : ia.inlineReturns) {
    auto def = iret->src(0)->inst();
    if (!optimize(ia, iret, reflow)) continue;
    assertx(def->is(DefInlineFP));

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
    resolve(
      ia, def->src(1),
      [&] (SSATmp* s) {
        if (!ia.fpUses.count(s)) return;
        ITRACE(2, "Removing use of {} from dead instruction {}\n",
               *s, *def);
        assertx(ia.fpUses[s].count(def));
        ia.fpUses[s].erase(def);
      }
    );

    ITRACE(2, "Replace InlineReturn: {}\n", *iret);
    convertToInlineReturnNoFrame(unit, *iret);

    ITRACE(2, "Removing dead DefInlineFP: {}\n", *def);
    def->block()->erase(def);
  }

  if (reflow) reflowTypes(unit);
}

}}
