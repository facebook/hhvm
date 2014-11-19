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

#include <bitset>
#include <iterator>
#include <sstream>
#include <string>

#include <boost/variant.hpp>

#include <folly/Format.h>
#include <folly/Optional.h>
#include <folly/ScopeGuard.h>

#include "hphp/util/match.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir_meme);

namespace {

//////////////////////////////////////////////////////////////////////

constexpr auto kMaxLocations = 128u;
using BitSet = std::bitset<kMaxLocations>;

std::string show(BitSet set) {
  std::ostringstream out;
  out << set;
  return out.str();
}

//////////////////////////////////////////////////////////////////////

using PostOrderId = uint32_t;

struct BlockState {
  PostOrderId id;
  BitSet liveIn;
  BitSet liveOut;
};

struct FrameBits { uint32_t numLocals; uint32_t startIndex; };

// Environment for the whole optimization pass.
struct Global {
  explicit Global(IRUnit& unit)
    : unit(unit)
    , blockStates(unit, BlockState{})
    , mainFp{unit.entry()->begin()->dst()}
    , numMainFpLocals(unit.entry()->begin()->marker().func()->numLocals())
    , allocatedBits{numMainFpLocals}
  {}

  IRUnit& unit;

  // Block states are indexed by block->id().  These are only meaningful after
  // we do dataflow.
  StateVector<Block,BlockState> blockStates;

  // Keep the main FP---any locals on the main frame are 'pre-allocated' the
  // first N location bits.
  const SSATmp* const mainFp;
  uint32_t const numMainFpLocals;
  uint32_t allocatedBits;

  // Map from frames to the number of local bits they've got allocated, and
  // what offset they start at.  The main frame is special (doesn't use this
  // map).
  jit::flat_map<const SSATmp*,folly::Optional<FrameBits>> frameBitsMap;
};

// Block-local environment.
struct Local {
  Global& global;

  BitSet gen;
  BitSet kill;
  BitSet live;
};

//////////////////////////////////////////////////////////////////////

/*
 * If the local doesn't live on the main frame, we can dynamically allocate a
 * bit to represent it here during the initial analysis pass that computes
 * per-block transfer functions.  Currently this is only possible for frames
 * produced by DefInlineFP.
 *
 * The reason this works for locals is that when we deal with memory effects
 * that affect all tracked locations, we flip the entire gen or kill set bits
 * instead of just the ones we have already allocated.  This means transfer
 * functions that were computed before all these bits are allocated will still
 * be correct.
 */
FrameBits frame_bits(Global& genv, const SSATmp* fp) {
  if (fp == genv.mainFp) {
    return FrameBits { genv.numMainFpLocals, 0 };
  }

  auto& info = genv.frameBitsMap[fp];
  if (!info) {
    auto const func = fp->inst()->extra<DefInlineFP>()->target;
    info = FrameBits { static_cast<uint32_t>(func->numLocals()),
                       genv.allocatedBits };
    genv.allocatedBits += func->numLocals();
    FTRACE(2, "      alloc [{}, {}) for {}\n",
      info->startIndex,
      info->startIndex + info->numLocals,
      fp->toString());
  }

  return *info;
}

template<class LocalInfo>
folly::Optional<uint32_t> local_bit(Local& env, LocalInfo li) {
  auto const info  = frame_bits(env.global, li.fp);
  auto const bitId = info.startIndex + li.id;
  if (bitId >= kMaxLocations) return folly::none;
  return bitId;
}

//////////////////////////////////////////////////////////////////////

bool isDead(Local& env, folly::Optional<uint32_t> bit) {
  return bit && !env.live[*bit];
}

void removeDead(Local& env, IRInstruction& inst) {
  FTRACE(4, "      dead (removed)\n");
  inst.block()->erase(&inst);
}

void addGen(Local& env, folly::Optional<uint32_t> bit) {
  if (!bit) return;
  FTRACE(4, "      gen:  {}\n", *bit);
  env.gen[*bit]  = 1;
  env.kill[*bit] = 0;
  env.live[*bit] = 1;
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

void killFrame(Local& env, const SSATmp* fp) {
  auto const info = frame_bits(env.global, fp);
  auto const stop = std::min(kMaxLocations, info.startIndex + info.numLocals);
  for (auto bit = info.startIndex; bit < stop; ++bit) {
    addKill(env, bit);
  }
}

//////////////////////////////////////////////////////////////////////

void visitLoad(Local& env, ALocation loc) {
  if (auto const fr = loc.frame()) return addGen(env, local_bit(env, *fr));
  if (loc.maybe(AFrameAny))        return addAllGen(env);
}

folly::Optional<uint32_t> pure_store_bit(Local& env, ALocation loc) {
  if (auto const fr = loc.frame()) {
    return local_bit(env, *fr);
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
    [&] (InterpOneEffects)  { addAllGen(env); },
    [&] (PureLoad l)        { visitLoad(env, l.loc); },
    [&] (MayLoadStore l)    { visitLoad(env, l.loads); },
    [&] (KillFrameLocals l) { killFrame(env, l.fp); },
    [&] (ReturnEffects)     { killFrame(env, env.global.mainFp); },

    /*
     * Call instructions potentially throw, even though we don't (yet) have
     * explicit catch traces for them, which means it counts as possibly
     * reading any local, on any frame---if it enters the unwinder it could
     * read them.
     */
    [&] (CallEffects) { addAllGen(env); },

    // Iterator effects can possibly redefine the local, but don't definitely
    // do so, so they add to GEN but not KILL.
    [&] (IterEffects l) {
      addGen(env, local_bit(env, AFrame { l.fp, l.id }));
    },
    [&] (IterEffects2 l) {
      addGen(env, local_bit(env, AFrame { l.fp, l.id1 }));
      addGen(env, local_bit(env, AFrame { l.fp, l.id2 }));
    },

    [&] (PureStore l) {
      auto bit = pure_store_bit(env, l.loc);
      if (isDead(env, bit)) {
        removeDead(env, inst);
      }
      addKill(env, bit);
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
      auto bit = pure_store_bit(env, l.loc);
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

struct BlockAnalysis { BitSet gen; BitSet kill; };

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

void optimizeMemory(IRUnit& unit) {
  if (RuntimeOption::EnableArgsInBacktraces) {
    // We don't run this pass if this is enabled, because it could omit stores
    // to argument locals.
    return;
  }

  FTRACE(1, "optimizeMemory:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(1, "optimizeMemory:^^^^^^^^^^^^^^^^^^^^\n"); };

  // This isn't required for correctness, but it may allow removing stores that
  // otherwise we would leave alone.
  splitCriticalEdges(unit);

  auto incompleteQ = std::set<PostOrderId>{};

  /*
   * Global state for this pass, visible while processing any block.
   */
  auto genv = Global { unit };

  /*
   * Initialize the block state structures.
   */
  auto const poBlockList = poSortCfg(unit);
  for (auto poId = uint32_t{0}; poId < poBlockList.size(); ++poId) {
    genv.blockStates[poBlockList[poId]->id()].id = poId;
    incompleteQ.insert(poId);
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
    auto const poId = *begin(incompleteQ);
    auto const blk  = poBlockList[poId];
    incompleteQ.erase(begin(incompleteQ));

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
        incompleteQ.insert(predState.id);
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
