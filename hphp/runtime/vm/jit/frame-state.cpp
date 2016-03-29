/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <algorithm>

#include "hphp/util/trace.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/translator.h"

TRACE_SET_MOD(hhir);

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
 * Merge SlotStates, returning whether anything changed.
 */
template<bool Stack>
bool merge_into(SlotState<Stack>& dst, const SlotState<Stack>& src) {
  auto changed = false;

  changed |= merge_util(dst.type, dst.type | src.type);

  // Get the least common ancestor across both states.
  changed |= merge_util(dst.value, least_common_ancestor(dst.value, src.value));

  // We may have changed either dst.value or dst.type in a way that could fail
  // to preserve SlotState invariants.  So check if we can't keep the value.
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

  if (dst.needRatchet != src.needRatchet) {
    dst.needRatchet = true;
    changed = true;
  }

  if (dst.mbase.value != src.mbase.value) {
    dst.mbase.value = nullptr;
    changed = true;
  }
  if (dst.mbase.ptr != src.mbase.ptr) {
    dst.mbase.ptr = nullptr;
    changed = true;
  }
  changed |= merge_util(dst.mbase.ptrType,
                        dst.mbase.ptrType | src.mbase.ptrType);

  // The tracked FPI state must always be the same, notice that the size of the
  // FPI stacks may differ as the FPush associated with one of the merged blocks
  // may be outside the region. In this case we must drop the unknown state.
  dst.fpiStack.resize(std::min(dst.fpiStack.size(), src.fpiStack.size()));
  for (int i = 0; i < dst.fpiStack.size(); ++i) {
    auto& dstInfo = dst.fpiStack[i];
    auto const& srcInfo = src.fpiStack[i];

    always_assert(dstInfo.returnSP == srcInfo.returnSP);
    always_assert(dstInfo.returnSPOff == srcInfo.returnSPOff);
    always_assert(isFPush(dstInfo.fpushOpc) &&
                  dstInfo.fpushOpc == srcInfo.fpushOpc);

    // If one of the merged edges was interp'ed mark the result as interp'ed
    if (!dstInfo.interp && srcInfo.interp) {
      dstInfo.interp = true;
      changed = true;
    }

    // If one of the merged edges spans a call then mark them both as spanning
    if (!dstInfo.spansCall && srcInfo.spansCall) {
      dstInfo.spansCall = true;
      changed = true;
    }

    // Merge the contexts from the respective spills
    if (dstInfo.ctx != srcInfo.ctx) {
      dstInfo.ctx = least_common_ancestor(dstInfo.ctx, srcInfo.ctx);
      changed = true;
    }

    // Merge the Funcs
    if (dstInfo.func != nullptr && dstInfo.func != srcInfo.func) {
      dstInfo.func = nullptr;
      changed = true;
    }
  }

  // This is available iff it's available in both states
  changed |= merge_util(dst.thisAvailable,
                        dst.thisAvailable && src.thisAvailable);

  // The frame may span a call if it could have done so in either state.
  changed |= merge_util(dst.frameMaySpanCall,
                        dst.frameMaySpanCall || src.frameMaySpanCall);

  for (auto i = uint32_t{0}; i < src.locals.size(); ++i) {
    changed |= merge_into(dst.locals[i], src.locals[i]);
  }

  changed |= merge_memory_stack_into(dst.stack, src.stack);

  changed |= merge_util(dst.stackModified,
                        dst.stackModified || src.stackModified);

  // Eval stack depth should be the same at merge points.
  assertx(dst.bcSPOff == src.bcSPOff);

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
        local.type == TGen,
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
 * When we're computing an update for a new predicted type, we sometimes need
 * to fall back to the proven type, e.g. if the new predicted type no longer
 * satisfies the invariant that predictedType <= provenType. predictedType must
 * not be Bottom.
 */
Type updatePredictedType(Type predictedType, Type provenType) {
  if (predictedType == TBottom) return provenType;
  return predictedType < provenType ? predictedType : provenType;
}

Type refinePredictedType(Type oldPrediction, Type newPrediction, Type proven) {
  auto refinedPrediction = oldPrediction & newPrediction;
  if (refinedPrediction == TBottom) refinedPrediction = newPrediction;
  return updatePredictedType(refinedPrediction, proven);
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
      setLocalValue(id, nullptr);
    }
  };

  assertx(checkInvariants());

  switch (inst->op()) {
  case DefInlineFP:    trackDefInlineFP(inst);  break;
  case InlineReturn:   trackInlineReturn(); break;

  case Call:
    {
      auto const extra = inst->extra<Call>();
      killLocalsForCall(extra->destroyLocals);
      for (auto& st : m_stack) st.frameMaySpanCall = true;
      // Remove tracked state for the slots for args and the actrec.
      for (auto i = uint32_t{0}; i < kNumActRecCells + extra->numParams; ++i) {
        setStackValue(extra->spOffset + i, nullptr);
      }
      clearStackForCall();
      // The return value is known to be at least a Gen.
      setStackType(
        extra->spOffset + kNumActRecCells + extra->numParams - 1,
        TGen);
      // We consider popping an ActRec and args to be synced to memory.
      assertx(cur().bcSPOff == inst->marker().spOff());
      cur().bcSPOff -= extra->numParams + kNumActRecCells;

      if (!cur().fpiStack.empty()) {
        cur().fpiStack.pop_front();
      }
      for (auto& st : m_stack) {
        for (auto& fpi : st.fpiStack) fpi.spansCall = true;
      }
    }
    break;

  case CallArray:
    {
      auto const extra = inst->extra<CallArray>();
      killLocalsForCall(extra->destroyLocals);
      for (auto& st : m_stack) st.frameMaySpanCall = true;
      // Remove tracked state for the actrec and array arg.
      uint32_t numCells = kNumActRecCells +
        (extra->numParams ? extra->numParams : 1);
      for (auto i = uint32_t{0}; i < numCells; ++i) {
        setStackValue(extra->spOffset + i, nullptr);
      }
      clearStackForCall();
      setStackType(extra->spOffset + numCells - 1, TGen);
      // A CallArray pops the ActRec, actual args, and an array arg.
      assertx(cur().bcSPOff == inst->marker().spOff());
      cur().bcSPOff -= numCells;

      if (!cur().fpiStack.empty()) {
        cur().fpiStack.pop_front();
      }
      for (auto& st : m_stack) {
        for (auto& fpi : st.fpiStack) fpi.spansCall = true;
      }
    }
    break;

  case CallBuiltin:
    if (inst->extra<CallBuiltin>()->destroyLocals) clearLocals();
    break;

  case ContEnter:
    {
      auto const extra = inst->extra<ContEnter>();
      killLocalsForCall(false);
      for (auto& st : m_stack) st.frameMaySpanCall = true;
      clearStackForCall();
      setStackType(extra->spOffset, TGen);
      // ContEnter pops a cell and pushes a yielded value.
      assertx(cur().bcSPOff == inst->marker().spOff());
    }
    break;

  case DefFP:
  case FreeActRec:
    cur().fpValue = inst->dst();
    break;

  case RetCtrl:
    cur().spValue = nullptr;
    cur().fpValue = nullptr;
    break;

  case DefSP:
    cur().spValue = inst->dst();
    cur().irSPOff = inst->extra<DefSP>()->offset;
    break;

  case StMem:
    // If we ever start using StMem to store to pointers that might be
    // stack/locals, we have to update tracked state here.
    always_assert(!inst->src(0)->type().maybe(TPtrToFrameGen));
    always_assert(!inst->src(0)->type().maybe(TPtrToStkGen));
    break;

  case LdStk:
    {
      auto const offset = inst->extra<LdStk>()->offset;
      auto& state = stackState(offset);
      refinePredictedTmpType(inst->dst(), state.predictedType);
      // Nearly all callers of setStackValue() represent a modification of the
      // stack, so it sets stackModified. LdStk is the one exception, so we
      // compensate for that here.
      auto oldModified = cur().stackModified;
      setStackValue(offset, inst->dst());
      cur().stackModified = oldModified;
    }
    break;

  case StStk:
    setStackValue(inst->extra<StStk>()->offset, inst->src(1));
    break;

  case CheckType:
  case AssertType:
    refineStackValues(inst->src(0), inst->dst());
    refineLocalValues(inst->src(0), inst->dst());
    break;

  case CheckStk:
  case AssertStk:
    refineStackType(inst->extra<IRSPRelOffsetData>()->offset,
                    inst->typeParam(),
                    TypeSource::makeGuard(inst));
    break;

  case AssertLoc:
  case CheckLoc: {
    auto const id = inst->extra<LocalId>()->locId;
    if (inst->marker().func()->isPseudoMain()) {
      setLocalPredictedType(id, inst->typeParam());
    } else {
      refineLocalType(id, inst->typeParam(),
                      TypeSource::makeGuard(inst));
    }
  } break;

  case HintStkInner:
    setBoxedStkPrediction(inst->extra<HintStkInner>()->offset,
                          inst->typeParam());
    break;

  case HintLocInner:
    setBoxedLocalPrediction(inst->extra<HintLocInner>()->locId,
                            inst->typeParam());
    break;

  case StLoc:
    setLocalValue(inst->extra<LocalId>()->locId, inst->src(1));
    break;

  case LdLoc:
    {
      auto const id = inst->extra<LdLoc>()->locId;
      refinePredictedTmpType(inst->dst(), cur().locals[id].predictedType);
      setLocalValue(id, inst->dst());
    }
    break;

  case StLocPseudoMain:
    setLocalPredictedType(inst->extra<LocalId>()->locId,
                          inst->src(1)->type());
    break;

  case CastStk:
    setStackType(inst->extra<CastStk>()->offset, inst->typeParam());
    break;

  case CoerceStk:
    setStackType(inst->extra<CoerceStk>()->offset, inst->typeParam());
    break;

  case StRef:
    updateLocalRefPredictions(inst->src(0), inst->src(1));
    break;

  case CastMem:
  case CoerceMem: {
    auto addr = inst->src(0);
    if (!addr->inst()->is(LdLocAddr)) break;
    auto locId = addr->inst()->extra<LdLocAddr>()->locId;
    setLocalValue(locId, nullptr);
    setLocalType(locId, inst->typeParam());
    break;
  }

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

  case CufIterSpillFrame:
    spillFrameStack(inst->extra<CufIterSpillFrame>()->spOffset,
                    cur().irSPOff, inst);
    break;
  case SpillFrame:
    spillFrameStack(inst->extra<SpillFrame>()->spOffset,
                    cur().bcSPOff, inst);
    break;

  case InterpOne:
  case InterpOneCF: {
    auto const& extra = *inst->extra<InterpOneData>();
    if (isFPush(extra.opcode)) {
      cur().fpiStack.push_front(FPIInfo { cur().spValue,
                                          cur().irSPOff,
                                          nullptr,
                                          extra.opcode,
                                          nullptr,
                                          true /* interp */,
                                          false /* spansCall */});
    } else if (isFCallStar(extra.opcode) && !cur().fpiStack.empty()) {
      cur().fpiStack.pop_front();
    }

    assertx(!extra.smashesAllLocals || extra.nChangedLocals == 0);
    if (extra.smashesAllLocals || inst->marker().func()->isPseudoMain()) {
      clearLocals();
    } else {
      auto it = extra.changedLocals;
      auto const end = it + extra.nChangedLocals;
      for (; it != end; ++it) {
        auto& loc = *it;
        // If changing the inner type of a boxed local, also drop the
        // information about inner types for any other boxed locals.
        if (loc.type <= TBoxedCell) dropLocalRefsInnerTypes();
        setLocalType(loc.id, loc.type);
      }
    }

    // Offset of the bytecode stack top relative to the IR stack pointer.
    auto const bcSPOff = extra.spOffset;

    // Clear tracked information for slots pushed and popped.
    for (auto i = uint32_t{0}; i < extra.cellsPopped; ++i) {
      setStackValue(bcSPOff + i, nullptr);
    }
    for (auto i = uint32_t{0}; i < extra.cellsPushed; ++i) {
      setStackValue(bcSPOff + extra.cellsPopped - 1 - i, nullptr);
    }
    auto adjustedTop = bcSPOff + extra.cellsPopped - extra.cellsPushed;

    switch (extra.opcode) {
      case Op::CGetL2:
        setStackType(adjustedTop + 1, inst->typeParam());
        break;
      case Op::CGetL3:
        setStackType(adjustedTop + 2, inst->typeParam());
        break;
      default:
        // We don't track cells pushed by interp one except the top of the
        // stack, aside from the above special cases.
        if (inst->hasTypeParam()) {
          auto const instrInfo = getInstrInfo(extra.opcode);
          if (instrInfo.out & InstrFlags::Stack1) {
            setStackType(adjustedTop, inst->typeParam());
          }
        }
        break;
    }

    cur().bcSPOff += extra.cellsPushed;
    cur().bcSPOff -= extra.cellsPopped;

    if (isMemberBaseOp(extra.opcode) || isMemberDimOp(extra.opcode) ||
        isMemberFinalOp(extra.opcode)) {
      cur().mbase.reset();
    }
    break;
  }

  case CheckCtxThis:
    cur().thisAvailable = true;
    break;

  case IterInitK:
  case WIterInitK:
    // kill the locals to which this instruction stores iter's key and value
    killIterLocals({inst->extra<IterData>()->keyId,
                    inst->extra<IterData>()->valId});
    break;

  case IterInit:
  case WIterInit:
    // kill the local to which this instruction stores iter's value
    killIterLocals({inst->extra<IterData>()->valId});
    break;

  case IterNextK:
  case WIterNextK:
    // kill the locals to which this instruction stores iter's key and value
    killIterLocals({inst->extra<IterData>()->keyId,
                    inst->extra<IterData>()->valId});
    break;

  case IterNext:
  case WIterNext:
    // kill the local to which this instruction stores iter's value
    killIterLocals({inst->extra<IterData>()->valId});
    break;

  case StMBase:
    cur().mbase.ptr = inst->src(0);
    cur().mbase.ptrType = inst->src(0)->type();
    cur().mbase.value = nullptr;
    break;

  case FinishMemberOp:
    cur().mbase.reset();
    break;

  case VerifyRetFail:
    if (!func()->unit()->useStrictTypes()) {
      // In PHP 7 mode scalar types can sometimes coerce; we do this during the
      // VerifyRetFail call -- we never allow this in HH files.
      auto const offset = BCSPRelOffset{0}
        .to<FPInvOffset>(inst->marker().spOff())
        .to<IRSPRelOffset>(irSPOff());
      setStackType(offset, TGen);
    }
    break;

  case VerifyParamFail:
    if (!func()->unit()->isHHFile() && !RuntimeOption::EnableHipHopSyntax &&
        RuntimeOption::PHP7_ScalarTypes) {
      // In PHP 7 mode scalar types can sometimes coerce; we do this during the
      // VerifyParamFail call -- we never allow this in HH files.
      auto id = inst->src(0)->intVal();
      setLocalType(id, TGen);
    }
    break;

  default:
    if (MInstrEffects::supported(inst)) {
      updateMInstr(inst);
    }
    break;
  }
}

void FrameStateMgr::updateMInstr(const IRInstruction* inst) {
  // We don't update tracked local types for pseudomains, but we do care about
  // stack types.
  auto const isPM = cur().curFunc->isPseudoMain();
  auto const base = inst->src(minstrBaseIdx(inst->op()));

  if (base->inst()->is(LdStkAddr)) {
    auto const offset = base->inst()->extra<LdStkAddr>()->offset;
    auto const prevTy = stack(offset).type;
    MInstrEffects effects(inst->op(), prevTy.ptr(Ptr::Stk));

    if (effects.baseTypeChanged || effects.baseValChanged) {
      auto const ty = effects.baseType.derefIfPtr();
      setStackType(
        offset,
        ty <= TBoxedCell ? TBoxedInitCell : ty
      );
    }
  } else if (base->inst()->is(LdLocAddr)) {
    if (isPM) return;
    auto const locId = base->inst()->extra<LdLocAddr>()->locId;
    auto const baseType = local(locId).type;

    MInstrEffects effects(inst->op(), baseType.ptr(Ptr::Frame));
    if (effects.baseTypeChanged || effects.baseValChanged) {
      auto const ty = effects.baseType.derefIfPtr();
      if (ty <= TBoxedCell) {
        setLocalType(locId, TBoxedInitCell);
        setBoxedLocalPrediction(locId, ty);
      } else {
        setLocalType(locId, ty);
      }
    }
  } else {
    Trace::Indent indent;
    // If we don't know exactly where the base is, we have to be conservative
    // and apply the operation to all locals/stack slots that could be
    // affected.
    if (base->type().maybe(TPtrToFrameGen) && !isPM) {
      for (size_t i = 0, n = cur().locals.size(); i < n; ++i) {
        auto const oldType = local(i).type;
        if (TGen <= oldType) {
          // Drop the value and don't bother with precise effects.
          setLocalType(i, oldType);
          continue;
        }
        if (oldType <= TBoxedCell) continue;
        MInstrEffects e(inst->op(), oldType);
        if (!e.baseValChanged && !e.baseTypeChanged) continue;
        widenLocalType(i, oldType | e.baseType);
      }
    }
    if (base->type().maybe(TPtrToStkGen)) {
      for (auto i = 0; i < cur().stack.size(); ++i) {
        // The FPInvOffset of the stack slot is just its 1-indexed slot.
        auto const spRel = FPInvOffset{i + 1}.to<IRSPRelOffset>(cur().irSPOff);

        auto const oldType = stack(spRel).type;
        if (TStkElem <= oldType) {
          // Drop the value and don't bother with precise effects.
          setStackType(spRel, oldType);
          continue;
        }
        if (oldType <= TBoxedCell) continue;

        MInstrEffects e(inst->op(), oldType);
        if (!e.baseValChanged && !e.baseTypeChanged) continue;
        widenStackType(spRel, oldType | e.baseType);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

/*
 * syncPrediction() is called after we update the predictedType and/or value
 * for a SlotState. It looks up the predicted type for the value in
 * cur().predictedTypes and ensures both locations have the most refined
 * predicted type possible.
 */
template<bool Stack>
void FrameStateMgr::syncPrediction(SlotState<Stack>& slot) {
  if (!slot.value) return;
  ITRACE(3, "Syncing prediction for {}\n", *slot.value);
  auto const canonValue = canonical(slot.value);

  auto& prediction = slot.predictedType;
  auto& map = cur().predictedTypes;
  auto it = map.find(canonValue);
  if (it == map.end()) {
    ITRACE(4, "No prediction in map; slot has {}\n", prediction);
    if (prediction < slot.default_type()) map.emplace(canonValue, prediction);
    return;
  }
  if (prediction == slot.default_type()) {
    ITRACE(4, "No prediction in slot; map has {}\n", it->second);
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

void FrameStateMgr::refinePredictedTmpType(SSATmp* tmp, Type prediction) {
  auto const canonTmp = canonical(tmp);
  auto& map = cur().predictedTypes;
  auto it = map.find(canonTmp);
  if (it == map.end()) {
    map.emplace(canonTmp, prediction);
    ITRACE(3, "New prediction for {}: {}\n", *tmp->inst(), prediction);
    return;
  }

  ITRACE(3, "Prediction for {} refined from {} to ", *tmp->inst(), it->second);
  it->second = refinePredictedType(it->second, prediction, tmp->type());
  FTRACE(3, "{}\n", it->second);
}

/*
 * Collects the post-conditions associated with the current state,
 * which is essentially a list of local/stack locations and their
 * known types at the end of `block'.
 */
void FrameStateMgr::collectPostConds(Block* block) {
  assertx(block->isExitNoThrow());
  PostConditions& pConds = m_exitPostConds[block];
  pConds.changed.clear();
  pConds.refined.clear();

  if (sp() != nullptr) {
    auto const& lastInst = block->back();
    auto const bcSPOff = lastInst.marker().spOff();
    auto const resumed = lastInst.marker().resumed();
    auto const skipCells = FPInvOffset{resumed ? 0 : func()->numSlotsInFrame()};
    auto const evalStkCells = bcSPOff - skipCells;

    for (int32_t i = 0; i < evalStkCells; i++) {
      auto const bcSPRel = BCSPRelOffset{i};
      auto const irSPRel = bcSPRel
        .to<FPInvOffset>(bcSPOff)
        .to<IRSPRelOffset>(irSPOff());

      auto const type    = stack(irSPRel).type;
      auto const changed = stack(irSPRel).maybeChanged;

      if (changed || type < TGen) {
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
      auto t = local(i).type;
      const bool changed = local(i).maybeChanged;
      if (changed || t < TGen) {
        FTRACE(1, "Local {}: {} ({})\n", i, t.toString(),
               changed ? "changed" : "refined");
        auto& vec = changed ? pConds.changed : pConds.refined;
        vec.push_back({ Location::Local{i}, t });
      }
    }
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
  } else {
    if (debug) save(block);
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

  auto changed = false;
  if (!block->back().isTerminal()) changed |= save(block->next());
  return changed;
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

/*
 * Save current state for block.  If this is the first time saving state for
 * block, create a new snapshot.  Otherwise merge the current state into the
 * existing snapshot.
 */
bool FrameStateMgr::save(Block* block) {
  ITRACE(4, "Saving current state to B{}: {}\n", block->id(), show(*this));

  auto const it = m_states.find(block);
  auto changed = true;

  if (it != m_states.end()) {
    changed = merge_into(it->second.in, m_stack);
    ITRACE(4, "Merged state: {}\n", show(*this));
  } else {
    assertx(!m_stack.empty());
    m_states[block].in = m_stack;
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
  for (auto& state : m_stack) {
    for (auto& stk : state.stack) {
      stk = StackState{};
    }
  }

  // These values must go toward their conservative state.
  cur().thisAvailable    = false;
  cur().frameMaySpanCall = true;
  cur().mbase.reset();

  cur().fpiStack.clear();
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
   * Remove the callee from the FPI Stack.
   */
  cur().fpiStack.pop_front();

  /*
   * Push a new state for the inlined callee; saving the state we'll need to
   * pop on return.
   */
  cur().bcSPOff = savedSPOff;
  auto stateCopy = m_stack.back();
  m_stack.emplace_back(std::move(stateCopy));

  /*
   * Set up the callee state.
   *
   * We set m_thisIsAvailable to true on any object method, because we
   * just don't inline calls to object methods with a null $this.
   */
  cur().fpValue          = calleeFP;
  cur().thisAvailable    = target->cls() != nullptr && !target->isStatic();
  cur().curFunc          = target;
  cur().frameMaySpanCall = false;
  cur().bcSPOff          = FPInvOffset{target->numLocals()};

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

bool FrameStateMgr::checkInvariants() const {
  for (auto& state : m_stack) {
    always_assert(check_invariants(state));
  }
  return true;
}

StackState& FrameStateMgr::stackState(IRSPRelOffset spRel) {
  auto const fpRel = spRel.to<FPInvOffset>(cur().irSPOff);
  auto const idx = fpRel.offset - 1;

  FTRACE(6, "stackState offset: {} (@ spOff {}) --> idx={}\n",
         spRel.offset, cur().irSPOff.offset, idx);
  always_assert_flog(
    idx >= 0,
    "idx went negative: curSpOffset: {}, offset: {}\n",
    cur().irSPOff.offset,
    spRel.offset
  );
  if (idx >= cur().stack.size()) {
    cur().stack.resize(idx + 1);
  }
  return cur().stack[idx];
}

const StackState& FrameStateMgr::stackState(IRSPRelOffset offset) const {
  // We consider it logically const to extend with default-constructed stack
  // values.
  return const_cast<FrameStateMgr&>(*this).stackState(offset);
}

const PostConditions& FrameStateMgr::postConds(Block* exitBlock) const {
  assertx(exitBlock->isExitNoThrow());
  auto it = m_exitPostConds.find(exitBlock);
  assertx(it != m_exitPostConds.end());
  return it->second;
}

const LocalState& FrameStateMgr::local(uint32_t id) const {
  always_assert(id < cur().locals.size());
  auto const& local = cur().locals[id];
  assert(local.value == nullptr || local.value->type() == local.type);
  return local;
}

const StackState& FrameStateMgr::stack(IRSPRelOffset offset) const {
  auto const& stack = stackState(offset);
  assert(stack.value == nullptr || stack.value->type() == stack.type);
  return stack;
}

void FrameStateMgr::setStackValue(IRSPRelOffset offset, SSATmp* value) {
  auto& stk = stackState(offset);
  FTRACE(2, "stk[{}] := {}\n", offset.offset,
    value ? value->toString() : std::string("<>"));
  stk.value         = value;
  stk.type          = value ? value->type() : TStkElem;
  stk.maybeChanged  = true;
  stk.predictedType = stk.type;
  syncPrediction(stk);

  stk.typeSrcs.clear();
  if (value) stk.typeSrcs.insert(TypeSource::makeValue(value));
  cur().stackModified = true;
}

void FrameStateMgr::setStackType(IRSPRelOffset offset, Type type) {
  auto& stk = stackState(offset);
  ITRACE(2, "stk[{}] :: {} -> {}\n", offset.offset, stk.type, type);
  stk.value         = nullptr;
  stk.type          = type;
  stk.maybeChanged  = true;
  stk.predictedType = type;
  stk.typeSrcs.clear();
  cur().stackModified = true;
}

void FrameStateMgr::widenStackType(IRSPRelOffset offset, Type type) {
  auto& stk = stackState(offset);
  ITRACE(2, "stk[{}] :: {} -> {}\n", offset.offset, stk.type, type);
  stk.value         = nullptr;
  stk.type          = type;
  stk.maybeChanged  = true;
  stk.predictedType = type;
  cur().stackModified = true;
}

void FrameStateMgr::setBoxedStkPrediction(IRSPRelOffset offset, Type type) {
  auto& state = stackState(offset);
  state.predictedType = state.type & type;
}

static const Func* getSpillFrameKnownCallee(const IRInstruction* inst) {
  if (!inst->is(SpillFrame)) return nullptr;

  const auto funcTmp = inst->src(1);
  if (!funcTmp->hasConstVal(TFunc)) return nullptr;

  const auto callee = funcTmp->funcVal();
  if (!callee->isMethod()) return callee;

  const auto ctx = inst->src(2);
  const auto ctxType = ctx->type();
  if (ctxType < TObj && ctxType.clsSpec().exact()) return callee;

  return nullptr;
}

void FrameStateMgr::spillFrameStack(IRSPRelOffset offset,
                                    FPInvOffset retOffset,
                                    const IRInstruction* inst) {
  for (auto i = uint32_t{0}; i < kNumActRecCells; ++i) {
    setStackValue(offset + i, nullptr);
  }
  auto const ctx = inst->op() == SpillFrame ? inst->src(2) : nullptr;

  const Func* func = getSpillFrameKnownCallee(inst);
  auto const opc = m_fpushOverride ?
    *m_fpushOverride : inst->marker().sk().op();
  m_fpushOverride.clear();

  cur().bcSPOff += kNumActRecCells;
  cur().fpiStack.push_front(FPIInfo { cur().spValue, retOffset, ctx, opc, func,
                                      false /* interp */, false /* spans */ });
}

void FrameStateMgr::refineStackType(IRSPRelOffset offset,
                                    Type ty,
                                    TypeSource typeSrc) {
  auto& state = stackState(offset);
  auto const newType = state.type & ty;
  ITRACE(2, "stk[{}] updating type {} as {} -> {}\n", offset.offset,
    state.type, ty, newType);
  // If the type gets more refined, we need to forget the old value.
  // Otherwise, we may end up using a value with a more general type
  // than is known about the stack slot.
  if (newType != state.type) state.value = nullptr;
  state.type          = newType;
  state.predictedType = updatePredictedType(state.predictedType, state.type);
  state.typeSrcs.clear();
  state.typeSrcs.insert(typeSrc);
}

void FrameStateMgr::clearStackForCall() {
  ITRACE(2, "clearStackForCall\n");
  for (auto& state : m_stack) {
    for (auto& stk : state.stack) {
      stk.value = nullptr;
    }
  }
}

void FrameStateMgr::clearLocals() {
  ITRACE(2, "clearLocals\n");
  for (auto i = uint32_t{0}; i < cur().locals.size(); ++i) {
    setLocalValue(i, nullptr);
  }
}

void FrameStateMgr::setLocalValue(uint32_t id, SSATmp* value) {
  always_assert(id < cur().locals.size());
  auto& loc = cur().locals[id];
  loc.value          = value;
  auto const newType = value ? value->type() : TGen;
  loc.type           = newType;
  loc.maybeChanged   = true;
  /*
   * Update the predicted type for boxed values in some special cases to
   * something smart.  The rest of the time, throw it away.
   */
  auto const newInnerPred = [&]() -> Type {
    if (value) {
      auto const inst = value->inst();
      switch (inst->op()) {
      case LdLoc:
        if (value->type() <= TBoxedCell) {
          // Keep the same prediction as this local.
          return cur().locals[inst->extra<LdLoc>()->locId].predictedType;
        }
        break;
      case Box:
        return boxType(inst->src(0)->type());
      default:
        break;
      }
    }
    return loc.type;  // just predict what we know
  }();

  // We need to make sure not to violate the invariant that predictedType is
  // always <= type.  Note that operator& can be conservative (it could just
  // return one of the two types in situations relating to specialized types we
  // can't represent), so it's necessary to double check.
  auto const rawIsect = newType & newInnerPred;
  auto const useTy = rawIsect <= newType ? rawIsect : newType;

  FTRACE(3, "setLocalValue setting prediction {} based on {}, using = {}\n",
    id, newInnerPred, useTy);
  loc.predictedType = useTy;
  syncPrediction(loc);

  loc.typeSrcs.clear();
  if (value) {
    loc.typeSrcs.insert(TypeSource::makeValue(value));
  }
}

void FrameStateMgr::refineLocalType(uint32_t id,
                                    Type type,
                                    TypeSource typeSrc) {
  always_assert(id < cur().locals.size());
  auto& local = cur().locals[id];
  auto const newType = local.type & type;
  ITRACE(2, "updating local {}'s type: {} -> {}\n", id, local.type, newType);
  // If the type gets more refined, we need to forget the old value.
  // Otherwise, we may end up using a value with a more general type
  // than is known about the local.
  if (newType != local.type) local.value = nullptr;
  local.type          = newType;
  local.predictedType = updatePredictedType(local.predictedType, newType);
  local.typeSrcs.clear();
  local.typeSrcs.insert(typeSrc);
}

void FrameStateMgr::setLocalPredictedType(uint32_t id, Type type) {
  always_assert(id < cur().locals.size());
  auto& local = cur().locals[id];
  ITRACE(2, "updating local {}'s type prediction: {} -> {}\n",
    id, local.predictedType, type & local.type);
  local.predictedType = updatePredictedType(type, local.type);
}

void FrameStateMgr::refineLocalPredictedType(uint32_t id, Type type) {
  always_assert(id < cur().locals.size());
  auto& local = cur().locals[id];
  local.predictedType = refinePredictedType(
    local.predictedType, type, local.type);
  syncPrediction(local);
}

void FrameStateMgr::refineStackPredictedType(IRSPRelOffset offset, Type type) {
  auto& state = stackState(offset);
  state.predictedType = refinePredictedType(
    state.predictedType, type, state.type);
  syncPrediction(state);
}

void FrameStateMgr::setLocalType(uint32_t id, Type type) {
  always_assert(id < cur().locals.size());
  ITRACE(2, "loc[{}] :: {} -> {}\n", id, cur().locals[id].type, type);
  cur().locals[id].value         = nullptr;
  cur().locals[id].type          = type;
  cur().locals[id].maybeChanged  = true;
  cur().locals[id].predictedType = type;
  cur().locals[id].typeSrcs.clear();
}

void FrameStateMgr::widenLocalType(uint32_t id, Type type) {
  always_assert(id < cur().locals.size());
  ITRACE(2, "loc[{}] :: {} -> {}\n", id, cur().locals[id].type, type);
  cur().locals[id].value         = nullptr;
  cur().locals[id].type          = type;
  cur().locals[id].maybeChanged  = true;
  cur().locals[id].predictedType = type;
}

void FrameStateMgr::setBoxedLocalPrediction(uint32_t id, Type type) {
  always_assert(id < cur().locals.size());
  always_assert(type <= TBoxedCell);

  cur().locals[id].predictedType = cur().locals[id].type & type;
}

/*
 * This is called when we store into a BoxedCell.  Any locals that we know
 * point to that cell can have their inner type predictions updated.
 */
void FrameStateMgr::updateLocalRefPredictions(SSATmp* boxedCell, SSATmp* val) {
  assertx(boxedCell->type() <= TBoxedCell);
  for (auto id = uint32_t{0}; id < cur().locals.size(); ++id) {
    if (canonical(cur().locals[id].value) == canonical(boxedCell)) {
      setBoxedLocalPrediction(id, boxType(val->type()));
    }
  }
}

void FrameStateMgr::setLocalTypeSource(uint32_t id, TypeSource typeSrc) {
  always_assert(id < cur().locals.size());
  cur().locals[id].typeSrcs.clear();
  cur().locals[id].typeSrcs.insert(typeSrc);
}

/*
 * Get a reference to the locals from an inline index, which is the index in
 * m_stack.
 */
jit::vector<LocalState>& FrameStateMgr::locals(unsigned inlineIdx) {
  assertx(inlineIdx < m_stack.size());
  return m_stack[inlineIdx].locals;
}

void FrameStateMgr::refineLocalValues(SSATmp* oldVal, SSATmp* newVal) {
  for (auto& frame : m_stack) {
    for (auto id = uint32_t{0}; id < frame.locals.size(); ++id) {
      auto& local = frame.locals[id];
      if (!local.value || canonical(local.value) != canonical(oldVal)) {
        continue;
      }
      ITRACE(2, "refining local {}'s value: {} -> {}\n",
             id, *local.value, *newVal);
      local.value         = newVal;
      local.type          = newVal->type();
      local.predictedType = updatePredictedType(local.predictedType,
                                                local.type);
      local.typeSrcs.clear();
      local.typeSrcs.insert(TypeSource::makeValue(newVal));
    }
  }
}

void FrameStateMgr::refineStackValues(SSATmp* oldVal, SSATmp* newVal) {
  for (auto& frame : m_stack) {
    for (auto& slot : frame.stack) {
      if (!slot.value || canonical(slot.value) != canonical(oldVal)) {
        continue;
      }
      ITRACE(2, "refining on stack {} -> {}\n", *oldVal, *newVal);
      slot.value         = newVal;
      slot.type          = newVal->type();
      slot.predictedType = updatePredictedType(slot.predictedType, slot.type);
      slot.typeSrcs.clear();
      slot.typeSrcs.insert(TypeSource::makeValue(newVal));
    }
  }
}

/*
 * Called to clear out the tracked local values at a call site.  Keeping a
 * value live across a Call requires spilling, so we avoid it. We do continue
 * tracking the types in locals, however.
 */
void FrameStateMgr::killLocalsForCall(bool callDestroysLocals) {
  if (callDestroysLocals) clearLocals();
  for (auto& frame : m_stack) {
    for (auto& loc : frame.locals) {
      if (loc.value && !loc.value->inst()->is(DefConst)) loc.value = nullptr;
    }
  }
}

/*
 * This function changes any boxed local into a BoxedInitCell type. It's safe
 * to assume they're init because you can never have a reference to uninit.
 */
void FrameStateMgr::dropLocalRefsInnerTypes() {
  for (auto& frame : m_stack) {
    for (auto& local : frame.locals) {
      if (local.type <= TBoxedCell) {
        local.type          = TBoxedInitCell;
        local.predictedType = TBoxedInitCell;
        local.maybeChanged  = true;
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////

std::string show(const FrameStateMgr& state) {
  auto func = state.func();
  auto funcName = func ? func->fullName() : makeStaticString("null");

  return folly::format(
    "func: {}, spOff: {}{}{}",
    funcName,
    state.irSPOff().offset,
    state.thisAvailable() ? ", thisAvailable" : "",
    state.frameMaySpanCall() ? ", frameMaySpanCall" : ""
  ).str();
}

//////////////////////////////////////////////////////////////////////

}}}
