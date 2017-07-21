/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/util/assertions.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/simple-propagation.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/type-constraint.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

TRACE_SET_MOD(hhir);
using Trace::Indent;

///////////////////////////////////////////////////////////////////////////////

template<typename M>
const typename M::mapped_type& get_required(const M& m,
                                            typename M::key_type key) {
  auto it = m.find(key);
  always_assert(it != m.end());
  return it->second;
}

SSATmp* fwdGuardSource(IRInstruction* inst) {
  if (inst->is(AssertType, CheckType)) return inst->src(0);

  assertx(inst->is(AssertLoc,   CheckLoc,
                   AssertStk,   CheckStk,
                   AssertMBase, CheckMBase));
  inst->convertToNop();
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

IRBuilder::IRBuilder(IRUnit& unit, BCMarker initMarker)
  : m_unit(unit)
  , m_initialMarker(initMarker)
  , m_curBCContext{initMarker, 0}
  , m_state(initMarker)
  , m_curBlock(m_unit.entry())
{
  if (RuntimeOption::EvalHHIRGenOpts) {
    m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  }
  m_state.startBlock(m_curBlock, false);
}

void IRBuilder::enableConstrainGuards() {
  m_constrainGuards = true;
}

bool IRBuilder::shouldConstrainGuards() const {
  return m_constrainGuards;
}

void IRBuilder::appendInstruction(IRInstruction* inst) {
  FTRACE(1, "  append {}\n", inst->toString());

  if (shouldConstrainGuards()) {
    auto const l = [&]() -> folly::Optional<Location> {
      switch (inst->op()) {
        case AssertLoc:
        case CheckLoc:
        case LdLoc:
          return loc(inst->extra<LocalId>()->locId);

        case AssertStk:
        case CheckStk:
        case LdStk:
          return stk(inst->extra<IRSPRelOffsetData>()->offset);

        case AssertMBase:
        case CheckMBase:
          return folly::make_optional<Location>(Location::MBase{});

        default:
          return folly::none;
      }
      not_reached();
    }();

    // If we're constraining guards, some instructions need certain information
    // to be recorded in side tables.
    if (l) {
      m_constraints.typeSrcs[inst] = m_state.typeSrcsOf(*l);
      if (!inst->is(LdLoc, LdStk)) {
        constrainLocation(*l, DataTypeGeneric, "appendInstruction");
        m_constraints.prevTypes[inst] = m_state.typeOf(*l);
      }
    }

    // And a LdRef or CheckRefInner automatically constrains the value to be a
    // boxed cell, specifically.
    if (inst->is(LdRef, CheckRefInner)) {
      constrainValue(inst->src(0), DataTypeSpecific);
    }
  }

  auto where = m_curBlock->end();

  // If the block isn't empty, check if we need to create a new block.
  if (where != m_curBlock->begin()) {
    auto prevIt = where;
    --prevIt;
    auto& prev = *prevIt;

    if (prev.isBlockEnd()) {
      assertx(where == m_curBlock->end());

      auto oldBlock = m_curBlock;

      // First make the inst's next block, so we can save state to it in
      // finishBlock.
      m_curBlock = m_unit.defBlock(prev.block()->profCount());
      if (!prev.isTerminal()) {
        // New block is reachable from old block so link it.
        prev.setNext(m_curBlock);
        m_curBlock->setHint(prev.block()->hint());
      }

      m_state.finishBlock(oldBlock);

      m_state.startBlock(m_curBlock, false);
      where = m_curBlock->begin();

      FTRACE(2, "lazily adding B{}\n", m_curBlock->id());

    }
  }

  assertx(IMPLIES(inst->isBlockEnd(), where == m_curBlock->end()) &&
         "Can't insert a BlockEnd instruction in the middle of a block");
  if (do_assert && where != m_curBlock->begin()) {
    UNUSED auto prevIt = where;
    --prevIt;
    assertx(!prevIt->isBlockEnd() &&
           "Can't append an instruction after a BlockEnd instruction");
  }

  assertx(inst->marker().valid());
  if (!inst->is(Nop, DefConst)) {
    where = m_curBlock->insert(where, inst);
    ++where;
  }

  m_state.update(inst);

  if (inst->isTerminal()) m_state.finishBlock(m_curBlock);
}

///////////////////////////////////////////////////////////////////////////////

SSATmp* IRBuilder::preOptimizeCheckLocation(IRInstruction* inst, Location l) {
  if (auto const prevValue = valueOf(l, DataTypeGeneric)) {
    gen(CheckType, inst->typeParam(), inst->taken(), prevValue);
    inst->convertToNop();
    return nullptr;
  }

  auto const oldType = typeOf(l, DataTypeGeneric);
  auto const newType = oldType & inst->typeParam();

  if (oldType <= newType) {
    // The type of the src is the same or more refined than type, so the guard
    // is unnecessary.
    return fwdGuardSource(inst);
  }
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeCheckLoc(IRInstruction* inst) {
  return preOptimizeCheckLocation(inst, loc(inst->extra<CheckLoc>()->locId));
}

SSATmp* IRBuilder::preOptimizeCheckStk(IRInstruction* inst) {
  return preOptimizeCheckLocation(inst, stk(inst->extra<CheckStk>()->offset));
}

SSATmp* IRBuilder::preOptimizeCheckMBase(IRInstruction* inst) {
  return preOptimizeCheckLocation(inst, Location::MBase{});
}

SSATmp* IRBuilder::preOptimizeHintInner(IRInstruction* inst, Location l) {
  if (!(typeOf(l, DataTypeGeneric) <= TBoxedCell) ||
      predictedInnerType(l).box() <= inst->typeParam()) {
    inst->convertToNop();
  }
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeHintLocInner(IRInstruction* inst) {
  return preOptimizeHintInner(inst, loc(inst->extra<HintLocInner>()->locId));
}

SSATmp* IRBuilder::preOptimizeHintMBaseInner(IRInstruction* inst) {
  return preOptimizeHintInner(inst, Location::MBase{});
}

SSATmp* IRBuilder::preOptimizeAssertTypeOp(IRInstruction* inst,
                                           const Type oldType,
                                           SSATmp* oldVal,
                                           const IRInstruction* srcInst) {
  ITRACE(3, "preOptimizeAssertTypeOp({}, {}, {}, {})\n",
         *inst, oldType,
         oldVal ? oldVal->toString() : "nullptr",
         srcInst ? srcInst->toString() : "nullptr");

  auto const newType = oldType & inst->typeParam();

  // Eliminate this AssertTypeOp if:
  // 1) oldType is at least as good as newType and:
  //      a) typeParam == Gen
  //      b) oldVal is from a DefConst
  //      c) oldType.hasConstVal()
  //    The AssertType will never be useful for guard constraining in these
  //     situations.
  // 2) The source instruction is known to be another assert that's at least
  //    as good as this one.
  if ((oldType <= newType &&
       (inst->typeParam() == TGen ||
        (oldVal && oldVal->inst()->is(DefConst)) ||
        oldType.hasConstVal())) ||
      (srcInst &&
       srcInst->is(AssertType, AssertLoc, AssertStk, AssertMBase) &&
       srcInst->typeParam() <= inst->typeParam())) {
    return fwdGuardSource(inst);
  }

  // 3) We're not constraining guards, and the old type is at least as good as
  //    the new type.
  if (!shouldConstrainGuards()) {
    if (newType == TBottom) {
      gen(Unreachable);
      return inst->is(AssertType) ? m_unit.cns(TBottom) : nullptr;
    }

    if (oldType <= newType) return fwdGuardSource(inst);
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeAssertLocation(IRInstruction* inst,
                                             Location l) {
  if (auto const prevValue = valueOf(l, DataTypeGeneric)) {
    gen(AssertType, inst->typeParam(), prevValue);
    inst->convertToNop();
    return nullptr;
  }

  return preOptimizeAssertTypeOp(
    inst,
    typeOf(l, DataTypeGeneric),
    valueOf(l, DataTypeGeneric),
    nullptr
  );
}

SSATmp* IRBuilder::preOptimizeAssertType(IRInstruction* inst) {
  auto const src = inst->src(0);
  return preOptimizeAssertTypeOp(inst, src->type(), src, src->inst());
}

SSATmp* IRBuilder::preOptimizeAssertLoc(IRInstruction* inst) {
  return preOptimizeAssertLocation(inst, loc(inst->extra<AssertLoc>()->locId));
}

SSATmp* IRBuilder::preOptimizeAssertStk(IRInstruction* inst) {
  return preOptimizeAssertLocation(inst, stk(inst->extra<AssertStk>()->offset));
}

SSATmp* IRBuilder::preOptimizeLdARFuncPtr(IRInstruction* inst) {
  auto const& fpiStack = fs().fpiStack();
  auto const arOff = inst->extra<LdARFuncPtr>()->offset;
  auto const invOff = arOff.to<FPInvOffset>(fs().irSPOff()) - kNumActRecCells;

  for (auto i = fpiStack.size(); i--; ) {
    auto const& info = fpiStack[i];
    if (info.returnSP == inst->src(0) &&
        info.returnSPOff == invOff) {
      if (info.func) return m_unit.cns(info.func);
      return nullptr;
    }
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeCheckCtxThis(IRInstruction* inst) {
  auto const func = inst->marker().func();
  if (!func->mayHaveThis()) {
    auto const taken = inst->taken();
    inst->convertToNop();
    return gen(Jmp, taken);
  }
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdCtxHelper(IRInstruction* inst) {
  // Change LdCtx in static functions to LdCctx, or if we're inlining try to
  // fish out a constant context.
  auto const func = inst->marker().func();
  assertx(func->cls());
  auto const ctx = [&]() -> SSATmp* {
    auto ret = m_state.ctx();
    if (!ret) return nullptr;
    if (ret->inst()->is(DefConst)) return ret;
    if (ret->hasConstVal() ||
        ret->type().subtypeOfAny(TInitNull, TUninit, TNullptr)) {
      return m_unit.cns(ret->type());
    }
    if (!m_state.frameMaySpanCall()) return ret;
    return nullptr;
  }();

  if (ctx) {
    if (ctx->hasConstVal(TCls)) {
      return m_unit.cns(ConstCctx::cctx(ctx->clsVal()));
    }
    if (ctx->isA(TCls)) {
      return gen(ConvClsToCctx, ctx);
    }
    if (ctx->isA(TCtx)) {
      return ctx;
    }
  }

  if (!func->mayHaveThis()) {
    // ActRec->m_cls of a static function is always a valid class pointer with
    // the bottom bit set
    if (func->cls()->attrs() & AttrNoOverride) {
      return m_unit.cns(ConstCctx::cctx(func->cls()));
    }
    if (inst->op() == LdCtx) {
      auto const src = inst->src(0);
      return gen(LdCctx, src);
    }
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdLocation(IRInstruction* inst, Location l) {
  if (auto tmp = valueOf(l, DataTypeGeneric)) return tmp;

  auto const type = typeOf(l, DataTypeGeneric);

  // The types may not be compatible in the presence of unreachable code.
  // Don't try to optimize the code in this case, and just let dead code
  // elimination take care of it later.
  if (!type.maybe(inst->typeParam())) {
    inst->setTypeParam(TBottom);
    return nullptr;
  }

  if (l.tag() == LTag::Local) {
    if (!inst->marker().func()->isClosureBody() ||
        l.localId() != inst->marker().func()->numParams()) {
      // If FrameStateMgr's type for a local isn't as good as the type param,
      // we're missing information in the IR.
      assertx(inst->typeParam() >= type);
    }
  }
  inst->setTypeParam(std::min(type, inst->typeParam()));

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdLoc(IRInstruction* inst) {
  return preOptimizeLdLocation(inst, loc(inst->extra<LdLoc>()->locId));
}

SSATmp* IRBuilder::preOptimizeLdStk(IRInstruction* inst) {
  return preOptimizeLdLocation(inst, stk(inst->extra<LdStk>()->offset));
}

SSATmp* IRBuilder::preOptimizeLdClsRef(IRInstruction* inst) {
  return preOptimizeLdLocation(inst, cslot(inst->extra<LdClsRef>()->slot));
}

SSATmp* IRBuilder::preOptimizeCastStk(IRInstruction* inst) {
  auto const off = inst->extra<CastStk>()->offset;
  auto const curType = stack(off, DataTypeGeneric).type;

  if (inst->typeParam() == TNullableObj && curType <= TNull) {
    // If we're casting Null to NullableObj, we still need to call
    // tvCastToNullableObjectInPlace. See comment there and t3879280 for
    // details.
    return nullptr;
  }
  if (curType <= inst->typeParam()) {
    inst->convertToNop();
    return nullptr;
  }
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeCoerceStk(IRInstruction* inst) {
  auto const off = inst->extra<CoerceStk>()->offset;
  auto const curType = stack(off, DataTypeGeneric).type;

  if (curType <= inst->typeParam()) {
    inst->convertToNop();
    return nullptr;
  }
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdMBase(IRInstruction* inst) {
  if (auto ptr = m_state.mbr().ptr) return ptr;

  inst->setTypeParam(inst->typeParam() & m_state.mbr().ptrType);
  return nullptr;
}

SSATmp* IRBuilder::preOptimize(IRInstruction* inst) {
#define X(op) case op: return preOptimize##op(inst);
  switch (inst->op()) {
  X(HintLocInner)
  X(HintMBaseInner)
  X(AssertType)
  X(AssertLoc)
  X(AssertStk)
  X(CheckLoc)
  X(CheckStk)
  X(CheckMBase)
  X(LdLoc)
  X(LdStk)
  X(LdClsRef)
  X(CastStk)
  X(CoerceStk)
  X(LdARFuncPtr)
  X(CheckCtxThis)
  X(LdCtx)
  X(LdCctx)
  X(LdMBase)
  default: break;
  }
#undef X
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Perform preoptimization and simplification on the input instruction.  If the
 * input instruction has a dest, this will return an SSATmp that represents the
 * same value as dst(0) of the input instruction.  If the input instruction has
 * no dest, this will return nullptr.
 *
 * The caller never needs to clone or append; all this has been done.
 */
SSATmp* IRBuilder::optimizeInst(IRInstruction* inst, CloneFlag doClone,
                                Block* /*srcBlock*/) {
  static DEBUG_ONLY __thread int instNest = 0;
  if (debug) ++instNest;
  SCOPE_EXIT { if (debug) --instNest; };
  DEBUG_ONLY auto indent = [&] { return std::string(instNest * 2, ' '); };

  FTRACE(1, "optimize: {}\n", inst->toString());

  auto cloneAndAppendOriginal = [&] () -> SSATmp* {
    if (inst->op() == Nop) return nullptr;
    if (doClone == CloneFlag::Yes) {
      inst = m_unit.clone(inst);
    }
    appendInstruction(inst);
    return inst->dst(0);
  };

  // Copy propagation on inst source operands. Only perform constant
  // propagation if we're not constraining guards, to avoid breaking the
  // use-def chains it uses to find guards.
  copyProp(inst);
  if (!shouldConstrainGuards()) constProp(m_unit, inst);

  // Since preOptimize can inspect tracked state, we don't
  // perform it on non-main traces.
  if (m_savedBlocks.size() == 0) {
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
  }

  // We skip simplification for AssertType when guard constraining is enabled
  // because information that appears to be redundant may allow us to avoid
  // constraining certain guards. preOptimizeAssertType() still eliminates some
  // subset of redundant AssertType instructions.
  if (!m_enableSimplification ||
      (shouldConstrainGuards() && inst->is(AssertType))) {
    return cloneAndAppendOriginal();
  }

  auto const simpResult = simplify(m_unit, inst);

  // These are the possible outputs:
  //
  // ([], nullptr): no optimization possible. Use original inst.
  //
  // ([], non-nullptr): passing through a src.
  //
  // ([X, ...], Y): throw away input instruction, append 'X, ...',
  //                return Y.

  if (!simpResult.instrs.empty()) {
    // New instructions were generated. Append the new ones, filtering out Nops.
    for (auto* newInst : simpResult.instrs) {
      assertx(!newInst->isTransient());
      if (newInst->op() == Nop) continue;
      appendInstruction(newInst);
    }
    return simpResult.dst;
  }

  // No new instructions were generated. Either simplification didn't do
  // anything, or we're using some other instruction's dst instead of our own.

  if (simpResult.dst) {
    // We're using some other instruction's output.  Don't append anything.
    assertx(simpResult.dst->inst() != inst);
    return simpResult.dst;
  }

  // No simplification happened.
  return cloneAndAppendOriginal();
}

void IRBuilder::exceptionStackBoundary() {
  /*
   * If this assert fires, we're trying to put things on the stack in a catch
   * trace that the unwinder won't be able to see.
   */
  FTRACE(2, "exceptionStackBoundary()\n");
  assertx(m_state.bcSPOff() == curMarker().spOff());
  m_exnStack.syncedSpLevel = m_state.bcSPOff();
  m_state.resetStackModified();
}

void IRBuilder::setCurMarker(BCMarker newMarker) {
  if (newMarker == curMarker()) return;
  FTRACE(2, "IRBuilder changing current marker from {} to {}\n",
         curMarker().valid() ? curMarker().show() : "<invalid>",
         newMarker.show());
  assertx(newMarker.valid());
  m_curBCContext.marker = newMarker;
}

///////////////////////////////////////////////////////////////////////////////
// Guard relaxation.

bool IRBuilder::constrainGuard(const IRInstruction* inst, TypeConstraint tc) {
  if (!shouldConstrainGuards()) return false;

  auto& guard = m_constraints.guards[inst];
  auto newTc = applyConstraint(guard, tc);
  ITRACE(2, "constrainGuard({}, {}): {} -> {}\n", *inst, tc, guard, newTc);
  Indent _i;

  auto const changed = guard != newTc;
  if (changed && !tc.weak) guard = newTc;

  return changed;
}

bool IRBuilder::constrainValue(SSATmp* const val, TypeConstraint tc) {
  if (!shouldConstrainGuards() || tc.empty()) return false;

  if (!val) {
    ITRACE(1, "attempted to constrain nullptr SSATmp*; bailing\n", tc);
    return false;
  }
  auto inst = val->inst();

  ITRACE(1, "constraining {} to {}\n", *inst, tc);
  Indent _i;

  if (inst->is(LdLoc, LdStk)) {
    // If the value's type source is non-null and not a FramePtr, it's a real
    // value that was killed by a Call. The value won't be live but it's ok to
    // use it to track down the guard.

    always_assert_flog(m_constraints.typeSrcs.count(inst),
                       "no typeSrcs found for {}", *inst);

    bool changed = false;
    auto const typeSrcs = get_required(m_constraints.typeSrcs, inst);

    for (auto typeSrc : typeSrcs) {
      if (typeSrc.isGuard()) {
        if (inst->is(LdLoc)) {
          ITRACE(1, "constraining guard for local[{}]\n",
                 inst->extra<LdLoc>()->locId);
        } else {
          assertx(inst->is(LdStk));
          ITRACE(1, "constraining guard for stack[{}]\n",
                 inst->extra<LdStk>()->offset.offset);
        }
      }
      changed |= constrainTypeSrc(typeSrc, tc);
    }
    return changed;
  }

  if (inst->is(AssertType)) {
    // Sometimes code in irgen asks for a value with DataTypeSpecific but can
    // tolerate a less specific value.  If that happens, there's nothing to
    // constrain.
    if (!typeFitsConstraint(val->type(), tc)) return false;

    return constrainAssert(inst, tc, inst->src(0)->type());
  }

  if (inst->is(CheckType)) {
    // Sometimes code in irgen asks for a value with DataTypeSpecific but can
    // tolerate a less specific value.  If that happens, there's nothing to
    // constrain.
    if (!typeFitsConstraint(val->type(), tc)) return false;

    return constrainCheck(inst, tc, inst->src(0)->type());
  }

  if (inst->isPassthrough()) {
    return constrainValue(inst->getPassthroughValue(), tc);
  }

  if (inst->is(DefLabel)) {
    auto changed = false;
    auto dst = 0;
    for (; dst < inst->numDsts(); dst++) {
      if (val == inst->dst(dst)) break;
    }
    assertx(dst != inst->numDsts());
    for (auto& pred : inst->block()->preds()) {
      assertx(pred.inst()->is(Jmp));
      auto src = pred.inst()->src(dst);
      changed |= constrainValue(src, tc);
    }
    return changed;
  }

  // Any instructions not special cased above produce a new value, so there's
  // no guard for us to constrain.
  ITRACE(2, "value is new in this trace, bailing\n");
  return false;
}

bool IRBuilder::constrainLocation(Location l, TypeConstraint tc,
                                  const std::string& why) {
  if (!shouldConstrainGuards() ||
      l.tag() == LTag::CSlot ||
      tc.empty()) return false;

  ITRACE(1, "constraining {} to {} (for {})\n", show(l), tc, why);
  Indent _i;

  bool changed = false;
  for (auto typeSrc : m_state.typeSrcsOf(l)) {
    changed |= constrainTypeSrc(typeSrc, tc);
  }
  return changed;
}

bool IRBuilder::constrainLocation(Location l, TypeConstraint tc) {
  return constrainLocation(l, tc, "");
}

bool IRBuilder::constrainLocal(uint32_t locID, TypeConstraint tc,
                               const std::string& why) {
  return constrainLocation(loc(locID), tc, why);
}

bool IRBuilder::constrainStack(IRSPRelOffset offset, TypeConstraint tc) {
  return constrainLocation(stk(offset), tc);
}

bool IRBuilder::constrainTypeSrc(TypeSource typeSrc, TypeConstraint tc) {
  if (!shouldConstrainGuards() || tc.empty()) return false;

  ITRACE(1, "constraining type source {} to {}\n", show(typeSrc), tc);
  Indent _i;

  if (typeSrc.isValue()) return constrainValue(typeSrc.value, tc);

  assertx(typeSrc.isGuard());
  auto const guard = typeSrc.guard;

  always_assert(guard->is(AssertLoc,   CheckLoc,
                          AssertStk,   CheckStk,
                          AssertMBase, CheckMBase));

  // If the dest of the Assert/Check doesn't fit `tc', there's no point in
  // continuing.
  auto prevType = get_required(m_constraints.prevTypes, guard);
  if (!typeFitsConstraint(prevType & guard->typeParam(), tc)) {
    return false;
  }

  if (guard->is(AssertLoc, AssertStk)) {
    return constrainAssert(guard, tc, prevType);
  }
  return constrainCheck(guard, tc, prevType);
}

/*
 * Constrain the sources of an Assert instruction.
 *
 * We also have to constrain the sources for Check instructions, and we share
 * this codepath for that purpose.  However, for Checks, we first pre-relax the
 * instruction's typeParam, which we pass as `knownType'.  (Otherwise, the
 * typeParam will be used as the `knownType'.)
 */
bool IRBuilder::constrainAssert(const IRInstruction* inst,
                                TypeConstraint tc, Type srcType,
                                folly::Optional<Type> knownType) {
  if (!knownType) knownType = inst->typeParam();

  // If the known type fits the constraint, we're done.
  if (typeFitsConstraint(*knownType, tc)) return false;

  auto const newTC = relaxConstraint(tc, *knownType, srcType);
  ITRACE(1, "tracing through {}, orig tc: {}, new tc: {}\n",
         *inst, tc, newTC);

  if (inst->is(AssertType, CheckType)) {
    return constrainValue(inst->src(0), newTC);
  }

  auto changed = false;
  auto const& typeSrcs = get_required(m_constraints.typeSrcs, inst);

  for (auto typeSrc : typeSrcs) {
    changed |= constrainTypeSrc(typeSrc, newTC);
  }
  return changed;
}

/*
 * Constrain the typeParam and sources of a Check instruction.
 */
bool IRBuilder::constrainCheck(const IRInstruction* inst,
                               TypeConstraint tc, Type srcType) {
  assertx(inst->is(CheckType, CheckLoc, CheckStk, CheckMBase));

  auto changed = false;
  auto const typeParam = inst->typeParam();

  // Constrain the guard on the Check instruction, but first relax the
  // constraint based on what's known about `srcType'.
  auto const guardTC = relaxConstraint(tc, srcType, typeParam);
  changed |= constrainGuard(inst, guardTC);

  // Relax typeParam with its current constraint.  This is used below to
  // recursively relax the constraint on the source, if needed.
  auto constraint = applyConstraint(m_constraints.guards[inst], guardTC);
  auto const knownType = relaxType(typeParam, constraint.category);

  changed |= constrainAssert(inst, tc, srcType, knownType);

  return changed;
}

uint32_t IRBuilder::numGuards() const {
  uint32_t count = 0;
  for (auto& g : m_constraints.guards) {
    if (g.second != DataTypeGeneric) count++;
  }
  return count;
}

///////////////////////////////////////////////////////////////////////////////

const LocalState& IRBuilder::local(uint32_t id, TypeConstraint tc) {
  constrainLocal(id, tc, "");
  return m_state.local(id);
}

const StackState& IRBuilder::stack(IRSPRelOffset offset, TypeConstraint tc) {
  constrainStack(offset, tc);
  return m_state.stack(offset);
}

const CSlotState& IRBuilder::clsRefSlot(uint32_t slot) {
  return m_state.clsRefSlot(slot);
}

SSATmp* IRBuilder::valueOf(Location l, TypeConstraint tc) {
  constrainLocation(l, tc, "");
  return m_state.valueOf(l);
}

Type IRBuilder::typeOf(Location l, TypeConstraint tc) {
  constrainLocation(l, tc, "");
  return m_state.typeOf(l);
}

Type IRBuilder::predictedInnerType(Location l) const {
  auto const ty = m_state.predictedTypeOf(l);
  assertx(ty <= TBoxedCell);
  return ldRefReturn(ty.unbox());
}

Type IRBuilder::predictedLocalInnerType(uint32_t id) const {
  return predictedInnerType(loc(id));
}

Type IRBuilder::predictedStackInnerType(IRSPRelOffset offset) const {
  return predictedInnerType(stk(offset));
}

Type IRBuilder::predictedMBaseInnerType() const {
  auto const ty = m_state.mbase().predictedType;
  assertx(ty <= TBoxedCell);
  return ldRefReturn(ty.unbox());
}

/*
 * Wrap a local, stack ID, or class-ref slot into a Location.
 */
Location IRBuilder::loc(uint32_t id) const {
  return Location::Local { id };
}
Location IRBuilder::stk(IRSPRelOffset off) const {
  auto const fpRel = off.to<FPInvOffset>(m_state.irSPOff());
  return Location::Stack { fpRel };
}
Location IRBuilder::cslot(uint32_t slot) const {
  return Location::CSlot { slot };
}

///////////////////////////////////////////////////////////////////////////////
// Bytecode-level control flow.

bool IRBuilder::canStartBlock(Block* block) const {
  return m_state.hasStateFor(block);
}

bool IRBuilder::startBlock(Block* block, bool hasUnprocessedPred) {
  assertx(block);
  assertx(m_savedBlocks.empty());  // No bytecode control flow in exits.

  if (block == m_curBlock) return true;

  // Return false if we don't have a FrameState saved for `block' yet
  // -- meaning it isn't reachable from the entry block yet.
  if (!canStartBlock(block)) return false;

  // There's no reason for us to be starting on the entry block when it's not
  // our current block.
  always_assert(!block->isEntry());

  auto& lastInst = m_curBlock->back();
  always_assert(lastInst.isBlockEnd());
  always_assert(lastInst.isTerminal() || m_curBlock->next() != nullptr);

  m_state.finishBlock(m_curBlock);
  m_curBlock = block;

  m_state.startBlock(m_curBlock, hasUnprocessedPred);
  always_assert(m_state.sp() != nullptr);
  always_assert(m_state.fp() != nullptr);

  FTRACE(2, "IRBuilder switching to block B{}: {}\n", block->id(),
         show(m_state));
  return true;
}

Block* IRBuilder::makeBlock(SrcKey sk, uint64_t profCount) {
  auto it = m_skToBlockMap.find(sk);
  if (it == m_skToBlockMap.end()) {
    auto const block = m_unit.defBlock(profCount);
    m_skToBlockMap.emplace(sk, block);
    return block;
  }
  return it->second;
}

void IRBuilder::resetOffsetMapping() {
  m_skToBlockMap.clear();
}

bool IRBuilder::hasBlock(SrcKey sk) const {
  return m_skToBlockMap.count(sk);
}

void IRBuilder::setBlock(SrcKey sk, Block* block) {
  assertx(!hasBlock(sk));
  m_skToBlockMap[sk] = block;
}

void IRBuilder::appendBlock(Block* block, Block* pred) {
  m_state.finishBlock(m_curBlock);

  FTRACE(2, "appending B{}\n", block->id());
  // Load up the state for the new block.
  m_state.startBlock(block, false, pred);
  m_curBlock = block;
}

Block* IRBuilder::guardFailBlock() const {
  return m_guardFailBlock;
}

void IRBuilder::setGuardFailBlock(Block* block) {
  m_guardFailBlock = block;
}

void IRBuilder::resetGuardFailBlock() {
  m_guardFailBlock = nullptr;
}

void IRBuilder::pushBlock(BCMarker marker, Block* b) {
  FTRACE(2, "IRBuilder saving {}@{} and using {}@{}\n",
         m_curBlock, curMarker().show(), b, marker.show());
  assertx(b);

  m_savedBlocks.push_back(
    BlockState { m_curBlock, m_curBCContext, m_exnStack }
  );
  m_state.pauseBlock(m_curBlock);
  m_state.startBlock(b, false);
  m_curBlock = b;
  m_curBCContext = BCContext { marker, 0 };

  if (do_assert) {
    for (UNUSED auto const& state : m_savedBlocks) {
      assertx(state.block != b &&
             "Can't push a block that's already in the saved stack");
    }
  }
}

void IRBuilder::popBlock() {
  assertx(!m_savedBlocks.empty());

  auto const& top = m_savedBlocks.back();
  FTRACE(2, "IRBuilder popping {}@{} to restore {}@{}\n",
         m_curBlock, curMarker().show(), top.block, top.bcctx.marker.show());
  m_state.finishBlock(m_curBlock);
  m_state.unpauseBlock(top.block);
  m_curBlock = top.block;
  m_curBCContext = top.bcctx;
  m_exnStack = top.exnStack;
  m_savedBlocks.pop_back();
}

///////////////////////////////////////////////////////////////////////////////

}}}
