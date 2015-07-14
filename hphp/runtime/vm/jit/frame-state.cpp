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
#include "hphp/runtime/vm/jit/frame-state.h"

#include <algorithm>

#include "hphp/util/trace.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/stack-offsets-defs.h"

TRACE_SET_MOD(hhir);

namespace HPHP { namespace jit {

namespace {

using Trace::Indent;

//////////////////////////////////////////////////////////////////////

// Helper that sets a value to a new value and also returns whether it changed.
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

  if (merge_util(dst.type, dst.type | src.type)) {
    changed = true;
  }

  // Get the least common ancestor across both states.
  if (merge_util(dst.value, least_common_ancestor(dst.value, src.value))) {
    changed = true;
  }

  // We may have changed either dst.value or dst.type in a way that could fail
  // to preserve SlotState invariants.  So check if we can't keep the value.
  if (dst.value != nullptr && dst.value->type() != dst.type) {
    dst.value = nullptr;
    changed = true;
  }

  if (merge_into(dst.typeSrcs, src.typeSrcs)) {
    changed = true;
  }

  if (!dst.maybeChanged && src.maybeChanged) {
    dst.maybeChanged = true;
    changed = true;
  }

  return
    merge_util(dst.predictedType, dst.predictedType | src.predictedType) ||
    changed;
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
    if (merge_into(dst[i], src[i])) changed = true;
  }
  return changed;
}

/*
 * Merge one FrameState into another, returning whether it changed.  Frame
 * pointers and stack depth must match.  If the stack pointer tmps are
 * different, clear the tracked value (we can make a new one, given fp and
 * spOffset).
 */
bool merge_into(FrameState& dst, const FrameState& src) {
  auto changed = false;

  // Cannot merge spOffset state, so assert they match.
  always_assert(dst.spOffset == src.spOffset);
  always_assert(dst.curFunc == src.curFunc);

  // The only thing that can change the FP is inlining, but we can't have one
  // of the predecessors in an inlined callee while the other isn't.
  always_assert(dst.fpValue == src.fpValue);

  // FrameState for the same function must always have the same number of
  // locals.
  always_assert(src.locals.size() == dst.locals.size());

  // We must always have the same spValue.
  always_assert(dst.spValue == src.spValue);

  // This is available iff it's available in both states
  if (merge_util(dst.thisAvailable, dst.thisAvailable && src.thisAvailable)) {
    changed = true;
  }

  // The frame may span a call if it could have done so in either state.
  if (merge_util(dst.frameMaySpanCall,
                 dst.frameMaySpanCall || src.frameMaySpanCall)) {
    changed = true;
  }

  for (auto i = uint32_t{0}; i < src.locals.size(); ++i) {
    if (merge_into(dst.locals[i], src.locals[i])) {
      changed = true;
    }
  }

  if (merge_memory_stack_into(dst.memoryStack, src.memoryStack)) {
    changed = true;
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
    if (merge_into(dst[idx], src[idx])) changed = true;
  }
  return changed;
}

//////////////////////////////////////////////////////////////////////

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

  // We require the memory stack is always at least as big as the spOffset,
  // unless spOffset went negative (because we're returning and have freed the
  // ActRec).  Note that there are some "wasted" slots where locals/iterators
  // would be in the vector right now.
  always_assert_flog(
    state.spOffset < 0 || state.memoryStack.size() >= state.spOffset,
    "memoryStack was smaller than possible"
  );

  return true;
}

//////////////////////////////////////////////////////////////////////

}

/*
 * When we're computing an update for a new predicted type, we sometimes need
 * to fall back to the proven type, e.g. if the new predicted type is Bottom or
 * no longer satisfies the invariant that predictedType <= provenType.
 */
Type updatePredictedType(Type predictedType, Type provenType) {
  if (predictedType == TBottom) return provenType;
  if (predictedType <= provenType) return predictedType;
  return provenType;
}

Type refinePredictedType(Type oldPrediction, Type newPrediction, Type proven) {
  auto refinedPrediction = oldPrediction & newPrediction;
  if (refinedPrediction == TBottom) {
    refinedPrediction = newPrediction;
  }
  return updatePredictedType(refinedPrediction, proven);
}

FrameStateMgr::FrameStateMgr(BCMarker marker) {
  m_stack.push_back(FrameState{});
  cur().curFunc       = marker.func();
  cur().spOffset      = marker.spOff();
  cur().syncedSpLevel = marker.spOff();
  cur().locals.resize(marker.func()->numLocals());
  cur().memoryStack.resize(marker.spOff().offset);
}

bool FrameStateMgr::update(const IRInstruction* inst) {
  assertx(m_status != Status::None);
  ITRACE(3, "FrameStateMgr::update processing {}\n", *inst);
  Indent _i;

  auto changed = false;

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
    if (debug && m_status == Status::Building && taken->isCatch()) {
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
    if (m_status != Status::Building || taken->empty()) changed |= save(taken);
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
      // What we're considering sync'd to memory is popping an actrec, popping
      // args, and pushing a return value.
      if (m_status == Status::Building) {
        assertx(cur().syncedSpLevel == inst->marker().spOff());
      } else {
        cur().syncedSpLevel = inst->marker().spOff();
      }
      cur().syncedSpLevel -= extra->numParams + kNumActRecCells;
      cur().syncedSpLevel += 1;
    }
    break;

  case CallArray:
    {
      auto const extra = inst->extra<CallArray>();
      killLocalsForCall(extra->destroyLocals);
      for (auto& st : m_stack) st.frameMaySpanCall = true;
      // Remove tracked state for the actrec and array arg.
      for (auto i = uint32_t{0}; i < kNumActRecCells + 1; ++i) {
        setStackValue(extra->spOffset + i, nullptr);
      }
      clearStackForCall();
      setStackType(extra->spOffset + kNumActRecCells, TGen);
      // A CallArray pops the ActRec, an array arg, and pushes a return value.
      if (m_status == Status::Building) {
        assertx(cur().syncedSpLevel == inst->marker().spOff());
      } else {
        cur().syncedSpLevel = inst->marker().spOff();
      }
      cur().syncedSpLevel -= kNumActRecCells;
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
      if (m_status == Status::Building) {
        assertx(cur().syncedSpLevel == inst->marker().spOff());
      } else {
        cur().syncedSpLevel = inst->marker().spOff();
      }
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
    cur().spOffset = inst->extra<DefSP>()->offset;
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
    refineStackType(inst->extra<RelOffsetData>()->irSpOffset,
                    inst->typeParam(),
                    TypeSource::makeGuard(inst));
    break;

  case AssertStk:
    refineStackType(inst->extra<IRSPOffsetData>()->offset,
                    inst->typeParam(),
                    TypeSource::makeGuard(inst));
    break;

  case HintStkInner:
    setBoxedStkPrediction(inst->extra<HintStkInner>()->irSpOffset,
                          inst->typeParam());
    break;

  case AssertLoc:
  case CheckLoc:
    {
      auto id = inst->extra<LocalId>()->locId;
      if (inst->marker().func()->isPseudoMain()) {
        setLocalPredictedType(id, inst->typeParam());
      } else {
        refineLocalType(id,
                        inst->typeParam(),
                        TypeSource::makeGuard(inst));
      }
    }
    break;

  case HintLocInner:
    setBoxedLocalPrediction(inst->extra<HintLocInner>()->locId,
                            inst->typeParam());
    break;

  case StLoc:
    setLocalValue(inst->extra<LocalId>()->locId, inst->src(1));
    break;

  case LdLoc:
    setLocalValue(inst->extra<LdLoc>()->locId, inst->dst());
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
     * trace.  If the stack isn't clean or doesn't match the marker's spOffset,
     * the unwinder won't see what we expect.
     */
    always_assert_flog(
      cur().spOffset + -inst->extra<EndCatch>()->offset.offset ==
          inst->marker().spOff() &&
        cur().stackDeficit == 0 &&
        cur().evalStack.size() == 0,
      "EndCatch stack didn't seem right:\n"
      "                 spOff: {}\n"
      "       EndCatch offset: {}\n"
      "        marker's spOff: {}\n"
      "  eval stack def, size: {}, {}\n",
      cur().spOffset.offset,
      inst->extra<EndCatch>()->offset.offset,
      inst->marker().spOff().offset,
      cur().stackDeficit,
      cur().evalStack.size()
    );
    break;

  case CufIterSpillFrame:
    spillFrameStack(inst->extra<CufIterSpillFrame>()->spOffset);
    break;
  case SpillFrame:
    spillFrameStack(inst->extra<SpillFrame>()->spOffset);
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
        auto& loc = *it;
        // If changing the inner type of a boxed local, also drop the
        // information about inner types for any other boxed locals.
        if (loc.type <= TBoxedCell) dropLocalRefsInnerTypes();
        setLocalType(loc.id, loc.type);
      }
    }

    auto const spOffset = extra.spOffset;

    // Clear tracked information for slots pushed and popped.
    for (auto i = uint32_t{0}; i < extra.cellsPopped; ++i) {
      setStackValue(spOffset + i, nullptr);
    }
    for (auto i = uint32_t{0}; i < extra.cellsPushed; ++i) {
      setStackValue(spOffset + extra.cellsPopped - 1 - i, nullptr);
    }
    auto adjustedTop = spOffset + extra.cellsPopped - extra.cellsPushed;

    switch (extra.opcode) {
    case Op::CGetL2:
      setStackType(adjustedTop + 1, inst->typeParam());
      break;
    case Op::CGetL3:
      setStackType(adjustedTop + 2, inst->typeParam());
      break;
    default:
      // We don't track cells pushed by interp one except the top of the stack,
      // aside from above special cases.
      if (inst->hasTypeParam()) setStackType(adjustedTop, inst->typeParam());
      break;
    }

    cur().syncedSpLevel += extra.cellsPushed;
    cur().syncedSpLevel -= extra.cellsPopped;
    assertx(cur().evalStack.size() == 0);
    assertx(cur().stackDeficit == 0);
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

  default:
    if (MInstrEffects::supported(inst)) {
      auto const base = inst->src(minstrBaseIdx(inst->op()));

      // We require that minstrs that could affect stack/local locations take
      // the stack address as an direct source. This restriction may be relaxed
      // in the future (t7580150).
      if (base->inst()->is(LdStkAddr)) {
        auto const offset = base->inst()->extra<LdStkAddr>()->offset;
        auto const prevTy = stackType(offset);
        MInstrEffects effects(inst->op(), prevTy.ptr(Ptr::Stk));

        if (effects.baseTypeChanged || effects.baseValChanged) {
          auto const ty = effects.baseType.derefIfPtr();
          assert(cur().evalStack.empty());
          setStackType(
            offset,
            ty <= TBoxedCell ? TBoxedInitCell : ty
          );
        }
      } else if (base->inst()->is(LdLocAddr)) {
        auto const locId = base->inst()->extra<LdLocAddr>()->locId;
        auto const baseType = localType(locId);

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
      }
    }
    break;
  }

  return changed;
}

void FrameStateMgr::walkAllInlinedLocals(
    const std::function<void (uint32_t, unsigned, const LocalState&)>& body,
    bool skipThisFrame) const {
  auto doBody = [&] (const jit::vector<LocalState>& locals,
                     unsigned inlineIdx) {
    for (uint32_t i = 0, n = locals.size(); i < n; ++i) {
      body(i, inlineIdx, locals[i]);
    }
  };

  assertx(!m_stack.empty());
  auto const thisFrame = m_stack.size() - 1;
  if (!skipThisFrame) doBody(m_stack[thisFrame].locals, thisFrame);
  for (auto i = uint32_t{0}; i < thisFrame; ++i) {
    doBody(m_stack[i].locals, i);
  }
}

void FrameStateMgr::forEachLocalValue(
    const std::function<void (SSATmp*)>& body) const {
  for (auto& frame : m_stack) {
    for (auto& loc : frame.locals) {
      if (loc.value) body(loc.value);
    }
  }
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
    const auto& lastInst = block->back();
    const FPInvOffset bcSpOff = lastInst.marker().spOff();
    const FPInvOffset irSpOff = spOffset();
    const bool resumed = lastInst.marker().resumed();
    const auto skipCells = FPInvOffset{resumed ? 0 : func()->numSlotsInFrame()};
    const auto evalStkCells = bcSpOff - skipCells;
    for (int32_t i = 0; i < evalStkCells; i++) {
      const auto bcSpRel = BCSPOffset{i};
      const auto fpRel   = bcSpOff - bcSpRel;
      const auto irSpRel = toIRSPOffset(bcSpRel, bcSpOff, irSpOff);
      const auto type    = stackType(irSpRel);
      const bool changed = stackMaybeChanged(irSpRel);
      if (changed || type < TGen) {
        FTRACE(1, "Stack({}, {}): {} ({})\n", bcSpRel.offset, fpRel.offset,
               type, changed ? "changed" : "refined");
        auto& vec = changed ? pConds.changed : pConds.refined;
        vec.push_back({RegionDesc::Location::Stack{fpRel}, type});
      }
    }
  }

  if (fp() != nullptr) {
    for (unsigned i = 0; i < func()->numLocals(); i++) {
      auto t = localType(i);
      const bool changed = localMaybeChanged(i);
      if (changed || t < TGen) {
        FTRACE(1, "Local {}: {} ({})\n", i, t.toString(),
               changed ? "changed" : "refined");
        auto& vec = changed ? pConds.changed : pConds.refined;
        vec.push_back({ RegionDesc::Location::Local{i}, t });
      }
    }
  }
}

///// Methods for managing and merge block state /////

bool FrameStateMgr::hasStateFor(Block* block) const {
  return m_states.count(block);
}

void FrameStateMgr::startBlock(Block* block,
                               bool hasUnprocessedPred /* = false */) {
  ITRACE(3, "FrameStateMgr::startBlock: {}\n", block->id());
  assertx(m_status != Status::None);
  auto const it = m_states.find(block);
  auto const end = m_states.end();

  DEBUG_ONLY auto const predsAllowed =
    it != end || block->isEntry() || RuntimeOption::EvalJitLoops;
  assertx(IMPLIES(block->numPreds() > 0, predsAllowed));

  if (it != end) {
    if (m_status == Status::Building) {
      always_assert_flog(
        block->empty(),
        "tried to startBlock a non-empty block while building\n"
      );
    }
    ITRACE(4, "Loading state for B{}: {}\n", block->id(), show(*this));
    m_stack = it->second.in;
    if (m_stack.empty()) {
      always_assert_flog(0, "invalid startBlock for B{}", block->id());
    }
  } else {
    if (m_status == Status::Building) {
      if (debug) save(block);
    }
  }
  assertx(!m_stack.empty());

  // Don't reset state for unprocessed predecessors if we're trying to reach a
  // fixed-point.
  if (m_status == Status::RunningFixedPoint) return;

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
  // Don't save any new state if we've already reached a fixed-point.
  if (m_status == Status::FinishedFixedPoint) return false;

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
  cur().syncedSpLevel = savedSPOff;
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
  cur().syncedSpLevel    = FPInvOffset{target->numLocals()};

  /*
   * To set up spOffset, we want the FPInvOffset for the new fpValue and
   * spValue.  We're not changing spValue while we inline, but we're changing
   * fpValue, so this needs to change.  We know where the new fpValue is
   * pointing (relative to the spValue, which we aren't changing).  So we just
   * need to do a change of coordinates, which turns out to be an indentity map
   * here:
   *
   *    fpValue = spValue + extra->spOffset (an IRSPOffset)
   * So,
   *    spValue = fpValue - extra->spOffset (an FPInvOffset)
   */
  cur().spOffset = FPInvOffset{extra->spOffset.offset};
  FTRACE(6, "DefInlineFP setting spOffset: {}\n", cur().spOffset.offset);

  /*
   * Initialize tracked memory state for locals and stack slots to empty
   * values.
   */
  cur().locals.clear();
  cur().locals.resize(target->numLocals());
  cur().memoryStack.clear();
  cur().memoryStack.resize(std::max(cur().syncedSpLevel.offset,
                                    cur().spOffset.offset));
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

/*
 * Modify state to conservative values given an unprocessed predecessor.
 *
 * The fpValue, spOffset, and curFunc must agree at bytecode
 * control-flow merge points, so these are not cleared.
 */
void FrameStateMgr::clearForUnprocessedPred() {
  FTRACE(1, "clearForUnprocessedPred\n");

  cur().stackDeficit = 0;
  cur().evalStack    = EvalStack();

  // Forget any information about stack values in memory. Note that
  // the stack must be fully sync'ed to memory at merge points.
  for (auto& state : m_stack) {
    assertx(state.evalStack.empty());
    assertx(state.stackDeficit == 0);
    for (auto& stk : state.memoryStack) {
      stk = StackState{};
    }
  }

  // These two values must go toward their conservative state.
  cur().thisAvailable    = false;
  cur().frameMaySpanCall = true;

  clearLocals();
}

StackState& FrameStateMgr::stackState(IRSPOffset offset) {
  auto const idx = cur().spOffset.offset - offset.offset;
  FTRACE(6, "stackState offset: {} (@ spOff {}) --> idx={}\n",
    offset.offset, cur().spOffset.offset, idx);
  always_assert_flog(
    idx >= 0,
    "idx went negative: curSpOffset: {}, offset: {}\n",
    cur().spOffset.offset,
    offset.offset
  );
  if (idx >= cur().memoryStack.size()) {
    cur().memoryStack.resize(idx + 1);
  }
  return cur().memoryStack[idx];
}

const StackState& FrameStateMgr::stackState(IRSPOffset offset) const {
  // We consider it logically const to extend with default-constructed stack
  // values.
  return const_cast<FrameStateMgr&>(*this).stackState(offset);
}

void FrameStateMgr::computeFixedPoint(const BlockList& blocks,
                                      const BlockIDs& rpoIDs) {
  ITRACE(4, "FrameStateMgr computing fixed-point\n");

  assertx(m_status == Status::None);  // we should have a clear state
  m_status = Status::RunningFixedPoint;

  auto const entry = blocks[0];
  DEBUG_ONLY auto const entryMarker = entry->front().marker();
  // So that we can actually call startBlock on the entry block.
  assertx(m_stack.size() == 1);
  m_states[entry].in = m_stack;
  assertx(m_states[entry].in.back().curFunc == entryMarker.func());

  // Use a worklist of RPO ids. That way, when we remove an active item to
  // process, we'll always pick the block earliest in RPO.
  auto worklist = dataflow_worklist<uint32_t>(blocks.size());

  // Start with entry.
  worklist.push(0);

  while (!worklist.empty()) {
    auto const rpoId = worklist.pop();
    auto const block = blocks[rpoId];

    ITRACE(5, "Processing block {}\n", block->id());

    auto const insert = [&] (Block* block) {
      if (block != nullptr) worklist.push(rpoIDs[block]);
    };

    startBlock(block);

    for (auto& inst : *block) {
      if (update(&inst)) insert(block->taken());
    }

    if (finishBlock(block)) insert(block->next());
  }

  m_status = Status::FinishedFixedPoint;
}

void FrameStateMgr::loadBlock(Block* block) {
  auto const it = m_states.find(block);
  assertx(it != m_states.end());
  m_stack = it->second.in;
  assertx(!m_stack.empty());
}

const PostConditions& FrameStateMgr::postConds(Block* exitBlock) const {
  assertx(exitBlock->isExitNoThrow());
  auto it = m_exitPostConds.find(exitBlock);
  assertx(it != m_exitPostConds.end());
  return it->second;
}

void FrameStateMgr::syncEvalStack() {
  cur().syncedSpLevel += cur().evalStack.size() - cur().stackDeficit;
  cur().evalStack.clear();
  cur().stackDeficit = 0;
  FTRACE(2, "syncEvalStack --- level {}\n", cur().syncedSpLevel.offset);
}

SSATmp* FrameStateMgr::localValue(uint32_t id) const {
  always_assert(id < cur().locals.size());
  auto const& local = cur().locals[id];
  assert(local.value == nullptr || local.value->type() == local.type);
  return local.value;
}

TypeSourceSet FrameStateMgr::localTypeSources(uint32_t id) const {
  always_assert(id < cur().locals.size());
  return cur().locals[id].typeSrcs;
}

Type FrameStateMgr::localType(uint32_t id) const {
  always_assert(id < cur().locals.size());
  return cur().locals[id].type;
}

bool FrameStateMgr::localMaybeChanged(uint32_t id) const {
  always_assert(id < cur().locals.size());
  return cur().locals[id].maybeChanged;
}

Type FrameStateMgr::predictedLocalType(uint32_t id) const {
  always_assert(id < cur().locals.size());
  auto const ty = cur().locals[id].predictedType;
  ITRACE(2, "Predicting {}: {}\n", id, ty);
  return ty;
}

Type FrameStateMgr::stackType(IRSPOffset offset) const {
  return stackState(offset).type;
}

bool FrameStateMgr::stackMaybeChanged(IRSPOffset offset) const {
  return stackState(offset).maybeChanged;
}

Type FrameStateMgr::predictedStackType(IRSPOffset offset) const {
  return stackState(offset).predictedType;
}

SSATmp* FrameStateMgr::stackValue(IRSPOffset offset) const {
  const auto& state = stackState(offset);
  assert(state.value == nullptr || state.value->type() == state.type);
  return state.value;
}

TypeSourceSet FrameStateMgr::stackTypeSources(IRSPOffset offset) const {
  return stackState(offset).typeSrcs;
}

void FrameStateMgr::setStackValue(IRSPOffset offset, SSATmp* value) {
  auto& stk = stackState(offset);
  FTRACE(2, "stk[{}] := {}\n", offset.offset,
    value ? value->toString() : std::string("<>"));
  stk.value         = value;
  stk.type          = value ? value->type() : TStkElem;
  stk.maybeChanged  = true;
  stk.predictedType = stk.type;
  stk.typeSrcs.clear();
  if (value) {
    stk.typeSrcs.insert(TypeSource::makeValue(value));
  }
}

void FrameStateMgr::setStackType(IRSPOffset offset, Type type) {
  auto& stk = stackState(offset);
  FTRACE(2, "stk[{}] :: {}\n", offset.offset, type.toString());
  stk.value         = nullptr;
  stk.type          = type;
  stk.maybeChanged  = true;
  stk.predictedType = type;
  stk.typeSrcs.clear();
}

void FrameStateMgr::setBoxedStkPrediction(IRSPOffset offset, Type type) {
  auto& state = stackState(offset);
  state.predictedType = state.type & type;
}

void FrameStateMgr::spillFrameStack(IRSPOffset offset) {
  for (auto i = uint32_t{0}; i < kNumActRecCells; ++i) {
    setStackValue(offset + i, nullptr);
  }
  cur().syncedSpLevel += kNumActRecCells;
}

void FrameStateMgr::refineStackType(IRSPOffset offset,
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
    for (auto& stk : state.memoryStack) {
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
  cur().locals[id].value        = value;
  auto const newType            = value ? value->type() : TGen;
  cur().locals[id].type         = newType;
  cur().locals[id].maybeChanged = true;
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
    return cur().locals[id].type;  // just predict what we know
  }();

  // We need to make sure not to violate the invariant that predictedType is
  // always <= type.  Note that operator& can be conservative (it could just
  // return one of the two types in situations relating to specialized types we
  // can't represent), so it's necessary to double check.
  auto const rawIsect = newType & newInnerPred;
  auto const useTy = rawIsect <= newType ? rawIsect : newType;

  FTRACE(3, "setLocalValue setting prediction {} based on {}, using = {}\n",
    id, newInnerPred, useTy);
  cur().locals[id].predictedType = useTy;

  cur().locals[id].typeSrcs.clear();
  if (value) {
    cur().locals[id].typeSrcs.insert(TypeSource::makeValue(value));
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
}

void FrameStateMgr::refineStackPredictedType(IRSPOffset offset, Type type) {
  auto& state = stackState(offset);
  state.predictedType = refinePredictedType(
    state.predictedType, type, state.type);
}

void FrameStateMgr::setLocalType(uint32_t id, Type type) {
  always_assert(id < cur().locals.size());
  cur().locals[id].value         = nullptr;
  cur().locals[id].type          = type;
  cur().locals[id].maybeChanged  = true;
  cur().locals[id].predictedType = type;
  cur().locals[id].typeSrcs.clear();
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
    for (auto& slot : frame.memoryStack) {
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
    state.spOffset().offset,
    state.thisAvailable() ? ", thisAvailable" : "",
    state.frameMaySpanCall() ? ", frameMaySpanCall" : ""
  ).str();
}

//////////////////////////////////////////////////////////////////////

}}
