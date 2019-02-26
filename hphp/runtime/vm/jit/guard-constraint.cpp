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

#include "hphp/runtime/vm/jit/guard-constraint.h"

#include "hphp/runtime/base/datatype.h"

#include <folly/Format.h>

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

///////////////////////////////////////////////////////////////////////////////

std::string GuardConstraint::toString() const {
  std::string ret = "<" + typeCategoryName(category);

  if (category == DataTypeSpecialized) {
    if (wantArrayKind()) ret += ",ArrayKind";
    if (wantClass()) {
      folly::toAppend("Cls:", desiredClass()->name()->data(), &ret);
    }
  }

  if (weak) ret += ",weak";

  return ret + '>';
}

///////////////////////////////////////////////////////////////////////////////

bool typeFitsConstraint(Type t, GuardConstraint gc) {
  switch (gc.category) {
    case DataTypeGeneric:
      return true;

    case DataTypeCountness:
    case DataTypeBoxAndCountness:
      // Consumers using this constraint expect the type to be relaxed to
      // Uncounted or left alone, so something like Arr|Obj isn't specific
      // enough.
      return !t.maybe(TCounted) ||
             t.subtypeOfAny(TStr, TArr, TVec, TDict, TKeyset, TObj,
                            TRes, TBoxedCell, TClsMeth);

    case DataTypeBoxAndCountnessInit:
      return typeFitsConstraint(t, DataTypeBoxAndCountness) &&
             (t <= TUninit || !t.maybe(TUninit));

    case DataTypeSpecific:
      return t.isKnownDataType();

    case DataTypeSpecialized:
      // Type::isSpecialized() returns true for types like {Arr<Packed>|Int}
      // and Arr has non-specialized subtypes, so we require that t is
      // specialized, a strict subtype of Obj or Arr, and that it fits the
      // specific requirements of gc.

      assertx(gc.wantClass() ^ gc.wantArrayKind());

      if (t < TObj && t.clsSpec()) {
        return gc.wantClass() &&
               t.clsSpec().cls()->classof(gc.desiredClass());
      }
      if (t < TArr && t.arrSpec()) {
        auto arrSpec = t.arrSpec();
        if (gc.wantArrayKind() && !arrSpec.kind()) return false;
        return true;
      }

      return false;
  }

  not_reached();
}

static void incCategory(DataTypeCategory& c) {
  always_assert(c != DataTypeSpecialized);
  c = static_cast<DataTypeCategory>(static_cast<uint8_t>(c) + 1);
}

GuardConstraint relaxConstraint(GuardConstraint origGc,
                                Type knownType, Type toRelax) {
  ITRACE(4, "relaxConstraint({}, knownType = {}, toRelax = {})\n",
         origGc, knownType, toRelax);
  Trace::Indent _i;

  // AssertType can be given TCtx, which should never relax.
  if (toRelax.maybe(TCctx)) {
    always_assert(toRelax <= TCtx);
    return origGc;
  }

  auto const dstType = knownType & toRelax;
  always_assert_flog(typeFitsConstraint(dstType, origGc),
                     "refine({}, {}) doesn't fit {}",
                     knownType, toRelax, origGc);

  // Preserve origGc's weak property.
  GuardConstraint newGc{DataTypeGeneric};
  newGc.weak = origGc.weak;

  while (true) {
    if (newGc.isSpecialized()) {
      // We need to ask for the right kind of specialization, so grab it from
      // origGc.
      if (origGc.wantArrayKind()) newGc.setWantArrayKind();
      if (origGc.wantClass()) newGc.setDesiredClass(origGc.desiredClass());
    }

    auto const relaxed = relaxType(toRelax, newGc.category);
    auto const newDstType = relaxed & knownType;
    if (typeFitsConstraint(newDstType, origGc)) break;

    ITRACE(5, "newDstType = {}, newGc = {}; incrementing constraint\n",
      newDstType, newGc);
    incCategory(newGc.category);
  }
  // DataTypeCountness can be relaxed to DataTypeGeneric in
  // optimizeProfiledGuards, so we can't rely on this category to give type
  // information through guards.  Since relaxConstraint is used to relax the
  // DataTypeCategory for guards, we cannot return DataTypeCountness unless we
  // already had it to start with.  Instead, we return DataTypeBoxCountness,
  // which won't be further relaxed by optimizeProfiledGuards.
  if (newGc.category == DataTypeCountness && origGc != DataTypeCountness) {
    newGc.category = DataTypeBoxAndCountness;
  }
  ITRACE(4, "Returning {}\n", newGc);
  // newGc shouldn't be any more specific than origGc.
  always_assert(newGc.category <= origGc.category);
  return newGc;
}

GuardConstraint applyConstraint(GuardConstraint gc,
                                GuardConstraint newGc) {
  gc.category = std::max(newGc.category, gc.category);

  if (newGc.wantArrayKind()) gc.setWantArrayKind();

  if (newGc.wantClass()) {
    if (gc.wantClass()) {
      // It only makes sense to constrain gc with a class that's related to its
      // existing class, and we want to preserve the more derived of the two.
      auto cls1 = gc.desiredClass();
      auto cls2 = newGc.desiredClass();
      gc.setDesiredClass(cls1->classof(cls2) ? cls1 : cls2);
    } else {
      gc.setDesiredClass(newGc.desiredClass());
    }
  }

  return gc;
}

///////////////////////////////////////////////////////////////////////////////

}}
