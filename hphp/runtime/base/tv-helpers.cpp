/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/vm/func.h"

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
      case KindOfStaticString:
        assertPtr(cell.m_data.pstr);
        assert(cell.m_data.pstr->isStatic());
        return;
      case KindOfString:
        assertPtr(cell.m_data.pstr);
        assert(check_refcount(cell.m_data.pstr->getCount()));
        return;
      case KindOfArray:
        assertPtr(cell.m_data.parr);
        assert(check_refcount(cell.m_data.parr->getCount()));
        return;
      case KindOfObject:
        assertPtr(cell.m_data.pobj);
        assert(!cell.m_data.pobj->isStatic());
        return;
      case KindOfResource:
        assertPtr(cell.m_data.pres);
        assert(!cell.m_data.pres->isStatic());
        return;
      case KindOfRef:
        assert(!"KindOfRef found in a Cell");
        break;
      case KindOfClass:
        assert(!"Invalid Cell type");
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
    assert(check_refcount(tv.m_data.pref->getRealCount()));
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

      case KindOfStaticString:
        b = tv->m_data.pstr->toBoolean();
        continue;

      case KindOfString:
        b = tv->m_data.pstr->toBoolean();
        tvDecRefStr(tv);
        continue;

      case KindOfArray:
        b = !!tv->m_data.parr->size();
        tvDecRefArr(tv);
        continue;

      case KindOfObject:
        b = tv->m_data.pobj->toBoolean();
        tvDecRefObj(tv);
        continue;

      case KindOfResource:
        b = tv->m_data.pres->o_toBoolean();
        tvDecRefRes(tv);
        continue;

      case KindOfRef:
      case KindOfClass:
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

      case KindOfStaticString:
        d = tv->m_data.pstr->toDouble();
        continue;

      case KindOfString:
        d = tv->m_data.pstr->toDouble(); tvDecRefStr(tv);
        continue;

      case KindOfArray:
        d = (double)(tv->m_data.parr->empty() ? 0LL : 1LL);
        tvDecRefArr(tv);
        continue;

      case KindOfObject:
        d = tv->m_data.pobj->toDouble();
        tvDecRefObj(tv);
        continue;

      case KindOfResource:
        d = tv->m_data.pres->o_toDouble();
        tvDecRefRes(tv);
        continue;

      case KindOfRef:
      case KindOfClass:
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

      case KindOfStaticString:
        i = (cell->m_data.pstr->toInt64());
        continue;

      case KindOfString:
        i = cell->m_data.pstr->toInt64();
        tvDecRefStr(cell);
        continue;

      case KindOfArray:
        i = cell->m_data.parr->empty() ? 0 : 1;
        tvDecRefArr(cell);
        continue;

      case KindOfObject:
        i = cell->m_data.pobj->toInt64();
        tvDecRefObj(cell);
        continue;

      case KindOfResource:
        i = cell->m_data.pres->o_toInt64();
        tvDecRefRes(cell);
        continue;

      case KindOfRef:
      case KindOfClass:
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

double tvCastToDouble(TypedValue* tv) {
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

    case KindOfStaticString:
    case KindOfString:
      return tv->m_data.pstr->toDouble();

    case KindOfArray:
      return tv->m_data.parr->empty() ? 0.0 : 1.0;

    case KindOfObject:
      return tv->m_data.pobj->toDouble();

    case KindOfResource:
      return tv->m_data.pres->o_toDouble();

    case KindOfRef:
    case KindOfClass:
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
  StringData* s;

  do {
    switch (tv->m_type) {
      case KindOfUninit:
      case KindOfNull:
        s = staticEmptyString();
        goto static_string;

      case KindOfBoolean:
        s = tv->m_data.num ? s_1.get() : staticEmptyString();
        goto static_string;

      case KindOfInt64:
        s = buildStringData(tv->m_data.num);
        continue;

      case KindOfDouble:
        s = buildStringData(tv->m_data.dbl);
        continue;

      case KindOfStaticString:
      case KindOfString:
        return;

      case KindOfArray:
        raise_notice("Array to string conversion");
        s = array_string.get();
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

      case KindOfRef:
      case KindOfClass:
        break;
    }
    not_reached();
  } while (0);

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

    case KindOfStaticString:
      return tv->m_data.pstr;

    case KindOfString: {
      auto s = tv->m_data.pstr;
      s->incRefCount();
      return s;
    }

    case KindOfArray:
      raise_notice("Array to string conversion");
      return array_string.get();

    case KindOfObject:
      return tv->m_data.pobj->invokeToString().detach();

    case KindOfResource:
      return tv->m_data.pres->o_toString().detach();

    case KindOfRef:
    case KindOfClass:
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
      case KindOfStaticString:
        a = ArrayData::Create(tvAsVariant(tv));
        continue;

      case KindOfString:
        a = ArrayData::Create(tvAsVariant(tv));
        tvDecRefStr(tv);
        continue;

      case KindOfArray:
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
      case KindOfClass:
        break;
    }
    not_reached();
  } while (0);

  assert(a->isStatic() || a->hasExactlyOneRef());

  tv->m_data.parr = a;
  tv->m_type = KindOfArray;
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
      case KindOfStaticString:
      case KindOfResource:
        o = SystemLib::AllocStdClassObject().detach();
        o->o_set(s_scalar, tvAsVariant(tv));
        continue;

      case KindOfString:
        o = SystemLib::AllocStdClassObject().detach();
        o->o_set(s_scalar, tvAsVariant(tv));
        tvDecRefStr(tv);
        continue;

      case KindOfArray:
        // For arrays, we fall back on the Variant machinery
        tvAsVariant(tv) = ObjectData::FromArray(tv->m_data.parr);
        return;

      case KindOfObject:
        return;

      case KindOfRef:
      case KindOfClass:
        break;
    }
    not_reached();
  } while (0);

  tv->m_data.pobj = o;
  tv->m_type = KindOfObject;
}

void tvCastToNullableObjectInPlace(TypedValue* tv) {
  if (IS_NULL_TYPE(tv->m_type)) {
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
      case KindOfArray:
      case KindOfObject:
        tvDecRef(tv);
        continue;
      case KindOfResource:
        // no op, return
        return;
      case KindOfRef:
      case KindOfClass:
        break;
    }
    not_reached();
  } while (0);

  tv->m_type = KindOfResource;
  tv->m_data.pres = newres<DummyResource>();
}

bool tvCoerceParamToBooleanInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  switch (tv->m_type) {
    DT_UNCOUNTED_CASE:
    case KindOfString:
      tvCastToBooleanInPlace(tv);
      return true;

    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
      return false;

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

bool tvCanBeCoercedToNumber(TypedValue* tv) {
  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
      return true;

    case KindOfStaticString:
    case KindOfString: {
      // Simplified version of is_numeric_string
      // which also allows for non-numeric garbage
      // Because PHP
      auto p = tv->m_data.pstr->data();
      auto l = tv->m_data.pstr->size();
      while (l && isspace(*p)) { ++p; --l; }
      if (l && (*p == '+' || *p == '-')) { ++p; --l; }
      if (l && *p == '.') { ++p; --l; }
      return l && isdigit(*p);
    }

    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
      return false;

    case KindOfRef:
    case KindOfClass:
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
    DT_UNCOUNTED_CASE:
    case KindOfString:
      tvCastToStringInPlace(tv);
      return true;

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
    case KindOfClass:
      break;
  }
  not_reached();
}

bool tvCoerceParamToArrayInPlace(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  tvUnboxIfNeeded(tv);

  switch (tv->m_type) {
    DT_UNCOUNTED_CASE:
    case KindOfString:
      return false;

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
    case KindOfClass:
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
  if (IS_NULL_TYPE(tv->m_type)) {
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

#define XX(kind, expkind)                                         \
void tvCoerceParamTo##kind##OrThrow(TypedValue* tv,               \
                                    const Func* callee,           \
                                    unsigned int arg_num) {       \
  if (LIKELY(tvCoerceParamTo##kind##InPlace(tv))) {               \
    return;                                                       \
  }                                                               \
  raise_param_type_warning(callee->name()->data(),                \
                           arg_num, KindOf##expkind, tv->m_type); \
  throw TVCoercionException(callee, arg_num, tv->m_type,          \
                            KindOf##expkind);                     \
}
#define X(kind) XX(kind, kind)
X(Boolean)
X(Int64)
X(Double)
X(String)
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
