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
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);
using Trace::Indent;

bool shouldHHIRRelaxGuards() {
  assert(!(RuntimeOption::EvalHHIRRelaxGuards &&
           RuntimeOption::EvalHHIRConstrictGuards));
  return RuntimeOption::EvalHHIRRelaxGuards &&
    // TODO (#5792564): Guard relaxation doesn't work with loops.
    // TODO (#6599498): Guard relaxation is broken in wholecfg mode.
    (mcg->tx().mode() != TransKind::Optimize ||
     RuntimeOption::EvalJitPGORegionSelector == "hottrace");
}

/* For each possible dest type, determine if its type might relax. */
#define ND             always_assert(false);
#define D(t)           return false; // fixed type
#define DofS(n)        return typeMightRelax(inst->src(n));
#define DRefineS(n)    return true;  // typeParam may relax
#define DParamMayRelax return true;  // typeParam may relax
#define DParam         return false;
#define DParamPtr(k)   return false;
#define DUnboxPtr      return false;
#define DBoxPtr        return false;
#define DAllocObj      return false; // fixed type from ExtraData
#define DArrPacked     return false; // fixed type
#define DArrElem       assertx(inst->is(LdStructArrayElem, ArrayGet));    \
                         return typeMightRelax(inst->src(0));
#define DCol           return false; // fixed in bytecode
#define DThis          return false; // fixed type from ctx class
#define DCtx           return false;
#define DMulti         return true;  // DefLabel; value could be anything
#define DSetElem       return false; // fixed type
#define DBuiltin       return false; // from immutable typeParam
#define DSubtract(n,t) DofS(n)
#define DCns           return false; // fixed type

bool typeMightRelax(const SSATmp* tmp) {
  if (tmp == nullptr) return true;

  if (tmp->isA(TCls) || tmp->type() == TGen) return false;
  if (canonical(tmp)->inst()->is(DefConst)) return false;

  auto inst = tmp->inst();
  // Do the rest based on the opcode's dest type
  switch (inst->op()) {
#   define O(name, dst, src, flags) case name: dst
  IR_OPCODES
#   undef O
  }

  return true;
}

#undef ND
#undef D
#undef DofS
#undef DRefineS
#undef DParamMayRelax
#undef DParam
#undef DParamPtr
#undef DUnboxPtr
#undef DBoxPtr
#undef DAllocObj
#undef DArrPacked
#undef DArrElem
#undef DCol
#undef DThis
#undef DCtx
#undef DMulti
#undef DSetElem
#undef DBuiltin
#undef DSubtract
#undef DCns

namespace {
/*
 * Given a load and the new type of that load's guard, update the type
 * of the load to match the relaxed type of the guard.
 */
void retypeLoad(IRInstruction* load, Type newType) {
  // Set new typeParam of 'load' if different from previous one,
  // but avoid doing it if newType is Bottom.  Note that we may end up
  // here with newType == Bottom, in case there's a type-check
  // instruction that is always going to fail but wasn't simplified
  // during IR generation.  In this case, this code is unreacheble and
  // will be eliminated later.
  if (newType != load->typeParam() && newType != TBottom) {
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
void visitLoad(IRInstruction* inst, const FrameStateMgr& state) {
  switch (inst->op()) {
    case LdLoc: {
      auto const id = inst->extra<LdLoc>()->locId;
      auto const newType = state.localType(id);
      retypeLoad(inst, newType);
      break;
    }

    case LdStk: {
      auto idx = inst->extra<LdStk>()->offset;
      auto newType = state.stackType(idx);
      // We know from hhbc invariants that stack slots are always either Cls or
      // Gen flavors---there's no need to relax beyond that.
      if (newType == TStkElem) {
        newType = inst->typeParam() <= TGen ? TGen :
                  inst->typeParam() <= TCls ? TCls :
                  TStkElem;
      }
      retypeLoad(inst, newType);
      break;
    }

    default: break;
  }
}

Type relaxCell(Type t, TypeConstraint tc) {
  assertx(t <= TCell);

  switch (tc.category) {
    case DataTypeGeneric:
      return TGen;

    case DataTypeCountness:
      return !t.maybe(TCounted) ? TUncounted : t.unspecialize();

    case DataTypeCountnessInit:
      if (t <= TUninit) return TUninit;
      return (!t.maybe(TCounted) && !t.maybe(TUninit))
        ? TUncountedInit : t.unspecialize();

    case DataTypeSpecific:
      return t.unspecialize();

    case DataTypeSpecialized:
      assertx(tc.wantClass() ^ tc.wantArrayKind());

      if (tc.wantClass()) {
        // We could try to relax t's specialized class to tc.desiredClass() if
        // they're related but not the same, but we only support guarding on
        // final classes so the resulting guard would be bogus.
      } else {
        // t might have a RepoAuthType::Array that wasn't asked for in tc, but
        // RATArrays always come from static analysis and never guards, so we
        // don't need to eliminate it here. Just make sure t actually fits the
        // constraint.
        assertx(t < TArr && t.arrSpec().kind());
        assertx(!tc.wantArrayShape() || t.arrSpec().shape());
      }

      return t;
  }

  not_reached();
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
                 RelaxGuardsFlags flags) {
  Timer _t(Timer::optimize_relaxGuards);
  ITRACE(2, "entering relaxGuards\n");
  Indent _i;
  bool const simple = flags & RelaxSimple;
  bool const reflow = flags & RelaxReflow;
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
  if (!reflow) return true;

  // Make a second pass to reflow types, with some special logic for loads.
  FrameStateMgr state{unit.entry()->front().marker()};
  state.setLegacyReoptimize();

  for (auto block : blocks) {
    ITRACE(2, "relaxGuards reflow entering B{}\n", block->id());
    Indent _i;
    state.startBlock(block);

    for (auto& inst : *block) {
      copyProp(&inst);
      visitLoad(&inst, state);
      retypeDests(&inst, &unit);
      state.update(&inst);
    }

    state.finishBlock(block);
  }

  return true;
}


bool typeFitsConstraint(Type t, TypeConstraint tc) {
  switch (tc.category) {
    case DataTypeGeneric:
      return true;

    case DataTypeCountness:
      // Consumers using this constraint expect the type to be relaxed to
      // Uncounted or left alone, so something like Arr|Obj isn't specific
      // enough.
      return !t.maybe(TCounted) ||
             t.subtypeOfAny(TStr, TArr, TObj,
                            TRes, TBoxedCell);

    case DataTypeCountnessInit:
      return typeFitsConstraint(t, DataTypeCountness) &&
             (t <= TUninit || !t.maybe(TUninit));

    case DataTypeSpecific:
      return t.isKnownDataType();

    case DataTypeSpecialized:
      // Type::isSpecialized() returns true for types like {Arr<Packed>|Int}
      // and Arr has non-specialized subtypes, so we require that t is
      // specialized, a strict subtype of Obj or Arr, and that it fits the
      // specific requirements of tc.

      assertx(tc.wantClass() ^ tc.wantArrayKind());

      if (t < TObj && t.clsSpec()) {
        return tc.wantClass() &&
               t.clsSpec().cls()->classof(tc.desiredClass());
      }
      if (t < TArr && t.arrSpec()) {
        auto arrSpec = t.arrSpec();
        if (tc.wantArrayShape() && !arrSpec.shape()) return false;
        if (tc.wantArrayKind() && !arrSpec.kind()) return false;
        return true;
      }

      return false;
  }

  not_reached();
}

/*
 * Returns the least specific supertype of t that maintains the properties
 * required by tc.
 */
Type relaxType(Type t, TypeConstraint tc) {
  always_assert_flog(t <= TGen && t != TBottom, "t = {}", t);
  if (tc.category == DataTypeGeneric) return TGen;
  auto const relaxed =
    (t & TCell) <= TBottom ? TBottom : relaxCell(t & TCell, tc);
  return t <= TCell ? relaxed : relaxed | TBoxedInitCell;
}

static void incCategory(DataTypeCategory& c) {
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

  auto const dstType = knownType & toRelax;
  always_assert_flog(typeFitsConstraint(dstType, origTc),
                     "refine({}, {}) doesn't fit {}",
                     knownType, toRelax, origTc);

  // Preserve origTc's weak property.
  TypeConstraint newTc{DataTypeGeneric};
  newTc.weak = origTc.weak;

  while (true) {
    if (newTc.isSpecialized()) {
      // We need to ask for the right kind of specialization, so grab it from
      // origTc.
      if (origTc.wantArrayKind()) newTc.setWantArrayKind();
      if (origTc.wantArrayShape()) newTc.setWantArrayShape();
      if (origTc.wantClass()) newTc.setDesiredClass(origTc.desiredClass());
    }

    auto const relaxed = relaxType(toRelax, newTc);
    auto const newDstType = relaxed & knownType;
    if (typeFitsConstraint(newDstType, origTc)) break;

    ITRACE(5, "newDstType = {}, newTc = {}; incrementing constraint\n",
      newDstType, newTc);
    incCategory(newTc.category);
  }

  ITRACE(4, "Returning {}\n", newTc);
  // newTc shouldn't be any more specific than origTc.
  always_assert(newTc.category <= origTc.category);
  return newTc;
}

/*
 * Return a copy of tc refined with any new information in newTc.
 */
TypeConstraint applyConstraint(TypeConstraint tc, const TypeConstraint newTc) {
  tc.category = std::max(newTc.category, tc.category);

  if (newTc.wantArrayKind()) tc.setWantArrayKind();
  if (newTc.wantArrayShape()) tc.setWantArrayShape();

  if (newTc.wantClass()) {
    if (tc.wantClass()) {
      // It only makes sense to constrain tc with a class that's related to its
      // existing class, and we want to preserve the more derived of the two.
      auto cls1 = tc.desiredClass();
      auto cls2 = newTc.desiredClass();
      tc.setDesiredClass(cls1->classof(cls2) ? cls1 : cls2);
    } else {
      tc.setDesiredClass(newTc.desiredClass());
    }
  }

  return tc;
}

} }
