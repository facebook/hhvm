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
#include "hphp/runtime/base/tv-comparisons.h"

#include <type_traits>

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/tv-conv-notice.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/ext/datetime/ext_datetime.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/rclass-meth-data.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////
namespace collections {
extern bool equals(const ObjectData*, const ObjectData*);
}
//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * Strict equality check between two vecs, accounting for bespoke vecs.
 */
bool vecSameHelper(const ArrayData* ad1, const ArrayData* ad2) {
  assertx(ad1->isVecType());
  assertx(ad2->isVecType());
  return ArrayData::bothVanilla(ad1, ad2)
    ? PackedArray::VecSame(ad1, ad2)
    : ArrayData::Same(ad1, ad2);
}

// helpers for coercion logging

template<class Op> constexpr bool isEqualityOp();

template<class Op>
void handleConvNotice(const std::string& lhs, const std::string& rhs) {
  if constexpr (isEqualityOp<Op>()) {
    handleConvNoticeForEq(lhs.c_str(), rhs.c_str());
  } else {
    handleConvNoticeForCmp(lhs.c_str(), rhs.c_str());
  }
}


/*
 * Return whether two DataTypes for primitive types are "equivalent" including
 * testing for in-progress migrations
 */
bool equivDataTypesIncludingMigrations(DataType t1, DataType t2) {
  if (equivDataTypes(t1, t2)) return true;
  if (RO::EvalIsCompatibleClsMethType &&
      ((t1 == KindOfClsMeth && equivDataTypes(t2, KindOfVec)) ||
        (equivDataTypes(t1, KindOfVec) && t2 == KindOfClsMeth))) {
    return true;
  }

  if (!RO::EvalRaiseClassConversionWarning) {
    const auto isStringOrClassish = [](DataType t) {
      return isStringType(t) || isClassType(t) || isLazyClassType(t);
    };
    return isStringOrClassish(t1) && isStringOrClassish(t2);
  }
  return false;
}

template<class Op> bool shouldMaybeTriggerConvNotice(
    DataType d1, DataType d2, typename Op::RetType res) {
  if (equivDataTypesIncludingMigrations(d1, d2)) return false;
  // if eq op, only notice if the comparison was true
  if constexpr (isEqualityOp<Op>()) return res;
  // only applies to comparison ops
  if ((d1 == KindOfInt64  && d2 == KindOfDouble) ||
      (d1 == KindOfDouble && d2 == KindOfInt64)) return false;
  return true;
}

/*
 * Family of relative op functions.
 *
 * These are used to implement the common parts of the php operators
 * ==, <, and >.  They handle some of the php behavior with regard to
 * numeric-ish strings, and delegate to the 'op' functor to perform
 * the actual comparison on primitive types, and between complex php
 * types of the same type.
 *
 * See below for the implementations of the Op template parameter.
 */

template<class Op>
typename Op::RetType tvRelOpNull(Op op, TypedValue cell) {
  if (isStringType(cell.m_type)) {
    return op(cell.m_data.pstr, staticEmptyString());
  } else if (cell.m_type == KindOfObject) {
    return op(true, false);
  } else {
    return tvRelOp(op, cell, false);
  }
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, bool val) {
  if (UNLIKELY(isArrayLikeType(cell.m_type))) {
    return op(cell.m_data.parr, val);
  } else if (UNLIKELY(isClsMethType(cell.m_type))) {
    return op.clsmethVsNonClsMeth();
  } else if (UNLIKELY(isFuncType(type(cell)))) {
    return op.funcVsNonFunc();
  } else if (UNLIKELY(isRFuncType(cell.m_type))) {
    return op(cell.m_data.prfunc, val);
  } else if (UNLIKELY(isRClsMethType(cell.m_type))) {
    return op(cell.m_data.prclsmeth, val);
  } else {
    return op(tvToBool(cell), val);
  }
}

template<class Op, typename Num>
auto strRelOp(Op op, TypedValue cell, Num val, const StringData* str) {
  auto const num = stringToNumeric(str);
  return num.m_type == KindOfInt64 ? op(num.m_data.num, val) :
         num.m_type == KindOfDouble ? op(num.m_data.dbl, val) :
         op(0, val);
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, int64_t val) {
  assertx(tvIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(false, !!val);

    case KindOfBoolean:
      return op(!!cell.m_data.num, val != 0);

    case KindOfInt64:
      return op(cell.m_data.num, val);

    case KindOfDouble:
      return op(cell.m_data.dbl, val);

    case KindOfPersistentString:
    case KindOfString:
      return strRelOp(op, cell, val, cell.m_data.pstr);

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfObject:
      if (cell.m_data.pobj->isCollection()) return op.collectionVsNonObj();
      return op(cell.m_data.pobj->toInt64(), val);

    case KindOfResource:
      return op(cell.m_data.pres->data()->o_toInt64(), val);

    case KindOfFunc:
      return op.funcVsNonFunc();

    case KindOfClass:
      return strRelOp(op, cell, val, classToStringHelper(cell.m_data.pclass));

    case KindOfLazyClass:
      return strRelOp(op, cell, val,
                      lazyClassToStringHelper(cell.m_data.plazyclass));

    case KindOfClsMeth:
      return op.clsmethVsNonClsMeth();

    case KindOfRClsMeth:
      return op.rclsMethVsNonRClsMeth();

    case KindOfRFunc:
      return op.rfuncVsNonRFunc();

    case KindOfRecord:
      return op.recordVsNonRecord();
  }
  not_reached();
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, double val) {
  assertx(tvIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(false, val != 0);

    case KindOfBoolean:
       return op(!!cell.m_data.num, val != 0);

    case KindOfInt64:
      return op(cell.m_data.num, val);

    case KindOfDouble:
      return op(cell.m_data.dbl, val);

    case KindOfPersistentString:
    case KindOfString:
      return strRelOp(op, cell, val, cell.m_data.pstr);

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfObject:
      if (cell.m_data.pobj->isCollection()) return op.collectionVsNonObj();
      return op(cell.m_data.pobj->toDouble(), val);

    case KindOfResource:
      return op(cell.m_data.pres->data()->o_toDouble(), val);

    case KindOfFunc:
      return op.funcVsNonFunc();

    case KindOfClass:
      return strRelOp(op, cell, val, classToStringHelper(cell.m_data.pclass));

    case KindOfLazyClass:
      return strRelOp(op, cell, val,
                      lazyClassToStringHelper(cell.m_data.plazyclass));

    case KindOfClsMeth:
      return op.clsmethVsNonClsMeth();

    case KindOfRClsMeth:
      return op.rclsMethVsNonRClsMeth();

    case KindOfRFunc:
      return op.rfuncVsNonRFunc();

    case KindOfRecord:
      return op.recordVsNonRecord();
  }
  not_reached();
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, const StringData* val) {
  assertx(tvIsPlausible(cell));
  assertx(val != nullptr);

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(staticEmptyString(), val);

    case KindOfInt64: {
       auto const num = stringToNumeric(val);
      return num.m_type == KindOfInt64  ? op(cell.m_data.num, num.m_data.num) :
             num.m_type == KindOfDouble ? op(cell.m_data.num, num.m_data.dbl) :
             op(cell.m_data.num, 0);
    }
    case KindOfBoolean:
      return op(!!cell.m_data.num, val->toBoolean());

    case KindOfDouble: {
      auto const num = stringToNumeric(val);
      return num.m_type == KindOfInt64  ? op(cell.m_data.dbl, num.m_data.num) :
             num.m_type == KindOfDouble ? op(cell.m_data.dbl, num.m_data.dbl) :
             op(cell.m_data.dbl, 0);
    }

    case KindOfPersistentString:
    case KindOfString:
      return op(cell.m_data.pstr, val);

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfObject: {
      auto od = cell.m_data.pobj;
      if (od->isCollection()) return op.collectionVsNonObj();
      if (od->hasToString()) {
        String str(od->invokeToString());
        return op(str.get(), val);
      }
      return op(true, false);
    }

    case KindOfResource: {
      auto const rd = cell.m_data.pres;
      return op(rd->data()->o_toDouble(), val->toDouble());
    }

    case KindOfFunc:
      return op.funcVsNonFunc();

    case KindOfClass:
      return op(classToStringHelper(cell.m_data.pclass), val);

    case KindOfLazyClass:
      return op(lazyClassToStringHelper(cell.m_data.plazyclass), val);

    case KindOfClsMeth:
      return op.clsmethVsNonClsMeth();

    case KindOfRClsMeth:
      return op.rclsMethVsNonRClsMeth();

    case KindOfRFunc:
      return op.rfuncVsNonRFunc();

    case KindOfRecord:
      return op.recordVsNonRecord();
  }
  not_reached();
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, const ObjectData* od) {
  assertx(tvIsPlausible(cell));

  auto strRelOp = [&] (const StringData* sd) {
    auto obj = const_cast<ObjectData*>(od);
    if (obj->isCollection()) return op.collectionVsNonObj();
    if (obj->hasToString()) {
      String str(obj->invokeToString());
      return op(sd, str.get());
    }
    return op(false, true);
  };

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(false, true);

    case KindOfBoolean:
      return op(!!cell.m_data.num, od->toBoolean());

    case KindOfInt64:
      if (od->isCollection()) return op.collectionVsNonObj();
      {
        // do the conversion first since it throws most of the time
        const auto res = op(cell.m_data.num, od->toInt64());
        return res;
      }

    case KindOfDouble:
      if (od->isCollection()) return op.collectionVsNonObj();
      {
        // do the conversion first since it throws most of the time
        const auto res = op(cell.m_data.dbl, od->toDouble());
        return res;
      }

    case KindOfPersistentString:
    case KindOfString:
      return strRelOp(cell.m_data.pstr);

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfObject:
      return op(cell.m_data.pobj, od);

    case KindOfResource:
      return op(false, true);

    case KindOfFunc:
      return op.funcVsNonFunc();

    case KindOfClass:
      return strRelOp(classToStringHelper(cell.m_data.pclass));

    case KindOfLazyClass:
      return strRelOp(lazyClassToStringHelper(cell.m_data.plazyclass));

    case KindOfClsMeth:
      return op.clsmethVsNonClsMeth();

    case KindOfRClsMeth:
      return op.rclsMethVsNonRClsMeth();

    case KindOfRFunc:
      return op.rfuncVsNonRFunc();

    case KindOfRecord:
      return op.recordVsNonRecord();
  }
  not_reached();
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, const RecordData* rec) {
  if (cell.m_type != KindOfRecord) {
    op.recordVsNonRecord();
  }
  return op(cell.m_data.prec, rec);
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, const ResourceData* rd) {
  assertx(tvIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(false, true);

    case KindOfBoolean:
      return op(!!cell.m_data.num, rd->o_toBoolean());

    case KindOfInt64:
      return op(cell.m_data.num, rd->o_toInt64());

    case KindOfDouble:
      return op(cell.m_data.dbl, rd->o_toDouble());

    case KindOfPersistentString:
    case KindOfString: {
      auto const str = cell.m_data.pstr;
      return op(str->toDouble(), rd->o_toDouble());
    }

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfObject:
      return op(true, false);

    case KindOfResource:
      return op(cell.m_data.pres->data(), rd);

    case KindOfFunc:
      return op.funcVsNonFunc();

    case KindOfClass: {
      auto const str = classToStringHelper(cell.m_data.pclass);
      return op(str->toDouble(), rd->o_toDouble());
    }

    case KindOfLazyClass: {
      auto const str = lazyClassToStringHelper(cell.m_data.plazyclass);
      return op(str->toDouble(), rd->o_toDouble());
    }

    case KindOfClsMeth:
      return op.clsmethVsNonClsMeth();

    case KindOfRClsMeth:
      return op.rclsMethVsNonRClsMeth();

    case KindOfRFunc:
      return op.rfuncVsNonRFunc();

    case KindOfRecord:
      return op.recordVsNonRecord();
  }
  not_reached();
}
template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, const ResourceHdr* r) {
  return tvRelOp(op, cell, r->data());
}

template<class Op>
typename Op::RetType tvRelOpVec(Op op, TypedValue cell, const ArrayData* a) {
  assertx(tvIsPlausible(cell));
  assertx(a->isVecType());

  if (isClsMethType(cell.m_type) && RO::EvalIsCompatibleClsMethType) {
    raiseClsMethToVecWarningHelper();
    return op.vec(clsMethToVecHelper(cell.m_data.pclsmeth).get(), a);
  }

  if (UNLIKELY(!isVecType(cell.m_type))) {
    if (cell.m_type == KindOfBoolean) return op(!!cell.m_data.num, a);
    if (cell.m_type == KindOfNull) return op(false, a);
    if (isDictType(cell.m_type)) return op.dictVsNonDict();
    if (isKeysetType(cell.m_type)) return op.keysetVsNonKeyset();
    if (isClsMethType(cell.m_type)) return op.clsmethVsNonClsMeth();
    return op.vecVsNonVec();
  }

  return op.vec(cell.m_data.parr, a);
}

template<class Op>
typename Op::RetType tvRelOpDict(Op op, TypedValue cell, const ArrayData* a) {
  assertx(tvIsPlausible(cell));
  assertx(a->isDictType());

  if (UNLIKELY(!isDictType(cell.m_type))) {
    if (cell.m_type == KindOfBoolean) return op(!!cell.m_data.num, a);
    if (cell.m_type == KindOfNull) return op(false, a);
    if (isVecType(cell.m_type)) return op.vecVsNonVec();
    if (isKeysetType(cell.m_type)) return op.keysetVsNonKeyset();
    if (isClsMethType(cell.m_type)) return op.clsmethVsNonClsMeth();
    return op.dictVsNonDict();
  }

  return op.dict(cell.m_data.parr, a);
}

template<class Op>
typename Op::RetType tvRelOpKeyset(Op op, TypedValue cell, const ArrayData* a) {
  assertx(tvIsPlausible(cell));
  assertx(a->isKeysetType());

  if (UNLIKELY(!isKeysetType(cell.m_type))) {
    if (cell.m_type == KindOfBoolean) return op(!!cell.m_data.num, a);
    if (cell.m_type == KindOfNull) return op(false, a);
    if (isVecType(cell.m_type)) return op.vecVsNonVec();
    if (isDictType(cell.m_type)) return op.dictVsNonDict();
    if (isClsMethType(cell.m_type)) return op.clsmethVsNonClsMeth();
    return op.keysetVsNonKeyset();
  }

  return op.keyset(cell.m_data.parr, a);
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, ClsMethDataRef clsMeth) {
  assertx(tvIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfFunc:
    case KindOfClass:
    case KindOfLazyClass:
    case KindOfResource:
    case KindOfBoolean:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfObject:
      return op.clsmethVsNonClsMeth();

    case KindOfClsMeth:  return op(cell.m_data.pclsmeth, clsMeth);

    case KindOfPersistentVec:
    case KindOfVec: {
      if (!RO::EvalIsCompatibleClsMethType) {
        return op.clsmethVsNonClsMeth();
      } else {
        raiseClsMethToVecWarningHelper();
        return op.vec(cell.m_data.parr, clsMethToVecHelper(clsMeth).get());
      }
    }

    case KindOfRClsMeth:
      return op.rclsMethVsNonRClsMeth();

    case KindOfRFunc:
      return op.rfuncVsNonRFunc();

    case KindOfRecord:
      return op.recordVsNonRecord();
  }
  not_reached();
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, RClsMethData* rclsmeth) {
  assertx(tvIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfFunc:
    case KindOfRFunc:
    case KindOfClass:
    case KindOfLazyClass:
    case KindOfResource:
    case KindOfClsMeth:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfObject:
    case KindOfRecord:
    case KindOfBoolean:
      return op.rclsMethVsNonRClsMeth();

    case KindOfRClsMeth:
      return op(cell.m_data.prclsmeth, rclsmeth);
  }
  not_reached();
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, RFuncData* rfunc) {
  assertx(tvIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfFunc:
    case KindOfClass:
    case KindOfLazyClass:
    case KindOfResource:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfObject:
    case KindOfRecord:
    case KindOfBoolean:
      return op.rfuncVsNonRFunc();

    case KindOfRFunc:
      return op(cell.m_data.prfunc, rfunc);
  }
  not_reached();
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, const Func* val) {
  assertx(tvIsPlausible(cell));
  assertx(val != nullptr);

  if (UNLIKELY(!isFuncType(type(cell)))) {
    if (isClsMethType(cell.m_type)) return op.clsmethVsNonClsMeth();
    return op.funcVsNonFunc();
  }

  return op(cell.m_data.pfunc, val);
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, LazyClassData val) {
  assertx(tvIsPlausible(cell));
  assertx(val.name() != nullptr && val.name()->isStatic());

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(staticEmptyString(), lazyClassToStringHelper(val));

    case KindOfInt64: {
      auto const num = stringToNumeric(lazyClassToStringHelper(val));
      return num.m_type == KindOfInt64  ? op(cell.m_data.num, num.m_data.num) :
             num.m_type == KindOfDouble ? op(cell.m_data.num, num.m_data.dbl) :
             op(cell.m_data.num, 0);
    }
    case KindOfBoolean:
      return op(!!cell.m_data.num, true);

    case KindOfDouble: {
      auto const num = stringToNumeric(lazyClassToStringHelper(val));
      return num.m_type == KindOfInt64  ? op(cell.m_data.dbl, num.m_data.num) :
             num.m_type == KindOfDouble ? op(cell.m_data.dbl, num.m_data.dbl) :
             op(cell.m_data.dbl, 0);
    }

    case KindOfPersistentString:
    case KindOfString:
      return op(cell.m_data.pstr, lazyClassToStringHelper(val));

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfObject: {
      auto od = cell.m_data.pobj;
      if (od->isCollection()) return op.collectionVsNonObj();
      if (od->hasToString()) {
        String str(od->invokeToString());
        return op(str.get(), lazyClassToStringHelper(val));
      }
      return op(true, false);
    }

    case KindOfResource: {
      auto const rd = cell.m_data.pres;
      return op(rd->data()->o_toDouble(),
                lazyClassToStringHelper(val)->toDouble());
    }

    case KindOfFunc:
      return op.funcVsNonFunc();

    case KindOfClass:
      return op(cell.m_data.pclass, val);

    case KindOfLazyClass:
      return op(cell.m_data.plazyclass.name(), val.name());

    case KindOfClsMeth:
      return op.clsmethVsNonClsMeth();

    case KindOfRClsMeth:
      return op.rclsMethVsNonRClsMeth();

    case KindOfRFunc:
      return op.rfuncVsNonRFunc();

    case KindOfRecord:
      return op.recordVsNonRecord();
  }
  not_reached();
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue cell, const Class* val) {
  assertx(tvIsPlausible(cell));
  assertx(val != nullptr);

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(staticEmptyString(), classToStringHelper(val));

    case KindOfInt64: {
      auto const num = stringToNumeric(classToStringHelper(val));
      return num.m_type == KindOfInt64  ? op(cell.m_data.num, num.m_data.num) :
             num.m_type == KindOfDouble ? op(cell.m_data.num, num.m_data.dbl) :
             op(cell.m_data.num, 0);
    }
    case KindOfBoolean:
      return op(!!cell.m_data.num, true);

    case KindOfDouble: {
      auto const num = stringToNumeric(classToStringHelper(val));
      return num.m_type == KindOfInt64  ? op(cell.m_data.dbl, num.m_data.num) :
             num.m_type == KindOfDouble ? op(cell.m_data.dbl, num.m_data.dbl) :
             op(cell.m_data.dbl, 0);
    }

    case KindOfPersistentString:
    case KindOfString:
      return op(cell.m_data.pstr, classToStringHelper(val));

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfObject: {
      auto od = cell.m_data.pobj;
      if (od->isCollection()) return op.collectionVsNonObj();
      if (od->hasToString()) {
        String str(od->invokeToString());
        return op(str.get(), classToStringHelper(val));
      }
      return op(true, false);
    }

    case KindOfResource: {
      auto const rd = cell.m_data.pres;
      return op(rd->data()->o_toDouble(), classToStringHelper(val)->toDouble());
    }

    case KindOfFunc:
      return op.funcVsNonFunc();

    case KindOfClass:
      return op(cell.m_data.pclass, val);

    case KindOfLazyClass:
      return op(cell.m_data.plazyclass, val);

    case KindOfClsMeth:
      return op.clsmethVsNonClsMeth();

    case KindOfRClsMeth:
      return op.rclsMethVsNonRClsMeth();

    case KindOfRFunc:
      return op.rfuncVsNonRFunc();

    case KindOfRecord:
      return op.recordVsNonRecord();
  }
  not_reached();
}

struct Eq;

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue c1, TypedValue c2) {
  assertx(tvIsPlausible(c1));
  assertx(tvIsPlausible(c2));

  if (std::is_same_v<Op, Eq> && useStrictEquality() &&
      !equivDataTypesIncludingMigrations(c1.m_type, c2.m_type)) {
    return false;
  }

  const auto res = [&](){
    switch (c2.m_type) {
    case KindOfUninit:
    case KindOfNull:         return tvRelOpNull(op, c1);
    case KindOfInt64:        return tvRelOp(op, c1, c2.m_data.num);
    case KindOfBoolean:      return tvRelOp(op, c1, !!c2.m_data.num);
    case KindOfDouble:       return tvRelOp(op, c1, c2.m_data.dbl);
    case KindOfPersistentString:
    case KindOfString:       return tvRelOp(op, c1, c2.m_data.pstr);
    case KindOfPersistentVec:
    case KindOfVec:          return tvRelOpVec(op, c1, c2.m_data.parr);
    case KindOfPersistentDict:
    case KindOfDict:         return tvRelOpDict(op, c1, c2.m_data.parr);
    case KindOfPersistentKeyset:
    case KindOfKeyset:       return tvRelOpKeyset(op, c1, c2.m_data.parr);
    case KindOfObject:       return tvRelOp(op, c1, c2.m_data.pobj);
    case KindOfResource:     return tvRelOp(op, c1, c2.m_data.pres);
    case KindOfRFunc:        return tvRelOp(op, c1, c2.m_data.prfunc);
    case KindOfFunc:         return tvRelOp(op, c1, c2.m_data.pfunc);
    case KindOfClass:        return tvRelOp(op, c1, c2.m_data.pclass);
    case KindOfLazyClass:    return tvRelOp(op, c1, c2.m_data.plazyclass);
    case KindOfClsMeth:      return tvRelOp(op, c1, c2.m_data.pclsmeth);
    case KindOfRClsMeth:     return tvRelOp(op, c1, c2.m_data.prclsmeth);
    case KindOfRecord:       return tvRelOp(op, c1, c2.m_data.prec);
    }
    not_reached();
  }();
  // put it after in case the original comparison throws
  if (shouldMaybeTriggerConvNotice<Op>(c1.m_type, c2.m_type, res)) {
    handleConvNotice<Op>(
      describe_actual_type(&c1), describe_actual_type(&c2));
  }
  return res;
}

/*
 * These relative ops helper function objects define operator() for
 * each primitive type, and for the case of a complex type being
 * compared with itself (that is obj with obj, string with string,
 * array with array).
 *
 * They must also define a function called collectionVsNonObj() which
 * is used when comparing collections with non-object types.  (The obj
 * vs obj function should handle the collection vs collection and
 * collection vs non-collection object cases.)  This is just to handle
 * that php operator == returns false in these cases, while the Lt/Gt
 * operators throw an exception.
 */

struct Eq {
  using RetType = bool;

  template<class T, class U>
  typename std::enable_if<
    !std::is_pointer<T>::value &&
    !std::is_pointer<U>::value,
    bool
  >::type operator()(T t, U u) const { return t == u; }

  bool operator()(const StringData* sd1, const StringData* sd2) const {
    return sd1->equal(sd2);
  }
  bool operator()(const ArrayData* ad, bool val) const {
    return !ad->empty() == val;
  }
  bool operator()(bool val, const ArrayData* ad) const {
    return val == !ad->empty();
  }

  bool operator()(const Func* f1, const Func* f2) const { return f1 == f2; }
  bool operator()(const Class* c1, const Class* c2) const { return c1 == c2; }

  bool operator()(const Class* c1, LazyClassData c2) const {
    return c1->name() == c2.name();
  }
  bool operator()(LazyClassData c1, const Class* c2) const {
    return c1.name() == c2->name();
  }

  bool operator()(const ObjectData* od1, const ObjectData* od2) const {
    assertx(od1);
    assertx(od2);
    return od1->equal(*od2);
  }

  bool operator()(const ResourceData* od1, const ResourceData* od2) const {
    return od1 == od2;
  }
  bool operator()(const ResourceHdr* od1, const ResourceHdr* od2) const {
    return od1 == od2;
  }

  bool vec(const ArrayData* ad1, const ArrayData* ad2) const {
    assertx(ad1->isVecType());
    assertx(ad2->isVecType());
    return ArrayData::bothVanilla(ad1, ad2)
      ? PackedArray::VecEqual(ad1, ad2)
      : ArrayData::Equal(ad1, ad2);
  }
  bool dict(const ArrayData* ad1, const ArrayData* ad2) const {
    assertx(ad1->isDictType());
    assertx(ad2->isDictType());
    return ArrayData::bothVanilla(ad1, ad2)
      ? MixedArray::DictEqual(ad1, ad2)
      : ArrayData::Equal(ad1, ad2);
  }
  bool keyset(const ArrayData* ad1, const ArrayData* ad2) const {
    assertx(ad1->isKeysetType());
    assertx(ad2->isKeysetType());
    return ArrayData::bothVanilla(ad1, ad2)
      ? SetArray::Equal(ad1, ad2)
      : ArrayData::Equal(ad1, ad2);
  }

  bool funcVsNonFunc() const { return false; }
  bool vecVsNonVec() const { return false; }
  bool dictVsNonDict() const { return false; }
  bool keysetVsNonKeyset() const { return false; }
  bool collectionVsNonObj() const { return false; }
  bool rfuncVsNonRFunc() const { return false; }
  bool rclsMethVsNonRClsMeth() const { return false; }
  bool recordVsNonRecord() const {
    throw_rec_non_rec_compare_exception();
  }
  bool clsmethVsNonClsMeth() const { return false; }

  bool operator()(ClsMethDataRef c1, ClsMethDataRef c2) const {
    return c1 == c2;
  }

  bool operator()(RClsMethData* c1, RClsMethData* c2) const {
    return RClsMethData::Same(c1, c2);
  }

  bool operator()(RClsMethData*, bool b) const {
    return b;
  }

  bool operator()(const RecordData* r1, const RecordData* r2) const {
    return RecordData::equal(r1, r2);
  }

  bool operator()(const RFuncData*, bool b) const {
    return b;
  }

  bool operator()(const RFuncData* r1, const RFuncData* r2) const {
    return RFuncData::Same(r1, r2);
  }
};

template<class RetType, class PrimitiveCmpOp>
struct CompareBase {
  template<class T, class U>
  typename std::enable_if<
    !std::is_pointer<T>::value &&
    !std::is_pointer<U>::value,
    RetType
  >::type operator()(T t, U u) const {
    auto c = PrimitiveCmpOp();
    return c(t, u);
  }

  RetType operator()(const ArrayData* ad, bool val) const {
    if (ad->isVecType()) throw_vec_compare_exception();
    if (ad->isDictType()) throw_dict_compare_exception();
    assertx(ad->isKeysetType());
    throw_keyset_compare_exception();
  }
  RetType operator()(bool val, const ArrayData* ad) const {
    if (ad->isVecType()) throw_vec_compare_exception();
    if (ad->isDictType()) throw_dict_compare_exception();
    assertx(ad->isKeysetType());
    throw_keyset_compare_exception();
  }

  RetType operator()(const StringData* sd1, const StringData* sd2) const {
    return operator()(sd1->compare(sd2), 0);
  }
  RetType operator()(const ResourceData* rd1, const ResourceData* rd2) const {
    return operator()(rd1->o_toInt64(), rd2->o_toInt64());
  }
  RetType operator()(const ResourceHdr* rd1, const ResourceHdr* rd2) const {
    return operator()(rd1->data()->o_toInt64(), rd2->data()->o_toInt64());
  }

  RetType operator()(const Class* c1, LazyClassData c2) const {
    return operator()(classToStringHelper(c1), lazyClassToStringHelper(c2));
  }
  RetType operator()(LazyClassData c1, const Class* c2) const {
    return operator()(lazyClassToStringHelper(c1), classToStringHelper(c2));
  }

  RetType dict(const ArrayData* ad1, const ArrayData* ad2) const {
    assertx(ad1->isDictType());
    assertx(ad2->isDictType());
    throw_dict_compare_exception();
  }
  RetType keyset(const ArrayData* ad1, const ArrayData* ad2) const {
    assertx(ad1->isKeysetType());
    assertx(ad2->isKeysetType());
    throw_keyset_compare_exception();
  }

  RetType funcVsNonFunc() const {
    throw_func_compare_exception();
  }
  RetType vecVsNonVec() const {
    throw_vec_compare_exception();
  }
  RetType dictVsNonDict() const {
    throw_dict_compare_exception();
  }
  RetType keysetVsNonKeyset() const {
    throw_keyset_compare_exception();
  }
  RetType collectionVsNonObj() const {
    throw_collection_compare_exception();
  }
  RetType rfuncVsNonRFunc() const {
    throw_rfunc_compare_exception();
  }
  RetType rclsMethVsNonRClsMeth() const {
    throw_rclsmeth_compare_exception();
  }
  RetType recordVsNonRecord() const {
    throw_rec_non_rec_compare_exception();
  }
  RetType clsmethVsNonClsMeth() const {
    throw_clsmeth_compare_exception();
  }

  bool operator()(const Func* f1, const Func* f2) const {
    throw_func_compare_exception();
  }

  bool operator()(const Class* c1, const Class* c2) const {
    return operator()(classToStringHelper(c1), classToStringHelper(c2));
  }

  RetType operator()(ClsMethDataRef c1, ClsMethDataRef c2) const {
    if (!RO::EvalIsCompatibleClsMethType) {
      throw_clsmeth_compare_exception();
    }
    if (RO::EvalRaiseClsMethComparisonWarning) {
      raiseClsMethClsMethRelCompareWarning();
    }
    auto const cls1 = c1->getClsStr().get();
    auto const cls2 = c2->getClsStr().get();
    auto const cmp = cls1->compare(cls2);
    if (cmp != 0) {
      return operator()(cmp, 0);
    }
    auto const func1 = c1->getFuncStr().get();
    auto const func2 = c2->getFuncStr().get();
    return operator()(func1, func2);
  }

  RetType operator()(const RecordData*, const RecordData*) const {
    throw_record_compare_exception();
  }

  RetType operator()(const RFuncData*, const RFuncData*) const {
    throw_rfunc_compare_exception();
  }

  RetType operator()(const RFuncData*, bool) const {
    throw_rfunc_compare_exception();
  }

  RetType operator()(const RClsMethData*, const RClsMethData*) const {
    throw_rclsmeth_compare_exception();
  }

  RetType operator()(const RClsMethData*, bool) const {
    throw_rclsmeth_compare_exception();
  }
};

struct Lt : CompareBase<bool, std::less<>> {
  using RetType = bool;

  using CompareBase::operator();

  bool operator()(const ObjectData* od1, const ObjectData* od2) const {
    assertx(od1);
    assertx(od2);
    return od1->less(*od2);
  }

  bool vec(const ArrayData* ad1, const ArrayData* ad2) const {
    assertx(ad1->isVecType());
    assertx(ad2->isVecType());
    return ArrayData::Lt(ad1, ad2);
  }
};

struct Lte : CompareBase<bool, std::less_equal<>> {
  using RetType = bool;

  using CompareBase::operator();

  bool operator()(const ObjectData* od1, const ObjectData* od2) const {
    assertx(od1);
    assertx(od2);
    return od1->lessEqual(*od2);
  }

  bool vec(const ArrayData* ad1, const ArrayData* ad2) const {
    assertx(ad1->isVecType());
    assertx(ad2->isVecType());
    return ArrayData::Lte(ad1, ad2);
  }
};

struct Gt : CompareBase<bool, std::greater<>> {
  using RetType = bool;

  using CompareBase::operator();

  bool operator()(const ObjectData* od1, const ObjectData* od2) const {
    assertx(od1);
    assertx(od2);
    return od1->more(*od2);
  }

  bool vec(const ArrayData* ad1, const ArrayData* ad2) const {
    assertx(ad1->isVecType());
    assertx(ad2->isVecType());
    return ArrayData::Gt(ad1, ad2);
  }
};

struct Gte : CompareBase<bool, std::greater_equal<>> {
  using RetType = bool;

  using CompareBase::operator();

  bool operator()(const ObjectData* od1, const ObjectData* od2) const {
    assertx(od1);
    assertx(od2);
    return od1->moreEqual(*od2);
  }

  bool vec(const ArrayData* ad1, const ArrayData* ad2) const {
    assertx(ad1->isVecType());
    assertx(ad2->isVecType());
    return ArrayData::Gte(ad1, ad2);
  }
};

struct PHPPrimitiveCmp {
  template<class T, class U>
  constexpr int64_t operator()(const T& t, const U& u) const {
    // This ordering is required so that -1 is returned for NaNs (to match PHP7
    // behavior).
    return (t == u) ? 0 : ((t > u) ? 1 : -1);
  }
};

struct Cmp : CompareBase<int64_t, struct PHPPrimitiveCmp> {
  using RetType = int64_t;

  using CompareBase::operator();

  int64_t operator()(const ObjectData* od1, const ObjectData* od2) const {
    return od1->compare(*od2);
  }

  int64_t vec(const ArrayData* ad1, const ArrayData* ad2) const {
    assertx(ad1->isVecType());
    assertx(ad2->isVecType());
    return ArrayData::Compare(ad1, ad2);
  }
};

template<class Op> constexpr bool isEqualityOp() {
  return std::is_same_v<Op, Eq>;
}

//////////////////////////////////////////////////////////////////////

}

bool tvSame(TypedValue c1, TypedValue c2) {
  assertx(tvIsPlausible(c1));
  assertx(tvIsPlausible(c2));

  bool const null1 = isNullType(c1.m_type);
  bool const null2 = isNullType(c2.m_type);
  if (null1 && null2) return true;
  if (null1 || null2) return false;

  switch (c1.m_type) {
    case KindOfBoolean:
    case KindOfInt64:
      if (c2.m_type != c1.m_type) return false;
      return c1.m_data.num == c2.m_data.num;

    case KindOfDouble:
      if (c2.m_type != c1.m_type) return false;
      return c1.m_data.dbl == c2.m_data.dbl;

    case KindOfPersistentString:
    case KindOfString:
      if (isClassType(c2.m_type)) {
        return c1.m_data.pstr->same(classToStringHelper(c2.m_data.pclass));
      }
      if (isLazyClassType(c2.m_type)) {
        return
          c1.m_data.pstr->same(lazyClassToStringHelper(c2.m_data.plazyclass));
      }
      if (!isStringType(c2.m_type)) return false;
      return c1.m_data.pstr->same(c2.m_data.pstr);

    case KindOfRFunc:
      return isRFuncType(c2.m_type) &&
        RFuncData::Same(c1.m_data.prfunc, c2.m_data.prfunc);

    case KindOfFunc:
      if (c2.m_type != KindOfFunc) return false;
      return c1.m_data.pfunc == c2.m_data.pfunc;

    case KindOfClass:
      if (isStringType(c2.m_type)) {
        return classToStringHelper(c1.m_data.pclass)->same(c2.m_data.pstr);
      }
      if (isLazyClassType(c2.m_type)) {
        return c1.m_data.pclass->name()->same(c2.m_data.plazyclass.name());
      }
      if (c2.m_type != KindOfClass) return false;
      return c1.m_data.pclass == c2.m_data.pclass;

    case KindOfLazyClass:
      if (isStringType(c2.m_type)) {
        return
          lazyClassToStringHelper(c1.m_data.plazyclass)->same(c2.m_data.pstr);
      }
      if (isClassType(c2.m_type)) {
        return c1.m_data.plazyclass.name()->same(c2.m_data.pclass->name());
      }
      if (c2.m_type != KindOfLazyClass) return false;
      return c1.m_data.plazyclass.name() == c2.m_data.plazyclass.name();

    case KindOfPersistentVec:
    case KindOfVec:
      if (isClsMethType(c2.m_type) && RO::EvalIsCompatibleClsMethType) {
        raiseClsMethToVecWarningHelper();
        return vecSameHelper(
          c1.m_data.parr, clsMethToVecHelper(c2.m_data.pclsmeth).get());
      }
      if (!isVecType(c2.m_type)) {
        return false;
      }
      return vecSameHelper(c1.m_data.parr, c2.m_data.parr);

    case KindOfPersistentDict:
    case KindOfDict: {
      if (!isDictType(c2.m_type)) {
        return false;
      }
      auto const ad1 = c1.m_data.parr;
      auto const ad2 = c2.m_data.parr;
      return ArrayData::bothVanilla(ad1, ad2)
        ? MixedArray::DictSame(ad1, ad2)
        : ArrayData::Same(ad1, ad2);
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset: {
      if (!isKeysetType(c2.m_type)) {
        return false;
      }
      auto const ad1 = c1.m_data.parr;
      auto const ad2 = c2.m_data.parr;
      return ArrayData::bothVanilla(ad1, ad2)
        ? SetArray::Same(ad1, ad2)
        : ArrayData::Same(ad1, ad2);
    }

    case KindOfObject:
      return c2.m_type == KindOfObject &&
        c1.m_data.pobj == c2.m_data.pobj;

    case KindOfResource:
      return c2.m_type == KindOfResource &&
        c1.m_data.pres == c2.m_data.pres;

    case KindOfClsMeth:
      if (RO::EvalIsCompatibleClsMethType && tvIsVec(c2)) {
        raiseClsMethToVecWarningHelper();
        return vecSameHelper(
          clsMethToVecHelper(c1.m_data.pclsmeth).get(), c2.m_data.parr);
      }
      if (!isClsMethType(c2.m_type)) {
        return false;
      }
      return c1.m_data.pclsmeth == c2.m_data.pclsmeth;

    case KindOfRClsMeth:
      return isRClsMethType(c2.m_type) &&
        RClsMethData::Same(c1.m_data.prclsmeth, c2.m_data.prclsmeth);

    case KindOfRecord:
      return c2.m_type == KindOfRecord &&
        RecordData::same(c1.m_data.prec, c2.m_data.prec);

    case KindOfUninit:
    case KindOfNull:
      break;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

template<class Op, DataType DT, typename T>
typename Op::RetType tvRelOp(TypedValue cell, T val) {
  if (std::is_same_v<Op, Eq> && useStrictEquality() &&
      !equivDataTypesIncludingMigrations(cell.m_type, DT)) {
    return false;
  }

  typename Op::RetType res;
  if constexpr (DT == DataType::Vec) res         = tvRelOpVec(Op(), cell, val);
  else if constexpr (DT == DataType::Dict) res   = tvRelOpDict(Op(), cell, val);
  else if constexpr (DT == DataType::Keyset) res = tvRelOpKeyset(Op(), cell, val);
  else res = tvRelOp(Op(), cell, val);

  // put it after in case the original comparison throws
  if (shouldMaybeTriggerConvNotice<Op>(cell.m_type, DT, res)) {
    const char* rhs = [&]() {
      switch(DT) {
        case DataType::Boolean:  return "bool";
        case DataType::Int64:    return "int";
        case DataType::Double:   return "float";
        case DataType::String:   return "string";
        case DataType::Vec:      return "vec";
        case DataType::Dict:     return "dict";
        case DataType::Keyset:   return "keyset";
        case DataType::Resource: return "resource";
        case DataType::ClsMeth:  return "clsmeth";
        case DataType::Object:
          return cell.val().pobj->getVMClass()->name()->data();
      }
      not_reached();
    }();
    handleConvNotice<Op>(describe_actual_type(&cell), rhs);
  }
  return res;
}

template<class Op>
typename Op::RetType tvRelOpArr(TypedValue cell, const ArrayData* val) {
  if (val->isVecType())    return tvRelOp<Op, DataType::Vec>(cell, val);
  if (val->isDictType())   return tvRelOp<Op, DataType::Dict>(cell, val);
  if (val->isKeysetType()) return tvRelOp<Op, DataType::Keyset>(cell, val);
  not_reached();
}

bool tvEqual(TypedValue cell, bool val) {
  return tvRelOp<Eq, DataType::Boolean>(cell, val);
}

bool tvEqual(TypedValue cell, int64_t val) {
  return tvRelOp<Eq, DataType::Int64>(cell, val);
}

bool tvEqual(TypedValue cell, double val) {
  return tvRelOp<Eq, DataType::Double>(cell, val);
}

bool tvEqual(TypedValue cell, const StringData* val) {
  return tvRelOp<Eq, DataType::String>(cell, val);
}

bool tvEqual(TypedValue cell, const ArrayData* val) {
  return tvRelOpArr<Eq>(cell, val);
}

bool tvEqual(TypedValue cell, const ObjectData* val) {
  return tvRelOp<Eq, DataType::Object>(cell, val);
}

bool tvEqual(TypedValue cell, const ResourceData* val) {
  return tvRelOp<Eq, DataType::Resource>(cell, val);
}
bool tvEqual(TypedValue cell, const ResourceHdr* val) {
  return tvRelOp<Eq, DataType::Resource>(cell, val);
}

bool tvEqual(TypedValue cell, ClsMethDataRef val) {
  return tvRelOp<Eq, DataType::ClsMeth>(cell, val);
}

bool tvEqual(TypedValue c1, TypedValue c2) {
  return tvRelOp(Eq(), c1, c2);
}

bool tvLess(TypedValue cell, bool val) {
  return tvRelOp<Lt, DataType::Boolean>(cell, val);
}

bool tvLess(TypedValue cell, int64_t val) {
  return tvRelOp<Lt, DataType::Int64>(cell, val);
}

bool tvLess(TypedValue cell, double val) {
  return tvRelOp<Lt, DataType::Double>(cell, val);
}

bool tvLess(TypedValue cell, const StringData* val) {
  return tvRelOp<Lt, DataType::String>(cell, val);
}

bool tvLess(TypedValue cell, const ArrayData* val) {
  return tvRelOpArr<Lt>(cell, val);
}

bool tvLess(TypedValue cell, const ObjectData* val) {
  return tvRelOp<Lt, DataType::Object>(cell, val);
}

bool tvLess(TypedValue cell, const ResourceData* val) {
  return tvRelOp<Lt, DataType::Resource>(cell, val);
}
bool tvLess(TypedValue cell, const ResourceHdr* val) {
  return tvRelOp<Lt, DataType::Resource>(cell, val);
}

bool tvLess(TypedValue cell, ClsMethDataRef val) {
  return tvRelOp<Lt, DataType::ClsMeth>(cell, val);
}

bool tvLess(TypedValue tv1, TypedValue tv2) {
  return tvRelOp(Lt(), tv1, tv2);
}

bool tvGreater(TypedValue cell, bool val) {
  return tvRelOp<Gt, DataType::Boolean>(cell, val);
}

bool tvGreater(TypedValue cell, int64_t val) {
  return tvRelOp<Gt, DataType::Int64>(cell, val);
}

bool tvGreater(TypedValue cell, double val) {
  return tvRelOp<Gt, DataType::Double>(cell, val);
}

bool tvGreater(TypedValue cell, const StringData* val) {
  return tvRelOp<Gt, DataType::String>(cell, val);
}

bool tvGreater(TypedValue cell, const ArrayData* val) {
  return tvRelOpArr<Gt>(cell, val);
}

bool tvGreater(TypedValue cell, const ObjectData* val) {
  return tvRelOp<Gt, DataType::Object>(cell, val);
}

bool tvGreater(TypedValue cell, const ResourceData* val) {
  return tvRelOp<Gt, DataType::Resource>(cell, val);
}
bool tvGreater(TypedValue cell, const ResourceHdr* val) {
  return tvRelOp<Gt, DataType::Resource>(cell, val);
}

bool tvGreater(TypedValue cell, ClsMethDataRef val) {
  return tvRelOp<Gt, DataType::ClsMeth>(cell, val);
}

bool tvGreater(TypedValue tv1, TypedValue tv2) {
  return tvRelOp(Gt(), tv1, tv2);
}

//////////////////////////////////////////////////////////////////////

int64_t tvCompare(TypedValue cell, bool val) {
  return tvRelOp<Cmp, DataType::Boolean>(cell, val);
}

int64_t tvCompare(TypedValue cell, int64_t val) {
  return tvRelOp<Cmp, DataType::Int64>(cell, val);
}

int64_t tvCompare(TypedValue cell, double val) {
  return tvRelOp<Cmp, DataType::Double>(cell, val);
}

int64_t tvCompare(TypedValue cell, const StringData* val) {
  return tvRelOp<Cmp, DataType::String>(cell, val);
}

int64_t tvCompare(TypedValue cell, const ArrayData* val) {
  return tvRelOpArr<Cmp>(cell, val);
}

int64_t tvCompare(TypedValue cell, const ObjectData* val) {
  return tvRelOp<Cmp, DataType::Object>(cell, val);
}

int64_t tvCompare(TypedValue cell, const ResourceData* val) {
  return tvRelOp<Cmp, DataType::Resource>(cell, val);
}
int64_t tvCompare(TypedValue cell, const ResourceHdr* val) {
  return tvRelOp<Cmp, DataType::Resource>(cell, val);
}

int64_t tvCompare(TypedValue cell, ClsMethDataRef val) {
  return tvRelOp<Cmp, DataType::ClsMeth>(cell, val);
}

int64_t tvCompare(TypedValue tv1, TypedValue tv2) {
  return tvRelOp(Cmp(), tv1, tv2);
}

//////////////////////////////////////////////////////////////////////

bool tvLessOrEqual(TypedValue c1, TypedValue c2) {
  assertx(tvIsPlausible(c1));
  assertx(tvIsPlausible(c2));
  return tvRelOp(Lte(), c1, c2);
}

bool tvGreaterOrEqual(TypedValue c1, TypedValue c2) {
  assertx(tvIsPlausible(c1));
  assertx(tvIsPlausible(c2));
  return tvRelOp(Gte(), c1, c2);
}

//////////////////////////////////////////////////////////////////////

}
