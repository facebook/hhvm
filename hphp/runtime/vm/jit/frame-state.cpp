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
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

TRACE_SET_MOD(hhir);

namespace HPHP {
namespace JIT {

FrameState::FrameState(IRUnit& unit)
  : FrameState(unit, unit.entry()->front().marker())
{
}

FrameState::FrameState(IRUnit& unit, BCMarker marker)
  : FrameState(unit, marker.spOff, marker.func)
{
}

FrameState::FrameState(IRUnit& unit, Offset initialSpOffset, const Func* func)
  : m_unit(unit)
  , m_curFunc(func)
  , m_spValue(nullptr)
  , m_fpValue(nullptr)
  , m_spOffset(initialSpOffset)
  , m_thisAvailable(false)
  , m_frameSpansCall(false)
  , m_stackDeficit(0)
  , m_evalStack()
  , m_locals(func->numLocals())
  , m_enableCse(false)
  , m_snapshots()
{
}

FrameState::~FrameState() {
}

void FrameState::update(const IRInstruction* inst) {
  FTRACE(3, "FrameState::update processing {}\n", *inst);

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

  switch (opc) {
  case DefInlineFP:    trackDefInlineFP(inst);  break;
  case InlineReturn:   trackInlineReturn(inst); break;

  case Call:
    m_spValue = inst->dst();
    m_frameSpansCall = true;
    // A call pops the ActRec and pushes a return value.
    m_spOffset -= kNumActRecCells;
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
    clearCse();
    break;

  case DefFP:
  case FreeActRec:
    m_fpValue = inst->dst();
    break;

  case ReDefGeneratorSP:
    m_spValue = inst->dst();
    break;

  case ReDefSP:
    m_spValue = inst->dst();
    m_spOffset = inst->extra<ReDefSP>()->spOffset;
    break;

  case DefInlineSP:
  case DefSP:
    m_spValue = inst->dst();
    m_spOffset = inst->extra<StackOffset>()->offset;
    break;

  case AssertStk:
  case CastStk:
  case CoerceStk:
  case CheckStk:
  case GuardStk:
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

  case AssertLoc:
  case GuardLoc:
  case CheckLoc:
    m_fpValue = inst->dst();
    break;

  case LdThis:
    m_thisAvailable = true;
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

void FrameState::getLocalEffects(const IRInstruction* inst,
                                 LocalStateHook& hook) const {
  auto killIterLocals = [&](const std::initializer_list<uint32_t>& ids) {
    for (auto id : ids) {
      hook.setLocalValue(id, nullptr);
    }
  };

  auto killedCallLocals = false;
  if ((inst->is(CallArray) && inst->extra<CallArrayData>()->destroyLocals) ||
      (inst->is(Call, CallBuiltin) && inst->extra<CallData>()->destroyLocals)) {
    clearLocals(hook);
    killedCallLocals = true;
  }

  switch (inst->op()) {
    case Call:
    case CallArray:
    case ContEnter:
      killLocalsForCall(hook, killedCallLocals);
      break;

    case StRef: {
      SSATmp* newRef = inst->dst();
      SSATmp* prevRef = inst->src(0);
      // update other tracked locals that also contain prevRef
      updateLocalRefValues(hook, prevRef, newRef);
      break;
    }

    case StLocNT:
    case StLoc:
      hook.setLocalValue(inst->extra<LocalId>()->locId, inst->src(1));
      break;

    case LdLoc:
      hook.setLocalValue(inst->extra<LdLoc>()->locId, inst->dst());
      break;

    case AssertLoc:
    case GuardLoc:
    case CheckLoc:
      hook.refineLocalType(inst->extra<LocalId>()->locId, inst->typeParam(),
                           inst->dst());
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

  if (MInstrEffects::supported(inst)) MInstrEffects::get(inst, hook);
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
    if (local.value == oldVal) {
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

//
// This method updates the tracked values and types of all locals that contain
// oldRef so that they now contain newRef.
// This should only be called for ref/boxed types.
//
void FrameState::updateLocalRefValues(LocalStateHook& hook,
                                      SSATmp* oldRef, SSATmp* newRef) const {
  assert(oldRef->type().isBoxed());
  assert(newRef->type().isBoxed());

  walkAllInlinedLocals(
  [&](uint32_t i, unsigned inlineIdx, const LocalState& local) {
    if (local.value != oldRef) return;

    hook.updateLocalRefValue(i, inlineIdx, oldRef, newRef);
  });
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
  });
}

///// Methods for managing and merge block state /////
void FrameState::startBlock(Block* block) {
  auto it = m_snapshots.find(block);
  if (it != m_snapshots.end()) {
    load(it->second);
    FTRACE(4, "Loading state for B{}: {}\n", block->id(), show(*this));
    m_inlineSavedStates = it->second.inlineSavedStates;
    m_snapshots.erase(it);
  }
}

void FrameState::finishBlock(Block* block) {
  assert(block->back().isTerminal() == !block->next());

  if (!block->back().isTerminal()) {
    save(block->next());
  }
}

void FrameState::pauseBlock(Block* block) {
  save(block);
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

/*
 * Save current state for block.  If this is the first time saving state for
 * block, create a new snapshot.  Otherwise merge the current state into the
 * existing snapshot.
 */
void FrameState::save(Block* block) {
  FTRACE(4, "Saving state for B{}: {}\n", block->id(), show(*this));
  auto it = m_snapshots.find(block);
  if (it != m_snapshots.end()) {
    merge(it->second);
    FTRACE(4, "Merged state: {}\n", show(*this));
  } else {
    auto& snapshot = m_snapshots[block] = createSnapshot();
    snapshot.inlineSavedStates = m_inlineSavedStates;
  }
}

bool FrameState::compatible(Block* block) {
  auto it = m_snapshots.find(block);
  // If we didn't find a snapshot, it's because we never saved one.
  // Probably because the other incoming edge is unreachable.
  if (it == m_snapshots.end()) return true;
  auto& snapshot = it->second;
  if (m_fpValue != snapshot.fpValue) return false;

  assert(m_locals.size() == snapshot.locals.size());
  for (int i = 0; i < m_locals.size(); ++i) {
    // Enforce strict equality of types for now.  Eventually we could
    // relax this depending on downstream operations.
    //
    // TODO(t3729135): We don't bother to check values here because we
    // clear the CSE table at any merge.  Eventually we will support
    // phis instead.
    if (m_locals[i].type != snapshot.locals[i].type) {
      return false;
    }
  }

  // TODO(t3730468): We don't check the stack here, because we always
  // spill the stack on all paths leading up to a merge, and insert a
  // DefSP at the merge point to block walking the use-def chain past
  // it.  It would be better to do proper type analysis on the stack
  // values flowing in and insert phis or exits as needed.

  return true;
}

void FrameState::load(Snapshot& state) {
  m_spValue = state.spValue;
  m_fpValue = state.fpValue;
  m_spOffset = state.spOffset;
  m_curFunc = state.curFunc;
  m_thisAvailable = state.thisAvailable;
  m_stackDeficit = state.stackDeficit;
  m_evalStack = std::move(state.evalStack);
  m_locals = std::move(state.locals);
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
  // cannot merge fp or spOffset state, so assert they match
  assert(state.fpValue == m_fpValue);
  assert(state.spOffset == m_spOffset);
  assert(state.curFunc == m_curFunc);
  if (state.spValue != m_spValue) {
    // we have two different sp definitions but we know they're equal
    // because spOffset matched.
    state.spValue = nullptr;
  }
  // this is available iff it's available in both states
  state.thisAvailable &= m_thisAvailable;

  assert(m_locals.size() == state.locals.size());
  for (unsigned i = 0; i < m_locals.size(); ++i) {
    auto& local = state.locals[i];

    // preserve local values if they're the same in both states,
    // This would be the place to insert phi nodes (jmps+deflabels) if we want
    // to avoid clearing state, which triggers a downstream reload.
    if (local.value != m_locals[i].value) local.value = nullptr;
    if (local.typeSource != m_locals[i].typeSource) local.typeSource = nullptr;

    local.type = Type::unionOf(local.type, m_locals[i].type);
  }

  // TODO(t3729135): If we are merging states from different bytecode
  // paths, we must conservatively clear the CSE table.  Since the
  // markers may or may not have been updated, we always clear.  What
  // we need is a global CSE algorithm.
  if (RuntimeOption::EvalHHIRBytecodeControlFlow) {
    clearCse();
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
    FTRACE(4, "    marking caller stack value available: {}\n",
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

void FrameState::trackInlineReturn(const IRInstruction* inst) {
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
                                const folly::Optional<IdomVector>& idoms) {
  auto tmp = cseHashTable(inst)->lookup(inst);
  if (tmp && idoms) {
    // During a reoptimize pass, we need to make sure that any values
    // we want to reuse for CSE are only reused in blocks dominated by
    // the block that defines it.
    if (!dominates(tmp->inst()->block(), inst->block(), *idoms)) {
      return nullptr;
    }
  }
  return tmp;
}

void FrameState::clear() {
  clearCse();
  clearLocals(*this);
  m_frameSpansCall = false;
  m_spValue = m_fpValue = nullptr;
  m_spOffset = 0;
  m_thisAvailable = false;
  m_marker = BCMarker();
  m_snapshots.clear();
  assert(m_inlineSavedStates.empty());
}

SSATmp* FrameState::localValue(uint32_t id) const {
  always_assert(id < m_locals.size());
  return m_locals[id].value;
}

SSATmp* FrameState::localTypeSource(uint32_t id) const {
  always_assert(id < m_locals.size());
  auto const& local = m_locals[id];

  always_assert(!local.value || local.value == local.typeSource ||
                local.typeSource->isA(Type::FramePtr));
  return local.typeSource;
}

Type FrameState::localType(uint32_t id) const {
  always_assert(id < m_locals.size());
  return m_locals[id].type;
}

void FrameState::setLocalValue(uint32_t id, SSATmp* value) {
  always_assert(id < m_locals.size());
  m_locals[id].value = value;
  m_locals[id].type = value ? value->type() : Type::Gen;
  m_locals[id].typeSource = value;
}

void FrameState::refineLocalType(uint32_t id, Type type, SSATmp* typeSource) {
  always_assert(id < m_locals.size());
  auto& local = m_locals[id];
  if (type.isBoxed() && local.type.isBoxed()) {
    // It's OK for the old and new inner types of boxed values not to
    // intersect, since the inner type is really just a prediction.
    FTRACE(2, "updating local {}'s inner type: {} -> {}\n",
           id, local.type, type);
    local.type = type;
  } else {
    auto const result = local.type & type;
    always_assert_log(
      result != Type::Bottom,
      [&] {
        return folly::format("Bad new type for local {}: {} & {} = {}",
                             id, local.type, type, result).str();
      });
    local.type = local.type & type;
  }
  local.typeSource = typeSource;
}

void FrameState::setLocalType(uint32_t id, Type type) {
  always_assert(id < m_locals.size());
  m_locals[id].value = nullptr;
  m_locals[id].type = type;
  m_locals[id].typeSource = nullptr;
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
  local.typeSource = newVal;
}

void FrameState::killLocalForCall(uint32_t id, unsigned inlineIdx,
                                  SSATmp* val) {
  auto& locs = locals(inlineIdx);
  always_assert(id < locs.size());
  locs[id].value = nullptr;
}

void FrameState::updateLocalRefValue(uint32_t id, unsigned inlineIdx,
                                     SSATmp* oldRef, SSATmp* newRef) {
  auto& local = locals(inlineIdx)[id];
  assert(local.value == oldRef);
  local.value = newRef;
  local.type  = newRef->type();
  local.typeSource = newRef;
}

void FrameState::dropLocalInnerType(uint32_t id, unsigned inlineIdx) {
  auto& local = locals(inlineIdx)[id];
  assert(local.type.isBoxed());
  local.type = Type::BoxedInitCell;
}

std::string show(const FrameState& state) {
  return folly::format("func: {}, bcOff: {}, spOff: {}{}{}",
                       state.func()->fullName()->data(),
                       state.marker().bcOff,
                       state.spOffset(),
                       state.thisAvailable() ? ", thisAvailable" : "",
                       state.frameSpansCall() ? ", frameSpansCall" : ""
                      ).str();
}

} }
