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

#include <cstdint>
#include <utility>
#include <tuple>

#include <folly/ScopeGuard.h>

#include "hphp/util/match.h"
#include "hphp/util/trace.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/alias-analysis.h"
#include "hphp/runtime/vm/jit/mutation.h"

namespace HPHP { namespace jit {

namespace {

TRACE_SET_MOD(hhir_load);

//////////////////////////////////////////////////////////////////////

// Reverse post order block ids.
using RpoId = uint32_t;

//////////////////////////////////////////////////////////////////////

struct TrackedLoc {
  /*
   * If we have a known value in a location, this will be non-null and point to
   * it.
   */
  SSATmp* knownValue{nullptr};

  /*
   * We may have a known type even without an available value, or that is more
   * refined than knownValue->type().  Always at least as refined as
   * knownValue->type(), if knownValue is not nullptr.
   */
  Type knownType{Type::Top};
};

/*
 * Abstract state for a program position.
 *
 * This has a set of TrackedLocs, and an availability mask to determine which
 * ones are currently valid.  When updating a tracked location, the visitor
 * below will kill any potentially conflicting locations in the availability
 * mask without updating them.
 */
struct State {
  bool initialized = false;  // if false, we never reached this block

  /*
   * Currently tracked locations, indexed by their ids.
   */
  jit::vector<TrackedLoc> tracked;

  /*
   * Currently available indexes in tracked.
   */
  ALocBits avail;
};

struct BlockInfo {
  RpoId rpoId;
  State stateIn;
};

struct Global {
  explicit Global(IRUnit& unit, BlocksWithIds&& sortedBlocks)
    : unit(unit)
    , idoms(findDominators(unit, sortedBlocks))
    , rpoBlocks(std::move(sortedBlocks.blocks))
    , ainfo(collect_aliases(unit, rpoBlocks))
    , blockInfo(unit, BlockInfo{})
  {}

  IRUnit& unit;
  IdomVector idoms;
  BlockList rpoBlocks;
  AliasAnalysis ainfo;

  /*
   * Map from each block to its information.  Before we've done the fixed point
   * analysis the states in the info structures are not necessarily meaningful.
   */
  StateVector<Block,BlockInfo> blockInfo;
};

//////////////////////////////////////////////////////////////////////

using HPHP::jit::show;

std::string show(TrackedLoc li) {
  return folly::sformat("{} :: {}",
    li.knownValue ? li.knownValue->toString() : std::string("<>"),
    li.knownType.toString()
  );
}

std::string show(const State& state) {
  auto ret = std::string{};
  if (!state.initialized) {
    ret = "  unreachable\n";
    return ret;
  }

  folly::format(&ret, "  av: {}\n", show(state.avail));
  for (auto idx = uint32_t{0}; idx < state.tracked.size(); ++idx) {
    if (state.avail.test(idx)) {
      folly::format(&ret, "  {: >3} = {}\n", idx, show(state.tracked[idx]));
    }
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////

struct Local {
  Global& global;
  State state;
};

struct Flags {
  /*
   * If set, the instruction was a pure load and the value was known to be
   * available in `replaceable'.
   */
  SSATmp* replaceable{nullptr};

  /*
   * If set, the instruction was a pure load, and `replaceable' is not nullptr,
   * this will be the best type we know for the value at this point.
   */
  Type knownType;

  /*
   * If true, the instruction was a conditional jump that can be converted to
   * an unconditional jump to it's next() edge.
   */
  bool convertToJmp{false};
};

//////////////////////////////////////////////////////////////////////

void clear_everything(Local& env) {
  FTRACE(3, "      clear_everything\n");
  env.state.avail.reset();
}

TrackedLoc* find_tracked(Local& env,
                         const IRInstruction& inst,
                         AliasClass acls) {
  auto const meta = env.global.ainfo.find(canonicalize(acls));
  if (!meta) return nullptr;
  assert(meta->index < kMaxTrackedALocs);
  return env.state.avail[meta->index] ? &env.state.tracked[meta->index]
                                      : nullptr;
}

std::pair<SSATmp*,Type> load(Local& env,
                             const IRInstruction& inst,
                             AliasClass acls) {
  acls = canonicalize(acls);
  auto const meta = env.global.ainfo.find(acls);
  if (!meta) return { nullptr, Type::Gen };
  assert(meta->index < kMaxTrackedALocs);
  auto& tracked = env.state.tracked[meta->index];

  if (env.state.avail[meta->index]) {
    if (tracked.knownValue) return { tracked.knownValue, tracked.knownType };
    /*
     * We didn't have a value, but we had an available type.  This can happen
     * at control flow joins right now (since we don't have support for
     * inserting phis for available values).  Whatever the old type is is still
     * valid, and whatever information the load instruction knows is also
     * valid, so we can keep their intersection.
     */
    tracked.knownType &= inst.dst()->type();
  } else {
    tracked.knownType = inst.dst()->type();
    env.state.avail.set(meta->index);
  }
  tracked.knownValue = inst.dst();

  FTRACE(4, "       {} <- {}\n", show(acls), inst.dst()->toString());
  FTRACE(5, "       av: {}\n", show(env.state.avail));
  return { nullptr, Type::Gen };
}

void store(Local& env, AliasClass acls, SSATmp* value) {
  acls = canonicalize(acls);

  auto const meta = env.global.ainfo.find(acls);
  if (!meta) {
    env.state.avail &= ~env.global.ainfo.may_alias(acls);
    return;
  }

  FTRACE(5, "       av: {}\n", show(env.state.avail));
  env.state.avail &= ~meta->conflicts;

  auto& current = env.state.tracked[meta->index];
  current.knownValue = value;
  current.knownType = value ? value->type() : Type::Top;
  env.state.avail.set(meta->index);
  FTRACE(4, "       {} <- {}\n", show(acls),
    value ? value->toString() : std::string("<>"));
  FTRACE(5, "       av: {}\n", show(env.state.avail));
}

void refine_value(Local& env, SSATmp* newVal, SSATmp* oldVal) {
  for (auto i = uint32_t{0}; i < kMaxTrackedALocs; ++i) {
    if (!env.state.avail[i]) continue;
    auto& tracked = env.state.tracked[i];
    if (tracked.knownValue != oldVal) continue;
    FTRACE(4, "       refining {} to {}\n", i, newVal->toString());
    tracked.knownValue = newVal;
    tracked.knownType  = newVal->type();
  }
}

//////////////////////////////////////////////////////////////////////

template<class Propagate>
Flags analyze_inst(Local& env,
                   const IRInstruction& inst,
                   Propagate propagate) {
  if (auto const taken = inst.taken()) propagate(taken, env.state);

  auto const effects = memory_effects(inst);
  FTRACE(3, "    {: <30} -- {}\n", show(effects), inst.toString());
  auto flags = Flags{};
  match<void>(
    effects,
    [&] (IrrelevantEffects) {},
    [&] (UnknownEffects)    { clear_everything(env); },
    [&] (InterpOneEffects)  { clear_everything(env); },
    [&] (KillFrameLocals l) {},
    [&] (ReturnEffects l)   {},
    [&] (CallEffects l)     { // Note: shouldn't need to give up types for
                              // locals, but it doesn't matter right now.
                              clear_everything(env); },
    [&] (IterEffects)       { clear_everything(env); },
    [&] (IterEffects2)      { clear_everything(env); },

    [&] (PureStore m)   { store(env, m.dst, m.value); },
    [&] (PureStoreNT m) { store(env, m.dst, m.value); },

    [&] (PureLoad m) {
      std::tie(flags.replaceable, flags.knownType) = load(env, inst, m.src);
    },

    [&] (MayLoadStore m) {
      store(env, m.stores, nullptr);

      switch (inst.op()) {
      /*
       * We could handle CheckLoc, but right now ir-builder does all that, so
       * it's not here yet.
       */
      case CheckTypeMem:
      case CheckTypePackedArrayElem:
        if (auto const tloc = find_tracked(env, inst, m.loads)) {
          if (tloc->knownType <= inst.typeParam()) {
            flags.convertToJmp = true;
            return;
          }
          tloc->knownType &= inst.typeParam();
        }
        break;
      default:
        break;
      }
    }
  );

  switch (inst.op()) {
  case CheckType:
    refine_value(env, inst.dst(), inst.src(0));
    break;
  default:
    break;
  }

  if (auto const next = inst.next()) propagate(next, env.state);

  return flags;
}

//////////////////////////////////////////////////////////////////////

void optimize_block(Local& env, Block* blk) {
  if (!env.state.initialized) {
    FTRACE(2, "  unreachable\n");
    return;
  }

  for (auto& inst : *blk) {
    auto const flags = analyze_inst(env, inst, [&] (Block*, const State&) {});

    auto const can_replace = [&] (SSATmp* what, Type knownType) -> bool {
      if (knownType == Type::Bottom) {
        // Unreachable code, but we're not allowed to create IR instructions
        // with a typeParam of Bottom.
        return false;
      }
      if (!(knownType <= inst.dst()->type())) {
        /*
         * It's possible we could assert the intersection of the types, but
         * it's not entirely clear what situations this would happen in, so
         * let's just not do it in this case for now.
         */
        FTRACE(2, "      knownType wasn't substitutable; not right now\n");
        return false;
      }

      if (what->inst()->is(DefConst)) return true;
      auto const defBlock = findDefiningBlock(what);
      if (!defBlock) return false;
      return dominates(defBlock, inst.block(), env.global.idoms);
    };

    if (flags.replaceable && can_replace(flags.replaceable, flags.knownType)) {
      FTRACE(2, "      redundant: {} :: {} = {}\n",
        inst.dst()->toString(),
        flags.knownType.toString(),
        flags.replaceable->toString());
      env.global.unit.replace(&inst, AssertType, flags.knownType,
        flags.replaceable);
      continue;
    }

    if (flags.convertToJmp) {
      FTRACE(2, "      unnecessary\n");
      env.global.unit.replace(&inst, Jmp, inst.next());
      continue;
    }
  }
}

//////////////////////////////////////////////////////////////////////

bool merge_into(TrackedLoc& dst, const TrackedLoc& src) {
  if (dst.knownValue != src.knownValue) {
    dst.knownValue = least_common_ancestor(
      dst.knownValue,
      src.knownValue
    );
    return true;
  }
  auto const newType = dst.knownType & src.knownType;
  auto const changed = newType != dst.knownType;
  dst.knownType = newType;
  return changed;
}

bool merge_into(Global& genv, State& dst, const State& src) {
  always_assert(src.initialized);
  if (!dst.initialized) {
    dst = src;
    return true;
  }

  always_assert(dst.tracked.size() == src.tracked.size());
  always_assert(dst.tracked.size() == genv.ainfo.locations.size());

  auto changed = false;

  auto mod_avail = [&] (ALocBits mask) {
    auto const new_avail = dst.avail & mask;
    if (new_avail != dst.avail) changed = true;
    dst.avail = new_avail;
  };

  mod_avail(src.avail);

  for (auto idx = uint32_t{0}; idx < kMaxTrackedALocs; ++idx) {
    if (!src.avail[idx]) continue;
    mod_avail(~genv.ainfo.locations_inv[idx].conflicts);

    if (dst.avail[idx]) {
      if (merge_into(dst.tracked[idx], src.tracked[idx])) {
        changed = true;
      }
    }
  }

  return changed;
}

//////////////////////////////////////////////////////////////////////

void analyze(Global& genv) {
  FTRACE(1, "\nAnalyze:\n");

  /*
   * Scheduled blocks for the fixed point computation.  We'll visit blocks with
   * lower reverse post order ids first.
   */
  auto incompleteQ = dataflow_worklist<RpoId>(genv.rpoBlocks.size());
  for (auto rpoId = uint32_t{0}; rpoId < genv.rpoBlocks.size(); ++rpoId) {
    genv.blockInfo[genv.rpoBlocks[rpoId]].rpoId = rpoId;
  }
  {
    auto& entryIn = genv.blockInfo[genv.unit.entry()].stateIn;
    entryIn.initialized = true;
    entryIn.tracked.resize(genv.ainfo.locations.size());
  }
  incompleteQ.push(0);

  while (!incompleteQ.empty()) {
    auto const blk = genv.rpoBlocks[incompleteQ.pop()];

    auto propagate = [&] (Block* target, const State& state) {
      auto& targetInfo = genv.blockInfo[target];
      FTRACE(3, "  -> {}\n", target->id());
      if (merge_into(genv, targetInfo.stateIn, state)) {
        FTRACE(7, "target state now:\n{}\n", show(targetInfo.stateIn));
        incompleteQ.push(targetInfo.rpoId);
      }
    };

    FTRACE(2, "B{}:\n", blk->id());
    auto env = Local { genv, genv.blockInfo[blk].stateIn };
    for (auto& inst : *blk) analyze_inst(env, inst, propagate);
  }
}

//////////////////////////////////////////////////////////////////////

}

/*
 * This pass does some analysis to try to avoid PureLoad instructions, or
 * remove unnecessary CheckTypeMem instructions.
 *
 * The way it works is to do an abstract interpretation of the IRUnit, where
 * we're tracking changes to abstract memory locations (TrackedLoc).  This pass
 * propagates input states to each block, and is repeated until every reachable
 * block's input state stops changing.
 *
 * Then we make a final walk over the CFG and try to perform some
 * optimizations.  Currently only a few things are done:
 *
 *    o Replace PureLoad instructions with new uses of an available value, if
 *      we know it would load that value.
 *
 *    o Remove instructions that check the type of memory locations if we
 *      proved the memory location must hold that type.
 */
void optimizeLoads(IRUnit& unit) {
  if (RuntimeOption::EnableArgsInBacktraces) {
    // We want to be able to run this pass with this flag on, but need to teach
    // memory_effects about it first.
    return;
  }
  PassTracer tracer{&unit, Trace::hhir_load, "optimizeLoads"};

  auto genv = Global { unit, rpoSortCfgWithIds(unit) };
  if (genv.ainfo.locations.size() == 0) {
    FTRACE(1, "no memory accesses to possibly optimize\n");
    return;
  }
  FTRACE(1, "\nLocations:\n{}\n", show(genv.ainfo));
  analyze(genv);

  FTRACE(1, "\n\nFixed point:\n{}\n",
    [&] () -> std::string {
      auto ret = std::string{};
      for (auto& blk : genv.rpoBlocks) {
        folly::format(&ret, "B{}:\n{}", blk->id(),
          show(genv.blockInfo[blk].stateIn));
      }
      return ret;
    }()
  );

  FTRACE(1, "\nOptimize:\n");
  for (auto& blk : genv.rpoBlocks) {
    FTRACE(1, "B{}:\n", blk->id());
    auto env = Local { genv, genv.blockInfo[blk].stateIn };
    optimize_block(env, blk);
  }
  FTRACE(2, "reflowing types\n");
  reflowTypes(genv.unit);
}

//////////////////////////////////////////////////////////////////////

}}
