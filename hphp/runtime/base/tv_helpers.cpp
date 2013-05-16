/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/type_conversions.h"

#include "hphp/system/lib/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void tvCastToBooleanInPlace(TypedValue* tv) {
  if (tv->m_type == KindOfRef) {
    tvUnbox(tv);
  }
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
  case KindOfObject:  b = (tv->m_data.pobj != nullptr); tvDecRefObj(tv); break;
  default:            assert(false); b = false; break;
  }
  tv->m_data.num = b;
  tv->m_type = KindOfBoolean;
}

void tvCastToInt64InPlace(TypedValue* tv, int base /* = 10 */) {
  if (tv->m_type == KindOfRef) {
    tvUnbox(tv);
  }
  int64_t i;
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:
    tv->m_data.num = 0LL;
    // Fall through
  case KindOfBoolean:
    assert(tv->m_data.num == 0LL || tv->m_data.num == 1LL);
    tv->m_type = KindOfInt64;
    // Fall through
  case KindOfInt64:
    return;
  case KindOfDouble:  {
    i = toInt64(tv->m_data.dbl);
    break;
  }
  case KindOfStaticString: i = (tv->m_data.pstr->toInt64(base)); break;
  case KindOfString:  {
    i = (tv->m_data.pstr->toInt64(base));
    tvDecRefStr(tv);
    break;
  }
  case KindOfArray:   {
    i = (tv->m_data.parr->empty() ? 0LL : 1LL);
    tvDecRefArr(tv);
    break;
  }
  case KindOfObject:  {
    i = (tv->m_data.pobj ? tv->m_data.pobj->o_toInt64() : 0LL);
    tvDecRefObj(tv);
    break;
  }
  default:            assert(false); i = 0LL; break;
  }
  tv->m_data.num = i;
  tv->m_type = KindOfInt64;
}

int64_t tvCastToInt64(TypedValue* tv, int base /* = 10 */) {
  if (tv->m_type == KindOfRef) {
    tv = tv->m_data.pref->tv();
  }

  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:
    return 0;
  case KindOfBoolean:
    assert(tv->m_data.num == 0LL || tv->m_data.num == 1LL);
    // Fall through
  case KindOfInt64:
    return tv->m_data.num;
  case KindOfDouble:
    return toInt64(tv->m_data.dbl);
  case KindOfStaticString:
  case KindOfString:
    return tv->m_data.pstr->toInt64(base);
  case KindOfArray:
    return tv->m_data.parr->empty() ? 0 : 1;
  case KindOfObject:
    return tv->m_data.pobj->o_toInt64();
  default:
    not_reached();
  }
}

void tvCastToDoubleInPlace(TypedValue* tv) {
  if (tv->m_type == KindOfRef) {
    tvUnbox(tv);
  }
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
    d = (double)(tv->m_data.pobj ? tv->m_data.pobj->o_toDouble() : 0LL);
    tvDecRefObj(tv);
    break;
  }
  default:            assert(false); d = 0.0; break;
  }
  tv->m_data.dbl = d;
  tv->m_type = KindOfDouble;
}

void tvCastToStringInPlace(TypedValue* tv) {
  if (tv->m_type == KindOfRef) {
    tvUnbox(tv);
  }
  StringData * s;
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:    s = buildStringData(""); break;
  case KindOfBoolean: s = buildStringData(tv->m_data.num ? "1" : ""); break;
  case KindOfInt64:   s = buildStringData(tv->m_data.num); break;
  case KindOfDouble:  s = buildStringData(tv->m_data.dbl); break;
  case KindOfStaticString:
  case KindOfString:  return;
  case KindOfArray:   s = buildStringData("Array"); tvDecRefArr(tv); break;
  case KindOfObject:  {
    // For objects, we fall back on the Variant machinery
    tvAsVariant(tv) = tv->m_data.pobj->t___tostring();
    return;
  }
  default:            not_reached();
  }
  tv->m_data.pstr = s;
  tv->m_type = KindOfString;
  tv->m_data.pstr->incRefCount();
}

StringData* tvCastToString(TypedValue* tv) {
  if (tv->m_type == KindOfRef) {
    tv = tv->m_data.pref->tv();
  }

  StringData* s;
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:    s = buildStringData(""); break;
  case KindOfBoolean: s = buildStringData(tv->m_data.num ? "1" : ""); break;
  case KindOfInt64:   s = buildStringData(tv->m_data.num); break;
  case KindOfDouble:  s = buildStringData(tv->m_data.dbl); break;
  case KindOfStaticString:
  case KindOfString:  s = tv->m_data.pstr; break;
  case KindOfArray:   return StringData::GetStaticString("Array");
  case KindOfObject:  return tv->m_data.pobj->t___tostring().detach();
  default:            not_reached();
  }

  s->incRefCount();
  return s;
}

void tvCastToArrayInPlace(TypedValue* tv) {
  if (tv->m_type == KindOfRef) {
    tvUnbox(tv);
  }
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
  default:            assert(false); a = ArrayData::Create(); break;
  }
  tv->m_data.parr = a;
  tv->m_type = KindOfArray;
  tv->m_data.parr->incRefCount();
}

void tvCastToObjectInPlace(TypedValue* tv) {
  if (tv->m_type == KindOfRef) {
    tvUnbox(tv);
  }
  ObjectData* o;
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:    o = SystemLib::AllocStdClassObject(); break;
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfStaticString: {
    o = SystemLib::AllocStdClassObject();
    o->o_set("scalar", tvAsVariant(tv));
    break;
  }
  case KindOfString: {
    o = SystemLib::AllocStdClassObject();
    o->o_set("scalar", tvAsVariant(tv));
    tvDecRefStr(tv);
    break;
  }
  case KindOfArray:   {
    // For arrays, we fall back on the Variant machinery
    tvAsVariant(tv) = tv->m_data.parr->toObject();
    return;
  }
  case KindOfObject:  return;
  default:            assert(false); o = SystemLib::AllocStdClassObject(); break;
  }
  tv->m_data.pobj = o;
  tv->m_type = KindOfObject;
  tv->m_data.pobj->incRefCount();
}

bool tvIsPlausible(const TypedValue* tv) {
  if (!tv) return false;
  auto okPtr = [](void* ptr) {
    return ptr && (uintptr_t(ptr) % sizeof(ptr) == 0);
  };
  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return true;
    case KindOfBoolean:
      return tv->m_data.num == 0 || tv->m_data.num == 1;
    case KindOfInt64:
    case KindOfDouble:
      return true;
    case KindOfStaticString:
      return okPtr(tv->m_data.pstr) && tv->m_data.pstr->isStatic();
    case KindOfString:
      return okPtr(tv->m_data.pstr) &&
        is_refcount_realistic(tv->m_data.pstr->getCount());
    case KindOfArray:
      return okPtr(tv->m_data.parr) &&
             is_refcount_realistic(tv->m_data.parr->getCount());
    case KindOfObject:
      return okPtr(tv->m_data.pobj) && !tv->m_data.pobj->isStatic();
    case KindOfRef:
      return okPtr(tv->m_data.pref) &&
             tv->m_data.pref->tv()->m_type != KindOfRef;
    default:
      return false;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
