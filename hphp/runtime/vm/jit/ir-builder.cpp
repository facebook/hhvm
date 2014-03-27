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

#include "hphp/runtime/vm/jit/ir-builder.h"
#include <algorithm>
#include <vector>

#include "folly/ScopeGuard.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/util/assertions.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

IRBuilder::IRBuilder(Offset initialBcOffset,
                     Offset initialSpOffsetFromFp,
                     IRUnit& unit,
                     const Func* func)
  : m_unit(unit)
  , m_simplifier(unit)
  , m_state(unit, initialSpOffsetFromFp, func)
  , m_curBlock(m_unit.entry())
  , m_enableSimplification(false)
  , m_constrainGuards(RuntimeOption::EvalHHIRRelaxGuards)
{
  m_state.setBuilding(true);
  if (RuntimeOption::EvalHHIRGenOpts) {
    m_state.setEnableCse(RuntimeOption::EvalHHIRCse);
    m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  }
}

IRBuilder::~IRBuilder() {
}

/*
 * Returns whether or not the given value might have its type relaxed by guard
 * relaxation. If tmp is null, only conditions that apply to all values are
 * checked.
 */
bool IRBuilder::typeMightRelax(SSATmp* tmp /* = nullptr */) const {
  if (!shouldConstrainGuards()) return false;
  if (tmp && (tmp->inst()->is(DefConst) || tmp->isA(Type::Cls))) return false;

  return true;
}

SSATmp* IRBuilder::genPtrToInitNull() {
  return cns(Type::cns(&init_null_variant, Type::PtrToInitNull));
}

SSATmp* IRBuilder::genPtrToUninit() {
  return cns(Type::cns(&null_variant, Type::PtrToUninit));
}

void IRBuilder::appendInstruction(IRInstruction* inst) {
  auto defaultWhere = m_curBlock->end();
  auto& where = m_curWhere ? *m_curWhere : defaultWhere;

  // If the block isn't empty, check if we need to create a new block.
  if (where != m_curBlock->begin()) {
    auto prevIt = where;
    --prevIt;
    auto& prev = *prevIt;

    if (prev.isBlockEnd()) {
      assert(where == m_curBlock->end());
      // start a new block
      m_curBlock = m_unit.defBlock();
      where = m_curBlock->begin();
      FTRACE(2, "lazily adding B{}\n", m_curBlock->id());
      if (!prev.isTerminal()) {
        // new block is reachable from old block so link it.
        prev.setNext(m_curBlock);
        m_curBlock->setHint(prev.block()->hint());
      }
    }
  }

  assert(IMPLIES(inst->isBlockEnd(), where == m_curBlock->end()) &&
         "Can't insert a BlockEnd instruction in the middle of a block");
  if (do_assert && where != m_curBlock->begin()) {
    UNUSED auto prevIt = where;
    --prevIt;
    assert(!prevIt->isBlockEnd() &&
           "Can't append an instruction after a BlockEnd instruction");
  }

  assert(inst->marker().valid());
  if (!inst->is(Nop, DefConst)) {
    where = m_curBlock->insert(where, inst);
    ++where;
  }

  if (m_savedBlocks.empty()) {
    // We don't track state on non-main traces for now. t2982555
    m_state.update(inst);
  }
}

void IRBuilder::appendBlock(Block* block) {
  assert(m_savedBlocks.empty()); // TODO(t2982555): Don't require this

  m_state.finishBlock(m_curBlock);

  FTRACE(2, "appending B{}\n", block->id());
  // Load up the state for the new block.
  m_state.startBlock(block);
  m_curBlock = block;
}

std::vector<RegionDesc::TypePred> IRBuilder::getKnownTypes() const {
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

SSATmp* IRBuilder::preOptimizeAssertTypeOp(IRInstruction* inst,
                                           Type oldType,
                                           ConstraintFunc constrain) {
  auto const newType = inst->typeParam();
  if (oldType.not(newType)) {
    // If both types are boxed this is ok and even expected as a means to
    // update the hint for the inner type.
    if (oldType.isBoxed() && newType.isBoxed()) return nullptr;

    // We got external information (probably from static analysis) that
    // conflicts with what we've built up so far. There's no reasonable way to
    // continue here: we can't properly fatal the request because we can't make
    // a catch trace or SpillStack without HhbcTranslator, we can't punt on
    // just this instruction because we might not be in the initial translation
    // phase, and we can't just plow on forward since we'll probably generate
    // malformed IR. Since this case is very rare, just punt on the whole trace
    // so it gets interpreted.
    TRACE_PUNT("Invalid AssertTypeOp");
  }

  // Asserting in these situations doesn't add any information.
  if (oldType <= Type::Cls || newType == Type::Gen) return inst->src(0);

  // We're asserting a strict subtype of the old type, so keep the assert
  // around.
  if (newType < oldType) return nullptr;

  // oldType is at least as good as the new type. Kill this assert op but
  // preserve the type we were asserting in case the source type gets relaxed
  // past it.
  if (newType >= oldType) {
    constrain({DataTypeGeneric, newType});
    return inst->src(0);
  }

  // Now we're left with cases where neither type is a subtype of the other but
  // they have some nonzero intersection. We want to end up asserting the
  // intersection, but we have to constrain the input to avoid reintroducing
  // types that were removed from the original typeParam.
  auto const intersect = newType & oldType;
  inst->setTypeParam(intersect);

  TypeConstraint tc;
  if (intersect != newType) {
    auto increment = [](DataTypeCategory& cat) {
      always_assert(cat != DataTypeSpecialized);
      cat = static_cast<DataTypeCategory>(static_cast<uint8_t>(cat) + 1);
    };

    Type relaxed;
    // Find the most general constraint that doesn't modify the type being
    // asserted.
    while ((relaxed = newType & relaxType(oldType, tc)) != intersect) {
      if (tc.category > DataTypeGeneric &&
          relaxed.maybeBoxed() && intersect.maybeBoxed() &&
          (relaxed & Type::Cell) == (intersect & Type::Cell)) {
        // If the inner type is why we failed, constrain that a level.
        increment(tc.innerCat);
      } else {
        increment(tc.category);
      }
    }
  }
  constrain(tc);

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeCheckType(IRInstruction* inst) {
  SSATmp* src  = inst->src(0);
  auto const oldType = src->type();
  auto const newType = inst->typeParam();

  if (oldType.not(newType)) {
    if (oldType.isBoxed() && newType.isBoxed()) {
      /* This CheckType serves to update the inner type hint for a boxed
       * value, which requires no runtime work.  This depends on the type being
       * boxed, and constraining it with DataTypeCountness will do it.  */
      constrainValue(src, DataTypeCountness);
      return gen(AssertType, newType, src);
    }
    /* This check will always fail. It's probably due to an incorrect
     * prediction. Generate a Jmp, and return src because
     * following instructions may depend on the output of CheckType
     * (they'll be DCEd later). Note that we can't use convertToJmp
     * because the return value isn't nullptr, so the original
     * instruction won't be inserted into the stream. */
    gen(Jmp, inst->taken());
    return src;
  }

  if (newType >= oldType) {
    /*
     * The type of the src is the same or more refined than type, so the guard
     * is unnecessary.
     */
    return src;
  }
  if (newType < oldType) {
    assert(!src->isConst());
    return nullptr;
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeCheckStk(IRInstruction* inst) {
  auto const newType = inst->typeParam();
  auto sp = inst->src(0);
  auto offset = inst->extra<CheckStk>()->offset;

  auto stkVal = getStackValue(sp, offset);
  auto const oldType = stkVal.knownType;

  if (newType < oldType) {
    // The new type is strictly better than the old type.
    return nullptr;
  }

  if (newType >= oldType) {
    // The new type isn't better than the old type.
    return sp;
  }

  if (newType.not(oldType)) {
    if (oldType.isBoxed() && newType.isBoxed()) {
      /* This CheckStk serves to update the inner type hint for a boxed
       * value, which requires no runtime work. This depends on the type being
       * boxed, and constraining it with DataTypeCountness will do it.  */
      constrainStack(sp, offset, DataTypeCountness);
      return gen(AssertStk, newType, sp);
    }
    /* This check will always fail. It's probably due to an incorrect
     * prediction. Generate a Jmp, and return the source because
     * following instructions may depend on the output of CheckStk
     * (they'll be DCEd later).  Note that we can't use convertToJmp
     * because the return value isn't nullptr, so the original
     * instruction won't be inserted into the stream. */
    gen(Jmp, inst->taken());
    return sp;
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeCheckLoc(IRInstruction* inst) {
  auto const locId = inst->extra<CheckLoc>()->locId;
  Type typeParam   = inst->typeParam();
  SSATmp* src      = inst->src(0);

  if (auto const prevValue = localValue(locId, DataTypeGeneric)) {
    return gen(CheckType, typeParam, inst->taken(), prevValue);
  }

  auto const prevType = localType(locId, DataTypeSpecific);

  if (prevType <= typeParam) {
    return src;
  }

  if (prevType.not(typeParam)) {
    if (typeParam.isBoxed() && prevType.isBoxed()) {
      /* When both types are non-intersecting boxed types, we're just
       * updating the inner type hint. This requires no runtime work. */
      constrainLocal(locId, DataTypeCountness, "preOptimizeCheckLoc");
      return gen(AssertLoc, LocalId(locId), typeParam, src);
    }
    /* This check will always fail. It's probably due to an incorrect
     * prediction. Generate a Jmp, and return the source because
     * following instructions may depend on the output of CheckLoc
     * (they'll be DCEd later).  Note that we can't use convertToJmp
     * because the return value isn't nullptr, so the original
     * instruction won't be inserted into the stream. */
    gen(Jmp, inst->taken());
    return src;
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeAssertLoc(IRInstruction* inst) {
  auto const locId = inst->extra<AssertLoc>()->locId;

  if (auto const prevValue = localValue(locId, DataTypeGeneric)) {
    return gen(AssertType, inst->typeParam(), prevValue);
  }

  return preOptimizeAssertTypeOp(
    inst, localType(locId, DataTypeGeneric), [&](TypeConstraint tc) {
      constrainLocal(locId, tc, "preOptimizeAssertLoc");
    }
  );
}

SSATmp* IRBuilder::preOptimizeAssertType(IRInstruction* inst) {
  auto const src = inst->src(0);

  return preOptimizeAssertTypeOp(inst, src->type(), [&](TypeConstraint tc) {
    constrainValue(src, tc);
  });
}

SSATmp* IRBuilder::preOptimizeAssertStk(IRInstruction* inst) {
  auto const idx = inst->extra<AssertStk>()->offset;
  auto const info = getStackValue(inst->src(0), idx);

  return preOptimizeAssertTypeOp(inst, info.knownType,
    [&](TypeConstraint tc) {
      constrainStack(inst->src(0), idx, tc);
    }
  );
}

SSATmp* IRBuilder::preOptimizeLdThis(IRInstruction* inst) {
  if (m_state.thisAvailable()) {
    auto fpInst = frameRoot(inst->src(0)->inst());

    if (fpInst->is(DefInlineFP)) {
      if (!m_state.frameSpansCall()) { // check that we haven't nuked the SSATmp
        auto spInst = findSpillFrame(fpInst->src(0));
        // In an inlined call, we should always be able to find our SpillFrame.
        always_assert(spInst && spInst->src(0) == fpInst->src(1));
        if (spInst->src(3)->isA(Type::Obj)) {
          return spInst->src(3);
        }
      }
    }
    inst->setTaken(nullptr);
  }
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdCtx(IRInstruction* inst) {
  if (m_state.thisAvailable()) return gen(LdThis, m_state.fp());
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeDecRefThis(IRInstruction* inst) {
  /*
   * If $this is available, convert to an instruction sequence that
   * doesn't need to test if it's already live.
   */
  if (thisAvailable()) {
    auto const thiss = gen(LdThis, m_state.fp());
    gen(DecRef, thiss);
    inst->convertToNop();
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeDecRefLoc(IRInstruction* inst) {
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
    inst->setTypeParam(std::min(knownType, inst->typeParam()));
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdLoc(IRInstruction* inst) {
  auto const locId = inst->extra<LdLoc>()->locId;
  if (auto tmp = localValue(locId, DataTypeGeneric)) {
    return tmp;
  }

  auto const type = localType(locId, DataTypeGeneric);
  // If FrameState's type isn't as good as the type param, we're missing
  // information in the IR.
  assert(inst->typeParam() >= type);
  inst->setTypeParam(std::min(type, inst->typeParam()));
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdLocAddr(IRInstruction* inst) {
  auto const locId = inst->extra<LdLocAddr>()->locId;
  auto const type = localType(locId, DataTypeGeneric);
  assert(inst->typeParam().deref() >= type);
  inst->setTypeParam(std::min(type.ptr(), inst->typeParam()));
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeStLoc(IRInstruction* inst) {
  // Guard relaxation might change the current local type, so don't try to
  // change to StLocNT until after relaxation happens.
  if (typeMightRelax()) return nullptr;

  auto locId = inst->extra<StLoc>()->locId;
  auto const curType = localType(locId, DataTypeGeneric);
  auto const newType = inst->src(1)->type();

  assert(!inst->hasTypeParam());

  /*
   * There's no need to store the type if it's going to be the same
   * KindOfFoo.  We'll still have to store string types because we
   * aren't specific about storing KindOfStaticString
   * vs. KindOfString, and a Type::Null might mean KindOfUninit or
   * KindOfNull.
   */
  auto const bothBoxed = curType.isBoxed() && newType.isBoxed();
  auto const sameUnboxed = [&] {
    auto avoidable = { Type::Uninit,
                       Type::InitNull,
                       Type::Bool,
                       Type::Int,
                       Type::Dbl,
                       // No strings.
                       Type::Arr,
                       Type::Obj,
                       Type::Res };
    for (auto t : avoidable) {
      if (curType <= t && newType <= t) return true;
    }
    return false;
  };
  if (bothBoxed || sameUnboxed()) {
    inst->setOpcode(StLocNT);
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimize(IRInstruction* inst) {
#define X(op) case op: return preOptimize##op(inst)
  switch (inst->op()) {
    X(CheckType);
    X(CheckStk);
    X(CheckLoc);
    X(AssertLoc);
    X(AssertStk);
    X(AssertType);
    X(LdThis);
    X(LdCtx);
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

/*
 * Performs simplification and CSE on the input instruction. If the input
 * instruction has a dest, this will return an SSATmp that represents the same
 * value as dst(0) of the input instruction. If the input instruction has no
 * dest, this will return nullptr.
 *
 * The caller never needs to clone or append; all this has been done.
 */
SSATmp* IRBuilder::optimizeInst(IRInstruction* inst,
                                CloneFlag doClone,
                                const folly::Optional<IdomVector>& idoms) {
  static DEBUG_ONLY __thread int instNest = 0;
  if (debug) ++instNest;
  SCOPE_EXIT { if (debug) --instNest; };
  DEBUG_ONLY auto indent = [&] { return std::string(instNest * 2, ' '); };

  auto doCse = [&] (IRInstruction* cseInput) -> SSATmp* {
    if (m_state.enableCse() && cseInput->canCSE()) {
      SSATmp* cseResult = m_state.cseLookup(cseInput, idoms);
      if (cseResult) {
        // Found a dominating instruction that can be used instead of input
        FTRACE(1, "  {}cse found: {}\n",
               indent(), cseResult->inst()->toString());

        assert(!cseInput->consumesReferences());
        if (cseInput->producesReference(0)) {
          // Replace with an IncRef
          FTRACE(1, "  {}cse of refcount-producing instruction\n", indent());
          gen(IncRef, cseResult);
        }
        return cseResult;
      }
    }
    return nullptr;
  };

  auto cloneAndAppendOriginal = [&] () -> SSATmp* {
    if (inst->op() == Nop) return nullptr;
    if (auto cseResult = doCse(inst)) {
      return cseResult;
    }
    if (doClone == CloneFlag::Yes) {
      inst = m_unit.cloneInstruction(inst);
    }
    appendInstruction(inst);
    return inst->dst(0);
  };

  // Since some of these optimizations inspect tracked state, we don't
  // perform any of them on non-main traces.
  if (m_savedBlocks.size() > 0) return cloneAndAppendOriginal();

  // copy propagation on inst source operands
  copyProp(inst);

  // First pass of IRBuilder optimizations try to replace an
  // instruction based on tracked state before we do anything else.
  // May mutate the IRInstruction in place (and return nullptr) or
  // return an SSATmp*.
  if (SSATmp* preOpt = preOptimize(inst)) {
    FTRACE(1, "  {}preOptimize returned: {}\n",
           indent(), preOpt->inst()->toString());
    return preOpt;
  }
  if (inst->op() == Nop) return cloneAndAppendOriginal();

  if (!m_enableSimplification) {
    return cloneAndAppendOriginal();
  }

  auto simpResult = m_simplifier.simplify(inst, shouldConstrainGuards());

  // These are the possible outputs:
  //
  // ([], nullptr): no optimization possible. Use original inst.
  //
  // ([], non-nullptr): passing through a src. Don't CSE.
  //
  // ([X, ...], Y): throw away input instruction, append 'X, ...' (CSEing
  //                as we go), return Y.

  if (!simpResult.instrs.empty()) {
    // New instructions were generated. Append the new ones, filtering out Nops.
    for (auto* newInst : simpResult.instrs) {
      assert(!newInst->isTransient());
      if (newInst->op() == Nop) continue;

      auto cseResult = doCse(newInst);
      if (cseResult) {
        appendInstruction(m_unit.mov(newInst->dst(), cseResult,
                                     newInst->marker()));
      } else {
        appendInstruction(newInst);
      }
    }

    return simpResult.dst;
  }

  // No new instructions were generated. Either simplification didn't do
  // anything, or we're using some other instruction's dst instead of our own.

  if (simpResult.dst) {
    // We're using some other instruction's output. Don't append anything, and
    // don't do any CSE.
    assert(simpResult.dst->inst() != inst);
    return simpResult.dst;
  }

  // No simplification happened.
  return cloneAndAppendOriginal();
}

/*
 * reoptimize() runs a trace through a second pass of IRBuilder
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
void IRBuilder::reoptimize() {
  Timer _t(Timer::optimize_reoptimize);
  FTRACE(5, "ReOptimize:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "ReOptimize:^^^^^^^^^^^^^^^^^^^^\n"); };
  always_assert(m_savedBlocks.empty());
  always_assert(!m_curWhere);
  always_assert(m_state.inlineDepth() == 0);

  m_state.setEnableCse(RuntimeOption::EvalHHIRCse);
  m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  if (!m_state.enableCse() && !m_enableSimplification) return;
  setConstrainGuards(false);
  if (RuntimeOption::EvalHHIRBytecodeControlFlow) {
    m_state.setBuilding(false);
  }

  auto blocksIds = rpoSortCfgWithIds(m_unit);
  auto const idoms = findDominators(m_unit, blocksIds);
  m_state.clear();

  for (auto* block : blocksIds.blocks) {
    FTRACE(5, "Block: {}\n", block->id());

    m_state.startBlock(block);
    m_curBlock = block;

    auto nextBlock = block->next();
    auto backMarker = block->back().marker();
    auto instructions = block->moveInstrs();
    assert(block->empty());
    while (!instructions.empty()) {
      auto* inst = &instructions.front();
      instructions.pop_front();

      // merging state looks at the current marker, and optimizeWork
      // below may create new instructions. Use the marker from this
      // instruction.
      assert(inst->marker().valid());
      setMarker(inst->marker());

      auto const tmp = optimizeInst(inst, CloneFlag::No, idoms);
      SSATmp* dst = inst->dst(0);

      if (dst != tmp) {
        // The result of optimization has a different destination than the inst.
        // Generate a mov(tmp->dst) to get result into dst. If we get here then
        // assume the last instruction in the block isn't a guard. If it was,
        // we would have to insert the mov on the fall-through edge.
        assert(inst->op() != DefLabel);
        assert(block->empty() || !block->back().isBlockEnd());
        appendInstruction(m_unit.mov(dst, tmp, inst->marker()));
      }

      if (inst->block() == nullptr && inst->isBlockEnd()) {
        // We're not re-adding the block-end instruction. Unset its edges.
        inst->setTaken(nullptr);
        inst->setNext(nullptr);
      }
    }

    if (block->empty() || !block->back().isBlockEnd()) {
      // Our block-end instruction was eliminated (most likely a Jmp* converted
      // to a nop). Replace it with a jump to the next block.
      appendInstruction(m_unit.gen(Jmp, backMarker, nextBlock));
    }
    assert(block->back().isBlockEnd());
    if (!block->back().isTerminal() && !block->next()) {
      // We converted the block-end instruction to a different one.
      // Set its next block appropriately.
      block->back().setNext(nextBlock);
    }

    m_state.finishBlock(block);
  }
}

/*
 * Returns true iff a guard to constrain was found, and tc was more specific
 * than the guard's existing constraint. Note that this doesn't necessarily
 * mean that the guard was constrained: tc.weak might be true.
 */
bool IRBuilder::constrainGuard(IRInstruction* inst, TypeConstraint tc) {
  if (!shouldConstrainGuards()) return false;

  auto& guard = m_guardConstraints[inst];
  auto changed = false;
  auto const assertFits = typeFitsConstraint(guard.assertedType, tc);
  FTRACE(2, "constrainGuard({}, {}): existing constraint {}, assertFits: {}\n",
         *inst, tc, guard, assertFits ? "true" : "false");

  // For category and innerCat, constrain the guard if the assertedType isn't
  // strong enough to fit what we want and tc is more specific than the
  // existing category.

  if (!assertFits && tc.innerCat > guard.innerCat) {
    if (!tc.weak) {
      FTRACE(1, "constraining inner type of {}: {} -> {}\n",
             *inst, guard.innerCat, tc.innerCat);
      guard.innerCat = tc.innerCat;
    }
    changed = true;
  } else {
    FTRACE(2, "not constraining innerCat\n");
  }

  if (!assertFits && tc.category > guard.category) {
    if (!tc.weak) {
      FTRACE(1, "constraining {}: {} -> {}\n",
             *inst, guard.category, tc.category);
      guard.category = tc.category;
    }
    changed = true;
  } else {
    FTRACE(2, "not constraining category\n");
  }

  // It's fairly common to have a local that we've asserted to be Obj, and then
  // later assert that it's Obj<C>|InitNull. We want to use their intersection,
  // so in this case we'd assert Obj<C>.
  always_assert(tc.assertedType.maybe(guard.assertedType));
  auto assertCommon = tc.assertedType & guard.assertedType;
  if (assertCommon < guard.assertedType) {
    // We don't check tc.weak here because assertedType is supposed to be
    // statically known type information.
    FTRACE(1, "using {} to refine assertedType of {}: {} -> {}\n",
           tc.assertedType, *inst, guard.assertedType, assertCommon);
    guard.assertedType = assertCommon;
  } else {
    FTRACE(2, "not refining assertedType\n");
  }

  return changed;
}

/**
 * Trace back to the guard that provided the type of val, if
 * any. Constrain it so its type will not be relaxed beyond the given
 * DataTypeCategory. Returns true iff one or more guard instructions
 * were constrained.
 */
bool IRBuilder::constrainValue(SSATmp* const val,
                               TypeConstraint tc) {
  if (!shouldConstrainGuards()) return false;

  if (!val) {
    FTRACE(1, "constrainValue(nullptr, {}), bailing\n", tc);
    return false;
  }

  FTRACE(1, "constrainValue({}, {})\n", *val->inst(), tc);

  auto inst = val->inst();
  if (inst->is(LdLoc, LdLocAddr)) {
    // We've hit a LdLoc(Addr). If the source of the value is non-null and not
    // a FramePtr, it's a real value that was killed by a Call. The value won't
    // be live but it's ok to use it to track down the guard.

    auto source = inst->extra<LocalData>()->typeSrc;
    if (!source) {
      // val was newly created in this trace. Nothing to constrain.
      FTRACE(2, "  - typeSrc is null, bailing\n");
      return false;
    }

    // If valSrc is a FramePtr, it represents the frame the value was
    // originally loaded from. Look for the guard for this local.
    if (source->isA(Type::FramePtr)) {
      return constrainLocal(inst->extra<LocalId>()->locId, source, tc,
                            "constrainValue");
    }

    // Otherwise, keep chasing down the source of val.
    return constrainValue(source, tc);
  } else if (inst->is(LdStack, LdStackAddr)) {
    return constrainStack(inst->src(0), inst->extra<StackOffset>()->offset,
                          tc);
  } else if (inst->is(CheckType, AssertType)) {
    // If the dest type of the instruction fits the constraint we want, we can
    // stop here without constraining any further. Otherwise, continue through
    // to the source.
    auto changed = false;
    if (inst->is(CheckType)) changed = constrainGuard(inst, tc) || changed;

    auto dstType = inst->typeParam();
    if (!typeFitsConstraint(dstType, tc.category)) {
      changed = constrainValue(inst->src(0), tc) || changed;
    }
    return changed;
  } else if (inst->is(StRef)) {
    // StRef requires that src(0) is boxed so we're relying on callers to
    // appropriately constrain the values they pass to it. Any innerCat in tc
    // should be applied to the value being stored.

    tc.category = tc.innerCat;
    tc.innerCat = DataTypeGeneric;
    tc.assertedType = Type::Gen;
    return constrainValue(inst->src(1), tc);
  } else if (inst->is(Box, BoxPtr, Unbox, UnboxPtr)) {
    // All Box/Unbox opcodes are similar to StRef/LdRef in some situations and
    // Mov in others (determined at runtime), so we need to constrain both
    // outer and inner.

    auto maxCat = std::max(tc.category, tc.innerCat);
    tc.category = maxCat;
    tc.innerCat = maxCat;
    tc.assertedType = Type::Gen;
    return constrainValue(inst->src(0), tc);
  } else if (inst->is(LdRef)) {
    // Like StRef, we're relying on the caller to have appropriately
    // constrained the outer type of the box. Constrain the inner type of the
    // box with tc.

    tc.innerCat = tc.category;
    tc.category = DataTypeGeneric;
    tc.assertedType = Type::Gen;
    return constrainValue(inst->src(0), tc);
  } else if (inst->isPassthrough()) {
    return constrainValue(inst->getPassthroughValue(), tc);
  } else {
    // Any instructions not special cased above produce a new value, so
    // there's no guard for us to constrain.
    FTRACE(2, "  - value is new in this trace, bailing\n");
    return false;
  }
  // TODO(t2598894): Should be able to do something with LdMem<T> here
}

bool IRBuilder::constrainLocal(uint32_t locId, TypeConstraint tc,
                               const std::string& why) {
  if (!shouldConstrainGuards()) return false;

  return constrainLocal(locId, localTypeSource(locId), tc, why);
}

bool IRBuilder::constrainLocal(uint32_t locId, SSATmp* valSrc,
                               TypeConstraint tc,
                               const std::string& why) {
  if (!shouldConstrainGuards()) return false;

  FTRACE(1, "constrainLocal({}, {}, {}, {})\n",
         locId, valSrc ? valSrc->inst()->toString() : "null", tc, why);

  if (!valSrc) return false;
  if (!valSrc->isA(Type::FramePtr)) {
    return constrainValue(valSrc, tc);
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
      if (typeFitsConstraint(guard->typeParam(), tc.category)) return false;
      guard = guardForLocal(locId, guard->src(0));
    } else {
      assert(guard->is(GuardLoc, CheckLoc));
      FTRACE(2, "    - found guard to constrain\n");
      return constrainGuard(guard, tc);
    }
  }

  FTRACE(2, "    - no guard to constrain\n");
  return false;
}

bool IRBuilder::constrainStack(int32_t idx, TypeConstraint tc) {
  if (!shouldConstrainGuards()) return false;

  return constrainStack(sp(), idx, tc);
}

bool IRBuilder::constrainStack(SSATmp* sp, int32_t idx,
                               TypeConstraint tc) {
  if (!shouldConstrainGuards()) return false;

  FTRACE(1, "constrainStack({}, {}, {})\n", *sp->inst(), idx, tc);
  assert(sp->isA(Type::StkPtr));

  // We've hit a LdStack. If getStackValue gives us a value, recurse on
  // that. Otherwise, look at the instruction that gave us the type of the
  // stack element. If it's a GuardStk or CheckStk, it's our target. If it's
  // anything else, the value is new so there's no guard to relax.
  auto stackInfo = getStackValue(sp, idx);
  if (stackInfo.value) {
    FTRACE(1, "  - value = {}\n", *stackInfo.value->inst());
    return constrainValue(stackInfo.value, tc);
  } else {
    auto typeSrc = stackInfo.typeSrc;
    FTRACE(1, "  - typeSrc = {}\n", *typeSrc);
    return typeSrc->is(GuardStk, CheckStk) && constrainGuard(typeSrc, tc);
  }
}

Type IRBuilder::localType(uint32_t id, TypeConstraint tc) {
  constrainLocal(id, tc, "localType");
  return m_state.localType(id);
}

SSATmp* IRBuilder::localValue(uint32_t id, TypeConstraint tc) {
  constrainLocal(id, tc, "localValue");
  return m_state.localValue(id);
}

void IRBuilder::setMarker(BCMarker marker) {
  auto const oldMarker = m_state.marker();

  if (marker == oldMarker) return;
  FTRACE(2, "IRBuilder changing current marker from {} to {}\n",
         oldMarker.func ? oldMarker.show() : "<invalid>", marker.show());
  assert(marker.valid());
  m_state.setMarker(marker);
}

void IRBuilder::startBlock() {
  assert(m_savedBlocks.empty());  // No bytecode control flow in exits.
  auto marker = m_state.marker();
  auto it = m_offsetToBlockMap.find(marker.bcOff);
  if (it != m_offsetToBlockMap.end() && it->second->empty()) {
    auto block = it->second;
    if (block != m_curBlock) {
      if (m_state.compatible(block)) {
        m_state.pauseBlock(block);
      } else {
        m_state.clearCse();
      }
      assert(m_curBlock);
      auto& prev = m_curBlock->back();
      if (!prev.isTerminal()) {
        prev.setNext(block);
      }
      m_curBlock = block;
      m_state.startBlock(m_curBlock);
      FTRACE(2, "IRBuilder switching to block B{}: {}\n", block->id(),
             show(m_state));
    }
  }
}

Block* IRBuilder::makeBlock(Offset offset) {
  auto it = m_offsetToBlockMap.find(offset);
  if (it == m_offsetToBlockMap.end()) {
    auto* block = m_unit.defBlock();
    m_offsetToBlockMap.insert(std::make_pair(offset, block));
    return block;
  }
  return it->second;
}

bool IRBuilder::blockExists(Offset offset) {
  return m_offsetToBlockMap.count(offset);
}

bool IRBuilder::blockIsIncompatible(Offset offset) {
  if (m_offsetSeen.count(offset)) return true;
  auto it = m_offsetToBlockMap.find(offset);
  if (it == m_offsetToBlockMap.end()) return false;
  auto* block = it->second;
  if (!it->second->empty()) return true;
  return !m_state.compatible(block);
}

void IRBuilder::recordOffset(Offset offset) {
  m_offsetSeen.insert(offset);
}

void IRBuilder::pushBlock(BCMarker marker, Block* b,
                          const folly::Optional<Block::iterator>& where) {
  FTRACE(2, "IRBuilder saving {}@{} and using {}@{}\n",
         m_curBlock, m_state.marker().show(), b, marker.show());
  assert(b);

  m_savedBlocks.push_back(
    BlockState{ m_curBlock, m_state.marker(), m_curWhere });
  m_state.pauseBlock(m_curBlock);
  m_state.startBlock(b);
  m_curBlock = b;
  setMarker(marker);
  m_curWhere = where ? where : b->end();

  if (do_assert) {
    for (UNUSED auto const& state : m_savedBlocks) {
      assert(state.block != b &&
             "Can't push a block that's already in the saved stack");
    }
  }
}

void IRBuilder::popBlock() {
  assert(!m_savedBlocks.empty());

  auto const& top = m_savedBlocks.back();
  FTRACE(2, "IRBuilder popping {}@{} to restore {}@{}\n",
         m_curBlock, m_state.marker().show(), top.block, top.marker.show());
  m_state.pauseBlock(m_curBlock);
  m_state.startBlock(top.block);
  m_curBlock = top.block;
  setMarker(top.marker);
  m_curWhere = top.where;
  m_savedBlocks.pop_back();
}

}}
