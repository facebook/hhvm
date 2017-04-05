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

#include "hphp/runtime/base/tv-helpers.h"

#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/translator-inline.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

bool cellIsPlausible(const Cell cell) {
  assert(cell.m_type != KindOfRef);

  auto assertPtr = [](void* ptr) {
    assert(ptr && (uintptr_t(ptr) % sizeof(ptr) == 0));
  };

  [&] {
    switch (cell.m_type) {
      case KindOfUninit:
      case KindOfNull:
        return;
      case KindOfBoolean:
        assert(cell.m_data.num == 0 || cell.m_data.num == 1);
        return;
      case KindOfInt64:
      case KindOfDouble:
        return;
      case KindOfPersistentString:
        assertPtr(cell.m_data.pstr);
        assert(cell.m_data.pstr->kindIsValid());
        assert(!cell.m_data.pstr->isRefCounted());
        return;
      case KindOfString:
        assertPtr(cell.m_data.pstr);
        assert(cell.m_data.pstr->kindIsValid());
        assert(cell.m_data.pstr->checkCount());
        return;
      case KindOfPersistentVec:
        assertPtr(cell.m_data.parr);
        assert(!cell.m_data.parr->isRefCounted());
        assert(cell.m_data.parr->isVecArray());
        return;
      case KindOfVec:
        assertPtr(cell.m_data.parr);
        assert(cell.m_data.parr->checkCount());
        assert(cell.m_data.parr->isVecArray());
        return;
      case KindOfPersistentDict:
        assertPtr(cell.m_data.parr);
        assert(!cell.m_data.parr->isRefCounted());
        assert(cell.m_data.parr->isDict());
        return;
      case KindOfDict:
        assertPtr(cell.m_data.parr);
        assert(cell.m_data.parr->checkCount());
        assert(cell.m_data.parr->isDict());
        return;
      case KindOfPersistentKeyset:
        assertPtr(cell.m_data.parr);
        assert(!cell.m_data.parr->isRefCounted());
        assert(cell.m_data.parr->isKeyset());
        return;
      case KindOfKeyset:
        assertPtr(cell.m_data.parr);
        assert(cell.m_data.parr->checkCount());
        assert(cell.m_data.parr->isKeyset());
        return;
      case KindOfPersistentArray:
        assertPtr(cell.m_data.parr);
        assert(cell.m_data.parr->kindIsValid());
        assert(!cell.m_data.parr->isRefCounted());
        assert(cell.m_data.parr->isPHPArray());
        return;
      case KindOfArray:
        assertPtr(cell.m_data.parr);
        assert(cell.m_data.parr->kindIsValid());
        assert(cell.m_data.parr->checkCount());
        assert(cell.m_data.parr->isPHPArray());
        return;
      case KindOfObject:
        assertPtr(cell.m_data.pobj);
        assert(cell.m_data.pobj->kindIsValid());
        assert(cell.m_data.pobj->checkCount());
        return;
      case KindOfResource:
        assertPtr(cell.m_data.pres);
        assert(cell.m_data.pres->kindIsValid());
        assert(cell.m_data.pres->checkCount());
        return;
      case KindOfRef:
        assert(!"KindOfRef found in a Cell");
        break;
    }
    not_reached();
  }();

  return true;
}

bool tvIsPlausible(TypedValue tv) {
  if (tv.m_type == KindOfRef) {
    assert(tv.m_data.pref);
    assert(uintptr_t(tv.m_data.pref) % sizeof(void*) == 0);
    assert(tv.m_data.pref->kindIsValid());
    assert(tv.m_data.pref->checkCount());
    tv = *tv.m_data.pref->tv();
  }
  return cellIsPlausible(tv);
}

bool refIsPlausible(const Ref ref) {
  assert(ref.m_type == KindOfRef);
  return tvIsPlausible(ref);
}

bool tvDecRefWillRelease(const TypedValue* tv) {
  if (!isRefcountedType(tv->m_type)) {
    return false;
  }
  if (tv->m_type == KindOfRef) {
    return tv->m_data.pref->getRealCount() <= 1;
  }
  return tv->m_data.pcnt->decWillRelease();
}

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

void cellCastToInt64InPlace(Cell* cell) {
  assert(cellIsPlausible(*cell));
  int64_t i;

  do {
    switch (cell->m_type) {
      case KindOfUninit:
      case KindOfNull:
        cell->m_data.num = 0LL;
        // fallthru
      case KindOfBoolean:
        assert(cell->m_data.num == 0LL || cell->m_data.num == 1LL);
        cell->m_type = KindOfInt64;
        // fallthru
      case KindOfInt64:
        return;

      case KindOfDouble:
        i = toInt64(cell->m_data.dbl);
        continue;

      case KindOfPersistentString:
        i = cell->m_data.pstr->toInt64();
        continue;

      case KindOfString:
        i = cell->m_data.pstr->toInt64();
        tvDecRefStr(cell);
        continue;

      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentArray:
        i = cell->m_data.parr->empty() ? 0 : 1;
        continue;

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfArray:
        i = cell->m_data.parr->empty() ? 0 : 1;
        tvDecRefArr(cell);
        continue;

      case KindOfObject:
        i = cell->m_data.pobj->toInt64();
        tvDecRefObj(cell);
        continue;

      case KindOfResource:
        i = cell->m_data.pres->data()->o_toInt64();
        tvDecRefRes(cell);
        continue;

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  cell->m_data.num = i;
  cell->m_type = KindOfInt64;
}

void tvCastToInt64InPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  cellCastToInt64InPlace(tv);
}

double tvCastToDouble(const TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  if (tv->m_type == KindOfRef) {
    tv = tv->m_data.pref->tv();
  }

  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return 0;

    case KindOfBoolean:
      assert(tv->m_data.num == 0LL || tv->m_data.num == 1LL);
      // fallthru
    case KindOfInt64:
      return (double)(tv->m_data.num);

    case KindOfDouble:
      return tv->m_data.dbl;

    case KindOfPersistentString:
    case KindOfString:
      return tv->m_data.pstr->toDouble();

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
      return tv->m_data.parr->empty() ? 0.0 : 1.0;

    case KindOfObject:
      return tv->m_data.pobj->toDouble();

    case KindOfResource:
      return tv->m_data.pres->data()->o_toDouble();

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

StringData* tvCastToString(const TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  if (tv->m_type == KindOfRef) {
    tv = tv->m_data.pref->tv();
  }

  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return staticEmptyString();

    case KindOfBoolean:
      return tv->m_data.num ? s_1.get() : staticEmptyString();

    case KindOfInt64:
      return buildStringData(tv->m_data.num);

    case KindOfDouble:
      return buildStringData(tv->m_data.dbl);

    case KindOfPersistentString:
      return tv->m_data.pstr;

    case KindOfString: {
      auto s = tv->m_data.pstr;
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
      return tv->m_data.pobj->invokeToString().detach();

    case KindOfResource:
      return tv->m_data.pres->data()->o_toString().detach();

    case KindOfRef:
      not_reached();
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

  assert(!a->isRefCounted() || a->hasExactlyOneRef());

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
        raise_warning("Null to vec conversion");
        a = staticEmptyVecArray();
        continue;

      case KindOfBoolean:
        raise_warning("Bool to vec conversion");
        a = staticEmptyVecArray();
        continue;

      case KindOfInt64:
        raise_warning("Int to vec conversion");
        a = staticEmptyVecArray();
        continue;

      case KindOfDouble:
        raise_warning("Double to vec conversion");
        a = staticEmptyVecArray();
        continue;

      case KindOfPersistentString:
      case KindOfString:
        raise_warning("String to vec conversion");
        a = staticEmptyVecArray();
        decRefStr(tv->m_data.pstr);
        continue;

      case KindOfResource:
        raise_warning("Resource to vec conversion");
        a = staticEmptyVecArray();
        decRefRes(tv->m_data.pres);
        continue;

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
          decRefObj(obj);
          tv->m_data.parr = a;
          tv->m_type = KindOfVec;
          assert(cellIsPlausible(*tv));
          return;
        } else if (obj->instanceof(SystemLib::s_IteratorClass)) {
          auto arr = Array::CreateVec();
          for (ArrayIter iter(obj); iter; ++iter) {
            arr.append(iter.second());
          }
          a = arr.detach();
        } else {
          raise_warning("Non-iterable object conversion to vec");
          a = staticEmptyVecArray();
        }
        decRefObj(obj);
        continue;
      }

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  assert(!a->isRefCounted() || a->hasExactlyOneRef());

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
        raise_warning("Null to dict conversion");
        a = staticEmptyDictArray();
        continue;

      case KindOfBoolean:
        raise_warning("Bool to dict conversion");
        a = staticEmptyDictArray();
        continue;

      case KindOfInt64:
        raise_warning("Int to dict conversion");
        a = staticEmptyDictArray();
        continue;

      case KindOfDouble:
        raise_warning("Double to dict conversion");
        a = staticEmptyDictArray();
        continue;

      case KindOfPersistentString:
      case KindOfString:
        raise_warning("String to dict conversion");
        a = staticEmptyDictArray();
        decRefStr(tv->m_data.pstr);
        continue;

      case KindOfResource:
        raise_warning("Resource to dict conversion");
        a = staticEmptyDictArray();
        decRefRes(tv->m_data.pres);
        continue;

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
          raise_warning("Non-iterable object conversion to dict");
          a = staticEmptyDictArray();
        }
        tv->m_data.parr = a;
        tv->m_type = KindOfDict;
        assert(cellIsPlausible(*tv));
        decRefObj(obj);
        return;
      }

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  assert(!a->isRefCounted() || a->hasExactlyOneRef());

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
        raise_warning("Null to keyset conversion");
        a = staticEmptyKeysetArray();
        continue;

      case KindOfBoolean:
        raise_warning("Bool to keyset conversion");
        a = staticEmptyKeysetArray();
        continue;

      case KindOfInt64:
        raise_warning("Int to keyset conversion");
        a = staticEmptyKeysetArray();
        continue;

      case KindOfDouble:
        raise_warning("Double to keyset conversion");
        a = staticEmptyKeysetArray();
        continue;

      case KindOfPersistentString:
      case KindOfString:
        raise_warning("String to keyset conversion");
        a = staticEmptyKeysetArray();
        decRefStr(tv->m_data.pstr);
        continue;

      case KindOfResource:
        raise_warning("Resource to keyset conversion");
        a = staticEmptyKeysetArray();
        decRefRes(tv->m_data.pres);
        continue;

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
            arr.append(iter.first());
          }
          a = arr.detach();
        } else {
          raise_warning("Non-iterable object conversion to keyset");
          a = staticEmptyKeysetArray();
        }
        decRefObj(obj);
        continue;
      }

      case KindOfRef:
        break;
    }
    not_reached();
  } while (0);

  assert(!a->isRefCounted() || a->hasExactlyOneRef());

  tv->m_data.parr = a;
  tv->m_type = KindOfKeyset;
  assert(cellIsPlausible(*tv));
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
        tvDecRef(tv);
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

bool tvCoerceParamToBooleanInPlace(TypedValue* tv) {
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
      // In PHP 7 mode handling of null types is stricter
      if (tv->m_type == KindOfNull && RuntimeOption::PHP7_ScalarTypes) {
        return false;
      }
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

bool tvCanBeCoercedToNumber(const TypedValue* tv) {
  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
      return true;

    case KindOfNull:
      // In PHP 7 mode handling of null types is stricter
      return !RuntimeOption::PHP7_ScalarTypes;

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

bool tvCoerceParamToInt64InPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  if (!tvCanBeCoercedToNumber(tv)) {
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

bool tvCoerceParamToDoubleInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  if (!tvCanBeCoercedToNumber(tv)) {
    return false;
  }
  tvCastToDoubleInPlace(tv);
  return true;
}

bool tvCoerceParamToStringInPlace(TypedValue* tv) {
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
      // In PHP 7 mode handling of null types is stricter
      if (tv->m_type == KindOfNull && RuntimeOption::PHP7_ScalarTypes) {
        return false;
      }
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

bool tvCoerceParamToArrayInPlace(TypedValue* tv) {
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

bool tvCoerceParamToVecInPlace(TypedValue* tv) {
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

bool tvCoerceParamToDictInPlace(TypedValue* tv) {
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

bool tvCoerceParamToKeysetInPlace(TypedValue* tv) {
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

bool tvCoerceParamToObjectInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  return tv->m_type == KindOfObject;
}

bool tvCoerceParamToNullableObjectInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  if (isNullType(tv->m_type)) {
    // See comment in tvCastToNullableObjectInPlace
    tv->m_data.pobj = nullptr;
    return true;
  }
  return tv->m_type == KindOfObject;
}

bool tvCoerceParamToResourceInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  return tv->m_type == KindOfResource;
}

namespace {
/*
 * Sometimes calls to builtin functions are inlined so that the call itself can
 * occur via CallBuiltin rather than NativeImpl. In these instances it's
 * possible that no ActRec was pushed for the builtin call, in which case the
 * liveFunc() will be the caller rather than the callee.
 *
 * If no ActRec was pushed for the builtin function inspect the caller to
 * determine if the call used strict types.
 */
bool useStrictTypes(const Func* callee) {
  return liveFunc() == callee
    ? !liveFrame()->useWeakTypes()
    : liveUnit()->useStrictTypes() && !liveUnit()->isHHFile();
}
}

void tvCoerceIfStrict(TypedValue& tv, int64_t argNum, const Func* func) {
  if (LIKELY(!RuntimeOption::PHP7_ScalarTypes ||
             RuntimeOption::EnableHipHopSyntax)) {
    return;
  }

  VMRegAnchor _;
  if (!useStrictTypes(func)) return;

  auto const& tc = func->params()[argNum - 1].typeConstraint;
  tc.verifyParam(&tv, func, argNum - 1, true);
}

#define XX(kind, expkind)                                         \
void tvCoerceParamTo##kind##OrThrow(TypedValue* tv,               \
                                    const Func* callee,           \
                                    unsigned int arg_num) {       \
  tvCoerceIfStrict(*tv, arg_num, callee);                         \
  if (LIKELY(tvCoerceParamTo##kind##InPlace(tv))) {               \
    return;                                                       \
  }                                                               \
  raise_param_type_warning(callee->displayName()->data(),         \
                           arg_num, KindOf##expkind, tv->m_type); \
  throw TVCoercionException(callee, arg_num, tv->m_type,          \
                            KindOf##expkind);                     \
}
#define X(kind) XX(kind, kind)
X(Boolean)
X(Int64)
X(Double)
X(String)
X(Vec)
X(Dict)
X(Keyset)
X(Array)
X(Object)
XX(NullableObject, Object)
X(Resource)
#undef X
#undef XX

TVCoercionException::TVCoercionException(const Func* func,
                                         int arg_num,
                                         DataType actual,
                                         DataType expected)
    : std::runtime_error(
        folly::format("Unable to coerce param {} to {}() "
                      "from {} to {}",
                      arg_num,
                      func->name(),
                      actual,
                      expected).str())
{
  if (func->attrs() & AttrParamCoerceModeFalse) {
    m_tv = make_tv<KindOfBoolean>(false);
  } else {
    m_tv = make_tv<KindOfNull>();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
