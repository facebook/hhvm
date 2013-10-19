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
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/util/assertions.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

TraceBuilder::TraceBuilder(Offset initialBcOffset,
                           Offset initialSpOffsetFromFp,
                           IRUnit& unit,
                           const Func* func)
  : m_unit(unit)
  , m_simplifier(*this)
  , m_state(unit, initialSpOffsetFromFp, func)
  , m_curTrace(m_unit.makeMain(func, initialBcOffset)->trace())
  , m_curBlock(nullptr)
  , m_enableSimplification(false)
  , m_inReoptimize(false)
{
  if (RuntimeOption::EvalHHIRGenOpts) {
    m_state.setEnableCse(RuntimeOption::EvalHHIRCse);
    m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  }
}

TraceBuilder::~TraceBuilder() {
}

/*
 * Returns whether or not the given value might have its type relaxed by guard
 * relaxation. If tmp is null, only conditions that apply to all values are
 * checked.
 */
bool TraceBuilder::typeMightRelax(SSATmp* tmp /* = nullptr */) const {
  if (!RuntimeOption::EvalHHIRRelaxGuards) return false;
  if (inReoptimize()) return false;
  if (tmp && tmp->isConst()) return false;

  return true;
}

/*
 * To help guard relaxation, there are some situations where we want to keep
 * around an Assert(Type|Stk|Loc) instruction that doesn't provide a more
 * specific type than its source.
 */
bool TraceBuilder::shouldElideAssertType(Type oldType, Type newType,
                                         SSATmp* oldVal) const {
  assert(oldType.maybe(newType));

  if (!typeMightRelax(oldVal)) return newType >= oldType;
  if (oldType == Type::Cls || newType == Type::Gen) return true;

  return newType > oldType;
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
  return gen(DefConst, Type::PtrToInitNull, ConstData(&init_null_variant));
}

SSATmp* TraceBuilder::genPtrToUninit() {
  return gen(DefConst, Type::PtrToUninit, ConstData(&null_variant));
}

SSATmp* TraceBuilder::genDefNone() {
  return gen(DefConst, Type::None, ConstData(0));
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
      Block* next = m_unit.defBlock(m_state.func());
      m_curTrace->push_back(next);
      if (!prev->isTerminal()) {
        // new block is reachable from old block so link it.
        block->setNext(next);
      }
      block = next;
    }
  }
  appendInstruction(inst, block);
  if (m_savedTraces.empty()) {
    // We don't track state on non-main traces for now. t2982555
    m_state.update(inst);
  }
}

void TraceBuilder::appendBlock(Block* block) {
  assert(m_savedTraces.empty()); // TODO(t2982555): Don't require this

  m_state.finishBlock(m_curTrace->back());

  // Load up the state for the new block.
  m_state.startBlock(block);
  m_curTrace->push_back(block);
}

std::vector<RegionDesc::TypePred> TraceBuilder::getKnownTypes() const {
  std::vector<RegionDesc::TypePred> result;
  auto const curFunc  = m_state.func();
  auto const sp       = m_state.sp();
  auto const spOffset = m_state.spOffset();

  for (unsigned i = 0; i < curFunc->maxStackCells(); ++i) {
    auto t = getStackValue(sp, i).knownType;
    if (!t.equals(Type::StackElem)) {
      result.push_back({ RegionDesc::Location::Stack{i, spOffset - i}, t });
    }
  }

  for (unsigned i = 0; i < curFunc->numLocals(); ++i) {
    auto t = m_state.localType(i);
    if (!t.equals(Type::Gen)) {
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

  if (prevType.subtypeOf(typeParam)) {
    return inst->src(0);
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
        return gen(Jmp, inst->taken());
      }
    }
  }

  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeAssertLoc(IRInstruction* inst) {
  auto const locId = inst->extra<AssertLoc>()->locId;

  auto const prevType = localType(locId, DataTypeGeneric);
  auto const typeParam = inst->typeParam();

  if (prevType.not(typeParam)) {
    /* Task #2553746
     * This is triggering for a case where the tracked state says the local is
     * InitNull but the AssertLoc says it's Str. */
    static auto const error =
      makeStaticString("Internal error: static analysis was "
                       "wrong about a local variable's type.");
    auto* errorInst = m_unit.gen(RaiseError, inst->marker(), cns(error));
    inst->become(m_unit, errorInst);

    // It's not a disaster to generate this in unreachable code for
    // now. t2590033.
    if (false) {
      assert_log(false,  [&]{
          return folly::format("\npreOptimizeAssertLoc: prevType: {} "
                               "typeParam: {}\nin instr: {}\nin trace: {}\n",
                               prevType.toString(),
                               typeParam.toString(),
                               inst->toString(),
                               m_unit.main()->toString()).str();
        });
    }
  } else if (shouldElideAssertType(prevType, typeParam, nullptr)) {
    // The type we're asserting is worse than the last known type.
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeLdThis(IRInstruction* inst) {
  if (m_state.thisAvailable()) {
    auto fpInst = inst->src(0)->inst();
    if (fpInst->op() == DefInlineFP) {
      if (!m_state.frameSpansCall()) { // check that we haven't nuked the SSATmp
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
  if (m_state.thisAvailable()) return gen(LdThis, m_state.fp());
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
    if (m_state.isValueAvailable(srcInst->src(0))) {
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
  if (thisAvailable()) {
    auto const thiss = gen(LdThis, m_state.fp());
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
        m_state.callerHasValueAvailable(thisInst->src(0))) {
      gen(DecRefNZ, thiss);
      inst->convertToNop();
      return nullptr;
    }

    assert(inst->src(0) == m_state.fp());
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
   * We can't really rely on the types held in the boxed values since aliasing
   * stores may change them, and we only guard during LdRef.  So we have to
   * change any boxed type to BoxedCell.
   *
   * DataTypeGeneric is used because we don't want a DecRef to be the only
   * thing keeping a guard around. This code is designed to tolerate the
   * incoming type being relaxed.
   */
  auto knownType = localType(locId, DataTypeGeneric);
  if (knownType.isBoxed()) {
    knownType = Type::BoxedCell;
  }

  /*
   * If we have the local value in flight, use a DecRef on it instead of doing
   * it in memory.
   */
  if (auto tmp = localValue(locId, DataTypeGeneric)) {
    gen(DecRef, tmp);
    inst->convertToNop();
    return nullptr;
  }

  if (!typeMightRelax()) {
    inst->setTypeParam(
      Type::mostRefined(knownType, inst->typeParam())
    );
  }

  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeLdLoc(IRInstruction* inst) {
  auto const locId = inst->extra<LdLoc>()->locId;
  if (auto tmp = localValue(locId, DataTypeGeneric)) {
    return tmp;
  }

  auto const type = localType(locId, DataTypeGeneric);
  // If FrameState's type isn't as good as the type param, we're missing
  // information in the IR.
  assert(inst->typeParam() >= type);
  inst->setTypeParam(Type::mostRefined(type, inst->typeParam()));
  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeLdLocAddr(IRInstruction* inst) {
  auto const locId = inst->extra<LdLocAddr>()->locId;
  auto const type = localType(locId, DataTypeGeneric);
  assert(inst->typeParam().deref() >= type);
  inst->setTypeParam(Type::mostRefined(type.ptr(), inst->typeParam()));
  return nullptr;
}

SSATmp* TraceBuilder::preOptimizeStLoc(IRInstruction* inst) {
  // Guard relaxation might change the current local type, so don't try to
  // change to StLocNT until after relaxation happens.
  if (!inReoptimize()) return nullptr;

  auto locId = inst->extra<StLoc>()->locId;
  auto const curType = localType(locId, DataTypeGeneric);
  auto const newType = inst->src(1)->type();

  assert(inst->typeParam() == Type::None);

  // There's no need to store the type if it's going to be the same
  // KindOfFoo. We still have to store string types because we don't
  // guard on KindOfStaticString vs. KindOfString.
  auto const bothBoxed = curType.isBoxed() && newType.isBoxed();
  auto const sameUnboxed =
    curType.isSameKindOf(newType) && !curType.isString();
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
  if (m_state.needsFPAnchor(inst)) {
    m_state.setHasFPAnchor();
    always_assert(m_state.fp() != nullptr);
    gen(InlineFPAnchor, m_state.fp());
    FTRACE(2, "Anchor for: {}\n", inst->toString());
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
  if (m_state.enableCse() && inst->canCSE()) {
    result = m_state.cseLookup(inst, idoms);
    if (result) {
      // Found a dominating instruction that can be used instead of inst
      FTRACE(1, "  {}cse found: {}\n",
             indent(), result->inst()->toString());

      // CheckType and AssertType are special. They're marked as both PRc and
      // CRc to placate our refcounting optimizations, for for the purposes of
      // CSE they're neither.
      if (inst->is(CheckType, AssertType)) {
        return result;
      }
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
    if (doClone == CloneFlag::Yes) inst = m_unit.cloneInstruction(inst);
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
  assert(m_curTrace->isMain());
  assert(m_savedTraces.empty());

  m_state.setEnableCse(RuntimeOption::EvalHHIRCse);
  m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  if (!m_state.enableCse() && !m_enableSimplification) return;
  always_assert(!m_inReoptimize);
  m_inReoptimize = true;

  BlockList sortedBlocks = rpoSortCfg(m_unit);
  auto const idoms = findDominators(m_unit, sortedBlocks);
  m_state.clear();

  auto blocks = std::move(m_curTrace->blocks());
  assert(m_curTrace->blocks().empty());
  while (!blocks.empty()) {
    Block* block = blocks.front();
    blocks.pop_front();
    assert(block->trace() == m_curTrace);
    FTRACE(5, "Block: {}\n", block->id());

    assert(m_curTrace->isMain());
    m_state.startBlock(block);
    m_curTrace->push_back(block);

    auto instructions = std::move(block->instrs());
    assert(block->empty());
    while (!instructions.empty()) {
      auto *inst = &instructions.front();
      instructions.pop_front();
      m_state.setMarker(inst->marker());

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
        m_state.update(inst);
        continue;
      }
      SSATmp* dst = inst->dst();
      if (dst->type() != Type::None && dst != tmp) {
        // The result of optimization has a different destination than the inst.
        // Generate a mov(tmp->dst) to get result into dst. If we get here then
        // assume the last instruction in the block isn't a guard. If it was,
        // we would have to insert the mov on the fall-through edge.
        assert(block->empty() || !block->back()->isBlockEnd());
        IRInstruction* mov = m_unit.mov(dst, tmp, inst->marker());
        appendInstruction(mov, block);
        m_state.update(mov);
      }
      // Not re-adding inst; remove the inst->taken edge
      if (inst->taken()) inst->setTaken(nullptr);
    }

    if (block->empty()) {
      // If all the instructions in the block were optimized away, remove it
      // from the trace.
      auto it = m_curTrace->blocks().end();
      --it;
      assert(*it == block);
      m_curTrace->unlink(it);
    } else {
      if (block->back()->isTerminal()) {
        // Could have converted a conditional branch to Jmp; clear next.
        block->setNext(nullptr);
      }
      m_state.finishBlock(block);
    }
  }
}

bool TraceBuilder::shouldConstrainGuards() const {
  return RuntimeOption::EvalHHIRRelaxGuards &&
    !inReoptimize();
}

void TraceBuilder::constrainGuard(IRInstruction* inst,
                                  TypeConstraint tc) {
  if (!shouldConstrainGuards()) return;

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
                                     TypeConstraint tc) {
  if (!shouldConstrainGuards()) return val;

  if (!val) {
    FTRACE(1, "constrainValue(nullptr, {}), bailing\n", tc);
    return nullptr;
  }

  FTRACE(1, "constrainValue({}, {})\n", *val->inst(), tc);

  auto inst = val->inst();
  if (inst->is(LdLoc, LdLocAddr)) {
    // We've hit a LdLoc(Addr). If the source of the value is non-null and not
    // a FramePtr, it's a real value that was killed by a Call. The value won't
    // be live but it's ok to use it to track down the guard.

    auto source = inst->extra<LocalData>()->valSrc;
    if (!source) {
      // val was newly created in this trace. Nothing to constrain.
      FTRACE(2, "  - valSrc is null, bailing\n");
      return val;
    }

    // If valSrc is a FramePtr, it represents the frame the value was
    // originally loaded from. Look for the guard for this local.
    if (source->isA(Type::FramePtr)) {
      constrainLocal(inst->extra<LocalId>()->locId, source, tc,
                     "constrainValue");
      return val;
    }

    // Otherwise, keep chasing down the source of val.
    constrainValue(source, tc);
  } else if (inst->is(LdStack, LdStackAddr)) {
    constrainStack(inst->src(0), inst->extra<StackOffset>()->offset, tc);
  } else if (inst->is(CheckType, AssertType)) {
    // If the dest type of the instruction fits the constraint we want, we can
    // stop here without constraining any further. Otherwise, continue through
    // to the source.
    if (inst->is(CheckType)) constrainGuard(inst, tc);

    auto dstType = inst->typeParam();
    if (!typeFitsConstraint(dstType, tc.category)) {
      constrainValue(inst->src(0), tc);
    }
  } else if (inst->is(StRef, StRefNT, Box, BoxPtr)) {
    // If our caller cares about the inner type, propagate that through.
    // Otherwise we're done.
    if (tc.innerCat) {
      auto src = inst->src(inst->is(StRef, StRefNT) ? 1 : 0);
      tc.innerCat.reset();
      constrainValue(src, tc);
    }
  } else if (inst->is(LdRef, Unbox, UnboxPtr)) {
    // Pass through to the source of the box, remembering that we care about
    // the inner type of the box.
    assert(!tc.innerCat);
    tc.innerCat = tc.category;
    constrainValue(inst->src(0), tc);
  } else if (inst->isPassthrough()) {
    constrainValue(inst->getPassthroughValue(), tc);
  } else {
    // Any instructions not special cased above produce a new value, so
    // there's no guard for us to constrain.
    FTRACE(2, "  - value is new in this trace, bailing\n");
  }
  // TODO(t2598894): Should be able to do something with LdMem<T> here

  return val;
}

void TraceBuilder::constrainLocal(uint32_t locId, TypeConstraint tc,
                                  const std::string& why) {
  constrainLocal(locId, localValueSource(locId), tc, why);
}

void TraceBuilder::constrainLocal(uint32_t locId, SSATmp* valSrc,
                                  TypeConstraint tc,
                                  const std::string& why) {
  if (!shouldConstrainGuards()) return;

  FTRACE(1, "constrainLocal({}, {}, {}, {})\n",
         locId, valSrc ? valSrc->inst()->toString() : "null", tc, why);

  if (!valSrc) return;
  if (!valSrc->isA(Type::FramePtr)) {
    constrainValue(valSrc, tc);
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
      if (typeFitsConstraint(guard->typeParam(), tc.category)) return;
      guard = guardForLocal(locId, guard->src(0));
    } else {
      assert(guard->is(GuardLoc, AssertLoc));
      FTRACE(2, "    - found guard to constrain\n");
      constrainGuard(guard, tc);
      return;
    }
  }

  FTRACE(2, "    - no guard to constrain\n");
}

void TraceBuilder::constrainStack(int32_t idx, TypeConstraint tc) {
  constrainStack(sp(), idx, tc);
}

void TraceBuilder::constrainStack(SSATmp* sp, int32_t idx,
                                  TypeConstraint tc) {
  if (!shouldConstrainGuards()) return;

  FTRACE(1, "constrainStack({}, {}, {})\n", *sp->inst(), idx, tc);
  assert(sp->isA(Type::StkPtr));

  // We've hit a LdStack. If getStackValue gives us a value, recurse on
  // that. Otherwise, look at the instruction that gave us the type of the
  // stack element. If it's a GuardStk or CheckStk, it's our target. If it's
  // anything else, the value is new so there's no guard to relax.
  auto stackInfo = getStackValue(sp, idx);
  if (stackInfo.value) {
    FTRACE(1, "  - value = {}\n", *stackInfo.value->inst());
    constrainValue(stackInfo.value, tc);
  } else {
    auto typeSrc = stackInfo.typeSrc;
    FTRACE(1, "  - typeSrc = {}\n", *typeSrc);
    if (typeSrc->is(GuardStk, CheckStk)) constrainGuard(typeSrc, tc);
  }
}

Type TraceBuilder::localType(uint32_t id, TypeConstraint tc) {
  constrainLocal(id, tc, "localType");
  return m_state.localType(id);
}

SSATmp* TraceBuilder::localValue(uint32_t id, TypeConstraint tc) {
  constrainLocal(id, tc, "localValue");
  return m_state.localValue(id);
}

void TraceBuilder::setMarker(BCMarker marker) {
  auto const oldMarker = m_state.marker();

  if (marker == oldMarker) return;
  FTRACE(2, "TraceBuilder changing current marker from {} to {}\n",
         oldMarker.func ? oldMarker.show() : "<invalid>", marker.show());
  assert(marker.valid());
  m_state.setMarker(marker);
}

void TraceBuilder::pushTrace(IRTrace* t, BCMarker marker, Block* b,
                             const boost::optional<Block::iterator>& where) {
  FTRACE(2, "TraceBuilder saving {}@{} and using {}@{}\n",
         m_curTrace, m_state.marker().show(), t, marker.show());
  assert(t);
  assert(bool(b) == bool(where));
  assert(IMPLIES(b, b->trace() == t));

  m_savedTraces.push(
    TraceState{ m_curTrace, m_curBlock, m_state.marker(), m_curWhere });
  m_curTrace = t;
  m_curBlock = b;
  setMarker(marker);
  m_curWhere = where;
}

void TraceBuilder::popTrace() {
  assert(!m_savedTraces.empty());

  auto const& top = m_savedTraces.top();
  FTRACE(2, "TraceBuilder popping {}@{} to restore {}@{}\n",
         m_curTrace, m_state.marker().show(), top.trace, top.marker.show());
  m_curTrace = top.trace;
  m_curBlock = top.block;
  setMarker(top.marker);
  m_curWhere = top.where;
  m_savedTraces.pop();
}

}}
