/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

TRACE_SET_MOD(hhir);

namespace HPHP {
namespace JIT {

FrameState::FrameState(IRUnit& unit, Offset initialSpOffset, const Func* func)
  : m_unit(unit)
  , m_curFunc(func)
  , m_spValue(nullptr)
  , m_fpValue(nullptr)
  , m_spOffset(initialSpOffset)
  , m_thisAvailable(false)
  , m_frameSpansCall(false)
  , m_hasFPAnchor(false)
  , m_locals(func->numLocals())
  , m_enableCse(false)
  , m_snapshots(unit, nullptr)
{
}

FrameState::~FrameState() {
  for (auto* state : m_snapshots) delete state;
}

void FrameState::update(IRInstruction* inst) {
  auto const opc = inst->op();

  switch (opc) {
  case DefInlineFP:    trackDefInlineFP(inst);  break;
  case InlineReturn:   trackInlineReturn(inst); break;

  case InlineFPAnchor: m_hasFPAnchor = true;  break;

  case Call:
    m_spValue = inst->dst();
    m_frameSpansCall = true;
    // A call pops the ActRec and pushes a return value.
    m_spOffset -= kNumActRecCells;
    m_spOffset += 1;
    assert(m_spOffset >= 0);
    clearCse();
    killLocalsForCall();
    break;

  case CallArray:
    m_spValue = inst->dst();
    m_frameSpansCall = true;
    // A CallArray pops the ActRec an array arg and pushes a return value.
    m_spOffset -= kNumActRecCells;
    assert(m_spOffset >= 0);
    clearCse();
    killLocalsForCall();
    break;

  case ContEnter:
    clearCse();
    killLocalsForCall();
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
    m_spOffset = inst->extra<ReDefSP>()->offset;
    break;

  case DefInlineSP:
  case DefSP:
    m_spValue = inst->dst();
    m_spOffset = inst->extra<StackOffset>()->offset;
    break;

  case AssertStk:
  case AssertStkVal:
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
    int64_t stackAdjustment = inst->src(1)->getValInt();
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

  case StRefNT:
  case StRef: {
    SSATmp* newRef = inst->dst();
    SSATmp* prevRef = inst->src(0);
    // update other tracked locals that also contain prevRef
    updateLocalRefValues(prevRef, newRef);
    break;
  }

  case StLocNT:
  case StLoc:
    setLocalValue(inst->extra<LocalId>()->locId, inst->src(1));
    break;

  case LdLoc:
    setLocalValue(inst->extra<LdLoc>()->locId, inst->dst());
    break;

  case OverrideLoc:
    // If changing the inner type of a boxed local, also drop the
    // information about inner types for any other boxed locals.
    if (inst->typeParam().isBoxed()) dropLocalRefsInnerTypes();
    setLocalType(inst->extra<LocalId>()->locId, inst->typeParam());
    break;

  case AssertLoc:
  case GuardLoc:
  case CheckLoc:
    m_fpValue = inst->dst();
    refineLocalType(inst->extra<LocalId>()->locId, inst->typeParam());
    break;

  case CheckType: {
    SSATmp* newVal = inst->dst();
    SSATmp* oldVal = inst->src(0);
    updateLocalValues(oldVal, newVal);
    break;
  }

  case OverrideLocVal:
    setLocalValue(inst->extra<LocalId>()->locId, inst->src(1));
    break;

  case SmashLocals:
    clearLocals();
    break;

  case IterInitK:
  case WIterInitK:
    // kill the locals to which this instruction stores iter's key and value
    killLocalValue(inst->src(3)->getValInt());
    killLocalValue(inst->src(4)->getValInt());
    break;

  case IterInit:
  case WIterInit:
    // kill the local to which this instruction stores iter's value
    killLocalValue(inst->src(3)->getValInt());
    break;

  case IterNextK:
  case WIterNextK:
    // kill the locals to which this instruction stores iter's key and value
    killLocalValue(inst->src(2)->getValInt());
    killLocalValue(inst->src(3)->getValInt());
    break;

  case IterNext:
  case WIterNext:
    // kill the local to which this instruction stores iter's value
    killLocalValue(inst->src(2)->getValInt());
    break;

  case LdThis:
    m_thisAvailable = true;
    break;

  default:
    break;
  }

  if (MInstrEffects::supported(inst)) {
    MInstrEffects::get(inst,
                       [&](uint32_t id, SSATmp* val) { // storeLocalValue
                         setLocalValue(id, val);
                       },
                       [&](uint32_t id, Type t) { // setLocalType
                         setLocalType(id, t);
                       });
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

  // save a copy of the current state for each successor.
  if (Block* target = inst->taken()) save(target);
}

void FrameState::startBlock(Block* block) {
  if (m_snapshots[block]) {
    load(block);
  }
}

void FrameState::finishBlock(Block* block) {
  assert(block->back()->isTerminal() == !block->next());

  if (!block->back()->isTerminal()) {
    save(block->next());
  }
}

std::unique_ptr<FrameState::Snapshot> FrameState::createSnapshot() const {
  std::unique_ptr<Snapshot> state(new Snapshot);
  state->spValue = m_spValue;
  state->fpValue = m_fpValue;
  state->curFunc = m_curFunc;
  state->spOffset = m_spOffset;
  state->thisAvailable = m_thisAvailable;
  state->locals = m_locals;
  state->callerAvailableValues = m_callerAvailableValues;
  state->curMarker = m_marker;
  state->frameSpansCall = m_frameSpansCall;
  state->needsFPAnchor = m_hasFPAnchor;
  assert(state->curMarker.valid());
  return state;
}

/*
 * Save current state for block.  If this is the first time saving state for
 * block, create a new snapshot.  Otherwise merge the current state into the
 * existing snapshot.
 */
void FrameState::save(Block* block) {
  if (auto* state = m_snapshots[block]) {
    merge(state);

    // reset caller's available values for all inlined frames
    for (auto& state : m_inlineSavedStates) {
      state->callerAvailableValues.clear();
    }
  } else {
    m_snapshots[block] = createSnapshot().release();
  }
}

void FrameState::load(std::unique_ptr<Snapshot> state) {
  m_spValue = state->spValue;
  m_fpValue = state->fpValue;
  m_spOffset = state->spOffset;
  m_curFunc = state->curFunc;
  m_thisAvailable = state->thisAvailable;
  m_locals = std::move(state->locals);
  m_callerAvailableValues = std::move(state->callerAvailableValues);
  m_marker = state->curMarker;
  m_hasFPAnchor = state->needsFPAnchor;
  m_frameSpansCall = m_frameSpansCall || state->frameSpansCall;

  // If spValue is null, we merged two different but equivalent values. We
  // could define a new sp but that would drop a lot of useful information on
  // the floor. Let's cross this bridge when we reach it.
  always_assert(m_spValue &&
                "Attempted to merge two states with different stack pointers");
}

void FrameState::load(Block* block) {
  assert(m_snapshots[block]);
  std::unique_ptr<Snapshot> state(m_snapshots[block]);
  m_snapshots[block] = nullptr;
  load(std::move(state));
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
void FrameState::merge(Snapshot* state) {
  // cannot merge fp or spOffset state, so assert they match
  assert(state->fpValue == m_fpValue);
  assert(state->spOffset == m_spOffset);
  assert(state->curFunc == m_curFunc);
  if (state->spValue != m_spValue) {
    // we have two different sp definitions but we know they're equal
    // because spOffset matched.
    state->spValue = nullptr;
  }
  // this is available iff it's available in both states
  state->thisAvailable &= m_thisAvailable;

  assert(m_locals.size() == state->locals.size());
  for (unsigned i = 0; i < m_locals.size(); ++i) {
    auto& local = state->locals[i];

    // preserve local values if they're the same in both states,
    // This would be the place to insert phi nodes (jmps+deflabels) if we want
    // to avoid clearing state, which triggers a downstream reload.
    if (local.value != m_locals[i].value) local.value = nullptr;

    local.type = Type::unionOf(local.type, m_locals[i].type);
    local.unsafe = local.unsafe || m_locals[i].unsafe;
    local.written = local.written || m_locals[i].written;
  }

  // Don't attempt to continue tracking caller's available values.
  state->callerAvailableValues.clear();

  // We should not be merging states that have different hhbc bytecode
  // boundaries.
  assert(m_marker.valid() && state->curMarker == m_marker);
}

void FrameState::trackDefInlineFP(IRInstruction* inst) {
  auto const target     = inst->extra<DefInlineFP>()->target;
  auto const savedSPOff = inst->extra<DefInlineFP>()->retSPOff;
  auto const calleeFP   = inst->dst();
  auto const calleeSP   = inst->src(0);
  auto const savedSP    = inst->src(1);

  // Saved tracebuilder state will include the "return" fp/sp.
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

  m_inlineSavedStates.push_back(createSnapshot());

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
  m_hasFPAnchor     = false;

  /*
   * Keep the outer locals somewhere for isValueAvailable() to know
   * about their liveness, to help with incref/decref elimination.
   */
  for (auto const& state : m_locals) {
    m_callerAvailableValues.push_back(state.value);
  }
  m_callerAvailableValues.insert(m_callerAvailableValues.end(),
                                 stackValues.begin(),
                                 stackValues.end());

  m_locals.clear();
  m_locals.resize(target->numLocals());
}

void FrameState::trackInlineReturn(IRInstruction* inst) {
  load(std::move(m_inlineSavedStates.back()));
  m_inlineSavedStates.pop_back();
}

/**
 * Checks if the given SSATmp, or any of its aliases, is available in
 * any VM location, including locals and the This pointer.
 */
bool FrameState::isValueAvailable(SSATmp* tmp) const {
  while (true) {
    if (anyLocalHasValue(tmp)) return true;
    if (callerHasValueAvailable(tmp)) return true;

    IRInstruction* srcInstr = tmp->inst();
    Opcode srcOpcode = srcInstr->op();

    // ensure that the LdThis is in the same frame
    if (srcOpcode == LdThis && srcInstr->src(0) == m_fpValue) return true;

    if (srcInstr->isPassthrough()) {
      tmp = srcInstr->getPassthroughValue();
    } else {
      return false;
    }
  }
}

bool FrameState::callerHasValueAvailable(SSATmp* tmp) const {
  return std::find(m_callerAvailableValues.begin(),
                   m_callerAvailableValues.end(),
                   tmp) != m_callerAvailableValues.end();
}

bool FrameState::needsFPAnchor(IRInstruction* inst) const {
  return m_inlineSavedStates.size() && !m_hasFPAnchor &&
    (inst->isNative() || inst->mayRaiseError());
}

CSEHash* FrameState::cseHashTable(IRInstruction* inst) {
  return inst->op() == DefConst ? &m_unit.constTable() :
         &m_cseHash;
}

void FrameState::cseInsert(IRInstruction* inst) {
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
  clearLocals();
  m_callerAvailableValues.clear();
  m_frameSpansCall = false;
  m_hasFPAnchor = false;
  m_spValue = m_fpValue = nullptr;
  m_spOffset = 0;
  m_thisAvailable = false;
  for (auto i = m_snapshots.begin(), end = m_snapshots.end(); i != end; ++i) {
    delete *i;
    *i = nullptr;
  }
  m_marker = BCMarker();
}

SSATmp* FrameState::localValue(uint32_t id) const {
  always_assert(id < m_locals.size());
  return m_locals[id].unsafe ? nullptr : m_locals[id].value;
}

SSATmp* FrameState::localValueSource(uint32_t id) const {
  always_assert(id < m_locals.size());
  auto const& local = m_locals[id];

  if (local.value) return local.value;
  if (local.written) return nullptr;
  return fp();
}

Type FrameState::localType(uint32_t id) const {
  always_assert(id < m_locals.size());
  assert(m_locals[id].type != Type::None);
  return m_locals[id].type;
}

void FrameState::setLocalValue(uint32_t id, SSATmp* value) {
  always_assert(id < m_locals.size());
  m_locals[id].value = value;
  m_locals[id].type = value ? value->type() : Type::Gen;
  m_locals[id].written = true;
  m_locals[id].unsafe = false;
}

void FrameState::setLocalType(uint32_t id, Type type) {
  always_assert(id < m_locals.size());
  m_locals[id].value = nullptr;
  m_locals[id].type = type;
  m_locals[id].written = true;
  m_locals[id].unsafe = false;
}

void FrameState::clearLocals() {
  for (unsigned i = 0; i < m_locals.size(); ++i) {
    setLocalValue(i, nullptr);
  }
}

void FrameState::refineLocalType(uint32_t id, Type type) {
  always_assert(id < m_locals.size());
  assert(type.subtypeOf(m_locals[id].type));
  m_locals[id].type = type;
}

// Needs to be called if a local escapes as a by-ref or
// otherwise set to an unknown value (e.g., by Iter[Init,Next][K])
void FrameState::killLocalValue(uint32_t id) {
  setLocalValue(id, nullptr);
}

void FrameState::updateLocalValues(SSATmp* oldVal, SSATmp* newVal) {
  Type newType = newVal->type();
  for (auto& loc : m_locals) {
    if (loc.value == oldVal) {
      assert(!loc.unsafe);
      loc.value = newVal;
      loc.type  = newType;
    }
  }
}

//
// This method updates the tracked values and types of all locals that contain
// oldRef so that they now contain newRef.
// This should only be called for ref/boxed types.
//
void FrameState::updateLocalRefValues(SSATmp* oldRef, SSATmp* newRef) {
  assert(oldRef->type().isBoxed());
  assert(newRef->type().isBoxed());

  auto findAndReplaceLocal = [&] (smart::vector<LocalState>& locals) {
    Type newRefType = newRef->type();
    for (auto& loc : locals) {
      if (loc.value == oldRef) {
        assert(!loc.unsafe);
        loc.value = newRef;
        loc.type  = newRefType;
      }
    }
  };

  auto findAndReplaceCallerAvailable = [&] (std::vector<SSATmp*>& vec) {
    size_t nTrackedLocs = vec.size();
    for (size_t id = 0; id < nTrackedLocs; id++) {
      if (vec[id] == oldRef) {
        vec[id] = newRef;
      }
    }
  };

  findAndReplaceLocal(m_locals);
  findAndReplaceCallerAvailable(m_callerAvailableValues);
  for (auto& state : m_inlineSavedStates) {
    findAndReplaceLocal(state->locals);
    findAndReplaceCallerAvailable(state->callerAvailableValues);
  }
}

/**
 * This method changes any boxed local into a BoxedCell type.
 */
void FrameState::dropLocalRefsInnerTypes() {
  auto dropTypes = [] (smart::vector<LocalState>& locals) {
    for (auto& loc : locals) {
      if (loc.type.isBoxed()) loc.type = Type::BoxedCell;
    }
  };
  dropTypes(m_locals);
  for (auto& state : m_inlineSavedStates) {
    dropTypes(state->locals);
  }
}

/**
 * Called to clear out the tracked local values at a call site.
 * Calls kill all registers, so we don't want to keep locals in
 * registers across calls. We do continue tracking the types in
 * locals, however.
 */
void FrameState::killLocalsForCall() {
  auto doKill = [&](smart::vector<LocalState>& locals) {
    for (auto& loc : locals) {
      SSATmp* t = loc.value;
      // should not kill DefConst, and LdConst should be replaced by DefConst
      if (!t || t->inst()->op() == DefConst) continue;

      if (t->inst()->op() == LdConst) {
        // make the new DefConst instruction
        IRInstruction* clone = m_unit.cloneInstruction(t->inst());
        clone->setOpcode(DefConst);
        loc.value = clone->dst();
        continue;
      }
      assert(!t->isConst());
      loc.unsafe = true;
    }
  };

  doKill(m_locals);
  m_callerAvailableValues.clear();

  for (auto& state : m_inlineSavedStates) {
    doKill(state->locals);
    state->callerAvailableValues.clear();
  }
}

bool FrameState::anyLocalHasValue(SSATmp* tmp) const {
  return std::any_of(m_locals.begin(), m_locals.end(),
                     [tmp](const LocalState& s) {
                       return !s.unsafe && s.value == tmp;
                     });
}

} }
