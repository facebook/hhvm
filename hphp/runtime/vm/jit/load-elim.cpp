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

#include <boost/variant.hpp>
#include <folly/Optional.h>
#include <folly/ScopeGuard.h>

#include "hphp/util/dataflow-worklist.h"
#include "hphp/util/match.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/alias-analysis.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/state-vector.h"

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
  explicit Global(IRUnit& unit)
    : unit(unit)
    , rpoBlocks(rpoSortCfg(unit))
    , idoms(findDominators(unit, rpoBlocks, numberBlocks(unit, rpoBlocks)))
    , ainfo(collect_aliases(unit, rpoBlocks))
    , blockInfo(unit, BlockInfo{})
  {}

  IRUnit& unit;
  BlockList rpoBlocks;
  IdomVector idoms;
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

DEBUG_ONLY std::string show(const State& state) {
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

struct FNone {};
/*
 * The instruction was a pure load, and its value was known to be available in
 * `knownValue', with best known type `knownType'.
 */
struct FRedundant { SSATmp* knownValue; Type knownType; };
/*
 * Like FRedundant, but the load is impure---rather than killing it, we can
 * factor out the load.
 */
struct FReducible { SSATmp* knownValue; Type knownType; };
/*
 * The instruction can be legally replaced with a Jmp to either its next or
 * taken edge.
 */
struct FJmpNext {};
struct FJmpTaken {};

using Flags = boost::variant<FNone,FRedundant,FReducible,FJmpNext,FJmpTaken>;

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
  assertx(meta->index < kMaxTrackedALocs);
  return env.state.avail[meta->index] ? &env.state.tracked[meta->index]
                                      : nullptr;
}

Flags load(Local& env,
           const IRInstruction& inst,
           AliasClass acls) {
  acls = canonicalize(acls);

  auto const meta = env.global.ainfo.find(acls);
  if (!meta) return FNone{};
  assert(meta->index < kMaxTrackedALocs);

  auto& tracked = env.state.tracked[meta->index];

  if (env.state.avail[meta->index]) {
    if (tracked.knownValue) {
      return FRedundant { tracked.knownValue, tracked.knownType };
    }
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
  if (tracked.knownType.hasConstVal() ||
      tracked.knownType.subtypeOfAny(Type::Uninit, Type::InitNull,
                                     Type::Nullptr)) {
    tracked.knownValue = env.global.unit.cns(tracked.knownType);

    FTRACE(4, "       {} <- {}\n", show(acls), inst.dst()->toString());
    FTRACE(5, "       av: {}\n", show(env.state.avail));
    return FRedundant { tracked.knownValue, tracked.knownType };
  }

  tracked.knownValue = inst.dst();

  FTRACE(4, "       {} <- {}\n", show(acls), inst.dst()->toString());
  FTRACE(5, "       av: {}\n", show(env.state.avail));
  return FNone{};
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

Flags handle_general_effects(Local& env,
                             const IRInstruction& inst,
                             GeneralEffects m) {
  switch (inst.op()) {
  case CheckTypeMem:
  case CheckLoc:
  case CheckStk:
    if (auto const tloc = find_tracked(env, inst, m.loads)) {
      if (tloc->knownType <= inst.typeParam()) {
        return FJmpNext{};
      }
      tloc->knownType &= inst.typeParam();

      if (tloc->knownType <= Type::Bottom) {
        // i.e., !maybe(inst.typeParam()); fail check.
        return FJmpTaken{};
      }
      if (tloc->knownValue) {
        return FReducible { tloc->knownValue, tloc->knownType };
      }
    }
    break;

  case CastStk:
  case CoerceStk:
    assert(m.loads.stack());

    // We only care about the stack component, since we only optimize the case
    // where we don't need to reenter.
    if (auto const tloc = find_tracked(env, inst, *m.loads.stack())) {
      if (inst.op() == CastStk &&
          inst.typeParam() == Type::NullableObj &&
          tloc->knownType <= Type::Null) {
        // If we're casting Null to NullableObj, we still need to call
        // tvCastToNullableObjectInPlace.  See comment there and t3879280 for
        // details.
        break;
      }
      if (tloc->knownType <= inst.typeParam()) {
        return FJmpNext{};
      }
    }
    break;

  default:
    break;
  }

  store(env, m.stores, nullptr);

  return FNone{};
}

Flags handle_assert(Local& env, const IRInstruction& inst) {
  auto const tloc = [&]() -> TrackedLoc* {
    folly::Optional<AliasClass> acls;

    switch (inst.op()) {
    case AssertLoc:
      acls = AFrame { inst.src(0), inst.extra<AssertLoc>()->locId };
      break;
    case AssertStk:
      acls = AStack { inst.src(0), inst.extra<AssertStk>()->offset.offset, 1 };
      break;
    default:
      break;
    }
    if (!acls) return nullptr;
    return find_tracked(env, inst, *acls);
  }();

  if (tloc) {
    tloc->knownType &= inst.typeParam();

    if (tloc->knownValue) {
      return FReducible { tloc->knownValue, tloc->knownType };
    }
  }

  return FNone{};
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
    [&] (ReturnEffects l)   {},
    [&] (CallEffects l)     { // Note: shouldn't need to give up types for some
                              // locations (e.g. locals), but CallEffects needs
                              // more information to do it correctly.
                              clear_everything(env); },
    [&] (IterEffects)       { clear_everything(env); },
    [&] (IterEffects2)      { clear_everything(env); },
    [&] (ExitEffects)       { clear_everything(env); },

    [&] (PureStore m)      { store(env, m.dst, m.value); },
    [&] (PureStoreNT m)    { store(env, m.dst, m.value); },
    [&] (PureSpillFrame m) { store(env, m.dst, nullptr); },

    [&] (PureLoad m)       { flags = load(env, inst, m.src); },

    [&] (GeneralEffects m) { flags = handle_general_effects(env, inst, m); }
  );

  switch (inst.op()) {
  case CheckType:
    refine_value(env, inst.dst(), inst.src(0));
    break;
  case AssertLoc:
  case AssertStk:
    flags = handle_assert(env, inst);
    break;
  default:
    break;
  }

  if (auto const next = inst.next()) propagate(next, env.state);

  return flags;
}

//////////////////////////////////////////////////////////////////////

bool can_replace(const Local& env,
                 const IRInstruction& inst,
                 const SSATmp* what,
                 Type knownType) {
  if (inst.dst() && !(knownType <= inst.dst()->type())) {
    /*
     * It's possible we could assert the intersection of the types, but
     * it's not entirely clear what situations this would happen in, so
     * let's just not do it in this case for now.
     */
    FTRACE(2, "      knownType wasn't substitutable; not right now\n");
    return false;
  }
  return is_tmp_usable(env.global.idoms, what, inst.block());
}

void reduce_inst(Local& env, IRInstruction& inst, const FReducible& flags) {
  if (!can_replace(env, inst, flags.knownValue, flags.knownType)) return;

  DEBUG_ONLY Opcode oldOp = inst.op();
  DEBUG_ONLY Opcode newOp;

  auto const reduce_to = [&] (Opcode op, Type typeParam) {
    auto block = inst.block();
    auto taken = hasEdges(op) ? inst.taken() : nullptr;

    auto newInst = env.global.unit.gen(op, inst.marker(), taken,
                                       typeParam, flags.knownValue);
    if (hasEdges(op)) newInst->setNext(inst.next());

    block->insert(++block->iteratorTo(&inst), newInst);
    inst.convertToNop();

    newOp = op;
  };

  switch (inst.op()) {
  case CheckTypeMem:
  case CheckLoc:
  case CheckStk:
    reduce_to(CheckType, inst.typeParam());
    break;

  case AssertLoc:
  case AssertStk:
    reduce_to(AssertType, flags.knownType);
    break;

  default: always_assert(false);
  }

  FTRACE(2, "      reducible: {} = {} {}\n",
         opcodeName(oldOp),
         opcodeName(newOp),
         flags.knownValue->toString());
}

//////////////////////////////////////////////////////////////////////

void optimize_block(Local& env, Block* blk) {
  if (!env.state.initialized) {
    FTRACE(2, "  unreachable\n");
    return;
  }

  for (auto& inst : *blk) {
    simplify(env.global.unit, &inst);

    auto const flags = analyze_inst(env, inst, [&] (Block*, const State&) {});

    match<void>(
      flags,
      [&] (FNone) {},
      [&] (FRedundant flags) {
        if (!can_replace(env, inst, flags.knownValue, flags.knownType)) return;

        FTRACE(2, "      redundant: {} :: {} = {}\n",
               inst.dst()->toString(),
               flags.knownType.toString(),
               flags.knownValue->toString());

        env.global.unit.replace(&inst, AssertType, flags.knownType,
                                flags.knownValue);
      },
      [&] (FReducible flags) { reduce_inst(env, inst, flags); },
      [&] (FJmpNext) {
        FTRACE(2, "      unnecessary\n");
        env.global.unit.replace(&inst, Jmp, inst.next());
      },
      [&] (FJmpTaken) {
        FTRACE(2, "      unnecessary\n");
        env.global.unit.replace(&inst, Jmp, inst.taken());
      }
    );

    // Re-simplify AssertType if we produced any.
    if (inst.is(AssertType)) simplify(env.global.unit, &inst);
  }
}

//////////////////////////////////////////////////////////////////////

bool merge_into(TrackedLoc& dst, const TrackedLoc& src) {
  if (dst.knownValue != src.knownValue) {
    dst.knownValue = least_common_ancestor(
      dst.knownValue,
      src.knownValue
    );
    dst.knownType |= src.knownType;
    return true;
  }
  auto const newType = dst.knownType | src.knownType;
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
 * remove unnecessary Check instructions.
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
  PassTracer tracer{&unit, Trace::hhir_load, "optimizeLoads"};

  auto genv = Global { unit };
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
