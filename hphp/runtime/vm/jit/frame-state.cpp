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
#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/translator.h"

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

  always_assert(src.clsRefSlots.size() == dst.clsRefSlots.size());

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

    // If one of the merged edges is not eligible for inlining, mark the result
    // as not eligibile.
    if (dstInfo.inlineEligible && !srcInfo.inlineEligible) {
      dstInfo.inlineEligible = false;
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

    if (dstInfo.ctxType != srcInfo.ctxType) {
      dstInfo.ctxType |= srcInfo.ctxType;
      changed = true;
    }

    // Merge the Funcs
    if (dstInfo.func != nullptr && dstInfo.func != srcInfo.func) {
      dstInfo.func = nullptr;
      changed = true;
    }
  }

  // The frame may span a call if it could have done so in either state.
  changed |= merge_util(dst.frameMaySpanCall,
                        dst.frameMaySpanCall || src.frameMaySpanCall);

  for (auto i = uint32_t{0}; i < src.locals.size(); ++i) {
    changed |= merge_into(dst.locals[i], src.locals[i]);
  }

  changed |= merge_memory_stack_into(dst.stack, src.stack);

  for (auto i = uint32_t{0}; i < src.clsRefSlots.size(); ++i) {
    changed |= merge_into(dst.clsRefSlots[i], src.clsRefSlots[i]);
  }

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

  for (auto id = uint32_t{0}; id < state.clsRefSlots.size(); ++id) {
    auto const& clsRef = state.clsRefSlots[id];

    always_assert_flog(
      clsRef.predictedType <= clsRef.type,
      "class-ref {} failed prediction invariants; pred = {}, type = {}\n",
      id,
      clsRef.predictedType,
      clsRef.type
    );

    always_assert_flog(
      clsRef.value == nullptr || clsRef.value->type() == clsRef.type,
      "class-ref {} had type {}, but value {}\n",
      id,
      clsRef.type,
      clsRef.value->toString()
    );

    always_assert_flog(
      clsRef.type <= TCls,
      "class-ref {} had non-Cls type {}\n",
      id,
      clsRef.type
    );
  }

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
  cur().clsRefSlots.resize(marker.func()->numClsRefSlots());
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
  case InlineReturn:   trackInlineReturn(); break;
  case InitCtx: {
    always_assert(!cur().ctx);
    cur().ctx = inst->src(1);
    break;
  }
  case Call:
    {
      auto const extra = inst->extra<Call>();
      // Remove tracked state for the slots for args and the actrec.
      for (auto i = uint32_t{0}; i < kNumActRecCells + extra->numParams; ++i) {
        setValue(stk(extra->spOffset + i), nullptr);
      }
      trackCall(extra->writeLocals);
      // The return value is known to be at least a Gen.
      setType(
        stk(extra->spOffset + kNumActRecCells + extra->numParams - 1),
        TGen
      );
      // We consider popping an ActRec and args to be synced to memory.
      assertx(cur().bcSPOff == inst->marker().spOff());
      cur().bcSPOff -= extra->numParams + kNumActRecCells;

      if (!cur().fpiStack.empty()) {
        cur().fpiStack.pop_back();
      }
      for (auto& st : m_stack) {
        for (auto& fpi : st.fpiStack) fpi.spansCall = true;
      }
    }
    break;

  case CallArray:
    {
      auto const extra = inst->extra<CallArray>();
      // Remove tracked state for the actrec and array arg.
      uint32_t numCells = kNumActRecCells +
        (extra->numParams ? extra->numParams : 1);
      for (auto i = uint32_t{0}; i < numCells; ++i) {
        setValue(stk(extra->spOffset + i), nullptr);
      }
      trackCall(extra->writeLocals);
      setType(stk(extra->spOffset + numCells - 1), TGen);
      // A CallArray pops the ActRec, actual args, and an array arg.
      assertx(cur().bcSPOff == inst->marker().spOff());
      cur().bcSPOff -= numCells;

      if (!cur().fpiStack.empty()) {
        cur().fpiStack.pop_back();
      }
      for (auto& st : m_stack) {
        for (auto& fpi : st.fpiStack) fpi.spansCall = true;
      }
    }
    break;

  case CallBuiltin:
    if (inst->extra<CallBuiltin>()->writeLocals) clearLocals();
    break;

  case ContEnter:
    {
      auto const extra = inst->extra<ContEnter>();
      setValue(stk(extra->spOffset), nullptr);
      trackCall(false);
      setType(stk(extra->spOffset), TGen);
      // ContEnter pops a cell.
      assertx(cur().bcSPOff == inst->marker().spOff());
      cur().bcSPOff--;
    }
    break;

  case FuncGuard:
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

  case LdMem:
    if (inst->src(0) == cur().mbr.ptr) {
      refinePredictedTmpType(inst->dst(), cur().mbase.predictedType);
      setValue(Location::MBase{}, inst->dst());
    }
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
      for (auto& state : frame.locals) {
        refineValue(state, inst->src(0), inst->dst());
      }
      for (auto& state : frame.stack) {
        refineValue(state, inst->src(0), inst->dst());
      }
      for (auto& state : frame.clsRefSlots) {
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

  case HintStkInner:
    setBoxedPrediction(stk(inst->extra<HintStkInner>()->offset),
                       inst->typeParam());
    break;

  case HintLocInner:
    setBoxedPrediction(loc(inst->extra<HintLocInner>()->locId),
                       inst->typeParam());
    break;

  case HintMBaseInner:
    setBoxedPrediction(Location::MBase{}, inst->typeParam());
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

  case StClsRef:
    setValue(cslot(inst->extra<StClsRef>()->slot), inst->src(1));
    break;

  case LdClsRef:
    {
      auto const slot = inst->extra<LdClsRef>()->slot;
      refinePredictedTmpType(
        inst->dst(),
        cur().clsRefSlots[slot].predictedType
      );
      setValue(cslot(slot), inst->dst());
    }
    break;

  case KillClsRef:
    setValue(cslot(inst->extra<KillClsRef>()->slot), nullptr);
    break;

  case CastStk:
    setType(stk(inst->extra<CastStk>()->offset), inst->typeParam());
    break;

  case CoerceStk:
    setType(stk(inst->extra<CoerceStk>()->offset), inst->typeParam());
    break;

  case StRef:
    updateLocalRefPredictions(inst->src(0), inst->src(1));
    break;

  case CastMem:
  case CoerceMem: {
    auto addr = inst->src(0);
    if (!addr->inst()->is(LdLocAddr)) break;
    auto locId = addr->inst()->extra<LdLocAddr>()->locId;
    setValue(loc(locId), nullptr);
    setType(loc(locId), inst->typeParam());
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

  case SpillFrame:
    spillFrameStack(inst->extra<SpillFrame>()->spOffset,
                    cur().bcSPOff, inst);
    break;

  case LookupClsMethod:
    writeToSpilledFrame(inst->extra<LookupClsMethod>()->calleeAROffset,
                        inst->src(2));
    break;
  case LdObjMethod:
    writeToSpilledFrame(inst->extra<LdObjMethod>()->offset,
                        inst->src(1));
    break;
  case LdArrFuncCtx:
  case LdArrFPushCuf:
  case LdStrFPushCuf:
  case LdFunc:
    writeToSpilledFrame(inst->extra<IRSPRelOffsetData>()->offset,
                        inst->src(1));
    break;

  case InterpOne:
  case InterpOneCF: {
    auto const& extra = *inst->extra<InterpOneData>();
    if (isFPush(extra.opcode)) {
      cur().fpiStack.push_back(FPIInfo { cur().spValue,
                                         cur().bcSPOff - extra.cellsPopped,
                                         TCtx,
                                         nullptr,
                                         extra.opcode,
                                         nullptr,
                                         false /* inlineEligible */,
                                         false /* spansCall */});
    } else if (isFCallStar(extra.opcode) && !cur().fpiStack.empty()) {
      cur().fpiStack.pop_back();
    }

    assertx(!extra.smashesAllLocals || extra.nChangedLocals == 0);
    if (extra.smashesAllLocals || inst->marker().func()->isPseudoMain()) {
      clearLocals();
    } else {
      auto it = extra.changedLocals;
      auto const end = it + extra.nChangedLocals;
      for (; it != end; ++it) {
        auto& local = *it;
        // If changing the inner type of a boxed local, also drop the
        // information about inner types for any other boxed locals.
        if (local.type <= TBoxedCell) dropLocalRefsInnerTypes();
        setType(loc(local.id), local.type);
      }
    }

    for (auto i = uint32_t{0}; i < extra.nChangedClsRefSlots; ++i) {
      auto const& slot = extra.changedClsRefSlots[i];
      // Either its written to, in which case we don't know the value, or its
      // read from, in which case it no longer has any value. Either way, drop
      // any information we have.
      setValue(cslot(slot.id), nullptr);
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

  case CheckCtxThis:
    break;

  case IterInitK:
  case WIterInitK:
  case MIterInitK:
    // kill the locals to which this instruction stores iter's key and value
    killIterLocals({inst->extra<IterData>()->keyId,
                    inst->extra<IterData>()->valId});
    break;

  case IterInit:
  case WIterInit:
  case MIterInit:
    // kill the local to which this instruction stores iter's value
    killIterLocals({inst->extra<IterData>()->valId});
    break;

  case IterNextK:
  case WIterNextK:
  case MIterNextK:
    // kill the locals to which this instruction stores iter's key and value
    killIterLocals({inst->extra<IterData>()->keyId,
                    inst->extra<IterData>()->valId});
    break;

  case IterNext:
  case WIterNext:
  case MIterNext:
    // kill the local to which this instruction stores iter's value
    killIterLocals({inst->extra<IterData>()->valId});
    break;

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

  case VerifyRetFail:
    if (!func()->unit()->useStrictTypes()) {
      // In PHP 7 mode scalar types can sometimes coerce; we do this during the
      // VerifyRetFail call -- we never allow this in HH files.
      auto const offset = BCSPRelOffset{0}
        .to<FPInvOffset>(inst->marker().spOff())
        .to<IRSPRelOffset>(irSPOff());
      setType(stk(offset), TGen);
    }
    break;

  case VerifyParamFail:
    if (!func()->unit()->isHHFile() && !RuntimeOption::EnableHipHopSyntax &&
        RuntimeOption::PHP7_ScalarTypes) {
      // In PHP 7 mode scalar types can sometimes coerce; we do this during the
      // VerifyParamFail call -- we never allow this in HH files.
      auto id = inst->src(0)->intVal();
      setType(loc(id), TGen);
    }
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
  if (!baseTmp->type().maybe(TPtrToGen)) return;

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
        case LTag::CSlot: always_assert(false); // Can't be a base
      }
      not_reached();
    }();

    if (maxType <= oldType) {
      // Drop the value and don't bother with precise effects.
      return setType(l, oldType);
    }
    if (oldType <= TBoxedCell) return;

    if (auto const baseType = effect_on(oldType)) {
      widenType(l, oldType | *baseType);
    }
  };

  if (base.isSingleLocation()) {
    // When the member base register refers to a single known memory location
    // `l' (with corresponding Ptr type `kind'), we apply the effect of `inst'
    // only to `l'.  Returns the base value type if `inst' had an effect.
    auto const apply_one = [&] (Location l, Ptr kind) -> folly::Optional<Type> {
      auto const oldTy = typeOf(l) & TGen;  // exclude TCls from ptr()
      if (auto const ptrTy = effect_on(oldTy.ptr(kind))) {
        auto const baseTy = ptrTy->derefIfPtr();
        setType(l, baseTy <= TBoxedCell ? TBoxedInitCell : baseTy);
        return baseTy;
      }
      return folly::none;
    };

    auto const apply_one_with_inner = [&] (Location l, Ptr kind) {
      if (auto const ty = apply_one(l, kind)) {
        if (*ty <= TBoxedCell) setBoxedPrediction(l, *ty);
      }
    };

    if (auto const bframe = base.frame()) {
      if (!isPM) {
        auto const l = loc(bframe->ids.singleValue());
        apply_one_with_inner(l, Ptr::Frame);
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
        apply_one_with_inner(Location::MBase{}, ptr);
      } else {
        apply(Location::MBase{});
      }
    }

    // We don't track anything else.
    return;
  }

  if (base.maybe(AFrameAny) && !isPM) {
    for (auto i = 0; i < cur().locals.size(); ++i) {
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
    X(Ref)      \
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
      for (auto i = 0; i < cur().locals.size(); ++i) {
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
              [&](PureSpillFrame m) { handle_stores(m.stk); },
              [&](CallEffects x) {
                if (x.writes_locals) handle_stores(AFrameAny);
                handle_stores(x.stack);
              },
              [&](PureLoad /*m*/) {}, [&](ReturnEffects) {},
              [&](ExitEffects) {}, [&](IrrelevantEffects) {},
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
      auto const t = local(i).type;
      auto const changed = local(i).maybeChanged;
      if (changed || t < TGen) {
        FTRACE(1, "Local {}: {} ({})\n", i, t.toString(),
               changed ? "changed" : "refined");
        auto& vec = changed ? pConds.changed : pConds.refined;
        vec.push_back({ Location::Local{i}, t });
      }
    }
  }

  auto const ty = mbase().type;
  auto const changed = mbase().maybeChanged;
  if (changed || ty < TGen) {
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

void FrameStateMgr::startBlock(Block* block, bool hasUnprocessedPred,
                               Block* pred) {
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
  } else if (debug || pred) {
    save(block, pred);
    if (pred) {
      assertx(hasStateFor(block));
      m_stack = m_states[block].in;
    }
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
  cur().frameMaySpanCall = true;
  cur().mbr = MBRState{};
  cur().mbase = MBaseState{};

  cur().fpiStack.clear();
  clearLocals();
  clearClsRefSlots();
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
  cur().fpiStack.pop_back();

  /*
   * Push a new state for the inlined callee; saving the state we'll need to
   * pop on return.
   */
  cur().bcSPOff = savedSPOff;
  auto stateCopy = m_stack.back();
  m_stack.emplace_back(std::move(stateCopy));

  /*
   * Set up the callee state.
   */
  cur().fpValue          = calleeFP;
  cur().ctx              = extra->ctx;
  cur().curFunc          = target;
  cur().frameMaySpanCall = false;
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
  cur().clsRefSlots.clear();
  cur().clsRefSlots.resize(target->numClsRefSlots());
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
void FrameStateMgr::trackCall(bool writesLocals) {
  if (writesLocals) clearLocals();
  for (auto& state : m_stack) {
    for (auto& loc : state.locals) {
      if (loc.value && !loc.value->inst()->is(DefConst)) loc.value = nullptr;
    }
    for (auto& stk : state.stack) {
      stk.value = nullptr;
    }
    for (auto& slot : state.clsRefSlots) {
      if (slot.value && !slot.value->inst()->is(DefConst)) slot.value = nullptr;
    }
    state.frameMaySpanCall = true;
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
 * Wrap a local, stack ID, or class-ref slot into a Location.
 */
Location FrameStateMgr::loc(uint32_t id) const {
  return Location::Local { id };
}
Location FrameStateMgr::stk(IRSPRelOffset off) const {
  auto const fpRel = off.to<FPInvOffset>(irSPOff());
  return Location::Stack { fpRel };
}
Location FrameStateMgr::cslot(uint32_t slot) const {
  return Location::CSlot { slot };
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

CSlotState& FrameStateMgr::clsRefSlotState(uint32_t slot) {
  assertx(slot < cur().clsRefSlots.size());
  auto& ret = cur().clsRefSlots[slot];

  assertx(ret.value == nullptr || ret.value->type() == ret.type);
  return ret;
}

CSlotState& FrameStateMgr::clsRefSlotState(Location l) {
  assertx(l.tag() == LTag::CSlot);
  return clsRefSlotState(l.clsRefSlot());
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

const CSlotState& FrameStateMgr::clsRefSlot(uint32_t slot) const {
  return const_cast<FrameStateMgr&>(*this).clsRefSlotState(slot);
}

#define IMPL_MEMBER_OF(type_t, name)                        \
  type_t FrameStateMgr::name##Of(Location l) const {        \
    return [&]() -> type_t {                                \
      switch (l.tag()) {                                    \
        case LTag::Local: return local(l.localId()).name;   \
        case LTag::Stack: return stack(l.stackIdx()).name;  \
        case LTag::MBase: return mbase().name;              \
        case LTag::CSlot: return clsRefSlot(l.clsRefSlot()).name;       \
      }                                                     \
      not_reached();                                        \
    }();                                                    \
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
  /*
   * We update the predicted type for boxed local values in some special cases
   * to something smart.
   */
  auto const predicted_local = [&]() -> folly::Optional<Type> {
    if (!value) return folly::none;
    auto const inst = value->inst();

    switch (inst->op()) {
      case LdLoc:
        if (value->type() <= TBoxedCell) {
          auto const fp = inst->src(0);
          auto const locID = inst->extra<LdLoc>()->locId;

          // Keep the same prediction as the src local.  It might have been
          // loaded in a parent frame, though, so we have to find the
          // appropriate FrameState.
          for (auto const& frame : boost::adaptors::reverse(m_stack)) {
            if (fp != frame.fpValue) continue;

            assertx(locID < frame.locals.size());
            return frame.locals[locID].predictedType;
          }
          // It's also possible it was loaded in the frame of a previously
          // inlined callee that we've already popped.  If that's the case,
          // just skip this optimization.
        }
        break;

      case Box:
        return boxType(inst->src(0)->type());

      default:
        break;
    }
    return folly::none;
  };

  switch (l.tag()) {
    case LTag::Local:
      return setValueImpl(l, localState(l), value, predicted_local());
    case LTag::Stack:
      cur().stackModified = true;
      return setValueImpl(l, stackState(l), value);
    case LTag::MBase:
      return setValueImpl(l, cur().mbase, value);
    case LTag::CSlot:
      return setValueImpl(l, clsRefSlotState(l), value);
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
    case LTag::CSlot:
      return setTypeImpl(l, clsRefSlotState(l), type);
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
    case LTag::CSlot:
      return widenTypeImpl(l, clsRefSlotState(l), type);
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
    case LTag::CSlot:
      return refineTypeImpl(l, clsRefSlotState(l), type, typeSrc);
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
    case LTag::CSlot: return refinePredictedTypeImpl(clsRefSlotState(l), type);
  }
  not_reached();
}

template<LTag tag>
static void setBoxedPredictionImpl(LocationState<tag>& state, Type type) {
  state.predictedType = refinePrediction(state.type, type, state.type);
}

/*
 * Set the predicted type for `l', discarding any previous prediction.
 */
void FrameStateMgr::setBoxedPrediction(Location l, Type type) {
  switch (l.tag()) {
    case LTag::Local: return setBoxedPredictionImpl(localState(l), type);
    case LTag::Stack: return setBoxedPredictionImpl(stackState(l), type);
    case LTag::MBase: return setBoxedPredictionImpl(cur().mbase, type);
    case LTag::CSlot: always_assert(false); // Never has a box
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

static const Func* getSpillFrameKnownCallee(const IRInstruction* inst) {
  if (!inst->is(SpillFrame)) return nullptr;

  const auto funcTmp = inst->src(1);
  if (!funcTmp->hasConstVal(TFunc)) return nullptr;

  return funcTmp->funcVal();
}

void FrameStateMgr::spillFrameStack(IRSPRelOffset offset,
                                    FPInvOffset retOffset,
                                    const IRInstruction* inst) {
  for (auto i = uint32_t{0}; i < kNumActRecCells; ++i) {
    setValue(stk(offset + i), nullptr);
  }
  auto const ctx = inst->op() == SpillFrame ? inst->src(2) : nullptr;

  const Func* func = getSpillFrameKnownCallee(inst);
  auto const opc = m_fpushOverride ?
    *m_fpushOverride : inst->marker().sk().op();
  m_fpushOverride.clear();

  cur().bcSPOff += kNumActRecCells;
  cur().fpiStack.push_back(FPIInfo {
    cur().spValue,
    retOffset,
    ctx ? ctx->type() : TCtx,
    ctx,
    opc,
    func,
    true /* inlineEligible */,
    false /* spans */
  });
}

void FrameStateMgr::writeToSpilledFrame(IRSPRelOffset offset,
                                        const SSATmp* sp) {
  auto const invOff = offset.to<FPInvOffset>(cur().irSPOff) - kNumActRecCells;
  for (auto& fpi : cur().fpiStack) {
    if (fpi.returnSP == sp && fpi.returnSPOff == invOff) {
      // The ops which write to a pre-live ActRec after the fact are generally
      // used when we don't have sufficient Func or Ctx information. This makes
      // it hard to predict what they actually write to the ActRec, so be
      // generic. Mark this entry as not eligible for inlining because the
      // generic type information isn't sufficient for inlining. (We usually
      // won't even try to inline such things anyways).
      fpi.ctxType = TCtx;
      fpi.ctx = nullptr;
      fpi.func = nullptr;
      fpi.inlineEligible = false;
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void FrameStateMgr::setLocalPredictedType(uint32_t id, Type type) {
  always_assert(id < cur().locals.size());
  auto& local = cur().locals[id];
  ITRACE(2, "updating local {}'s type prediction: {} -> {}\n",
    id, local.predictedType, type & local.type);
  local.predictedType = updatePrediction(type, local.type);
}

/*
 * This is called when we store into a BoxedCell.  Any locals that we know
 * point to that cell can have their inner type predictions updated.
 */
void FrameStateMgr::updateLocalRefPredictions(SSATmp* boxedCell, SSATmp* val) {
  assertx(boxedCell->type() <= TBoxedCell);
  for (auto id = uint32_t{0}; id < cur().locals.size(); ++id) {
    if (canonical(cur().locals[id].value) == canonical(boxedCell)) {
      setBoxedPrediction(loc(id), boxType(val->type()));
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

void FrameStateMgr::clearLocals() {
  ITRACE(2, "clearLocals\n");
  for (auto i = uint32_t{0}; i < cur().locals.size(); ++i) {
    setValue(loc(i), nullptr);
  }
}

///////////////////////////////////////////////////////////////////////////////

void FrameStateMgr::clearClsRefSlots() {
  ITRACE(2, "clearClsRefSlots\n");
  for (auto i = uint32_t{0}; i < cur().clsRefSlots.size(); ++i) {
    setValue(cslot(i), nullptr);
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
    "func: {}, spOff: {}{}",
    funcName,
    state.irSPOff().offset,
    state.frameMaySpanCall() ? ", frameMaySpanCall" : ""
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
