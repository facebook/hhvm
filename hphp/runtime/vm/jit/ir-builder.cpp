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
#include <utility>

#include <folly/ScopeGuard.h>

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/util/assertions.h"

namespace HPHP { namespace jit {

namespace {

TRACE_SET_MOD(hhir);
using Trace::Indent;

//////////////////////////////////////////////////////////////////////

template<typename M>
const typename M::mapped_type& get_required(const M& m,
                                            typename M::key_type key) {
  auto it = m.find(key);
  always_assert(it != m.end());
  return it->second;
}

SSATmp* fwdGuardSource(IRInstruction* inst) {
  if (inst->is(AssertType, CheckType, AssertStk, CheckStk)) return inst->src(0);

  assert(inst->is(AssertLoc, CheckLoc));
  inst->convertToNop();
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

}

IRBuilder::IRBuilder(IRUnit& unit, BCMarker initMarker)
  : m_unit(unit)
  , m_initialMarker(initMarker)
  , m_curMarker(initMarker)
  , m_state(m_unit, initMarker)
  , m_curBlock(m_unit.entry())
  , m_enableSimplification(false)
  , m_constrainGuards(shouldHHIRRelaxGuards())
{
  m_state.setBuilding();
  if (RuntimeOption::EvalHHIRGenOpts) {
    m_enableCse = false; // CSE is disabled at gen time.
    m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  }
}

/*
 * Returns whether or not the given value might have its type relaxed by guard
 * relaxation. If tmp is null, only conditions that apply to all values are
 * checked.
 */
bool IRBuilder::typeMightRelax(SSATmp* tmp /* = nullptr */) const {
  return shouldConstrainGuards() && jit::typeMightRelax(tmp);
}

SSATmp* IRBuilder::genPtrToInitNull() {
  // Nothing is allowed to write anything to the init null variant, so this
  // inner type is always true.
  return cns(Type::cns(&init_null_variant, Type::PtrToMembInitNull));
}

SSATmp* IRBuilder::genPtrToUninit() {
  // Nothing can write to the uninit null variant either, so the inner type
  // here is also always true.
  return cns(Type::cns(&null_variant, Type::PtrToMembUninit));
}

void IRBuilder::appendInstruction(IRInstruction* inst) {
  FTRACE(1, "  append {}\n", inst->toString());

  if (shouldConstrainGuards()) {
    // If we're constraining guards, some instructions need certain information
    // to be recorded in side tables.
    if (inst->is(AssertLoc, CheckLoc, LdLoc)) {
      auto const locId = inst->extra<LocalId>()->locId;
      m_constraints.typeSrcs[inst] = localTypeSources(locId);
      if (inst->is(AssertLoc, CheckLoc)) {
        m_constraints.prevTypes[inst] = localType(locId, DataTypeGeneric);
      }
    }

    // And a LdRef or CheckRefInner automatically constrains the value to be a
    // boxed cell, specifically.
    if (inst->is(LdRef, CheckRefInner)) {
      constrainValue(inst->src(0), DataTypeSpecific);
    }

    if (inst->marker().func()->isPseudoMain() &&
        inst->is(GuardLoc, CheckLoc)) {
      constrainGuard(inst, DataTypeSpecific);
    }
  }

  auto where = m_curBlock->end();

  // If the block isn't empty, check if we need to create a new block.
  if (where != m_curBlock->begin()) {
    auto prevIt = where;
    --prevIt;
    auto& prev = *prevIt;

    if (prev.isBlockEnd()) {
      assert(where == m_curBlock->end());

      auto oldBlock = m_curBlock;

      // First make the inst's next block, so we can save state to it in
      // finishBlock.
      m_curBlock = m_unit.defBlock();
      if (!prev.isTerminal()) {
        // New block is reachable from old block so link it.
        prev.setNext(m_curBlock);
        m_curBlock->setHint(prev.block()->hint());
      }

      m_state.finishBlock(oldBlock);

      m_state.startBlock(m_curBlock, inst->marker());
      where = m_curBlock->begin();

      FTRACE(2, "lazily adding B{}\n", m_curBlock->id());

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

  m_state.update(inst);
  cseUpdate(*inst);
}

void IRBuilder::appendBlock(Block* block) {
  m_state.finishBlock(m_curBlock);

  FTRACE(2, "appending B{}\n", block->id());
  // Load up the state for the new block.
  m_state.startBlock(block, m_curMarker);
  m_curBlock = block;
}

//////////////////////////////////////////////////////////////////////

SSATmp* IRBuilder::preOptimizeCheckTypeOp(IRInstruction* inst,
                                          Type oldType,
                                          ConstrainBoxedFunc constrainBoxed) {
  auto const typeParam = inst->typeParam();

  if (oldType.isBoxed() && typeParam.isBoxed() &&
      (oldType.not(typeParam) || typeParam < oldType)) {
    // TODO(#2939547): this used to update inner type predictions, but now it's
    // really just serving to change BoxedCell into BoxedInitCell in some
    // cases.  We should make sure all boxed values are BoxedInitCell subtypes
    // and then remove this code.
    return constrainBoxed(typeParam);
  }

  if (oldType.not(typeParam)) {
    /* This check will always fail. It's probably due to an incorrect
     * prediction. Generate a Jmp and return the src. The fact that the type
     * will be slightly off is ok because all the code after the Jmp is
     * unreachable. */
    gen(Jmp, inst->taken());
    return fwdGuardSource(inst);
  }

  auto const newType = refineType(oldType, inst->typeParam());

  if (oldType <= newType) {
    /* The type of the src is the same or more refined than type, so the guard
     * is unnecessary. */
    return fwdGuardSource(inst);
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeCheckType(IRInstruction* inst) {
  auto* src = inst->src(0);

  return preOptimizeCheckTypeOp(
    inst, src->type(),
    [&](Type newType){
      constrainValue(src, DataTypeCountness);
      return gen(AssertType, newType, src);
    });
}

SSATmp* IRBuilder::preOptimizeCheckStk(IRInstruction* inst) {
  auto sp = inst->src(0);
  auto offset = inst->extra<CheckStk>()->offset;
  auto const stackInfo = getStackValue(sp, offset);

  return preOptimizeCheckTypeOp(
    inst, stackInfo.knownType,
    [&](Type newType) {
      constrainStack(sp, offset, DataTypeCountness);
      return gen(AssertStk, newType, StackOffset(offset), sp);
  });
}

SSATmp* IRBuilder::preOptimizeCheckLoc(IRInstruction* inst) {
  auto const locId = inst->extra<CheckLoc>()->locId;

  if (auto const prevValue = localValue(locId, DataTypeGeneric)) {
    gen(CheckType, inst->typeParam(), inst->taken(), prevValue);
    inst->convertToNop();
    return nullptr;
  }

  return preOptimizeCheckTypeOp(
    inst, localType(locId, DataTypeGeneric),
    [&](Type newType) {
      constrainLocal(locId, DataTypeCountness, "preOptimizeCheckLoc");
      return gen(AssertLoc, newType, LocalId(locId), inst->src(0));
    });
}

SSATmp* IRBuilder::preOptimizeHintLocInner(IRInstruction* inst) {
  auto const locId = inst->extra<HintLocInner>()->locId;
  if (!localType(locId, DataTypeGeneric).subtypeOf(Type::BoxedCell) ||
      predictedInnerType(locId).box() <= inst->typeParam()) {
    inst->convertToNop();
    return nullptr;
  }
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeAssertTypeOp(IRInstruction* inst,
                                           const Type oldType,
                                           SSATmp* oldVal,
                                           const IRInstruction* typeSrc) {
  ITRACE(3, "preOptimizeAssertTypeOp({}, {}, {}, {})\n",
         *inst, oldType,
         oldVal ? oldVal->toString() : "nullptr",
         typeSrc ? typeSrc->toString() : "nullptr");
  auto const typeParam = inst->typeParam();

  if (oldType.not(typeParam)) {
    // If both types are boxed this is ok and even expected as a means to
    // update the hint for the inner type.
    if (oldType.isBoxed() && typeParam.isBoxed()) return nullptr;

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
  if (oldType <= Type::Cls || typeParam == Type::Gen) return inst->src(0);

  auto const newType = refineType(oldType, typeParam);

  if (oldType <= newType) {
    // oldType is at least as good as the new type. Eliminate this
    // AssertTypeOp, but only if the src type won't relax, or the source value
    // is another assert that's good enough. We do this to avoid eliminating
    // apparently redundant assert opcodes that may become useful after prior
    // guards are relaxed.
    if (!typeMightRelax(oldVal) ||
        (typeSrc && typeSrc->is(AssertType, AssertLoc, AssertStk) &&
         typeSrc->typeParam() <= inst->typeParam())) {
      return fwdGuardSource(inst);
    }

    if (oldType < newType) {
      // This can happen because of limitations in how Type::operator& (used in
      // refinedType()) handles specialized types: sometimes it returns a Type
      // that's wider than it could be. It shouldn't affect correctness but it
      // can cause us to miss out on some perf.
      ITRACE(1, "Suboptimal AssertTypeOp: refineType({}, {}) -> {} in {}\n",
             oldType, typeParam, newType, *inst);

      // We don't currently support intersecting RepoAuthType::Arrays
      // (t4473238), so we might be here because oldType and typeParam have
      // different RATArrays. If that's the case and typeParam provides no
      // other useful information we can unconditionally eliminate this
      // instruction: RATArrays never come from guards so we can't miss out on
      // anything by doing so.
      if (oldType < Type::Arr && oldType.getArrayType() &&
          typeParam < Type::Arr && typeParam.getArrayType() &&
          !typeParam.hasArrayKind()) {
        return fwdGuardSource(inst);
      }
    }
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeAssertType(IRInstruction* inst) {
  auto const src = inst->src(0);
  return preOptimizeAssertTypeOp(inst, src->type(), src, src->inst());
}

SSATmp* IRBuilder::preOptimizeAssertStk(IRInstruction* inst) {
  auto const prevSp = inst->src(0);
  auto const idx = inst->extra<AssertStk>()->offset;
  auto const stackInfo = getStackValue(prevSp, idx);

  return preOptimizeAssertTypeOp(
    inst,
    stackInfo.knownType,
    stackInfo.value,
    stackInfo.typeSrc
  );
}

static const IRInstruction* singleTypeSrcInst(const TypeSourceSet& typeSrcs) {
  if (typeSrcs.size() == 1) {
    auto typeSrc = *typeSrcs.begin();
    return typeSrc.isValue() ? typeSrc.value->inst() :
           typeSrc.isGuard() ? typeSrc.guard :
                               nullptr;
  }
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeAssertLoc(IRInstruction* inst) {
  auto const locId = inst->extra<AssertLoc>()->locId;

  if (auto const prevValue = localValue(locId, DataTypeGeneric)) {
    gen(AssertType, inst->typeParam(), prevValue);
    inst->convertToNop();
    return nullptr;
  }

  // If the local has a single type-source instruction, pass it along
  // to preOptimizeAssertTypeOp, which may be able to use it to
  // optimize away the AssertLoc.
  auto const typeSrcInst = singleTypeSrcInst(localTypeSources(locId));

  return preOptimizeAssertTypeOp(
    inst,
    localType(locId, DataTypeGeneric),
    localValue(locId, DataTypeGeneric),
    typeSrcInst
  );
}

SSATmp* IRBuilder::preOptimizeCheckCtxThis(IRInstruction* inst) {
  if (m_state.thisAvailable()) inst->convertToNop();
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdCtx(IRInstruction* inst) {
  auto const fpInst = inst->src(0)->inst();
  if (fpInst->is(DefInlineFP)) {
    // TODO(#5623596): this optimization required for correctness in refcount
    // opts right now.
    // check that we haven't nuked the SSATmp
    if (!m_state.frameMaySpanCall()) {
      auto spInst = findSpillFrame(fpInst->src(0));
      // In an inlined call, we should always be able to find our SpillFrame.
      always_assert(spInst && spInst->src(0) == fpInst->src(1));
      if (spInst->src(2)->isA(Type::Obj)) {
        return spInst->src(2);
      }
    }
  }
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeDecRefThis(IRInstruction* inst) {
  /*
   * If $this is available, convert to an instruction sequence that doesn't
   * need to test $this is available.  (Hopefully we CSE the load, too.)
   */
  if (thisAvailable()) {
    auto const ctx   = gen(LdCtx, m_state.fp());
    auto const thiss = gen(CastCtxThis, ctx);
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

  if (typeMightRelax()) return nullptr;

  inst->setTypeParam(std::min(knownType, inst->typeParam()));
  if (inst->typeParam().notCounted()) {
    inst->convertToNop();
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdLoc(IRInstruction* inst) {
  auto const locId = inst->extra<LdLoc>()->locId;
  if (auto tmp = localValue(locId, DataTypeGeneric)) return tmp;

  auto const type = localType(locId, DataTypeGeneric);

  // The types may not be compatible in the presence of unreachable code.
  // Don't try to optimize the code in this case, and just let
  // unreachable code elimination take care of it later.
  if (type.not(inst->typeParam())) return nullptr;

  // If FrameStateMgr's type isn't as good as the type param, we're missing
  // information in the IR.
  assert(inst->typeParam() >= type);
  inst->setTypeParam(std::min(type, inst->typeParam()));
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeStLoc(IRInstruction* inst) {
  // Guard relaxation might change the current local type, so don't try to
  // change to StLocNT until after relaxation happens.
  if (typeMightRelax()) return nullptr;

  auto locId = inst->extra<StLoc>()->locId;
  auto const curType = localType(locId, DataTypeGeneric);
  auto const newType = inst->src(1)->type();

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
    X(HintLocInner);
    X(AssertLoc);
    X(AssertStk);
    X(AssertType);
    X(CheckCtxThis);
    X(LdCtx);
    X(DecRefThis);
    X(DecRefLoc);
    X(LdLoc);
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
                                Block* srcBlock,
                                const folly::Optional<IdomVector>& idoms) {
  static DEBUG_ONLY __thread int instNest = 0;
  if (debug) ++instNest;
  SCOPE_EXIT { if (debug) --instNest; };
  DEBUG_ONLY auto indent = [&] { return std::string(instNest * 2, ' '); };

  FTRACE(1, "optimize: {}\n", inst->toString());

  auto doCse = [&] (IRInstruction* cseInput) -> SSATmp* {
    if (m_enableCse && cseInput->canCSE()) {
      if (auto const cseResult = cseLookup(*cseInput, srcBlock, idoms)) {
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
  if (auto const preOpt = preOptimize(inst)) {
    FTRACE(1, "  {}preOptimize returned: {}\n",
           indent(), preOpt->inst()->toString());
    return preOpt;
  }
  if (inst->op() == Nop) return cloneAndAppendOriginal();

  if (!m_enableSimplification) {
    return cloneAndAppendOriginal();
  }

  auto const simpResult = simplify(m_unit, inst, shouldConstrainGuards());

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

void IRBuilder::prepareForNextHHBC() {
  assert(
    spOffset() + m_state.evalStack().size() - m_state.stackDeficit() ==
    m_curMarker.spOff()
  );
  m_exnStack = ExnStackState {
    m_state.stackDeficit(),
    m_state.evalStack(),
    m_state.sp()
  };
}

void IRBuilder::exceptionStackBoundary() {
  // If this assert fires, we're trying to put things on the stack in a catch
  // trace that the unwinder won't be able to see.
  assert(
    spOffset() + m_state.evalStack().size() - m_state.stackDeficit() ==
    m_curMarker.spOff()
  );
  m_exnStack.stackDeficit = m_state.stackDeficit();
  m_exnStack.evalStack = m_state.evalStack();
  m_exnStack.sp = m_state.sp();
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

  if (splitCriticalEdges(m_unit)) {
    printUnit(6, m_unit, "after splitting critical edges for reoptimize");
  }

  m_state = FrameStateMgr(m_unit, m_initialMarker);
  m_cseHash.clear();
  m_enableCse = RuntimeOption::EvalHHIRCse;
  m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  if (!m_enableCse && !m_enableSimplification) return;
  setConstrainGuards(false);

  // This is a work in progress that we want committed, but that has some
  // issues still.
  auto const use_fixed_point = false;

  auto blocksIds = rpoSortCfgWithIds(m_unit);
  auto const idoms = findDominators(m_unit, blocksIds);
  boost::dynamic_bitset<> reachable(m_unit.numBlocks());
  reachable.set(m_unit.entry()->id());

  if (use_fixed_point) {
    m_state.computeFixedPoint(blocksIds);
  } else {
    m_state.setLegacyReoptimize();
  }

  for (auto block : blocksIds.blocks) {
    ITRACE(5, "reoptimize entering block: {}\n", block->id());
    Indent _i;

    // Skip block if it's unreachable.
    if (!reachable.test(block->id())) continue;

    if (use_fixed_point) {
      m_state.loadBlock(block);
    } else {
      m_state.startBlock(block, block->front().marker());
    }
    m_curBlock = block;

    auto nextBlock = block->next();
    auto backMarker = block->back().marker();
    auto instructions = block->moveInstrs();
    assert(block->empty());
    while (!instructions.empty()) {
      auto* inst = &instructions.front();
      instructions.pop_front();

      // optimizeWork below may create new instructions, and will use
      // m_nextMarker to decide where they are. Use the marker from this
      // instruction.
      assert(inst->marker().valid());
      setCurMarker(inst->marker());

      auto const tmp = optimizeInst(inst, CloneFlag::No, block, idoms);
      SSATmp* dst = inst->dst(0);

      if (dst != tmp) {
        // The result of optimization has a different destination than the inst.
        // Generate a mov(tmp->dst) to get result into dst.
        assert(inst->op() != DefLabel);
        assert(block->empty() || !block->back().isBlockEnd() || inst->next());
        auto src = tmp->inst()->is(Mov) ? tmp->inst()->src(0) : tmp;
        if (inst->next()) {
          // If the last instruction is a guard, insert the mov on the
          // fall-through edge (inst->next()).
          auto nextBlk = inst->next();
          nextBlk->insert(nextBlk->begin(),
                          m_unit.mov(dst, src, inst->marker()));
        } else {
          appendInstruction(m_unit.mov(dst, src, inst->marker()));
        }
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
    // Mark successor blocks as reachable.
    if (block->back().next())  reachable.set(block->back().next()->id());
    if (block->back().taken()) reachable.set(block->back().taken()->id());
    if (!use_fixed_point) {
      m_state.finishBlock(block);
    }
  }
}

/*
 * Returns true iff a guard to constrain was found, and tc was more specific
 * than the guard's existing constraint. Note that this doesn't necessarily
 * mean that the guard was constrained: tc.weak might be true.
 */
bool IRBuilder::constrainGuard(const IRInstruction* inst, TypeConstraint tc) {
  if (!shouldConstrainGuards()) return false;

  auto& guard = m_constraints.guards[inst];
  auto newTc = applyConstraint(guard, tc);
  ITRACE(2, "constrainGuard({}, {}): {} -> {}\n", *inst, tc, guard, newTc);
  Indent _i;

  auto const changed = guard != newTc;
  if (!tc.weak) guard = newTc;

  return changed;
}

/**
 * Trace back to the guard that provided the type of val, if
 * any. Constrain it so its type will not be relaxed beyond the given
 * DataTypeCategory. Returns true iff one or more guard instructions
 * were constrained.
 */
bool IRBuilder::constrainValue(SSATmp* const val, TypeConstraint tc) {
  if (!shouldConstrainGuards() || tc.empty()) return false;

  if (!val) {
    ITRACE(1, "constrainValue(nullptr, {}), bailing\n", tc);
    return false;
  }

  ITRACE(1, "constrainValue({}, {})\n", *val->inst(), tc);
  Indent _i;

  auto inst = val->inst();
  if (inst->is(LdLoc)) {
    // We've hit a LdLoc. If the value's type source is non-null and not a
    // FramePtr, it's a real value that was killed by a Call. The value won't
    // be live but it's ok to use it to track down the guard.

    always_assert_flog(m_constraints.typeSrcs.count(inst),
                       "No typeSrcs found for {}\n\n{}", *inst, m_unit);

    auto const typeSrcs = get_required(m_constraints.typeSrcs, inst);

    bool changed = false;
    for (auto typeSrc : typeSrcs) {
      if (typeSrc.isGuard()) {
        changed = constrainLocal(inst->extra<LocalId>()->locId, typeSrc, tc,
                                  "constrainValue") || changed;
      } else {
        // Keep chasing down the source of val.
        assert(typeSrc.isValue());
        changed = constrainValue(typeSrc.value, tc) || changed;
      }
    }
    return changed;
  } else if (inst->is(LdStack)) {
    return constrainStack(inst->src(0), inst->extra<StackOffset>()->offset, tc);
  } else if (inst->is(AssertType)) {
    // Sometimes code in HhbcTranslator asks for a value with DataTypeSpecific
    // but can tolerate a less specific value. If that happens, there's nothing
    // to constrain.
    if (!typeFitsConstraint(val->type(), tc)) return false;

    // If the immutable typeParam fits the constraint, we're done.
    auto const typeParam = inst->typeParam();
    if (typeFitsConstraint(typeParam, tc)) return false;

    auto const newTc = relaxConstraint(tc, typeParam, inst->src(0)->type());
    ITRACE(1, "tracing through {}, orig tc: {}, new tc: {}\n",
           *inst, tc, newTc);
    return constrainValue(inst->src(0), newTc);
  } else if (inst->is(CheckType)) {
    // Sometimes code in HhbcTranslator asks for a value with DataTypeSpecific
    // but can tolerate a less specific value. If that happens, there's nothing
    // to constrain.
    if (!typeFitsConstraint(val->type(), tc)) return false;

    bool changed = false;
    auto const typeParam = inst->typeParam();
    auto const srcType = inst->src(0)->type();

    // Constrain the guard on the CheckType, but first relax the constraint
    // based on what's known about srcType.
    auto const guardTc = relaxConstraint(tc, srcType, typeParam);
    changed = constrainGuard(inst, guardTc) || changed;

    // Relax typeParam with its current constraint. This is used below to
    // recursively relax the constraint on the source, if needed.
    auto constraint = applyConstraint(m_constraints.guards[inst], guardTc);
    auto const knownType = relaxType(typeParam, constraint);

    if (!typeFitsConstraint(knownType, tc)) {
      auto const newTc = relaxConstraint(tc, knownType, srcType);
      ITRACE(1, "tracing through {}, orig tc: {}, new tc: {}\n",
             *inst, tc, newTc);
      changed = constrainValue(inst->src(0), newTc) || changed;
    }
    return changed;
  } else if (inst->isPassthrough()) {
    return constrainValue(inst->getPassthroughValue(), tc);
  } else if (inst->is(DefLabel)) {
    auto changed = false;
    auto dst = 0;
    for (; dst < inst->numDsts(); dst++) {
      if (val == inst->dst(dst)) break;
    }
    assert(dst != inst->numDsts());
    for (auto& pred : inst->block()->preds()) {
      assert(pred.inst()->is(Jmp));
      auto src = pred.inst()->src(dst);
      changed = constrainValue(src, tc) || changed;
    }
    return changed;
  } else {
    // Any instructions not special cased above produce a new value, so
    // there's no guard for us to constrain.
    ITRACE(2, "value is new in this trace, bailing\n");
    return false;
  }
}

bool IRBuilder::constrainLocal(uint32_t locId,
                               TypeConstraint tc,
                               const std::string& why) {
  if (!shouldConstrainGuards() || tc.empty()) return false;
  bool changed = false;
  for (auto typeSrc : localTypeSources(locId)) {
    changed = constrainLocal(locId, typeSrc, tc, why) || changed;
  }
  return changed;
}

bool IRBuilder::constrainLocal(uint32_t locId,
                               TypeSource typeSrc,
                               TypeConstraint tc,
                               const std::string& why) {
  if (!shouldConstrainGuards() || tc.empty()) return false;

  ITRACE(1, "constrainLocal({}, {}, {}, {})\n",
         locId, show(typeSrc), tc, why);
  Indent _i;

  if (typeSrc.isValue()) return constrainValue(typeSrc.value, tc);

  assert(typeSrc.isGuard());
  auto const guard = typeSrc.guard;

  if (guard->is(GuardLoc)) {
    ITRACE(2, "found guard to constrain\n");
    return constrainGuard(guard, tc);
  }

  always_assert(guard->is(AssertLoc, CheckLoc));

  // If the dest of the (Assert|Check)Loc doesn't fit tc there's no point in
  // continuing.
  auto prevType = get_required(m_constraints.prevTypes, guard);
  if (!typeFitsConstraint(refineType(prevType, guard->typeParam()), tc)) {
    return false;
  }

  if (guard->is(AssertLoc)) {
    // If the immutable typeParam fits the constraint, we're done.
    auto const typeParam = guard->typeParam();
    if (typeFitsConstraint(typeParam, tc)) return false;

    auto const newTc = relaxConstraint(tc, typeParam, prevType);
    ITRACE(1, "tracing through {}, orig tc: {}, new tc: {}\n",
           *guard, tc, newTc);
    bool changed = false;
    auto typeSrcs = get_required(m_constraints.typeSrcs, guard);
    for (auto typeSrc : typeSrcs) {
      changed = constrainLocal(locId, typeSrc, newTc, why) || changed;
    }
    return changed;
  }

  // guard is a CheckLoc
  auto changed = false;
  auto const typeParam = guard->typeParam();

  // Constrain the guard on the CheckLoc, but first relax the constraint based
  // on what's known about prevType.
  auto const guardTc = relaxConstraint(tc, prevType, typeParam);
  changed = constrainGuard(guard, guardTc) || changed;

  // Relax typeParam with its current constraint.  This is used below to
  // recursively relax the constraint on the source, if needed.
  auto constraint = applyConstraint(m_constraints.guards[guard], guardTc);
  auto const knownType = relaxType(typeParam, constraint);

  if (!typeFitsConstraint(knownType, tc)) {
    auto const newTc = relaxConstraint(tc, knownType, prevType);
    ITRACE(1, "tracing through {}, orig tc: {}, new tc: {}\n",
           *guard, tc, newTc);
    auto typeSrcs = get_required(m_constraints.typeSrcs, guard);
    for (auto typeSrc : typeSrcs) {
      changed = constrainLocal(locId, typeSrc, newTc, why) || changed;
    }
  }
  return changed;
}

bool IRBuilder::constrainStack(int32_t idx, TypeConstraint tc) {
  if (!shouldConstrainGuards() || tc.empty()) return false;
  return constrainStack(sp(), idx, tc);
}

bool IRBuilder::constrainStack(SSATmp* sp, int32_t idx,
                               TypeConstraint tc) {
  if (!shouldConstrainGuards() || tc.empty()) return false;

  ITRACE(1, "constrainStack({}, {}, {})\n", *sp->inst(), idx, tc);
  Indent _i;
  assert(sp->isA(Type::StkPtr));

  // We've hit a LdStack. If getStackValue gives us a value, recurse on
  // that. Otherwise, look at the instruction that gave us the type of the
  // stack element. If it's a GuardStk or CheckStk, it's our target. If it's
  // anything else, the value is new so there's no guard to relax.
  auto stackInfo = getStackValue(sp, idx);

  // Sometimes code in HhbcTranslator asks for a value with DataTypeSpecific
  // but can tolerate a less specific value. If that happens, there's nothing
  // to constrain.
  if (!typeFitsConstraint(stackInfo.knownType, tc)) return false;

  auto const typeSrc = stackInfo.typeSrc;
  if (!typeSrc) {
    ITRACE(1, "  no typesrc\n");
    return false;
  }
  if (stackInfo.value) {
    ITRACE(1, "value = {}\n", *stackInfo.value->inst());
    return constrainValue(stackInfo.value, tc);
  } else if (typeSrc->is(AssertStk)) {
    // If the immutable typeParam fits the constraint, we're done.
    auto const typeParam = typeSrc->typeParam();
    if (typeFitsConstraint(typeParam, tc)) return false;

    auto const srcIdx = typeSrc->extra<StackOffset>()->offset;
    auto const srcType = getStackValue(typeSrc->src(0), srcIdx).knownType;
    auto const newTc = relaxConstraint(tc, typeParam, srcType);
    ITRACE(1, "tracing through {}, orig tc: {}, new tc: {}\n",
           *typeSrc, tc, newTc);
    return constrainStack(typeSrc->src(0), srcIdx, newTc);
  } else if (typeSrc->is(CheckStk)) {
    auto changed = false;
    auto const typeParam = typeSrc->typeParam();
    auto const srcIdx = typeSrc->extra<StackOffset>()->offset;
    auto const srcType = getStackValue(typeSrc->src(0), srcIdx).knownType;

    // Constrain the guard on the CheckType, but first relax the constraint
    // based on what's known about srcType.
    auto const guardTc = relaxConstraint(tc, srcType, typeParam);
    changed = constrainGuard(typeSrc, guardTc) || changed;

    // Relax typeParam with its current constraint. This is used below to
    // recursively relax the constraint on the source, if needed.
    auto constraint = applyConstraint(m_constraints.guards[typeSrc], guardTc);
    auto const knownType = relaxType(typeParam, constraint);

    if (!typeFitsConstraint(knownType, tc)) {
      auto const newTc = relaxConstraint(tc, knownType, srcType);
      ITRACE(1, "tracing through {}, orig tc: {}, new tc: {}\n",
             *typeSrc, tc, newTc);
      changed = constrainStack(typeSrc->src(0), srcIdx, newTc) || changed;
    }
    return changed;
  } else {
    ITRACE(1, "typeSrc = {}\n", *typeSrc);
    return typeSrc->is(GuardStk) && constrainGuard(typeSrc, tc);
  }
}

Type IRBuilder::localType(uint32_t id, TypeConstraint tc) {
  constrainLocal(id, tc, "localType");
  return m_state.localType(id);
}

Type IRBuilder::predictedInnerType(uint32_t id) {
  auto const ty = m_state.predictedLocalType(id);
  assert(ty.isBoxed());
  return ldRefReturn(ty.unbox());
}

Type IRBuilder::predictedLocalType(uint32_t id) {
  return m_state.predictedLocalType(id);
}

SSATmp* IRBuilder::localValue(uint32_t id, TypeConstraint tc) {
  constrainLocal(id, tc, "localValue");
  return m_state.localValue(id);
}

void IRBuilder::setCurMarker(BCMarker newMarker) {
  if (newMarker == m_curMarker) return;
  FTRACE(2, "IRBuilder changing current marker from {} to {}\n",
         m_curMarker.valid() ? m_curMarker.show() : "<invalid>",
         newMarker.show());
  assert(newMarker.valid());
  m_curMarker = newMarker;
}

bool IRBuilder::startBlock(Block* block, const BCMarker& marker,
                           bool isLoopHeader) {
  assert(block);
  assert(m_savedBlocks.empty());  // No bytecode control flow in exits.

  if (block == m_curBlock) return true;

  // Return false if we don't have a state for block. This can happen
  // when trying to start a region block that turned out to be unreachable.
  if (!m_state.hasStateFor(block)) return false;

  // There's no reason for us to be starting on the entry block when it's not
  // our current block.
  always_assert(!block->isEntry());
  always_assert(fp() != nullptr);

  auto& lastInst = m_curBlock->back();
  always_assert(lastInst.isBlockEnd());
  always_assert(lastInst.isTerminal() || m_curBlock->next() != nullptr);

  m_state.finishBlock(m_curBlock);
  m_curBlock = block;

  m_state.startBlock(m_curBlock, marker, isLoopHeader);
  if (sp() == nullptr) gen(DefSP, StackOffset{spOffset()}, fp());

  FTRACE(2, "IRBuilder switching to block B{}: {}\n", block->id(),
         show(m_state));
  return true;
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

void IRBuilder::resetOffsetMapping() {
  m_offsetToBlockMap.clear();
}

bool IRBuilder::hasBlock(Offset offset) const {
  return m_offsetToBlockMap.count(offset);
}

void IRBuilder::setBlock(Offset offset, Block* block) {
  assert(!hasBlock(offset));
  m_offsetToBlockMap[offset] = block;
}

void IRBuilder::pushBlock(BCMarker marker, Block* b) {
  FTRACE(2, "IRBuilder saving {}@{} and using {}@{}\n",
         m_curBlock, m_curMarker.show(), b, marker.show());
  assert(b);

  m_savedBlocks.push_back(
    BlockState { m_curBlock, m_curMarker, m_exnStack }
  );
  m_state.pauseBlock(m_curBlock);
  m_state.startBlock(b, marker);
  m_curBlock = b;
  m_curMarker = marker;

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
         m_curBlock, m_curMarker.show(), top.block, top.marker.show());
  m_state.finishBlock(m_curBlock);
  m_state.unpauseBlock(top.block);
  m_curBlock = top.block;
  m_curMarker = top.marker;
  m_exnStack = top.exnStack;
  m_savedBlocks.pop_back();
}

//////////////////////////////////////////////////////////////////////

CSEHash& IRBuilder::cseHashTable(const IRInstruction& inst) {
  return inst.is(DefConst) ? m_unit.constTable() : m_cseHash;
}

const CSEHash& IRBuilder::cseHashTable(const IRInstruction& inst) const {
  return const_cast<IRBuilder*>(this)->cseHashTable(inst);
}

SSATmp* IRBuilder::cseLookup(const IRInstruction& inst,
                             const Block* srcBlock,
                             const folly::Optional<IdomVector>& idoms) const {
  auto tmp = cseHashTable(inst).lookup(const_cast<IRInstruction*>(&inst));
  if (tmp && idoms) {
    // During a reoptimize pass, we need to make sure that any values we want
    // to reuse for CSE are only reused in blocks dominated by the block that
    // defines it.
    if (tmp->inst()->is(DefConst)) return tmp;
    auto const dom = findDefiningBlock(tmp);
    if (!dom || !dominates(dom, srcBlock, *idoms)) return nullptr;
  }
  return tmp;
}

void IRBuilder::cseUpdate(const IRInstruction& inst) {
  switch (inst.op()) {
  case Call:
  case CallArray:
  case ContEnter:
    m_cseHash.clear();
    break;
  default:
    break;
  }

  if (m_enableCse && inst.canCSE()) {
    cseHashTable(inst).insert(inst.dst());
  }
  if (inst.killsSources()) {
    for (auto i = uint32_t{0}; i < inst.numSrcs(); ++i) {
      if (inst.killsSource(i) && inst.src(i)->inst()->canCSE()) {
        cseHashTable(inst).erase(inst.src(i));
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////

}}
