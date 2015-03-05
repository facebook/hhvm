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

#include <iterator>
#include <string>

#include <folly/Format.h>
#include <folly/Optional.h>
#include <folly/ScopeGuard.h>

#include "hphp/util/match.h"
#include "hphp/util/trace.h"
#include "hphp/util/dataflow-worklist.h"

#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/alias-analysis.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir_store);

namespace {

//////////////////////////////////////////////////////////////////////

using PostOrderId = uint32_t;

struct BlockState {
  PostOrderId id;
  ALocBits liveIn;
  ALocBits liveOut;
};

// Environment for the whole optimization pass.
struct Global {
  explicit Global(IRUnit& unit)
    : unit(unit)
    , poBlockList(poSortCfg(unit))
    , ainfo(collect_aliases(unit, poBlockList))
    , blockStates(unit, BlockState{})
  {}

  IRUnit& unit;
  BlockList poBlockList;
  AliasAnalysis ainfo;

  // Block states are indexed by block->id().  These are only meaningful after
  // we do dataflow.
  StateVector<Block,BlockState> blockStates;
};

// Block-local environment.
struct Local {
  Global& global;

  ALocBits gen;
  ALocBits kill;
  ALocBits live;
};

//////////////////////////////////////////////////////////////////////

bool isDead(Local& env, folly::Optional<uint32_t> bit) {
  return bit && !env.live[*bit];
}

bool isDeadSet(Local& env, const ALocBits& bits) {
  return (env.live & bits).none();
}

void removeDead(Local& env, IRInstruction& inst) {
  FTRACE(4, "      dead (removed)\n");

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    switch (inst.op()) {
    case StStk:
      env.global.unit.replace(
        &inst,
        DbgTrashStk,
        IRSPOffsetData { inst.extra<StStk>()->offset },
        inst.src(0)
      );
      return;
    case SpillFrame:
      env.global.unit.replace(
        &inst,
        DbgTrashFrame,
        IRSPOffsetData { inst.extra<SpillFrame>()->spOffset },
        inst.src(0)
      );
      return;
    case StMem:
      env.global.unit.replace(&inst, DbgTrashMem, inst.src(0));
      return;
    default:
      break;
    }
  }

  inst.block()->erase(&inst);
}

void addGenSet(Local& env, const ALocBits& bits) {
  FTRACE(4, "      gen:  {}\n", show(bits));
  env.gen |= bits;
  env.kill &= ~bits;
  env.live |= bits;
}

void addAllGen(Local& env) {
  env.kill.reset();
  env.gen.set();
  env.live.set();
}

void addKill(Local& env, folly::Optional<uint32_t> bit) {
  if (!bit) return;
  FTRACE(4, "      kill: {}\n", *bit);
  env.kill[*bit] = 1;
  env.gen[*bit]  = 0;
  env.live[*bit] = 0;
}

void addKillSet(Local& env, const ALocBits& bits) {
  FTRACE(4, "      kill: {}\n", show(bits));
  env.kill |= bits;
  env.gen  &= ~bits;
  env.live &= ~bits;
}

//////////////////////////////////////////////////////////////////////

void load(Local& env, AliasClass acls) {
  addGenSet(env, env.global.ainfo.may_alias(canonicalize(acls)));
}

void kill(Local& env, AliasClass acls) {
  addKillSet(env, env.global.ainfo.must_alias(canonicalize(acls)));
}

folly::Optional<uint32_t> pure_store_bit(Local& env, AliasClass acls) {
  if (auto const meta = env.global.ainfo.find(canonicalize(acls))) {
    return meta->index;
  }
  return folly::none;
}

void visit(Local& env, IRInstruction& inst) {
  auto const effects = memory_effects(inst);
  FTRACE(3, "    {: <30} -- {}\n", show(effects), inst.toString());
  match<void>(
    effects,
    [&] (IrrelevantEffects) {},
    [&] (UnknownEffects)    { addAllGen(env); },
    [&] (PureLoad l)        { load(env, l.src); },
    [&] (GeneralEffects l)  { load(env, l.loads);
                              kill(env, l.kills); },

    [&] (InterpOneEffects m) {
      addAllGen(env);
      kill(env, m.kills);
    },

    [&] (ReturnEffects l) {
      if (inst.is(InlineReturn)) {
        // Returning from an inlined function.  This adds nothing to gen, but
        // kills frame and stack locations for the callee.
        auto const fp = inst.src(0);
        auto const killSet = env.global.ainfo.per_frame_bits[fp];
        addKillSet(env, killSet);
        kill(env, l.kills);
        return;
      }

      // Return from the main function.  Locations other than the frame and
      // stack (e.g. object properties and whatnot) are always live on a
      // function return---so add everything to gen before we start killing
      // things.
      addAllGen(env);
      addKillSet(env, env.global.ainfo.all_frame);
      kill(env, l.kills);
    },

    [&] (ExitEffects l) {
      load(env, l.live);
      kill(env, l.kills);
    },

    /*
     * Call instructions potentially throw, even though we don't (yet) have
     * explicit catch traces for them, which means it counts as possibly
     * reading any local, on any frame---if it enters the unwinder it could
     * read them.
     */
    [&] (CallEffects l) {
      load(env, AHeapAny);
      load(env, AFrameAny);  // Not necessary for some builtin calls, but it
                             // depends which builtin...
      load(env, l.stack);
      kill(env, l.kills);
    },

    [&] (IterEffects l) {
      if (RuntimeOption::EnableArgsInBacktraces) {
        load(env, AFrameAny);
      }
      load(env, AFrame { l.fp, l.id });
      load(env, AHeapAny);
      kill(env, l.kills);
    },
    [&] (IterEffects2 l) {
      if (RuntimeOption::EnableArgsInBacktraces) {
        load(env, AFrameAny);
      }
      load(env, AFrame { l.fp, l.id1 });
      load(env, AFrame { l.fp, l.id2 });
      load(env, AHeapAny);
      kill(env, l.kills);
    },

    [&] (PureStore l) {
      auto bit = pure_store_bit(env, l.dst);
      if (isDead(env, bit)) {
        removeDead(env, inst);
      }
      addKill(env, bit);
    },

    [&] (PureSpillFrame l) {
      auto const it = env.global.ainfo.stack_ranges.find(canonicalize(l.dst));
      if (it == end(env.global.ainfo.stack_ranges)) return;
      // If all the bits corresponding to the stack range are dead, we can
      // eliminate this instruction.  We can also count all of them as
      // redefined.
      if (isDeadSet(env, it->second)) {
        removeDead(env, inst);
      }
      addKillSet(env, it->second);
    },

    /*
     * A PureStoreNT means it's writing the local's m_data, but not its m_type.
     * If the local is dead, we don't need to do this.  However, we can't count
     * it as a redefinition (can't add to KILL), because it only partially
     * defines it.
     *
     * Normally this pass should run before we've lowered StLocs into StLocNTs
     * where we can, but we must support this anyway for correctness.
     */
    [&] (PureStoreNT l) {
      auto bit = pure_store_bit(env, l.dst);
      if (isDead(env, bit)) {
        removeDead(env, inst);
      }
    }
  );
}

void block_visit(Local& env, Block* block) {
  FTRACE(2, "  visiting B{}\n", block->id());
  auto it = block->instrs().end();
  --it;
  for (;;) {
    folly::Optional<Block::iterator> prev;
    if (it != block->instrs().begin()) {
      prev = std::prev(it);
    }
    visit(env, *it);
    if (!prev) break;
    it = *prev;
  }
}

//////////////////////////////////////////////////////////////////////

struct BlockAnalysis { ALocBits gen; ALocBits kill; };

BlockAnalysis analyze_block(Global& genv, Block* block) {
  auto env = Local { genv };
  env.live.set(); // During this first analysis pass, we need to pretend
                  // everything is live on block out (because it might be).  We
                  // may still remove some stores during this visit if they are
                  // locally proven dead, however.
  block_visit(env, block);
  return BlockAnalysis { env.gen, env.kill };
}

void optimize_block(Global& genv, Block* block) {
  auto& state = genv.blockStates[block->id()];
  FTRACE(1, "Optimize B{}:\n"
            "  liveOut: {}\n",
            block->id(),
            show(state.liveOut));
  auto env = Local { genv };
  env.live = state.liveOut;
  block_visit(env, block);
}

//////////////////////////////////////////////////////////////////////

}

void optimizeStores(IRUnit& unit) {
  PassTracer tracer{&unit, Trace::hhir_store, "optimizeStores"};

  // This isn't required for correctness, but it may allow removing stores that
  // otherwise we would leave alone.
  splitCriticalEdges(unit);

  auto incompleteQ = dataflow_worklist<PostOrderId>(unit.numBlocks());

  /*
   * Global state for this pass, visible while processing any block.
   */
  auto genv = Global { unit };
  auto const& poBlockList = genv.poBlockList;
  if (genv.ainfo.locations.size() == 0) {
    FTRACE(1, "no memory accesses to possibly optimize\n");
    return;
  }
  FTRACE(1, "\nLocations:\n{}\n", show(genv.ainfo));

  /*
   * Initialize the block state structures.
   *
   * Important note: every block starts with an empty liveOut set, including
   * blocks that are exiting the region.  When an HHIR region is exited,
   * there's always some instruction we can use to indicate via memory_effects
   * what may be read (e.g. EndCatch, RetCtrl, ReqBindJmp, etc).  When we start
   * iterating, we'll appropriately add things to GEN based on these.
   */
  for (auto poId = uint32_t{0}; poId < poBlockList.size(); ++poId) {
    genv.blockStates[poBlockList[poId]->id()].id = poId;
    incompleteQ.push(poId);
  }

  /*
   * Analyze each block to compute its transfer function.
   *
   * The blockAnalysis vector is indexed by post order id.
   */
  auto const blockAnalysis = [&] () -> jit::vector<BlockAnalysis> {
    auto ret = jit::vector<BlockAnalysis>{};
    ret.reserve(unit.numBlocks());
    for (auto id = uint32_t{0}; id < poBlockList.size(); ++id) {
      ret.push_back(analyze_block(genv, poBlockList[id]));
    }
    return ret;
  }();

  FTRACE(2, "Transfer functions:\n{}\n",
    [&]() -> std::string {
      auto ret = std::string{};
      for (auto poId = uint32_t{0}; poId < poBlockList.size(); ++poId) {
        auto& analysis = blockAnalysis[poId];
        folly::format(
          &ret,
          " B{}\n"
          "   gen:  {}\n"
          "   kill: {}\n",
          poBlockList[poId]->id(),
          show(analysis.gen),
          show(analysis.kill)
        );
      }
      return ret;
    }()
  );

  /*
   * Iterate on the liveOut states until we reach a fixed point.
   */
  FTRACE(4, "Iterating\n");
  while (!incompleteQ.empty()) {
    auto const poId = incompleteQ.pop();
    auto const blk  = poBlockList[poId];

    auto& state         = genv.blockStates[blk->id()];
    auto const transfer = blockAnalysis[poId];
    assert(state.id == poId);

    state.liveIn = transfer.gen | (state.liveOut & ~transfer.kill);
    FTRACE(4, "  block B{}\n"
              "    live out: {}\n"
              "        gen : {}\n"
              "       kill : {}\n"
              "    live in : {}\n",
              blk->id(),
              show(state.liveOut),
              show(transfer.gen),
              show(transfer.kill),
              show(state.liveIn));

    /*
     * Update predecessors by merging the live in state into the live out state
     * of each predecessor.
     *
     * If anything changes, reschedule the predecessor.
     */
    blk->forEachPred([&] (Block* pred) {
      FTRACE(4, "   -> {}\n", pred->id());
      auto& predState = genv.blockStates[pred->id()];

      auto const oldLiveOut = predState.liveOut;
      predState.liveOut |= state.liveIn;

      if (predState.liveOut != oldLiveOut) {
        incompleteQ.push(predState.id);
      }
    });
  }

  /*
   * We've reached a fixed point.  Now we can act on this information to remove
   * dead stores.
   */
  FTRACE(2, "\nFixed point:\n{}\n",
    [&]() -> std::string {
      auto ret = std::string{};
      for (auto& blk : poBlockList) {
        folly::format(
          &ret,
          " B{: <3}: {}\n",
          blk->id(),
          show(genv.blockStates[blk->id()].liveOut)
        );
      }
      return ret;
    }()
  );
  for (auto& block : poBlockList) {
    optimize_block(genv, block);
  }
}

//////////////////////////////////////////////////////////////////////

}}
