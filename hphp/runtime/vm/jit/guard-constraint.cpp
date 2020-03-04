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
    if (wantArrayKind()) {
      ret += ",ArrayKind";
    } else if (wantVanillaArray()) {
      ret += ",VanillaArray";
    } else if (wantClass()) {
      folly::toAppend("Cls:", desiredClass()->name()->data(), &ret);
    }
    if (wantRecord()) {
      folly::toAppend("Rec:", desiredRecord()->name()->data(), &ret);
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
      // When we use this constraint, we expect the type to either be relaxed
      // to Uncounted or left alone, so if counted, it must have a DataType.
      return t.isKnownDataType() || !t.maybe(TCounted);

    case DataTypeCountnessInit:
      return typeFitsConstraint(t, DataTypeCountness) &&
             (t <= TUninit || !t.maybe(TUninit));

    case DataTypeSpecific:
      return t.isKnownDataType();

    case DataTypeSpecialized:
      // Type::isSpecialized() returns true for types like {Arr<Packed>|Int},
      // so we need to check both for specialization and isKnownDataType.
      assertx(gc.wantClass() + gc.wantVanillaArray() + gc.wantRecord() == 1);
      if (!t.isKnownDataType()) return false;
      if (gc.wantClass()) {
        auto const clsSpec = t.clsSpec();
        return clsSpec && clsSpec.cls()->classof(gc.desiredClass());
      } else if (gc.wantRecord()) {
        auto const recSpec = t.recSpec();
        return recSpec && recSpec.rec()->recordDescOf(gc.desiredRecord());
      } else {
        if (gc.wantArrayKind()) return !!t.arrSpec().kind();
        if (gc.wantVanillaArray()) return t.arrSpec().vanilla();
      }
      return false;
  }
  always_assert(false);
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

  auto const dstType = knownType & toRelax;
  always_assert_flog(typeFitsConstraint(dstType, origGc),
                     "refine({}, {}) doesn't fit {}",
                     knownType, toRelax, origGc);

  // Preserve origGc's weak property.
  GuardConstraint newGc{DataTypeGeneric};
  newGc.weak = origGc.weak;
  Type newDstType{};

  while (true) {
    if (newGc.isSpecialized()) {
      // Check if a vanilla constraint is sufficient before proceeding to kind.
      if (RO::EvalAllowBespokeArrayLikes && origGc.wantVanillaArray()) {
        newGc.setWantVanillaArray();
        newDstType = knownType & relaxToConstraint(toRelax, newGc);
        if (typeFitsConstraint(newDstType, origGc)) break;
        ITRACE(5, "newDstType = {}, newGc = {}; vanilla is insufficient\n",
               newDstType, newGc);
      }
      // Include the remainder of the specialization from origGc.
      if (origGc.wantArrayKind()) newGc.setWantArrayKind();
      if (origGc.wantClass()) newGc.setDesiredClass(origGc.desiredClass());
      if (origGc.wantRecord()) newGc.setDesiredRecord(origGc.desiredRecord());
    }

    newDstType = knownType & relaxToConstraint(toRelax, newGc);
    if (typeFitsConstraint(newDstType, origGc)) break;
    ITRACE(5, "newDstType = {}, newGc = {}; incrementing constraint\n",
           newDstType, newGc);
    incCategory(newGc.category);
  }
  // DataTypeCountness can be relaxed to DataTypeGeneric in
  // optimizeProfiledGuards, so we can't rely on this category to give type
  // information through guards.  Since relaxConstraint is used to relax the
  // DataTypeCategory for guards, we cannot return DataTypeCountness unless we
  // already had it to start with.  Instead, we return DataTypeCountnessInit,
  // which won't be further relaxed by optimizeProfiledGuards.
  if (newGc.category == DataTypeCountness && origGc != DataTypeCountness) {
    newGc.category = DataTypeCountnessInit;
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
  if (newGc.wantVanillaArray()) gc.setWantVanillaArray();

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

  if (newGc.wantRecord()) {
    if (gc.wantRecord()) {
      // It only makes sense to constrain gc with a record that's related to its
      // existing record, and we want to preserve the more derived of the two.
      auto rec1 = gc.desiredRecord();
      auto rec2 = newGc.desiredRecord();
      gc.setDesiredRecord(rec1->recordDescOf(rec2) ? rec1 : rec2);
    } else {
      gc.setDesiredRecord(newGc.desiredRecord());
    }
  }

  return gc;
}

///////////////////////////////////////////////////////////////////////////////

}}
