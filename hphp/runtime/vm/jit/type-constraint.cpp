/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/type-constraint.h"

#include "hphp/runtime/base/datatype.h"

#include <folly/Format.h>

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

///////////////////////////////////////////////////////////////////////////////

std::string TypeConstraint::toString() const {
  std::string ret = "<" + typeCategoryName(category);

  if (category == DataTypeSpecialized) {
    if (wantArrayKind()) ret += ",ArrayKind";
    if (wantArrayShape()) ret += ",ArrayShape";
    if (wantClass()) {
      folly::toAppend("Cls:", desiredClass()->name()->data(), &ret);
    }
  }

  if (weak) ret += ",weak";

  return ret + '>';
}

///////////////////////////////////////////////////////////////////////////////

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

static void incCategory(DataTypeCategory& c) {
  always_assert(c != DataTypeSpecialized);
  c = static_cast<DataTypeCategory>(static_cast<uint8_t>(c) + 1);
}

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

    auto const relaxed = relaxType(toRelax, newTc.category);
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

///////////////////////////////////////////////////////////////////////////////

}}
