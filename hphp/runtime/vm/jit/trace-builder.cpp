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

#include "hphp/runtime/vm/jit/trace-builder.h"

#include "folly/ScopeGuard.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/ir-factory.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/util/assertions.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

TraceBuilder::TraceBuilder(Offset initialBcOffset,
                           Offset initialSpOffsetFromFp,
                           IRFactory& irFactory,
                           const Func* func)
  : m_irFactory(irFactory)
  , m_simplifier(*this)
  , m_mainTrace(makeTrace(func, initialBcOffset))
  , m_curTrace(m_mainTrace.get())
  , m_curBlock(nullptr)
  , m_enableCse(false)
  , m_enableSimplification(false)
  , m_snapshots(irFactory, nullptr)
  , m_spValue(nullptr)
  , m_fpValue(nullptr)
  , m_spOffset(initialSpOffsetFromFp)
  , m_thisIsAvailable(false)
  , m_frameSpansCall(false)
  , m_needsFPAnchor(false)
  , m_refCountedMemValue(nullptr)
  , m_locals(func->numLocals())
{
  m_curFunc = m_irFactory.cns(func);
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

    // ensure that the LdThis is in the same frame
    if (srcOpcode == LdThis && srcInstr->src(0) == m_fpValue) return true;

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
  m_thisIsAvailable = target->cls() != nullptr && !target->isStatic();
  m_curFunc         = cns(target);
  m_frameSpansCall  = false;
  m_needsFPAnchor   = false;

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

void TraceBuilder::trackInlineReturn(IRInstruction* inst) {
  useState(std::move(m_inlineSavedStates.back()));
  m_inlineSavedStates.pop_back();
}

void TraceBuilder::updateTrackedState(IRInstruction* inst) {
  // We don't track state for any trace other than the main trace.
  if (m_savedTraces.size() > 0) return;

  Opcode opc = inst->op();
  // Update tracked state of local values/types, stack/frame pointer, CSE, etc.

  // kill tracked memory values
  if (inst->mayModifyRefs()) {
    m_refCountedMemValue = nullptr;
  }

  switch (opc) {
  case DefInlineFP:    trackDefInlineFP(inst);  break;
  case InlineReturn:   trackInlineReturn(inst); break;

  case InlineFPAnchor: m_needsFPAnchor = true;  break;

  case Call:
    m_spValue = inst->dst();
    m_frameSpansCall = true;
    // A call pops the ActRec and pushes a return value.
    m_spOffset -= kNumActRecCells;
    m_spOffset += 1;
    assert(m_spOffset >= 0);
    killCse();
    killLocalsForCall();
    break;

  case CallArray:
    m_spValue = inst->dst();
    m_frameSpansCall = true;
    // A CallArray pops the ActRec an array arg and pushes a return value.
    m_spOffset -= kNumActRecCells;
    assert(m_spOffset >= 0);
    killCse();
    killLocalsForCall();
    break;

  case ContEnter:
    killCse();
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

  case StElem:
  case StProp:
  case StPropNT:
    // fall through to StMem; stored value is the same arg number (2)
  case StMem:
  case StMemNT:
    m_refCountedMemValue = inst->src(2);
    break;

  case LdMem:
  case LdProp:
  case LdElem:
  case LdRef:
  case ArrayGet:
  case VectorGet:
  case PairGet:
  case MapGet:
  case StableMapGet:
    m_refCountedMemValue = inst->dst();
    break;

  case StRefNT:
  case StRef: {
    m_refCountedMemValue = inst->src(1);
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
    m_thisIsAvailable = true;
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
  if (Block* target = inst->taken()) saveState(target);
}

std::unique_ptr<TraceBuilder::State> TraceBuilder::createState() const {
  std::unique_ptr<State> state(new State);
  state->spValue = m_spValue;
  state->fpValue = m_fpValue;
  state->curFunc = m_curFunc;
  state->spOffset = m_spOffset;
  state->thisAvailable = m_thisIsAvailable;
  state->locals = m_locals;
  state->callerAvailableValues = m_callerAvailableValues;
  state->refCountedMemValue = m_refCountedMemValue;
  state->curMarker = m_curMarker;
  state->frameSpansCall = m_frameSpansCall;
  state->needsFPAnchor = m_needsFPAnchor;
  assert(state->curMarker.valid());
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

    // reset caller's available values for all inlined frames
    for (auto& state : m_inlineSavedStates) {
      state->callerAvailableValues.clear();
    }
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

  assert(m_locals.size() == state->locals.size());
  for (unsigned i = 0; i < m_locals.size(); ++i) {
    auto& local = state->locals[i];

    // preserve local values if they're the same in both states,
    // This would be the place to insert phi nodes (jmps+deflabels) if we want
    // to avoid clearing state, which triggers a downstream reload.
    if (local.value != m_locals[i].value) local.value = nullptr;

    // combine types using Type::unionOf(), but handle Type::None here: t2135185
    Type t1 = local.type;
    Type t2 = m_locals[i].type;
    local.type =
      (t1 == Type::None || t2 == Type::None) ? Type::None
                                             : Type::unionOf(t1, t2);

    local.unsafe = local.unsafe || m_locals[i].unsafe;
    local.written = local.written || m_locals[i].written;
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
  assert(m_curMarker.valid() && state->curMarker == m_curMarker);
}

void TraceBuilder::useState(std::unique_ptr<State> state) {
  m_spValue = state->spValue;
  m_fpValue = state->fpValue;
  m_spOffset = state->spOffset;
  m_curFunc = state->curFunc;
  m_thisIsAvailable = state->thisAvailable;
  m_refCountedMemValue = state->refCountedMemValue;
  m_locals = std::move(state->locals);
  m_callerAvailableValues = std::move(state->callerAvailableValues);
  m_curMarker = state->curMarker;
  m_needsFPAnchor = state->needsFPAnchor;
  m_frameSpansCall = m_frameSpansCall || state->frameSpansCall;
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
  clearLocals();
  m_callerAvailableValues.clear();
  m_frameSpansCall = false;
  m_needsFPAnchor = false;
  m_spValue = m_fpValue = nullptr;
  m_spOffset = 0;
  m_thisIsAvailable = false;
  m_refCountedMemValue = nullptr;
  for (auto i = m_snapshots.begin(), end = m_snapshots.end(); i != end; ++i) {
    delete *i;
    *i = nullptr;
  }
  m_curMarker = BCMarker();
}

void TraceBuilder::appendInstruction(IRInstruction* inst, Block* block) {
  assert(inst->marker().valid());
  Opcode opc = inst->op();
  if (opc != Nop && opc != DefConst) {
    block->push_back(inst);
  }
}

void TraceBuilder::appendInstruction(IRInstruction* inst) {
  if (m_curWhere) {
    // We have a specific position to insert instructions.
    assert(!inst->isBlockEnd());
    auto& it = m_curWhere.get();
    it = m_curBlock->insert(it, inst);
    ++it;
    return;
  }

  Block* block = m_curTrace->back();
  if (!block->empty()) {
    IRInstruction* prev = block->back();
    if (prev->isBlockEnd()) {
      // start a new block
      Block* next = m_irFactory.defBlock(m_curFunc->getValFunc());
      m_curTrace->push_back(next);
      if (!prev->isTerminal()) {
        // new block is reachable from old block so link it.
        block->setNext(next);
      }
      block = next;
    }
  }
  appendInstruction(inst, block);
  updateTrackedState(inst);
}

void TraceBuilder::appendBlock(Block* block) {
  if (!m_curTrace->back()->back()->isTerminal()) {
    // previous instruction falls through; merge current state with block.
    saveState(block);
  }
  m_curTrace->push_back(block);
  useState(block);
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

SSATmp* TraceBuilder::cseLookup(IRInstruction* inst,
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

std::vector<RegionDesc::TypePred> TraceBuilder::getKnownTypes() const {
  std::vector<RegionDesc::TypePred> result;
  const Func* curFunc = m_curFunc->getValFunc();

  for (unsigned i = 0; i < curFunc->maxStackCells(); ++i) {
    auto t = getStackValue(m_spValue, i).knownType;
    if (!t.equals(Type::None) && !t.equals(Type::Gen)) {
      result.push_back({ RegionDesc::Location::Stack{i}, t });
    }
  }

  // XXX(t2598894) This is only safe right now because it's not called on a
  // trace with relaxed guards.
  for (unsigned i = 0; i < curFunc->numLocals(); ++i) {
    auto t = m_locals[i].type;
    if (!t.equals(Type::None) && !t.equals(Type::Gen)) {
      result.push_back({ RegionDesc::Location::Local{i}, t });
    }
  }
  return result;
}

//////////////////////////////////////////////////////////////////////

SSATmp* TraceBuilder::preOptimizeCheckLoc(IRInstruction* inst) {
  auto const locId = inst->extra<CheckLoc>()->locId;
  Type typeParam = inst->typeParam();

  if (auto const prevValue = localValue(locId, DataTypeGeneric)) {
    return gen(CheckType, typeParam, inst->taken(), prevValue);
  }

  auto const prevType = localType(locId, DataTypeSpecific);

  if (prevType.equals(Type::None)) {
    return nullptr;
  }

  if (prevType.subtypeOf(typeParam)) {
    inst->convertToNop();
  } else {
    //
    // Normally, it doesn't make sense to be checking something that's
    // deemed to fail.  Incompatible boxed types are ok though, since
    // we don't track them precisely, but instead check them at every
    // use.
    //
    // However, in JitPGO mode right now, this pathological case can
    // happen, because profile counters are not accurate and we
    // currently don't analyze Block post-conditions when picking its
    // successors during region selection.  This can lead to
    // incompatible types in blocks selected for the same region.
    //
    if (!typeParam.isBoxed() || !prevType.isBoxed()) {
      if ((typeParam & prevType) == Type::Bottom) {
        assert(RuntimeOption::EvalJitPGO);
        return gen(Jmp_, inst->taken());
      }
    }
  }

  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeAssertLoc(IRInstruction* inst) {
  auto const locId = inst->extra<AssertLoc>()->locId;
  auto const prevType = localType(locId, DataTypeGeneric);
  auto const typeParam = inst->typeParam();

  if (!prevType.equals(Type::None) && !typeParam.strictSubtypeOf(prevType)) {
    if (!prevType.subtypeOf(typeParam)) {
      /* Task #2553746
       * This is triggering for a case where the tracked state says the local is
       * InitNull but the AssertLoc says it's Str. */
      static auto const error =
        StringData::GetStaticString("Internal error: static analysis was "
                                    "wrong about a local variable's type.");
      auto* errorInst = m_irFactory.gen(RaiseError, inst->marker(), cns(error));
      inst->become(m_irFactory, errorInst);

      // It's not a disaster to generate this in unreachable code for
      // now. t2590033.
      if (false) {
        assert_log(false,  [&]{
            IRTrace& mainTrace = trace()->isMain() ? *trace()
                                                   : *(trace()->main());
            return folly::format("\npreOptimizeAssertLoc: prevType: {} "
                                 "typeParam: {}\nin instr: {}\nin trace: {}\n",
                                 prevType.toString(), typeParam.toString(),
                                 inst->toString(), mainTrace.toString()).str();
          });
      }
    } else {
      inst->convertToNop();
    }
  }
  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeLdThis(IRInstruction* inst) {
  if (isThisAvailable()) {
    auto fpInst = inst->src(0)->inst();
    if (fpInst->op() == DefInlineFP) {
      if (!m_frameSpansCall) { // check that we haven't nuked the SSATmp
        auto spInst = fpInst->src(0)->inst();
        if (spInst->op() == SpillFrame && spInst->src(3)->isA(Type::Obj)) {
          return spInst->src(3);
        }
      }
    }
    inst->setTaken(nullptr);
  }
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
  auto knownType = localType(locId, DataTypeCountness);
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
  if (auto tmp = localValue(locId, DataTypeCountness)) {
    gen(DecRef, tmp);
    inst->convertToNop();
  }

  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeLdLoc(IRInstruction* inst) {
  auto const locId = inst->extra<LdLoc>()->locId;
  if (auto tmp = localValue(locId, DataTypeGeneric)) {
    return tmp;
  }

  auto const type = localType(locId, DataTypeGeneric);
  if (!type.equals(Type::None)) { // TODO(#2135185)
    inst->setTypeParam(Type::mostRefined(type, inst->typeParam()));
  }
  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeLdLocAddr(IRInstruction* inst) {
  auto const locId = inst->extra<LdLocAddr>()->locId;
  auto const type = localType(locId, DataTypeGeneric);
  if (!type.equals(Type::None)) { // TODO(#2135185)
    inst->setTypeParam(Type::mostRefined(type.ptr(), inst->typeParam()));
  }
  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeStLoc(IRInstruction* inst) {
  auto locId = inst->extra<StLoc>()->locId;
  auto const curType = localType(locId, DataTypeGeneric);
  auto const newType = inst->src(1)->type();

  assert(inst->typeParam().equals(Type::None));

  // There's no need to store the type if it's going to be the same
  // KindOfFoo. We still have to store string types because we don't
  // guard on KindOfStaticString vs. KindOfString.
  auto const bothBoxed = curType.isBoxed() && newType.isBoxed();
  auto const sameUnboxed = curType != Type::None && // TODO(#2135185)
    curType.isSameKindOf(newType) &&
    !curType.isString();
  if (bothBoxed || sameUnboxed) {
    // TODO(t2598894) once relaxGuards supports proper type reflowing, we
    // should be able to relax the constraint here and degrade StLocNT to
    // StLoc if we relax its input.
    if (sameUnboxed) constrainLocal(locId, DataTypeSpecific,
                                    "StLoc -> StLocNT");
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

SSATmp* TraceBuilder::optimizeWork(IRInstruction* inst,
                                   const folly::Optional<IdomVector>& idoms) {
  // Since some of these optimizations inspect tracked state, we don't
  // perform any of them on non-main traces.
  if (m_savedTraces.size() > 0) return nullptr;

  static DEBUG_ONLY __thread int instNest = 0;
  if (debug) ++instNest;
  SCOPE_EXIT { if (debug) --instNest; };
  DEBUG_ONLY auto indent = [&] { return std::string(instNest * 2, ' '); };

  FTRACE(1, "{}{}\n", indent(), inst->toString());

  // turn off ActRec optimization for instructions that will require a frame
  if (m_inlineSavedStates.size() && !m_needsFPAnchor) {
    if (inst->isNative() || inst->mayRaiseError()) {
      m_needsFPAnchor = true;
      always_assert(m_fpValue != nullptr);
      gen(InlineFPAnchor, m_fpValue);
      FTRACE(2, "Anchor for: {}\n", inst->toString());
    }
  }

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
    result = cseLookup(inst, idoms);
    if (result) {
      // Found a dominating instruction that can be used instead of inst
      FTRACE(1, "  {}cse found: {}\n",
             indent(), result->inst()->toString());
      assert(!inst->consumesReferences());
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
  if (auto const tmp = optimizeWork(inst, folly::none)) {
    return tmp;
  }
  // Couldn't CSE or simplify the instruction; clone it and append.
  if (inst->op() != Nop) {
    if (doClone == CloneFlag::Yes) inst = m_irFactory.cloneInstruction(inst);
    appendInstruction(inst);
    // returns nullptr if instruction has no dest, returns the first
    // (possibly only) dest otherwise
    return inst->dst(0);
  }
  return nullptr;
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
  assert(m_curTrace == m_mainTrace.get());
  assert(m_savedTraces.empty());
  assert(m_inlineSavedStates.empty());

  m_enableCse = RuntimeOption::EvalHHIRCse;
  m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  if (!m_enableCse && !m_enableSimplification) return;
  if (m_mainTrace->blocks().size() >
      RuntimeOption::EvalHHIRSimplificationMaxBlocks) {
    // TODO CSEHash::filter is very slow for large block sizes
    // t2135219 should address that
    return;
  }

  BlockList sortedBlocks = rpoSortCfg(m_mainTrace.get(), m_irFactory);
  auto const idoms = findDominators(sortedBlocks);
  clearTrackedState();

  auto blocks = std::move(m_mainTrace->blocks());
  assert(m_mainTrace->blocks().empty());
  while (!blocks.empty()) {
    Block* block = blocks.front();
    blocks.pop_front();
    assert(block->trace() == m_mainTrace.get());
    FTRACE(5, "Block: {}\n", block->id());

    m_mainTrace->push_back(block);
    if (m_snapshots[block]) {
      useState(block);
    }

    auto instructions = std::move(block->instrs());
    assert(block->empty());
    while (!instructions.empty()) {
      auto *inst = &instructions.front();
      instructions.pop_front();

      // last attempt to elide ActRecs, if we still need the InlineFPAnchor
      // it will be added back to the trace when we re-add instructions that
      // rely on it
      if (inst->op() == InlineFPAnchor) {
        continue;
      }

      // merging state looks at the current marker, and optimizeWork
      // below may create new instructions. Use the marker from this
      // instruction.
      assert(inst->marker().valid());
      setMarker(inst->marker());

      auto const tmp = optimizeWork(inst, idoms); // Can generate new instrs!
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
        assert(block->empty() || !block->back()->isBlockEnd());
        IRInstruction* mov = m_irFactory.mov(dst, tmp, inst->marker());
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

void TraceBuilder::clearLocals() {
  for (unsigned i = 0; i < m_locals.size(); ++i) {
    setLocalValue(i, nullptr);
  }
}

SSATmp* TraceBuilder::localValue(unsigned id, DataTypeCategory cat) {
  always_assert(id < m_locals.size());
  constrainLocal(id, cat, "localValue");
  return m_locals[id].unsafe ? nullptr : m_locals[id].value;
}

SSATmp* TraceBuilder::localValueSource(unsigned id) const {
  always_assert(id < m_locals.size());
  auto const& local = m_locals[id];

  if (local.value) return local.value;
  if (local.written) return nullptr;
  return fp();
}

Type TraceBuilder::localType(unsigned id, DataTypeCategory cat) {
  always_assert(id < m_locals.size());
  constrainLocal(id, cat, "localType");
  return m_locals[id].type;
}

void TraceBuilder::setLocalValue(unsigned id, SSATmp* value) {
  always_assert(id < m_locals.size());
  m_locals[id].value = value;
  m_locals[id].type = value ? value->type() : Type::None;
  m_locals[id].written = true;
  m_locals[id].unsafe = false;
}

void TraceBuilder::setLocalType(uint32_t id, Type type) {
  always_assert(id < m_locals.size());
  m_locals[id].value = nullptr;
  m_locals[id].type = type;
  m_locals[id].written = true;
  m_locals[id].unsafe = false;
}

void TraceBuilder::refineLocalType(uint32_t id, Type type) {
  always_assert(id < m_locals.size());
  auto UNUSED oldType = m_locals[id].type;
  oldType = oldType.equals(Type::None) ? Type::Gen : oldType;
  assert(type.subtypeOf(oldType));
  m_locals[id].type = type;
}

// Needs to be called if a local escapes as a by-ref or
// otherwise set to an unknown value (e.g., by Iter[Init,Next][K])
void TraceBuilder::killLocalValue(uint32_t id) {
  setLocalValue(id, nullptr);
}

void TraceBuilder::constrainGuard(IRInstruction* inst,
                                  TypeConstraint tc) {
  if (!RuntimeOption::EvalHHIRRelaxGuards) return;

  auto& guard = m_guardConstraints[inst];

  if (tc.innerCat) {
    // If the constraint is for the inner type and is better than what guard
    // has, update it.
    auto cat = tc.innerCat.get();
    if (guard.innerCat && guard.innerCat >= cat) return;
    FTRACE(1, "constraining inner type of {}: {} -> {}\n",
           *inst, guard.innerCat ? guard.innerCat.get() : DataTypeGeneric, cat);
    guard.innerCat = cat;
    return;
  }

  if (tc.category > guard.category) {
    FTRACE(1, "constraining {}: {} -> {}\n",
           *inst, guard.category, tc.category);
    guard.category = tc.category;
  }
  if (tc.knownType < guard.knownType) {
    FTRACE(1, "refining knownType of {}: {} -> {}\n",
           *inst, guard.knownType, tc.knownType);
    guard.knownType = tc.knownType;
  }
}

/**
 * Trace back to the guard that provided the type of val, if
 * any. Constrain it so its type will not be relaxed beyond the given
 * DataTypeCategory. Always returns val, for convenience.
 */
SSATmp* TraceBuilder::constrainValue(SSATmp* const val,
                                     TypeConstraint cat) {
  if (!RuntimeOption::EvalHHIRRelaxGuards) return val;

  if (!val) {
    FTRACE(1, "constrainValue(nullptr, {}), bailing\n", cat);
    return nullptr;
  }

  FTRACE(1, "constrainValue({}, {})\n", *val->inst(), cat);

  auto inst = val->inst();
  if (inst->is(LdLoc, LdLocAddr)) {
    // We've hit a LdLoc(Addr). If the source of the value is non-null and not
    // a FramePtr, it's a real value that was killed by a Call. The value won't
    // be live but it's ok to use it to track down the guard.

    auto source = inst->extra<LdLocData>()->valSrc;
    if (!source) {
      // val was newly created in this trace. Nothing to constrain.
      FTRACE(2, "  - valSrc is null, bailing\n");
      return val;
    }

    // If valSrc is a FramePtr, it represents the frame the value was
    // originally loaded from. Look for the guard for this local.
    if (source->isA(Type::FramePtr)) {
      constrainLocal(inst->extra<LocalId>()->locId, source, cat,
                     "constrainValue");
      return val;
    }

    // Otherwise, keep chasing down the source of val.
    constrainValue(source, cat);
  } else if (inst->is(LdStack, LdStackAddr)) {
    constrainStack(inst->src(0), inst->extra<StackOffset>()->offset, cat);
  } else if (inst->is(CheckType, AssertType)) {
    // If the dest type of the instruction fits the constraint we want, we can
    // stop here without constraining any further. Otherwise, continue through
    // to the source.
    if (inst->is(CheckType)) constrainGuard(inst, cat);

    auto dstType = inst->typeParam();
    if (!typeFitsConstraint(dstType, cat.category)) {
      constrainValue(inst->src(0), cat);
    }
  } else if (inst->is(StRef, StRefNT, Box, BoxPtr)) {
    // If our caller cares about the inner type, propagate that through.
    // Otherwise we're done.
    if (cat.innerCat) {
      auto src = inst->src(inst->is(StRef, StRefNT) ? 1 : 0);
      cat.innerCat.reset();
      constrainValue(src, cat);
    }
  } else if (inst->is(LdRef, Unbox, UnboxPtr)) {
    // Pass through to the source of the box, remembering that we care about
    // the inner type of the box.
    assert(!cat.innerCat);
    cat.innerCat = cat.category;
    constrainValue(inst->src(0), cat);
  } else if (inst->isPassthrough()) {
    constrainValue(inst->getPassthroughValue(), cat);
  } else {
    // Any instructions not special cased above produce a new value, so
    // there's no guard for us to constrain.
    FTRACE(2, "  - value is new in this trace, bailing\n");
  }
  // TODO(t2598894): Should be able to do something with LdMem<T> here

  return val;
}

void TraceBuilder::constrainLocal(uint32_t locId, TypeConstraint cat,
                                  const std::string& why) {
  constrainLocal(locId, localValueSource(locId), cat, why);
}

void TraceBuilder::constrainLocal(uint32_t locId, SSATmp* valSrc,
                                  TypeConstraint cat,
                                  const std::string& why) {
  if (!RuntimeOption::EvalHHIRRelaxGuards) return;

  FTRACE(1, "constrainLocal({}, {}, {}, {})\n",
         locId, valSrc ? valSrc->inst()->toString() : "null", cat, why);

  if (!valSrc) return;
  if (!valSrc->isA(Type::FramePtr)) {
    constrainValue(valSrc, cat);
    return;
  }

  // When valSrc is a FramePtr, that means we loaded the value the local had
  // coming into the trace. Trace through the FramePtr chain, looking for a
  // guard for this local id. If we find it, constrain the guard. If we don't
  // find it, there wasn't a guard for this local so there's nothing to
  // constrain.
  auto guard = guardForLocal(locId, valSrc);
  while (guard) {
    if (guard->is(AssertLoc)) {
      // If the refined the type of the local satisfies the constraint we're
      // trying to apply, we can stop here. This can happen if we assert a
      // more general type than what we already know. Otherwise we need to
      // keep tracing back to the guard.
      if (typeFitsConstraint(guard->typeParam(), cat.category)) return;
      guard = guardForLocal(locId, guard->src(0));
    } else {
      assert(guard->is(GuardLoc, AssertLoc));
      FTRACE(2, "    - found guard to constrain\n");
      constrainGuard(guard, cat);
      return;
    }
  }

  FTRACE(2, "    - no guard to constrain\n");
}

void TraceBuilder::constrainStack(int32_t idx, TypeConstraint cat) {
  constrainStack(sp(), idx, cat);
}

void TraceBuilder::constrainStack(SSATmp* sp, int32_t idx,
                                  TypeConstraint cat) {
  if (!RuntimeOption::EvalHHIRRelaxGuards) return;

  FTRACE(1, "constrainStack({}, {}, {})\n", *sp->inst(), idx, cat);
  assert(sp->isA(Type::StkPtr));

  // We've hit a LdStack. If getStackValue gives us a value, recurse on
  // that. Otherwise, look at the instruction that gave us the type of the
  // stack element. If it's a GuardStk or CheckStk, it's our target. If it's
  // anything else, the value is new so there's no guard to relax.
  auto stackInfo = getStackValue(sp, idx);
  if (stackInfo.value) {
    FTRACE(1, "  - value = {}\n", *stackInfo.value->inst());
    constrainValue(stackInfo.value, cat);
  } else {
    auto typeSrc = stackInfo.typeSrc;
    FTRACE(1, "  - typeSrc = {}\n", *typeSrc);
    if (typeSrc->is(GuardStk, CheckStk)) constrainGuard(typeSrc, cat);
  }
}

bool TraceBuilder::anyLocalHasValue(SSATmp* tmp) const {
  return std::any_of(m_locals.begin(), m_locals.end(),
                     [tmp](const LocalState& s) {
                       return !s.unsafe && s.value == tmp;
                     });
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
void TraceBuilder::dropLocalRefsInnerTypes() {
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
void TraceBuilder::killLocalsForCall() {
  auto doKill = [&](smart::vector<LocalState>& locals) {
    for (auto& loc : locals) {
      SSATmp* t = loc.value;
      // should not kill DefConst, and LdConst should be replaced by DefConst
      if (!t || t->inst()->op() == DefConst) continue;

      if (t->inst()->op() == LdConst) {
        // make the new DefConst instruction
        IRInstruction* clone = m_irFactory.cloneInstruction(t->inst());
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

void TraceBuilder::setMarker(BCMarker marker) {
  if (m_curMarker == marker) return;
  FTRACE(2, "TraceBuilder changing current marker from {} to {}\n",
         m_curMarker.func ? m_curMarker.show() : "<invalid>", marker.show());
  assert(marker.valid());
  m_curMarker = marker;
}

void TraceBuilder::pushTrace(IRTrace* t, BCMarker marker, Block* b,
                             const boost::optional<Block::iterator>& where) {
  FTRACE(2, "TraceBuilder saving {}@{} and using {}@{}\n",
         m_curTrace, m_curMarker.show(), t, marker.show());
  assert(t);
  assert(bool(b) == bool(where));
  assert(IMPLIES(b, b->trace() == t));

  m_savedTraces.push(
    TraceState{ m_curTrace, m_curBlock, m_curMarker, m_curWhere });
  m_curTrace = t;
  m_curBlock = b;
  setMarker(marker);
  m_curWhere = where;
}

void TraceBuilder::popTrace() {
  assert(!m_savedTraces.empty());

  auto const& top = m_savedTraces.top();
  FTRACE(2, "TraceBuilder popping {}@{} to restore {}@{}\n",
         m_curTrace, m_curMarker.show(), top.trace, top.marker.show());
  m_curTrace = top.trace;
  m_curBlock = top.block;
  setMarker(top.marker);
  m_curWhere = top.where;
  m_savedTraces.pop();
}

}}
