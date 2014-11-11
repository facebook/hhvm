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
#include "hphp/runtime/vm/jit/id-set.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

TRACE_SET_MOD(hhir);

namespace HPHP { namespace jit {

using Trace::Indent;

/*
 * Finds the least common ancestor of two SSATmps. A temp has a parent if it
 * is the result of a passthrough instruction.
 *
 * Returns nullptr when there is no LCA.
 */
static SSATmp* least_common_ancestor(SSATmp* s1, SSATmp* s2) {
  if (s1 == s2) return s1;
  if (s1 == nullptr || s2 == nullptr) return nullptr;

  IdSet<SSATmp> seen;

  auto const step = [] (SSATmp* v) {
    assert(v != nullptr);
    return v->inst()->isPassthrough() ?
      v->inst()->getPassthroughValue() :
      nullptr;
  };

  auto const process = [&] (SSATmp*& v) {
    if (v == nullptr) return false;
    if (seen[v]) return true;
    seen.add(v);
    v = step(v);
    return false;
  };

  while (s1 != nullptr || s2 != nullptr) {
    if (process(s1)) return s1;
    if (process(s2)) return s2;
  }

  return nullptr;
}

FrameState::FrameState(IRUnit& unit, BCMarker marker)
  : FrameState(unit, marker.spOff(), marker.func())
{
  assert(!marker.isDummy());
}

FrameState::FrameState(IRUnit& unit, Offset initialSpOffset, const Func* func)
  : m_unit(unit)
  , m_curFunc(func)
  , m_spOffset(initialSpOffset)
  , m_locals(func ? func->numLocals() : 0)
  , m_visited(unit.numBlocks())
{
}

void FrameState::update(const IRInstruction* inst) {
  ITRACE(3, "FrameState::update processing {}\n", *inst);
  Indent _i;

  if (auto* taken = inst->taken()) {
    // When we're building the IR, we append a conditional jump after
    // generating its target block: see emitJmpCondHelper, where we
    // call makeExit() before gen(JmpZero).  It doesn't make sense to
    // update the target block state at this point, so don't.  The
    // state doesn't have this problem during optimization passes,
    // because we'll always process the jump before the target block.
    if (!m_building || taken->empty()) save(taken);
  }

  auto const opc = inst->op();

  getLocalEffects(inst, *this);
  assert(checkInvariants());

  switch (opc) {
  case DefInlineFP:    trackDefInlineFP(inst);  break;
  case InlineReturn:   trackInlineReturn(); break;

  case Call:
    m_spValue = inst->dst();
    m_frameSpansCall = true;
    // A call pops the ActRec and the arguments, and then pushes a
    // return value.
    m_spOffset -= kNumActRecCells + inst->extra<Call>()->numParams;
    m_spOffset += 1;
    assert(m_spOffset >= 0);
    clearCse();
    break;

  case CallArray:
    m_spValue = inst->dst();
    m_frameSpansCall = true;
    // A CallArray pops the ActRec an array arg and pushes a return value.
    m_spOffset -= kNumActRecCells;
    assert(m_spOffset >= 0);
    clearCse();
    break;

  case ContEnter:
    m_spValue = inst->dst();
    m_frameSpansCall = true;
    clearCse();
    break;

  case DefFP:
  case FreeActRec:
    m_fpValue = inst->dst();
    break;

  case ReDefSP:
    m_spValue = inst->dst();
    m_spOffset = inst->extra<ReDefSP>()->spOffset;
    break;

  case DefSP:
    m_spValue = inst->dst();
    m_spOffset = inst->extra<StackOffset>()->offset;
    break;

  case AssertStk:
  case CastStk:
  case CastStkIntToDbl:
  case CoerceStk:
  case CheckStk:
  case GuardStk:
  case HintStkInner:
  case ExceptionBarrier:
    m_spValue = inst->dst();
    break;

  case SpillStack: {
    m_spValue = inst->dst();
    // Push the spilled values but adjust for the popped values
    int64_t stackAdjustment = inst->src(1)->intVal();
    m_spOffset -= stackAdjustment;
    m_spOffset += spillValueCells(inst);
    break;
  }

  case SpillFrame:
  case CufIterSpillFrame:
    m_spValue = inst->dst();
    m_spOffset += kNumActRecCells;
    break;

  case InterpOne:
  case InterpOneCF: {
    m_spValue = inst->dst();
    auto const& extra = *inst->extra<InterpOneData>();
    int64_t stackAdjustment = extra.cellsPopped - extra.cellsPushed;
    // push the return value if any and adjust for the popped values
    m_spOffset -= stackAdjustment;
    break;
  }

  case LdThis:
    m_thisAvailable = true;
    break;

  case DefLabel:
    if (inst->numDsts() > 0) {
      auto dst0 = inst->dst(0);
      if (dst0->isA(Type::StkPtr)) m_spValue = dst0;
    }
    break;

  default:
    break;
  }

  if (inst->modifiesStack()) {
    m_spValue = inst->modifiedStkPtr();
  }

  // update the CSE table
  if (m_enableCse && inst->canCSE()) {
    cseInsert(inst);
  }

  // if the instruction kills any of its sources, remove them from the
  // CSE table
  if (inst->killsSources()) {
    for (int i = 0; i < inst->numSrcs(); ++i) {
      if (inst->killsSource(i)) {
        cseKill(inst->src(i));
      }
    }
  }
}

static const StaticString s_php_errormsg("php_errormsg");

void FrameState::getLocalEffects(const IRInstruction* inst,
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
      hook.setLocalType(id, type);
      hook.setLocalTypeSource(id, TypeSource::makeValue(inst->dst()));
      break;
    }
    case StLocPseudoMain: {
      auto const type = inst->src(1)->type().relaxToGuardable();
      auto id = inst->extra<StLocPseudoMain>()->locId;
      hook.setLocalType(id, type);
      hook.setLocalTypeSource(id, TypeSource::makeValue(inst->src(1)));
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

  // If this instruction may raise an error and our function has a local named
  // "php_errormsg", we have to clobber it. See
  // http://www.php.net/manual/en/reserved.variables.phperrormsg.php
  if (inst->mayRaiseError()) {
    auto id = m_curFunc->lookupVarId(s_php_errormsg.get());
    if (id != -1) hook.setLocalValue(id, nullptr);
  }

  if (MInstrEffects::supported(inst)) MInstrEffects::get(inst, *this, hook);
}

///// Support helpers for getLocalEffects /////
void FrameState::clearLocals(LocalStateHook& hook) const {
  for (unsigned i = 0; i < m_locals.size(); ++i) {
    hook.setLocalValue(i, nullptr);
  }
}

void FrameState::refineLocalValues(LocalStateHook& hook,
                                   SSATmp* oldVal, SSATmp* newVal) const {
  assert(newVal->inst()->is(CheckType, AssertType));
  assert(newVal->inst()->src(0) == oldVal);

  walkAllInlinedLocals(
  [&](uint32_t i, unsigned inlineIdx, const LocalState& local) {
    if (canonical(local.value) == canonical(oldVal)) {
      hook.refineLocalValue(i, inlineIdx, oldVal, newVal);
    }
  });
}

void FrameState::forEachFrame(FrameFunc body) const {
  body(m_fpValue, m_spOffset);

  // We push each new frame onto the end of m_inlineSavedStates, so walk it
  // backwards to go from inner frames to outer frames.
  for (auto it = m_inlineSavedStates.rbegin();
       it != m_inlineSavedStates.rend(); ++it) {
    auto const& state = *it;
    body(state.fpValue, state.spOffset);
  }
}

template<typename L>
void FrameState::walkAllInlinedLocals(L body, bool skipThisFrame) const {
  auto doBody = [&](const LocalVec& locals, unsigned inlineIdx) {
    for (uint32_t i = 0, n = locals.size(); i < n; ++i) {
      body(i, inlineIdx, locals[i]);
    }
  };

  if (!skipThisFrame) {
    doBody(m_locals, 0);
  }
  for (int i = 0, n = m_inlineSavedStates.size(); i < n; ++i) {
    doBody(m_inlineSavedStates[i].locals, i + 1);
  }
}

void FrameState::forEachLocal(LocalFunc body) const {
  walkAllInlinedLocals(
  [&](uint32_t i, unsigned inlineIdx, const LocalState& local) {
    body(i, local.value);
  });
}

/**
 * Called to clear out the tracked local values at a call site.  Calls kill all
 * registers, so we don't want to keep locals in registers across calls. We do
 * continue tracking the types in locals, however.
 */
void FrameState::killLocalsForCall(LocalStateHook& hook,
                                   bool skipThisFrame) const {
  walkAllInlinedLocals(
  [&](uint32_t i, unsigned inlineIdx, const LocalState& local) {
    auto* value = local.value;
    if (!value || value->inst()->is(DefConst)) return;

    hook.killLocalForCall(i, inlineIdx, value);
  },
  skipThisFrame);
}

/*
 * This is called when we store into a BoxedCell.  Any locals that we know
 * point to that cell can have their inner type predictions updated.
 */
void FrameState::updateLocalRefPredictions(LocalStateHook& hook,
                                           SSATmp* boxedCell,
                                           SSATmp* val) const {
  assert(boxedCell->type().isBoxed());
  for (auto id = uint32_t{0}; id < m_locals.size(); ++id) {
    if (canonical(m_locals[id].value) == canonical(boxedCell)) {
      hook.setBoxedLocalPrediction(id, boxType(val->type()));
    }
  }
}

/**
 * This method changes any boxed local into a BoxedInitCell type. It's safe to
 * assume they're init because you can never have a reference to uninit.
 */
void FrameState::dropLocalRefsInnerTypes(LocalStateHook& hook) const {
  walkAllInlinedLocals(
    [&](uint32_t i, unsigned inlineIdx, const LocalState& local) {
      if (local.type.isBoxed()) {
        hook.dropLocalInnerType(i, inlineIdx);
      }
    }
  );
}

///// Methods for managing and merge block state /////

bool FrameState::hasStateFor(Block* block) const {
  return m_states.count(block);
}

Block* FrameState::findUnprocessedPred(Block* block) const {
  for (auto const& edge : block->preds()) {
    auto const pred = edge.from();
    if (!isVisited(pred)) return pred;
  }
  return nullptr;
}

const FrameState::LocalVec& FrameState::localsLeavingBlock(Block* block) const {
  auto const it = m_states.find(block);
  assert(it != m_states.end());
  return it->second.out.locals;
}

SSATmp* FrameState::spLeavingBlock(Block* b) const {
  auto it = m_states.find(b);
  assert(it != m_states.end());
  return it->second.out.spValue;
}

void FrameState::startBlock(Block* block,
                            BCMarker marker,
                            LocalStateHook* hook /* = nullptr */,
                            bool isLoopHeader /* = false */) {
  auto const it = m_states.find(block);
  auto const end = m_states.end();

  DEBUG_ONLY auto const predsAllowed =
    it != end || block->isEntry() || RuntimeOption::EvalJitLoops;
  assert(IMPLIES(block->numPreds() > 0, predsAllowed));

  if (it != end) {
    load(it->second.in);
    ITRACE(4, "Loading state for B{}: {}\n", block->id(), show(*this));
    m_inlineSavedStates = it->second.in.inlineSavedStates;
  }

  // Reset state if the block is a loop header.
  if (isLoopHeader || findUnprocessedPred(block)) {
    Indent _;
    ITRACE(4, "B{} has unprocessed predecessor, resetting state\n",
           block->id());
    loopHeaderClear(marker, hook);
  }

  markVisited(block);
}

void FrameState::finishBlock(Block* block) {
  assert(block->back().isTerminal() == !block->next() || m_building);

  m_states[block].out = createSnapshotWithInline();

  if (!block->back().isTerminal()) save(block->next());
}

void FrameState::pauseBlock(Block* block) {
  m_states[block].out = createSnapshotWithInline();
}

void FrameState::unpauseBlock(Block* block) {
  auto& snap = m_states[block].out;

  load(snap);
  m_inlineSavedStates = snap.inlineSavedStates;
}

FrameState::Snapshot FrameState::createSnapshot() const {
  Snapshot state;
  state.spValue = m_spValue;
  state.fpValue = m_fpValue;
  state.curFunc = m_curFunc;
  state.spOffset = m_spOffset;
  state.thisAvailable = m_thisAvailable;
  state.stackDeficit = m_stackDeficit;
  state.evalStack = m_evalStack;
  state.locals = m_locals;
  state.curMarker = m_marker;
  state.frameSpansCall = m_frameSpansCall;
  assert(state.curMarker.valid());
  return state;
}

FrameState::Snapshot FrameState::createSnapshotWithInline() const {
  auto snap = createSnapshot();
  snap.inlineSavedStates = m_inlineSavedStates;
  return snap;
}

/*
 * Save current state for block.  If this is the first time saving state for
 * block, create a new snapshot.  Otherwise merge the current state into the
 * existing snapshot.
 */
void FrameState::save(Block* block) {
  ITRACE(4, "Saving current state to B{}: {}\n", block->id(), show(*this));
  auto const it = m_states.find(block);
  if (it != m_states.end()) {
    merge(it->second.in);
    ITRACE(4, "Merged state: {}\n", show(*this));
  } else {
    m_states[block].in = createSnapshotWithInline();
  }
}

void FrameState::load(Snapshot& state) {
  m_spValue = state.spValue;
  m_fpValue = state.fpValue;
  m_spOffset = state.spOffset;
  m_curFunc = state.curFunc;
  m_thisAvailable = state.thisAvailable;
  m_stackDeficit = state.stackDeficit;
  m_evalStack = state.evalStack;
  m_locals = state.locals;
  m_marker = state.curMarker;
  m_frameSpansCall = m_frameSpansCall || state.frameSpansCall;
}

/*
 * Merge current state into state.  Frame pointers and stack depth must match.
 * If the stack pointer tmps are different, clear the tracked value (we can
 * make a new one, given fp and spOffset).
 *
 * thisIsAvailable remains true if it's true in both states.
 * local variable values are preserved if the match in both states.
 * types are combined using Type::unionOf.
 */
void FrameState::merge(Snapshot& state) {
  // cannot merge spOffset state, so assert they match
  assert(state.spOffset == m_spOffset);
  assert(state.curFunc == m_curFunc);
  if (state.spValue != m_spValue) {
    // we have two different sp definitions but we know they're equal
    // because spOffset matched.
    state.spValue = nullptr;
  }

  // The only thing that can change the FP is inlining, but we can't have one
  // of the predecessors in an inlined callee while the other isn't.
  always_assert(state.fpValue == m_fpValue);

  // this is available iff it's available in both states
  state.thisAvailable &= m_thisAvailable;

  assert(m_locals.size() == state.locals.size());
  for (unsigned i = 0; i < m_locals.size(); ++i) {
    auto& local = state.locals[i];

    // Get the least common ancestor across both states.
    local.value = least_common_ancestor(local.value, m_locals[i].value);

    // merge the typeSrcs
    for (auto newTypeSrc : m_locals[i].typeSrcs) {
      local.typeSrcs.insert(newTypeSrc);
    }

    local.type = Type::unionOf(local.type, m_locals[i].type);
    local.boxedPrediction =
      Type::unionOf(local.boxedPrediction, m_locals[i].boxedPrediction);

    // Throw away the prediction if we merged Type::Gen for the type.
    if (!(local.type <= Type::BoxedInitCell)) {
      local.boxedPrediction = Type::Bottom;
    }
  }

  // For now, we shouldn't be merging states with different inline states.
  assert(m_inlineSavedStates == state.inlineSavedStates);
}

void FrameState::trackDefInlineFP(const IRInstruction* inst) {
  auto const target     = inst->extra<DefInlineFP>()->target;
  auto const savedSPOff = inst->extra<DefInlineFP>()->retSPOff;
  auto const calleeFP   = inst->dst();
  auto const calleeSP   = inst->src(0);
  auto const savedSP    = inst->src(1);

  // Saved IRBuilder state will include the "return" fp/sp.
  // Whatever the current fpValue is is good enough, but we have to be
  // passed in the StkPtr that represents the stack prior to the
  // ActRec being allocated.
  m_spOffset = savedSPOff;
  m_spValue = savedSP;

  auto const stackValues = collectStackValues(m_spValue, m_spOffset);
  for (DEBUG_ONLY auto& val : stackValues) {
    ITRACE(4, "    marking caller stack value available: {}\n",
           val->toString());
  }

  m_inlineSavedStates.emplace_back(createSnapshot());

  /*
   * Set up the callee state.
   *
   * We set m_thisIsAvailable to true on any object method, because we
   * just don't inline calls to object methods with a null $this.
   */
  m_fpValue         = calleeFP;
  m_spValue         = calleeSP;
  m_thisAvailable   = target->cls() != nullptr && !target->isStatic();
  m_curFunc         = target;
  m_frameSpansCall  = false;

  m_locals.clear();
  m_locals.resize(target->numLocals());
}

void FrameState::trackInlineReturn() {
  assert(m_inlineSavedStates.size());
  assert(m_inlineSavedStates.back().inlineSavedStates.empty());
  load(m_inlineSavedStates.back());
  m_inlineSavedStates.pop_back();
}

CSEHash* FrameState::cseHashTable(const IRInstruction* inst) {
  return inst->is(DefConst) ? &m_unit.constTable() : &m_cseHash;
}

void FrameState::cseInsert(const IRInstruction* inst) {
  cseHashTable(inst)->insert(inst->dst());
}

void FrameState::cseKill(SSATmp* src) {
  if (src->inst()->canCSE()) {
    cseHashTable(src->inst())->erase(src);
  }
}

void FrameState::clearCse() {
  m_cseHash.clear();
}

SSATmp* FrameState::cseLookup(IRInstruction* inst,
                              Block* srcBlock,
                              const folly::Optional<IdomVector>& idoms) {
  auto tmp = cseHashTable(inst)->lookup(inst);
  if (tmp && idoms) {
    // During a reoptimize pass, we need to make sure that any values
    // we want to reuse for CSE are only reused in blocks dominated by
    // the block that defines it.
    if (!dominates(tmp->inst()->block(), srcBlock, *idoms)) {
      return nullptr;
    }
  }
  return tmp;
}

void FrameState::clear() {
  // A previous run of reoptimize could've legitimately exited the trace in an
  // inlined callee. If that happened, just pop all the saved states to return
  // to the top-level func.
  while (inlineDepth()) {
    trackInlineReturn();
  }
  clearCurrentState();
  clearCse();
  m_states.clear();
  m_visited.clear();
  assert(m_inlineSavedStates.empty());
}

bool FrameState::checkInvariants() const {
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

void FrameState::clearCurrentState() {
  m_spValue        = nullptr;
  m_fpValue        = nullptr;
  m_spOffset       = 0;
  m_marker         = BCMarker();
  m_thisAvailable  = false;
  m_frameSpansCall = false;
  m_stackDeficit   = 0;
  m_evalStack      = EvalStack();
  clearLocals(*this);
}

void FrameState::loopHeaderClear(BCMarker marker,
                                 LocalStateHook* hook /* = nullptr */) {
  m_spValue        = nullptr;
  m_marker         = marker;
  m_spOffset       = marker.spOff();
  m_curFunc        = marker.func();
  m_stackDeficit   = 0;
  m_evalStack      = EvalStack();

  // Clear hook first so that it can read local info from the FrameState.
  if (hook != nullptr) clearLocals(*hook);
  clearLocals(*this);
}

void FrameState::resetCurrentState(BCMarker marker) {
  clearCurrentState();
  m_marker   = marker;
  m_spOffset = marker.spOff();
  m_curFunc  = marker.func();
}

SSATmp* FrameState::localValue(uint32_t id) const {
  always_assert(id < m_locals.size());
  return m_locals[id].value;
}

TypeSourceSet FrameState::localTypeSources(uint32_t id) const {
  always_assert(id < m_locals.size());
  return m_locals[id].typeSrcs;
}

Type FrameState::localType(uint32_t id) const {
  always_assert(id < m_locals.size());
  return m_locals[id].type;
}

Type FrameState::predictedInnerType(uint32_t id) const {
  always_assert(id < m_locals.size());
  always_assert(m_locals[id].type.isBoxed());
  if (m_locals[id].boxedPrediction <= Type::Bottom) {
    ITRACE(2, "No prediction: {}\n", id);
    return Type::InitCell;
  }
  auto t = ldRefReturn(m_locals[id].boxedPrediction.unbox());
  ITRACE(2, "Predicting{}: {}\n", id, t);
  return t;
}

void FrameState::setLocalValue(uint32_t id, SSATmp* value) {
  always_assert(id < m_locals.size());
  m_locals[id].value = value;
  m_locals[id].type = value ? value->type() : Type::Gen;

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
        return m_locals[inst->extra<LdLoc>()->locId].boxedPrediction;
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
  m_locals[id].boxedPrediction = newInnerPred;

  m_locals[id].typeSrcs.clear();
  if (value) m_locals[id].typeSrcs.insert(TypeSource::makeValue(value));
}

void FrameState::refineLocalType(uint32_t id, Type type, TypeSource typeSrc) {
  always_assert(id < m_locals.size());
  auto& local = m_locals[id];
  Type newType = refineTypeNoCheck(local.type, type);
  ITRACE(2, "updating local {}'s type: {} -> {}\n", id, local.type, newType);
  local.type = newType;
  local.typeSrcs.clear();
  local.typeSrcs.insert(typeSrc);
}

void FrameState::setLocalType(uint32_t id, Type type) {
  always_assert(id < m_locals.size());
  m_locals[id].value = nullptr;
  m_locals[id].type = type;
  m_locals[id].boxedPrediction = Type::Bottom;
  m_locals[id].typeSrcs.clear();
}

void FrameState::setBoxedLocalPrediction(uint32_t id, Type type) {
  always_assert(id < m_locals.size());
  always_assert(type <= Type::BoxedCell);
  always_assert_flog(
    m_locals[id].type == Type::BoxedInitCell ||
    m_locals[id].type == Type::Gen,
    "PredictLocInner {} with base type {}",
    id,
    m_locals[id].type
  );
  if (m_locals[id].type <= Type::BoxedCell) {
    m_locals[id].boxedPrediction = type;
  } else {
    m_locals[id].boxedPrediction = Type::Bottom;
  }
}

void FrameState::setLocalTypeSource(uint32_t id, TypeSource typeSrc) {
  always_assert(id < m_locals.size());
  m_locals[id].typeSrcs.clear();
  m_locals[id].typeSrcs.insert(typeSrc);
}

/*
 * Get a reference to the LocalVec from an inline index. 0 means the current
 * frame, otherwise it's index (inlineIdx - 1) in m_inlineSavedStates.
 */
FrameState::LocalVec& FrameState::locals(unsigned inlineIdx) {
  if (inlineIdx == 0) {
    return m_locals;
  } else {
    --inlineIdx;
    assert(inlineIdx < m_inlineSavedStates.size());
    return m_inlineSavedStates[inlineIdx].locals;
  }
}

void FrameState::refineLocalValue(uint32_t id, unsigned inlineIdx,
                                  SSATmp* oldVal, SSATmp* newVal) {
  auto& locs = locals(inlineIdx);
  always_assert(id < locs.size());
  auto& local = locs[id];
  local.value = newVal;
  local.type = newVal->type();
  local.typeSrcs.clear();
  local.typeSrcs.insert(TypeSource::makeValue(newVal));
}

void FrameState::killLocalForCall(uint32_t id, unsigned inlineIdx,
                                  SSATmp* val) {
  auto& locs = locals(inlineIdx);
  always_assert(id < locs.size());
  locs[id].value = nullptr;
}

void FrameState::dropLocalInnerType(uint32_t id, unsigned inlineIdx) {
  auto& local = locals(inlineIdx)[id];
  assert(local.type.isBoxed());
  local.boxedPrediction = local.type = Type::BoxedInitCell;
}

void FrameState::markVisited(const Block* b) {
  // The number of blocks in the unit can change over time.
  if (b->id() >= m_visited.size()) {
    m_visited.resize(b->id() + 1);
  }

  m_visited.set(b->id());
}

bool FrameState::isVisited(const Block* b) const {
  return b->id() < m_visited.size() && m_visited.test(b->id());
}

std::string show(const FrameState& state) {
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
