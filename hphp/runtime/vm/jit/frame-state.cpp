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
#include <folly/Optional.h>

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

bool merge_memory_stack_into(jit::vector<StackState>& dst,
                             const jit::vector<StackState>& src) {
  auto changed = false;
  // We may need to merge different-sized memory stacks, because a predecessor
  // may not touch some stack memory that another pred did.  We just need to
  // conservatively throw away slots that aren't tracked on all preds.
  auto const result_size = std::min(dst.size(), src.size());
  dst.resize(result_size);
  for (auto i = uint32_t{0}; i < result_size; ++i) {
    changed |= merge_into(dst[i], src[i]);
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

  // FrameState for the same function must always have the same number of
  // locals.
  always_assert(src.locals.size() == dst.locals.size());

  // We must always have the same spValue.
  always_assert(dst.spValue == src.spValue);

  // We must always have the same stublogue mode.
  always_assert(dst.stublogue == src.stublogue);

  if (dst.needRatchet != src.needRatchet) {
    dst.needRatchet = true;
    changed = true;
  }

  if (dst.mbr.ptr != src.mbr.ptr) {
    dst.mbr.ptr = nullptr;
    changed = true;
  }
  changed |= merge_util(dst.mbr.pointee, dst.mbr.pointee | src.mbr.pointee);
  changed |= merge_util(dst.mbr.ptrType, dst.mbr.ptrType | src.mbr.ptrType);

  changed |= merge_into(dst.mbase, src.mbase);

  for (auto i = uint32_t{0}; i < src.locals.size(); ++i) {
    changed |= merge_into(dst.locals[i], src.locals[i]);
  }

  changed |= merge_memory_stack_into(dst.stack, src.stack);

  changed |= merge_util(dst.stackModified,
                        dst.stackModified || src.stackModified);

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
bool merge_into(jit::vector<FrameState>& dst, const jit::vector<FrameState>& src) {
  always_assert(src.size() == dst.size());
  auto changed = false;
  for (auto idx = uint32_t{0}; idx < dst.size(); ++idx) {
    changed |= merge_into(dst[idx], src[idx]);
  }
  return changed;
}

///////////////////////////////////////////////////////////////////////////////

bool check_invariants(const FrameState& state) {
  for (auto id = uint32_t{0}; id < state.locals.size(); ++id) {
    auto const& local = state.locals[id];

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

    if (state.curFunc->isPseudoMain()) {
      always_assert_flog(
        local.value == nullptr,
        "We should never be tracking values for locals in a pseudomain "
          "right now.  Local {} had value {}",
        id,
        local.value->toString()
      );
      always_assert_flog(
        local.type == TCell,
        "We should never be tracking non-predicted types for locals in "
          "a pseudomain right now.  Local {} had type {}",
        id,
        local.type.toString()
      );
    }
  }

  // We require the memory stack is always at least as big as the irSPOff,
  // unless irSPOff went negative (because we're returning and have freed the
  // ActRec).  Note that there are some "wasted" slots where locals/iterators
  // would be in the vector right now.
  always_assert_flog(
    state.irSPOff < FPInvOffset{0} ||
    state.stack.size() >= state.irSPOff.offset,
    "stack was smaller than possible"
  );

  return true;
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

FrameStateMgr::FrameStateMgr(BCMarker marker) {
  m_stack.push_back(FrameState{});
  cur().curFunc = marker.func();
  cur().irSPOff = marker.spOff();
  cur().bcSPOff = marker.spOff();
  cur().locals.resize(marker.func()->numLocals());
  cur().stack.resize(marker.spOff().offset);
}

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
  case DefInlineFP:    trackDefInlineFP(inst);  break;
  case InlineSuspend:
  case InlineReturn:   trackInlineReturn(); break;
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
      assertx(cur().bcSPOff == inst->marker().spOff());
      cur().bcSPOff -= numCells;
    }
    break;

  case CallUnpack:
    {
      auto const extra = inst->extra<CallUnpack>();
      // Remove tracked state for the actrec and array arg.
      uint32_t numCells = kNumActRecCells + extra->numInputs();
      for (auto i = uint32_t{0}; i < numCells; ++i) {
        setValue(stk(extra->spOffset + i), nullptr);
      }
      // Mark out parameter locations as being at least InitCell
      auto const base = extra->spOffset + numCells;
      for (auto i = uint32_t{0}; i < extra->numOut; ++i) {
        setType(stk(base + i), TInitCell);
      }
      trackCall();
      // A CallUnpack pops the ActRec, actual args, and an array arg.
      assertx(cur().bcSPOff == inst->marker().spOff());
      cur().bcSPOff -= numCells;
    }
    break;

  case CallBuiltin:
    {
      auto const extra = inst->extra<CallBuiltin>();
      auto const base = extra->retSpOffset;
      auto const numOut = extra->callee->numInOutParams();
      for (auto i = uint32_t{0}; i < numOut; ++i) {
        auto const ty = irgen::callOutType(extra->callee, i);
        setType(stk(base + i), ty);
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
    cur().ctx = inst->src(4);
    cur().stublogue = false;
    break;

  case EnterPrologue:
    cur().stublogue = true;
    break;

  case RetCtrl:
    cur().spValue = nullptr;
    cur().fpValue = nullptr;
    break;

  case DefFrameRelSP:
  case DefRegSP:
    cur().spValue = inst->dst();
    cur().irSPOff = inst->extra<FPInvOffsetData>()->offset;
    break;

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
  case CheckVArray:
  case CheckDArray:
    for (auto& frame : m_stack) {
      for (auto& state : frame.locals) {
        refineValue(state, inst->src(0), inst->dst());
      }
      for (auto& state : frame.stack) {
        refineValue(state, inst->src(0), inst->dst());
      }
      refineValue(frame.mbase, inst->src(0), inst->dst());
    }
    break;

  case AssertLoc:
  case CheckLoc: {
    auto const id = inst->extra<LocalId>()->locId;
    if (inst->marker().func()->isPseudoMain()) {
      setLocalPredictedType(id, inst->typeParam());
    } else {
      refineType(loc(id), inst->typeParam(), TypeSource::makeGuard(inst));
    }
  } break;

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

  case StLocPseudoMain:
    setLocalPredictedType(inst->extra<LocalId>()->locId,
                          inst->src(1)->type());
    break;

  case EndCatch:
    /*
     * Hitting this means we've messed up with syncing the stack in a catch
     * trace.  If the stack isn't clean or doesn't match the marker's irSPOff,
     * the unwinder won't see what we expect.
     */
    always_assert_flog(
      inst->extra<EndCatch>()->offset.to<FPInvOffset>(cur().irSPOff) ==
        inst->marker().spOff(),
      "EndCatch stack didn't seem right:\n"
      "                 spOff: {}\n"
      "       EndCatch offset: {}\n"
      "        marker's spOff: {}\n",
      cur().irSPOff.offset,
      inst->extra<EndCatch>()->offset.offset,
      inst->marker().spOff().offset
    );
    break;

  case InterpOne:
  case InterpOneCF: {
    auto const& extra = *inst->extra<InterpOneData>();
    assertx(!extra.smashesAllLocals || extra.nChangedLocals == 0);
    if (extra.smashesAllLocals || inst->marker().func()->isPseudoMain()) {
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

  case LdMBase: {
    auto const mbr = inst->dst();
    cur().mbr.ptr = mbr;
    cur().mbr.pointee = pointee(mbr);
    cur().mbr.ptrType = mbr->type();
  } break;

  case StMBase: {
    auto const mbr = inst->src(0);
    cur().mbr.ptr = mbr;
    cur().mbr.pointee = pointee(mbr);
    cur().mbr.ptrType = mbr->type();
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
  auto const isPM = cur().curFunc->isPseudoMain();

  auto const baseTmp = inst->src(minstrBaseIdx(inst->op()));
  if (!baseTmp->type().maybe(TLvalToCell)) return;

  auto const base = pointee(baseTmp);
  auto const mbase = cur().mbr.pointee;

  auto const effect_on = [&] (Type ty) -> folly::Optional<Type> {
    auto const effects = MInstrEffects(inst->op(), ty);
    if (effects.baseTypeChanged || effects.baseValChanged) {
      return effects.baseType;
    }
    return folly::none;
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
          always_assert(false); // Can't be a base
      }
      not_reached();
    }();

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
    auto const apply_one = [&] (Location l, Ptr kind) -> folly::Optional<Type> {
      auto const oldTy = typeOf(l) & TCell;  // exclude TCls from ptr()
      if (auto const ptrTy = effect_on(oldTy.lval(kind))) {
        auto const baseTy = ptrTy->derefIfPtr();
        setType(l, baseTy);
        return baseTy;
      }
      return folly::none;
    };

    if (auto const bframe = base.frame()) {
      if (!isPM) {
        auto const l = loc(bframe->ids.singleValue());
        apply_one(l, Ptr::Frame);
      }
    }
    if (auto const bstack = base.stack()) {
      assertx(bstack->size == 1);
      auto const l = stk(bstack->offset.to<IRSPRelOffset>(irSPOff()));
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

  if (base.maybe(AFrameAny) && !isPM) {
    for (uint32_t i = 0; i < cur().locals.size(); ++i) {
      if (base.maybe(AFrame { fp(), i })) {
        apply(loc(i));
      }
    }
  }
  if (base.maybe(AStackAny)) {
    for (auto i = 0; i < cur().stack.size(); ++i) {
      // The FPInvOffset of the stack slot is just its 1-indexed slot.
      auto const fpRel = -FPInvOffset{i + 1};
      if (base.maybe(AStack { fpRel, 1 })) {
        apply(stk(fpRel.to<IRSPRelOffset>(irSPOff())));
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
    X(IterPos)  \
    X(IterBase) \
    X(Prop)     \
    X(ElemI)    \
    X(ElemS)    \
    X(MIState)
#define X(Mem)  \
    (base.maybe(A##Mem##Any) && stores.maybe(A##Mem##Any)) ||

    if (UNTRACKED_ALIAS_CLASSES false) {
      // AliasClass doesn't support intersection yet, so if `base' and `stores'
      // might intersect outside of frame locals and stack slots, we pessimize.
      // FrameState only tracks type information for AFrame and AStack anyway,
      // so we aren't actually losing any information.
      return pessimize_mbase();
    }

#undef X
#undef UNTRACKED_ALIAS_CLASSES

    auto updated = false;

    if (base.maybe(AFrameAny) && stores.maybe(AFrameAny)) {
      for (uint32_t i = 0; i < cur().locals.size(); ++i) {
        auto const aloc = AFrame { fp(), i };
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
      for (auto i = 0; i < cur().stack.size(); ++i) {
        auto const fpRel = -FPInvOffset{i + 1};
        auto const astk = AStack { fpRel, 1 };

        if (base.maybe(astk) && stores.maybe(astk)) {
          if (!updated) {
            cur().mbase = stack(-fpRel);
            cur().mbase.maybeChanged = true;
            updated = true;
          } else {
            merge_into(cur().mbase, stack(-fpRel));
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
              [&](InlineEnterEffects x) { handle_stores(x.actrec); },
              [&](InlineExitEffects) {},
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
 * of `block'.
 */
void FrameStateMgr::collectPostConds(Block* block) {
  assertx(block->isExitNoThrow());
  PostConditions& pConds = m_exitPostConds[block];
  pConds.changed.clear();
  pConds.refined.clear();

  if (sp() != nullptr) {
    auto const& lastInst = block->back();
    auto const bcSPOff = lastInst.marker().spOff();
    auto const resumed = lastInst.marker().resumeMode() != ResumeMode::None;
    auto const skipCells = FPInvOffset{resumed ? 0 : func()->numSlotsInFrame()};
    auto const evalStkCells = bcSPOff - skipCells;

    for (int32_t i = 0; i < evalStkCells; i++) {
      auto const bcSPRel = BCSPRelOffset{i};
      auto const irSPRel = bcSPRel
        .to<FPInvOffset>(bcSPOff)
        .to<IRSPRelOffset>(irSPOff());

      auto const type    = stack(irSPRel).type;
      auto const changed = stack(irSPRel).maybeChanged;

      if (changed || type < TCell) {
        auto const fpRel = bcSPRel.to<FPInvOffset>(bcSPOff);

        FTRACE(1, "Stack({}, {}): {} ({})\n", bcSPRel.offset, fpRel.offset,
               type, changed ? "changed" : "refined");
        auto& vec = changed ? pConds.changed : pConds.refined;
        vec.push_back({ Location::Stack{fpRel}, type });
      }
    }
  }

  if (fp() != nullptr) {
    for (unsigned i = 0; i < func()->numLocals(); i++) {
      auto const t = local(i).type;
      auto const changed = local(i).maybeChanged;
      if (changed || t < TCell) {
        FTRACE(1, "Local {}: {} ({})\n", i, t.toString(),
               changed ? "changed" : "refined");
        auto& vec = changed ? pConds.changed : pConds.refined;
        vec.push_back({ Location::Local{i}, t });
      }
    }
  }

  auto const ty = mbase().type;
  auto const changed = mbase().maybeChanged;
  if (changed || ty < TCell) {
    FTRACE(1, "MBase{{}}: {} ({})\n", ty, changed ? "changed" : "refined");
    auto& vec = changed ? pConds.changed : pConds.refined;
    vec.push_back({ Location::MBase{}, ty });
  }
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
    ITRACE(4, "Loading state for B{}: {}\n", block->id(), show(*this));
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
    collectPostConds(block);
    FTRACE(2, "PostConditions for exit Block {}:\n{}\n",
           block->id(), show(m_exitPostConds[block]));
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
  return it->second;
}

/*
 * Save the current state as the in-state for `block'.
 *
 * If this is the first time saving state for `block', create a new snapshot
 * using either the current state or the out-state of `pred' if we are given
 * one.  Otherwise merge the current state into the existing snapshot.
 */
bool FrameStateMgr::save(Block* block, Block* pred) {
  ITRACE(4, "Saving current state to B{}: {}\n", block->id(), show(*this));

  auto const it = m_states.find(block);
  auto changed = true;

  if (it != m_states.end()) {
    changed = merge_into(it->second.in, m_stack);
    ITRACE(4, "Merged state: {}\n", show(*this));
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
  for (auto& state : cur().stack) {
    state = StackState{};
  }

  // These values must go toward their conservative state.
  cur().mbr = MBRState{};
  cur().mbase = MBaseState{};

  clearLocals();
}

///////////////////////////////////////////////////////////////////////////////

void FrameStateMgr::trackDefInlineFP(const IRInstruction* inst) {
  auto const extra      = inst->extra<DefInlineFP>();
  auto const target     = extra->target;
  auto const savedSPOff = extra->retSPOff;
  auto const calleeFP   = inst->dst();
  auto const calleeSP   = inst->src(0);

  always_assert(calleeSP == cur().spValue);

  /*
   * Push a new state for the inlined callee; saving the state we'll need to
   * pop on return.
   */
  for (auto i = uint32_t{0}; i < kNumActRecCells; ++i) {
    setValue(stk(extra->spOffset + i), nullptr);
  }
  cur().bcSPOff = savedSPOff;
  auto stateCopy = m_stack.back();
  m_stack.emplace_back(std::move(stateCopy));

  /*
   * Set up the callee state.
   */
  cur().fpValue          = calleeFP;
  cur().ctx              = inst->src(2);
  cur().curFunc          = target;
  cur().bcSPOff          = FPInvOffset{target->numSlotsInFrame()};

  /*
   * To set up irSPOff, we want the FPInvOffset for the new fpValue and
   * spValue.  We're not changing spValue while we inline, but we're changing
   * fpValue, so this needs to change.  We know where the new fpValue is
   * pointing (relative to the spValue, which we aren't changing).  So we just
   * need to do a change of coordinates, which turns out to be an identity map
   * here:
   *
   *    fpValue = spValue + extra->spOffset (an IRSPRelOffset)
   * So,
   *    spValue = fpValue - extra->spOffset (an FPInvOffset)
   */
  cur().irSPOff = FPInvOffset{extra->spOffset.offset};
  FTRACE(6, "DefInlineFP setting irSPOff: {}\n", cur().irSPOff.offset);

  /*
   * Initialize tracked memory state for locals and stack slots to empty
   * values.
   */
  cur().locals.clear();
  cur().locals.resize(target->numLocals());
  cur().stack.clear();
  cur().stack.resize(std::max(cur().bcSPOff.offset,
                              cur().irSPOff.offset));
}

void FrameStateMgr::trackInlineReturn() {
  DEBUG_ONLY auto const cur_SP = cur().spValue;
  m_stack.pop_back();
  assertx(!m_stack.empty());
  assertx(cur_SP == cur().spValue); // inlining may not change spValue
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
      if (loc.value && !loc.value->inst()->is(DefConst)) loc.value = nullptr;
    }
    for (auto& stk : state.stack) {
      stk.value = nullptr;
    }
  }
}

bool FrameStateMgr::checkInvariants() const {
  for (auto& state : m_stack) {
    always_assert(check_invariants(state));
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
  auto const fpRel = off.to<FPInvOffset>(irSPOff());
  return Location::Stack { fpRel };
}

LocalState& FrameStateMgr::localState(uint32_t id) {
  assertx(id < cur().locals.size());
  auto& ret = cur().locals[id];

  assertx(ret.value == nullptr || ret.value->type() == ret.type);
  return ret;
}

LocalState& FrameStateMgr::localState(Location l) {
  assertx(l.tag() == LTag::Local);
  return localState(l.localId());
}

StackState& FrameStateMgr::stackState(IRSPRelOffset spRel) {
  auto const fpRel = spRel.to<FPInvOffset>(irSPOff());
  return stackState(fpRel);
}

StackState& FrameStateMgr::stackState(FPInvOffset fpRel) {
  auto const idx = fpRel.offset - 1;

  always_assert_flog(
    idx >= 0,
    "stack idx went negative: irSPOff: {}, fpRel: {}\n",
    cur().irSPOff.offset,
    fpRel.offset
  );
  if (idx >= cur().stack.size()) {
    cur().stack.resize(idx + 1);
  }
  auto& ret = cur().stack[idx];

  assertx(ret.value == nullptr || ret.value->type() == ret.type);
  return ret;
}

StackState& FrameStateMgr::stackState(Location l) {
  assertx(l.tag() == LTag::Stack);
  return stackState(l.stackIdx());
}

const LocalState& FrameStateMgr::local(uint32_t id) const {
  return const_cast<FrameStateMgr&>(*this).localState(id);
}

/*
 * We consider it logically const to extend with default-constructed stack
 * values.
 */
const StackState& FrameStateMgr::stack(IRSPRelOffset offset) const {
  return const_cast<FrameStateMgr&>(*this).stackState(offset);
}
const StackState& FrameStateMgr::stack(FPInvOffset offset) const {
  return const_cast<FrameStateMgr&>(*this).stackState(offset);
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
IMPL_MEMBER_OF(const TypeSourceSet&, typeSrcs)

#undef IMPL_MEMBER_AT

///////////////////////////////////////////////////////////////////////////////

template<LTag tag>
void FrameStateMgr::setValueImpl(Location l,
                                 LocationState<tag>& state,
                                 SSATmp* value,
                                 folly::Optional<Type> predicted) {
  FTRACE(2, "{} := {}\n", show(l), value ? value->toString() : "<>");
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
  FTRACE(2, "{} :: {} -> {}\n", show(l), state.type, type);
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
  FTRACE(2, "{} :: {} -> {}\n", show(l), state.type, type);
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
  FTRACE(2, "{} :: {} -> {} (via {})\n", show(l), state.type, refined, type);

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
  always_assert(id < cur().locals.size());
  auto& local = cur().locals[id];
  ITRACE(2, "updating local {}'s type prediction: {} -> {}\n",
    id, local.predictedType, type & local.type);
  local.predictedType = updatePrediction(type, local.type);
}

void FrameStateMgr::clearLocals() {
  ITRACE(2, "clearLocals\n");
  for (auto i = uint32_t{0}; i < cur().locals.size(); ++i) {
    setValue(loc(i), nullptr);
  }
}

///////////////////////////////////////////////////////////////////////////////

void FrameStateMgr::setMemberBase(SSATmp* base,
                                  folly::Optional<Type> predicted) {
  setValueImpl(Location::MBase{}, cur().mbase, base, predicted);
}

///////////////////////////////////////////////////////////////////////////////

std::string show(const FrameStateMgr& state) {
  auto func = state.func();
  auto funcName = func ? func->fullName() : makeStaticString("null");

  return folly::sformat(
    "func: {}, spOff: {}",
    funcName,
    state.irSPOff().offset
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
