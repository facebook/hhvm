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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/double-to-int64.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/assertions.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

template<typename T>
enable_if_lval_t<T, void> tvCastToBooleanInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  bool b;

  do {
    switch (type(tv)) {
      case KindOfUninit:
      case KindOfNull:
        b = false;
        continue;

      case KindOfBoolean:
        return;

      case KindOfInt64:
        b = (val(tv).num != 0LL);
        continue;

      case KindOfDouble:
        b = (val(tv).dbl != 0);
        continue;

      case KindOfPersistentString:
        b = val(tv).pstr->toBoolean();
        continue;

      case KindOfString:
        b = val(tv).pstr->toBoolean();
        tvDecRefStr(tv);
        continue;

      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentShape:
      case KindOfPersistentArray:
        b = !val(tv).parr->empty();
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfShape:
      case KindOfArray:
        b = !val(tv).parr->empty();
        tvDecRefArr(tv);
        continue;

      case KindOfObject:
        b = val(tv).pobj->toBoolean();
        tvDecRefObj(tv);
        continue;

      case KindOfResource:
        b = val(tv).pres->data()->o_toBoolean();
        tvDecRefRes(tv);
        continue;

      case KindOfFunc:
        b = funcToStringHelper(val(tv).pfunc)->toBoolean();
        continue;

      case KindOfClass:
        b = classToStringHelper(val(tv).pclass)->toBoolean();
        continue;

      case KindOfClsMeth:
        raiseClsMethConvertWarningHelper("bool");
        b = true;
        tvDecRefClsMeth(tv);
        continue;

      case KindOfRecord:
        raise_convert_record_to_type("bool");

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  val(tv).num = b;
  type(tv) = KindOfBoolean;
}

bool tvCastToBoolean(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->cell();
  }
  return cellToBool(tv);
}

template<typename T>
enable_if_lval_t<T, void> tvCastToDoubleInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  double d;

  do {
    switch (type(tv)) {
      case KindOfUninit:
      case KindOfNull:
        d = 0.0;
        continue;

      case KindOfBoolean:
        assertx(val(tv).num == 0LL || val(tv).num == 1LL);
        // fallthru
      case KindOfInt64:
        d = (double)(val(tv).num);
        continue;

      case KindOfDouble:
        return;

      case KindOfPersistentString:
        d = val(tv).pstr->toDouble();
        continue;

      case KindOfString:
        d = val(tv).pstr->toDouble();
        tvDecRefStr(tv);
        continue;

      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentShape:
      case KindOfPersistentArray:
        d = val(tv).parr->empty() ? 0 : 1;
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfShape:
      case KindOfArray:
        d = val(tv).parr->empty() ? 0 : 1;
        tvDecRefArr(tv);
        continue;

      case KindOfObject:
        d = val(tv).pobj->toDouble();
        tvDecRefObj(tv);
        continue;

      case KindOfResource:
        d = val(tv).pres->data()->o_toDouble();
        tvDecRefRes(tv);
        continue;

      case KindOfFunc:
        d = funcToStringHelper(val(tv).pfunc)->toDouble();
        continue;

      case KindOfClass:
        d = classToStringHelper(val(tv).pclass)->toDouble();
        continue;

      case KindOfClsMeth:
        raiseClsMethConvertWarningHelper("double");
        d = 1.0;
        tvDecRefClsMeth(tv);
        continue;

      case KindOfRecord:
        raise_convert_record_to_type("double");

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  val(tv).dbl = d;
  type(tv) = KindOfDouble;
}

template<typename T>
enable_if_lval_t<T, void> tvCastToInt64InPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  assertx(cellIsPlausible(*tv));
  int64_t i;

  do {
    switch (type(tv)) {
      case KindOfUninit:
      case KindOfNull:
        val(tv).num = 0LL;
        // fallthru
      case KindOfBoolean:
        assertx(val(tv).num == 0LL || val(tv).num == 1LL);
        type(tv) = KindOfInt64;
        // fallthru
      case KindOfInt64:
        return;

      case KindOfDouble:
        i = double_to_int64(val(tv).dbl);
        continue;

      case KindOfPersistentString:
        i = val(tv).pstr->toInt64();
        continue;

      case KindOfString:
        i = val(tv).pstr->toInt64();
        tvDecRefStr(tv);
        continue;

      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentShape:
      case KindOfPersistentArray:
        i = val(tv).parr->empty() ? 0 : 1;
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfShape:
      case KindOfArray:
        i = val(tv).parr->empty() ? 0 : 1;
        tvDecRefArr(tv);
        continue;

      case KindOfObject:
        i = val(tv).pobj->toInt64();
        tvDecRefObj(tv);
        continue;

      case KindOfResource:
        i = val(tv).pres->data()->o_toInt64();
        tvDecRefRes(tv);
        continue;

      case KindOfFunc:
        i = funcToStringHelper(val(tv).pfunc)->toInt64();
        continue;

      case KindOfClass:
        i = classToStringHelper(val(tv).pclass)->toInt64();
        continue;

      case KindOfClsMeth:
        raiseClsMethConvertWarningHelper("int");
        i = 1;
        tvDecRefClsMeth(tv);
        continue;

      case KindOfRecord:
        raise_convert_record_to_type("int");

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  val(tv).num = i;
  type(tv) = KindOfInt64;
}

int64_t tvCastToInt64(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->cell();
  }
  return cellToInt(tv);
}

double tvCastToDouble(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->cell();
  }

  switch (tv.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return 0;

    case KindOfBoolean:
      assertx(tv.m_data.num == 0LL || tv.m_data.num == 1LL);
      // fallthru
    case KindOfInt64:
      return (double)(tv.m_data.num);

    case KindOfDouble:
      return tv.m_data.dbl;

    case KindOfPersistentString:
    case KindOfString:
      return tv.m_data.pstr->toDouble();

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray:
      return tv.m_data.parr->empty() ? 0.0 : 1.0;

    case KindOfObject:
      return tv.m_data.pobj->toDouble();

    case KindOfResource:
      return tv.m_data.pres->data()->o_toDouble();

    case KindOfFunc:
      return funcToStringHelper(tv.m_data.pfunc)->toDouble();

    case KindOfClass:
      return classToStringHelper(tv.m_data.pclass)->toDouble();

    case KindOfClsMeth:
      raiseClsMethConvertWarningHelper("double");
      return 1.0;

    case KindOfRecord:
      raise_convert_record_to_type("double");

    case KindOfRef:
      break;
  }
  not_reached();
}

const StaticString
  s_1("1"),
  s_scalar("scalar");

template<typename T>
enable_if_lval_t<T, void> tvCastToStringInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  cellCastToStringInPlace(tv);
}

void cellCastToStringInPlace(tv_lval tv) {
  auto string = [&](StringData* s) {
    type(tv) = KindOfString;
    val(tv).pstr = s;
  };
  auto persistentString = [&](StringData* s) {
    assertx(!s->isRefCounted());
    type(tv) = KindOfPersistentString;
    val(tv).pstr = s;
  };

  switch (type(tv)) {
    case KindOfUninit:
    case KindOfNull:
      return persistentString(staticEmptyString());

    case KindOfBoolean:
      return persistentString(val(tv).num ? s_1.get() : staticEmptyString());

    case KindOfInt64:
      return string(buildStringData(val(tv).num));

    case KindOfDouble:
      return string(buildStringData(val(tv).dbl));

    case KindOfPersistentString:
    case KindOfString:
      return;

    case KindOfVec:
    case KindOfPersistentVec:
      raise_notice("Vec to string conversion");
      if (type(tv) == KindOfVec) tvDecRefArr(*tv);
      return persistentString(vec_string.get());

    case KindOfDict:
    case KindOfPersistentDict:
      raise_notice("Dict to string conversion");
      if (type(tv) == KindOfDict) tvDecRefArr(*tv);
      return persistentString(dict_string.get());

    case KindOfKeyset:
    case KindOfPersistentKeyset:
      raise_notice("Keyset to string conversion");
      if (type(tv) == KindOfKeyset) tvDecRefArr(*tv);
      return persistentString(keyset_string.get());

    case KindOfPersistentShape:
    case KindOfShape:
      if (RuntimeOption::EvalHackArrDVArrs) {
        raise_notice("Dict to string conversion");
        if (type(tv) == KindOfShape) tvDecRefArr(*tv);
        return persistentString(dict_string.get());
      }
      // Fallthrough

    case KindOfArray:
    case KindOfPersistentArray:
      raise_notice("Array to string conversion");
      if (type(tv) == KindOfArray) tvDecRefArr(*tv);
      return persistentString(array_string.get());

    case KindOfObject:
      cellMove(
        make_tv<KindOfString>(val(tv).pobj->invokeToString().detach()),
        tv
      );
      return;

    case KindOfResource:
      cellMove(
        make_tv<KindOfString>(val(tv).pres->data()->o_toString().detach()),
        tv
      );
      return;

    case KindOfFunc: {
      auto const s = funcToStringHelper(val(tv).pfunc);
      return persistentString(const_cast<StringData*>(s));
    }

    case KindOfClass: {
      auto const s = classToStringHelper(val(tv).pclass);
      return persistentString(const_cast<StringData*>(s));
    }

    case KindOfClsMeth:
      raiseClsMethConvertWarningHelper("string");
      tvDecRefClsMeth(tv);
      if (RuntimeOption::EvalHackArrDVArrs) {
        return persistentString(vec_string.get());
      } else {
        return persistentString(array_string.get());
      }

    case KindOfRecord:
      raise_convert_record_to_type("string");

    case KindOfRef:
      break;
  }
  not_reached();
}

StringData* tvCastToStringData(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->cell();
  }
  return cellCastToStringData(tv);
}

StringData* cellCastToStringData(Cell tv) {
  assert(tv.m_type != KindOfRef);

  switch (tv.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return staticEmptyString();

    case KindOfBoolean:
      return tv.m_data.num ? s_1.get() : staticEmptyString();

    case KindOfInt64:
      return buildStringData(tv.m_data.num);

    case KindOfDouble:
      return buildStringData(tv.m_data.dbl);

    case KindOfPersistentString:
      return tv.m_data.pstr;

    case KindOfString: {
      auto s = tv.m_data.pstr;
      s->incRefCount();
      return s;
    }

    case KindOfPersistentVec:
    case KindOfVec:
      raise_notice("Vec to string conversion");
      return vec_string.get();

    case KindOfPersistentDict:
    case KindOfDict:
      raise_notice("Dict to string conversion");
      return dict_string.get();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      raise_notice("Keyset to string conversion");
      return keyset_string.get();

    case KindOfPersistentShape:
    case KindOfShape:
      if (RuntimeOption::EvalHackArrDVArrs) {
        raise_notice("Dict to string conversion");
        return dict_string.get();
      }
      // Fallthrough

    case KindOfPersistentArray:
    case KindOfArray:
      raise_notice("Array to string conversion");
      return array_string.get();

    case KindOfObject:
      return tv.m_data.pobj->invokeToString().detach();

    case KindOfResource:
      return tv.m_data.pres->data()->o_toString().detach();

    case KindOfFunc: {
      auto const s = funcToStringHelper(tv.m_data.pfunc);
      return const_cast<StringData*>(s);
    }

    case KindOfClass: {
      auto const s = classToStringHelper(tv.m_data.pclass);
      return const_cast<StringData*>(s);
    }

    case KindOfClsMeth:
      raiseClsMethConvertWarningHelper("string");
      if (RuntimeOption::EvalHackArrDVArrs) {
        return vec_string.get();
      } else {
        return array_string.get();
      }

    case KindOfRecord:
      raise_convert_record_to_type("string");

    case KindOfRef:
      not_reached();
  }
  not_reached();
}

String tvCastToString(TypedValue tv) {
  return String::attach(tvCastToStringData(tv));
}

template <IntishCast IC>
ArrayData* tvCastToArrayLikeData(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->cell();
  }

  switch (tv.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return ArrayData::Create();

    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return ArrayData::Create(tv);

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray: {
      auto const ad = tv.m_data.parr;
      ad->incRefCount();
      return ad;
    }

    case KindOfClsMeth:
      raiseClsMethToVecWarningHelper();
      return clsMethToVecHelper(tv.m_data.pclsmeth).detach();

    case KindOfObject: {
      auto ad = tv.m_data.pobj->toArray<IC>();
      assertx(ad->isPHPArray());
      return ad.detach();
    }

    case KindOfRecord:
      raise_convert_record_to_type("array");

    case KindOfRef:
      break;
  }
  not_reached();
}

template
ArrayData* tvCastToArrayLikeData<IntishCast::None>(TypedValue);
template
ArrayData* tvCastToArrayLikeData<IntishCast::Cast>(TypedValue);

template <IntishCast IC /* = IntishCast::None */>
Array tvCastToArrayLike(TypedValue tv) {
  return Array::attach(tvCastToArrayLikeData<IC>(tv));
}

template Array tvCastToArrayLike<IntishCast::Cast>(TypedValue);
template Array tvCastToArrayLike<IntishCast::None>(TypedValue);

template <typename LHS, typename T>
static enable_if_lval_t<
  LHS,
  typename std::enable_if<std::is_rvalue_reference<T&&>::value, void>::type>
assign(LHS lhs, T&& rhs) {
  variant_ref{lhs} = std::forward<T>(rhs);
}

template<typename T>
enable_if_lval_t<T, void> tvCastToShapeInPlace(T tv) {
  if (isShapeType(type(tv))) {
    return;
}
  if (RuntimeOption::EvalHackArrDVArrs) {
    tvCastToDictInPlace(tv);
  } else {
    tvCastToDArrayInPlace(tv);
  }
  auto const ad = val(tv).parr;
  assign(tv, ad->toShape(ad->cowCheck()));
}

template<typename T, IntishCast IC /* = IntishCast::None */>
enable_if_lval_t<T, void> tvCastToArrayInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  ArrayData* a;

  do {
    switch (type(tv)) {
      case KindOfUninit:
      case KindOfNull:
        a = ArrayData::Create();
        continue;

      case KindOfBoolean:
      case KindOfInt64:
      case KindOfDouble:
      case KindOfPersistentString:
        a = ArrayData::Create(*tv);
        continue;

      case KindOfString:
        a = ArrayData::Create(*tv);
        tvDecRefStr(tv);
        continue;

      case KindOfPersistentVec: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToPHPArrayVec(adIn, true);
        assertx(a != adIn);
        continue;
      }

      case KindOfVec: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToPHPArrayVec(adIn, adIn->cowCheck());
        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentDict: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isDict());

        if (IC == IntishCast::Cast) {
          a = MixedArray::ToPHPArrayIntishCastDict(adIn, true);
        } else {
          assertx(IC == IntishCast::None);
          a = MixedArray::ToPHPArrayDict(adIn, true);
        }

        assertx(a != adIn);
        continue;
      }

      case KindOfDict: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isDict());

        if (IC == IntishCast::Cast) {
          a = MixedArray::ToPHPArrayIntishCastDict(adIn, adIn->cowCheck());
        } else {
          assertx(IC == IntishCast::None);
          a = MixedArray::ToPHPArrayDict(adIn, adIn->cowCheck());
        }

        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentKeyset: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isKeyset());

        if (IC == IntishCast::Cast) {
          a = SetArray::ToPHPArrayIntishCast(adIn, true);
        } else {
          assertx(IC == IntishCast::None);
          a = SetArray::ToPHPArray(adIn, true);
        }

        assertx(a != adIn);
        continue;
      }

      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isKeyset());

        if (IC == IntishCast::Cast) {
          a = SetArray::ToPHPArrayIntishCast(adIn, adIn->cowCheck());
        } else {
          assertx(IC == IntishCast::None);
          a = SetArray::ToPHPArray(adIn, adIn->cowCheck());
        }

        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentShape: {
        auto const adIn = val(tv).parr;
        assertx(adIn->isShape());
        a = MixedArray::ToPHPArrayShape(adIn, true);
        assertx(a != adIn);
        continue;
      }

      case KindOfShape: {
        auto const adIn = val(tv).parr;
        assertx(adIn->isShape());
        a = MixedArray::ToPHPArrayShape(adIn, adIn->cowCheck());
        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArray());
        if (IC == IntishCast::Cast) {
          a = adIn->toPHPArrayIntishCast(true);
        } else {
          assertx(IC == IntishCast::None);
          a = adIn->toPHPArray(true);
        }
        continue;
      }

      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArray());
        if (IC == IntishCast::Cast) {
          a = adIn->toPHPArrayIntishCast(adIn->cowCheck());
        } else {
          assertx(IC == IntishCast::None);
          a = adIn->toPHPArray(adIn->cowCheck());
        }
        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfObject: {
        assign(tv, ((ObjectData*)val(tv).pobj)->toArray<IC>());
        return;
      }

      case KindOfResource:
        a = ArrayData::Create(*tv);
        tvDecRefRes(tv);
        continue;

      case KindOfFunc:
      case KindOfClass:
        a = ArrayData::Create(*tv);
        continue;

      case KindOfClsMeth: {
        raiseClsMethToVecWarningHelper();
        a = make_packed_array(
          val(tv).pclsmeth->getCls(), val(tv).pclsmeth->getFunc()).detach();
        tvDecRefClsMeth(tv);
        continue;
      }

      case KindOfRecord:
        raise_convert_record_to_type("array");

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  assertx(a->isPHPArray());
  assertx(a->isNotDVArray());

  val(tv).parr = a;
  type(tv) = KindOfArray;
  assertx(cellIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToVecInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  ArrayData* a;

  do {
    switch (type(tv)) {
      case KindOfUninit:
      case KindOfNull:
        SystemLib::throwInvalidOperationExceptionObject(
          "Null to vec conversion"
        );

      case KindOfBoolean:
        SystemLib::throwInvalidOperationExceptionObject(
          "Bool to vec conversion"
        );

      case KindOfInt64:
        SystemLib::throwInvalidOperationExceptionObject(
          "Int to vec conversion"
        );

      case KindOfDouble:
        SystemLib::throwInvalidOperationExceptionObject(
          "Double to vec conversion"
        );

      case KindOfPersistentString:
      case KindOfString:
        SystemLib::throwInvalidOperationExceptionObject(
          "String to vec conversion"
        );

      case KindOfResource:
        SystemLib::throwInvalidOperationExceptionObject(
          "Resource to vec conversion"
        );

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isDict());
        a = MixedArray::ToVecDict(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isKeyset());
        a = SetArray::ToVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentShape:
      case KindOfShape: {
        auto const adIn = val(tv).parr;
        assertx(adIn->isShape());
        a = MixedArray::ToVecShape(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArray());
        a = adIn->toVec(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentVec:
      case KindOfVec:
        assertx(val(tv).parr->isVecArray());
        return;

      case KindOfObject:
        a = castObjToVec(val(tv).pobj);
        // We might have re-entered, so tv may not contain the object anymore.
        tvDecRefGen(tv);
        continue;

      case KindOfFunc:
        SystemLib::throwInvalidOperationExceptionObject(
          "Func to vec conversion"
        );

      case KindOfClass:
        SystemLib::throwInvalidOperationExceptionObject(
          "Class to vec conversion"
        );

      case KindOfClsMeth: {
        raiseClsMethToVecWarningHelper();
        a = make_vec_array(
          val(tv).pclsmeth->getCls(), val(tv).pclsmeth->getFunc()).detach();
        tvDecRefClsMeth(tv);
        continue;
      }

      case KindOfRecord:
        raise_convert_record_to_type("vec");

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  val(tv).parr = a;
  type(tv) = KindOfVec;
  assertx(cellIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToDictInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  ArrayData* a;

  do {
    switch (type(tv)) {
      case KindOfUninit:
      case KindOfNull:
        SystemLib::throwInvalidOperationExceptionObject(
          "Null to dict conversion"
        );

      case KindOfBoolean:
        SystemLib::throwInvalidOperationExceptionObject(
          "Bool to dict conversion"
        );

      case KindOfInt64:
        SystemLib::throwInvalidOperationExceptionObject(
          "Int to dict conversion"
        );

      case KindOfDouble:
        SystemLib::throwInvalidOperationExceptionObject(
          "Double to dict conversion"
        );

      case KindOfPersistentString:
      case KindOfString:
        SystemLib::throwInvalidOperationExceptionObject(
          "String to dict conversion"
        );

      case KindOfResource:
        SystemLib::throwInvalidOperationExceptionObject(
          "Resource to dict conversion"
        );

      case KindOfPersistentVec:
      case KindOfVec: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToDictVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isKeyset());
        a = SetArray::ToDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentShape:
      case KindOfShape: {
        auto const adIn = val(tv).parr;
        assertx(adIn->isShape());
        a = MixedArray::ToDictShape(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArray());
        a = adIn->toDict(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict:
        assertx(val(tv).parr->isDict());
        return;

      case KindOfObject:
        a = castObjToDict(val(tv).pobj);
        // We might have re-entered, so tv may not contain the object anymore.
        tvDecRefGen(tv);
        continue;

      case KindOfFunc:
        SystemLib::throwInvalidOperationExceptionObject(
          "Func to dict conversion"
        );

      case KindOfClass:
        SystemLib::throwInvalidArgumentExceptionObject(
          "Class to dict conversion"
        );

      case KindOfClsMeth: {
        raiseClsMethToVecWarningHelper();
        a = make_dict_array(
          0, val(tv).pclsmeth->getCls(),
          1, val(tv).pclsmeth->getFunc()).detach();
        tvDecRefClsMeth(tv);
        continue;
      }

      case KindOfRecord:
        raise_convert_record_to_type("dict");

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  val(tv).parr = a;
  type(tv) = KindOfDict;
  assertx(cellIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToKeysetInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  ArrayData* a;

  do {
    switch (type(tv)) {
      case KindOfUninit:
      case KindOfNull:
        SystemLib::throwInvalidOperationExceptionObject(
          "Null to keyset conversion"
        );

      case KindOfBoolean:
        SystemLib::throwInvalidOperationExceptionObject(
          "Bool to keyset conversion"
        );

      case KindOfInt64:
        SystemLib::throwInvalidOperationExceptionObject(
          "Int to keyset conversion"
        );

      case KindOfDouble:
        SystemLib::throwInvalidOperationExceptionObject(
          "Double to keyset conversion"
        );

      case KindOfPersistentString:
      case KindOfString:
        SystemLib::throwInvalidOperationExceptionObject(
          "String to keyset conversion"
        );

      case KindOfResource:
        SystemLib::throwInvalidOperationExceptionObject(
          "Resource to keyset conversion"
        );

      case KindOfPersistentVec:
      case KindOfVec: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToKeysetVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isDict());
        a = MixedArray::ToKeysetDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentShape:
      case KindOfShape: {
        auto const adIn = val(tv).parr;
        assertx(adIn->isShape());
        a = MixedArray::ToKeysetShape(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArray());
        a = adIn->toKeyset(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset:
        assertx(val(tv).parr->isKeyset());
        return;

      case KindOfObject:
        a = castObjToKeyset(val(tv).pobj);
        // We might have re-entered, so tv may not contain the object anymore.
        tvDecRefGen(tv);
        continue;

      case KindOfFunc:
        SystemLib::throwInvalidOperationExceptionObject(
          "Func to keyset conversion"
        );

      case KindOfClass:
        SystemLib::throwInvalidOperationExceptionObject(
          "Class to keyset conversion"
        );

      case KindOfClsMeth:
        SystemLib::throwInvalidOperationExceptionObject(
          "clsmeth to keyset conversion"
        );

      case KindOfRecord:
        raise_convert_record_to_type("keyset");

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  val(tv).parr = a;
  type(tv) = KindOfKeyset;
  assertx(cellIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToVArrayInPlace(T tv) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  ArrayData* a;

  do {
    switch (type(tv)) {
      case KindOfUninit:
      case KindOfNull:
        SystemLib::throwInvalidOperationExceptionObject(
          "Null to varray conversion"
        );

      case KindOfBoolean:
        SystemLib::throwInvalidOperationExceptionObject(
          "Bool to varray conversion"
        );

      case KindOfInt64:
        SystemLib::throwInvalidOperationExceptionObject(
          "Int to varray conversion"
        );

      case KindOfDouble:
        SystemLib::throwInvalidOperationExceptionObject(
          "Double to varray conversion"
        );

      case KindOfPersistentString:
      case KindOfString:
        SystemLib::throwInvalidOperationExceptionObject(
          "String to varray conversion"
        );

      case KindOfResource:
        SystemLib::throwInvalidOperationExceptionObject(
          "Resource to varray conversion"
        );

      case KindOfPersistentVec:
      case KindOfVec: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToVArrayVec(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isDict());
        a = MixedArray::ToVArrayDict(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isKeyset());
        a = SetArray::ToVArray(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentShape:
      case KindOfShape: {
        auto const adIn = val(tv).parr;
        assertx(adIn->isShape());
        a = MixedArray::ToVArrayShape(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArray());
        if (adIn->isVArray()) return;
        a = adIn->toVArray(adIn->cowCheck());
        assertx(a->isPacked());
        assertx(a->isVArray());
        if (a == adIn) return;
        decRefArr(adIn);
        continue;
      }

      case KindOfObject:
        a = castObjToVArray(val(tv).pobj);
        // We might have re-entered, so tv may not contain the object anymore.
        tvDecRefGen(tv);
        continue;

      case KindOfFunc:
        SystemLib::throwInvalidOperationExceptionObject(
          "Func to varray conversion"
        );

      case KindOfClass:
        SystemLib::throwInvalidOperationExceptionObject(
          "Class to varray conversion"
        );

      case KindOfClsMeth: {
        raiseClsMethToVecWarningHelper();
        a = make_varray(
          val(tv).pclsmeth->getCls(), val(tv).pclsmeth->getFunc()).detach();
        tvDecRefClsMeth(tv);
        continue;
      }

      case KindOfRecord:
        raise_convert_record_to_type("varray");

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  assertx(a->isPacked());
  assertx(a->isVArray());

  val(tv).parr = a;
  type(tv) = KindOfArray;
  assertx(cellIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToDArrayInPlace(T tv) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  ArrayData* a;

  do {
    switch (type(tv)) {
      case KindOfUninit:
      case KindOfNull:
        SystemLib::throwInvalidOperationExceptionObject(
          "Null to darray conversion"
        );

      case KindOfBoolean:
        SystemLib::throwInvalidOperationExceptionObject(
          "Bool to darray conversion"
        );

      case KindOfInt64:
        SystemLib::throwInvalidOperationExceptionObject(
          "Int to darray conversion"
        );

      case KindOfDouble:
        SystemLib::throwInvalidOperationExceptionObject(
          "Double to darray conversion"
        );

      case KindOfPersistentString:
      case KindOfString:
        SystemLib::throwInvalidOperationExceptionObject(
          "String to darray conversion"
        );

      case KindOfResource:
        SystemLib::throwInvalidOperationExceptionObject(
          "Resource to darray conversion"
        );

      case KindOfPersistentVec:
      case KindOfVec: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToDArrayVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isDict());
        a = MixedArray::ToDArrayDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isKeyset());
        a = SetArray::ToDArray(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentShape:
      case KindOfShape: {
        auto const adIn = val(tv).parr;
        assertx(adIn->isShape());
        a = MixedArray::ToDArrayShape(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArray());
        if (adIn->isDArray()) return;
        a = adIn->toDArray(adIn->cowCheck());
        assertx(a->isMixed());
        assertx(a->isDArray());
        if (a == adIn) return;
        decRefArr(adIn);
        continue;
      }

      case KindOfObject:
        a = castObjToDArray(val(tv).pobj);
        // We might have re-entered, so tv may not contain the object anymore.
        tvDecRefGen(tv);
        continue;

      case KindOfFunc:
        SystemLib::throwInvalidOperationExceptionObject(
          "Func to darray conversion"
        );

      case KindOfClass:
        SystemLib::throwInvalidOperationExceptionObject(
          "Class to darray conversion"
        );

      case KindOfClsMeth: {
        raiseClsMethToVecWarningHelper();
        a = make_darray(
          0, val(tv).pclsmeth->getCls(),
          1, val(tv).pclsmeth->getFunc()).detach();
        tvDecRefClsMeth(tv);
        continue;
      }

      case KindOfRecord:
        raise_convert_record_to_type("darray");

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  assertx(a->isMixed());
  assertx(a->isDArray());

  val(tv).parr = a;
  type(tv) = KindOfArray;
  assertx(cellIsPlausible(*tv));
}

ObjectData* tvCastToObjectData(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->cell();
  }

  switch (tv.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return SystemLib::AllocStdClassObject().detach();

    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfFunc:
    case KindOfClass:
    case KindOfResource: {
      ArrayInit props(1, ArrayInit::Map{});
      props.set(s_scalar, tv);
      return ObjectData::FromArray(props.create()).detach();
    }

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentKeyset:
    case KindOfKeyset: {
      auto const arr = Array::attach(tv.m_data.parr->toPHPArray(true));
      return ObjectData::FromArray(arr.get()).detach();
    }

    case KindOfPersistentArray:
    case KindOfArray:
      return ObjectData::FromArray(tv.m_data.parr).detach();

    case KindOfObject:
      tv.m_data.pobj->incRefCount();
      return tv.m_data.pobj;

    case KindOfClsMeth: {
      raiseClsMethToVecWarningHelper();
      auto arr = make_packed_array(
        val(tv).pclsmeth->getCls(), val(tv).pclsmeth->getFunc());
      return ObjectData::FromArray(arr.get()).detach();
    }

    case KindOfRecord:
      raise_convert_record_to_type("object");

    case KindOfRef:
      break;
  }
  not_reached();
}

Object tvCastToObject(TypedValue tv) {
  return Object::attach(tvCastToObjectData(tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToObjectInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  ObjectData* o;

  do {
    switch (type(tv)) {
      case KindOfUninit:
      case KindOfNull:
        o = SystemLib::AllocStdClassObject().detach();
        continue;

      case KindOfBoolean:
      case KindOfInt64:
      case KindOfDouble:
      case KindOfPersistentString:
      case KindOfFunc:
      case KindOfClass:
      case KindOfResource: {
        ArrayInit props(1, ArrayInit::Map{});
        props.set(s_scalar, *tv);
        o = ObjectData::FromArray(props.create()).detach();
        continue;
      }

      case KindOfString: {
        ArrayInit props(1, ArrayInit::Map{});
        props.set(s_scalar, *tv);
        o = ObjectData::FromArray(props.create()).detach();
        tvDecRefStr(tv);
        continue;
      }

      case KindOfPersistentShape:
      case KindOfShape:
        if (RuntimeOption::EvalHackArrDVArrs) {
          tvCastToArrayInPlace(tv);
        }
        assign(tv, ObjectData::FromArray(val(tv).parr));
        return;

      case KindOfPersistentVec:
      case KindOfVec:
      case KindOfPersistentDict:
      case KindOfDict:
      case KindOfPersistentKeyset:
      case KindOfKeyset:
        tvCastToArrayInPlace(tv);
        // Fall-through to array case
      case KindOfPersistentArray:
      case KindOfArray:
        // For arrays, we fall back on the Variant machinery
        assign(tv, ObjectData::FromArray(val(tv).parr));
        return;
      case KindOfClsMeth:
        raiseClsMethToVecWarningHelper();
        tvCastToArrayInPlace(tv);
        assign(tv, ObjectData::FromArray(val(tv).parr));
        return;
      case KindOfObject:
        return;

      case KindOfRecord:
        raise_convert_record_to_type("object");

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  val(tv).pobj = o;
  type(tv) = KindOfObject;
  assertx(cellIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToNullableObjectInPlace(T tv) {
  if (isNullType(type(tv))) {
    // XXX(t3879280) This happens immediately before calling an extension
    // function that takes an optional Object argument. We want to end up
    // passing const Object& holding nullptr, so by clearing out m_data.pobj we
    // can unconditionally treat &val(tv).pobj as a const Object& in the
    // function being called. This violates the invariant that the value of
    // m_data doesn't matter in a KindOfNull TypedValue.
    val(tv).pobj = nullptr;
  } else {
    tvCastToObjectInPlace(tv);
  }
}

template<typename T>
enable_if_lval_t<T, void> tvCastToResourceInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  do {
    switch (type(tv)) {
      DT_UNCOUNTED_CASE:
        continue;
      case KindOfString:
      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfShape:
      case KindOfArray:
      case KindOfObject:
      case KindOfClsMeth:
      case KindOfRecord:
        tvDecRefCountable(tv);
        continue;
      case KindOfResource:
        // no op, return
        return;
      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  type(tv) = KindOfResource;
  val(tv).pres = req::make<DummyResource>().detach()->hdr();
  assertx(cellIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, bool> tvCoerceParamToBooleanInPlace(T tv, bool builtin) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  switch (type(tv)) {
    case KindOfNull:
      // In PHP 7 mode handling of null types is stricter
      if (RuntimeOption::PHP7_ScalarTypes && !builtin) return false;
      // fall-through

    case KindOfUninit:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfFunc:
    case KindOfClass:
      tvCastToBooleanInPlace<T>(tv);
      return true;

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfClsMeth:
    case KindOfObject:
    case KindOfResource:
    case KindOfRecord:
      return false;

    case KindOfRef:
      break;
  }
  not_reached();
}

template<typename T>
static enable_if_lval_t<T, bool> tvCanBeCoercedToNumber(T tv, bool builtin) {
  switch (type(tv)) {
    case KindOfUninit:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
      return true;

    case KindOfNull:
      // In PHP 7 mode handling of null types is stricter
      return !RuntimeOption::PHP7_ScalarTypes || builtin;

    case KindOfPersistentString:
    case KindOfString:
      return val(tv).pstr->isNumeric();

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:
    case KindOfRecord:
      return false;

    case KindOfRef:
      break;
  }
  not_reached();
}

template<typename T>
enable_if_lval_t<T, bool> tvCoerceParamToInt64InPlace(T tv, bool builtin) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  if (!tvCanBeCoercedToNumber(tv, builtin)) {
    return false;
  }
  // In PHP 7 mode doubles only convert to integers when the conversion is non-
  // narrowing
  if (RuntimeOption::PHP7_ScalarTypes && type(tv) == KindOfDouble) {
    if (val(tv).dbl < std::numeric_limits<int64_t>::min()) return false;
    if (val(tv).dbl > std::numeric_limits<int64_t>::max()) return false;
    if (std::isnan(val(tv).dbl)) return false;
  }
  tvCastToInt64InPlace(tv);
  return true;
}

template<typename T>
enable_if_lval_t<T, bool> tvCoerceParamToDoubleInPlace(T tv, bool builtin) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  if (!tvCanBeCoercedToNumber(tv, builtin)) {
    return false;
  }
  tvCastToDoubleInPlace(tv);
  return true;
}

template<typename T>
enable_if_lval_t<T, bool> tvCoerceParamToStringInPlace(T tv, bool builtin) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  switch (type(tv)) {
    case KindOfNull:
      // In PHP 7 mode handling of null types is stricter
      if (RuntimeOption::PHP7_ScalarTypes && !builtin) {
        return false;
      }
      // fall-through

    case KindOfUninit:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfFunc:
    case KindOfClass:
      tvCastToStringInPlace(tv);
      return true;

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfClsMeth:
    case KindOfRecord:
      return false;

    case KindOfObject:
      if (val(tv).pobj->hasToString()) {
        assign(tv, val(tv).pobj->invokeToString());
        return true;
      }
      return false;

    case KindOfResource:
      return false;

    case KindOfRef:
      break;
  }
  not_reached();
}

template<typename T>
enable_if_lval_t<T, bool> tvCoerceParamToShapeInPlace(T tv, bool builtin) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  switch (type(tv)) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfFunc:
    case KindOfClass:
    case KindOfRecord:
      return false;

    case KindOfPersistentShape:
    case KindOfShape:
      return true;

    case KindOfPersistentArray:
    case KindOfArray:
      if (!RuntimeOption::EvalHackArrDVArrs) {
        assign(tv, val(tv).parr->toShape(true));
        return true;
      }
      return false;

    case KindOfPersistentDict:
    case KindOfDict:
      if (RuntimeOption::EvalHackArrDVArrs) {
        assign(tv, val(tv).parr->toShape(true));
        return true;
      }
      return false;

    case KindOfObject:
      if (RuntimeOption::EvalHackArrDVArrs) return false;
      if (LIKELY(val(tv).pobj->isCollection())) {
        assign(tv, val(tv).pobj->toArray());
        assign(tv, val(tv).parr->toShape(true));
        return true;
      }
      return false;
    case KindOfResource:
      return false;

    case KindOfClsMeth:
      if (!RuntimeOption::EvalHackArrDVArrs) {
        tvCastToArrayInPlace(tv);
        assign(tv, val(tv).parr->toShape(true));
        return true;
      }
      return false;

    case KindOfRef:
      break;
  }
  not_reached();
}

template<typename T, IntishCast IC /* = IntishCast::None */>
enable_if_lval_t<T, bool> tvCoerceParamToArrayInPlace(T tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  switch (type(tv)) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfFunc:
    case KindOfClass:
    case KindOfRecord:
      return false;

    case KindOfPersistentShape:
      if (!RuntimeOption::EvalHackArrDVArrs) {
        assign(tv, val(tv).parr->toDArray(true));
        return true;
      }
      return false;
    case KindOfShape:
      if (!RuntimeOption::EvalHackArrDVArrs) {
        assign(tv, val(tv).parr->toDArray(true));
        return true;
      }
      return false;

    case KindOfPersistentArray:
    case KindOfArray:
      return true;

    case KindOfObject:
      if (LIKELY(val(tv).pobj->isCollection())) {
        ObjectData* obj = val(tv).pobj;
        assign(tv, obj->toArray<IC>());
        return true;
      }
      return false;
    case KindOfResource:
      return false;

    case KindOfClsMeth:
      if (!RuntimeOption::EvalHackArrDVArrs) {
        tvCastToArrayInPlace(tv);
        return true;
      }
      return false;

    case KindOfRef:
      break;
  }
  not_reached();
}

template<typename T>
enable_if_lval_t<T, bool> tvCoerceParamToVecInPlace(T tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  switch (type(tv)) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfObject:
    case KindOfResource:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfFunc:
    case KindOfClass:
    case KindOfRecord:
      return false;

    case KindOfPersistentVec:
    case KindOfVec:
      return true;

    case KindOfClsMeth:
      if (RuntimeOption::EvalHackArrDVArrs) {
        tvCastToVecInPlace(tv);
        return true;
      }
      return false;

    case KindOfRef:
      break;
  }
  not_reached();
}

template<typename T>
enable_if_lval_t<T, bool> tvCoerceParamToDictInPlace(T tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  switch (type(tv)) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfObject:
    case KindOfResource:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:
    case KindOfRecord:
      return false;

    case KindOfPersistentShape:
      if (RuntimeOption::EvalHackArrDVArrs) {
        assign(tv, val(tv).parr->toDict(true));
        return true;
      }
      return false;
    case KindOfShape:
      if (RuntimeOption::EvalHackArrDVArrs) {
        assign(tv, val(tv).parr->toDict(true));
        return true;
      }
      return false;

    case KindOfPersistentDict:
    case KindOfDict:
      return true;

    case KindOfRef:
      break;
  }
  not_reached();
}

template<typename T>
enable_if_lval_t<T, bool> tvCoerceParamToKeysetInPlace(T tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  switch (type(tv)) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfObject:
    case KindOfResource:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:
    case KindOfRecord:
      return false;

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return true;

    case KindOfRef:
      break;
  }
  not_reached();
}

template<typename T>
enable_if_lval_t<T, bool> tvCoerceParamToObjectInPlace(T tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  return type(tv) == KindOfObject;
}

template<typename T>
enable_if_lval_t<T, bool> tvCoerceParamToNullableObjectInPlace(
  T tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  if (isNullType(type(tv))) {
    // See comment in tvCastToNullableObjectInPlace
    val(tv).pobj = nullptr;
    return true;
  }
  return type(tv) == KindOfObject;
}

template<typename T>
enable_if_lval_t<T, bool> tvCoerceParamToResourceInPlace(
  T tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  return type(tv) == KindOfResource;
}
///////////////////////////////////////////////////////////////////////////////
#define X(kind) \
template void tvCastTo##kind##InPlace<TypedValue*>(TypedValue*); \
template void tvCastTo##kind##InPlace<tv_lval>(tv_lval); \
template void tvCastTo##kind##InPlace<arr_lval>(arr_lval);
#define Y(kind) \
X(kind) \
template bool tvCoerceParamTo##kind##InPlace<TypedValue*>(TypedValue*, bool); \
template bool tvCoerceParamTo##kind##InPlace<tv_lval>(tv_lval, bool); \
template bool tvCoerceParamTo##kind##InPlace<arr_lval>(arr_lval, bool);
Y(Boolean)
Y(Int64)
Y(Double)
Y(String)
Y(Vec)
Y(Dict)
Y(Keyset)
Y(Shape)
Y(Object)
Y(NullableObject)
Y(Resource)
X(VArray)
X(DArray)
#undef Y
#undef X

#define X(kind, IC) \
template void tvCastTo##kind##InPlace<TypedValue*, IC>(TypedValue*); \
template void tvCastTo##kind##InPlace<tv_lval, IC>(tv_lval); \
template void tvCastTo##kind##InPlace<arr_lval, IC>(arr_lval);
#define Y(kind, IC) \
X(kind, IC) \
template bool tvCoerceParamTo##kind##InPlace<TypedValue*, IC>( \
  TypedValue*, bool); \
template bool tvCoerceParamTo##kind##InPlace<tv_lval, IC>( \
  tv_lval, bool); \
template bool tvCoerceParamTo##kind##InPlace<arr_lval, IC>( \
  arr_lval, bool);
Y(Array, IntishCast::Cast)
Y(Array, IntishCast::None)
#undef Y
#undef X
}
