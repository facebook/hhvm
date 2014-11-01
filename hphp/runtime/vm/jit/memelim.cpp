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
  BitSet antIn;    // anticipated: live on every possible path from this point
  BitSet antOut;
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

  // We lazily compute the idom vector---we only need it if we have potentially
  // sinkable stores.
  folly::Optional<IdomVector> idoms;

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
  BitSet ant;  // anticipated: live on every possible path from this point
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

bool isPartiallyDead(Local& env, folly::Optional<uint32_t> bit) {
  assert(!isDead(env, bit));
  if (!bit) return false;
  /*
   * If the anticipated bit is clear, the store must not be locally live, so it
   * can potentially move to another block if that's profitable.  (Hopefully a
   * catch or exit.)
   */
  return env.live[*bit] && !env.ant[*bit];
}

/*
 * Search for reasonable placements for a live, but unanticipated store,
 * subject to a constraint (see sinkStore).
 *
 * findPlacements is responsible for finding locations reachable from a block
 * that we could possibly move a store.  It will not attempt to put stores on
 * paths where they are dead, and will call the `constraint' whenever it
 * reaches a block where the store is anticipated.  See more in sinkStore for
 * how this works.
 */
template<class Constraint>
bool findPlacements(Global&,
                    boost::dynamic_bitset<>&,
                    jit::vector<Block*>&,
                    Block*,
                    uint32_t,
                    Constraint);

template<class Constraint>
bool doFindPlacements(Global& genv,
                      boost::dynamic_bitset<>& visited,
                      jit::vector<Block*>& placements,
                      Block* attempt,
                      uint32_t bit,
                      Constraint constraint) {
  if (visited[attempt->id()]) return true;
  visited[attempt->id()] = true;

  if (!genv.blockStates[attempt->id()].liveIn[bit]) {
    // It's dead on this path.
    return true;
  }
  if (genv.blockStates[attempt->id()].antIn[bit]) {
    // It's anticipated here, so this is a reasonable spot to stop moving it on
    // this direction---it's live on all successors.  We still need to check
    // that it's legal to place it here, however.
    if (!constraint(attempt)) {
      FTRACE(5, "        constraint failed: {}\n", attempt->id());
      return false;
    }
    placements.push_back(attempt);
    return true;
  }

  return findPlacements(genv, visited, placements, attempt, bit, constraint);
}

template<class Constraint>
bool findPlacements(Global& genv,
                    boost::dynamic_bitset<>& visited,
                    jit::vector<Block*>& placements,
                    Block* attempt,
                    uint32_t bit,
                    Constraint constraint) {
  if (auto const next = attempt->next()) {
    if (!doFindPlacements(genv, visited, placements, next, bit, constraint)) {
      return false;
    }
  }
  if (auto const taken = attempt->taken()) {
    if (!doFindPlacements(genv, visited, placements, taken, bit, constraint)) {
      return false;
    }
  }
  return true;
}

void sinkStore(Global& genv, uint32_t bit, IRInstruction& inst) {
  FTRACE(4, "      unanticipated\n");

  // This isn't quite done (known incorrectness); don't turn it on yet.
  if (!RuntimeOption::EvalHHIRSinkStores) return;

  /*
   * The findPlacements call will explore reachable blocks from
   * inst.block()---every path must either lead to a block where the store is
   * dead, or a block where the store is anticipated (either because all the
   * successors of that block read it, or that block itself reads it).
   *
   * We however have to impose the following constraints:
   *
   *   o We aren't allowed to move it to a block that isn't dominated by
   *     whichever block computed the value being stored, or we'd violate SSA
   *     rules.
   *
   *   o We can't move it to our own block.  It is legitimate for this algorithm
   *     to move the store earlier---to some predecessor of our own block---but
   *     hosting it within our own block requires more information than we have,
   *     since we're only looking at block-in states.  E.g. we only know if it
   *     is anticipated coming into our block---it might also be read,
   *     redefined, and then read again, or some similar sequence.  (This edge
   *     case is possible, for example, if you have a single block loop both
   *     reading and redefining a local, where the value being stored is loop
   *     invariant so its computation dominates the loop body.)
   *
   * Note that `constraintBlock' will be null if the stored value is a
   * constant---it won't be computed in any block.
   */
  always_assert(inst.op() == StLoc || inst.op() == StLocNT);
  auto const value = inst.src(1);
  auto const constraintBlock = value->inst()->block();
  auto constraint = [&] (const Block* candidate) -> bool {
    if (candidate == inst.block()) {
      // We need to refuse to move it to our own block---see above.
      return false;
    }
    if (!constraintBlock) return true;
    if (!genv.idoms) {
      auto blocks = rpoSortCfgWithIds(genv.unit);
      genv.idoms = findDominators(genv.unit, blocks);
    }
    return dominates(constraintBlock, candidate, *genv.idoms);
  };

  auto placements = jit::vector<Block*>{};
  auto visitedBlocks = boost::dynamic_bitset<>(genv.unit.numBlocks());
  auto const canMove = findPlacements(
    genv,
    visitedBlocks,
    placements,
    inst.block(),
    bit,
    constraint
  );
  if (!canMove) {
    FTRACE(4, "        no movement possible\n");
    return;
  }
  always_assert(!placements.empty());

  // We have a set of locations we can put the guy.  We can only move it to one
  // of them; after that we have to clone.
  auto const srcBlock = inst.block();
  auto it = begin(placements);
  FTRACE(4, "        moving to B{}\n", (*it)->id());
  assert(*it != srcBlock);
  srcBlock->erase(&inst);
  (*it)->prepend(&inst);
  ++it;
  for (auto const stop = end(placements); it != stop; ++it) {
    FTRACE(4, "        cloning to B{}\n", (*it)->id());
    auto const clone = genv.unit.cloneInstruction(&inst);
    assert(*it != srcBlock);
    (*it)->prepend(clone);
  }
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
  env.ant[*bit]  = 1;  // required
}

void addAllGen(Local& env) {
  env.kill.reset();
  env.gen.set();
  env.live.set();
  env.ant.set();  // required
}

void addKill(Local& env, folly::Optional<uint32_t> bit) {
  if (!bit) return;
  FTRACE(4, "      kill: {}\n", *bit);
  env.kill[*bit] = 1;
  env.gen[*bit]  = 0;
  env.live[*bit] = 0;
  env.ant[*bit]  = 0;  // not actually meaningful
}

void killFrame(Local& env, const SSATmp* fp) {
  auto const info = frame_bits(env.global, fp);
  auto const stop = std::min(kMaxLocations, info.startIndex + info.numLocals);
  for (auto bit = info.startIndex; bit < stop; ++bit) {
    addKill(env, bit);
  }
}

//////////////////////////////////////////////////////////////////////

void visit(Local& env, IRInstruction& inst) {
  auto const effects = memory_effects(inst);
  FTRACE(3, "    {: <20} -- {}\n", show(effects), inst.toString());
  match<void>(
    effects,
    [&] (IrrelevantEffects) {},
    [&] (UnknownEffects)    { addAllGen(env); },
    [&] (ReadAllLocals)     { addAllGen(env); },
    [&] (KillFrameLocals l) { killFrame(env, l.fp); },
    [&] (ReadLocal l)       { addGen(env, local_bit(env, l)); },

    [&] (ReadLocal2 l) {
      addGen(env, local_bit(env, ReadLocal { l.fp, l.id1 }));
      addGen(env, local_bit(env, ReadLocal { l.fp, l.id2 }));
    },

    [&] (StoreLocal l) {
      auto bit = local_bit(env, l);
      if (isDead(env, bit)) {
        removeDead(env, inst);
      } else if (isPartiallyDead(env, bit)) {
        sinkStore(env.global, *bit, inst);
      }
      addKill(env, bit);
    },

    /*
     * A StoreLocalNT means it's writing the local's m_data, but not its
     * m_type.  If the local is dead, we don't need to do this.  However, we
     * can't count it as a redefinition (can't add to KILL), because it only
     * partially defines it.
     *
     * Normally this pass should run before we've lowered StLocs into StLocNTs
     * where we can, but we must support this anyway for correctness.
     */
    [&] (StoreLocalNT l) {
      auto bit = local_bit(env, l);
      if (isDead(env, bit)) {
        removeDead(env, inst);
      } else if (isPartiallyDead(env, bit)) {
        sinkStore(env.global, *bit, inst);
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
  env.ant.set();
  block_visit(env, block);
  return BlockAnalysis { env.gen, env.kill };
}

void optimize_block(Global& genv, Block* block) {
  auto& state = genv.blockStates[block->id()];
  FTRACE(1, "Optimize B{}:\n"
            "  liveOut: {}\n"
            "   antOut: {}\n",
            block->id(),
            show(state.liveOut),
            show(state.antOut));
  auto env = Local { genv };
  env.live = state.liveOut;
  env.ant  = state.antOut;
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

  // This isn't required for correctness, but it prevents possibly inserting
  // stores on paths that do not need to compute them.
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
    state.antIn  = transfer.gen | (state.antOut & ~transfer.kill);
    FTRACE(4, "  block B{}\n"
              "    live out: {}\n"
              "     ant out: {}\n"
              "        gen : {}\n"
              "       kill : {}\n"
              "    live in : {}\n"
              "     ant in : {}\n",
              blk->id(),
              show(state.liveOut),
              show(state.antOut),
              show(transfer.gen),
              show(transfer.kill),
              show(state.liveIn),
              show(state.antIn));

    /*
     * Update predecessors:
     *
     *   o Merge the live in state into the live out state of each predecessor.
     *
     *   o Recompute pred anticipated out set as the intersection of each
     *     successor's anticipated in set.
     *
     * If anything changes, reschedule the predecessor.
     */
    blk->forEachPred([&] (Block* pred) {
      FTRACE(4, "   -> {}\n", pred->id());
      auto& predState = genv.blockStates[pred->id()];

      auto const oldLiveOut = predState.liveOut;
      predState.liveOut |= state.liveIn;

      assert(pred->numSuccs() != 0);
      auto const oldAntOut = predState.antOut;
      predState.antOut.set();
      if (auto const n = pred->next()) {
        predState.antOut &= genv.blockStates[n->id()].antIn;
      }
      if (auto const t = pred->taken()) {
        predState.antOut &= genv.blockStates[t->id()].antIn;
      }

      if (predState.liveOut != oldLiveOut || predState.antOut != oldAntOut) {
        incompleteQ.insert(predState.id);
      }
    });
  }

  /*
   * We've reached a fixed point.  Now we can act on this information to remove
   * dead stores, or push partially dead stores to points where they are
   * anticipated.
   */
  FTRACE(2, "\nFixed point:\n{}\n",
    [&]() -> std::string {
      auto ret = std::string{};
      for (auto& blk : poBlockList) {
        folly::format(
          &ret,
          " B{: <3}: {}\n"
          "     : {}\n",
          blk->id(),
          show(genv.blockStates[blk->id()].liveOut),
          show(genv.blockStates[blk->id()].antOut)
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
