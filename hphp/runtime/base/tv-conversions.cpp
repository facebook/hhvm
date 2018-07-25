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
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/assertions.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

void tvCastToBooleanInPlace(TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  bool b;

  do {
    switch (tv->m_type) {
      case KindOfUninit:
      case KindOfNull:
        b = false;
        continue;

      case KindOfBoolean:
        return;

      case KindOfInt64:
        b = (tv->m_data.num != 0LL);
        continue;

      case KindOfDouble:
        b = (tv->m_data.dbl != 0);
        continue;

      case KindOfPersistentString:
        b = tv->m_data.pstr->toBoolean();
        continue;

      case KindOfString:
        b = tv->m_data.pstr->toBoolean();
        tvDecRefStr(tv);
        continue;

      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentArray:
        b = !tv->m_data.parr->empty();
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfArray:
        b = !tv->m_data.parr->empty();
        tvDecRefArr(tv);
        continue;

      case KindOfObject:
        b = tv->m_data.pobj->toBoolean();
        tvDecRefObj(tv);
        continue;

      case KindOfResource:
        b = tv->m_data.pres->data()->o_toBoolean();
        tvDecRefRes(tv);
        continue;

      case KindOfFunc:
        b = funcToStringHelper(tv->m_data.pfunc)->toBoolean();
        continue;

      case KindOfClass:
        b = classToStringHelper(tv->m_data.pclass)->toBoolean();
        continue;

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.num = b;
  tv->m_type = KindOfBoolean;
}

bool tvCastToBoolean(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->tv();
  }
  return cellToBool(tv);
}

void tvCastToDoubleInPlace(TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  double d;

  do {
    switch (tv->m_type) {
      case KindOfUninit:
      case KindOfNull:
        d = 0.0;
        continue;

      case KindOfBoolean:
        assertx(tv->m_data.num == 0LL || tv->m_data.num == 1LL);
        // fallthru
      case KindOfInt64:
        d = (double)(tv->m_data.num);
        continue;

      case KindOfDouble:
        return;

      case KindOfPersistentString:
        d = tv->m_data.pstr->toDouble();
        continue;

      case KindOfString:
        d = tv->m_data.pstr->toDouble();
        tvDecRefStr(tv);
        continue;

      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentArray:
        d = tv->m_data.parr->empty() ? 0 : 1;
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfArray:
        d = tv->m_data.parr->empty() ? 0 : 1;
        tvDecRefArr(tv);
        continue;

      case KindOfObject:
        d = tv->m_data.pobj->toDouble();
        tvDecRefObj(tv);
        continue;

      case KindOfResource:
        d = tv->m_data.pres->data()->o_toDouble();
        tvDecRefRes(tv);
        continue;

      case KindOfFunc:
        d = funcToStringHelper(tv->m_data.pfunc)->toDouble();
        continue;

      case KindOfClass:
        d = classToStringHelper(tv->m_data.pclass)->toDouble();
        continue;

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.dbl = d;
  tv->m_type = KindOfDouble;
}

void tvCastToInt64InPlace(TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);

  assertx(cellIsPlausible(*tv));
  int64_t i;

  do {
    switch (tv->m_type) {
      case KindOfUninit:
      case KindOfNull:
        tv->m_data.num = 0LL;
        // fallthru
      case KindOfBoolean:
        assertx(tv->m_data.num == 0LL || tv->m_data.num == 1LL);
        tv->m_type = KindOfInt64;
        // fallthru
      case KindOfInt64:
        return;

      case KindOfDouble:
        i = double_to_int64(tv->m_data.dbl);
        continue;

      case KindOfPersistentString:
        i = tv->m_data.pstr->toInt64();
        continue;

      case KindOfString:
        i = tv->m_data.pstr->toInt64();
        tvDecRefStr(tv);
        continue;

      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentArray:
        i = tv->m_data.parr->empty() ? 0 : 1;
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfArray:
        i = tv->m_data.parr->empty() ? 0 : 1;
        tvDecRefArr(tv);
        continue;

      case KindOfObject:
        i = tv->m_data.pobj->toInt64();
        tvDecRefObj(tv);
        continue;

      case KindOfResource:
        i = tv->m_data.pres->data()->o_toInt64();
        tvDecRefRes(tv);
        continue;

      case KindOfFunc:
        i = funcToStringHelper(tv->m_data.pfunc)->toInt64();
        continue;

      case KindOfClass:
        i = classToStringHelper(tv->m_data.pclass)->toInt64();
        continue;

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.num = i;
  tv->m_type = KindOfInt64;
}

int64_t tvCastToInt64(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->tv();
  }
  return cellToInt(tv);
}

double tvCastToDouble(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->tv();
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

    case KindOfRef:
      break;
  }
  not_reached();
}

const StaticString
  s_1("1"),
  s_scalar("scalar");

void tvCastToStringInPlace(TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
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

    case KindOfRef:
      break;
  }
  not_reached();
}

StringData* tvCastToStringData(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->tv();
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

    case KindOfRef:
      not_reached();
  }
  not_reached();
}

String tvCastToString(TypedValue tv) {
  return String::attach(tvCastToStringData(tv));
}

ArrayData* tvCastToArrayLikeData(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->tv();
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
    case KindOfPersistentArray:
    case KindOfArray: {
      auto const ad = tv.m_data.parr;
      ad->incRefCount();
      return ad;
    }

    case KindOfObject: {
      auto ad = tv.m_data.pobj->toArray();
      assertx(ad->isPHPArray());
      return ad.detach();
    }

    case KindOfRef:
      break;
  }
  not_reached();
}

Array tvCastToArrayLike(TypedValue tv) {
  return Array::attach(tvCastToArrayLikeData(tv));
}

void tvCastToArrayInPlace(TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  ArrayData* a;

  do {
    switch (tv->m_type) {
      case KindOfUninit:
      case KindOfNull:
        a = ArrayData::Create();
        continue;

      case KindOfBoolean:
      case KindOfInt64:
      case KindOfDouble:
      case KindOfPersistentString:
        a = ArrayData::Create(tvAsVariant(tv));
        continue;

      case KindOfString:
        a = ArrayData::Create(tvAsVariant(tv));
        tvDecRefStr(tv);
        continue;

      case KindOfPersistentVec: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToPHPArrayVec(adIn, true);
        assertx(a != adIn);
        continue;
      }

      case KindOfVec: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToPHPArrayVec(adIn, adIn->cowCheck());
        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentDict: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isDict());
        a = MixedArray::ToPHPArrayDict(adIn, true);
        assertx(a != adIn);
        continue;
      }

      case KindOfDict: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isDict());
        a = MixedArray::ToPHPArrayDict(adIn, adIn->cowCheck());
        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentKeyset: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isKeyset());
        a = SetArray::ToPHPArray(adIn, true);
        assertx(a != adIn);
        continue;
      }

      case KindOfKeyset: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isKeyset());
        a = SetArray::ToPHPArray(adIn, adIn->cowCheck());
        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentArray: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isPHPArray());
        if (adIn->isNotDVArray()) return;
        a = adIn->toPHPArray(true);
        assertx(a != adIn);
        continue;
      }

      case KindOfArray: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isPHPArray());
        if (adIn->isNotDVArray()) return;
        a = adIn->toPHPArray(adIn->cowCheck());
        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfObject:
        // For objects, we fall back on the Variant machinery
        tvAsVariant(tv) = tv->m_data.pobj->toArray();
        return;

      case KindOfResource:
        a = ArrayData::Create(tvAsVariant(tv));
        tvDecRefRes(tv);
        continue;

      case KindOfFunc:
      case KindOfClass:
        a = ArrayData::Create(tvAsVariant(tv));
        continue;

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  assertx(a->isPHPArray());
  assertx(a->isNotDVArray());

  tv->m_data.parr = a;
  tv->m_type = KindOfArray;
  assertx(cellIsPlausible(*tv));
}

void tvCastToVecInPlace(TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  ArrayData* a;

  do {
    switch (tv->m_type) {
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
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isDict());
        a = MixedArray::ToVecDict(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isKeyset());
        a = SetArray::ToVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isPHPArray());
        a = adIn->toVec(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentVec:
      case KindOfVec:
        assertx(tv->m_data.parr->isVecArray());
        return;

      case KindOfObject:
        a = castObjToVec(tv->m_data.pobj);
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

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.parr = a;
  tv->m_type = KindOfVec;
  assertx(cellIsPlausible(*tv));
}

void tvCastToDictInPlace(TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  ArrayData* a;

  do {
    switch (tv->m_type) {
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
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToDictVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isKeyset());
        a = SetArray::ToDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isPHPArray());
        a = adIn->toDict(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict:
        assertx(tv->m_data.parr->isDict());
        return;

      case KindOfObject:
        a = castObjToDict(tv->m_data.pobj);
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

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.parr = a;
  tv->m_type = KindOfDict;
  assertx(cellIsPlausible(*tv));
}

void tvCastToKeysetInPlace(TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  ArrayData* a;

  do {
    switch (tv->m_type) {
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
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToKeysetVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isDict());
        a = MixedArray::ToKeysetDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isPHPArray());
        a = adIn->toKeyset(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset:
        assertx(tv->m_data.parr->isKeyset());
        return;

      case KindOfObject:
        a = castObjToKeyset(tv->m_data.pobj);
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

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.parr = a;
  tv->m_type = KindOfKeyset;
  assertx(cellIsPlausible(*tv));
}

void tvCastToVArrayInPlace(TypedValue* tv) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  ArrayData* a;

  do {
    switch (tv->m_type) {
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
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToVArrayVec(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isDict());
        a = MixedArray::ToVArrayDict(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isKeyset());
        a = SetArray::ToVArray(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = tv->m_data.parr;
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
        a = castObjToVArray(tv->m_data.pobj);
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

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  assertx(a->isPacked());
  assertx(a->isVArray());

  tv->m_data.parr = a;
  tv->m_type = KindOfArray;
  assertx(cellIsPlausible(*tv));
}

void tvCastToDArrayInPlace(TypedValue* tv) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  ArrayData* a;

  do {
    switch (tv->m_type) {
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
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isVecArray());
        a = PackedArray::ToDArrayVec(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isDict());
        a = MixedArray::ToDArrayDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = tv->m_data.parr;
        assertx(adIn->isKeyset());
        a = SetArray::ToDArray(adIn, adIn->cowCheck());
        assertx(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = tv->m_data.parr;
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
        a = castObjToDArray(tv->m_data.pobj);
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

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  assertx(a->isMixed());
  assertx(a->isDArray());

  tv->m_data.parr = a;
  tv->m_type = KindOfArray;
  assertx(cellIsPlausible(*tv));
}

ObjectData* tvCastToObjectData(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefType(tv.m_type)) {
    tv = *tv.m_data.pref->tv();
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

    case KindOfRef:
      break;
  }
  not_reached();
}

Object tvCastToObject(TypedValue tv) {
  return Object::attach(tvCastToObjectData(tv));
}

void tvCastToObjectInPlace(TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  ObjectData* o;

  do {
    switch (tv->m_type) {
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
        tvAsVariant(tv) = ObjectData::FromArray(tv->m_data.parr);
        return;

      case KindOfObject:
        return;

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.pobj = o;
  tv->m_type = KindOfObject;
  assertx(cellIsPlausible(*tv));
}

void tvCastToNullableObjectInPlace(TypedValue* tv) {
  if (isNullType(tv->m_type)) {
    // XXX(t3879280) This happens immediately before calling an extension
    // function that takes an optional Object argument. We want to end up
    // passing const Object& holding nullptr, so by clearing out m_data.pobj we
    // can unconditionally treat &tv->m_data.pobj as a const Object& in the
    // function being called. This violates the invariant that the value of
    // m_data doesn't matter in a KindOfNull TypedValue.
    tv->m_data.pobj = nullptr;
  } else {
    tvCastToObjectInPlace(tv);
  }
}

void tvCastToResourceInPlace(TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);

  do {
    switch (tv->m_type) {
      DT_UNCOUNTED_CASE:
        continue;
      case KindOfString:
      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfArray:
      case KindOfObject:
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

  tv->m_type = KindOfResource;
  tv->m_data.pres = req::make<DummyResource>().detach()->hdr();
  assertx(cellIsPlausible(*tv));
}

bool tvCoerceParamToBooleanInPlace(TypedValue* tv,
                                   bool builtin) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);

  switch (tv->m_type) {
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
      tvCastToBooleanInPlace(tv);
      return true;

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
      return false;

    case KindOfRef:
      break;
  }
  not_reached();
}

static bool tvCanBeCoercedToNumber(const TypedValue* tv,
                                   bool builtin) {
  switch (tv->m_type) {
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
      return tv->m_data.pstr->isNumeric();

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return false;

    case KindOfRef:
      break;
  }
  not_reached();
}

bool tvCoerceParamToInt64InPlace(TypedValue* tv,
                                 bool builtin) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  if (!tvCanBeCoercedToNumber(tv, builtin)) {
    return false;
  }
  // In PHP 7 mode doubles only convert to integers when the conversion is non-
  // narrowing
  if (RuntimeOption::PHP7_ScalarTypes && tv->m_type == KindOfDouble) {
    if (tv->m_data.dbl < std::numeric_limits<int64_t>::min()) return false;
    if (tv->m_data.dbl > std::numeric_limits<int64_t>::max()) return false;
    if (std::isnan(tv->m_data.dbl)) return false;
  }
  tvCastToInt64InPlace(tv);
  return true;
}

bool tvCoerceParamToDoubleInPlace(TypedValue* tv,
                                  bool builtin) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  if (!tvCanBeCoercedToNumber(tv, builtin)) {
    return false;
  }
  tvCastToDoubleInPlace(tv);
  return true;
}

bool tvCoerceParamToStringInPlace(TypedValue* tv,
                                  bool builtin) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);

  switch (tv->m_type) {
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
    case KindOfPersistentArray:
    case KindOfArray:
      return false;

    case KindOfObject:
      if (tv->m_data.pobj->hasToString()) {
        tvAsVariant(tv) = tv->m_data.pobj->invokeToString();
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

bool tvCoerceParamToArrayInPlace(TypedValue* tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);

  switch (tv->m_type) {
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
      return false;

    case KindOfPersistentArray:
    case KindOfArray:
      return true;

    case KindOfObject:
      if (LIKELY(tv->m_data.pobj->isCollection())) {
        tvAsVariant(tv) = tv->m_data.pobj->toArray();
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

bool tvCoerceParamToVecInPlace(TypedValue* tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);

  switch (tv->m_type) {
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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfFunc:
    case KindOfClass:
      return false;

    case KindOfPersistentVec:
    case KindOfVec:
      return true;

    case KindOfRef:
      break;
  }
  not_reached();
}

bool tvCoerceParamToDictInPlace(TypedValue* tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);

  switch (tv->m_type) {
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
      return false;

    case KindOfPersistentDict:
    case KindOfDict:
      return true;

    case KindOfRef:
      break;
  }
  not_reached();
}

bool tvCoerceParamToKeysetInPlace(TypedValue* tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);

  switch (tv->m_type) {
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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfFunc:
    case KindOfClass:
      return false;

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return true;

    case KindOfRef:
      break;
  }
  not_reached();
}

bool tvCoerceParamToObjectInPlace(TypedValue* tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  return tv->m_type == KindOfObject;
}

bool tvCoerceParamToNullableObjectInPlace(TypedValue* tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  if (isNullType(tv->m_type)) {
    // See comment in tvCastToNullableObjectInPlace
    tv->m_data.pobj = nullptr;
    return true;
  }
  return tv->m_type == KindOfObject;
}

bool tvCoerceParamToResourceInPlace(TypedValue* tv, bool /*builtin*/) {
  assertx(tvIsPlausible(*tv));
  tvUnboxIfNeeded(*tv);
  return tv->m_type == KindOfResource;
}

///////////////////////////////////////////////////////////////////////////////

}
