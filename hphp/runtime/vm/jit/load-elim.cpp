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
#include <algorithm>
#include <utility>

#include <boost/variant.hpp>
#include <folly/Optional.h>
#include <folly/ScopeGuard.h>
#include <folly/Hash.h>

#include "hphp/util/functional.h"
#include "hphp/util/either.h"
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
using RpoID = uint32_t;

//////////////////////////////////////////////////////////////////////

/*
 * Information about a value in memory.
 *
 * We either have nullptr, meaning we know nothing, or we know a specific
 * SSATmp* for the value, or we'll track the Block* where different non-null
 * states were merged, which is also a location we will be able to insert a phi
 * to make a new available SSATmp.
 *
 * For some discussion on how we know whether the SSATmp* is usable (i.e. its
 * definition dominates code that might want to reuse it), see the bottom of
 * this file.
 */
using ValueInfo = Either<SSATmp*,Block*>;

/*
 * Each tracked location has both a ValueInfo and a separately-tracked type.
 * We may have a known type even without an SSATmp for an available value, or
 * that is more refined than an SSATmp knownValue's type.  This is always at
 * least as refined as knownValue->type(), if knownValue is an SSATmp.
 */
struct TrackedLoc {
  ValueInfo knownValue{nullptr};
  Type knownType{TTop};
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
  bool initialized = false;  // if false, we'll never reach this block

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
  RpoID rpoID;
  State stateIn;

  /*
   * Output states for the next and taken edge.  These are stored explicitly
   * for two reasons:
   *
   *    o When inserting phis, we need to see the state on each edge coming to
   *      a join point to know what to phi.
   *
   *    o This lets us re-merge all predecessors into a block's stateIn while
   *      doing the dataflow analysis.  This can get better results than just
   *      merging into stateIn in place, basically because of the way we
   *      represent the locations that need phis.  If we didn't remerge, it's
   *      possible for the analysis to believe it needs to insert phis even in
   *      blocks with only a single predecessor (e.g. if the first time it saw
   *      the block it had a SSATmp* in some location, but the second time it
   *      tries to merge a Block* for a phi location).
   */
  State stateOutNext;
  State stateOutTaken;
};

struct PhiKey {
  struct Hash {
    size_t operator()(PhiKey k) const {
      return folly::hash::hash_combine(pointer_hash<Block>()(k.block), k.aloc);
    }
  };

  bool operator==(PhiKey o) const {
    return block == o.block && aloc == o.aloc;
  }

  Block* block;
  uint32_t aloc;
};

struct Global {
  explicit Global(IRUnit& unit)
    : unit(unit)
    , rpoBlocks(rpoSortCfg(unit))
    , ainfo(collect_aliases(unit, rpoBlocks))
    , blockInfo(unit, BlockInfo{})
  {}

  IRUnit& unit;
  BlockList rpoBlocks;
  AliasAnalysis ainfo;

  /*
   * Map from each block to its information.  Before we've done the fixed point
   * analysis the states in the info structures are not necessarily meaningful.
   */
  StateVector<Block,BlockInfo> blockInfo;

  /*
   * During the optimize pass, we may add phis for the values known to be in
   * memory locations at particular control-flow-joins.  To avoid doing it more
   * than once for any given join point, this keeps track of what we've added
   * so if it's needed in more than one place we can reuse it.
   */
  jit::hash_map<PhiKey,SSATmp*,PhiKey::Hash> insertedPhis;
};

//////////////////////////////////////////////////////////////////////

using HPHP::jit::show;

std::string show(ValueInfo vi) {
  if (vi == nullptr) return "<>";
  return vi.match(
    [&] (SSATmp* t) { return t->toString(); },
    [&] (Block* b) { return folly::sformat("phi@B{}", b->id()); }
  );
}

std::string show(TrackedLoc li) {
  return folly::sformat(
    "{: >12} | {}",
    li.knownType.toString(),
    show(li.knownValue)
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

/*
 * No flags.  This means no transformations were possible for the analyzed
 * instruction.
 */
struct FNone {};

/*
 * The instruction was a pure load, and its value was known to be available in
 * `knownValue', with best known type `knownType'.
 */
struct FRedundant { ValueInfo knownValue; Type knownType; uint32_t aloc; };

/*
 * Like FRedundant, but the load is impure---rather than killing it, we can
 * factor out the load.
 */
struct FReducible { ValueInfo knownValue; Type knownType; uint32_t aloc; };

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
                         folly::Optional<ALocMeta> meta) {
  if (!meta) return nullptr;
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
    if (tracked.knownValue != nullptr) {
      return FRedundant { tracked.knownValue, tracked.knownType, meta->index };
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
      tracked.knownType.subtypeOfAny(TUninit, TInitNull, TNullptr)) {
    tracked.knownValue = env.global.unit.cns(tracked.knownType);

    FTRACE(4, "       {} <- {}\n", show(acls), inst.dst()->toString());
    FTRACE(5, "       av: {}\n", show(env.state.avail));
    return FRedundant {
      tracked.knownValue,
      tracked.knownType,
      kMaxTrackedALocs    // it's a constant, so it is nowhere
    };
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
  current.knownType = value ? value->type() : TTop;
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
  case CheckRefInner:
    {
      auto const meta = env.global.ainfo.find(canonicalize(m.loads));
      auto const tloc = find_tracked(env, inst, meta);
      if (!tloc) break;
      if (tloc->knownType <= inst.typeParam()) {
        return FJmpNext{};
      }
      tloc->knownType &= inst.typeParam();

      if (tloc->knownType <= TBottom) {
        // i.e., !maybe(inst.typeParam()); fail check.
        return FJmpTaken{};
      }
      if (tloc->knownValue != nullptr) {
        return FReducible { tloc->knownValue, tloc->knownType, meta->index };
      }
    }
    break;

  case CastStk:
  case CoerceStk:
    {
      // We only care about the stack component, since we only optimize the case
      // where we don't need to reenter.
      assert(m.loads.stack());
      AliasClass const stk = *m.loads.stack();

      auto const meta = env.global.ainfo.find(canonicalize(stk));
      auto const tloc = find_tracked(env, inst, meta);
      if (!tloc) break;
      if (inst.op() == CastStk &&
          inst.typeParam() == TNullableObj &&
          tloc->knownType <= TNull) {
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

void handle_call_effects(Local& env, CallEffects effects) {
  if (effects.destroys_locals) {
    clear_everything(env);
    return;
  }

  /*
   * Keep types for stack and frame locations, and throw away the values.  We
   * are just doing this to avoid extending lifetimes across php calls, which
   * currently always leads to spilling.
   */
  auto const stk_and_frame = env.global.ainfo.all_stack |
                             env.global.ainfo.all_frame;
  env.state.avail &= stk_and_frame;
  for (auto aloc = uint32_t{0};
      aloc < env.global.ainfo.locations.size();
      ++aloc) {
    if (!env.state.avail[aloc]) continue;
    env.state.tracked[aloc].knownValue = nullptr;
  }
}

Flags handle_assert(Local& env, const IRInstruction& inst) {
  auto const acls = [&] () -> folly::Optional<AliasClass> {
    switch (inst.op()) {
    case AssertLoc:
      return AliasClass {
        AFrame { inst.src(0), inst.extra<AssertLoc>()->locId }
      };
    case AssertStk:
      return AliasClass {
        AStack { inst.src(0), inst.extra<AssertStk>()->offset.offset, 1 }
      };
    default: break;
    }
    return folly::none;
  }();
  if (!acls) return FNone{};

  auto const meta = env.global.ainfo.find(canonicalize(*acls));
  auto const tloc = find_tracked(env, inst, meta);
  if (!tloc) return FNone{};

  tloc->knownType &= inst.typeParam();
  if (tloc->knownValue != nullptr) {
    return FReducible { tloc->knownValue, tloc->knownType, meta->index };
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

Flags analyze_inst(Local& env, const IRInstruction& inst) {
  if (inst.taken()) {
    env.global.blockInfo[inst.block()].stateOutTaken = env.state;
  }

  auto const effects = memory_effects(inst);
  FTRACE(3, "    {}\n"
            "      {}\n",
            inst.toString(),
            show(effects));
  auto flags = Flags{};
  match<void>(
    effects,
    [&] (IrrelevantEffects) {},
    [&] (UnknownEffects)    { clear_everything(env); },
    [&] (ExitEffects)       { clear_everything(env); },
    [&] (ReturnEffects)     {},

    [&] (PureStore m)       { store(env, m.dst, m.value); },
    [&] (PureSpillFrame m)  { store(env, m.stk, nullptr); },

    [&] (PureLoad m)        { flags = load(env, inst, m.src); },

    [&] (GeneralEffects m)  { flags = handle_general_effects(env, inst, m); },
    [&] (CallEffects x)     { handle_call_effects(env, x); }
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

  if (inst.next()) env.global.blockInfo[inst.block()].stateOutNext = env.state;

  return flags;
}

//////////////////////////////////////////////////////////////////////

/*
 * Make sure that every predecessor ends with a Jmp that we can add arguments
 * to, so we can create a new phi'd value.  This may involve splitting some
 * edges.
 */
void prepare_predecessors_for_phi(Global& env, Block* block) {
  auto preds = jit::vector<Block*>{};
  block->forEachPred([&] (Block* b) { preds.push_back(b); });

  for (auto& pred : preds) {
    if (pred->back().is(Jmp)) continue;
    auto const middle = splitEdge(env.unit, pred, block);
    ITRACE(3, "      B{} -> B{} to add a Jmp\n", pred->id(), middle->id());
    auto& midInfo = env.blockInfo[middle];
    auto& predInfo = env.blockInfo[pred];
    auto const& srcState = pred->next() == middle
      ? predInfo.stateOutNext
      : predInfo.stateOutTaken;
    // We don't need to set the in state on the new edge-splitting block,
    // because it isn't in our env.rpoBlocks, so we don't ever visit it for
    // the optimize pass.
    midInfo.stateOutTaken = srcState;
  }
}

SSATmp* resolve_phis_work(Global& env,
                          jit::vector<IRInstruction*>& mutated_labels,
                          Block* block,
                          uint32_t alocID) {
  auto& phi_record = env.insertedPhis[PhiKey { block, alocID }];
  if (phi_record) {
    ITRACE(3, "      resolve_phis: B{} for aloc {}: already made {}\n",
           block->id(),
           alocID,
           phi_record->toString());
    return phi_record;
  }

  // We should never require phis going into a catch block, because they may
  // not have multiple predecessors in HHIR.
  assertx(!block->isCatch());

  ITRACE(3, "      resolve_phis: B{} for aloc {}\n", block->id(), alocID);
  Trace::Indent indenter;

  prepare_predecessors_for_phi(env, block);

  // Create the new destination of the DefLabel at this block.  This has to
  // happen before we start hooking up the Jmps on predecessors, because the
  // block could be one of its own predecessors and it might need to send this
  // new dst into the same phi.
  if (block->front().is(DefLabel)) {
    env.unit.expandLabel(&block->front(), 1);
  } else {
    block->prepend(env.unit.defLabel(1, block->front().marker()));
  }
  auto const label = &block->front();
  mutated_labels.push_back(label);
  phi_record = label->dst(label->numDsts() - 1);

  // Now add the new argument to each incoming Jmp.  If one of the predecessors
  // has a phi state with this same block, it'll find the dst we just added in
  // phi_record.
  block->forEachPred([&] (Block* pred) {
    auto& predInfo = env.blockInfo[pred];
    auto& state = pred->next() == block ? predInfo.stateOutNext
                                        : predInfo.stateOutTaken;
    assertx(state.initialized);
    auto& incomingLoc = state.tracked[alocID];
    assertx(incomingLoc.knownValue != nullptr);

    auto const incomingTmp = incomingLoc.knownValue.match(
      [&] (SSATmp* t) { return t; },
      [&] (Block* incoming_phi_block) {
        // Note resolve_phis can add blocks, which can cause a resize of
        // env.blockInfo---don't reuse pointers/references to any block state
        // structures across here.
        return resolve_phis_work(
          env,
          mutated_labels,
          incoming_phi_block,
          alocID
        );
      }
    );

    ITRACE(4, "      B{} <~~> {}\n", pred->id(), incomingTmp->toString());
    env.unit.expandJmp(&pred->back(), incomingTmp);
  });

  ITRACE(4, "     dst: {}\n", phi_record->toString());
  return phi_record;
}

void reflow_deflabel_types(const IRUnit& unit,
                           const jit::vector<IRInstruction*>& labels) {
  for (bool changed = true; changed;) {
    changed = false;
    for (auto& l : labels) if (retypeDests(l, &unit)) changed = true;
  }
}

SSATmp* resolve_phis(Global& env, Block* block, uint32_t alocID) {
  auto mutated_labels = jit::vector<IRInstruction*>{};
  auto const ret = resolve_phis_work(env, mutated_labels, block, alocID);
  /*
   * We've potentially created some SSATmp's (that are defined by DefLabels),
   * which have types that depend on their own types.  This should only be
   * possible with loops.
   *
   * Right now, we can't just wait for the final reflowTypes at the end of this
   * pass to update things for this, because SSATmp types may be inspected by
   * our simplify() calls while we're still optimizing, so reflow them now.
   */
  reflow_deflabel_types(env.unit, mutated_labels);
  return ret;
}

template<class Flag>
SSATmp* resolve_value(Local& env, const IRInstruction& inst, Flag flags) {
  if (inst.dst() && !(flags.knownType <= inst.dst()->type())) {
    /*
     * It's possible we could assert the intersection of the types, but it's
     * not entirely clear what situations this would happen in, so let's just
     * not do it in this case for now.
     */
    FTRACE(2, "      knownType wasn't substitutable; not right now\n");
    return nullptr;
  }

  auto const val = flags.knownValue;
  if (val == nullptr) return nullptr;
  return val.match(
    [&] (SSATmp* t) { return t; },
    [&] (Block* b) { return resolve_phis(env.global, b, flags.aloc); }
  );
}

void reduce_inst(Local& env, IRInstruction& inst, const FReducible& flags) {
  auto const resolved = resolve_value(env, inst, flags);
  if (!resolved) return;

  DEBUG_ONLY Opcode oldOp = inst.op();
  DEBUG_ONLY Opcode newOp;

  auto const reduce_to = [&] (Opcode op, Type typeParam) {
    auto const taken = hasEdges(op) ? inst.taken() : nullptr;
    auto const newInst = env.global.unit.gen(
      op,
      inst.marker(),
      taken,
      typeParam,
      resolved
    );
    if (hasEdges(op)) newInst->setNext(inst.next());

    auto const block = inst.block();
    block->insert(++block->iteratorTo(&inst), newInst);
    inst.convertToNop();

    newOp = op;
  };

  switch (inst.op()) {
  case CheckTypeMem:
  case CheckLoc:
  case CheckStk:
  case CheckRefInner:
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
         resolved->toString());
}

//////////////////////////////////////////////////////////////////////

void optimize_inst(Local& env, IRInstruction& inst, Flags flags) {
  match<void>(
    flags,
    [&] (FNone) {},

    [&] (FRedundant flags) {
      auto const resolved = resolve_value(env, inst, flags);
      if (!resolved) return;

      FTRACE(2, "      redundant: {} :: {} = {}\n",
             inst.dst()->toString(),
             flags.knownType.toString(),
             resolved->toString());

      env.global.unit.replace(
        &inst,
        AssertType,
        flags.knownType,
        resolved
      );
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

void optimize_block(Local& env, Block* blk) {
  if (!env.state.initialized) {
    FTRACE(2, "  unreachable\n");
    return;
  }

  for (auto& inst : *blk) {
    simplify(env.global.unit, &inst);
    auto const flags = analyze_inst(env, inst);
    optimize_inst(env, inst, flags);
  }
}

void optimize(Global& genv) {
  /*
   * Simplify() calls can make blocks unreachable as we walk, and visiting the
   * unreachable blocks with simplify calls is not allowed.  They may have uses
   * of SSATmps that no longer have defs, which can break how the simplifier
   * chases up to definitions.
   *
   * We use a StateVector because we can add new blocks during optimize().
   */
  StateVector<Block,bool> reachable(genv.unit, false);
  reachable[genv.unit.entry()] = true;
  for (auto& blk : genv.rpoBlocks) {
    FTRACE(1, "B{}:\n", blk->id());

    if (!reachable[blk]) {
      FTRACE(2, "   unreachable\n");
      continue;
    }

    auto env = Local { genv, genv.blockInfo[blk].stateIn };
    optimize_block(env, blk);

    if (auto const x = blk->next()) reachable[x] = true;
    if (auto const x = blk->taken()) reachable[x] = true;
  }
}

//////////////////////////////////////////////////////////////////////

bool operator==(const TrackedLoc& a, const TrackedLoc& b) {
  return a.knownValue == b.knownValue && a.knownType == b.knownType;
}

bool operator==(const State& a, const State& b) {
  if (!a.initialized) return !b.initialized;
  if (!b.initialized) return false;
  return a.tracked == b.tracked && a.avail == b.avail;
}

bool operator!=(const State& a, const State& b) { return !(a == b); }

//////////////////////////////////////////////////////////////////////

void merge_into(Block* target, TrackedLoc& dst, const TrackedLoc& src) {
  dst.knownType |= src.knownType;

  if (dst.knownValue == src.knownValue || dst.knownValue == nullptr) return;
  if (src.knownValue == nullptr) {
    dst.knownValue = nullptr;
    return;
  }

  dst.knownValue.match(
    [&] (SSATmp* tdst) {
      src.knownValue.match(
        [&] (SSATmp* tsrc) {
          if (auto const lcm = least_common_ancestor(tdst, tsrc)) {
            dst.knownValue = lcm;
            return;
          }
          // We will need a phi at this block if we want to reuse this value.
          dst.knownValue = target;
        },

        [&] (Block* blk) { dst.knownValue = target; }
      );
    },

    [&] (Block* b) { dst.knownValue = target; }
  );
}

void merge_into(Global& genv, Block* target, State& dst, const State& src) {
  always_assert(src.initialized);
  if (!dst.initialized) {
    dst = src;
    return;
  }

  always_assert(dst.tracked.size() == src.tracked.size());
  always_assert(dst.tracked.size() == genv.ainfo.locations.size());

  dst.avail &= src.avail;

  for (auto idx = uint32_t{0}; idx < kMaxTrackedALocs; ++idx) {
    if (!src.avail[idx]) continue;
    dst.avail &= ~genv.ainfo.locations_inv[idx].conflicts;

    if (dst.avail[idx]) {
      merge_into(target, dst.tracked[idx], src.tracked[idx]);
    }
  }
}

//////////////////////////////////////////////////////////////////////

void analyze(Global& genv) {
  FTRACE(1, "\nAnalyze:\n");

  /*
   * Scheduled blocks for the fixed point computation.  We'll visit blocks with
   * lower reverse post order ids first.
   */
  auto incompleteQ = dataflow_worklist<RpoID>(genv.rpoBlocks.size());
  for (auto rpoID = uint32_t{0}; rpoID < genv.rpoBlocks.size(); ++rpoID) {
    genv.blockInfo[genv.rpoBlocks[rpoID]].rpoID = rpoID;
  }
  {
    auto& entryIn = genv.blockInfo[genv.unit.entry()].stateIn;
    entryIn.initialized = true;
    entryIn.tracked.resize(genv.ainfo.locations.size());
  }
  incompleteQ.push(0);

  do {
    auto propagate = [&] (Block* target) {
      FTRACE(3, "  -> {}\n", target->id());
      auto& targetInfo = genv.blockInfo[target];
      auto const oldState = targetInfo.stateIn;
      targetInfo.stateIn.initialized = false; // re-merge of all pred states
      target->forEachPred([&] (Block* pred) {
        auto const& predInfo = genv.blockInfo[pred];
        auto const& predState = pred->next() == target
          ? predInfo.stateOutNext
          : predInfo.stateOutTaken;
        if (predState.initialized) {
          FTRACE(7, "pulling state from pred B{}:\n{}\n",
                 pred->id(), show(predState));
          merge_into(genv, target, targetInfo.stateIn, predState);
        }
      });
      if (oldState != targetInfo.stateIn) {
        FTRACE(7, "target state now:\n{}\n", show(targetInfo.stateIn));
        incompleteQ.push(targetInfo.rpoID);
      }
    };

    auto const blk = genv.rpoBlocks[incompleteQ.pop()];
    auto& blkInfo = genv.blockInfo[blk];
    FTRACE(2, "B{}:\n", blk->id());

    auto env = Local { genv, blkInfo.stateIn };
    for (auto& inst : *blk) analyze_inst(env, inst);
    if (auto const t = blk->taken()) propagate(t);
    if (auto const n = blk->next())  propagate(n);
  } while (!incompleteQ.empty());
}

//////////////////////////////////////////////////////////////////////

}

/*
 * This pass does some analysis to try to avoid PureLoad instructions, and
 * avoid "hidden" loads that are part of some instructions like CheckLoc.  It
 * also runs the simplify() subroutine on every instruction in the unit, and
 * can eliminate some conditional branches that test types of memory locations.
 *
 *
 * Currently the following things are done:
 *
 *    o Replace PureLoad instructions with new uses of an available value, if
 *      we know it would load that value.
 *
 *    o Remove instructions that check the type of memory locations if we
 *      proved the memory location must hold that type.
 *
 *    o Convert instructions that check types of values in memory to use values
 *      in SSATmp virtual registers, if we know that we have a register that
 *      contains the same value as that memory location.
 *
 *    o Insert phis iff it allows us to do any of the above.
 *
 *    o Run the simplifier on every instruction.
 *
 *
 * Notes about preserving SSA form:
 *
 *   This pass can add new uses of SSATmp*'s to replace accesses to memory.
 *   This means we need to think about whether the SSATmp*'s will be defined in
 *   a location that dominates the places that we want to add new uses, which
 *   is one of the requirements to keep the IR in SSA form.
 *
 *   To understand why this seems to "just work", without any code specifically
 *   dealing with it, consider the following:
 *
 *   Suppose this pass sees a LdLoc at a program point and has determined the
 *   local being loaded contains an SSATmp `t1'.  We want to add a new use of
 *   `t1' and delete the LdLoc.
 *
 *   What this analysis state tells us is that every path through the program
 *   to the point of this LdLoc has provably stored `t1' to that local.  This
 *   also means that on every path to this point, `t1' was used in some
 *   instruction that did a store(*), which means `t1' must have been defined
 *   on that path, or else the input program already wasn't in SSA form.  But
 *   this is just the dominance condition we wanted: every path through the
 *   program to this load must have defined `t1', so its definition dominates
 *   the load.
 *
 *   (*) `t1' could also be defined by an instruction doing a load from that
 *       location, but it's even more obviously ok in that situation.
 *
 *   One further note: some parts of the code here is structured in
 *   anticipation of allowing the pass to conditionally explore parts of the
 *   CFG, so we don't need to propagate states down unreachable paths.  But
 *   this isn't implemented yet: the above will need to be revisted at that
 *   point (essentially we'll have to guarantee we eliminate any jumps to
 *   blocks this pass proved were unreachable to keep the above working at that
 *   point).
 *
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
  optimize(genv);
  FTRACE(2, "reflowing types\n");
  reflowTypes(genv.unit);
}

//////////////////////////////////////////////////////////////////////

}}
