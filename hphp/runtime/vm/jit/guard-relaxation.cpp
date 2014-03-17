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

/*
 * Trace back through the source of fp, looking for a guard with the
 * given locId. If one can't be found, return nullptr.
 */
IRInstruction* guardForLocal(uint32_t locId, SSATmp* fp) {
  FTRACE(2, "guardForLocal({}, {})\n", locId, *fp);

  for (auto fpInst = fp->inst(); !fpInst->is(DefFP, DefInlineFP);
       fpInst = fpInst->src(0)->inst()) {
    FTRACE(2, "    - fp = {}\n", *fpInst);
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
    FTRACE(2, "retypeLoad changing type param of {} to {}\n",
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
      auto const id = inst->extra<LocalData>()->locId;
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

  FTRACE(2, "removeGuard inspecting {}\n", *inst);
  auto type = inst->typeParam();
  if (type < prevType) return false;

  if (!(type >= prevType)) {
    // Neither is a subtype of the other. If they have no intersection the
    // guard will always fail but we can let the simplifier take care of
    // that.
    return false;
  }

  FTRACE(2, "replacing {} with Mov due to prevType {}\n", *inst, prevType);
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
bool relaxGuards(IRUnit& unit, const GuardConstraints& guards, bool simple) {
  Timer _t(Timer::optimize_relaxGuards);

  splitCriticalEdges(unit);
  auto blocks = rpoSortCfg(unit);
  auto changed = false;

  for (auto* block : blocks) {
    for (auto& inst : *block) {
      if (!isGuardOp(inst.op())) continue;

      auto it = guards.find(&inst);
      auto constraint = it == guards.end() ? TypeConstraint() : it->second;
      FTRACE(2, "relaxGuards processing {} with constraint {}\n",
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

      // Sometimes we (legitimately) end up with a guard like this:
      //
      // t4:StkPtr = GuardStk<BoxedArr,0,<DataTypeGeneric,
      //                                  inner:DataTypeSpecific,
      //                                  Type::BoxedCell>> t2:StkPtr
      //
      // The outer category is DataTypeGeneric because we know from eval stack
      // flavors that the top of the stack here is always boxed. The inner
      // category is DataTypeSpecific, indicating we care what the inner type
      // is, even though it's just a hint. If we treated this like any other
      // guard, we would relax the typeParam to Type::Gen and insert an assert
      // to Type::BoxedCell right after it. Unfortunately, this loses the hint
      // that the inner type is Arr. Eventually we should have some side
      // channel for passing around hints for inner ref types, but for now the
      // best we can do is forcibly keep the guard around, preserving the inner
      // type hint.
      if (constraint.assertedType.isBoxed() &&
          oldType < constraint.assertedType) {
        auto relaxedInner = relaxInner(oldType, constraint);

        if (relaxedInner < Type::BoxedCell && newType >= Type::BoxedCell) {
          FTRACE(1, "relaxGuards changing newType to {}\n", newType);
          newType = relaxedInner;
        }
      }

      if (constraint.assertedType < newType) {
        // If the asserted type is more specific than the new guarded type, set
        // the guard to the relaxed type but insert an assert operation between
        // the instruction and its dst. We go from something like this:
        //
        // t5:FramePtr = GuardLoc<Int, 4, <DataTypeGeneric,Int>> t4:FramePtr
        //
        // to this:
        //
        // t6:FramePtr = GuardLoc<Gen, 4> t4:FramePtr
        // t5:FramePtr = AssertLoc<Int, 4> t6:FramePtr

        auto* oldDst = inst.dst();
        auto* newDst = unit.genDst(&inst);
        auto* newAssert = [&] {
          switch (inst.op()) {
            case GuardLoc:
            case CheckLoc:
              return unit.genWithDst(oldDst,
                                     guardToAssert(inst.op()),
                                     inst.marker(),
                                     *inst.extra<LocalId>(),
                                     constraint.assertedType,
                                     newDst);

            case GuardStk:
            case CheckStk:
              return unit.genWithDst(oldDst,
                                     guardToAssert(inst.op()),
                                     inst.marker(),
                                     *inst.extra<StackOffset>(),
                                     constraint.assertedType,
                                     newDst);

            case CheckType:
              return unit.genWithDst(oldDst,
                                     guardToAssert(inst.op()),
                                     inst.marker(),
                                     constraint.assertedType,
                                     newDst);

            default: always_assert(false);
          }
        }();

        FTRACE(1, "relaxGuards inserting {} between {} and its dst, "
               "changing typeParam to {}\n",
               *newAssert, inst, newType);
        inst.setTypeParam(newType);

        // Now, insert the assert after the guard. For control flow guards,
        // this means inserting it on the next edge.
        if (inst.isControlFlow()) {
          auto* block = inst.next();
          block->insert(block->skipHeader(), newAssert);
        } else {
          auto* block = inst.block();
          auto it = block->iteratorTo(&inst);
          ++it;
          block->insert(it, newAssert);
        }

        changed = true;
      } else if (oldType != newType) {
        FTRACE(1, "relaxGuards changing {}'s type to {}\n", inst, newType);
        inst.setTypeParam(newType);
        changed = true;
      }
    }
  }

  if (!changed) return false;

  // Make a second pass to reflow types, with some special logic for loads.
  FrameState state(unit);
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
      uint32_t offsetFromFp = inst.marker().spOff - offsetFromSp;
      func(L::Stack{offsetFromSp, offsetFromFp},
           inst.typeParam());
    }
  }
}

/*
 * Returns true iff t is specific enough to fit tc, meaning a consumer
 * constraining a value with tc would be satisfied with t as the value's type
 * after relaxation.
 */
bool typeFitsConstraint(Type t, TypeConstraint tc) {
  assert(t != Type::Bottom);

  if (tc.innerCat > DataTypeGeneric) {
    // First check the outer constraint.
    if (!typeFitsConstraint(t, tc.category)) return false;

    // Then, if t might be boxed, check the inner type.
    return t.notBoxed() ||
      typeFitsConstraint((t & Type::BoxedCell).innerType(), tc.innerCat);
  }

  switch (tc.category) {
    case DataTypeGeneric:
      return true;

    case DataTypeCountness:
      // Consumers using this constraint are probably going to decref the
      // value, so it's ok if we know whether t is counted or not. Arr and Str
      // are special cased because we don't guard on staticness for them.
      return t.notCounted() ||
             t <= (Type::Counted | Type::StaticArr | Type::StaticStr);

    case DataTypeCountnessInit:
      return typeFitsConstraint(t, DataTypeCountness) &&
             (t <= Type::Uninit || t.not(Type::Uninit));

    case DataTypeSpecific:
      return t.isKnownDataType();

    case DataTypeSpecialized:
      return t.isSpecialized();
  }

  not_reached();
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
      return t.notCounted() ? Type::UncountedInit
                            : relaxInner(t.unspecialize(), tc);

    case DataTypeSpecific:
      assert(t.isKnownDataType());
      return relaxInner(t.unspecialize(), tc);

    case DataTypeSpecialized:
      assert(t.isSpecialized());
      return relaxInner(t, tc);
  }

  not_reached();
}

} }
