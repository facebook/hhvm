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
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

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
 * Merge LocalStates, returning whether anything changed.
 */
bool merge_into(LocalState& dst, const LocalState& src) {
  auto changed = false;

  // Get the least common ancestor across both states.
  if (merge_util(dst.value, least_common_ancestor(dst.value, src.value))) {
    changed = true;
  }

  if (merge_into(dst.typeSrcs, src.typeSrcs)) {
    changed = true;
  }

  changed =
    merge_util(dst.type, Type::unionOf(dst.type, src.type)) ||
    merge_util(dst.boxedPrediction,
               Type::unionOf(dst.boxedPrediction, src.boxedPrediction)) ||
    changed;

  // Throw away the prediction if we merged Type::Gen for the type.
  if (!(dst.type <= Type::BoxedInitCell)) {
    changed = merge_util(dst.boxedPrediction, Type::Bottom);
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

  if (dst.spValue != src.spValue) {
    // We have two different sp definitions but we know they're equal because
    // spOffset matched.
    dst.spValue = nullptr;
    changed = true;
  }

  // this is available iff it's available in both states
  if (merge_util(dst.thisAvailable, dst.thisAvailable && src.thisAvailable)) {
    changed = true;
  }

  for (auto i = uint32_t{0}; i < src.locals.size(); ++i) {
    if (merge_into(dst.locals[i], src.locals[i])) {
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
    if (merge_into(dst[idx], src[idx])) changed = true;
  }
  return changed;
}

//////////////////////////////////////////////////////////////////////

}

FrameStateMgr::FrameStateMgr(IRUnit& unit, BCMarker marker)
  : FrameStateMgr(unit, marker.spOff(), marker.func())
{
  assert(!marker.isDummy());
}

FrameStateMgr::FrameStateMgr(IRUnit& unit,
                             Offset initialSpOffset,
                             const Func* func)
  : m_unit(unit)
  , m_visited(unit.numBlocks())
{
  m_stack.push_back(FrameState{});
  cur().curFunc = func;
  cur().spOffset = initialSpOffset;
  cur().locals.resize(func ? func->numLocals() : 0);
}

bool FrameStateMgr::update(const IRInstruction* inst) {
  ITRACE(3, "FrameStateMgr::update processing {}\n", *inst);
  Indent _i;

  auto changed = false;

  if (auto const taken = inst->taken()) {
    // When we're building the IR, we append a conditional jump after
    // generating its target block: see emitJmpCondHelper, where we
    // call makeExit() before gen(JmpZero).  It doesn't make sense to
    // update the target block state at this point, so don't.  The
    // state doesn't have this problem during optimization passes,
    // because we'll always process the jump before the target block.
    if (m_status != Status::Building || taken->empty()) changed |= save(taken);
  }

  auto const opc = inst->op();

  getLocalEffects(inst, *this);
  assert(checkInvariants());

  switch (opc) {
  case DefInlineFP:    trackDefInlineFP(inst);  break;
  case InlineReturn:   trackInlineReturn(); break;

  case Call:
    cur().spValue = inst->dst();
    for (auto& st : m_stack) st.frameSpansCall = true;
    // A call pops the ActRec and the arguments, and then pushes a
    // return value.
    cur().spOffset -= kNumActRecCells + inst->extra<Call>()->numParams;
    cur().spOffset += 1;
    assert(cur().spOffset >= 0);
    break;

  case CallArray:
    cur().spValue = inst->dst();
    for (auto& st : m_stack) st.frameSpansCall = true;
    // A CallArray pops the ActRec an array arg and pushes a return value.
    cur().spOffset -= kNumActRecCells;
    assert(cur().spOffset >= 0);
    break;

  case ContEnter:
    cur().spValue = inst->dst();
    for (auto& st : m_stack) st.frameSpansCall = true;
    break;

  case DefFP:
  case FreeActRec:
    cur().fpValue = inst->dst();
    break;

  case ReDefSP:
    cur().spValue = inst->dst();
    cur().spOffset = inst->extra<ReDefSP>()->spOffset;
    break;

  case DefSP:
    cur().spValue = inst->dst();
    cur().spOffset = inst->extra<StackOffset>()->offset;
    break;

  case AssertStk:
  case CastStk:
  case CastStkIntToDbl:
  case CoerceStk:
  case CheckStk:
  case GuardStk:
  case HintStkInner:
  case ExceptionBarrier:
    cur().spValue = inst->dst();
    break;

  case SpillStack: {
    cur().spValue = inst->dst();
    // Push the spilled values but adjust for the popped values
    int64_t stackAdjustment = inst->src(1)->intVal();
    cur().spOffset -= stackAdjustment;
    cur().spOffset += spillValueCells(inst);
    break;
  }

  case SpillFrame:
  case CufIterSpillFrame:
    cur().spValue = inst->dst();
    cur().spOffset += kNumActRecCells;
    break;

  case InterpOne:
  case InterpOneCF: {
    cur().spValue = inst->dst();
    auto const& extra = *inst->extra<InterpOneData>();
    int64_t stackAdjustment = extra.cellsPopped - extra.cellsPushed;
    // push the return value if any and adjust for the popped values
    cur().spOffset -= stackAdjustment;
    break;
  }

  case CheckCtxThis:
    cur().thisAvailable = true;
    break;

  case DefLabel:
    if (inst->numDsts() > 0) {
      auto dst0 = inst->dst(0);
      if (dst0->isA(Type::StkPtr)) cur().spValue = dst0;
    }
    break;

  default:
    break;
  }

  if (inst->modifiesStack()) {
    cur().spValue = inst->modifiedStkPtr();
  }

  return changed;
}

void FrameStateMgr::getLocalEffects(const IRInstruction* inst,
                                 LocalStateHook& hook) const {
  auto killIterLocals = [&](const std::initializer_list<uint32_t>& ids) {
    for (auto id : ids) {
      hook.setLocalValue(id, nullptr);
    }
  };

  auto killedCallLocals = false;
  if ((inst->is(CallArray) && inst->extra<CallArray>()->destroyLocals) ||
      (inst->is(Call) && inst->extra<Call>()->destroyLocals) ||
      (inst->is(CallBuiltin) && inst->extra<CallBuiltin>()->destroyLocals)) {
    clearLocals(hook);
    killedCallLocals = true;
  }

  switch (inst->op()) {
    case Call:
    case CallArray:
    case ContEnter:
      killLocalsForCall(hook, killedCallLocals);
      break;

    case StRef:
      updateLocalRefPredictions(hook, inst->src(0), inst->src(1));
      break;

    case StLocNT:
    case StLoc:
      hook.setLocalValue(inst->extra<LocalId>()->locId, inst->src(1));
      break;

    case LdLocPseudoMain: {
      auto const type = inst->typeParam().relaxToGuardable();
      auto id = inst->extra<LdLocPseudoMain>()->locId;
      hook.setLocalType(id, type);  // TODO(#5635167): not always true
      break;
    }
    case StLocPseudoMain: {
      auto const type = inst->src(1)->type().relaxToGuardable();
      auto id = inst->extra<StLocPseudoMain>()->locId;
      hook.setLocalType(id, type);  // TODO(#5635167): not always true
      break;
    }

    case LdLoc:
      hook.setLocalValue(inst->extra<LdLoc>()->locId, inst->dst());
      break;

    case AssertLoc:
    case GuardLoc:
    case CheckLoc: {
      auto id = inst->extra<LocalId>()->locId;
      hook.refineLocalType(id, inst->typeParam(), TypeSource::makeGuard(inst));
      break;
    }

    case HintLocInner:
      hook.setBoxedLocalPrediction(inst->extra<HintLocInner>()->locId,
                                   inst->typeParam());
      break;

    case TrackLoc:
      hook.setLocalValue(inst->extra<TrackLoc>()->locId, inst->src(0));
      break;

    case CheckType:
    case AssertType: {
      SSATmp* newVal = inst->dst();
      SSATmp* oldVal = inst->src(0);
      refineLocalValues(hook, oldVal, newVal);
      break;
    }

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

    case InterpOne:
    case InterpOneCF: {
      auto const& id = *inst->extra<InterpOneData>();
      assert(!id.smashesAllLocals || id.nChangedLocals == 0);
      if (id.smashesAllLocals) {
        clearLocals(hook);
      } else {
        auto it = id.changedLocals;
        auto const end = it + id.nChangedLocals;
        for (; it != end; ++it) {
          auto& loc = *it;
          // If changing the inner type of a boxed local, also drop the
          // information about inner types for any other boxed locals.
          if (loc.type.isBoxed()) dropLocalRefsInnerTypes(hook);
          hook.setLocalType(loc.id, loc.type);
        }
      }
      break;
    }
    default:
      break;
  }

  if (MInstrEffects::supported(inst)) MInstrEffects::get(inst, *this, hook);
}

///// Support helpers for getLocalEffects /////
void FrameStateMgr::clearLocals(LocalStateHook& hook) const {
  for (unsigned i = 0; i < cur().locals.size(); ++i) {
    hook.setLocalValue(i, nullptr);
  }
}

void FrameStateMgr::refineLocalValues(LocalStateHook& hook,
                                   SSATmp* oldVal, SSATmp* newVal) const {
  assert(newVal->inst()->is(CheckType, AssertType));
  assert(newVal->inst()->src(0) == oldVal);

  walkAllInlinedLocals(
    [&](uint32_t i, unsigned inlineIdx, const LocalState& local) {
      if (canonical(local.value) == canonical(oldVal)) {
        hook.refineLocalValue(i, inlineIdx, oldVal, newVal);
      }
    }
  );
}

void FrameStateMgr::forEachFrame(FrameFunc body) const {
  for (auto it = m_stack.rbegin(); it != m_stack.rend(); ++it) {
    body(it->fpValue, it->spOffset);
  }
}

template<typename L>
void FrameStateMgr::walkAllInlinedLocals(L body, bool skipThisFrame) const {
  auto doBody = [&] (const jit::vector<LocalState>& locals,
                     unsigned inlineIdx) {
    for (uint32_t i = 0, n = locals.size(); i < n; ++i) {
      body(i, inlineIdx, locals[i]);
    }
  };

  // For historical reasons: inlineIdx == 0 means current frame, and otherwise
  // it's m_stack index + 1.
  assert(!m_stack.empty());
  auto const thisFrame = m_stack.size() - 1;
  if (!skipThisFrame) doBody(m_stack[thisFrame].locals, 0);
  for (int i = 0; i < thisFrame; ++i) {
    doBody(m_stack[i].locals, i + 1);
  }
}

void FrameStateMgr::forEachLocal(LocalFunc body) const {
  walkAllInlinedLocals(
    [&](uint32_t i, unsigned inlineIdx, const LocalState& local) {
      body(i, local.value);
    }
  );
}

/**
 * Called to clear out the tracked local values at a call site.  Calls kill all
 * registers, so we don't want to keep locals in registers across calls. We do
 * continue tracking the types in locals, however.
 */
void FrameStateMgr::killLocalsForCall(LocalStateHook& hook,
                                   bool skipThisFrame) const {
  walkAllInlinedLocals(
    [&](uint32_t i, unsigned inlineIdx, const LocalState& local) {
      auto* value = local.value;
      if (!value || value->inst()->is(DefConst)) return;

      hook.killLocalForCall(i, inlineIdx, value);
    },
    skipThisFrame
  );
}

/*
 * This is called when we store into a BoxedCell.  Any locals that we know
 * point to that cell can have their inner type predictions updated.
 */
void FrameStateMgr::updateLocalRefPredictions(LocalStateHook& hook,
                                           SSATmp* boxedCell,
                                           SSATmp* val) const {
  assert(boxedCell->type().isBoxed());
  for (auto id = uint32_t{0}; id < cur().locals.size(); ++id) {
    if (canonical(cur().locals[id].value) == canonical(boxedCell)) {
      hook.setBoxedLocalPrediction(id, boxType(val->type()));
    }
  }
}

/**
 * This method changes any boxed local into a BoxedInitCell type. It's safe to
 * assume they're init because you can never have a reference to uninit.
 */
void FrameStateMgr::dropLocalRefsInnerTypes(LocalStateHook& hook) const {
  walkAllInlinedLocals(
    [&](uint32_t i, unsigned inlineIdx, const LocalState& local) {
      if (local.type.isBoxed()) {
        hook.dropLocalInnerType(i, inlineIdx);
      }
    }
  );
}

///// Methods for managing and merge block state /////

bool FrameStateMgr::hasStateFor(Block* block) const {
  return m_states.count(block);
}

Block* FrameStateMgr::findUnprocessedPred(Block* block) const {
  for (auto const& edge : block->preds()) {
    auto const pred = edge.from();
    if (!isVisited(pred)) return pred;
  }
  return nullptr;
}

const jit::vector<LocalState>&
FrameStateMgr::localsLeavingBlock(Block* block) const {
  auto const it = m_states.find(block);
  assert(it != m_states.end());
  assert(!it->second.out.empty());
  return it->second.out.back().locals;
}

SSATmp* FrameStateMgr::spLeavingBlock(Block* b) const {
  auto it = m_states.find(b);
  assert(it != m_states.end());
  assert(!it->second.out.empty());
  return it->second.out.back().spValue;
}

void FrameStateMgr::startBlock(Block* block,
                            BCMarker marker,
                            LocalStateHook* hook /* = nullptr */,
                            bool isLoopHeader /* = false */) {
  auto const it = m_states.find(block);
  auto const end = m_states.end();

  DEBUG_ONLY auto const predsAllowed =
    it != end || block->isEntry() || RuntimeOption::EvalJitLoops;
  assert(IMPLIES(block->numPreds() > 0, predsAllowed));

  if (it != end) {
    ITRACE(4, "Loading state for B{}: {}\n", block->id(), show(*this));
    m_stack = it->second.in;
    if (m_stack.empty()) {
      /*
       * TODO(#4323657): In situations like catch or exit traces, we will call
       * startBlock after pausing whatever block we had been working on, which
       * will leave the catch with no in state.  And in fact we don't really
       * know what its in-state should be because we don't have any
       * information.  But this is pretty dangerous right now, because we're
       * starting with things like spValue as null.
       */
      m_stack.push_back(FrameState{});
    }
  }
  assert(!m_stack.empty());

  // Don't reset state for unprocessed predecessors if we're trying to reach a
  // fixed-point.
  if (m_status == Status::RunningFixedPoint) return;

  // Reset state if the block has an unprocessed predecessor.
  if (isLoopHeader || findUnprocessedPred(block)) {
    Indent _;
    ITRACE(4, "B{} has unprocessed predecessor, resetting state\n",
           block->id());
    loopHeaderClear(marker, hook);
  }

  markVisited(block);
}

bool FrameStateMgr::finishBlock(Block* block) {
  assert(block->back().isTerminal() == !block->next());

  auto& old = m_states[block].out;
  const auto& snap = m_stack;
  auto changed = old != snap;
  old = snap;

  if (!block->back().isTerminal()) changed |= save(block->next());

  return changed;
}

void FrameStateMgr::pauseBlock(Block* block) {
  // Note: this can't use std::move, because pauseBlock must leave the current
  // state alone.
  m_states[block].out = m_stack;
}

void FrameStateMgr::unpauseBlock(Block* block) {
  assert(hasStateFor(block));
  m_stack = m_states[block].out;
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
    assert(!m_stack.empty());
    m_states[block].in = m_stack;
  }

  return changed;
}

void FrameStateMgr::trackDefInlineFP(const IRInstruction* inst) {
  auto const target     = inst->extra<DefInlineFP>()->target;
  auto const savedSPOff = inst->extra<DefInlineFP>()->retSPOff;
  auto const calleeFP   = inst->dst();
  auto const calleeSP   = inst->src(0);
  auto const savedSP    = inst->src(1);

  // Saved IRBuilder state will include the "return" fp/sp.
  // Whatever the current fpValue is is good enough, but we have to be
  // passed in the StkPtr that represents the stack prior to the
  // ActRec being allocated.
  cur().spOffset = savedSPOff;
  cur().spValue = savedSP;

  auto const stackValues = collectStackValues(cur().spValue,
                                              cur().spOffset);
  for (DEBUG_ONLY auto& val : stackValues) {
    ITRACE(4, "    marking caller stack value available: {}\n",
           val->toString());
  }

  /*
   * Push a new state for the inlined callee.
   */
  auto stateCopy = m_stack.back();
  m_stack.emplace_back(std::move(stateCopy));

  /*
   * Set up the callee state.
   *
   * We set m_thisIsAvailable to true on any object method, because we
   * just don't inline calls to object methods with a null $this.
   */
  cur().fpValue         = calleeFP;
  cur().spValue         = calleeSP;
  cur().thisAvailable   = target->cls() != nullptr && !target->isStatic();
  cur().curFunc         = target;
  cur().frameSpansCall  = false;

  cur().locals.clear();
  cur().locals.resize(target->numLocals());
}

void FrameStateMgr::trackInlineReturn() {
  m_stack.pop_back();
  assert(!m_stack.empty());
}

void FrameStateMgr::clear() {
  // A previous run of reoptimize could've legitimately exited the trace in an
  // inlined callee. If that happened, just pop all the saved states to return
  // to the top-level func.
  while (inlineDepth()) {
    trackInlineReturn();
  }
  clearCurrentState();
  m_states.clear();
  m_visited.clear();
  assert(m_stack.size() == 1);
  m_status = Status::None;
}

bool FrameStateMgr::checkInvariants() const {
  walkAllInlinedLocals(
    [&] (uint32_t id, unsigned inlineIdx, const LocalState& local) {
      always_assert_flog(
        local.boxedPrediction <= local.type &&
        local.boxedPrediction <= Type::BoxedInitCell &&
        IMPLIES(local.boxedPrediction != Type::Bottom,
                local.type <= Type::BoxedInitCell),
        "local {} failed boxed invariants; pred = {}, type = {}\n",
        id,
        local.boxedPrediction,
        local.type
      );
    }
  );
  return true;
}

void FrameStateMgr::clearCurrentState() {
  cur().spValue        = nullptr;
  cur().fpValue        = nullptr;
  cur().spOffset       = 0;
  cur().marker         = BCMarker();
  cur().thisAvailable  = false;
  cur().frameSpansCall = false;
  cur().stackDeficit   = 0;
  cur().evalStack      = EvalStack();
  clearLocals(*this);
}

void FrameStateMgr::loopHeaderClear(BCMarker marker,
                                 LocalStateHook* hook /* = nullptr */) {
  cur().spValue        = nullptr;
  cur().marker         = marker;
  cur().spOffset       = marker.spOff();
  cur().curFunc        = marker.func();
  cur().stackDeficit   = 0;
  cur().evalStack      = EvalStack();

  // Clear hook first so that it can read local info from the FrameStateMgr.
  if (hook != nullptr) clearLocals(*hook);
  clearLocals(*this);
}

void FrameStateMgr::resetCurrentState(BCMarker marker) {
  clearCurrentState();
  cur().marker   = marker;
  cur().spOffset = marker.spOff();
  cur().curFunc  = marker.func();
}

void FrameStateMgr::computeFixedPoint(const BlocksWithIds& blocks) {
  ITRACE(4, "FrameStateMgr computing fixed-point\n");

  clear();

  m_status = Status::RunningFixedPoint;

  auto const entry = blocks.blocks[0];
  auto const entryMarker = entry->front().marker();

  // So that we can actually call startBlock on the entry block.
  m_states[entry].in.push_back(FrameState{});
  m_states[entry].in.back().curFunc = entryMarker.func();
  m_states[entry].in.back().marker = entryMarker;
  m_states[entry].in.back().locals.resize(entryMarker.func()->numLocals());
  resetCurrentState(entryMarker);

  // Use a worklist of RPO ids. That way, when we remove an active item to
  // process, we'll always pick the block earliest in RPO.
  auto worklist = dataflow_worklist<uint32_t>(blocks.blocks.size());

  // Start with entry.
  worklist.push(0);

  while (!worklist.empty()) {
    auto const rpoId = worklist.pop();
    auto const block = blocks.blocks[rpoId];

    ITRACE(5, "Processing block {}\n", block->id());

    auto const insert = [&] (Block* block) {
      if (block != nullptr) worklist.push(blocks.ids[block]);
    };

    startBlock(block, block->front().marker());

    for (auto& inst : *block) {
      setMarker(inst.marker());
      if (update(&inst)) insert(block->taken());
    }

    if (finishBlock(block)) insert(block->next());
  }

  resetCurrentState(entryMarker);

  m_status = Status::FinishedFixedPoint;
}

void FrameStateMgr::loadBlock(Block* block) {
  auto const it = m_states.find(block);
  assert(it != m_states.end());
  m_stack = it->second.in;
  assert(!m_stack.empty());
}

SSATmp* FrameStateMgr::localValue(uint32_t id) const {
  always_assert(id < cur().locals.size());
  return cur().locals[id].value;
}

TypeSourceSet FrameStateMgr::localTypeSources(uint32_t id) const {
  always_assert(id < cur().locals.size());
  return cur().locals[id].typeSrcs;
}

Type FrameStateMgr::localType(uint32_t id) const {
  always_assert(id < cur().locals.size());
  return cur().locals[id].type;
}

Type FrameStateMgr::predictedInnerType(uint32_t id) const {
  always_assert(id < cur().locals.size());
  always_assert(cur().locals[id].type.isBoxed());
  if (cur().locals[id].boxedPrediction <= Type::Bottom) {
    ITRACE(2, "No prediction: {}\n", id);
    return Type::InitCell;
  }
  auto t = ldRefReturn(cur().locals[id].boxedPrediction.unbox());
  ITRACE(2, "Predicting{}: {}\n", id, t);
  return t;
}

void FrameStateMgr::setLocalValue(uint32_t id, SSATmp* value) {
  always_assert(id < cur().locals.size());
  cur().locals[id].value = value;
  cur().locals[id].type = value ? value->type() : Type::Gen;

  /*
   * Update the inner-type prediction for boxed values in some special cases to
   * something smart.  The rest of the time, throw it away.
   */
  auto const newInnerPred = [&]() -> Type {
    if (!value) return Type::Bottom;
    auto const inst = value->inst();
    switch (inst->op()) {
    case LdLoc:
      if (value->type().isBoxed()) {
        // Keep the same prediction as this local.
        return cur().locals[inst->extra<LdLoc>()->locId].boxedPrediction;
      }
      break;
    case Box:
      return boxType(inst->src(0)->type());
    default:
      break;
    }
    return Type::Bottom;
  }();

  FTRACE(3, "setLocalValue setting inner prediction {} to {}\n",
    id, newInnerPred);
  cur().locals[id].boxedPrediction = newInnerPred;

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
  Type newType = refineTypeNoCheck(local.type, type);
  ITRACE(2, "updating local {}'s type: {} -> {}\n", id, local.type, newType);
  local.type = newType;
  local.typeSrcs.clear();
  local.typeSrcs.insert(typeSrc);
}

void FrameStateMgr::setLocalType(uint32_t id, Type type) {
  always_assert(id < cur().locals.size());
  cur().locals[id].value = nullptr;
  cur().locals[id].type = type;
  cur().locals[id].boxedPrediction = Type::Bottom;
  cur().locals[id].typeSrcs.clear();
}

void FrameStateMgr::setBoxedLocalPrediction(uint32_t id, Type type) {
  always_assert(id < cur().locals.size());
  always_assert(type <= Type::BoxedCell);
  always_assert_flog(
    cur().locals[id].type == Type::BoxedInitCell ||
    cur().locals[id].type == Type::Gen,
    "PredictLocInner {} with base type {}",
    id,
    cur().locals[id].type
  );
  if (cur().locals[id].type <= Type::BoxedCell) {
    cur().locals[id].boxedPrediction = type;
  } else {
    cur().locals[id].boxedPrediction = Type::Bottom;
  }
}

void FrameStateMgr::setLocalTypeSource(uint32_t id, TypeSource typeSrc) {
  always_assert(id < cur().locals.size());
  cur().locals[id].typeSrcs.clear();
  cur().locals[id].typeSrcs.insert(typeSrc);
}

/*
 * Get a reference to the locals from an inline index. 0 means the current
 * frame, otherwise it's index (inlineIdx - 1) in m_inlineSavedStates.
 *
 * FIXME: change what inlineIdx means
 */
jit::vector<LocalState>& FrameStateMgr::locals(unsigned inlineIdx) {
  if (inlineIdx == 0) {
    return cur().locals;
  } else {
    --inlineIdx;
    assert(inlineIdx < m_stack.size());
    return m_stack[inlineIdx].locals;
  }
}

void FrameStateMgr::refineLocalValue(uint32_t id, unsigned inlineIdx,
                                  SSATmp* oldVal, SSATmp* newVal) {
  auto& locs = locals(inlineIdx);
  always_assert(id < locs.size());
  auto& local = locs[id];
  ITRACE(2, "refining local {}'s ({}) value: {} -> {}\n",
         id, inlineIdx, *local.value, *newVal);
  local.value = newVal;
  local.type = newVal->type();
  local.typeSrcs.clear();
  local.typeSrcs.insert(TypeSource::makeValue(newVal));
}

void FrameStateMgr::killLocalForCall(uint32_t id, unsigned inlineIdx,
                                  SSATmp* val) {
  auto& locs = locals(inlineIdx);
  always_assert(id < locs.size());
  locs[id].value = nullptr;
}

void FrameStateMgr::dropLocalInnerType(uint32_t id, unsigned inlineIdx) {
  auto& local = locals(inlineIdx)[id];
  assert(local.type.isBoxed());
  local.boxedPrediction = local.type = Type::BoxedInitCell;
}

void FrameStateMgr::markVisited(const Block* b) {
  // The number of blocks in the unit can change over time.
  if (b->id() >= m_visited.size()) {
    m_visited.resize(b->id() + 1);
  }

  m_visited.set(b->id());
}

bool FrameStateMgr::isVisited(const Block* b) const {
  return b->id() < m_visited.size() && m_visited.test(b->id());
}

std::string show(const FrameStateMgr& state) {
  auto bcOff = state.marker().valid() ? state.marker().bcOff() : -1;
  auto func = state.func();
  auto funcName = func ? func->fullName() : makeStaticString("null");

  return folly::format("func: {}, bcOff: {}, spOff: {}{}{}",
                       funcName->data(),
                       bcOff,
                       state.spOffset(),
                       state.thisAvailable() ? ", thisAvailable" : "",
                       state.frameSpansCall() ? ", frameSpansCall" : ""
                      ).str();
}

} }
