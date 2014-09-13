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
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);
using Trace::Indent;

bool shouldHHIRRelaxGuards() {
  return RuntimeOption::EvalHHIRRelaxGuards &&
    (RuntimeOption::EvalJitRegionSelector == "tracelet" ||
     mcg->tx().mode() == TransKind::Optimize);
}

/* For each possible dest type, determine if its type might relax. */
#define ND             always_assert(false);
#define D(t)           return false; // fixed type
#define DofS(n)        return typeMightRelax(inst->src(n));
#define DUnbox(n)      DofS(n)
#define DBox(n)        DofS(n)
#define DRefineS(n)    return true;  // typeParam may relax
#define DParam         return true;  // typeParam may relax
#define DLdRef         return true;  // typeParam may relax
#define DAllocObj      return false; // fixed type from ExtraData
#define DArrPacked     return false; // fixed type
#define DArrElem       assert(inst->is(LdPackedArrayElem));     \
  return typeMightRelax(inst->src(0));
#define DThis          return false; // fixed type from ctx class
#define DMulti         return true;  // DefLabel; value could be anything
#define DSetElem       return false; // fixed type
#define DStk(x)        x;
#define DBuiltin       return false; // from immutable typeParam
#define DSubtract(n,t) DofS(n)
#define DLdRaw         return false; // fixed type from ExtraData
#define DCns           return false; // fixed type

bool typeMightRelax(const SSATmp* tmp) {
  if (tmp == nullptr) return true;

  if (tmp->isA(Type::Cls) || tmp->type() == Type::Gen) return false;
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
#undef DUnbox
#undef DBox
#undef DRefineS
#undef DParam
#undef DLdRef
#undef DAllocObj
#undef DArrPacked
#undef DArrElem
#undef DThis
#undef DMulti
#undef DSetElem
#undef DStk
#undef DBuiltin
#undef DSubtract
#undef DLdRaw
#undef DCns

namespace {
/*
 * Given a load and the new type of that load's guard, update the type
 * of the load to match the relaxed type of the guard.
 */
void retypeLoad(IRInstruction* load, Type newType) {
  newType = load->is(LdLocAddr, LdStackAddr) ? newType.ptr() : newType;

  // Set new typeParam of 'load' if different from previous one,
  // but avoid doing it if newType is Bottom.  Note that we may end up
  // here with newType == Bottom, in case there's a type-check
  // instruction that is always going to fail but wasn't simplified
  // during IR generation.  In this case, this code is unreacheble and
  // will be eliminated later.
  if (!newType.equals(load->typeParam()) && newType != Type::Bottom) {
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
    case LdLocAddr:
    case LdGbl: {
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

Type relaxOuter(Type t, TypeConstraint tc) {
  assert(t.notBoxed());

  switch (tc.category) {
    case DataTypeGeneric:
      return Type::Gen;

    case DataTypeCountness:
      return t.notCounted() ? Type::Uncounted : t.unspecialize();

    case DataTypeCountnessInit:
      if (t <= Type::Uninit) return Type::Uninit;
      return (t.notCounted() && t.not(Type::Uninit))
        ? Type::UncountedInit : t.unspecialize();

    case DataTypeSpecific:
      return t.unspecialize();

    case DataTypeSpecialized:
      assert(tc.wantClass() ^ tc.wantArrayKind());

      if (tc.wantClass()) {
        // We could try to relax t's specialized class to tc.desiredClass() if
        // they're related but not the same, but we only support guarding on
        // final classes so the resulting guard would be bogus.
      } else {
        // t might have a RepoAuthType::Array that wasn't asked for in tc, but
        // RATArrays always come from static analysis and never guards, so we
        // don't need to eliminate it here. Just make sure t actually fits the
        // constraint.
        assert(t < Type::Arr && t.hasArrayKind());
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
  bool simple = flags & RelaxSimple;
  bool reflow = flags & RelaxReflow;
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
  if (!reflow) return true;

  // Make a second pass to reflow types, with some special logic for loads.
  FrameState state{unit, unit.entry()->front().marker()};
  for (auto* block : blocks) {
    ITRACE(2, "relaxGuards reflow entering B{}\n", block->id());
    Indent _i;
    state.startBlock(block, block->front().marker());

    for (auto& inst : *block) {
      state.setMarker(inst.marker());
      copyProp(&inst);
      visitLoad(&inst, state);
      retypeDests(&inst, &unit);
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
      // and Arr has non-specialized subtypes, so we require that t is
      // specialized, a strict subtype of Obj or Arr, and that it fits the
      // specific requirements of tc.

      assert(tc.wantClass() ^ tc.wantArrayKind());
      if (!t.isSpecialized()) return false;
      if (t < Type::Obj) {
        return tc.wantClass() && t.getClass()->classof(tc.desiredClass());
      }
      if (t < Type::Arr) {
        return tc.wantArrayKind() && t.hasArrayKind();
      }
      return false;
  }

  not_reached();
}

/*
 * Returns true iff t is not boxed, or if tc.innerCat is satisfied by t's inner
 * type.
 */
static bool typeFitsInnerConstraint(Type t, TypeConstraint tc) {
  return tc.innerCat == DataTypeGeneric || t.notBoxed() ||
    typeFitsOuterConstraint((t & Type::BoxedCell).innerType(), tc.inner());
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
  always_assert(t <= Type::Gen && t != Type::Bottom);
  if (tc.category == DataTypeGeneric) return Type::Gen;

  auto outerRelaxed = relaxOuter(t & Type::Cell, tc);
  if (t.notBoxed()) return outerRelaxed;

  auto innerType = (t & Type::BoxedCell).innerType();
  auto innerRelaxed = tc.innerCat == DataTypeGeneric ? Type::Cell
                                                     : relaxOuter(innerType,
                                                                  tc.inner());

  // Only add outerRelax into the result type if t had a meaningful outer type
  // coming in.
  return (t.isBoxed() ? Type::Bottom : outerRelaxed) |
    (innerRelaxed - Type::Uninit).box();
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
    if (newTc.isSpecialized()) {
      // We need to ask for the right kind of specialization, so grab it from
      // origTc.
      if (origTc.wantArrayKind()) newTc.setWantArrayKind();
      if (origTc.wantClass()) newTc.setDesiredClass(origTc.desiredClass());
    }

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

/*
 * Return a copy of tc refined with any new information in newTc.
 */
TypeConstraint applyConstraint(TypeConstraint tc, const TypeConstraint newTc) {
  tc.category = std::max(newTc.category, tc.category);
  tc.innerCat = std::max(newTc.innerCat, tc.innerCat);

  if (newTc.wantArrayKind()) tc.setWantArrayKind();

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
