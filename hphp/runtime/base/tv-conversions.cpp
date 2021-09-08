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
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/vanilla-keyset.h"
#include "hphp/runtime/base/vanilla-vec.h"

#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/assertions.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

namespace {
template <typename T>
ArrayData* makeSingletonDict(const T& t) {
  DictInit init(1);
  init.append(t);
  return init.create();
}
}

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

      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
        b = !val(tv).parr->empty();
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
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
        b = true;
        continue;

      case KindOfRClsMeth:
        tvDecRefRClsMeth(tv);
        b = true;
        continue;

      case KindOfRFunc:
        tvDecRefRFunc(tv);
        b = true;
        continue;

      case KindOfFunc:
      case KindOfClass:
      case KindOfLazyClass:
        b = true;
        continue;
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

      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
        d = val(tv).parr->empty() ? 0 : 1;
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
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

      case KindOfRFunc:
        raise_convert_rfunc_to_type("double");

      case KindOfFunc:
        invalidFuncConversion("double");

      case KindOfClass:
        d = classToStringHelper(val(tv).pclass)->toDouble();
        continue;

      case KindOfLazyClass:
        d = lazyClassToStringHelper(val(tv).plazyclass)->toDouble();
        continue;

      case KindOfClsMeth:
        throwInvalidClsMethToType("double");

      case KindOfRClsMeth:
        raise_convert_rcls_meth_to_type("double");
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

      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
        i = val(tv).parr->empty() ? 0 : 1;
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
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

      case KindOfRFunc:
        raise_convert_rfunc_to_type("int");

      case KindOfFunc:
        invalidFuncConversion("int");
        continue;

      case KindOfClass:
        i = classToStringHelper(val(tv).pclass)->toInt64();
        continue;

      case KindOfLazyClass:
        i = lazyClassToStringHelper(val(tv).plazyclass)->toInt64();
        continue;

      case KindOfClsMeth:
        throwInvalidClsMethToType("int");

      case KindOfRClsMeth:
        raise_convert_rcls_meth_to_type("int");
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

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return tv.m_data.parr->empty() ? 0.0 : 1.0;

    case KindOfObject:
      return tv.m_data.pobj->toDouble();

    case KindOfResource:
      return tv.m_data.pres->data()->o_toDouble();

    case KindOfRFunc:
      raise_convert_rfunc_to_type("double");

    case KindOfFunc:
      invalidFuncConversion("double");

    case KindOfClass:
      return classToStringHelper(tv.m_data.pclass)->toDouble();

    case KindOfLazyClass:
      return lazyClassToStringHelper(tv.m_data.plazyclass)->toDouble();

    case KindOfClsMeth:
      throwInvalidClsMethToType("double");

    case KindOfRClsMeth:
      raise_convert_rcls_meth_to_type("double");
  }
  not_reached();
}

const StaticString
  s_1("1"),
  s_scalar("scalar");

template<typename T> enable_if_lval_t<T, void> tvCastToStringInPlace(
  T tv, const ConvNoticeLevel notice_level, const StringData* notice_reason) {
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
      handleConvNoticeLevel(notice_level, "null", "string", notice_reason);
      return persistentString(staticEmptyString());

    case KindOfBoolean:
      handleConvNoticeLevel(notice_level, "bool", "string", notice_reason);
      return persistentString(val(tv).num ? s_1.get() : staticEmptyString());

    case KindOfInt64:
      return string(buildStringData(val(tv).num));

    case KindOfDouble:
      handleConvNoticeLevel(notice_level, "double", "string", notice_reason);
      return string(buildStringData(val(tv).dbl));

    case KindOfPersistentString:
    case KindOfString:
      return;

    case KindOfVec:
    case KindOfPersistentVec:
      SystemLib::throwInvalidOperationExceptionObject(
        "Vec to string conversion"
      );

    case KindOfDict:
    case KindOfPersistentDict:
      SystemLib::throwInvalidOperationExceptionObject(
        "Dict to string conversion"
      );

    case KindOfKeyset:
    case KindOfPersistentKeyset:
      SystemLib::throwInvalidOperationExceptionObject(
        "Keyset to string conversion"
      );

    case KindOfObject:
      tvMove(
        make_tv<KindOfString>(val(tv).pobj->invokeToString().detach()),
        tv
      );
      return;

    case KindOfResource:
      handleConvNoticeLevel(notice_level, "resource", "string", notice_reason);
      tvMove(
        make_tv<KindOfString>(val(tv).pres->data()->o_toString().detach()),
        tv
      );
      return;

    case KindOfRFunc:
      raise_convert_rfunc_to_type("string");

    case KindOfFunc: {
      invalidFuncConversion("string");
    }

    case KindOfClass: {
      auto const s = classToStringHelper(val(tv).pclass);
      return persistentString(const_cast<StringData*>(s));
    }

    case KindOfLazyClass: {
      auto const s = lazyClassToStringHelper(val(tv).plazyclass);
      return persistentString(const_cast<StringData*>(s));
    }

    case KindOfClsMeth:
      throwInvalidClsMethToType("string");

    case KindOfRClsMeth:
      raise_convert_rcls_meth_to_type("string");
  }
  not_reached();
}

template<typename T> enable_if_lval_t<T, void> tvCastToStringInPlace(T tv) {
  tvCastToStringInPlace(tv, ConvNoticeLevel::None, nullptr);
}

void tvSetLegacyArrayInPlace(tv_lval tv, bool isLegacy) {
  if (!tvIsVec(tv) && !tvIsDict(tv)) return;

  auto const adIn = val(tv).parr;
  auto const ad = adIn->setLegacyArray(adIn->cowCheck(), isLegacy);
  if (ad == adIn) return;

  decRefArr(adIn);
  val(tv).parr = ad;
  type(tv) = dt_with_rc(type(tv));
  assertx(tvIsPlausible(*tv));
}

StringData* tvCastToStringData(
  TypedValue tv,
  const ConvNoticeLevel notice_level,
  const StringData* notice_reason) {
  assertx(tvIsPlausible(tv));

  switch (tv.m_type) {
    case KindOfUninit:
    case KindOfNull:
      handleConvNoticeLevel(notice_level, "null", "string", notice_reason);
      return staticEmptyString();

    case KindOfBoolean:
      handleConvNoticeLevel(notice_level, "bool", "string", notice_reason);
      return tv.m_data.num ? s_1.get() : staticEmptyString();

    case KindOfInt64:
      return buildStringData(tv.m_data.num);

    case KindOfDouble:
      handleConvNoticeLevel(notice_level, "double", "string", notice_reason);
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
      SystemLib::throwInvalidOperationExceptionObject(
        "Vec to string conversion"
      );

    case KindOfPersistentDict:
    case KindOfDict:
      SystemLib::throwInvalidOperationExceptionObject(
        "Dict to string conversion"
      );

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      SystemLib::throwInvalidOperationExceptionObject(
        "Keyset to string conversion"
      );

    case KindOfObject:
      // not adding new conversion notice here because it's
      // handled downstream via another notice
      return tv.m_data.pobj->invokeToString().detach();

    case KindOfResource:
      handleConvNoticeLevel(notice_level, "resource", "string", notice_reason);
      return tv.m_data.pres->data()->o_toString().detach();

    case KindOfRFunc:
      raise_convert_rfunc_to_type("string");

    case KindOfFunc: {
      invalidFuncConversion("string");
    }

    case KindOfClass: {
      auto const s = classToStringHelper(tv.m_data.pclass);
      return const_cast<StringData*>(s);
    }

    case KindOfLazyClass: {
      auto const s = lazyClassToStringHelper(tv.m_data.plazyclass);
      return const_cast<StringData*>(s);
    }

    case KindOfClsMeth:
      throwInvalidClsMethToType("string");

    case KindOfRClsMeth:
      raise_convert_rcls_meth_to_type("string");
  }
  not_reached();
}

StringData* tvCastToStringData(TypedValue tv) {
  return tvCastToStringData(tv, ConvNoticeLevel::None, nullptr);
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
      return ArrayData::CreateDict();

    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
    case KindOfLazyClass:
      return makeSingletonDict(tv);

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset: {
      auto const ad = tv.m_data.parr;
      ad->incRefCount();
      return ad;
    }

    case KindOfClsMeth:
      throwInvalidClsMethToType("array");

    case KindOfRClsMeth:
      raise_convert_rcls_meth_to_type("array");

    case KindOfObject: {
      auto ad = tv.m_data.pobj->toArray<IC>();
      // When we cast a closure to an array, we get back a vec.
      assertx(ad->isDictType() || ad->isVecType());
      return ad.detach();
    }

    case KindOfRFunc:
      raise_convert_rfunc_to_type("array");
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

template<typename T, IntishCast IC /* = IntishCast::None */>
enable_if_lval_t<T, void> tvCastToArrayInPlace(T tv) {
  assertx(tvIsPlausible(*tv));
  ArrayData* a;

  do {
    switch (type(tv)) {
      case KindOfUninit:
      case KindOfNull:
        a = ArrayData::CreateDict();
        continue;

      case KindOfBoolean:
      case KindOfInt64:
      case KindOfDouble:
      case KindOfPersistentString:
        a = makeSingletonDict(*tv);
        continue;

      case KindOfString:
        a = makeSingletonDict(*tv);
        tvDecRefStr(tv);
        continue;

      case KindOfPersistentVec:
      case KindOfVec:
      case KindOfPersistentDict:
      case KindOfDict:
      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
        a = [&]{
          assertx(IC == IntishCast::Cast || IC == IntishCast::None);
          return IC == IntishCast::Cast
            ? adIn->toDictIntishCast(adIn->cowCheck())
            : adIn->toDict(adIn->cowCheck());
        }();
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfObject: {
        a = val(tv).pobj->template toArray<IC>().toDict().detach();
        tvDecRefObj(tv);
        continue;
      }

      case KindOfResource:
        a = makeSingletonDict(*tv);
        tvDecRefRes(tv);
        continue;

      case KindOfFunc:
      case KindOfClass:
      case KindOfLazyClass:
        a = makeSingletonDict(*tv);
        continue;

      case KindOfClsMeth:
        throwInvalidClsMethToType("array");

      case KindOfRClsMeth:
        raise_convert_rcls_meth_to_type("array");

      case KindOfRFunc:
        raise_convert_rfunc_to_type("array");
    }
    not_reached();
  } while (0);

  *tv = make_array_like_tv(a);
  assertx(tvIsPlausible(*tv));
  assertx(a->isDictType());
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
      case KindOfDict:
      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
        a = adIn->toVec(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentVec:
      case KindOfVec:
        assertx(val(tv).parr->isVecType());
        return;

      case KindOfObject:
        a = castObjToVec(val(tv).pobj);
        // We might have re-entered, so tv may not contain the object anymore.
        tvDecRefGen(tv);
        continue;

      case KindOfRFunc:
        raise_convert_rfunc_to_type("vec");

      case KindOfFunc:
        SystemLib::throwInvalidOperationExceptionObject(
          "Func to vec conversion"
        );

      case KindOfClass:
        SystemLib::throwInvalidOperationExceptionObject(
          "Class to vec conversion"
        );

      case KindOfLazyClass:
        SystemLib::throwInvalidOperationExceptionObject(
          "Lazy class to vec conversion"
        );

      case KindOfClsMeth:
        SystemLib::throwInvalidOperationExceptionObject(
          "ClsMeth to vec conversion"
        );

      case KindOfRClsMeth:
        raise_convert_rcls_meth_to_type("vec");
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
      case KindOfVec:
      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = val(tv).parr;
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

      case KindOfRFunc:
        raise_convert_rfunc_to_type("dict");

      case KindOfFunc:
        SystemLib::throwInvalidOperationExceptionObject(
          "Func to dict conversion"
        );

      case KindOfClass:
        SystemLib::throwInvalidArgumentExceptionObject(
          "Class to dict conversion"
        );

      case KindOfLazyClass:
        SystemLib::throwInvalidArgumentExceptionObject(
          "Lazy class to dict conversion"
        );

      case KindOfClsMeth: {
        SystemLib::throwInvalidOperationExceptionObject(
          "ClsMeth to dict conversion"
        );
      }

      case KindOfRClsMeth:
        raise_convert_rcls_meth_to_type("dict");
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
      case KindOfVec:
      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = val(tv).parr;
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

      case KindOfRFunc:
        raise_convert_rfunc_to_type("keyset");

      case KindOfFunc:
        SystemLib::throwInvalidOperationExceptionObject(
          "Func to keyset conversion"
        );

      case KindOfClass:
        SystemLib::throwInvalidOperationExceptionObject(
          "Class to keyset conversion"
        );

      case KindOfLazyClass:
        SystemLib::throwInvalidOperationExceptionObject(
          "Lazy class to keyset conversion"
        );

      case KindOfClsMeth: {
        SystemLib::throwInvalidOperationExceptionObject(
          "ClsMeth to keyset conversion"
        );
      }

      case KindOfRClsMeth:
        raise_convert_rcls_meth_to_type("keyset");
    }
    not_reached();
  } while (0);

  val(tv).parr = a;
  type(tv) = KindOfKeyset;
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
    case KindOfLazyClass:
    case KindOfResource: {
      DictInit props(1);
      props.set(s_scalar, tv);
      return ObjectData::FromArray(props.create()).detach();
    }

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return ObjectData::FromArray(tv.m_data.parr).detach();

    case KindOfObject:
      tv.m_data.pobj->incRefCount();
      return tv.m_data.pobj;

    case KindOfClsMeth:
      throwInvalidClsMethToType("object");

    case KindOfRClsMeth:
      raise_convert_rcls_meth_to_type("object");

    case KindOfRFunc:
      raise_convert_rfunc_to_type("object");
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
      case KindOfRFunc:
      case KindOfString:
      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfObject:
      case KindOfRClsMeth:
        tvDecRefCountable(tv);
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

char const *conv_notice_level_names[] = { "None", "Log", "Throw" };

const char* convOpToName(ConvNoticeLevel level) {
  return conv_notice_level_names[
    static_cast<uint8_t>(level) - static_cast<uint8_t>(ConvNoticeLevel::None)
  ];
}

void handleConvNoticeLevel(
  ConvNoticeLevel level,
  const char* const from,
  const char* const to,
  const StringData* reason) {
  if (level == ConvNoticeLevel::None) return;
  assertx(reason != nullptr);
  const auto str =
    folly::sformat("Implicit {} to {} conversion for {}", from, to, reason);
  if (level == ConvNoticeLevel::Throw) {
    SystemLib::throwInvalidOperationExceptionObject(str);
  } else if (level == ConvNoticeLevel::Log) {
    raise_notice(str);
  }
}

const StaticString s_ConvNoticeReasonConcat("string concatenation/interpolation");

void throwBitOpBadTypesException(tv_rval t1, tv_rval t2) {
  SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
    "Cannot perform a bitwise operation on {} and {}",
    describe_actual_type(t1), describe_actual_type(t2)));
}
void throwIncDecBadTypeException(const char* t) {
  SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
    "Cannot perform pre/post inc/dec on {}", t));
}
void throwMathBadTypesException(tv_rval t1, tv_rval t2) {
  SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
    "Cannot perform mathematical operation on {} and {}",
    describe_actual_type(t1), describe_actual_type(t2)));
}

namespace {
void throwCmpBadTypesExceptionImpl(tv_rval t1, const char* t2) {
  SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
    "Cannot compare {} and {} using a relational operator",
    describe_actual_type(t1), t2));
}
}

void throwCmpBadTypesException(tv_rval t1, tv_rval t2) {
  throwCmpBadTypesExceptionImpl(t1, describe_actual_type(t2).c_str());
}
void throwCmpBadTypesException(tv_rval t1, DataType dt) {
   const char* rhs = [&] {
      switch(dt) {
        case DataType::Boolean:  return "bool";
        case DataType::Int64:    return "int";
        case DataType::Double:   return "float";
        case DataType::String:   return "string";
        case DataType::Vec:      return "vec";
        case DataType::Dict:     return "dict";
        case DataType::Keyset:   return "keyset";
        case DataType::Resource: return "resource";
        case DataType::ClsMeth:  return "clsmeth";
        default: always_assert(false);
      }
      not_reached();
   }();
  throwCmpBadTypesExceptionImpl(t1, rhs);
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
#undef X

#define X(kind) template void \
  tvCastToStringInPlace<kind>(kind, const ConvNoticeLevel, const StringData*);
X(TypedValue*)
X(tv_lval)
X(arr_lval)
#undef X

#define X(kind, IC) \
template void tvCastTo##kind##InPlace<TypedValue*, IC>(TypedValue*); \
template void tvCastTo##kind##InPlace<tv_lval, IC>(tv_lval); \
template void tvCastTo##kind##InPlace<arr_lval, IC>(arr_lval);
X(Array, IntishCast::Cast)
X(Array, IntishCast::None)
#undef X
}
