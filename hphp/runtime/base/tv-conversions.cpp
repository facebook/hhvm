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
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/assertions.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

void tvCastToBooleanInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
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

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.num = b;
  tv->m_type = KindOfBoolean;
}

void tvCastToDoubleInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  double d;

  do {
    switch (tv->m_type) {
      case KindOfUninit:
      case KindOfNull:
        d = 0.0;
        continue;

      case KindOfBoolean:
        assert(tv->m_data.num == 0LL || tv->m_data.num == 1LL);
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

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.dbl = d;
  tv->m_type = KindOfDouble;
}

void tvCastToInt64InPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  assert(cellIsPlausible(*tv));
  int64_t i;

  do {
    switch (tv->m_type) {
      case KindOfUninit:
      case KindOfNull:
        tv->m_data.num = 0LL;
        // fallthru
      case KindOfBoolean:
        assert(tv->m_data.num == 0LL || tv->m_data.num == 1LL);
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

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.num = i;
  tv->m_type = KindOfInt64;
}

double tvCastToDouble(TypedValue tv) {
  assert(tvIsPlausible(tv));
  if (tv.m_type == KindOfRef) {
    tv = *tv.m_data.pref->tv();
  }

  switch (tv.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return 0;

    case KindOfBoolean:
      assert(tv.m_data.num == 0LL || tv.m_data.num == 1LL);
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

    case KindOfRef:
      break;
  }
  not_reached();
}

const StaticString
  s_1("1"),
  s_scalar("scalar");

void tvCastToStringInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  auto string = [&](StringData* s) {
    tv->m_type = KindOfString;
    tv->m_data.pstr = s;
  };
  auto persistentString = [&](StringData* s) {
    assert(!s->isRefCounted());
    tv->m_type = KindOfPersistentString;
    tv->m_data.pstr = s;
  };

  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return persistentString(staticEmptyString());

    case KindOfBoolean:
      return persistentString(tv->m_data.num ? s_1.get() : staticEmptyString());

    case KindOfInt64:
      return string(buildStringData(tv->m_data.num));

    case KindOfDouble:
      return string(buildStringData(tv->m_data.dbl));

    case KindOfPersistentString:
    case KindOfString:
      return;

    case KindOfVec:
    case KindOfPersistentVec:
      raise_notice("Vec to string conversion");
      if (tv->m_type == KindOfVec) tvDecRefArr(tv);
      return persistentString(vec_string.get());

    case KindOfDict:
    case KindOfPersistentDict:
      raise_notice("Dict to string conversion");
      if (tv->m_type == KindOfDict) tvDecRefArr(tv);
      return persistentString(dict_string.get());

    case KindOfKeyset:
    case KindOfPersistentKeyset:
      raise_notice("Keyset to string conversion");
      if (tv->m_type == KindOfKeyset) tvDecRefArr(tv);
      return persistentString(keyset_string.get());

    case KindOfArray:
    case KindOfPersistentArray:
      raise_notice("Array to string conversion");
      if (tv->m_type == KindOfArray) tvDecRefArr(tv);
      return persistentString(array_string.get());

    case KindOfObject:
      // For objects, we fall back on the Variant machinery
      tvAsVariant(tv) = tv->m_data.pobj->invokeToString();
      return;

    case KindOfResource:
      // For resources, we fall back on the Variant machinery
      tvAsVariant(tv) = tv->m_data.pres->data()->o_toString();
      return;

    case KindOfRef:
      break;
  }
  not_reached();
}

StringData* tvCastToString(TypedValue tv) {
  assert(tvIsPlausible(tv));
  if (tv.m_type == KindOfRef) {
    tv = *tv.m_data.pref->tv();
  }

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

    case KindOfRef:
      not_reached();
  }
  not_reached();
}

ArrayData* tvCastToArrayLike(TypedValue tv) {
  assert(tvIsPlausible(tv));
  if (tv.m_type == KindOfRef) {
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

void tvCastToArrayInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
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
        assert(adIn->isVecArray());
        a = PackedArray::ToPHPArrayVec(adIn, true);
        assert(a != adIn);
        continue;
      }

      case KindOfVec: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isVecArray());
        a = PackedArray::ToPHPArrayVec(adIn, adIn->cowCheck());
        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentDict: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isDict());
        a = MixedArray::ToPHPArrayDict(adIn, true);
        assert(a != adIn);
        continue;
      }

      case KindOfDict: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isDict());
        a = MixedArray::ToPHPArrayDict(adIn, adIn->cowCheck());
        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentKeyset: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isKeyset());
        a = SetArray::ToPHPArray(adIn, true);
        assert(a != adIn);
        continue;
      }

      case KindOfKeyset: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isKeyset());
        a = SetArray::ToPHPArray(adIn, adIn->cowCheck());
        if (a != adIn) tvDecRefArr(tv);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray:
        assert(tv->m_data.parr->isPHPArray());
        return;

      case KindOfObject:
        // For objects, we fall back on the Variant machinery
        tvAsVariant(tv) = tv->m_data.pobj->toArray();
        return;

      case KindOfResource:
        a = ArrayData::Create(tvAsVariant(tv));
        tvDecRefRes(tv);
        continue;

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.parr = a;
  tv->m_type = KindOfArray;
  assert(cellIsPlausible(*tv));
}

static Array arrayFromCollection(ObjectData* obj) {
  if (auto ad = collections::asArray(obj)) {
    return ArrNR{ad}.asArray();
  }
  return collections::toArray(obj);
}

void tvCastToVecInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
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
        assert(adIn->isDict());
        a = MixedArray::ToVecDict(adIn, adIn->cowCheck());
        assert(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isKeyset());
        a = SetArray::ToVec(adIn, adIn->cowCheck());
        assert(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isPHPArray());
        a = adIn->toVec(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentVec:
      case KindOfVec:
        assert(tv->m_data.parr->isVecArray());
        return;

      case KindOfObject: {
        auto* obj = tv->m_data.pobj;
        if (obj->isCollection()) {
          a = arrayFromCollection(obj).toVec().detach();
        } else if (obj->instanceof(SystemLib::s_IteratorClass)) {
          auto arr = Array::CreateVec();
          for (ArrayIter iter(obj); iter; ++iter) {
            arr.append(iter.second());
          }
          a = arr.detach();
        } else {
          SystemLib::throwInvalidOperationExceptionObject(
            "Non-iterable object to vec conversion"
          );
        }
        decRefObj(obj);
        continue;
      }

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.parr = a;
  tv->m_type = KindOfVec;
  assert(cellIsPlausible(*tv));
}

void tvCastToDictInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
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
        assert(adIn->isVecArray());
        a = PackedArray::ToDictVec(adIn, adIn->cowCheck());
        assert(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isKeyset());
        a = SetArray::ToDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isPHPArray());
        a = adIn->toDict(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict:
        assert(tv->m_data.parr->isDict());
        return;

      case KindOfObject: {
        auto* obj = tv->m_data.pobj;
        if (obj->isCollection()) {
          a = arrayFromCollection(obj).toDict().detach();
        } else if (obj->instanceof(SystemLib::s_IteratorClass)) {
          auto arr = Array::CreateDict();
          for (ArrayIter iter(obj); iter; ++iter) {
            arr.set(iter.first(), iter.second());
          }
          a = arr.detach();
        } else {
          SystemLib::throwInvalidOperationExceptionObject(
            "Non-iterable object to dict conversion"
          );
        }
        decRefObj(obj);
        continue;
      }

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.parr = a;
  tv->m_type = KindOfDict;
  assert(cellIsPlausible(*tv));
}

void tvCastToKeysetInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
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
        assert(adIn->isVecArray());
        a = PackedArray::ToKeysetVec(adIn, adIn->cowCheck());
        assert(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isDict());
        a = MixedArray::ToKeysetDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isPHPArray());
        a = adIn->toKeyset(adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset:
        assert(tv->m_data.parr->isKeyset());
        return;

      case KindOfObject: {
        auto* obj = tv->m_data.pobj;
        if (obj->isCollection()) {
          a = arrayFromCollection(obj).toKeyset().detach();
        } else if (obj->instanceof(SystemLib::s_IteratorClass)) {
          auto arr = Array::CreateKeyset();
          for (ArrayIter iter(obj); iter; ++iter) {
            arr.append(iter.second());
          }
          a = arr.detach();
        } else {
          SystemLib::throwInvalidOperationExceptionObject(
            "Non-iterable object to keyset conversion"
          );
        }
        decRefObj(obj);
        continue;
      }

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.parr = a;
  tv->m_type = KindOfKeyset;
  assert(cellIsPlausible(*tv));
}

void tvCastToVArrayInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
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
        assert(adIn->isVecArray());
        a = PackedArray::ToVArrayVec(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isDict());
        a = MixedArray::ToVArrayDict(adIn, adIn->cowCheck());
        assert(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isKeyset());
        a = SetArray::ToVArray(adIn, adIn->cowCheck());
        assert(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isPHPArray());
        if (adIn->isVArray()) return;
        a = adIn->toVArray(adIn->cowCheck());
        if (a == adIn) return;
        decRefArr(adIn);
        continue;
      }

      case KindOfObject: {
        auto* obj = tv->m_data.pobj;
        if (obj->isCollection()) {
          a = arrayFromCollection(obj).toVArray().detach();
        } else if (obj->instanceof(SystemLib::s_IteratorClass)) {
          // This assumes that appending to an initially empty array will never
          // promote to mixed.
          auto arr = Array::Create();
          for (ArrayIter iter(obj); iter; ++iter) {
            arr.append(iter.second());
          }
          a = arr.detach();
        } else {
          SystemLib::throwInvalidOperationExceptionObject(
            "Non-iterable object to varray conversion"
          );
        }
        decRefObj(obj);
        continue;
      }

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  assert(a->isVArray());

  tv->m_data.parr = a;
  tv->m_type = KindOfArray;
  assert(cellIsPlausible(*tv));
}

void tvCastToDArrayInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
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
        assert(adIn->isVecArray());
        a = PackedArray::ToPHPArrayVec(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentDict:
      case KindOfDict: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isDict());
        a = MixedArray::ToPHPArrayDict(adIn, adIn->cowCheck());
        if (a != adIn) decRefArr(adIn);
        continue;
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset: {
        auto* adIn = tv->m_data.parr;
        assert(adIn->isKeyset());
        a = SetArray::ToPHPArray(adIn, adIn->cowCheck());
        assert(a != adIn);
        decRefArr(adIn);
        continue;
      }

      case KindOfPersistentArray:
      case KindOfArray:
        assert(tv->m_data.parr->isPHPArray());
        return;

      case KindOfObject: {
        auto* obj = tv->m_data.pobj;
        if (obj->isCollection()) {
          a = arrayFromCollection(obj).toPHPArray().detach();
        } else if (obj->instanceof(SystemLib::s_IteratorClass)) {
          auto arr = Array::Create();
          for (ArrayIter iter(obj); iter; ++iter) {
            arr.set(iter.first(), iter.second());
          }
          a = arr.detach();
        } else {
          SystemLib::throwInvalidOperationExceptionObject(
            "Non-iterable object to darray conversion"
          );
        }
        decRefObj(obj);
        continue;
      }

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.parr = a;
  tv->m_type = KindOfArray;
  assert(cellIsPlausible(*tv));
}

ObjectData* tvCastToObject(TypedValue tv) {
  assert(tvIsPlausible(tv));
  if (tv.m_type == KindOfRef) {
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
    case KindOfResource: {
      auto o = SystemLib::AllocStdClassObject();
      o->o_set(s_scalar, VarNR(tv));
      return o.detach();
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

void tvCastToObjectInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
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
      case KindOfResource:
        o = SystemLib::AllocStdClassObject().detach();
        o->o_set(s_scalar, tvAsVariant(tv));
        continue;

      case KindOfString:
        o = SystemLib::AllocStdClassObject().detach();
        o->o_set(s_scalar, tvAsVariant(tv));
        tvDecRefStr(tv);
        continue;

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
  assert(cellIsPlausible(*tv));
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
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

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
  assert(cellIsPlausible(*tv));
}

bool tvCoerceParamToBooleanInPlace(TypedValue* tv,
                                   bool builtin) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

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
    case KindOfString: {
      // Simplified version of is_numeric_string
      // which also allows for non-numeric garbage
      // Because PHP
      auto str = tv->m_data.pstr;
      auto p = str->data();
      auto l = tv->m_data.pstr->size();
      while (l && isspace(*p)) { ++p; --l; }
      if (l && (*p == '+' || *p == '-')) { ++p; --l; }
      if (l && *p == '.') { ++p; --l; }
      bool okay = l && isdigit(*p);
      // In PHP7 garbage at the end of a numeric string will trigger a notice
      if (RuntimeOption::PHP7_ScalarTypes && okay && !str->isNumeric()) {
        raise_notice("A non well formed numeric value encountered");
      }
      return okay;
    }

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

bool tvCoerceParamToInt64InPlace(TypedValue* tv,
                                 bool builtin) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
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
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  if (!tvCanBeCoercedToNumber(tv, builtin)) {
    return false;
  }
  tvCastToDoubleInPlace(tv);
  return true;
}

bool tvCoerceParamToStringInPlace(TypedValue* tv,
                                  bool builtin) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

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
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

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
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

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
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

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
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

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
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  return tv->m_type == KindOfObject;
}

bool tvCoerceParamToNullableObjectInPlace(TypedValue* tv, bool /*builtin*/) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  if (isNullType(tv->m_type)) {
    // See comment in tvCastToNullableObjectInPlace
    tv->m_data.pobj = nullptr;
    return true;
  }
  return tv->m_type == KindOfObject;
}

bool tvCoerceParamToResourceInPlace(TypedValue* tv, bool /*builtin*/) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  return tv->m_type == KindOfResource;
}

///////////////////////////////////////////////////////////////////////////////

}
