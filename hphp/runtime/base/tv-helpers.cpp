/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
        assert(!cell.m_data.pstr->isRefCounted());
        return;
      case KindOfString:
        assertPtr(cell.m_data.pstr);
        assert(cell.m_data.pstr->checkCount());
        return;
      case KindOfPersistentArray:
        assertPtr(cell.m_data.parr);
        assert(!cell.m_data.parr->isRefCounted());
        return;
      case KindOfArray:
        assertPtr(cell.m_data.parr);
        assert(cell.m_data.parr->checkCount());
        return;
      case KindOfObject:
        assertPtr(cell.m_data.pobj);
        assert(cell.m_data.pobj->checkCount());
        return;
      case KindOfResource:
        assertPtr(cell.m_data.pres);
        assert(cell.m_data.pres->checkCount());
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
    assert(tv.m_data.pref->checkCount());
    tv = *tv.m_data.pref->tv();
  }
  return cellIsPlausible(tv);
}

bool refIsPlausible(const Ref ref) {
  assert(ref.m_type == KindOfRef);
  return tvIsPlausible(ref);
}

bool tvDecRefWillRelease(TypedValue* tv) {
  if (!isRefcountedType(tv->m_type)) {
    return false;
  }
  if (tv->m_type == KindOfRef) {
    return tv->m_data.pref->getRealCount() <= 1;
  }
  return TV_GENERIC_DISPATCH(*tv, decWillRelease);
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

      case KindOfPersistentArray:
        b = !tv->m_data.parr->empty();
        continue;

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

      case KindOfPersistentString:
        d = tv->m_data.pstr->toDouble();
        continue;

      case KindOfString:
        d = tv->m_data.pstr->toDouble();
        tvDecRefStr(tv);
        continue;

      case KindOfPersistentArray:
        d = tv->m_data.parr->empty() ? 0 : 1;
        continue;

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

      case KindOfPersistentString:
        i = cell->m_data.pstr->toInt64();
        continue;

      case KindOfString:
        i = cell->m_data.pstr->toInt64();
        tvDecRefStr(cell);
        continue;

      case KindOfPersistentArray:
        i = cell->m_data.parr->empty() ? 0 : 1;
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
        i = cell->m_data.pres->data()->o_toInt64();
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

    case KindOfPersistentString:
    case KindOfString:
      return tv->m_data.pstr->toDouble();

    case KindOfPersistentArray:
    case KindOfArray:
      return tv->m_data.parr->empty() ? 0.0 : 1.0;

    case KindOfObject:
      return tv->m_data.pobj->toDouble();

    case KindOfResource:
      return tv->m_data.pres->data()->o_toDouble();

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
    case KindOfClass:
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

    case KindOfPersistentArray:
    case KindOfArray:
      raise_notice("Array to string conversion");
      return array_string.get();

    case KindOfObject:
      return tv->m_data.pobj->invokeToString().detach();

    case KindOfResource:
      return tv->m_data.pres->data()->o_toString().detach();

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
      case KindOfPersistentString:
        a = ArrayData::Create(tvAsVariant(tv));
        continue;

      case KindOfString:
        a = ArrayData::Create(tvAsVariant(tv));
        tvDecRefStr(tv);
        continue;

      case KindOfPersistentArray:
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

  assert(!a->isRefCounted() || a->hasExactlyOneRef());

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

      case KindOfPersistentArray:
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
  tv->m_data.pres = req::make<DummyResource>().detach()->hdr();
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

    case KindOfPersistentArray:
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

    case KindOfPersistentArray:
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
  // In PHP 7 mode doubles only convert to integers when the conversion is non-
  // narrowing
  if (RuntimeOption::PHP7_ScalarTypes && tv->m_type == KindOfDouble) {
    if (tv->m_data.dbl < std::numeric_limits<int64_t>::min()) return false;
    if (tv->m_data.dbl > std::numeric_limits<int64_t>::max()) return false;
    if (isnan(tv->m_data.dbl)) return false;
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
    case KindOfClass:
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
