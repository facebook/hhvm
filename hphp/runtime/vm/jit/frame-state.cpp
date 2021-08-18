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

#include "hphp/runtime/vm/jit/frame-state.h"

#include "hphp/runtime/vm/jit/alias-class.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/irgen-call.h"
#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/resumable.h"

#include "hphp/util/dataflow-worklist.h"
#include "hphp/util/match.h"
#include "hphp/util/trace.h"

#include <boost/range/adaptor/reversed.hpp>

#include <algorithm>

TRACE_SET_MOD(hhir_fsm);

namespace HPHP { namespace jit { namespace irgen {

namespace {

using Trace::Indent;

///////////////////////////////////////////////////////////////////////////////

/*
 * Helper that sets a value to a new value and also returns whether it changed.
 */
template<class T>
bool merge_util(T& oldVal, const T& newVal) {
  auto changed = oldVal != newVal;
  oldVal = newVal;
  return changed;
}

/*
 * Merge TypeSourceSets, returning whether anything changed.
 *
 * TypeSourceSets are merged a join points by unioning the type sources.  The
 * reason for this is that if a type is constrained, we need to be able to find
 * "all" possible sources of the type and constrain them.
 */
bool merge_into(TypeSourceSet& dst, const TypeSourceSet& src) {
  auto changed = false;
  for (auto x : src) changed = dst.insert(x).second || changed;
  return changed;
}

/*
 * Merge LocationStates, returning whether anything changed.
 */
template<LTag lt, LTag rt>
bool merge_into(LocationState<lt>& dst, const LocationState<rt>& src) {
  auto changed = false;

  changed |= merge_util(dst.type, dst.type | src.type);

  // Get the least common ancestor across both states.
  changed |= merge_util(dst.value, least_common_ancestor(dst.value, src.value));

  // We may have changed either dst.value or dst.type in a way that could fail
  // to preserve LocationState invariants.  So check if we can't keep the value.
  if (dst.value != nullptr && dst.value->type() != dst.type) {
    dst.value = nullptr;
    changed = true;
  }

  changed |= merge_into(dst.typeSrcs, src.typeSrcs);

  if (!dst.maybeChanged && src.maybeChanged) {
    dst.maybeChanged = true;
    changed = true;
  }

  changed |= merge_util(dst.predictedType,
                        dst.predictedType | src.predictedType);
  return changed;
}

bool merge_memory_stack_into(StackStateMap& dst, const StackStateMap& src) {
  auto changed = false;
  // Throw away any information only known in dst.
  for (auto& [dIdx, dState] : dst) {
    if (src.count(dIdx) == 0) {
      dState = StackState{};
      changed = true;
    }
  }
  // Merge the information from src into dst.
  for (auto& [sIdx, sState] : src) {
    changed |= merge_into(dst[sIdx], sState);
  }
  return changed;
}

/*
 * Merge one FrameState into another, returning whether it changed.  Frame
 * pointers and stack depth must match.  If the stack pointer tmps are
 * different, clear the tracked value (we can make a new one, given fp and
 * irSPOff).
 */
bool merge_into(FrameState& dst, const FrameState& src) {
  auto changed = false;

  // Cannot merge irSPOff state, so assert they match.
  always_assert(dst.irSPOff == src.irSPOff);
  always_assert(dst.curFunc == src.curFunc);

  // The only thing that can change the FP is inlining, but we can't have one
  // of the predecessors in an inlined callee while the other isn't.
  always_assert(dst.fpValue == src.fpValue);

  // We must always have the same spValue.
  always_assert(dst.spValue == src.spValue);

  // We must always have the same stublogue mode.
  always_assert(dst.stublogue == src.stublogue);

  if (dst.mbr.ptr != src.mbr.ptr) {
    dst.mbr.ptr = nullptr;
    changed = true;
  }
  changed |= merge_util(dst.mbr.pointee, dst.mbr.pointee | src.mbr.pointee);
  changed |= merge_util(dst.mbr.ptrType, dst.mbr.ptrType | src.mbr.ptrType);

  changed |= merge_into(dst.mbase, src.mbase);

  // Throw away any local information only known at dst.
  for (auto& it : dst.locals) {
    auto const id = it.first;
    auto& dState = it.second;
    if (src.locals.count(id) == 0) {
      dState = LocalState{};
      changed = true;
    }
  }
  // Merge the information from src into dst.
  for (auto& it : src.locals) {
    auto const srcId = it.first;
    auto const& srcState = it.second;
    changed |= merge_into(dst.locals[srcId], srcState);
  }

  changed |= merge_memory_stack_into(dst.stack, src.stack);

  changed |= merge_util(dst.stackModified,
                        dst.stackModified || src.stackModified);

  changed |= merge_util(dst.localsCleared,
                        dst.localsCleared || src.localsCleared);

  // Eval stack depth should be the same at merge points.
  always_assert(dst.bcSPOff == src.bcSPOff);

  for (auto const& srcPair : src.predictedTypes) {
    auto dstIt = dst.predictedTypes.find(srcPair.first);
    if (dstIt == dst.predictedTypes.end()) {
      dst.predictedTypes.emplace(srcPair);
      changed = true;
      continue;
    }

    auto const newType = dstIt->second | srcPair.second;
    if (newType != dstIt->second) {
      dstIt->second = newType;
      changed = true;
    }
  }

  return changed;
}

/*
 * Merge two state-stacks.  The stacks must have the same depth.  Returns
 * whether any states changed.
 */
bool merge_into(jit::vector<FrameState>& dst,
                const jit::vector<FrameState>& src) {
  always_assert(src.size() == dst.size());
  auto changed = false;
  for (auto idx = uint32_t{0}; idx < dst.size(); ++idx) {
    changed |= merge_into(dst[idx], src[idx]);
  }
  return changed;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Recompute a predicted type for when the proven type changes (or when a new
 * prediction is made and we want to discard the old one).
 *
 * This maintains the invariant `predicted <= proven'.
 */
Type updatePrediction(Type predicted, Type proven) {
  return predicted < proven ? predicted : proven;
}

/*
 * Compute the refinement of `oldPredicted' with `newPredicted', maintaining
 * the invariant that `refined <= proven'.
 */
Type refinePrediction(Type oldPredicted, Type newPredicted, Type proven) {
  auto refined = oldPredicted & newPredicted;
  if (refined == TBottom) refined = newPredicted;
  return updatePrediction(refined, proven);
}

///////////////////////////////////////////////////////////////////////////////

}

FrameState::FrameState(const Func* func)
  : curFunc(func)
{}

FrameStateMgr::FrameStateMgr(const Func* func)
  : m_stack{FrameState(func)}
{}

void FrameStateMgr::update(const IRInstruction* inst) {
  ITRACE(3, "FrameStateMgr::update processing {}\n", *inst);
  Indent _i;

  if (auto const taken = inst->taken()) {
    /*
     * TODO(#4323657): we should make this assertion for all non-empty blocks
     * (exits in addition to catches).  It would fail right now for exit
     * traces.
     *
     * If you hit this assertion: you've created a catch block and then
     * modified tracked state, then generated a potentially exception-throwing
     * instruction using that catch block as a target.  This is not allowed.
     */
    if (debug && taken->isCatch()) {
      auto const tmp = save(taken);
      always_assert_flog(
        !tmp,
        "catch block B{} had non-matching in state",
        taken->id()
      );
    }

    // When we're building the IR, we append a conditional jump after
    // generating its target block: see emitJmpCondHelper, where we
    // call makeExit() before gen(JmpZero).  It doesn't make sense to
    // update the target block state at this point, so don't.  The
    // state doesn't have this problem during optimization passes,
    // because we'll always process the jump before the target block.
    if (taken->empty()) save(taken);
  }

  auto killIterLocals = [&](const std::initializer_list<uint32_t>& ids) {
    for (auto id : ids) {
      setValue(loc(id), nullptr);
    }
  };

  assertx(checkInvariants());

  switch (inst->op()) {
  case InlineCall:     trackInlineCall(inst); break;
  case InlineReturn:   trackInlineReturn(); break;
  case StFrameCtx:
    if (cur().fpValue == inst->src(0)) cur().ctx = inst->src(1);
    break;
  case Call:
    {
      auto const extra = inst->extra<Call>();
      // Remove tracked state for the slots for args and the actrec.
      uint32_t numCells = kNumActRecCells + extra->numInputs();
      for (auto i = uint32_t{0}; i < numCells; ++i) {
        setValue(stk(extra->spOffset + i), nullptr);
      }
      // Mark out parameter locations as being at least InitCell
      auto const callee = inst->src(2)->hasConstVal(TFunc)
        ? inst->src(2)->funcVal() : nullptr;
      auto const base = extra->spOffset + numCells;
      for (auto i = uint32_t{0}; i < extra->numOut; ++i) {
        auto const ty = callee && callee->takesInOutParams()
          ? irgen::callOutType(callee, i)
          : TInitCell;
        setType(stk(base + i), ty);
      }
      trackCall();
      // We consider popping an ActRec and args to be synced to memory.
      assertx(cur().bcSPOff == inst->marker().bcSPOff());
      cur().bcSPOff -= numCells;
    }
    break;

  case CallBuiltin:
    {
      auto const extra = inst->extra<CallBuiltin>();
      if (auto const base = extra->retSpOffset) {
        auto const numOut = extra->callee->numInOutParams();
        for (auto i = uint32_t{0}; i < numOut; ++i) {
          auto const ty = irgen::callOutType(extra->callee, i);
          setType(stk(*base + i), ty);
        }
      }
    }
    break;

  case ContEnter:
    trackCall();
    break;

  case DefFP:
    cur().fpValue = inst->dst();
    break;

  case DefFuncEntryFP:
    cur().fpValue = inst->dst();
    cur().ctx = inst->src(3);
    cur().stublogue = false;
    break;

  case EnterPrologue:
    cur().stublogue = true;
    break;

  case RetCtrl:
    uninitStack();
    cur().fpValue = nullptr;
    break;

  case DefFrameRelSP:
  case DefRegSP: {
    auto const data = inst->extra<DefStackData>();
    initStack(inst->dst(), data->irSPOff, data->bcSPOff);
    break;
  }

  case LdMem:
    if (inst->src(0) == cur().mbr.ptr) {
      refinePredictedTmpType(inst->dst(), cur().mbase.predictedType);
      setValue(Location::MBase{}, inst->dst());
    }
    break;

  case StMem:
    // If we ever start using StMem to store to pointers that might be
    // stack/locals, we have to update tracked state here.
    always_assert(!inst->src(0)->type().maybe(TMemToFrameCell));
    always_assert(!inst->src(0)->type().maybe(TMemToStkCell));
    break;

  case LdStk:
    {
      auto const offset = inst->extra<LdStk>()->offset;
      auto const& state = stack(offset);
      refinePredictedTmpType(inst->dst(), state.predictedType);

      // Nearly all callers of setValue() for stack slots represent a
      // modification of the stack, so it sets stackModified. LdStk is the one
      // exception, so we compensate for that here.
      auto oldModified = cur().stackModified;
      setValue(stk(offset), inst->dst());
      cur().stackModified = oldModified;
    }
    break;

  case StStk:
    setValue(stk(inst->extra<StStk>()->offset), inst->src(1));
    break;

  case CheckType:
  case AssertType:
    for (auto& frame : m_stack) {
      for (auto& it : frame.locals) {
        refineValue(it.second, inst->src(0), inst->dst());
      }
      for (auto& it : frame.stack) {
        refineValue(it.second, inst->src(0), inst->dst());
      }
    }
    // MInstrState can only be live for the current frame.
    refineValue(cur().mbase, inst->src(0), inst->dst());
    if (inst->src(0) == cur().mbr.ptr) {
      setMBR(inst->dst());
    }
    break;

  case AssertLoc:
  case CheckLoc: {
    auto const id = inst->extra<LocalId>()->locId;
    refineType(loc(id), inst->typeParam(), TypeSource::makeGuard(inst));
    break;
  }

  case AssertStk:
  case CheckStk:
    refineType(stk(inst->extra<IRSPRelOffsetData>()->offset),
               inst->typeParam(),
               TypeSource::makeGuard(inst));
    break;

  case AssertMBase:
  case CheckMBase:
    refineType(Location::MBase{}, inst->typeParam(),
               TypeSource::makeGuard(inst));
    break;

  case StLoc:
    setValue(loc(inst->extra<LocalId>()->locId), inst->src(1));
    break;

  case LdLoc:
    {
      auto const id = inst->extra<LdLoc>()->locId;
      refinePredictedTmpType(inst->dst(), cur().locals[id].predictedType);
      setValue(loc(id), inst->dst());
    }
    break;

  case EndCatch:
    /*
     * Hitting this means we've messed up with syncing the stack in a catch
     * trace.  If the stack isn't clean or doesn't match the marker's irSPOff,
     * the unwinder won't see what we expect.
     */
    always_assert_flog(
      inst->extra<EndCatch>()->offset.to<SBInvOffset>(cur().irSPOff) ==
        inst->marker().bcSPOff(),
      "EndCatch stack didn't seem right:\n"
      "                 spOff: {}\n"
      "       EndCatch offset: {}\n"
      "        marker's spOff: {}\n",
      cur().irSPOff.offset,
      inst->extra<EndCatch>()->offset.offset,
      inst->marker().bcSPOff().offset
    );
    break;

  case InterpOne:
  case InterpOneCF: {
    auto const& extra = *inst->extra<InterpOneData>();
    assertx(!extra.smashesAllLocals || extra.nChangedLocals == 0);
    if (extra.smashesAllLocals) {
      clearLocals();
    } else {
      auto it = extra.changedLocals;
      auto const end = it + extra.nChangedLocals;
      for (; it != end; ++it) {
        auto& local = *it;
        setType(loc(local.id), local.type);
      }
    }

    // Offset of the bytecode stack top relative to the IR stack pointer.
    auto const bcSPOff = extra.spOffset;

    // Clear tracked information for slots pushed and popped.
    for (auto i = uint32_t{0}; i < extra.cellsPopped; ++i) {
      setValue(stk(bcSPOff + i), nullptr);
    }
    for (auto i = uint32_t{0}; i < extra.cellsPushed; ++i) {
      setValue(stk(bcSPOff + extra.cellsPopped - 1 - i), nullptr);
    }
    auto adjustedTop = bcSPOff + extra.cellsPopped - extra.cellsPushed;

    switch (extra.opcode) {
      case Op::CGetL2:
        setType(stk(adjustedTop + 1), inst->typeParam());
        break;
      default:
        // We don't track cells pushed by interp one except the top of the
        // stack, aside from the above special cases.
        if (inst->hasTypeParam()) {
          auto const instrInfo = getInstrInfo(extra.opcode);
          if (instrInfo.out & InstrFlags::Stack1) {
            setType(stk(adjustedTop), inst->typeParam());
          }
        }
        break;
    }

    cur().bcSPOff += extra.cellsPushed;
    cur().bcSPOff -= extra.cellsPopped;

    if (isMemberBaseOp(extra.opcode) ||
        isMemberDimOp(extra.opcode) ||
        isMemberFinalOp(extra.opcode)) {
      cur().mbr = MBRState{};
      cur().mbase = MBaseState{};
    }
    break;
  }

  case IterInit:
  case LIterInit:
  case IterNext:
  case LIterNext: {
    auto const& args = inst->extra<IterData>()->args;
    assertx(!args.hasKey());
    killIterLocals({safe_cast<uint32_t>(args.valId)});
    break;
  }

  case IterInitK:
  case LIterInitK:
  case IterNextK:
  case LIterNextK: {
    auto const& args = inst->extra<IterData>()->args;
    assertx(args.hasKey());
    killIterLocals({safe_cast<uint32_t>(args.keyId),
                    safe_cast<uint32_t>(args.valId)});
    break;
  }

  case LdMBase:
    setMBR(inst->dst());
    break;

  case StMBase: {
    auto const mbr = inst->src(0);
    setMBR(mbr);
    setValue(Location::MBase{}, nullptr);
    setType(Location::MBase{}, mbr->type().deref());
  } break;

  case FinishMemberOp:
    cur().mbr = MBRState{};
    setValue(Location::MBase{}, nullptr);
    break;

  default:
    break;
  }

  if (MInstrEffects::supported(inst)) {
    updateMInstr(inst);
  } else {
    // Update the mbase state according to the memory effects of `inst'.  We
    // only call this for instructions without minstr effects; those are
    // handled more precisely as part of updateMInstr().
    updateMBase(inst);
  }
}

void FrameStateMgr::updateMInstr(const IRInstruction* inst) {
  // We don't update tracked local types for pseudomains, but we do care about
  // stack types.
  auto const baseTmp = inst->src(minstrBaseIdx(inst->op()));
  if (!baseTmp->type().maybe(TLvalToCell)) return;

  auto const base = pointee(baseTmp);
  auto const mbase = cur().mbr.pointee;

  auto const effect_on = [&] (Type ty) -> Optional<Type> {
    auto const effects = MInstrEffects(inst->op(), ty);
    if (effects.baseTypeChanged || effects.baseValChanged) {
      return effects.baseType;
    }
    return std::nullopt;
  };

  // If we don't know exactly where the base is, we have to be conservative and
  // apply the operation to all locals/stack slots that could be affected.
  auto const apply = [&] (Location l) {
    auto const oldType = typeOf(l);
    auto const maxType = [&] {
      switch (l.tag()) {
        case LTag::Local: return LocalState::default_type();
        case LTag::Stack: return StackState::default_type();
        case LTag::MBase: return MBaseState::default_type();
      }
      not_reached();
    }();

    // Skip locations that disagree with our source on type.
    if (!oldType.derefIfPtr().maybe(baseTmp->type().deref())) return;

    if (maxType <= oldType) {
      // Drop the value and don't bother with precise effects.
      return setType(l, oldType);
    }

    if (auto const baseType = effect_on(oldType)) {
      widenType(l, oldType | *baseType);
    }
  };

  if (base.isSingleLocation()) {
    // When the member base register refers to a single known memory location
    // `l' (with corresponding Ptr type `kind'), we apply the effect of `inst'
    // only to `l'.  Returns the base value type if `inst' had an effect.
    auto const apply_one = [&] (Location l, Ptr kind) -> Optional<Type> {
      auto const oldTy = typeOf(l) & TCell;  // exclude TCls from ptr()
      if (auto const ptrTy = effect_on(oldTy.lval(kind))) {
        auto const baseTy = ptrTy->derefIfPtr();
        setType(l, baseTy);
        return baseTy;
      }
      return std::nullopt;
    };

    if (auto const blocal = base.local()) {
      auto const l = loc(blocal->ids.singleValue());
      apply_one(l, Ptr::Frame);
    }
    if (auto const bstack = base.stack()) {
      assertx(bstack->size() == 1);
      auto const l = stk(bstack->low);
      apply_one(l, Ptr::Stk);
    }

    if (base.maybe(mbase)) {
      if (mbase.isSingleLocation()) {
        auto const ptr = cur().mbr.ptrType.ptrKind();
        apply_one(Location::MBase{}, ptr);
      } else {
        apply(Location::MBase{});
      }
    }

    // We don't track anything else.
    return;
  }

  if (base.maybe(ALocalAny)) {
    for (auto& l : cur().locals) {
      auto const id = l.first;
      if (base.maybe(ALocal { fp(), id })) {
        apply(loc(id));
      }
    }
    // This instruction could also affect locals that aren't being tracked.
    // Be conservative and assume that they could be affected.
    cur().localsCleared = true;
  }
  if (base.maybe(AStackAny)) {
    for (auto& it : cur().stack) {
      // The SBInvOffset of the stack slot is just its 1-indexed slot.
      auto const sbRel = it.first;
      auto const spRel = sbRel.to<IRSPRelOffset>(irSPOff());
      if (base.maybe(AStack::at(spRel))) {
        apply(stk(spRel));
      }
    }
  }
  if (base.maybe(mbase)) {
    apply(Location::MBase{});
  }
}

void FrameStateMgr::updateMBase(const IRInstruction* inst) {
  auto const base = cur().mbr.pointee;

  auto const pessimize_mbase = [&] {
    setValue(Location::MBase{}, nullptr);
  };

  auto const handle_stores = [&] (AliasClass stores) {
    if (!base.maybe(stores)) return;

#define UNTRACKED_ALIAS_CLASSES \
    X(Iter)     \
    X(Prop)     \
    X(ElemI)    \
    X(ElemS)    \
    X(MIState)
#define X(Mem)  \
    (base.maybe(A##Mem##Any) && stores.maybe(A##Mem##Any)) ||

    if (UNTRACKED_ALIAS_CLASSES false) {
      // AliasClass doesn't support intersection yet, so if `base' and `stores'
      // might intersect outside of frame locals and stack slots, we pessimize.
      // FrameState only tracks type information for ALocal and AStack anyway,
      // so we aren't actually losing any information.
      return pessimize_mbase();
    }

#undef X
#undef UNTRACKED_ALIAS_CLASSES

    auto updated = false;

    if (base.maybe(ALocalAny) && stores.maybe(ALocalAny)) {
      for (uint32_t i = 0; i < cur().curFunc->numLocals(); ++i) {
        auto const aloc = ALocal { fp(), i };
        if (base.maybe(aloc) && stores.maybe(aloc)) {
          if (!updated) {
            cur().mbase = local(i);
            cur().mbase.maybeChanged = true;
            updated = true;
          } else {
            merge_into(cur().mbase, local(i));
          }
        }
      }
    }

    if (base.maybe(AStackAny) && stores.maybe(AStackAny)) {
      for (auto sbRel = bcSPOff(); sbRel > SBInvOffset{0}; sbRel--) {
        auto const spRel = sbRel.to<IRSPRelOffset>(irSPOff());
        auto const astk = AStack::at(spRel);

        if (base.maybe(astk) && stores.maybe(astk)) {
          if (!updated) {
            cur().mbase = stack(sbRel);
            cur().mbase.maybeChanged = true;
            updated = true;
          } else {
            merge_into(cur().mbase, stack(sbRel));
          }
        }
      }
    }
  };

  auto const effects = memory_effects(*inst);
  match<void>(effects, [&](GeneralEffects m) { handle_stores(m.stores); },
              [&](PureStore m) { handle_stores(m.dst); },
              [&](CallEffects x) {
                handle_stores(x.kills);
                handle_stores(x.inputs);
                handle_stores(x.actrec);
                handle_stores(x.outputs);
              },
              [&](PureLoad /*m*/) {}, [&](ReturnEffects) {},
              [&](ExitEffects) {}, [&](IrrelevantEffects) {},
              [&](PureInlineCall) {}, [&](PureInlineReturn) {},
              [&](UnknownEffects) { pessimize_mbase(); });
}

///////////////////////////////////////////////////////////////////////////////

/*
 * syncPrediction() is called after we update the predictedType and/or value
 * for a LocationState. It looks up the predicted type for the value in
 * cur().predictedTypes and ensures both locations have the most refined
 * predicted type possible.
 */
template<LTag tag>
void FrameStateMgr::syncPrediction(LocationState<tag>& state) {
  if (!state.value) return;
  ITRACE(3, "Syncing prediction for {}\n", *state.value);
  auto const canonValue = canonical(state.value);

  auto& prediction = state.predictedType;
  auto& map = cur().predictedTypes;
  auto it = map.find(canonValue);
  if (it == map.end()) {
    ITRACE(4, "No prediction in map; state has {}\n", prediction);
    if (prediction < state.default_type()) map.emplace(canonValue, prediction);
    return;
  }
  if (prediction == state.default_type()) {
    ITRACE(4, "No prediction in state; map has {}\n", it->second);
    prediction = it->second;
    return;
  }

  auto const newPred = prediction & it->second;
  ITRACE(3, "New prediction: {} & {} -> {}\n",
         prediction, it->second, newPred);
  if (newPred == TBottom) return;
  if (newPred < prediction) prediction = newPred;
  if (newPred < it->second) it->second = newPred;
}

/*
 * Collects the post-conditions associated with the current state, which is
 * essentially a list of local/stack locations and their known types at the end
 * of a block.
 */
PostConditions FrameStateMgr::collectPostConds() {
  PostConditions postConds;

  if (sp() != nullptr) {
    for (auto& it : cur().stack) {
      auto const sbRel = it.first;
      auto& state = it.second;
      auto const type = state.type;
      auto const changed = state.maybeChanged;

      if (changed || type < TCell) {
        FTRACE(1, "Stack({}, {}): {} ({})\n",
               sbRel.to<BCSPRelOffset>(bcSPOff()).offset, sbRel.offset,
               type, changed ? "changed" : "refined");
        auto& vec = changed ? postConds.changed : postConds.refined;
        vec.push_back({ Location::Stack{sbRel}, type });
      }
    }
  }

  if (fp() != nullptr) {
    for (auto& l : cur().locals) {
      auto id = l.first;
      auto& state = l.second;
      auto const type = state.type;
      auto const changed = state.maybeChanged;
      if (changed || type < TCell) {
        FTRACE(1, "Local {}: {} ({})\n", id, type.toString(),
               changed ? "changed" : "refined");
        auto& vec = changed ? postConds.changed : postConds.refined;
        vec.push_back({ Location::Local{id}, type });
      }
    }
  }

  auto const ty = mbase().type;
  auto const changed = mbase().maybeChanged;
  if (changed || ty < TCell) {
    FTRACE(1, "MBase{{}}: {} ({})\n", ty, changed ? "changed" : "refined");
    auto& vec = changed ? postConds.changed : postConds.refined;
    vec.push_back({ Location::MBase{}, ty });
  }

  return postConds;
}

///////////////////////////////////////////////////////////////////////////////
// Per-block state.

bool FrameStateMgr::hasStateFor(Block* block) const {
  return m_states.count(block);
}

void FrameStateMgr::startBlock(Block* block, bool hasUnprocessedPred) {
  ITRACE(3, "FrameStateMgr::startBlock: {}\n", block->id());
  auto const it = m_states.find(block);
  auto const end = m_states.end();

  if (it != end) {
    always_assert_flog(
      block->empty(),
      "tried to startBlock a non-empty block while building\n"
    );
    ITRACE(4, "Loading state for B{}: {}\n", block->id(), show());
    m_stack = it->second.in;
    if (m_stack.empty()) {
      always_assert_flog(0, "invalid startBlock for B{}", block->id());
    }
  } else if (debug) {
    // NOTE: Highly suspect; different debug vs. non-debug behavior.
    save(block, nullptr);
  }
  assertx(!m_stack.empty());

  // Reset state if the block has any predecessor that we haven't processed yet.
  if (hasUnprocessedPred) {
    Indent _;
    ITRACE(4, "B{} is a loop header; resetting state\n", block->id());
    clearForUnprocessedPred();
  }
}

bool FrameStateMgr::finishBlock(Block* block) {
  assertx(block->back().isTerminal() == !block->next());

  if (block->isExitNoThrow()) {
    m_exitPostConds[block] = collectPostConds();
    FTRACE(2, "PostConditions for exit Block {}:\n{}\n",
           block->id(), jit::show(m_exitPostConds[block]));
  }

  assertx(hasStateFor(block));
  if (m_states[block].out) {
    assertx(m_states[block].out->empty());
    m_states[block].out = m_stack;
  }

  auto changed = false;
  if (!block->back().isTerminal()) changed |= save(block->next());
  return changed;
}

void FrameStateMgr::setSaveOutState(Block* block) {
  assertx(hasStateFor(block));
  assertx(!m_states[block].out || m_states[block].out->empty());
  m_states[block].out.emplace();
}

void FrameStateMgr::pauseBlock(Block* block) {
  // Note: this can't use std::move, because pauseBlock must leave the current
  // state alone so startBlock can use it as the in state for another block.
  m_states[block].paused = m_stack;
}

void FrameStateMgr::unpauseBlock(Block* block) {
  assertx(hasStateFor(block));
  m_stack = *m_states[block].paused;
}

void FrameStateMgr::resetBlock(Block* block, Block* pred) {
  assertx(m_states[pred].out && !m_states[pred].out->empty());
  m_states[block].in = *m_states[pred].out;
}

const PostConditions& FrameStateMgr::postConds(Block* exitBlock) const {
  assertx(exitBlock->isExitNoThrow());
  auto it = m_exitPostConds.find(exitBlock);
  assertx(it != m_exitPostConds.end());
  auto& pconds = it->second;
  if (debug) {
    for (DEBUG_ONLY auto& c : pconds.changed) {
      for (DEBUG_ONLY auto& r : pconds.refined) {
        assert_flog(c.location != r.location,
                    "Location {} in both changed and refined sets",
                    jit::show(c.location));
      }
    }
  }
  return pconds;
}

/*
 * Save the current state as the in-state for `block'.
 *
 * If this is the first time saving state for `block', create a new snapshot
 * using either the current state or the out-state of `pred' if we are given
 * one.  Otherwise merge the current state into the existing snapshot.
 */
bool FrameStateMgr::save(Block* block, Block* pred) {
  ITRACE(4, "Saving current state to B{}: {}\n", block->id(), show());

  // If the destination block is unreachable, there's no need to merge in the
  // frame state.
  if (block->isUnreachable()) return false;

  auto const it = m_states.find(block);
  auto changed = true;

  if (it != m_states.end()) {
    changed = merge_into(it->second.in, m_stack);
    ITRACE(4, "Merged state: {}\n", show());
  } else {
    if (pred) {
      assertx(hasStateFor(pred));
      assertx(m_states[pred].out);
      assertx(!m_states[pred].out->empty());

      m_states[block].in = *m_states[pred].out;
    } else {
      assertx(!m_stack.empty());
      m_states[block].in = m_stack;
    }
  }

  return changed;
}

/*
 * Modify state to conservative values given an unprocessed predecessor.
 *
 * The fpValue, irSPOff, and curFunc are not cleared because they must agree
 * at bytecode-level control-flow merge points (which can be either merge
 * points at the bytecode or due to retranslated blocks).
 */
void FrameStateMgr::clearForUnprocessedPred() {
  FTRACE(1, "clearForUnprocessedPred\n");

  // Forget any information about stack values in memory.
  for (auto& it : cur().stack) {
    it.second = StackState{};
  }

  // These values must go toward their conservative state.
  cur().mbr = MBRState{};
  cur().mbase = MBaseState{};

  clearLocals();
}

///////////////////////////////////////////////////////////////////////////////

void FrameStateMgr::initStack(SSATmp* sp, SBInvOffset irSPOff,
                              SBInvOffset bcSPOff) {
  cur().spValue = sp;
  cur().irSPOff = irSPOff;
  cur().bcSPOff = bcSPOff;
  cur().stack.clear();
}

void FrameStateMgr::uninitStack() {
  cur().spValue = nullptr;
  cur().irSPOff = SBInvOffset{0};
  cur().bcSPOff = SBInvOffset{0};
  cur().stack.clear();
}

void FrameStateMgr::trackInlineCall(const IRInstruction* inst) {
  assertx(inst->src(0)->inst()->is(BeginInlining));
  auto const extra      = inst->extra<InlineCall>();
  auto const extraBI    = inst->src(0)->inst()->extra<BeginInlining>();
  auto const target     = extraBI->func;
  auto const calleeFP   = inst->src(0);
  auto const savedSPOff =
    extra->spOffset.to<SBInvOffset>(irSPOff()) - kNumActRecCells;

  /*
   * Push a new state for the inlined callee; saving the state we'll need to
   * pop on return.
   */
  for (auto i = uint32_t{0}; i < kNumActRecCells; ++i) {
    setValue(stk(extra->spOffset + i), nullptr);
  }
  cur().bcSPOff = savedSPOff;

  m_stack.emplace_back(FrameState{target});

  // Set up the callee's frame.
  cur().fpValue = calleeFP;

  /*
   * Set up the callee's stack.
   *
   * We need to calculate the new `irSPOff`, which is an offset of `spValue`
   * from the new stack base. It consists of two parts:
   *
   * - the inverse offset of the new `fpValue` from the new stack base, given by
   *   `-target->numSlotsInFrame()`
   * - the inverse offset of `spValue` from the new `fpValue`, which is the same
   *   numeric value as a regular offset of `fpValue` from `spValue`, given by
   *   `extra->spOffset`
   *
   * The callee's stack starts empty, so `bcSPOff` is zero.
   */
  auto const irSPOff =
    SBInvOffset{extra->spOffset.offset - target->numSlotsInFrame()};
  auto const bcSPOff = SBInvOffset{0};
  initStack(caller().spValue, irSPOff, bcSPOff);
  FTRACE(6, "InlineCall setting irSPOff: {}\n", irSPOff.offset);

  // Copy the type predictions.
  cur().predictedTypes = caller().predictedTypes;
}

void FrameStateMgr::trackInlineReturn() {
  // Inlining may not change spValue
  assertx(caller().spValue == cur().spValue);

  // Copy back the type predictions, as we may have refined types further.
  caller().predictedTypes = std::move(cur().predictedTypes);

  // Pop the inlined frame.
  m_stack.pop_back();
  assertx(!m_stack.empty());
}

/*
 * Clear the tracked values at a Call instruction.
 *
 * Keeping a value live across a Call requires spilling, so we avoid it---but
 * we do continue keeping track of types.
 */
void FrameStateMgr::trackCall() {
  for (auto& state : m_stack) {
    for (auto& loc : state.locals) {
      auto& lState = loc.second;
      if (lState.value && !lState.value->inst()->is(DefConst)) {
        lState.value = nullptr;
      }
    }
    for (auto& it : state.stack) {
      auto& sState = it.second;
      sState.value = nullptr;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

bool FrameState::checkInvariants() const {
  for (auto& it : locals) {
    auto const id = it.first;
    auto const& local = it.second;

    always_assert_flog(
      local.predictedType <= local.type,
      "local {} failed prediction invariants; pred = {}, type = {}\n",
      id,
      local.predictedType,
      local.type
    );

    always_assert_flog(
      local.value == nullptr || local.value->type() == local.type,
      "local {} had type {}, but value {}\n",
      id,
      local.type,
      local.value->toString()
    );
  }

  // Make sure the stack is either initialized or read-only empty.
  always_assert_flog(
    spValue != nullptr || (irSPOff.offset == 0 && bcSPOff.offset == 0),
    "incorrectly initialized stack"
  );

  return true;
}

bool FrameStateMgr::checkInvariants() const {
  for (auto& state : m_stack) {
    always_assert(state.checkInvariants());
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Wrap a local or stack ID into a Location.
 */
Location FrameStateMgr::loc(uint32_t id) const {
  return Location::Local { id };
}
Location FrameStateMgr::stk(IRSPRelOffset off) const {
  auto const sbRel = off.to<SBInvOffset>(irSPOff());
  return Location::Stack { sbRel };
}

LocalState& FrameStateMgr::localState(uint32_t id) {
  assertx(id < cur().curFunc->numLocals());
  auto& ret = cur().locals[id];

  assertx(ret.value == nullptr || ret.value->type() == ret.type);
  return ret;
}

LocalState& FrameStateMgr::localState(Location l) {
  assertx(l.tag() == LTag::Local);
  return localState(l.localId());
}

StackState& FrameStateMgr::stackState(IRSPRelOffset spRel) {
  auto const sbRel = spRel.to<SBInvOffset>(irSPOff());
  return stackState(sbRel);
}

StackState& FrameStateMgr::stackState(SBInvOffset sbRel) {
  always_assert_flog(
    sbRel.offset >= 1,
    "stack sbRel.offset went below 1: irSPOff: {}, sbRel: {}\n",
    cur().irSPOff.offset,
    sbRel.offset
  );

  auto& ret = cur().stack[sbRel];
  assertx(ret.value == nullptr || ret.value->type() == ret.type);
  return ret;
}

StackState& FrameStateMgr::stackState(Location l) {
  assertx(l.tag() == LTag::Stack);
  return stackState(l.stackIdx());
}

LocalState FrameStateMgr::local(uint32_t id) const {
  auto const& locals = cur().locals;
  auto const it = locals.find(id);
  return it != locals.end() ? it->second : LocalState{};
}

bool FrameStateMgr::tracked(Location l) const {
  switch (l.tag()) {
    case LTag::Local:
      return cur().locals.count(l.localId()) > 0;
    case LTag::Stack: {
      auto const sbRel = l.stackIdx();
      return cur().stack.count(sbRel) > 0;
    }
    case LTag::MBase:
      return true;
  }
  not_reached();
}

/*
 * We consider it logically const to extend with default-constructed stack
 * values.
 */
StackState FrameStateMgr::stack(IRSPRelOffset offset) const {
  auto const sbRel = offset.to<SBInvOffset>(irSPOff());
  return stack(sbRel);
}

StackState FrameStateMgr::stack(SBInvOffset offset) const {
  auto const& curStack = cur().stack;
  auto const it = curStack.find(offset);
  return it != curStack.end() ? it->second : StackState{};
}

#define IMPL_MEMBER_OF(type_t, name)                           \
  type_t FrameStateMgr::name##Of(Location l) const {           \
    return [&]() -> type_t {                                   \
      switch (l.tag()) {                                       \
        case LTag::Local:    return local(l.localId()).name;   \
        case LTag::Stack:    return stack(l.stackIdx()).name;  \
        case LTag::MBase:    return mbase().name;              \
      }                                                        \
      not_reached();                                           \
    }();                                                       \
  }

IMPL_MEMBER_OF(SSATmp*, value)
IMPL_MEMBER_OF(Type, type)
IMPL_MEMBER_OF(Type, predictedType)
IMPL_MEMBER_OF(TypeSourceSet, typeSrcs)

#undef IMPL_MEMBER_AT

///////////////////////////////////////////////////////////////////////////////

template<LTag tag>
void FrameStateMgr::setValueImpl(Location l,
                                 LocationState<tag>& state,
                                 SSATmp* value,
                                 Optional<Type> predicted) {
  FTRACE(2, "{} := {}\n", jit::show(l), value ? value->toString() : "<>");
  state.value = value;
  state.type = value ? value->type() : LocationState<tag>::default_type();
  state.maybeChanged = true;

  state.predictedType = [&] {
    if (predicted) {
      return refinePrediction(state.type, *predicted, state.type);
    } else {
      return state.type;
    }
  }();
  syncPrediction(state);

  state.typeSrcs.clear();
  if (value) {
    state.typeSrcs.insert(TypeSource::makeValue(value));
  }
}

/*
 * Update the value (and type) for `l'.
 */
void FrameStateMgr::setValue(Location l, SSATmp* value) {
  switch (l.tag()) {
    case LTag::Local:
      return setValueImpl(l, localState(l), value);
    case LTag::Stack:
      cur().stackModified = true;
      return setValueImpl(l, stackState(l), value);
    case LTag::MBase:
      return setValueImpl(l, cur().mbase, value);
  }
  not_reached();
}

template<LTag tag>
static void setTypeImpl(Location l, LocationState<tag>& state, Type type) {
  FTRACE(2, "{} :: {} -> {}\n", jit::show(l), state.type, type);
  state.value = nullptr;
  state.type = type;
  state.predictedType = type;
  state.maybeChanged = true;
  state.typeSrcs.clear();
}

/*
 * Update the type for `l' to reflect a possible change in the value---but when
 * we don't have that value.
 *
 * Setting the type clears the typeSrcs, so the new type may not be derived
 * from the old type in any way.
 */
void FrameStateMgr::setType(Location l, Type type) {
  switch (l.tag()) {
    case LTag::Local:
      return setTypeImpl(l, localState(l), type);
    case LTag::Stack:
      cur().stackModified = true;
      return setTypeImpl(l, stackState(l), type);
    case LTag::MBase:
      return setTypeImpl(l, cur().mbase, type);
  }
  not_reached();
}

template<LTag tag>
static void widenTypeImpl(Location l, LocationState<tag>& state, Type type) {
  FTRACE(2, "{} :: {} -> {}\n", jit::show(l), state.type, type);
  state.value = nullptr;
  state.type = type;
  state.predictedType = type;
  state.maybeChanged = true;
}

/*
 * Update the type for `l' as a result of an operation that might change the
 * value.
 *
 * This is just like setType() except that typeSrcs are preserved, so the new
 * type may be derived from the old type.
 */
void FrameStateMgr::widenType(Location l, Type type) {
  switch (l.tag()) {
    case LTag::Local:
      return widenTypeImpl(l, localState(l), type);
    case LTag::Stack:
      cur().stackModified = true;
      return widenTypeImpl(l, stackState(l), type);
    case LTag::MBase:
      return widenTypeImpl(l, cur().mbase, type);
  }
  not_reached();
}

template<LTag tag>
static void refineTypeImpl(Location l, LocationState<tag>& state,
                           Type type, TypeSource typeSrc) {
  auto const refined = state.type & type;
  FTRACE(2, "{} :: {} -> {} (via {})\n",
         jit::show(l), state.type, refined, type);

  // If the type gets more refined, we need to forget the old value, or else we
  // may end up using a value with a more general type than is known about the
  // stack slot.
  if (refined != state.type) state.value = nullptr;
  state.type = refined;
  state.predictedType = updatePrediction(state.predictedType, refined);
  state.typeSrcs.clear();
  state.typeSrcs.insert(typeSrc);
}

/*
 * Update the type for `l' to reflect new information that we've obtained from
 * guards, assertions, or the like.
 *
 * A type refinement does /not/ indicate a change in value, so the various
 * changed flags are not touched.
 */
void FrameStateMgr::refineType(Location l, Type type, TypeSource typeSrc) {
  switch (l.tag()) {
    case LTag::Local: return refineTypeImpl(l, localState(l), type, typeSrc);
    case LTag::Stack: return refineTypeImpl(l, stackState(l), type, typeSrc);
    case LTag::MBase: return refineTypeImpl(l, cur().mbase, type, typeSrc);
  }
  not_reached();
}

template<LTag tag>
void FrameStateMgr::refinePredictedTypeImpl(LocationState<tag>& state,
                                            Type type) {
  state.predictedType = refinePrediction(
    state.predictedType,
    type,
    state.type
  );
  syncPrediction(state);
}

/*
 * Update the predicted type for `l'.
 */
void FrameStateMgr::refinePredictedType(Location l, Type type) {
  switch (l.tag()) {
    case LTag::Local: return refinePredictedTypeImpl(localState(l), type);
    case LTag::Stack: return refinePredictedTypeImpl(stackState(l), type);
    case LTag::MBase: return refinePredictedTypeImpl(cur().mbase, type);
  }
  not_reached();
}

/*
 * Refine the value for `state' to `newVal' if it was set to `oldVal'.
 *
 * This function refines, rather than invalidates, the old prediction.
 */
template<LTag tag>
void FrameStateMgr::refineValue(LocationState<tag>& state,
                                SSATmp* oldVal, SSATmp* newVal) {
  if (!state.value || canonical(state.value) != canonical(oldVal)) {
    return;
  }
  FTRACE(2, "refining value: {} -> {}\n", *state.value, *newVal);

  state.value = newVal;
  state.type = newVal->type();
  state.predictedType = updatePrediction(state.predictedType, state.type);
  state.typeSrcs.clear();
  state.typeSrcs.insert(TypeSource::makeValue(newVal));
}

void FrameStateMgr::refinePredictedTmpType(SSATmp* tmp, Type predicted) {
  auto const canon = canonical(tmp);
  auto& map = cur().predictedTypes;
  auto it = map.find(canon);
  if (it == map.end()) {
    map.emplace(canon, predicted);
    FTRACE(3, "New prediction for {}: {}\n", *tmp->inst(), predicted);
    return;
  }

  FTRACE(3, "Prediction for {} refined from {} to ", *tmp->inst(), it->second);
  it->second = refinePrediction(it->second, predicted, tmp->type());
  FTRACE(3, "{}\n", it->second);
}

///////////////////////////////////////////////////////////////////////////////

void FrameStateMgr::setLocalPredictedType(uint32_t id, Type type) {
  always_assert(id < cur().curFunc->numLocals());
  auto& local = cur().locals[id];
  ITRACE(2, "updating local {}'s type prediction: {} -> {}\n",
    id, local.predictedType, type & local.type);
  local.predictedType = updatePrediction(type, local.type);
}

void FrameStateMgr::clearLocals() {
  ITRACE(2, "clearLocals\n");
  for (auto& it : cur().locals) {
    auto const id = it.first;
    setValue(loc(id), nullptr);
  }
  cur().localsCleared = true;
}

///////////////////////////////////////////////////////////////////////////////

void FrameStateMgr::setMemberBase(SSATmp* base) {
  setValueImpl(Location::MBase{}, cur().mbase, base, std::nullopt);
}

void FrameStateMgr::setMBR(SSATmp* mbr) {
  cur().mbr.ptr = mbr;
  cur().mbr.pointee = pointee(mbr);
  cur().mbr.ptrType = mbr->type();
}

///////////////////////////////////////////////////////////////////////////////

std::string FrameStateMgr::show() const {
  auto const funcName = cur().curFunc->fullName();
  if (sp() == nullptr) {
    return folly::sformat("func: {}, stack uninit", funcName);
  }

  return folly::sformat(
    "func: {}, irSPOff: {}, bcSPOff: {}",
    funcName, irSPOff().offset, bcSPOff().offset
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
