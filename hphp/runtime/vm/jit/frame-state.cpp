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

  if (merge_util(dst.type, Type::unionOf(dst.type, src.type))) {
    changed = true;
  }

  return
    merge_util(dst.predictedType,
               Type::unionOf(dst.predictedType, src.predictedType)) ||
    changed;
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

    if (state.curFunc->isPseudoMain()) {
      always_assert_flog(
        local.value == nullptr,
        "We should never be tracking values for locals in a pseudomain "
          "right now.  Local {} had value {}",
        id,
        local.value->toString()
      );
      always_assert_flog(
        local.type == Type::Gen,
        "We should never be tracking non-predicted types for locals in "
          "a pseudomain right now.  Local {} had type {}",
        id,
        local.type.toString()
      );
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

}

FrameStateMgr::FrameStateMgr(const IRUnit& unit, BCMarker marker) {
  m_stack.push_back(FrameState{});
  cur().curFunc = marker.func();
  cur().spOffset = marker.spOff();
  cur().locals.resize(marker.func()->numLocals());
}

bool FrameStateMgr::update(const IRInstruction* inst) {
  assert(m_status != Status::None);
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

  local_effects(*this, inst, *this);
  assert(checkInvariants());

  switch (inst->op()) {
  case DefInlineFP:    trackDefInlineFP(inst);  break;
  case InlineReturn:   trackInlineReturn(); break;

  case Call:
    cur().spValue = inst->dst();
    for (auto& st : m_stack) st.frameMaySpanCall = true;
    // A call pops the ActRec and the arguments, and then pushes a
    // return value.
    cur().spOffset -= kNumActRecCells + inst->extra<Call>()->numParams;
    cur().spOffset += 1;
    assert(cur().spOffset >= 0);
    break;

  case CallArray:
    cur().spValue = inst->dst();
    for (auto& st : m_stack) st.frameMaySpanCall = true;
    // A CallArray pops the ActRec an array arg and pushes a return value.
    cur().spOffset -= kNumActRecCells;
    assert(cur().spOffset >= 0);
    break;

  case ContEnter:
    cur().spValue = inst->dst();
    for (auto& st : m_stack) st.frameMaySpanCall = true;
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

  case EndCatch:
    /*
     * Hitting this means we've messed up with syncing the stack in a catch
     * trace.  If the stack isn't clean or doesn't match the marker's spOffset,
     * the unwinder won't see what we expect.
     */
    always_assert_flog(
      cur().spOffset == inst->marker().spOff() &&
      cur().stackDeficit == 0 &&
      cur().evalStack.size() == 0,
      "EndCatch stack didn't seem right:\n"
      "                 spOff: {}\n"
      "        marker's spOff: {}\n"
      "  eval stack def, size: {}, {}\n",
      cur().spOffset,
      inst->marker().spOff(),
      cur().stackDeficit,
      cur().evalStack.size()
    );
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

  default:
    break;
  }

  if (inst->modifiesStack()) {
    cur().spValue = inst->modifiedStkPtr();
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

  assert(!m_stack.empty());
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

///// Methods for managing and merge block state /////

bool FrameStateMgr::hasStateFor(Block* block) const {
  return m_states.count(block);
}

void FrameStateMgr::startBlock(Block* block,
                               BCMarker marker,
                               bool isLoopHeader /* = false */) {
  ITRACE(3, "FrameStateMgr::startBlock: {}\n", block->id());
  assert(m_status != Status::None);
  auto const it = m_states.find(block);
  auto const end = m_states.end();

  DEBUG_ONLY auto const predsAllowed =
    it != end || block->isEntry() || RuntimeOption::EvalJitLoops;
  assert(IMPLIES(block->numPreds() > 0, predsAllowed));

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
  assert(!m_stack.empty());

  // Don't reset state for unprocessed predecessors if we're trying to reach a
  // fixed-point.
  if (m_status == Status::RunningFixedPoint) return;

  // Reset state if the block is the target of a back edge.
  if (isLoopHeader) {
    Indent _;
    ITRACE(4, "B{} is a loop header; resetting state\n", block->id());
    loopHeaderClear(marker);
  }
}

bool FrameStateMgr::finishBlock(Block* block) {
  assert(block->back().isTerminal() == !block->next());
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
  assert(hasStateFor(block));
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
  cur().fpValue          = calleeFP;
  cur().spValue          = calleeSP;
  cur().thisAvailable    = target->cls() != nullptr && !target->isStatic();
  cur().curFunc          = target;
  cur().frameMaySpanCall = false;

  cur().locals.clear();
  cur().locals.resize(target->numLocals());
}

void FrameStateMgr::trackInlineReturn() {
  m_stack.pop_back();
  assert(!m_stack.empty());
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
 * We do not support unprocessed predecessors from different frames.  fpValue
 * must be the same, so it is not cleared.
 */
void FrameStateMgr::loopHeaderClear(BCMarker marker) {
  /*
   * Important note: we're setting our tracked spOffset to the marker spOffset.
   * These two spOffsets have different meanings.  One is the offset for the
   * last StkPtr, and one is the logical offset for the hhbc stack machine.
   *
   * For loops, since we're only supporting fully-sync'd stacks this is ok for
   * now.  The distinction between the kinds of spOffsets will go away after we
   * stop threading stack pointers.
   */
  cur().spOffset       = marker.spOff();
  cur().spValue        = nullptr;
  cur().curFunc        = marker.func();
  cur().stackDeficit   = 0;
  cur().evalStack      = EvalStack();

  // These two values must go toward their conservative state.
  cur().thisAvailable    = false;
  cur().frameMaySpanCall = true;

  clearLocals();
}

void FrameStateMgr::computeFixedPoint(const BlocksWithIds& blocks) {
  ITRACE(4, "FrameStateMgr computing fixed-point\n");

  assert(m_status == Status::None);  // we should have a clear state
  m_status = Status::RunningFixedPoint;

  auto const entry = blocks.blocks[0];
  DEBUG_ONLY auto const entryMarker = entry->front().marker();
  // So that we can actually call startBlock on the entry block.
  assert(m_stack.size() == 1);
  m_states[entry].in = m_stack;
  assert(m_states[entry].in.back().curFunc == entryMarker.func());

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
      if (update(&inst)) insert(block->taken());
    }

    if (finishBlock(block)) insert(block->next());
  }

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

Type FrameStateMgr::predictedLocalType(uint32_t id) const {
  always_assert(id < cur().locals.size());
  auto const ty = cur().locals[id].predictedType;
  ITRACE(2, "Predicting {}: {}\n", id, ty);
  return ty;
}

void FrameStateMgr::clearLocals() {
  ITRACE(2, "clearLocals\n");
  for (auto i = uint32_t{0}; i < cur().locals.size(); ++i) {
    setLocalValue(i, nullptr);
  }
}

void FrameStateMgr::setLocalValue(uint32_t id, SSATmp* value) {
  always_assert(id < cur().locals.size());
  cur().locals[id].value = value;
  auto const newType = value ? value->type() : Type::Gen;
  cur().locals[id].type = newType;

  /*
   * Update the predicted type for boxed values in some special cases to
   * something smart.  The rest of the time, throw it away.
   */
  auto const newInnerPred = [&]() -> Type {
    if (value) {
      auto const inst = value->inst();
      switch (inst->op()) {
      case LdLoc:
        if (value->type().isBoxed()) {
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
  auto const newType = refineTypeNoCheck(local.type, type);
  ITRACE(2, "updating local {}'s type: {} -> {}\n", id, local.type, newType);
  local.type = newType;
  local.predictedType = newType;
  local.typeSrcs.clear();
  local.typeSrcs.insert(typeSrc);
}

void FrameStateMgr::predictLocalType(uint32_t id, Type type) {
  always_assert(id < cur().locals.size());
  auto& local = cur().locals[id];
  ITRACE(2, "updating local {}'s type prediction: {} -> {}\n",
    id, local.predictedType, type & local.type);
  local.predictedType = type & local.type;
  if (!(local.predictedType <= local.type)) {
    local.predictedType = local.type;
  }
}

void FrameStateMgr::setLocalType(uint32_t id, Type type) {
  always_assert(id < cur().locals.size());
  cur().locals[id].value = nullptr;
  cur().locals[id].type = type;
  cur().locals[id].predictedType = type;
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
    cur().locals[id].predictedType = type;
  } else {
    cur().locals[id].predictedType = cur().locals[id].type;
  }
}

/*
 * This is called when we store into a BoxedCell.  Any locals that we know
 * point to that cell can have their inner type predictions updated.
 */
void FrameStateMgr::updateLocalRefPredictions(SSATmp* boxedCell, SSATmp* val) {
  assert(boxedCell->type().isBoxed());
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
  assert(inlineIdx < m_stack.size());
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
      local.value = newVal;
      local.type = newVal->type();
      local.predictedType = local.type;
      local.typeSrcs.clear();
      local.typeSrcs.insert(TypeSource::makeValue(newVal));
    }
  }
}

/*
 * Called to clear out the tracked local values at a call site.  Calls kill all
 * registers, so we don't want to keep locals in registers across calls. We do
 * continue tracking the types in locals, however.
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
      if (local.type.isBoxed()) {
        local.type          = Type::BoxedInitCell;
        local.predictedType = Type::BoxedInitCell;
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
    funcName->data(),
    state.spOffset(),
    state.thisAvailable() ? ", thisAvailable" : "",
    state.frameMaySpanCall() ? ", frameMaySpanCall" : ""
  ).str();
}

//////////////////////////////////////////////////////////////////////

}}
