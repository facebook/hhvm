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

      case KindOfPersistentDArray:
      case KindOfPersistentVArray:
      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentArray:
        b = !val(tv).parr->empty();
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfDArray:
      case KindOfVArray:
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

      case KindOfClsMeth:
        tvDecRefClsMeth(tv);
      case KindOfFunc:
      case KindOfClass:
        b = true;
        continue;

      case KindOfRecord:
        raise_convert_record_to_type("bool");
    }
    not_reached();
  } while (0);

  val(tv).num = b;
  type(tv) = KindOfBoolean;
}

bool tvCastToBoolean(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  return tvToBool(tv);
}

template<typename T>
enable_if_lval_t<T, void> tvCastToDoubleInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
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

      case KindOfPersistentDArray:
      case KindOfPersistentVArray:
      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentArray:
        d = val(tv).parr->empty() ? 0 : 1;
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfDArray:
      case KindOfVArray:
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
        if (RuntimeOption::EvalRaiseFuncConversionWarning) {
          raise_warning("Func to double conversion");
        }
        d = val(tv).pfunc->name()->toDouble();
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
    }
    not_reached();
  } while (0);

  val(tv).dbl = d;
  type(tv) = KindOfDouble;
}

template<typename T>
enable_if_lval_t<T, void> tvCastToInt64InPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  assertx(tvIsPlausible(*tv));
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

      case KindOfPersistentDArray:
      case KindOfPersistentVArray:
      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentArray:
        i = val(tv).parr->empty() ? 0 : 1;
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfDArray:
      case KindOfVArray:
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
        i = funcToInt64Helper(val(tv).pfunc);
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
    }
    not_reached();
  } while (0);

  val(tv).num = i;
  type(tv) = KindOfInt64;
}

int64_t tvCastToInt64(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  return tvToInt(tv);
}

double tvCastToDouble(TypedValue tv) {
  assertx(tvIsPlausible(tv));
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

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
      return tv.m_data.parr->empty() ? 0.0 : 1.0;

    case KindOfObject:
      return tv.m_data.pobj->toDouble();

    case KindOfResource:
      return tv.m_data.pres->data()->o_toDouble();

    case KindOfFunc:
      if (RuntimeOption::EvalRaiseFuncConversionWarning) {
        raise_warning("Func to double conversion");
      }
      return tv.m_data.pfunc->name()->toDouble();

    case KindOfClass:
      return classToStringHelper(tv.m_data.pclass)->toDouble();

    case KindOfClsMeth:
      raiseClsMethConvertWarningHelper("double");
      return 1.0;

    case KindOfRecord:
      raise_convert_record_to_type("double");
  }
  not_reached();
}

const StaticString
  s_1("1"),
  s_scalar("scalar");

template<typename T>
enable_if_lval_t<T, void> tvCastToStringInPlace(T tv) {
  assertx(tvIsPlausible(*tv));

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

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfArray:
    case KindOfPersistentArray:
      raise_notice("Array to string conversion");
      if (isRefcountedType(type(tv))) {
        tvDecRefArr(*tv);
      }
      return persistentString(array_string.get());

    case KindOfObject:
      tvMove(
        make_tv<KindOfString>(val(tv).pobj->invokeToString().detach()),
        tv
      );
      return;

    case KindOfResource:
      tvMove(
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
      if (RuntimeOption::EvalRaiseClsMethConversionWarning) {
        raise_notice("Implicit clsmeth to string conversion");
      }
      tvDecRefClsMeth(tv);
      if (RuntimeOption::EvalHackArrDVArrs) {
        return persistentString(vec_string.get());
      } else {
        return persistentString(array_string.get());
      }

    case KindOfRecord:
      raise_convert_record_to_type("string");
  }
  not_reached();
}

StringData* tvCastToStringData(TypedValue tv) {
  assertx(tvIsPlausible(tv));

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

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
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
      if (RuntimeOption::EvalRaiseClsMethConversionWarning) {
        raise_notice("Implicit clsmeth to string conversion");
      }
      if (RuntimeOption::EvalHackArrDVArrs) {
        return vec_string.get();
      } else {
        return array_string.get();
      }

    case KindOfRecord:
      raise_convert_record_to_type("string");
  }
  not_reached();
}

String tvCastToString(TypedValue tv) {
  return String::attach(tvCastToStringData(tv));
}

template <IntishCast IC>
ArrayData* tvCastToArrayLikeData(TypedValue tv) {
  assertx(tvIsPlausible(tv));
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

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
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
      assertx(ad->isPHPArrayType());
      return ad.detach();
    }

    case KindOfRecord:
      raise_convert_record_to_type("array");
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

template<typename T, IntishCast IC /* = IntishCast::None */>
enable_if_lval_t<T, void> tvCastToArrayInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
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
        assertx(adIn->isVecArrayKind());
        a = PackedArray::ToPHPArrayVec(adIn, true);
        assertx(a != adIn);
        continue;
      }

      case KindOfVec: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isVecArrayKind());
        a = PackedArray::ToPHPArrayVec(adIn, adIn->cowCheck());
        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentDict: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isDictKind());

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
        assertx(adIn->isDictKind());

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
        assertx(adIn->isKeysetKind());

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
        assertx(adIn->isKeysetKind());

        if (IC == IntishCast::Cast) {
          a = SetArray::ToPHPArrayIntishCast(adIn, adIn->cowCheck());
        } else {
          assertx(IC == IntishCast::None);
          a = SetArray::ToPHPArray(adIn, adIn->cowCheck());
        }

        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentDArray:
      case KindOfPersistentVArray:
      case KindOfPersistentArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArrayType());
        if (IC == IntishCast::Cast) {
          a = adIn->toPHPArrayIntishCast(true);
        } else {
          assertx(IC == IntishCast::None);
          a = adIn->toPHPArray(true);
        }
        continue;
      }

      case KindOfDArray:
      case KindOfVArray:
      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArrayType());
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
        raiseClsMethConvertWarningHelper("array");
        a = make_packed_array(
          val(tv).pclsmeth->getClsStr(), val(tv).pclsmeth->getFuncStr()
        ).detach();
        tvDecRefClsMeth(tv);
        continue;
      }

      case KindOfRecord:
        raise_convert_record_to_type("array");
    }
    not_reached();
  } while (0);

  assertx(a->isPHPArrayType());
  assertx(a->isNotDVArray());

  val(tv).parr = a;
  type(tv) = KindOfArray;
  assertx(tvIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToVecInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
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
        assertx(adIn->isDictKind());
        a = MixedArray::ToVecDict(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isKeysetKind());
        a = SetArray::ToVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDArray:
      case KindOfDArray:
      case KindOfPersistentVArray:
      case KindOfVArray:
      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArrayType());
        a = adIn->toVec(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentVec:
      case KindOfVec:
        assertx(val(tv).parr->isVecArrayType());
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
        raiseClsMethConvertWarningHelper("vec");
        a = make_vec_array(
          val(tv).pclsmeth->getClsStr(), val(tv).pclsmeth->getFuncStr()
        ).detach();
        tvDecRefClsMeth(tv);
        continue;
      }

      case KindOfRecord:
        raise_convert_record_to_type("vec");
    }
    not_reached();
  } while (0);

  val(tv).parr = a;
  type(tv) = KindOfVec;
  assertx(tvIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToDictInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
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
        assertx(adIn->isVecArrayKind());
        a = PackedArray::ToDictVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isKeysetKind());
        a = SetArray::ToDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDArray:
      case KindOfDArray:
      case KindOfPersistentVArray:
      case KindOfVArray:
      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArrayType());
        a = adIn->toDict(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict:
        assertx(val(tv).parr->isDictType());
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
        raiseClsMethConvertWarningHelper("dict");
        a = make_dict_array(
          0, val(tv).pclsmeth->getClsStr(),
          1, val(tv).pclsmeth->getFuncStr()).detach();
        tvDecRefClsMeth(tv);
        continue;
      }

      case KindOfRecord:
        raise_convert_record_to_type("dict");
    }
    not_reached();
  } while (0);

  val(tv).parr = a;
  type(tv) = KindOfDict;
  assertx(tvIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToKeysetInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
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
        assertx(adIn->isVecArrayKind());
        a = PackedArray::ToKeysetVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isDictKind());
        a = MixedArray::ToKeysetDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDArray:
      case KindOfDArray:
      case KindOfPersistentVArray:
      case KindOfVArray:
      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArrayType());
        a = adIn->toKeyset(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset:
        assertx(val(tv).parr->isKeysetType());
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

      case KindOfClsMeth: {
        raiseClsMethConvertWarningHelper("keyset");
        a = make_keyset_array(
          const_cast<StringData*>(val(tv).pclsmeth->getCls()->name()),
          const_cast<StringData*>(val(tv).pclsmeth->getFunc()->name())
        ).detach();
        tvDecRefClsMeth(tv);
        continue;
      }

      case KindOfRecord:
        raise_convert_record_to_type("keyset");
    }
    not_reached();
  } while (0);

  val(tv).parr = a;
  type(tv) = KindOfKeyset;
  assertx(tvIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToVArrayInPlace(T tv) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  assertx(tvIsPlausible(*tv));
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
        assertx(adIn->isVecArrayKind());
        a = PackedArray::ToVArrayVec(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isDictKind());
        a = MixedArray::ToVArrayDict(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isKeysetKind());
        a = SetArray::ToVArray(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDArray:
      case KindOfDArray:
      case KindOfPersistentVArray:
      case KindOfVArray:
      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArrayType());
        if (adIn->isVArray()) return;
        a = adIn->toVArray(adIn->cowCheck());
        assertx(a->isPackedKind());
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
        raiseClsMethConvertWarningHelper("varray");
        a = make_varray(
          val(tv).pclsmeth->getClsStr(), val(tv).pclsmeth->getFuncStr()
        ).detach();
        tvDecRefClsMeth(tv);
        continue;
      }

      case KindOfRecord:
        raise_convert_record_to_type("varray");
    }
    not_reached();
  } while (0);

  assertx(a->isPackedKind());
  assertx(a->isVArray());
  assertx(a->dvArraySanityCheck());

  val(tv).parr = a;
  type(tv) = RuntimeOption::EvalEmitDVArray ? KindOfVArray : KindOfArray;
  assertx(tvIsPlausible(*tv));
}

template<typename T>
enable_if_lval_t<T, void> tvCastToDArrayInPlace(T tv) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  assertx(tvIsPlausible(*tv));
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
        assertx(adIn->isVecArrayKind());
        a = PackedArray::ToDArrayVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isDictKind());
        a = MixedArray::ToDArrayDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isKeysetKind());
        a = SetArray::ToDArray(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDArray:
      case KindOfDArray:
      case KindOfPersistentVArray:
      case KindOfVArray:
      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = val(tv).parr;
        assertx(adIn->isPHPArrayType());
        if (adIn->isDArray()) return;
        a = adIn->toDArray(adIn->cowCheck());
        assertx(a->isMixedKind());
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
        raiseClsMethConvertWarningHelper("darray");
        a = make_darray(
          0, val(tv).pclsmeth->getClsStr(),
          1, val(tv).pclsmeth->getFuncStr()
        ).detach();
        tvDecRefClsMeth(tv);
        continue;
      }

      case KindOfRecord:
        raise_convert_record_to_type("darray");
    }
    not_reached();
  } while (0);

  assertx(a->isMixedKind());
  assertx(a->isDArray());
  assertx(a->dvArraySanityCheck());

  val(tv).parr = a;
  type(tv) = RuntimeOption::EvalEmitDVArray ? KindOfDArray : KindOfArray;
  assertx(tvIsPlausible(*tv));
}

ObjectData* tvCastToObjectData(TypedValue tv) {
  assertx(tvIsPlausible(tv));
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
    case KindOfPersistentKeyset:
    case KindOfKeyset: {
      auto const arr = Array::attach(tv.m_data.parr->toPHPArray(true));
      return ObjectData::FromArray(arr.get()).detach();
    }

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      return ObjectData::FromArray(tv.m_data.parr).detach();

    case KindOfObject:
      tv.m_data.pobj->incRefCount();
      return tv.m_data.pobj;

    case KindOfClsMeth: {
      raiseClsMethConvertWarningHelper("array");
      auto arr = make_varray(
        val(tv).pclsmeth->getClsStr(), val(tv).pclsmeth->getFuncStr());
      return ObjectData::FromArray(arr.get()).detach();
    }

    case KindOfRecord:
      raise_convert_record_to_type("object");
  }
  not_reached();
}

template<typename T>
enable_if_lval_t<T, void> tvCastToResourceInPlace(T tv) {
  assertx(tvIsPlausible(*tv));

  do {
    switch (type(tv)) {
      DT_UNCOUNTED_CASE:
        continue;
      case KindOfString:
      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfDArray:
      case KindOfVArray:
      case KindOfArray:
      case KindOfObject:
      case KindOfRecord:
        tvDecRefCountable(tv);
        continue;
      case KindOfClsMeth:
        tvDecRefClsMeth(tv);
        continue;
      case KindOfResource:
        // no op, return
        return;
    }
    not_reached();
  } while (0);

  type(tv) = KindOfResource;
  val(tv).pres = req::make<DummyResource>().detach()->hdr();
  assertx(tvIsPlausible(*tv));
}

///////////////////////////////////////////////////////////////////////////////
#define X(kind) \
template void tvCastTo##kind##InPlace<TypedValue*>(TypedValue*); \
template void tvCastTo##kind##InPlace<tv_lval>(tv_lval); \
template void tvCastTo##kind##InPlace<arr_lval>(arr_lval);
X(Boolean)
X(Int64)
X(Double)
X(String)
X(Vec)
X(Dict)
X(Keyset)
X(Resource)
X(VArray)
X(DArray)
#undef X

#define X(kind, IC) \
template void tvCastTo##kind##InPlace<TypedValue*, IC>(TypedValue*); \
template void tvCastTo##kind##InPlace<tv_lval, IC>(tv_lval); \
template void tvCastTo##kind##InPlace<arr_lval, IC>(arr_lval);
X(Array, IntishCast::Cast)
X(Array, IntishCast::None)
#undef X
}
