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
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/ir-trace.h"
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

/*
 * For all guard instructions in trace, check to see if we can relax the
 * destination type to something less specific. The GuardConstraints map
 * contains information about what properties of the guarded type matter for
 * each instruction. Returns true iff any changes were made to the trace.
 */
bool relaxGuards(const IRUnit& unit, const GuardConstraints& guards) {
  FTRACE(1, "relaxing guards for trace {}\n", unit.main());
  auto blocks = rpoSortCfg(unit);
  Block* reflowBlock = nullptr;

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
        FTRACE(1, "relaxGuards changing {}'s type to {}, op to {}\n",
               inst, constraint.knownType, newOp);
        inst.setTypeParam(constraint.knownType);
        inst.setOpcode(newOp);
        inst.setTaken(nullptr);

        if (!reflowBlock) reflowBlock = block;
      } else if (!oldType.equals(newType)) {
        FTRACE(1, "relaxGuards changing {}'s type to {}\n", inst, newType);
        inst.setTypeParam(newType);

        if (!reflowBlock) reflowBlock = block;
      }
    }
  }

  if (reflowBlock) reflowTypes(reflowBlock, blocks);

  return (bool)reflowBlock;
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
        t.subtypeOf(Type::Counted | Type::StaticArr | Type::StaticStr);

    case DataTypeCountnessInit:
      return typeFitsConstraint(t, DataTypeCountness) &&
        (t.subtypeOf(Type::Uninit) || t.not(Type::Uninit));

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
  if (Type::Gen.subtypeOf(t)) return DataTypeGeneric;
  if (Type::Uncounted.subtypeOf(t) || t.isCounted()) return DataTypeCountness;
  if (Type::UncountedInit.subtypeOf(t)) return DataTypeCountnessInit;
  return t.isSpecialized() ? DataTypeSpecialized : DataTypeSpecific;
}

/*
 * Returns the least specific supertype of t that maintains the properties
 * required by cat.
 */
Type relaxType(Type t, DataTypeCategory cat) {
  always_assert(t.subtypeOf(Type::Gen));

  switch (cat) {
    case DataTypeGeneric:
      return Type::Gen;

    case DataTypeCountness:
      return t.notCounted() ? Type::Uncounted : t.unspecialize();

    case DataTypeCountnessInit:
      if (t.subtypeOf(Type::Uninit)) return Type::Uninit;
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
