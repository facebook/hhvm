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

#include <cstdint>
#include <algorithm>

#include <boost/variant.hpp>
#include <folly/ScopeGuard.h>
#include <folly/Hash.h>

#include "hphp/util/bitset-utils.h"
#include "hphp/util/functional.h"
#include "hphp/util/either.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/util/match.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/perf-warning.h"

#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/unwind.h"

#include "hphp/runtime/vm/jit/alias-analysis.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/dce.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/type-array-elem.h"

namespace HPHP::jit {

namespace {

TRACE_SET_MOD(hhir_load);

//////////////////////////////////////////////////////////////////////

// Reverse post order block ids.
using RpoID = uint32_t;

//////////////////////////////////////////////////////////////////////

BlockList reverse(const BlockList& list) {
  return BlockList(list.rbegin(), list.rend());
}

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

  /*
   * If we know whether various RDS entries have been initialized at this
   * position.
   */
  jit::flat_set<rds::Handle> initRDS{};
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
    , vmRegsLiveness(analyzeVMRegLiveness(unit, reverse(rpoBlocks)))
  {}

  bool changed() const {
    return
      instrsReduced || loadsRemoved || loadsRefined || jumpsRemoved ||
      edgesOptimized || stackTeardownsOptimized;
  }

  IRUnit& unit;
  BlockList rpoBlocks;
  AliasAnalysis ainfo;

  /*
   * Map from each block to its information.  Before we've done the fixed point
   * analysis the states in the info structures are not necessarily meaningful.
   */
  StateVector<Block,BlockInfo> blockInfo;
  StateVector<IRInstruction,KnownRegState> vmRegsLiveness;

  /*
   * During the optimize pass, we may add phis for the values known to be in
   * memory locations at particular control-flow-joins.  To avoid doing it more
   * than once for any given join point, this keeps track of what we've added
   * so if it's needed in more than one place we can reuse it.
   */
  jit::hash_map<PhiKey,SSATmp*,PhiKey::Hash> insertedPhis;

  /*
   * Stats
   */
  size_t instrsReduced = 0;
  size_t loadsRemoved  = 0;
  size_t loadsRefined  = 0;
  size_t jumpsRemoved  = 0;
  size_t edgesOptimized = 0;
  size_t stackTeardownsOptimized = 0;
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

  folly::format(
    &ret,
    "  initRDS : {}\n",
    [&] {
      using namespace folly::gen;
      return from(state.initRDS) | unsplit<std::string>(",");
    }()
  );

  return ret;
}

//////////////////////////////////////////////////////////////////////

struct Local {
  const Global& global;
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
 * The instruction is a load which can be refined to a better type than its
 * type-parameter currently has.
 */
struct FRefinableLoad { Type refinedType; };

/*
 * The instruction can be replaced with a nop.
 */
struct FRemovable {};

/*
 * The instruction can be legally replaced with a Jmp to either its next or
 * taken edge.
 */
struct FJmpNext {};
struct FJmpTaken {};

using Flags = boost::variant<FNone,FRedundant,FReducible,FRefinableLoad,
                             FRemovable,FJmpNext,FJmpTaken>;

//////////////////////////////////////////////////////////////////////

// Conservative list of instructions which have type-parameters safe to refine.
bool refinable_load_eligible(const IRInstruction& inst) {
  switch (inst.op()) {
    case LdLoc:
    case LdStk:
    case LdMem:
    case LdMBase:
    case LdIterPos:
    case LdIterEnd:
    case LdFrameThis:
    case LdFrameCls:
      assertx(inst.hasTypeParam());
      return true;
    default:
      return false;
  }
}

void clear_everything(Local& env) {
  FTRACE(3, "      clear_everything\n");
  env.state.avail.reset();
}

// Construct an immediate that represents all of an iterator's type fields.
int64_t iter_type_immediate(const IterTypeData& data) {
  return static_cast<uint32_t>(data.type.as_byte) << 16 |
         static_cast<uint32_t>(data.layout.toUint16());
}

TrackedLoc* find_tracked(State& state,
                         Optional<ALocMeta> meta) {
  if (!meta) return nullptr;
  return state.avail[meta->index] ? &state.tracked[meta->index]
                                  : nullptr;
}

TrackedLoc* find_tracked(Local& env,
                         Optional<ALocMeta> meta) {
  return find_tracked(env.state, meta);
}

Flags load(Local& env,
           const IRInstruction& inst,
           AliasClass acls) {
  acls = canonicalize(acls);

  auto const meta = env.global.ainfo.find(acls);
  if (!meta) return FNone{};
  assertx(meta->index < kMaxTrackedALocs);

  auto& tracked = env.state.tracked[meta->index];

  // We can always trust the dst type of the load. Add its knowledge to
  // knownType before doing anything else.
  if (!env.state.avail[meta->index]) {
    tracked.knownValue = nullptr;
    tracked.knownType = inst.dst()->type();
    env.state.avail.set(meta->index);
  } else {
    tracked.knownType &= inst.dst()->type();
  }

  if (tracked.knownType.admitsSingleVal()) {
    tracked.knownValue = env.global.unit.cns(tracked.knownType);

    FTRACE(4, "       {} <- {}\n", show(acls), inst.dst()->toString());
    FTRACE(5, "       av: {}\n", show(env.state.avail));
    return FRedundant {
      tracked.knownValue,
      tracked.knownType,
      kMaxTrackedALocs    // it's a constant, so it is nowhere
    };
  }

  if (tracked.knownValue != nullptr) {
    return FRedundant { tracked.knownValue, tracked.knownType, meta->index };
  } else {
    // Only set a new known value if we previously didn't have one. If we had a
    // known value already, we would have made the load redundant above, unless
    // we're in an Hint::Unused block. In that case, we want to keep any old
    // known value to avoid disturbing the main trace in case we merge back.
    tracked.knownValue = inst.dst();
    FTRACE(4, "       {} <- {}\n", show(acls), inst.dst()->toString());
    FTRACE(5, "       av: {}\n", show(env.state.avail));
  }

  // Even if we can't make this load redundant, we might be able to refine its
  // type parameter.
  if (refinable_load_eligible(inst)) {
    auto const refined = tracked.knownType & inst.typeParam();
    if (refined < inst.typeParam()) return FRefinableLoad { refined };
  }

  return FNone{};
}

Flags store(Local& env, AliasClass acls, SSATmp* value) {
  acls = canonicalize(acls);

  auto const meta = env.global.ainfo.find(acls);
  if (!meta) {
    env.state.avail &= ~env.global.ainfo.may_alias(acls);
    return FNone{};
  }

  FTRACE(5, "       av: {}\n", show(env.state.avail));
  env.state.avail &= ~meta->conflicts;

  auto& current = env.state.tracked[meta->index];
  if (env.state.avail[meta->index] && value &&
      current.knownValue == value) {
    return FRemovable{};
  }
  current.knownValue = value;
  current.knownType = value ? value->type() : TTop;
  env.state.avail.set(meta->index);
  FTRACE(4, "       {} <- {}\n", show(acls),
    value ? value->toString() : std::string("<>"));
  FTRACE(5, "       av: {}\n", show(env.state.avail));

  return FNone{};
}

bool handle_minstr(Local& env, const IRInstruction& inst,
                   const GeneralEffects& m) {
  if (!hasMInstrBaseEffects(inst)) return false;

  auto const base = inst.src(0);
  assertx(base->isA(TLval));
  auto const acls = canonicalize(pointee(base));
  auto const meta = env.global.ainfo.find(acls);
  if (!meta || !env.state.avail[meta->index]) return false;

  auto const old_val = env.state.tracked[meta->index];
  store(env, m.stores, nullptr);
  store(env, m.inout, nullptr);

  SCOPE_ASSERT_DETAIL("handle_minstr") { return inst.toString(); };
  always_assert(!env.state.avail[meta->index]);
  auto& new_val = env.state.tracked[meta->index];
  if (auto const n = mInstrBaseEffects(inst, old_val.knownType)) {
    new_val.knownValue = nullptr;
    new_val.knownType  = *n;
    FTRACE(5, "      Base changed. New type: {}\n", new_val.knownType);
  } else {
    new_val.knownValue = old_val.knownValue;
    new_val.knownType  = old_val.knownType;
    FTRACE(5, "      Base unchanged. Keeping type: {}\n", old_val.knownType);
  }
  env.state.avail.set(meta->index);
  store(env, m.kills, nullptr);
  return true;
}

Flags handle_general_effects(Local& env,
                             const IRInstruction& inst,
                             const GeneralEffects& preM) {
  auto const liveness = env.global.vmRegsLiveness[inst];
  auto const m = general_effects_for_vmreg_liveness(preM, liveness);

  if (inst.is(DecRef, DecRefNZ)) {
    // DecRef can only free things, which means from load-elim's point
    // of view it only has a kill set, which load-elim
    // ignores. DecRefNZ is always a no-op.
    return FNone{};
  } else if (handle_minstr(env, inst, m)) {
    return FNone{};
  }

  auto const handleCheck = [&](Type typeParam) -> Optional<Flags> {
    assertx(m.inout == AEmpty);
    assertx(m.backtrace.empty());
    auto const meta = env.global.ainfo.find(canonicalize(m.loads));
    if (!meta) return std::nullopt;

    auto const tloc = &env.state.tracked[meta->index];
    if (!env.state.avail[meta->index]) {
      // We know nothing about the location. Initialize it with our typeParam.
      tloc->knownValue = nullptr;
      tloc->knownType = typeParam;
      env.state.avail.set(meta->index);
    } else if (tloc->knownType <= typeParam) {
      // We had a type that was good enough to pass the check.
      return Flags{FJmpNext{}};
    } else {
      // We had a type that didn't pass the check. Narrow it using our
      // typeParam.
      tloc->knownType &= typeParam;
    }

    if (tloc->knownType <= TBottom) {
      // i.e., !maybe(typeParam); fail check.
      return Flags{FJmpTaken{}};
    }
    if (tloc->knownValue != nullptr) {
      return Flags{FReducible{tloc->knownValue, tloc->knownType, meta->index}};
    }
    return std::nullopt;
  };

  // Given a pointer, return the best known type for what it points at
  auto const pointeeType = [&] (SSATmp* ptr) {
    if (!ptr->isA(TMem)) return TTop;
    auto const acls = canonicalize(pointee(ptr));
    auto const meta = env.global.ainfo.find(acls);
    if (!meta || !env.state.avail[meta->index]) return TTop;
    auto const& tracked = env.state.tracked[meta->index];
    return tracked.knownType;
  };

  // Given a pointer, try to reduce an instruction to a constant. The
  // given lambda will be given the best known type, and must return a
  // value suitable for cns (or nullopt).
  auto const reduceToConstantFromPtr =
    [&] (SSATmp* ptr, auto f) -> Optional<Flags> {
    auto const type = pointeeType(ptr);
    auto const r = f(type);
    if (!r) return std::nullopt;
    auto const c = env.global.unit.cns(*r);
    return FRedundant { c, c->type(), kMaxTrackedALocs };
  };

  auto const flags = [&] () -> Optional<Flags> {
    switch (inst.op()) {
      case CheckTypeMem:
      case CheckLoc:
      case CheckStk:
      case CheckMBase:
        return handleCheck(inst.typeParam());

      case CheckInitMem:
        return handleCheck(TInitCell);

      case CheckMROProp:
        return handleCheck(Type::cns(true));

      case CheckIter: {
        assertx(m.inout == AEmpty);
        assertx(m.backtrace.empty());
        auto const meta = env.global.ainfo.find(canonicalize(m.loads));
        if (!meta || !env.state.avail[meta->index]) return std::nullopt;
        auto const& type = env.state.tracked[meta->index].knownType;
        if (!type.hasConstVal(TInt)) return std::nullopt;
        auto const value = iter_type_immediate(*inst.extra<CheckIter>());
        auto const match = type.intVal() == value;
        return match ? Flags{FJmpNext{}} : Flags{FJmpTaken{}};
      }

      case InitSProps: {
        auto const handle = inst.extra<ClassData>()->cls->sPropInitHandle();
        if (env.state.initRDS.count(handle) > 0) return FJmpNext{};
        env.state.initRDS.insert(handle);
        return std::nullopt;
      }

      case InitProps: {
        auto const handle = inst.extra<ClassData>()->cls->propHandle();
        if (env.state.initRDS.count(handle) > 0) return FJmpNext{};
        env.state.initRDS.insert(handle);
        return std::nullopt;
      }

      case CheckRDSInitialized: {
        auto const handle = inst.extra<CheckRDSInitialized>()->handle;
        if (env.state.initRDS.count(handle) > 0) return FJmpNext{};
        // set this unconditionally; we record taken state before every
        // instruction, and next state after each instruction
        env.state.initRDS.insert(handle);
        return std::nullopt;
      }

      case MarkRDSInitialized: {
        auto const handle = inst.extra<MarkRDSInitialized>()->handle;
        if (env.state.initRDS.count(handle) > 0) return FRemovable{};
        env.state.initRDS.insert(handle);
        return std::nullopt;
      }

      case CheckVecBounds: {
        assertx(inst.src(0)->isA(TVec));
        if (!inst.src(1)->hasConstVal(TInt)) return std::nullopt;
        auto const idx = inst.src(1)->intVal();
        auto const acls = canonicalize(AliasClass(AElemI { inst.src(0), idx }));
        auto const meta = env.global.ainfo.find(acls);
        if (!meta) return std::nullopt;
        if (!env.state.avail[meta->index]) return std::nullopt;
        if (env.state.tracked[meta->index].knownType.maybe(TUninit)) {
          return std::nullopt;
        }
        return Flags{FJmpNext{}};
      }

      case IsTypeMem:
      case IsNTypeMem:
        return reduceToConstantFromPtr(
          inst.src(0),
          [&] (Type type) -> Optional<bool> {
            if (type <= inst.typeParam()) return inst.is(IsTypeMem);
            if (!type.maybe(inst.typeParam())) return !inst.is(IsTypeMem);
            return std::nullopt;
          }
        );

      case LdInitPropAddr: {
        // If know the location can't be Uninit, we can dispense with
        // the check (this will be lowered into a LdPropAddr).
        auto const meta = env.global.ainfo.find(canonicalize(m.loads));
        if (!meta || !env.state.avail[meta->index]) return std::nullopt;
        auto const& type = env.state.tracked[meta->index].knownType;
        if (type.maybe(TUninit)) return std::nullopt;
        return FJmpNext{};
      }

      default:
        return std::nullopt;
    }
  }();
  if (flags) return *flags;

  store(env, m.stores, nullptr);
  store(env, m.inout, nullptr);
  store(env, m.kills, nullptr);

  return FNone{};
}

void handle_call_effects(Local& env,
                         const IRInstruction& inst,
                         const CallEffects& effects) {
  /*
   * Keep types for stack, locals, and iterators, and throw away the
   * values.  We are just doing this to avoid extending lifetimes
   * across php calls, which currently always leads to spilling.
   */
  auto const keep = env.global.ainfo.all_stack    |
                    env.global.ainfo.all_fcontext |
                    env.global.ainfo.all_ffunc    |
                    env.global.ainfo.all_fmeta    |
                    env.global.ainfo.all_local    |
                    env.global.ainfo.all_iter;
  env.state.avail &= keep;

  // Any stack locations modified by the callee are no longer valid
  store(env, effects.kills, nullptr);
  store(env, effects.uninits, nullptr);
  store(env, effects.inputs, nullptr);
  store(env, effects.actrec, nullptr);
  store(env, effects.outputs, nullptr);
}

Flags handle_assert(Local& env, const IRInstruction& inst) {
  auto const acls = [&] () -> Optional<AliasClass> {
    switch (inst.op()) {
    case AssertLoc:
      return AliasClass {
        ALocal { inst.src(0), inst.extra<AssertLoc>()->locId }
      };
    case AssertStk:
      return AliasClass {
        AStack::at(inst.extra<AssertStk>()->offset)
      };
    default: break;
    }
    return std::nullopt;
  }();
  if (!acls) return FNone{};

  auto const meta = env.global.ainfo.find(canonicalize(*acls));
  if (!meta) {
    FTRACE(4, "      untracked assert\n");
    return FNone{};
  }

  auto const tloc = &env.state.tracked[meta->index];
  if (!env.state.avail[meta->index]) {
    // We know nothing about the location. Initialize it with our typeParam.
    tloc->knownValue = nullptr;
    tloc->knownType = inst.typeParam();
    env.state.avail.set(meta->index);
  }

  tloc->knownType &= inst.typeParam();
  if (tloc->knownValue != nullptr || tloc->knownType.admitsSingleVal()) {
    if (tloc->knownValue == nullptr) {
      tloc->knownValue = env.global.unit.cns(tloc->knownType);
    }

    FTRACE(4, "      reducible assert: {}\n", show(*tloc));
    return FReducible { tloc->knownValue, tloc->knownType, meta->index };
  }

  FTRACE(4, "      non-reducible assert: {}\n", show(*tloc));
  return FNone{};
}

void check_decref_eligible(
  Local& env,
  CompactVector<std::pair<uint32_t, Type>>& elems,
  size_t index,
  AliasClass acls) {
    auto const meta = env.global.ainfo.find(canonicalize(acls));
    auto const tloc = find_tracked(env, meta);
    FTRACE(5, "    {}: {}\n", index, tloc ? show(*tloc) : "x");
    if (!tloc) {
      elems.push_back({index, TCell});
    } else if (tloc->knownType.maybe(TCounted)) {
      // The type needs to be at least a TCell for LdStk and LdLoc to work
      auto const type = tloc->knownType <= TCell ? tloc->knownType : TCell;
      elems.push_back({index, type});
    }
  };

void refine_value(Local& env, SSATmp* newVal, SSATmp* oldVal) {
  bitset_for_each_set(
    env.state.avail,
    [&](size_t i) {
      auto& tracked = env.state.tracked[i];
      if (tracked.knownValue != oldVal) return;
      FTRACE(4, "       refining {} to {}\n", i, newVal->toString());
      tracked.knownValue = newVal;
      tracked.knownType  = newVal->type();
    }
  );
}

// Certain instructions give us knowledge about the type in a location
// without an explicit store.
void assert_mem(Local& env, const IRInstruction& inst) {
  if (!inst.hasDst() || inst.dst()->isA(TBottom)) return;

  auto const update = [&] (Type type) {
    assertx(inst.dst()->type().maybe(TMem));
    auto const acls = canonicalize(pointee(inst.dst()));
    auto const meta = env.global.ainfo.find(acls);
    if (!meta) return;
    auto& current = env.state.tracked[meta->index];
    if (!env.state.avail[meta->index]) {
      current.knownValue = nullptr;
      current.knownType = type;
      env.state.avail.set(meta->index);
    } else {
      current.knownType &= type;
    }
    if (current.knownType.admitsSingleVal()) {
      current.knownValue = env.global.unit.cns(current.knownType);
    }
    FTRACE(
      4,"    assert {}: {} <- {}\n",
      inst.dst()->toString(),
      show(acls),
      show(current)
    );
  };

  switch (inst.op()) {
    case LdPropAddr:
    case LdInitPropAddr:
    case LdClsPropAddrOrNull:
    case LdClsPropAddrOrRaise:
      assertx(inst.typeParam() <= TCell);
      update(inst.typeParam());
      break;
    case LdRDSAddr:
    case LdInitRDSAddr:
      update(inst.extra<RDSHandleAndType>()->type);
      break;
    case ElemDictD:
    case ElemDictU:
    case BespokeElem: {
      assertx(inst.typeParam() <= TArrLike);
      auto elem = arrLikeElemType(
        inst.typeParam(),
        inst.src(1)->type(),
        inst.ctx()
      );
      if (!elem.second) {
        if (inst.is(ElemDictU) ||
            (inst.is(BespokeElem) && !inst.src(2)->boolVal())) {
          elem.first |= TInitNull;
        }
      }
      assertx(elem.first != TBottom);
      update(elem.first);
      break;
    }
    case ElemDictK:
      update(
        arrLikePosType(
          inst.src(3)->type(),
          inst.src(2)->type(),
          false,
          inst.ctx()
        )
      );
      break;
    case LdVecElemAddr:
      update(
        arrLikeElemType(
          inst.src(2)->type(),
          inst.src(1)->type(),
          inst.ctx()
        ).first
      );
      break;
    case StructDictElemAddr: {
      auto const arrType = inst.src(3)->type();
      auto elem = arrLikeElemType(
        arrType,
        inst.src(1)->type(),
        inst.ctx()
      );
      auto const& layout = arrType.arrSpec().layout();
      if (!elem.second && !layout.slotAlwaysPresent(inst.src(2)->type())) {
        elem.first |= TUninit;
      }
      update(elem.first);
      break;
    }
    default:
      break;
  }
}

//////////////////////////////////////////////////////////////////////

Flags analyze_inst(Local& env, const IRInstruction& inst) {
  auto const effects = memory_effects(inst);
  FTRACE(3, "    {}\n"
            "      {}\n",
            inst.toString(),
            show(effects));

  auto flags = Flags{};
  match<void>(
    effects,
    [&] (const IrrelevantEffects&) {},
    [&] (const UnknownEffects&)    { clear_everything(env); },
    [&] (const ExitEffects&)       { clear_everything(env); },
    [&] (const ReturnEffects&)     {},

    [&] (const PureStore& m)       { flags = store(env, m.dst, m.value); },
    [&] (const PureLoad& m)        { flags = load(env, inst, m.src); },

    [&] (const PureInlineCall& m)  { store(env, m.base, m.fp); },
    [&] (const GeneralEffects& m) {
      flags = handle_general_effects(env, inst, m);
    },
    [&] (const CallEffects& x)     { handle_call_effects(env, inst, x); }
  );

  switch (inst.op()) {
  case AssertType:
  case CheckType:
  case CheckNonNull:
    refine_value(env, inst.dst(), inst.src(0));
    break;
  case AssertLoc:
  case AssertStk:
    flags = handle_assert(env, inst);
    break;
  case LdIterPos: {
    // For pointer iters, the type of the pointee of the pos is a lower bound
    // on the union of the types of the base's values. The same is true for the
    // pointee type of the end.
    //
    // Since the end is loop-invariant, we can use its type to refine the pos
    // and so avoid value type-checks. Here, "dropConstVal" drops the precise
    // value of the end (for static bases) but preserves the pointee type.
    auto const iter = inst.extra<LdIterPos>()->iterId;
    auto const end_cls = canonicalize(AliasClass(aiter_end(inst.src(0), iter)));
    auto const end = find_tracked(env, env.global.ainfo.find(end_cls));
    if (end != nullptr) {
      auto const end_type = end->knownType.dropConstVal();
      if (end_type < inst.typeParam()) return FRefinableLoad { end_type };
    }
    break;
  }
  case StIterType: {
    // StIterType stores an immediate to the iter's type fields. We construct a
    // tmp to represent the immediate. (memory-effects can't do so w/o a unit.)
    auto const extra = inst.extra<StIterType>();
    auto const value = iter_type_immediate(*extra);
    auto const acls = canonicalize(AliasClass(aiter_type(inst.src(0), extra->iterId)));
    store(env, acls, env.global.unit.cns(value));
    break;
  }
  case EndInlining:
    if (RO::EvalHHIRInliningAssertMemoryEffects) {
      assertx(inst.src(0)->inst()->is(BeginInlining));
      auto const fp = inst.src(0);
      auto const callee = fp->inst()->extra<BeginInlining>()->func;

      auto const assertDead = [&] (AliasClass acls, const char* what) {
        auto const canon = canonicalize(acls);
        auto const mustBeDead = env.global.ainfo.expand(canon);
        always_assert_flog(
          (env.state.avail & mustBeDead).none(),
          "Detected that {} locations were still live after accounting for all "
          "effects at an EndInlining position\n    Locations: {}\n",
          what,
          show(mustBeDead)
        );
      };

      // Assert that all of the locals and iterators for this frame as well as
      // the ActRec itself and the minstr state have been marked dead.
      for_each_frame_location(fp, callee, assertDead);
    }
    break;
  default:
    assert_mem(env, inst);
    break;
  }

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
    env.unit.defLabel(1, block, block->front().bcctx());
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
SSATmp* resolve_value(Global& env, const IRInstruction& inst, Flag flags,
                      bool createPhis) {
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
    [&] (Block* b) -> SSATmp* {
      if (!createPhis) return nullptr;
      return resolve_phis(env, b, flags.aloc);
    }
  );
}

bool reduce_inst(Global& env, IRInstruction& inst, const FReducible& flags,
                 bool createPhis) {
  auto const resolved = resolve_value(env, inst, flags, createPhis);
  if (!resolved) return false;

  DEBUG_ONLY Opcode oldOp = inst.op();
  DEBUG_ONLY Opcode newOp;

  auto const reduce_to = [&] (Opcode op, Optional<Type> typeParam) {
    auto const taken = hasEdges(op) ? inst.taken() : nullptr;
    auto const newInst = env.unit.gen(
      op,
      inst.bcctx(),
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
  case CheckMBase:
    reduce_to(CheckType, inst.typeParam());
    break;

  case CheckInitMem:
    reduce_to(CheckInit, std::nullopt);
    break;

  case AssertLoc:
  case AssertStk:
    reduce_to(AssertType, flags.knownType);
    break;

  default: always_assert(false);
  }

  FTRACE(2, "      reduced: {} = {} {}\n",
         opcodeName(oldOp),
         opcodeName(newOp),
         resolved->toString());

  ++env.instrsReduced;
  return true;
}

void refine_load(Global& env,
                 IRInstruction& inst,
                 const FRefinableLoad& flags) {
  assertx(refinable_load_eligible(inst));
  assertx(flags.refinedType < inst.typeParam());

  FTRACE(2, "      refinable: {} :: {} -> {}\n",
         inst.toString(),
         inst.typeParam(),
         flags.refinedType);

  inst.setTypeParam(flags.refinedType);
  retypeDests(&inst, &env.unit);
  ++env.loadsRefined;
}

void optimize_end_catch(Global& env, IRInstruction& inst,
                        int32_t numStackElems,
                        CompactVector<std::pair<uint32_t, Type>>& elems) {
  FTRACE(3, "Optimizing EndCatch\n{}\nNumStackElems: {}\n",
            inst.marker().show(), numStackElems);

  auto const numLocals = inst.func()->numLocals();
  auto const block = inst.block();

  auto add = [&](IRInstruction* loadInst, int locId = -1) {
    block->insert(block->iteratorTo(&inst), loadInst);
    auto const decref =
      env.unit.gen(DecRef, inst.bcctx(), DecRefData{locId}, loadInst->dst());
    block->insert(block->iteratorTo(&inst), decref);
  };

  auto const original = inst.extra<EndCatchData>();
  if (RuntimeOption::EvalHHIRGenerateAsserts &&
      original->mode != EndCatchData::CatchMode::LocalsDecRefd) {
    block->insert(block->iteratorTo(&inst),
      env.unit.gen(DbgCheckLocalsDecRefd, inst.bcctx(), inst.src(0)));
  }

  for (auto elem : elems) {
    auto const i = elem.first;
    auto const type = elem.second;
    if (i < numLocals) {
      FTRACE(5, "    Emitting decref for LocalId {}\n", i);
      add(env.unit.gen(LdLoc, inst.bcctx(), type, LocalId{i}, inst.src(0)),
          i);
      continue;
    }
    auto const index = i - numLocals;
    auto const offset = IRSPRelOffsetData {
      inst.extra<EndCatchData>()->offset + numStackElems - 1 - index
    };
    FTRACE(5, "    Emitting decref for StackElem {} at IRSPRel {}\n",
            index, offset.offset.offset);
    add(env.unit.gen(LdStk, inst.bcctx(), type, offset, inst.src(1)));
  }

  auto const newOffset = original->offset + numStackElems;
  auto const bcSP = env.unit.gen(
    LoadBCSP,
    inst.bcctx(),
    IRSPRelOffsetData { newOffset },
    inst.src(1)
  );
  auto const syncSP = env.unit.gen(
    StVMSP,
    inst.bcctx(),
    bcSP->dst()
  );
  auto const teardownMode =
    original->mode != EndCatchData::CatchMode::LocalsDecRefd &&
    inst.func()->hasThisInBody()
      ? EndCatchData::Teardown::OnlyThis
      : EndCatchData::Teardown::None;
  auto const data = EndCatchData {
    newOffset,
    original->mode,
    original->stublogue,
    teardownMode
  };
  auto iter = block->iteratorTo(&inst)--;
  block->insert(iter++, bcSP);
  block->insert(iter++, syncSP);
  env.unit.replace(&inst, EndCatch, data, inst.src(0), inst.src(1));
  env.stackTeardownsOptimized++;
}

//////////////////////////////////////////////////////////////////////

void optimize_inst(Global& env, IRInstruction& inst, Flags flags,
                   bool createPhis) {
  auto simplify = false;
  match<void>(
    flags,
    [&] (FNone) {},

    [&] (FRedundant redundantFlags) {
      auto const resolved =
        resolve_value(env, inst, redundantFlags, createPhis);
      if (!resolved) return;

      FTRACE(2, "      redundant: {} :: {} = {}\n",
             inst.dst()->toString(),
             redundantFlags.knownType.toString(),
             resolved->toString());

      // If this instruction had control flow, add a Jmp to the next
      // block. (We assume that if we're making it redundant, any
      // taken edge isn't used).
      if (auto const next = inst.next()) {
        auto const jmp = env.unit.gen(Jmp, inst.bcctx(), next);
        auto const block = inst.block();
        block->insert(++block->iteratorTo(&inst), jmp);
      }

      if (resolved->inst()->is(DefConst) ||
          !resolved->type().subtypeOfAny(TCell, TMem)) {
        env.unit.replace(&inst, Mov, resolved);
      } else {
        env.unit.replace(&inst, AssertType, redundantFlags.knownType, resolved);
      }

      ++env.loadsRemoved;
      simplify = true;
    },

    [&] (FReducible reducibleFlags) {
      if (reduce_inst(env, inst, reducibleFlags, createPhis)) simplify = true;
    },

    [&] (FRefinableLoad f) {
      refine_load(env, inst, f);
      simplify = true;
    },

    [&] (FRemovable) {
      FTRACE(2, "      removable\n");
      assertx(!inst.isControlFlow());
      inst.convertToNop();
    },

    [&] (FJmpNext) {
      FTRACE(2, "      unnecessary\n");
      if (inst.is(LdInitPropAddr)) {
        // Special case: Since LdInitPropAddr produces a result, if
        // the init check is unnecessary, turn it into a LdPropAddr
        // and Jmp. If the Lval it produces is also unnecessary, it
        // will be later DCEd away.
        auto const block = inst.block();
        auto const jmp = env.unit.gen(Jmp, inst.bcctx(), inst.next());
        block->insert(++block->iteratorTo(&inst), jmp);
        inst.setOpcode(LdPropAddr);
        inst.setNext(nullptr);
        inst.setTaken(nullptr);
      } else {
        env.unit.replace(&inst, Jmp, inst.next());
      }
      ++env.jumpsRemoved;
    },

    [&] (FJmpTaken) {
      FTRACE(2, "      unnecessary\n");
      env.unit.replace(&inst, Jmp, inst.taken());
      ++env.jumpsRemoved;
    }
  );

  // simplifyInPlace may perform CFG changes incompatible with phi creation.
  if (simplify && !createPhis) simplifyInPlace(env.unit, &inst);
}

void optimize_edges(Global& env, Block* blk) {
  /*
   * If a target block of an edge is a branch which could be trivially resolved
   * in the context of the source block state, replace the target block with
   * the resolved branch.
   */
  auto const optimize = [&](const State& state, Block* targetBlk) -> Block* {
    if (targetBlk == nullptr) return nullptr;

    while (targetBlk->front().is(Jmp) && targetBlk->front().numSrcs() == 0) {
      targetBlk = targetBlk->front().taken();
    }

    if (targetBlk->instrs().size() != 1) return nullptr;
    auto const& inst = targetBlk->front();

    auto const handleCheck = [&](Type typeParam) -> Block* {
      assertx(inst.next() && inst.taken());
      auto const ge = boost::get<GeneralEffects>(memory_effects(inst));
      auto const meta = env.ainfo.find(canonicalize(ge.loads));
      if (!meta) return nullptr;
      if (!state.avail[meta->index]) return nullptr;
      auto const& tloc = state.tracked[meta->index];
      if (tloc.knownType <= typeParam) return inst.next();
      if (!tloc.knownType.maybe(typeParam)) return inst.taken();
      return nullptr;
    };

    switch (inst.op()) {
      case CheckTypeMem:
      case CheckLoc:
      case CheckStk:
      case CheckMBase:
        return handleCheck(inst.typeParam());

      case CheckInitMem:
        return handleCheck(TInitCell);

      case CheckMROProp:
        return handleCheck(Type::cns(true));

      default:
        return nullptr;
    }
  };

  auto const& bi = env.blockInfo[blk];
  while (auto const newNext = optimize(bi.stateOutNext, blk->next())) {
    FTRACE(2, "      setNext: {} -> {}\n", blk->next()->id(), newNext->id());
    blk->back().setNext(newNext);
    ++env.edgesOptimized;
  }
  while (auto const newTaken = optimize(bi.stateOutTaken, blk->taken())) {
    FTRACE(2, "      setTaken: {} -> {}\n", blk->taken()->id(), newTaken->id());
    blk->back().setTaken(newTaken);
    ++env.edgesOptimized;
  }
}

void optimize_block(Local& env, Global& genv, Block* blk, bool createPhis) {
  if (!env.state.initialized) {
    FTRACE(2, "  unreachable\n");
    return;
  }

  for (auto& inst : *blk) {
    // simplifyInPlace may perform CFG changes incompatible with phi creation.
    if (!createPhis) simplifyInPlace(genv.unit, &inst);
    auto const flags = analyze_inst(env, inst);
    optimize_inst(genv, inst, flags, createPhis);
  }

  // If we are allowed to create phis, we can't perform CFG optimizations that
  // may move jumps through blocks defining these phis.
  if (!createPhis) optimize_edges(genv, blk);
}

void optimize(Global& genv, bool createPhis) {
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
    optimize_block(env, genv, blk, createPhis);

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
    [&](SSATmp* tdst) {
      src.knownValue.match(
        [&](SSATmp* tsrc) {
          if (auto const lcm = least_common_ancestor(tdst, tsrc)) {
            dst.knownValue = lcm;
            return;
          }
          // We will need a phi at this block if we want to reuse this value.
          dst.knownValue = target;
        },

        [&](Block* /*blk*/) { dst.knownValue = target; });
    },

    [&](Block* /*b*/) { dst.knownValue = target; });
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

  bitset_for_each_set(
    src.avail,
    [&](size_t idx) {
      dst.avail &= ~genv.ainfo.locations_inv[idx].conflicts;
      if (dst.avail[idx]) {
        merge_into(target, dst.tracked[idx], src.tracked[idx]);
      }
    }
  );

  for (auto it = dst.initRDS.begin(); it != dst.initRDS.end();) {
    if (!src.initRDS.count(*it)) {
      it = dst.initRDS.erase(it);
    } else {
      ++it;
    }
  }
}

//////////////////////////////////////////////////////////////////////

void save_taken_state(Global& genv, const IRInstruction& inst,
                      const State& state) {
  if (!inst.taken()) return;

  auto& outState = genv.blockInfo[inst.block()].stateOutTaken = state;

  auto const handleCheck = [&](Type maxTakenType, Type subtractedType) {
    assertx(!inst.maySyncVMRegsWithSources());
    auto const effects = memory_effects(inst);
    auto const ge = boost::get<GeneralEffects>(effects);
    assertx(ge.inout == AEmpty);
    assertx(ge.backtrace.empty());
    auto const meta = genv.ainfo.find(canonicalize(ge.loads));
    if (auto const tloc = find_tracked(outState, meta)) {
      tloc->knownType = negativeCheckType(
        tloc->knownType & maxTakenType, subtractedType);
    } else if (meta) {
      auto tloc = &outState.tracked[meta->index];
      tloc->knownValue = nullptr;
      tloc->knownType = negativeCheckType(maxTakenType, subtractedType);
      outState.avail.set(meta->index);
    }
  };

  switch (inst.op()) {
    case CheckTypeMem:
    case CheckLoc:
    case CheckStk:
    case CheckMBase:
      // Subtract inst.typeParam() on the taken branch.
      handleCheck(TCell, inst.typeParam());
      break;

    case CheckInitMem:
      // The pointee is TUninit on the taken branch.
      handleCheck(TUninit, TBottom);
      break;

    case CheckMROProp:
      // The bit is set to false on taken branch.
      handleCheck(Type::cns(false), TBottom);
      break;

    default:
      break;
  }
}

void save_next_state(Global& genv, const IRInstruction& inst,
                     const State& state) {
  if (inst.next()) genv.blockInfo[inst.block()].stateOutNext = state;
}

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
        auto const merge = [&](const State& predState) {
          if (predState.initialized) {
            FTRACE(7, "pulling state from pred B{}:\n{}\n",
                   pred->id(), show(predState));
            merge_into(genv, target, targetInfo.stateIn, predState);
          }
        };

        auto const& predInfo = genv.blockInfo[pred];
        if (pred->next() != pred->taken()) {
          auto const& predState = pred->next() == target
            ? predInfo.stateOutNext
            : predInfo.stateOutTaken;
          merge(predState);
        } else {
          // The predecessor jumps to this block along both its next and taken
          // branches. We can't distinguish the two paths, so just merge both
          // in.
          assertx(pred->next() == target);
          merge(predInfo.stateOutNext);
          merge(predInfo.stateOutTaken);
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
    for (auto& inst : *blk) {
      save_taken_state(genv, inst, env.state);
      analyze_inst(env, inst);
      save_next_state(genv, inst, env.state);
    }
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
  Timer t(Timer::optimize_loads, unit.logEntry().get_pointer());

  // Unfortunately load-elim may not reach a fixed-point after just one
  // round. This is because the optimization stage can cause the types of
  // SSATmps to change, which then can affect what we infer in the analysis
  // stage. So, loop until we reach a fixed point. Bound the iterations at some
  // max value for safety.
  size_t iters = 0;
  size_t instrsReduced = 0;
  size_t loadsRemoved = 0;
  size_t loadsRefined = 0;
  size_t jumpsRemoved = 0;
  size_t edgesOptimized = 0;
  size_t stackTeardownsOptimized = 0;
  do {
    auto genv = Global { unit };
    if (genv.ainfo.locations.size() == 0) {
      FTRACE(1, "no memory accesses to possibly optimize\n");
      break;
    }

    ++iters;
    FTRACE(1, "\nIteration #{}\n", iters);
    FTRACE(1, "Locations:\n{}\n", show(genv.ainfo));

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
    optimize(genv, false);

    if (!genv.changed()) {
      FTRACE(1, "\nOptimize with phis:\n");
      optimize(genv, true);
    }

    if (!genv.changed()) {
      // Nothing changed so we're done
      break;
    }
    instrsReduced += genv.instrsReduced;
    loadsRemoved += genv.loadsRemoved;
    loadsRefined += genv.loadsRefined;
    jumpsRemoved += genv.jumpsRemoved;
    edgesOptimized += genv.edgesOptimized;
    stackTeardownsOptimized += genv.stackTeardownsOptimized;

    FTRACE(2, "reflowing types\n");
    reflowTypes(genv.unit);

    // Restore reachability invariants
    mandatoryDCE(unit);

    if (iters >= RuntimeOption::EvalHHIRLoadElimMaxIters) {
      // We've iterated way more than usual without reaching a fixed
      // point. Either there's some bug in load-elim, or this unit is especially
      // pathological. Emit a perf warning so we're aware and stop iterating.
      logPerfWarning(
        "optimize_loads_max_iters", 1,
        [&](StructuredLogEntry& cols) {
          auto const func = unit.context().initSrcKey.func();
          cols.setStr("func", func->fullName()->slice());
          cols.setStr("filename", func->unit()->origFilepath()->slice());
          cols.setStr("hhir_unit", show(unit));
        }
      );
      break;
    }
  } while (true);

  if (auto& entry = unit.logEntry()) {
    entry->setInt("optimize_loads_iters", iters);
    entry->setInt("optimize_loads_instrs_reduced", instrsReduced);
    entry->setInt("optimize_loads_loads_removed", loadsRemoved);
    entry->setInt("optimize_loads_loads_refined", loadsRefined);
    entry->setInt("optimize_loads_jumps_removed", jumpsRemoved);
    entry->setInt("optimize_loads_edges_optimized", edgesOptimized);
    entry->setInt("optimize_loads_stack_teardowns_optimized",
      stackTeardownsOptimized);
  }
}

//////////////////////////////////////////////////////////////////////

}
