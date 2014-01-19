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

#include "hphp/runtime/vm/jit/guard-relaxation.h"

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/frame-state.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-trace.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/trace-builder.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

/*
 * Trace back through the source of fp, looking for a guard with the
 * given locId. If one can't be found, return nullptr.
 */
IRInstruction* guardForLocal(uint32_t locId, SSATmp* fp) {
  FTRACE(2, "guardForLdLoc({}, {})\n", locId, *fp);

  for (auto fpInst = fp->inst(); !fpInst->is(DefFP, DefInlineFP);
       fpInst = fpInst->src(0)->inst()) {
    FTRACE(2, "    - fp = {}\n", *fpInst);
    assert(fpInst->dst()->isA(Type::FramePtr));
    auto instLoc = [fpInst]{ return fpInst->extra<LocalId>()->locId; };

    switch (fpInst->op()) {
      case GuardLoc:
      case CheckLoc:
        if (instLoc() == locId) return fpInst;
        break;

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
 * guard instruction but have no direct reference to that guard. This block
 * only changes the load's type param; the loop afterwards will retype the dest
 * if needed.
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
      auto typeSrc = getStackValue(inst->src(0), idx).typeSrc;
      if (typeSrc->is(GuardStk, CheckStk, AssertStk)) {
        retypeLoad(inst, typeSrc->typeParam());
      }
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
 * Returns the best known type of the value being guarded by guard, from right
 * before guard.
 */
Type previousGuardType(const IRInstruction* guard) {
  switch (guard->op()) {
    case GuardLoc:
    case CheckLoc: {
      auto* prevGuard = guardForLocal(guard->extra<LocalId>()->locId,
                                      guard->src(0));
      if (!prevGuard) return Type::Gen;
      return prevGuard->typeParam();
    }

    case GuardStk:
    case CheckStk: {
      auto stkVal = getStackValue(guard->src(0),
                                  guard->extra<StackOffset>()->offset);
      return stkVal.knownType;
    }

    case CheckType:
      return guard->src(0)->type();

    default:
      not_reached();
  }
}
}

/*
 * For all guard instructions in trace, check to see if we can relax the
 * destination type to something less specific. The GuardConstraints map
 * contains information about what properties of the guarded type matter for
 * each instruction. Returns true iff any changes were made to the trace.
 */
bool relaxGuards(IRUnit& unit, const GuardConstraints& guards) {
  FTRACE(1, "relaxing guards for trace {}\n", unit.main());
  auto blocks = rpoSortCfg(unit);
  auto changed = false;

  for (auto* block : blocks) {
    for (auto& inst : *block) {
      if (!isGuardOp(inst.op())) continue;

      auto it = guards.find(&inst);
      auto constraint = it == guards.end() ? TypeConstraint() : it->second;

      // TODO(t2598894): Support relaxing inner types
      auto const oldType = inst.typeParam();
      auto newType = relaxType(oldType, constraint.category);

      if (constraint.knownType <= newType) {
        // If the known type is at least as good as the relaxed type, we can
        // replace the guard with an assert.
        auto newOp = guardToAssert(inst.op());
        auto newType = std::min(constraint.knownType, previousGuardType(&inst));
        FTRACE(1, "relaxGuards changing {}'s type to {}, op to {}\n",
               inst, newType, newOp);
        assert(!hasEdges(newOp));
        if (inst.hasEdges()) {
          block->push_back(unit.gen(Jmp, inst.marker(), inst.next()));
        }
        inst.setTypeParam(newType);
        inst.setOpcode(newOp);
        changed = true;
      } else if (!oldType.equals(newType)) {
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
      visitLoad(&inst, state);
      retypeDests(&inst);
      state.update(&inst);
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
    if (inst.typeParam().equals(Type::Gen)) continue;

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
 * Returns true iff t is specific enough to fit cat.
 */
bool typeFitsConstraint(Type t, DataTypeCategory cat) {
  switch (cat) {
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
 * Returns the most general category 'cat' that satisfies this expression:
 * relaxType(t, categoryForType(t)) == t
 */
DataTypeCategory categoryForType(Type t) {
  if (t >= Type::Gen) return DataTypeGeneric;
  if (t >= Type::Uncounted || t.isCounted()) return DataTypeCountness;
  if (t >= Type::UncountedInit) return DataTypeCountnessInit;
  return t.isSpecialized() ? DataTypeSpecialized : DataTypeSpecific;
}

/*
 * Returns the least specific supertype of t that maintains the properties
 * required by cat.
 */
Type relaxType(Type t, DataTypeCategory cat) {
  always_assert(t <= Type::Gen);

  switch (cat) {
    case DataTypeGeneric:
      return Type::Gen;

    case DataTypeCountness:
      return t.notCounted() ? Type::Uncounted : t.unspecialize();

    case DataTypeCountnessInit:
      if (t <= Type::Uninit) return Type::Uninit;
      return t.notCounted() ? Type::UncountedInit : t.unspecialize();

    case DataTypeSpecific:
      assert(t.isKnownDataType());
      return t.unspecialize();

    case DataTypeSpecialized:
      assert(t.isSpecialized());
      return t;
  }

  not_reached();
}

} }
