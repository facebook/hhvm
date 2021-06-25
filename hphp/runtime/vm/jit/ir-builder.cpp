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
#include "hphp/runtime/vm/jit/guard-constraint.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/simple-propagation.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/translator.h"

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

bool isMBaseLoad(const IRInstruction* inst) {
  if (!inst->is(LdMem)) return false;
  auto src = inst->src(0)->inst();
  while (src->isPassthrough()) src = src->getPassthroughValue()->inst();
  return src->is(LdMBase);
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

IRBuilder::IRBuilder(IRUnit& unit, const Func* func)
  : m_unit(unit)
  , m_curBCContext{BCMarker{}, 0}
  , m_state(func)
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
  assertx(inst->marker().valid());
  assertx(checkOperandTypes(inst));
  if (inst->is(Nop, DefConst)) return;

  if (shouldConstrainGuards()) {
    auto const l = [&]() -> Optional<Location> {
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
          return make_optional<Location>(Location::MBase{});

        case LdMem:
          return isMBaseLoad(inst)
            ? make_optional<Location>(Location::MBase{})
            : std::nullopt;

        default:
          return std::nullopt;
      }
      not_reached();
    }();

    // If we're constraining guards, some instructions need certain information
    // to be recorded in side tables.
    if (l) {
      m_constraints.typeSrcs[inst] = m_state.typeSrcsOf(*l);
      if (!inst->is(LdLoc, LdStk) && !isMBaseLoad(inst)) {
        constrainLocation(*l, DataTypeGeneric, "appendInstruction");
        m_constraints.prevTypes[inst] = m_state.typeOf(*l);
      }
    }
  }

  // If the block isn't empty, check if we need to create a new block.
  if (!m_curBlock->empty()) {
    auto& prev = m_curBlock->back();
    if (prev.isBlockEnd()) {
      assertx(!prev.isTerminal());
      m_curBlock = m_unit.defBlock(prev.block()->profCount());
      m_curBlock->setHint(prev.block()->hint());
      prev.setNext(m_curBlock);
      m_state.finishBlock(prev.block());
      FTRACE(2, "lazily appending B{}\n", m_curBlock->id());
      m_state.startBlock(m_curBlock, false);
    }
  }

  assertx((m_curBlock->empty() || !m_curBlock->back().isBlockEnd()) &&
          "Can't append an instruction after a BlockEnd instruction");
  m_curBlock->push_back(inst);
  m_state.update(inst);
  if (inst->isTerminal()) m_state.finishBlock(m_curBlock);

  // If the instruction is block ending and introduces a bottom type, we are
  // now in an unreachable state.
  if (inst->isNextEdgeUnreachable() && inst->isBlockEnd()) {
    assertx(!inst->isTerminal());
    m_curBlock = m_unit.defBlock(1);
    m_curBlock->setHint(Block::Hint::Unused);
    inst->setNext(m_curBlock);
    m_state.finishBlock(inst->block());
    m_state.startBlock(m_curBlock, false);
    gen(Unreachable, ASSERT_REASON);
  }
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
  //      a) typeParam == Cell
  //      b) oldVal is from a DefConst
  //      c) oldType.hasConstVal()
  //    The AssertType will never be useful for guard constraining in these
  //     situations.
  // 2) The source instruction is known to be another assert that's at least
  //    as good as this one.
  if ((oldType <= newType &&
       (inst->typeParam() == TCell ||
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
      gen(Unreachable, ASSERT_REASON);
      return m_unit.cns(TBottom);
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
    // If FrameStateMgr's type for a local isn't as good as the type param,
    // we're missing information in the IR.
    assertx(inst->typeParam() >= type);
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

SSATmp* IRBuilder::preOptimizeLdMem(IRInstruction* inst) {
  auto const mbr = isMBaseLoad(inst) || inst->src(0) == m_state.mbr().ptr;
  return mbr ? preOptimizeLdLocation(inst, Location::MBase{}) : nullptr;
}

SSATmp* IRBuilder::preOptimizeLdMBase(IRInstruction* inst) {
  auto const type = m_state.mbase().type.lval(Ptr::Ptr);
  inst->setTypeParam(inst->typeParam() & type);

  if (auto const ptr = m_state.mbr().ptr) {
    if (ptr->isA(inst->typeParam())) return ptr;
    return gen(AssertType, inst->typeParam(), ptr);
  }
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdClosureCtx(IRInstruction* inst) {
  auto const closure = canonical(inst->src(0));
  if (!closure->inst()->is(ConstructClosure)) return nullptr;
  return gen(AssertType, inst->typeParam(), closure->inst()->src(0));
}

SSATmp* IRBuilder::preOptimizeLdClosureCls(IRInstruction* inst) {
  return preOptimizeLdClosureCtx(inst);
}

SSATmp* IRBuilder::preOptimizeLdClosureThis(IRInstruction* inst) {
  return preOptimizeLdClosureCtx(inst);
}

SSATmp* IRBuilder::preOptimizeLdFrameCtx(IRInstruction* inst) {
  auto const func = inst->marker().func();
  assertx(func->cls() || func->isClosureBody());
  if (auto ctx = m_state.ctx()) {
    assertx(!inst->is(LdFrameCls) || ctx->type() <= TCls);
    assertx(!inst->is(LdFrameThis) || ctx->type() <= TObj);
    if (ctx->inst()->is(DefConst)) return ctx;
    if (ctx->hasConstVal() ||
        ctx->type().subtypeOfAny(TInitNull, TUninit, TNullptr)) {
      return m_unit.cns(ctx->type());
    }
    return ctx;
  }

  if (inst->is(LdFrameCls)) {
    // ActRec->m_cls of a static function is always a valid class pointer with
    // the bottom bit set
    assertx(func->cls());
    if (func->cls()->attrs() & AttrNoOverride) {
      return m_unit.cns(func->cls());
    }
  }

  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdObjClass(IRInstruction* inst) {
  if (auto const spec = inst->src(0)->type().clsSpec()) {
    if (spec.exact() || spec.cls()->attrs() & AttrNoOverride) {
      return m_unit.cns(spec.cls());
    }
  }
  return nullptr;
}

SSATmp* IRBuilder::preOptimizeLdFrameThis(IRInstruction* inst) {
  return preOptimizeLdFrameCtx(inst);
}

SSATmp* IRBuilder::preOptimizeLdFrameCls(IRInstruction* inst) {
  return preOptimizeLdFrameCtx(inst);
}

SSATmp* IRBuilder::preOptimize(IRInstruction* inst) {
#define X(op) case op: return preOptimize##op(inst);
  switch (inst->op()) {
  X(AssertType)
  X(AssertLoc)
  X(AssertStk)
  X(CheckLoc)
  X(CheckStk)
  X(CheckMBase)
  X(LdLoc)
  X(LdStk)
  X(LdMem)
  X(LdMBase)
  X(LdClosureCls)
  X(LdClosureThis)
  X(LdFrameCls)
  X(LdFrameThis)
  X(LdObjClass)
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
      return inst->hasDst() ? preOpt : nullptr;
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
  assertx(m_state.bcSPOff() == curMarker().bcSPOff());
  m_exnStack.syncedSpLevel = m_state.bcSPOff();
  m_state.resetStackModified();
}

void IRBuilder::setCurMarker(const BCMarker& newMarker) {
  if (newMarker == curMarker()) return;
  FTRACE(2, "IRBuilder::setCurMarker:\n  old: {}\n  new: {}\n",
         curMarker().valid() ? curMarker().show() : "<invalid>",
         newMarker.show());
  assertx(newMarker.valid());
  m_curBCContext.marker = newMarker;
}

///////////////////////////////////////////////////////////////////////////////
// Guard relaxation.

bool IRBuilder::constrainGuard(const IRInstruction* inst, GuardConstraint gc) {
  if (!shouldConstrainGuards()) return false;

  auto& guard = m_constraints.guards[inst];
  auto newGc = applyConstraint(guard, gc);
  ITRACE(2, "constrainGuard({}, {}): {} -> {}\n", *inst, gc, guard, newGc);
  Indent _i;

  auto const changed = guard != newGc;
  if (changed && !gc.weak) guard = newGc;

  return changed;
}

bool IRBuilder::constrainValue(SSATmp* const val, GuardConstraint gc) {
  if (!shouldConstrainGuards() || gc.empty()) return false;

  if (!val) {
    ITRACE(1, "attempted to constrain nullptr SSATmp*; bailing\n", gc);
    return false;
  }
  auto inst = val->inst();

  ITRACE(1, "constraining {} to {}\n", *inst, gc);
  Indent _i;

  if (inst->is(LdLoc, LdStk) || isMBaseLoad(inst)) {
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
        } else if (inst->is(LdStk)) {
          ITRACE(1, "constraining guard for stack[{}]\n",
                 inst->extra<LdStk>()->offset.offset);
        } else {
          assertx(isMBaseLoad(inst));
          ITRACE(1, "constraining guard for mbase\n");
        }
      }
      changed |= constrainTypeSrc(typeSrc, gc);
    }
    return changed;
  }

  if (inst->is(AssertType)) {
    // Sometimes code in irgen asks for a value with DataTypeSpecific but can
    // tolerate a less specific value.  If that happens, there's nothing to
    // constrain.
    if (!typeFitsConstraint(val->type(), gc)) return false;

    return constrainAssert(inst, gc, inst->src(0)->type());
  }

  if (inst->is(CheckType)) {
    // Sometimes code in irgen asks for a value with DataTypeSpecific but can
    // tolerate a less specific value.  If that happens, there's nothing to
    // constrain.
    if (!typeFitsConstraint(val->type(), gc)) return false;

    return constrainCheck(inst, gc, inst->src(0)->type());
  }

  if (inst->isPassthrough()) {
    return constrainValue(inst->getPassthroughValue(), gc);
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
      changed |= constrainValue(src, gc);
    }
    return changed;
  }

  // Any instructions not special cased above produce a new value, so there's
  // no guard for us to constrain.
  ITRACE(2, "value is new in this trace, bailing\n");
  return false;
}

bool IRBuilder::constrainLocation(Location l, GuardConstraint gc,
                                  const std::string& why) {
  if (!shouldConstrainGuards() || gc.empty()) return false;

  ITRACE(1, "constraining {} to {} (for {})\n", show(l), gc, why);
  Indent _i;

  bool changed = false;
  for (auto typeSrc : m_state.typeSrcsOf(l)) {
    changed |= constrainTypeSrc(typeSrc, gc);
  }
  return changed;
}

bool IRBuilder::constrainLocation(Location l, GuardConstraint gc) {
  return constrainLocation(l, gc, "");
}

bool IRBuilder::constrainLocal(uint32_t locID, GuardConstraint gc,
                               const std::string& why) {
  return constrainLocation(loc(locID), gc, why);
}

bool IRBuilder::constrainStack(IRSPRelOffset offset, GuardConstraint gc) {
  return constrainLocation(stk(offset), gc);
}

bool IRBuilder::constrainTypeSrc(TypeSource typeSrc, GuardConstraint gc) {
  if (!shouldConstrainGuards() || gc.empty()) return false;

  ITRACE(1, "constraining type source {} to {}\n", show(typeSrc), gc);
  Indent _i;

  if (typeSrc.isValue()) return constrainValue(typeSrc.value, gc);

  assertx(typeSrc.isGuard());
  auto const guard = typeSrc.guard;

  always_assert(guard->is(AssertLoc,   CheckLoc,
                          AssertStk,   CheckStk,
                          AssertMBase, CheckMBase));

  // If the dest of the Assert/Check doesn't fit `gc', there's no point in
  // continuing.
  auto prevType = get_required(m_constraints.prevTypes, guard);
  if (!typeFitsConstraint(prevType & guard->typeParam(), gc)) {
    return false;
  }

  if (guard->is(AssertLoc, AssertStk, AssertMBase)) {
    return constrainAssert(guard, gc, prevType);
  }
  return constrainCheck(guard, gc, prevType);
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
                                GuardConstraint gc, Type srcType,
                                Optional<Type> knownType) {
  if (!knownType) knownType = inst->typeParam();

  // If the known type fits the constraint, we're done.
  if (typeFitsConstraint(*knownType, gc)) return false;

  auto const newGC = relaxConstraint(gc, *knownType, srcType);
  ITRACE(1, "tracing through {}, orig gc: {}, new gc: {}\n",
         *inst, gc, newGC);

  if (inst->is(AssertType, CheckType)) {
    return constrainValue(inst->src(0), newGC);
  }

  auto changed = false;
  auto const& typeSrcs = get_required(m_constraints.typeSrcs, inst);

  for (auto typeSrc : typeSrcs) {
    changed |= constrainTypeSrc(typeSrc, newGC);
  }
  return changed;
}

/*
 * Constrain the typeParam and sources of a Check instruction.
 */
bool IRBuilder::constrainCheck(const IRInstruction* inst,
                               GuardConstraint gc, Type srcType) {
  assertx(inst->is(CheckType, CheckLoc, CheckStk, CheckMBase));

  auto changed = false;
  auto const typeParam = inst->typeParam();

  // Constrain the guard on the Check instruction, but first relax the
  // constraint based on what's known about `srcType'.
  auto const guardGC = relaxConstraint(gc, srcType, typeParam);
  changed |= constrainGuard(inst, guardGC);

  // Relax typeParam with its current constraint.  This is used below to
  // recursively relax the constraint on the source, if needed.
  auto constraint = applyConstraint(m_constraints.guards[inst], guardGC);
  auto const knownType = relaxToConstraint(typeParam, constraint);

  changed |= constrainAssert(inst, gc, srcType, knownType);

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

LocalState IRBuilder::local(uint32_t id, GuardConstraint gc) {
  constrainLocal(id, gc, "");
  return m_state.local(id);
}

StackState IRBuilder::stack(IRSPRelOffset offset, GuardConstraint gc) {
  constrainStack(offset, gc);
  return m_state.stack(offset);
}

SSATmp* IRBuilder::valueOf(Location l, GuardConstraint gc) {
  constrainLocation(l, gc, "");
  return m_state.valueOf(l);
}

Type IRBuilder::typeOf(Location l, GuardConstraint gc) {
  constrainLocation(l, gc, "");
  return m_state.typeOf(l);
}

/*
 * Wrap a local or stack ID into a Location.
 */
Location IRBuilder::loc(uint32_t id) const {
  return Location::Local { id };
}
Location IRBuilder::stk(IRSPRelOffset off) const {
  auto const fpRel = off.to<SBInvOffset>(m_state.irSPOff());
  return Location::Stack { fpRel };
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
         m_state.show());
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

bool IRBuilder::hasBlock(SrcKey sk) const {
  return m_skToBlockMap.count(sk);
}

void IRBuilder::setBlock(SrcKey sk, Block* block) {
  assertx(!hasBlock(sk));
  m_skToBlockMap[sk] = block;
}

void IRBuilder::appendBlock(Block* block) {
  m_state.finishBlock(m_curBlock);
  FTRACE(2, "appending B{}\n", block->id());
  m_state.startBlock(block, false);
  m_curBlock = block;

  if (block->numPreds() == 0) {
    FTRACE(2, "Newly-appended B{} is unreachable!\n", block->id());
    gen(Unreachable, ASSERT_REASON);
  }
}

void IRBuilder::resetBlock(Block* block, Block* pred) {
  block->instrs().clear();
  m_state.resetBlock(block, pred);
}

void IRBuilder::resetOffsetMapping() {
  m_skToBlockMap.clear();
}

IRBuilder::SkToBlockMap IRBuilder::saveAndClearOffsetMapping() {
  return std::move(m_skToBlockMap);
}

void IRBuilder::restoreOffsetMapping(SkToBlockMap&& offsetMapping) {
  m_skToBlockMap = std::move(offsetMapping);
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

void IRBuilder::pushBlock(const BCMarker& marker, Block* b) {
  FTRACE(2, "IRBuilder::pushBlock:\n  saved: B{} @ {}\n pushed: B{} @ {}\n",
         m_curBlock->id(), curMarker().show(), b->id(), marker.show());
  assertx(b);

  m_savedBlocks.push_back(
    BlockState { m_curBlock, m_curBCContext, m_exnStack }
  );
  m_state.pauseBlock(m_curBlock);
  m_state.startBlock(b, false);
  m_curBlock = b;
  m_curBCContext = BCContext { marker, 0 };

  if (debug) {
    for (UNUSED auto const& state : m_savedBlocks) {
      assertx(state.block != b &&
             "Can't push a block that's already in the saved stack");
    }
  }
}

void IRBuilder::popBlock() {
  assertx(!m_savedBlocks.empty());

  auto const& top = m_savedBlocks.back();
  FTRACE(2, "IRBuilder::popBlock:\n  popped: B{} @ {}\n restored: B{} @ {}\n",
         m_curBlock->id(), curMarker().show(),
         top.block->id(), top.bcctx.marker.show());
  m_state.finishBlock(m_curBlock);
  m_state.unpauseBlock(top.block);
  m_curBlock = top.block;
  m_curBCContext = top.bcctx;
  m_exnStack = top.exnStack;
  m_savedBlocks.pop_back();
}

bool IRBuilder::inUnreachableState() const {
  return !m_curBlock->empty() && m_curBlock->back().isTerminal();
}

///////////////////////////////////////////////////////////////////////////////

}}}
