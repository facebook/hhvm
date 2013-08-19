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
#include "hphp/runtime/vm/jit/trace.h"
#include "hphp/runtime/vm/jit/trace-builder.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

/*
 * For all guard instructions in trace, check to see if we can relax the
 * destination type to something less specific. The GuardConstraints map
 * contains information about what properties of the guarded type matter for
 * each instruction.
 */
bool relaxGuards(IRTrace* trace, const IRFactory& factory,
                 const GuardConstraints& guards) {
  FTRACE(1, "relaxing guards for trace {}\n", trace);
  auto blocks = rpoSortCfg(trace, factory);
  Block* reflowBlock = nullptr;

  for (auto* block : blocks) {
    for (auto& inst : *block) {
      if (!isGuardOp(inst.op())) continue;

      auto it = guards.find(inst.id());
      auto category = it == guards.end() ? DataTypeGeneric : it->second;

      auto const oldType = inst.typeParam();
      auto newType = relaxType(oldType, category);

      if (!oldType.equals(newType)) {
        FTRACE(1, "relaxGuards changing {}'s type to {}\n", inst, newType);
        inst.setTypeParam(newType);
        if (!reflowBlock) reflowBlock = block;
      }
    }
  }

  // TODO(t2598894): For now we require regenerating the IR after guard
  // relaxation, so it's only useful in the tracelet region selector.
  if (false && reflowBlock) reflowTypes(reflowBlock, blocks);

  return (bool)reflowBlock;
}

/*
 * For every instruction in trace representing a tracelet guard, call func with
 * its location and type.
 */
void visitGuards(IRTrace* trace, const VisitGuardFn& func) {
  typedef RegionDesc::Location L;

  for (auto const& inst : *trace->front()) {
    if (inst.typeParam().equals(Type::Gen)) continue;

    if (inst.op() == GuardLoc) {
      func(L::Local{inst.extra<LocalId>()->locId}, inst.typeParam());
    } else if (inst.op() == GuardStk) {
      func(L::Stack{safe_cast<uint32_t>(inst.extra<StackOffset>()->offset)},
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
      return t.notCounted() ? Type::Uncounted : t;

    case DataTypeCountnessInit:
      if (t.subtypeOf(Type::Uninit)) return Type::Uninit;
      return relaxType(t, DataTypeCountness);

    case DataTypeSpecific:
      assert(t.isKnownDataType());
      return t.isSpecialized() ? t.unspecialize() : t;

    case DataTypeSpecialized:
      not_implemented();
      break;
  }

  not_reached();
}

} }
