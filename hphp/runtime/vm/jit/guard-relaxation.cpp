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

#include "hphp/runtime/vm/jit/guard-relaxation.h"

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/frame-state.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);
using Trace::Indent;

bool shouldHHIRRelaxGuards() {
  return RuntimeOption::EvalHHIRRelaxGuards &&
    (RuntimeOption::EvalJitRegionSelector == "tracelet" ||
     tx->mode() == TransKind::Optimize);
}

/*
 * Trace back through the source of fp, looking for a guard with the
 * given locId. If one can't be found, return nullptr.
 */
IRInstruction* guardForLocal(uint32_t locId, SSATmp* fp) {
  ITRACE(2, "guardForLocal({}, {})\n", locId, *fp);
  Indent _i;

  for (auto fpInst = fp->inst(); !fpInst->is(DefFP, DefInlineFP);
       fpInst = fpInst->src(0)->inst()) {
    ITRACE(2, "fp = {}\n", *fpInst);
    assert(fpInst->dst()->isA(Type::FramePtr));
    auto instLoc = [fpInst]{ return fpInst->extra<LocalId>()->locId; };

    switch (fpInst->op()) {
      case GuardLoc:
      case CheckLoc:
      case AssertLoc:
        if (instLoc() == locId) return fpInst;
        break;

      case FreeActRec:
        always_assert(0 && "Attempt to read a local after freeing its frame");

      default:
        not_reached();
    }
  }

  return nullptr;
}

namespace {
/*
 * Given a load and the new type of that load's guard, update the type
 * of the load to match the relaxed type of the guard.
 */
void retypeLoad(IRInstruction* load, Type newType) {
  newType = load->is(LdLocAddr, LdStackAddr) ? newType.ptr() : newType;

  if (!newType.equals(load->typeParam())) {
    ITRACE(2, "retypeLoad changing type param of {} to {}\n",
           *load, newType);
    load->setTypeParam(newType);
  }
}

/*
 * Loads from locals and the stack are special: they get their type from a
 * guard instruction but have no direct reference to that guard. This function
 * only changes the load's type param; the caller is responsible for retyping
 * the dest if needed.
 */
void visitLoad(IRInstruction* inst, const FrameState& state) {
  switch (inst->op()) {
    case LdLoc:
    case LdLocAddr: {
      auto const id = inst->extra<LocalId>()->locId;
      auto const newType = state.localType(id);

      retypeLoad(inst, newType);
      break;
    }

    case LdStack:
    case LdStackAddr: {
      auto idx = inst->extra<StackOffset>()->offset;
      auto newType = getStackValue(inst->src(0), idx).knownType;

      retypeLoad(inst, newType);
      break;
    }

    case LdRef: {
      auto inner = inst->src(0)->type().innerType();
      auto param = inst->typeParam();
      assert(inner.maybe(param));

      // If the type of the src has been relaxed past the LdRef's type param,
      // update the type param.
      if (inner > param) {
        inst->setTypeParam(inner);
      }
      break;
    }

    default: break;
  }
}

/*
 * If inst is a guard/assert instruction that can be removed with its newly
 * relaxed typeParam, do so and return true. Otherwise, return false.
 */
bool removeGuard(IRUnit& unit, IRInstruction* inst, const FrameState& state) {
  Type prevType;
  switch (inst->op()) {
    case GuardLoc:
    case CheckLoc:
    case AssertLoc:
      prevType = state.localType(inst->extra<LocalId>()->locId);
      break;

    case GuardStk:
    case CheckStk:
    case AssertStk:
      prevType = getStackValue(inst->src(0),
                               inst->extra<StackOffset>()->offset).knownType;
      break;

    case CheckType:
    case AssertType:
      prevType = inst->src(0)->type();
      break;

    default:
      return false;
  }

  ITRACE(2, "removeGuard inspecting {}\n", *inst);
  auto type = inst->typeParam();
  if (type < prevType) return false;

  if (!(type >= prevType)) {
    // Neither is a subtype of the other. If they have no intersection the
    // guard will always fail but we can let the simplifier take care of
    // that.
    return false;
  }

  ITRACE(2, "replacing {} with Mov due to prevType {}\n", *inst, prevType);
  if (inst->isControlFlow()) {
    // We can't replace CF instructions with a Mov, so stick a Mov in front of
    // it and convert it to a Jmp to the next block.
    auto* block = inst->block();
    block->insert(block->iteratorTo(inst),
                  unit.mov(inst->dst(), inst->src(0), inst->marker()));
    inst->setTaken(inst->next());
    inst->convertToJmp();
  } else {
    inst->convertToMov();
  }
  return true;
}

Type relaxInner(Type t, TypeConstraint tc) {
  if (t.notBoxed()) return t;

  auto cell = t & Type::Cell;
  auto inner = (t & Type::BoxedCell).innerType();
  auto innerCat = tc.innerCat;

  auto innerRelaxed = innerCat == DataTypeGeneric ? Type::Cell
                                                  : relaxType(inner, innerCat);
  return cell | (innerRelaxed - Type::Uninit).box();
}
}

/*
 * For all guard instructions in unit, check to see if we can relax the
 * destination type to something less specific. The GuardConstraints map
 * contains information about what properties of the guarded type matter for
 * each instruction. If simple is true, guards will not be relaxed past
 * DataTypeSpecific except guards which are relaxed all the way to
 * DataTypeGeneric. Returns true iff any changes were made to the trace.
 */
bool relaxGuards(IRUnit& unit, const GuardConstraints& constraints,
                 bool simple) {
  Timer _t(Timer::optimize_relaxGuards);
  ITRACE(2, "entering relaxGuards\n");
  Indent _i;

  splitCriticalEdges(unit);
  auto& guards = constraints.guards;
  auto blocks = rpoSortCfg(unit);
  auto changed = false;

  for (auto* block : blocks) {
    for (auto& inst : *block) {
      if (!isGuardOp(inst.op())) continue;

      auto it = guards.find(&inst);
      auto constraint = it == guards.end() ? TypeConstraint() : it->second;
      ITRACE(2, "relaxGuards processing {} with constraint {}\n",
             inst, constraint);

      auto simplifyCategory = [simple](DataTypeCategory& cat) {
        if (simple && cat > DataTypeGeneric && cat < DataTypeSpecific) {
          cat = DataTypeSpecific;
        }
      };
      simplifyCategory(constraint.category);
      simplifyCategory(constraint.innerCat);

      auto const oldType = inst.typeParam();
      auto newType = relaxType(oldType, constraint);

      if (oldType != newType) {
        ITRACE(1, "relaxGuards changing {}'s type to {}\n", inst, newType);
        inst.setTypeParam(newType);
        changed = true;
      }
    }
  }

  if (!changed) return false;

  // Make a second pass to reflow types, with some special logic for loads.
  FrameState state{unit, unit.entry()->front().marker()};
  for (auto* block : blocks) {
    state.startBlock(block);

    for (auto& inst : *block) {
      state.setMarker(inst.marker());
      copyProp(&inst);
      visitLoad(&inst, state);
      if (!removeGuard(unit, &inst, state)) {
        retypeDests(&inst);
        state.update(&inst);
      }
    }

    state.finishBlock(block);
  }

  return true;
}

/*
 * For every instruction in trace representing a tracelet guard, call func with
 * its location and type.
 */
void visitGuards(IRUnit& unit, const VisitGuardFn& func) {
  typedef RegionDesc::Location L;

  for (auto const& inst : *unit.entry()) {
    if (inst.hasTypeParam() && inst.typeParam().equals(Type::Gen)) continue;

    if (inst.op() == GuardLoc) {
      func(L::Local{inst.extra<LocalId>()->locId}, inst.typeParam());
    } else if (inst.op() == GuardStk) {
      uint32_t offsetFromSp =
        safe_cast<uint32_t>(inst.extra<StackOffset>()->offset);
      uint32_t offsetFromFp = inst.marker().spOff() - offsetFromSp;
      func(L::Stack{offsetFromSp, offsetFromFp},
           inst.typeParam());
    }
  }
}

/*
 * Returns true iff tc.category is satisfied by t.
 */
static bool typeFitsOuterConstraint(Type t, TypeConstraint tc) {
  switch (tc.category) {
    case DataTypeGeneric:
      return true;

    case DataTypeCountness:
      // Consumers using this constraint expect the type to be relaxed to
      // Uncounted or left alone, so something like Arr|Obj isn't specific
      // enough.
      return t.notCounted() ||
             t.subtypeOfAny(Type::Str, Type::Arr, Type::Obj,
                            Type::Res, Type::BoxedCell);

    case DataTypeCountnessInit:
      return typeFitsOuterConstraint(t, DataTypeCountness) &&
             (t <= Type::Uninit || t.not(Type::Uninit));

    case DataTypeSpecific:
      return t.isKnownDataType();

    case DataTypeSpecialized:
      // Type::isSpecialized() returns true for types like {Arr<Packed>|Int}
      // and Arr has non-specialized subtypes, so we check that t is both
      // specialized and a strict subtype of Obj or Arr.
      return t.isSpecialized() && (t < Type::Obj || t < Type::Arr);
  }

  not_reached();
}

/*
 * Returns true iff t is not boxed, or if tc.innerCat is satisfied by t's inner
 * type.
 */
static bool typeFitsInnerConstraint(Type t, TypeConstraint tc) {
  return tc.innerCat == DataTypeGeneric || t.notBoxed() ||
    typeFitsOuterConstraint((t & Type::BoxedCell).innerType(), tc.innerCat);
}

/*
 * Returns true iff t is specific enough to fit tc, meaning a consumer
 * constraining a value with tc would be satisfied with t as the value's type
 * after relaxation.
 */
bool typeFitsConstraint(Type t, TypeConstraint tc) {
  always_assert(t != Type::Bottom);
  return typeFitsInnerConstraint(t, tc) &&
    typeFitsOuterConstraint(t, tc);
}

/*
 * Returns the least specific supertype of t that maintains the properties
 * required by tc.
 */
Type relaxType(Type t, TypeConstraint tc) {
  always_assert(t <= Type::Gen);

  switch (tc.category) {
    case DataTypeGeneric:
      return Type::Gen;

    case DataTypeCountness:
      return t.notCounted() ? Type::Uncounted
                            : relaxInner(t.unspecialize(), tc);

    case DataTypeCountnessInit:
      if (t <= Type::Uninit) return Type::Uninit;
      return (t.notCounted() && t.not(Type::Uninit))
        ? Type::UncountedInit : relaxInner(t.unspecialize(), tc);

    case DataTypeSpecific:
      return relaxInner(t.unspecialize(), tc);

    case DataTypeSpecialized:
      return relaxInner(t, tc);
  }

  not_reached();
}

void incCategory(DataTypeCategory& c) {
  always_assert(c != DataTypeSpecialized);
  c = static_cast<DataTypeCategory>(static_cast<uint8_t>(c) + 1);
}

/*
 * relaxConstraint returns the least specific TypeConstraint 'tc' that doesn't
 * prevent the intersection of knownType and relaxType(toRelax, tc) from
 * satisfying origTc. It is used in IRBuilder::constrainValue and
 * IRBuilder::constrainStack to determine how to constrain the typeParam and
 * src values of CheckType/CheckStk instructions, and the src values of
 * AssertType/AssertStk instructions.
 *
 * AssertType example:
 * t24:Obj<C> = AssertType<{Obj<C>|InitNull}> t4:Obj
 *
 * If constrainValue is called with (t24, DataTypeSpecialized), relaxConstraint
 * will be called with (DataTypeSpecialized, Obj<C>|InitNull, Obj). After a few
 * iterations it will determine that constraining Obj with DataTypeCountness
 * will still allow the result type of the AssertType instruction to satisfy
 * DataTypeSpecialized, because relaxType(Obj, DataTypeCountness) == Obj.
 */
TypeConstraint relaxConstraint(const TypeConstraint origTc,
                               const Type knownType, const Type toRelax) {
  ITRACE(4, "relaxConstraint({}, knownType = {}, toRelax = {})\n",
         origTc, knownType, toRelax);
  Trace::Indent _i;

  auto const dstType = refineType(knownType, toRelax);
  always_assert_flog(typeFitsConstraint(dstType, origTc),
                     "refine({}, {}) doesn't fit {}",
                     knownType, toRelax, origTc);

  // Preserve origTc's weak property.
  TypeConstraint newTc{DataTypeGeneric, DataTypeGeneric};
  newTc.weak = origTc.weak;

  while (true) {
    auto const relaxed = relaxType(toRelax, newTc);
    auto const newDstType = refineType(relaxed, knownType);
    if (typeFitsConstraint(newDstType, origTc)) break;

    ITRACE(5, "newDstType = {}, newTc = {}; ", newDstType, newTc);
    if (newTc.category == DataTypeGeneric ||
        !typeFitsOuterConstraint(newDstType, origTc)) {
      FTRACE(5, "incrementing outer\n");
      incCategory(newTc.category);
    } else if (!typeFitsInnerConstraint(newDstType, origTc)) {
      FTRACE(5, "incrementing inner\n");
      incCategory(newTc.innerCat);
    } else {
      not_reached();
    }
  }

  ITRACE(4, "Returning {}\n", newTc);
  // newTc shouldn't be any more specific than origTc.
  always_assert(newTc.category <= origTc.category &&
                newTc.innerCat <= origTc.innerCat);
  return newTc;
}

} }
