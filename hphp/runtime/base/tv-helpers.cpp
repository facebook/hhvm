/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/runtime-error.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

bool cellIsPlausible(const Cell cell) {
  assert(cell.m_type != KindOfRef);

  auto assertPtr = [](void* ptr) {
    assert(ptr && (uintptr_t(ptr) % sizeof(ptr) == 0));
  };

  switch (cell.m_type) {
  case KindOfUninit:
  case KindOfNull:
    break;
  case KindOfBoolean:
    assert(cell.m_data.num == 0 || cell.m_data.num == 1);
    break;
  case KindOfInt64:
  case KindOfDouble:
    break;
  case KindOfStaticString:
    assertPtr(cell.m_data.pstr);
    assert(cell.m_data.pstr->isStatic());
    break;
  case KindOfString:
    assertPtr(cell.m_data.pstr);
    assert_refcount_realistic(cell.m_data.pstr->getCount());
    break;
  case KindOfArray:
    assertPtr(cell.m_data.parr);
    assert_refcount_realistic(cell.m_data.parr->getCount());
    break;
  case KindOfObject:
    assertPtr(cell.m_data.pobj);
    assert(!cell.m_data.pobj->isStatic());
    break;
  case KindOfResource:
    assertPtr(cell.m_data.pres);
    assert(!cell.m_data.pres->isStatic());
    break;
  case KindOfRef:
    assert(!"KindOfRef found in a Cell");
    break;
  default:
    not_reached();
  }
  return true;
}

bool tvIsPlausible(TypedValue tv) {
  if (tv.m_type == KindOfRef) {
    assert(tv.m_data.pref);
    assert(uintptr_t(tv.m_data.pref) % sizeof(void*) == 0);
    assert_refcount_realistic(tv.m_data.pref->getRealCount());
    tv = *tv.m_data.pref->tv();
  }
  return cellIsPlausible(tv);
}

bool refIsPlausible(const Ref ref) {
  assert(ref.m_type == KindOfRef);
  return tvIsPlausible(ref);
}

bool tvDecRefWillRelease(TypedValue* tv) {
  if (!IS_REFCOUNTED_TYPE(tv->m_type)) {
    return false;
  }
  if (tv->m_type == KindOfRef) {
    return tv->m_data.pref->getRealCount() <= 1;
  }
  return !tv->m_data.pstr->hasMultipleRefs();
}

void tvCastToBooleanInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  bool b;
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:    b = false; break;
  case KindOfBoolean: return;
  case KindOfInt64:   b = (tv->m_data.num != 0LL); break;
  case KindOfDouble:  b = (tv->m_data.dbl != 0); break;
  case KindOfStaticString: b = tv->m_data.pstr->toBoolean(); break;
  case KindOfString:  b = tv->m_data.pstr->toBoolean(); tvDecRefStr(tv); break;
  case KindOfArray:   b = (!tv->m_data.parr->empty()); tvDecRefArr(tv); break;
  case KindOfObject:  b = tv->m_data.pobj->o_toBoolean();
                      tvDecRefObj(tv);
                      break;
  case KindOfResource: b = tv->m_data.pres->o_toBoolean();
                       tvDecRefRes(tv);
                       break;
  default:            assert(false); b = false; break;
  }
  tv->m_data.num = b;
  tv->m_type = KindOfBoolean;
}

void tvCastToDoubleInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  double d;
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:    d = 0.0; break;
  case KindOfBoolean: assert(tv->m_data.num == 0LL || tv->m_data.num == 1LL);
  case KindOfInt64:   d = (double)(tv->m_data.num); break;
  case KindOfDouble:  return;
  case KindOfStaticString: d = tv->m_data.pstr->toDouble(); break;
  case KindOfString:  d = tv->m_data.pstr->toDouble(); tvDecRefStr(tv); break;
  case KindOfArray:   {
    d = (double)(tv->m_data.parr->empty() ? 0LL : 1LL);
    tvDecRefArr(tv);
    break;
  }
  case KindOfObject:  {
    d = tv->m_data.pobj->o_toDouble();
    tvDecRefObj(tv);
    break;
  }
  case KindOfResource:  {
    d = tv->m_data.pres->o_toDouble();
    tvDecRefRes(tv);
    break;
  }
  default:            assert(false); d = 0.0; break;
  }
  tv->m_data.dbl = d;
  tv->m_type = KindOfDouble;
}

void cellCastToInt64InPlace(Cell* cell) {
  assert(cellIsPlausible(*cell));

  int64_t i;
  switch (cell->m_type) {
  case KindOfUninit:
  case KindOfNull:
    cell->m_data.num = 0LL;
    // Fall through
  case KindOfBoolean:
    assert(cell->m_data.num == 0LL || cell->m_data.num == 1LL);
    cell->m_type = KindOfInt64;
    // Fall through
  case KindOfInt64:
    return;
  case KindOfDouble:  {
    i = toInt64(cell->m_data.dbl);
    break;
  }
  case KindOfStaticString: i = (cell->m_data.pstr->toInt64()); break;
  case KindOfString:  {
    i = cell->m_data.pstr->toInt64();
    tvDecRefStr(cell);
    break;
  }
  case KindOfArray:
    i = cell->m_data.parr->empty() ? 0 : 1;
    tvDecRefArr(cell);
    break;
  case KindOfObject:
    i = cell->m_data.pobj->o_toInt64();
    tvDecRefObj(cell);
    break;
  case KindOfResource:
    i = cell->m_data.pres->o_toInt64();
    tvDecRefRes(cell);
    break;
  default:
    not_reached();
  }
  cell->m_data.num = i;
  cell->m_type = KindOfInt64;
}

void tvCastToInt64InPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  cellCastToInt64InPlace(tv);
}

double tvCastToDouble(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  if (tv->m_type == KindOfRef) {
    tv = tv->m_data.pref->tv();
  }

  switch(tv->m_type) {
  case KindOfUninit:
  case KindOfNull:
    return 0;
  case KindOfBoolean:
    assert(tv->m_data.num == 0LL || tv->m_data.num == 1LL);
    // Fall through
  case KindOfInt64:
    return (double)(tv->m_data.num);
  case KindOfDouble:
    return tv->m_data.dbl;
  case KindOfStaticString:
  case KindOfString:
    return tv->m_data.pstr->toDouble();
  case KindOfArray:
    return tv->m_data.parr->empty() ? 0.0 : 1.0;
  case KindOfObject:
    return tv->m_data.pobj->o_toDouble();
  case KindOfResource:
    return tv->m_data.pres->o_toDouble();
  default:
    not_reached();
  }
}

const StaticString
  s_1("1"),
  s_Array("Array"),
  s_scalar("scalar");

void tvCastToStringInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  StringData * s;
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:    s = empty_string.get(); goto static_string;
  case KindOfBoolean:
    s = tv->m_data.num ? s_1.get() : empty_string.get();
    goto static_string;
  case KindOfInt64:   s = buildStringData(tv->m_data.num); break;
  case KindOfDouble:  s = buildStringData(tv->m_data.dbl); break;
  case KindOfStaticString:
  case KindOfString:  return;
  case KindOfArray:
    raise_notice("Array to string conversion");
    s = s_Array.get();
    tvDecRefArr(tv);
    goto static_string;
  case KindOfObject:
    // For objects, we fall back on the Variant machinery
    tvAsVariant(tv) = tv->m_data.pobj->invokeToString();
    return;
  case KindOfResource:
    // For resources, we fall back on the Variant machinery
    tvAsVariant(tv) = tv->m_data.pres->o_toString();
    return;
  default:
    not_reached();
  }

  s->incRefCount();
  tv->m_data.pstr = s;
  tv->m_type = KindOfString;
  return;
static_string:
  tv->m_data.pstr = s;
  tv->m_type = KindOfStaticString;
}

StringData* tvCastToString(const TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  if (tv->m_type == KindOfRef) {
    tv = tv->m_data.pref->tv();
  }

  StringData* s;
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:    return empty_string.get();
  case KindOfBoolean: return tv->m_data.num ? s_1.get() : empty_string.get();
  case KindOfInt64:   s = buildStringData(tv->m_data.num); break;
  case KindOfDouble:  s = buildStringData(tv->m_data.dbl); break;
  case KindOfStaticString: return tv->m_data.pstr;
  case KindOfString:  s = tv->m_data.pstr; break;
  case KindOfArray:   raise_notice("Array to string conversion");
                      return s_Array.get();
  case KindOfObject:  return tv->m_data.pobj->invokeToString().detach();
  case KindOfResource: return tv->m_data.pres->o_toString().detach();
  default:            not_reached();
  }

  s->incRefCount();
  return s;
}

void tvCastToArrayInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  ArrayData * a;
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:    a = ArrayData::Create(); break;
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfStaticString: a = ArrayData::Create(tvAsVariant(tv)); break;
  case KindOfString:  {
    a = ArrayData::Create(tvAsVariant(tv));
    tvDecRefStr(tv);
    break;
  }
  case KindOfArray:   return;
  case KindOfObject:  {
    // For objects, we fall back on the Variant machinery
    tvAsVariant(tv) = tv->m_data.pobj->o_toArray();
    return;
  }
  case KindOfResource:  {
    a = ArrayData::Create(tvAsVariant(tv));
    tvDecRefRes(tv);
    break;
  }
  default:            assert(false); a = ArrayData::Create(); break;
  }
  tv->m_data.parr = a;
  tv->m_type = KindOfArray;
  tv->m_data.parr->incRefCount();
}

void tvCastToObjectInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  ObjectData* o;
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:    o = SystemLib::AllocStdClassObject(); break;
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfStaticString: {
    o = SystemLib::AllocStdClassObject();
    o->o_set(s_scalar, tvAsVariant(tv));
    break;
  }
  case KindOfString: {
    o = SystemLib::AllocStdClassObject();
    o->o_set(s_scalar, tvAsVariant(tv));
    tvDecRefStr(tv);
    break;
  }
  case KindOfArray:   {
    // For arrays, we fall back on the Variant machinery
    tvAsVariant(tv) = tv->m_data.parr->toObject();
    return;
  }
  case KindOfObject: return;
  case KindOfResource: return;
  default: assert(false); o = SystemLib::AllocStdClassObject(); break;
  }
  tv->m_data.pobj = o;
  tv->m_type = KindOfObject;
  tv->m_data.pobj->incRefCount();
}

void tvCastToResourceInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfStaticString:
    break;
  case KindOfString:
  case KindOfArray:
  case KindOfObject:
    tvDecRef(tv);
    break;
  case KindOfResource:
    // no op, return
    return;
  default:
    assert(false);
    break;
  }
  tv->m_type = KindOfResource;
  tv->m_data.pres = NEWOBJ(DummyResource);
  tv->m_data.pres->incRefCount();
  return;
}

bool tvCoerceParamToBooleanInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  if (tv->m_type == KindOfArray || tv->m_type == KindOfObject ||
      tv->m_type == KindOfResource) {
    return false;
  }
  tvCastToBooleanInPlace(tv);
  return true;
}

bool tvCanBeCoercedToNumber(TypedValue* tv) {
  switch (tv->m_type) {
  case KindOfStaticString:
  case KindOfString:
    StringData* s;
    DataType type;
    s = tv->m_data.pstr;
    type = is_numeric_string(s->data(), s->size(), nullptr, nullptr);
    if (type != KindOfDouble && type != KindOfInt64) {
      return false;
    }
    break;
  case KindOfArray:
  case KindOfObject:
  case KindOfResource:
    return false;
  default:
    break;
  }
  return true;
}

bool tvCoerceParamToInt64InPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  if (!tvCanBeCoercedToNumber(tv)) {
    return false;
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
  default:
    break;
  }
  tvCastToStringInPlace(tv);
  return true;
}

bool tvCoerceParamToArrayInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  if (tv->m_type == KindOfArray) {
    return true;
  } else if (tv->m_type == KindOfObject) {
    tvAsVariant(tv) = tv->m_data.pobj->o_toArray();
    return true;
  } else if (tv->m_type == KindOfResource) {
    tvAsVariant(tv) = tv->m_data.pres->o_toArray();
    return true;
  }
  return false;
}

bool tvCoerceParamToObjectInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  return tv->m_type == KindOfObject;
}

bool tvCoerceParamToResourceInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);
  return tv->m_type == KindOfResource;
}

///////////////////////////////////////////////////////////////////////////////
}
