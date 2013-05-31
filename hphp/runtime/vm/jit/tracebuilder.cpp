/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/vm/jit/tracebuilder.h"

#include "folly/ScopeGuard.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/targetcache.h"
#include "hphp/runtime/vm/jit/irfactory.h"

namespace HPHP {
namespace JIT {

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

TraceBuilder::TraceBuilder(Offset initialBcOffset,
                           Offset initialSpOffsetFromFp,
                           IRFactory& irFactory,
                           const Func* func)
  : m_irFactory(irFactory)
  , m_simplifier(this)
  , m_trace(makeTrace(func, initialBcOffset))
  , m_enableCse(false)
  , m_enableSimplification(false)
  , m_snapshots(&irFactory, nullptr)
  , m_spValue(nullptr)
  , m_fpValue(nullptr)
  , m_spOffset(initialSpOffsetFromFp)
  , m_thisIsAvailable(false)
  , m_refCountedMemValue(nullptr)
  , m_localValues(func->numLocals(), nullptr)
  , m_localTypes(func->numLocals(), Type::None)
{
  m_curFunc = cns(func);
  if (RuntimeOption::EvalHHIRGenOpts) {
    m_enableCse = RuntimeOption::EvalHHIRCse;
    m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  }
}

TraceBuilder::~TraceBuilder() {
  for (State* state : m_snapshots) delete state;
}

/**
 * Checks if the given SSATmp, or any of its aliases, is available in
 * any VM location, including locals and the This pointer.
 */
bool TraceBuilder::isValueAvailable(SSATmp* tmp) const {
  while (true) {
    if (m_refCountedMemValue == tmp) return true;
    if (anyLocalHasValue(tmp)) return true;
    if (callerHasValueAvailable(tmp)) return true;

    IRInstruction* srcInstr = tmp->inst();
    Opcode srcOpcode = srcInstr->op();

    if (srcOpcode == LdThis) return true;

    if (srcInstr->isPassthrough()) {
      tmp = srcInstr->getPassthroughValue();
    } else {
      return false;
    }
  }
}

SSATmp* TraceBuilder::genDefUninit() {
  return gen(DefConst, Type::Uninit, ConstData(0));
}

SSATmp* TraceBuilder::genDefInitNull() {
  return gen(DefConst, Type::InitNull, ConstData(0));
}

SSATmp* TraceBuilder::genDefNull() {
  return gen(DefConst, Type::Null, ConstData(0));
}

SSATmp* TraceBuilder::genPtrToInitNull() {
  return gen(DefConst, Type::PtrToUninit, ConstData(&null_variant));
}

SSATmp* TraceBuilder::genPtrToUninit() {
  return gen(DefConst, Type::PtrToInitNull, ConstData(&init_null_variant));
}

SSATmp* TraceBuilder::genDefNone() {
  return gen(DefConst, Type::None, ConstData(0));
}

void TraceBuilder::trackDefInlineFP(IRInstruction* inst) {
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

  m_inlineSavedStates.push_back(createState());

  /*
   * Set up the callee state.
   *
   * We set m_thisIsAvailable to true on any object method, because we
   * just don't inline calls to object methods with a null $this.
   */
  m_fpValue         = calleeFP;
  m_spValue         = calleeSP;
  m_thisIsAvailable = target->cls() != nullptr;
  m_curFunc         = cns(target);

  /*
   * Keep the outer locals somewhere for isValueAvailable() to know
   * about their liveness, to help with incref/decref elimination.
   */
  m_callerAvailableValues.insert(m_callerAvailableValues.end(),
                                 m_localValues.begin(),
                                 m_localValues.end());
  m_callerAvailableValues.insert(m_callerAvailableValues.end(),
                                 stackValues.begin(),
                                 stackValues.end());

  m_localValues.clear();
  m_localTypes.clear();
  m_localValues.resize(target->numLocals(), nullptr);
  m_localTypes.resize(target->numLocals(), Type::None);
}

void TraceBuilder::trackInlineReturn(IRInstruction* inst) {
  useState(std::move(m_inlineSavedStates.back()));
  m_inlineSavedStates.pop_back();
}

void TraceBuilder::updateTrackedState(IRInstruction* inst) {
  Opcode opc = inst->op();
  // Update tracked state of local values/types, stack/frame pointer, CSE, etc.

  // kill tracked memory values
  if (inst->mayModifyRefs()) {
    m_refCountedMemValue = nullptr;
  }

  switch (opc) {
  case DefInlineFP:    trackDefInlineFP(inst);  break;
  case InlineReturn:   trackInlineReturn(inst); break;

  case Marker:
    m_lastMarker = *inst->extra<Marker>();
    break;

  case Call:
    m_spValue = inst->dst();
    // A call pops the ActRec and pushes a return value.
    m_spOffset -= kNumActRecCells;
    m_spOffset += 1;
    assert(m_spOffset >= 0);
    killCse();
    killLocals();
    break;

  case CallArray:
    m_spValue = inst->dst();
    // A CallArray pops the ActRec an array arg and pushes a return value.
    m_spOffset -= kNumActRecCells;
    assert(m_spOffset >= 0);
    killCse();
    killLocals();
    break;

  case ContEnter:
    killCse();
    killLocals();
    break;

  case DefFP:
  case FreeActRec:
    m_fpValue = inst->dst();
    break;

  case ReDefGeneratorSP:
  case DefSP:
  case ReDefSP:
    m_spValue = inst->dst();
    m_spOffset = inst->extra<StackOffset>()->offset;
    break;

  case AssertStk:
  case CastStk:
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

  case InterpOne: {
    m_spValue = inst->dst();
    int64_t stackAdjustment = inst->src(3)->getValInt();
    // push the return value if any and adjust for the popped values
    m_spOffset -= stackAdjustment;
    break;
  }

  case StProp:
  case StPropNT:
    // fall through to StMem; stored value is the same arg number (2)
  case StMem:
  case StMemNT:
    m_refCountedMemValue = inst->src(2);
    break;

  case LdMem:
  case LdProp:
  case LdRef:
    m_refCountedMemValue = inst->dst();
    break;

  case StRefNT:
  case StRef: {
    m_refCountedMemValue = inst->src(2);
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
    if (inst->typeParam().isBoxed()) {
      dropLocalRefsInnerTypes();
    }
    // fallthrough
  case AssertLoc:
  case GuardLoc:
    setLocalType(inst->extra<LocalId>()->locId,
                 inst->typeParam());
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
    m_thisIsAvailable = true;
    break;

  default:
    break;
  }

  if (VectorEffects::supported(inst)) {
    VectorEffects::get(inst,
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
  if (Block* target = inst->taken()) saveState(target);
}

std::unique_ptr<TraceBuilder::State> TraceBuilder::createState() const {
  std::unique_ptr<State> state(new State);
  state->spValue = m_spValue;
  state->fpValue = m_fpValue;
  state->curFunc = m_curFunc;
  state->spOffset = m_spOffset;
  state->thisAvailable = m_thisIsAvailable;
  state->localValues = m_localValues;
  state->localTypes = m_localTypes;
  state->callerAvailableValues = m_callerAvailableValues;
  state->refCountedMemValue = m_refCountedMemValue;
  state->lastMarker = *m_lastMarker;
  return state;
}

/*
 * Save current state for block.  If this is the first time saving state
 * for block, create a new snapshot.  Otherwise merge the current state into
 * the existing snapshot.
 */
void TraceBuilder::saveState(Block* block) {
  if (State* state = m_snapshots[block]) {
    mergeState(state);
  } else {
    m_snapshots[block] = createState().release();
  }
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
void TraceBuilder::mergeState(State* state) {
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
  state->thisAvailable &= m_thisIsAvailable;
  for (unsigned i = 0, n = state->localValues.size(); i < n; ++i) {
    // preserve local values if they're the same in both states,
    // This would be the place to insert phi nodes (jmps+deflabels) if we want
    // to avoid clearing state, which triggers a downstream reload.
    if (state->localValues[i] != m_localValues[i]) {
      state->localValues[i] = nullptr;
    }
  }
  for (unsigned i = 0, n = state->localTypes.size(); i < n; ++i) {
    // combine types using Type::unionOf(), but handle Type::None here (t2135185)
    Type t1 = state->localTypes[i];
    Type t2 = m_localTypes[i];
    state->localTypes[i] = (t1 == Type::None || t2 == Type::None) ? Type::None :
                           Type::unionOf(t1, t2);
  }
  // Reference counted memory value is available only if it is available on both
  // paths
  if (state->refCountedMemValue != m_refCountedMemValue) {
    state->refCountedMemValue = nullptr;
  }

  // Don't attempt to continue tracking caller's available values.
  state->callerAvailableValues.clear();

  // We should not be merging states that have different hhbc bytecode
  // boundaries.
  assert(m_lastMarker &&
    state->lastMarker.bcOff    == m_lastMarker->bcOff &&
    state->lastMarker.stackOff == m_lastMarker->stackOff &&
    state->lastMarker.func     == m_lastMarker->func);
}

void TraceBuilder::useState(std::unique_ptr<State> state) {
  m_spValue = state->spValue;
  m_fpValue = state->fpValue;
  m_spOffset = state->spOffset;
  m_curFunc = state->curFunc;
  m_thisIsAvailable = state->thisAvailable;
  m_refCountedMemValue = state->refCountedMemValue;
  m_localValues = std::move(state->localValues);
  m_localTypes = std::move(state->localTypes);
  m_callerAvailableValues = std::move(state->callerAvailableValues);
  m_lastMarker = state->lastMarker;
  // If spValue is null, we merged two different but equivalent values.
  // Define a new sp using the known-good spOffset.
  if (!m_spValue) {
    gen(DefSP, StackOffset(m_spOffset), m_fpValue);
  }
}

void TraceBuilder::useState(Block* block) {
  assert(m_snapshots[block]);
  std::unique_ptr<State> state(m_snapshots[block]);
  m_snapshots[block] = nullptr;
  useState(std::move(state));
}

void TraceBuilder::clearTrackedState() {
  killCse(); // clears m_cseHash
  for (uint32_t i = 0; i < m_localValues.size(); i++) {
    m_localValues[i] = nullptr;
  }
  for (uint32_t i = 0; i < m_localTypes.size(); i++) {
    m_localTypes[i] = Type::None;
  }
  m_callerAvailableValues.clear();
  m_spValue = m_fpValue = nullptr;
  m_spOffset = 0;
  m_thisIsAvailable = false;
  m_refCountedMemValue = nullptr;
  for (auto i = m_snapshots.begin(), end = m_snapshots.end(); i != end; ++i) {
    delete *i;
    *i = nullptr;
  }
  m_lastMarker = folly::none;
}

void TraceBuilder::appendInstruction(IRInstruction* inst, Block* block) {
  Opcode opc = inst->op();
  if (opc != Nop && opc != DefConst) {
    block->push_back(inst);
  }
}

void TraceBuilder::appendInstruction(IRInstruction* inst) {
  Block* block = m_trace->back();
  if (!block->empty()) {
    IRInstruction* prev = block->back();
    if (prev->isBlockEnd()) {
      // start a new block
      Block* next = m_irFactory.defBlock(m_curFunc->getValFunc());
      m_trace->push_back(next);
      if (!prev->isTerminal()) {
        // new block is reachable from old block so link it.
        block->setNext(next);
      }
      block = next;
      assert(m_lastMarker.hasValue());
      gen(Marker, *m_lastMarker);
    }
  }
  appendInstruction(inst, block);
  updateTrackedState(inst);
}

void TraceBuilder::appendBlock(Block* block) {
  if (!m_trace->back()->back()->isTerminal()) {
    // previous instruction falls through; merge current state with block.
    saveState(block);
  }
  m_trace->push_back(block);
  useState(block);
  assert(m_lastMarker.hasValue());
  gen(Marker, *m_lastMarker);
}

CSEHash* TraceBuilder::cseHashTable(IRInstruction* inst) {
  return inst->op() == DefConst ? &m_irFactory.constTable() :
         &m_cseHash;
}

void TraceBuilder::cseInsert(IRInstruction* inst) {
  cseHashTable(inst)->insert(inst->dst());
}

void TraceBuilder::cseKill(SSATmp* src) {
  if (src->inst()->canCSE()) {
    cseHashTable(src->inst())->erase(src);
  }
}

SSATmp* TraceBuilder::cseLookup(IRInstruction* inst) {
  return cseHashTable(inst)->lookup(inst);
}

//////////////////////////////////////////////////////////////////////

SSATmp* TraceBuilder::preOptimizeCheckLoc(IRInstruction* inst) {
  auto const locId = inst->extra<CheckLoc>()->locId;

  if (auto const prevValue = getLocalValue(locId)) {
    always_assert(false && "WTF");
    return gen(
      CheckType, inst->typeParam(), inst->taken(), prevValue
    );
  }

  auto const prevType = getLocalType(locId);
  if (prevType != Type::None) {
    always_assert(false && "WTF2");
    // It doesn't make sense to be checking something that's deemed to
    // fail.
    assert(prevType == inst->typeParam());
    inst->convertToNop();
  }

  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeAssertLoc(IRInstruction* inst) {
  auto const locId = inst->extra<AssertLoc>()->locId;
  auto const prevType = getLocalType(locId);
  auto const typeParam = inst->typeParam();

  if (!prevType.equals(Type::None) && !typeParam.strictSubtypeOf(prevType)) {
    assert(prevType.subtypeOf(typeParam));
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeLdThis(IRInstruction* inst) {
  if (isThisAvailable()) inst->setTaken(nullptr);
  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeLdCtx(IRInstruction* inst) {
  if (isThisAvailable()) return gen(LdThis, m_fpValue);
  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeDecRef(IRInstruction* inst) {
  /*
   * Refcount optimization:
   *
   * If the decref'ed value is guaranteed to be available after the decref,
   * generate DecRefNZ instead of DecRef.
   *
   * This is safe WRT copy-on-write because all the instructions that
   * could cause a COW return a new SSATmp that will replace the
   * tracked state that we are using to determine the value is still
   * available.  I.e. by the time they get to the DecRef we won't see
   * it in isValueAvailable anymore and won't convert to DecRefNZ.
   */
  auto const srcInst = inst->src(0)->inst();
  if (srcInst->op() == IncRef) {
    if (isValueAvailable(srcInst->src(0))) {
      inst->setOpcode(DecRefNZ);
    }
  }

  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeDecRefThis(IRInstruction* inst) {
  /*
   * If $this is available, convert to an instruction sequence that
   * doesn't need to test if it's already live.
   */
  if (isThisAvailable()) {
    auto const thiss = gen(LdThis, m_fpValue);
    auto const thisInst = thiss->inst();

    /*
     * DecRef optimization for $this in an inlined frame: if a caller
     * local contains the $this, we know it can't go to zero and can
     * switch DecRef to DecRefNZ.
     *
     * It's ok not to do DecRefThis (which normally nulls out the ActRec
     * $this), because there is still a reference to it in the caller
     * frame, so debug_backtrace() can't see a non-live pointer value.
     */
    if (thisInst->op() == IncRef &&
        callerHasValueAvailable(thisInst->src(0))) {
      gen(DecRefNZ, thiss);
      inst->convertToNop();
      return nullptr;
    }

    assert(inst->src(0) == m_fpValue);
    gen(DecRef, thiss);
    inst->convertToNop();
    return nullptr;
  }

  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeDecRefLoc(IRInstruction* inst) {
  auto const locId = inst->extra<DecRefLoc>()->locId;

  /*
   * Refine the type if we can.
   *
   * We can't really rely on the types held in the boxed values since
   * aliasing stores may change them, and we only guard during LdRef.
   * So we have to change any boxed type to BoxedCell.
   */
  auto knownType = getLocalType(locId);
  if (knownType.isBoxed()) {
    knownType = Type::BoxedCell;
  }
  if (knownType != Type::None) { // TODO(#2135185)
    inst->setTypeParam(
      Type::mostRefined(knownType, inst->typeParam())
    );
  }

  /*
   * If we have the local value in flight, use a DecRef on it instead
   * of doing it in memory.
   */
  if (auto tmp = getLocalValue(locId)) {
    gen(DecRef, tmp);
    inst->convertToNop();
  }

  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeLdLoc(IRInstruction* inst) {
  auto const locId = inst->extra<LdLoc>()->locId;
  if (auto tmp = getLocalValue(locId)) {
    return tmp;
  }
  if (getLocalType(locId) != Type::None) { // TODO(#2135185)
    inst->setTypeParam(
      Type::mostRefined(getLocalType(locId), inst->typeParam())
    );
  }
  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeLdLocAddr(IRInstruction* inst) {
  auto const locId = inst->extra<LdLocAddr>()->locId;
  if (getLocalType(locId) != Type::None) { // TODO(#2135185)
    inst->setTypeParam(
      Type::mostRefined(getLocalType(locId).ptr(), inst->typeParam())
    );
  }
  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeStLoc(IRInstruction* inst) {
  auto const curType = getLocalType(inst->extra<StLoc>()->locId);
  auto const newType = inst->src(1)->type();

  assert(inst->typeParam().equals(Type::None));

  // There's no need to store the type if it's going to be the same
  // KindOfFoo.  We still have to store string types because we don't
  // guard on KindOfStaticString vs. KindOfString.
  auto const bothBoxed = curType.isBoxed() && newType.isBoxed();
  auto const sameUnboxed = curType != Type::None && // TODO(#2135185)
    curType.isKnownDataType() &&
    curType.equals(newType) && !curType.isString();
  if (bothBoxed || sameUnboxed) {
    inst->setOpcode(StLocNT);
  }

  return nullptr;
}

SSATmp* TraceBuilder::preOptimize(IRInstruction* inst) {
#define X(op) case op: return preOptimize##op(inst)
  switch (inst->op()) {
    X(CheckLoc);
    X(AssertLoc);
    X(LdThis);
    X(LdCtx);
    X(DecRef);
    X(DecRefThis);
    X(DecRefLoc);
    X(LdLoc);
    X(LdLocAddr);
    X(StLoc);
  default:
    break;
  }
#undef X
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

SSATmp* TraceBuilder::optimizeWork(IRInstruction* inst) {
  static DEBUG_ONLY __thread int instNest = 0;
  if (debug) ++instNest;
  SCOPE_EXIT { if (debug) --instNest; };
  DEBUG_ONLY auto indent = [&] { return std::string(instNest * 2, ' '); };

  FTRACE(1, "{}{}\n", indent(), inst->toString());

  // First pass of tracebuilder optimizations try to replace an
  // instruction based on tracked state before we do anything else.
  // May mutate the IRInstruction in place (and return nullptr) or
  // return an SSATmp*.
  if (SSATmp* preOpt = preOptimize(inst)) {
    FTRACE(1, "  {}preOptimize returned: {}\n",
           indent(), preOpt->inst()->toString());
    return preOpt;
  }
  if (inst->op() == Nop) return nullptr;

  // copy propagation on inst source operands
  copyProp(inst);

  SSATmp* result = nullptr;
  if (m_enableCse && inst->canCSE()) {
    result = cseLookup(inst);
    if (result) {
      // Found a dominating instruction that can be used instead of inst
      FTRACE(1, "  {}cse found: {}\n",
             indent(), result->inst()->toString());
      if (inst->producesReference()) {
        // Replace with an IncRef
        FTRACE(1, "  {}cse of refcount-producing instruction\n", indent());
        return gen(IncRef, result);
      } else {
        return result;
      }
    }
  }

  if (m_enableSimplification) {
    result = m_simplifier.simplify(inst);
    if (result) {
      // Found a simpler instruction that can be used instead of inst
      FTRACE(1, "  {}simplification returned: {}\n",
             indent(), result->inst()->toString());
      assert(inst->hasDst());
      return result;
    }
  }
  return nullptr;
}

SSATmp* TraceBuilder::optimizeInst(IRInstruction* inst, CloneFlag doClone) {
  if (SSATmp* tmp = optimizeWork(inst)) {
    return tmp;
  }
  // Couldn't CSE or simplify the instruction; clone it and append.
  if (inst->op() != Nop) {
    if (doClone == CloneFlag::Yes) inst = inst->clone(&m_irFactory);
    appendInstruction(inst);
    // returns nullptr if instruction has no dest, returns the first
    // (possibly only) dest otherwise
    return inst->dst(0);
  }
  return nullptr;
}

void CSEHash::filter(Block* block, IdomVector& idoms) {
  for (auto it = map.begin(), end = map.end(); it != end;) {
    auto next = it; ++next;
    if (!dominates(it->second->inst()->block(), block, idoms)) {
      map.erase(it);
    }
    it = next;
  }
}

/*
 * reoptimize() runs a trace through a second pass of TraceBuilder
 * optimizations, like this:
 *
 *   reset state.
 *   move all blocks to a temporary list.
 *   compute immediate dominators.
 *   for each block in trace order:
 *     if we have a snapshot state for this block:
 *       clear cse entries that don't dominate this block.
 *       use snapshot state.
 *     move all instructions to a temporary list.
 *     for each instruction:
 *       optimizeWork - do CSE and simplify again
 *       if not simplified:
 *         append existing instruction and update state.
 *       else:
 *         if the instruction has a result, insert a mov from the
 *         simplified tmp to the original tmp and discard the instruction.
 *     if the last conditional branch was turned into a jump, remove the
 *     fall-through edge to the next block.
 */
void TraceBuilder::reoptimize() {
  FTRACE(5, "ReOptimize:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "ReOptimize:^^^^^^^^^^^^^^^^^^^^\n"); };

  m_enableCse = RuntimeOption::EvalHHIRCse;
  m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  if (!m_enableCse && !m_enableSimplification) return;
  if (m_trace->blocks().size() >
      RuntimeOption::EvalHHIRSimplificationMaxBlocks) {
    // TODO CSEHash::filter is very slow for large block sizes
    // t2135219 should address that
    return;
  }

  BlockList sortedBlocks = rpoSortCfg(m_trace.get(), m_irFactory);
  IdomVector idoms = findDominators(sortedBlocks);
  clearTrackedState();

  auto blocks = std::move(m_trace->blocks());
  assert(m_trace->blocks().empty());
  while (!blocks.empty()) {
    Block* block = blocks.front();
    blocks.pop_front();
    assert(block->trace() == m_trace.get());
    FTRACE(5, "Block: {}\n", block->id());

    m_trace->push_back(block);
    if (m_snapshots[block]) {
      useState(block);
      m_cseHash.filter(block, idoms);
    }

    auto instructions = std::move(block->instrs());
    assert(block->empty());
    while (!instructions.empty()) {
      auto *inst = &instructions.front();
      instructions.pop_front();

      SSATmp* tmp = optimizeWork(inst); // Can generate new instrs!
      if (!tmp) {
        // Could not optimize; keep the old instruction
        appendInstruction(inst, block);
        updateTrackedState(inst);
        continue;
      }
      SSATmp* dst = inst->dst();
      if (dst->type() != Type::None && dst != tmp) {
        // The result of optimization has a different destination than the inst.
        // Generate a mov(tmp->dst) to get result into dst. If we get here then
        // assume the last instruction in the block isn't a guard. If it was,
        // we would have to insert the mov on the fall-through edge.
        assert(!block->back()->isBlockEnd());
        IRInstruction* mov = m_irFactory.mov(dst, tmp);
        appendInstruction(mov, block);
        updateTrackedState(mov);
      }
      // Not re-adding inst; remove the inst->taken edge
      if (inst->taken()) inst->setTaken(nullptr);
    }
    if (block->back()->isTerminal()) {
      // Could have converted a conditional branch to Jmp; clear next.
      block->setNext(nullptr);
    } else {
      // if the last instruction was a branch, we already saved state
      // for the target in updateTrackedState().  Now save state for
      // the fall-through path.
      saveState(block->next());
    }
  }
}

void TraceBuilder::killCse() {
  m_cseHash.clear();
}

SSATmp* TraceBuilder::getLocalValue(unsigned id) const {
  assert(id < m_localValues.size());
  return m_localValues[id];
}

Type TraceBuilder::getLocalType(unsigned id) const {
  assert(id < m_localTypes.size());
  return m_localTypes[id];
}

void TraceBuilder::setLocalValue(unsigned id, SSATmp* value) {
  assert(id < m_localValues.size() && id < m_localTypes.size());
  m_localValues[id] = value;
  m_localTypes[id] = value ? value->type() : Type::None;
}

void TraceBuilder::setLocalType(uint32_t id, Type type) {
  assert(id < m_localValues.size() && id < m_localTypes.size());
  m_localValues[id] = nullptr;
  m_localTypes[id] = type;
}

// Needs to be called if a local escapes as a by-ref or
// otherwise set to an unknown value (e.g., by Iter[Init,Next][K])
void TraceBuilder::killLocalValue(uint32_t id) {
  assert(id < m_localValues.size() && id < m_localTypes.size());
  m_localValues[id] = nullptr;
  m_localTypes[id] = Type::None;
}

bool TraceBuilder::anyLocalHasValue(SSATmp* tmp) const {
  return std::find(m_localValues.begin(),
                   m_localValues.end(),
                   tmp) != m_localValues.end();
}

bool TraceBuilder::callerHasValueAvailable(SSATmp* tmp) const {
  return std::find(m_callerAvailableValues.begin(),
                   m_callerAvailableValues.end(),
                   tmp) != m_callerAvailableValues.end();
}

//
// This method updates the tracked values and types of all locals that contain
// oldRef so that they now contain newRef.
// This should only be called for ref/boxed types.
//
void TraceBuilder::updateLocalRefValues(SSATmp* oldRef, SSATmp* newRef) {
  assert(oldRef->type().isBoxed());
  assert(newRef->type().isBoxed());

  Type newRefType = newRef->type();
  size_t nTrackedLocs = m_localValues.size();
  for (size_t id = 0; id < nTrackedLocs; id++) {
    if (m_localValues[id] == oldRef) {
      m_localValues[id] = newRef;
      m_localTypes[id]  = newRefType;
    }
  }
}

/**
 * This method changes any boxed local into a BoxedCell type.
 */
void TraceBuilder::dropLocalRefsInnerTypes() {
  for (size_t id = 0; id < m_localTypes.size(); id++) {
    if (m_localTypes[id].isBoxed()) {
      m_localTypes[id] = Type::BoxedCell;
    }
  }
}

/**
 * Called to clear out the tracked local values at a call site.
 * Calls kill all registers, so we don't want to keep locals in
 * registers across calls. We do continue tracking the types in
 * locals, however.
 */
void TraceBuilder::killLocals() {
  for (uint32_t i = 0; i < m_localValues.size(); i++) {
    SSATmp* t = m_localValues[i];
    // should not kill DefConst, and LdConst should be replaced by DefConst
    if (!t || t->inst()->op() == DefConst) {
      continue;
    }
    if (t->inst()->op() == LdConst) {
      // make the new DefConst instruction
      IRInstruction* clone = t->inst()->clone(&m_irFactory);
      clone->setOpcode(DefConst);
      m_localValues[i] = clone->dst();
      continue;
    }
    assert(!t->isConst());
    m_localValues[i] = nullptr;
  }
}

}} // namespace HPHP::JIT
