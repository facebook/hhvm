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

#include "hphp/runtime/base/type-variant.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/double-to-int64.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/array-init.h"

#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/system/systemlib.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/logger.h"

#include <limits>
#include <utility>
#include <vector>

namespace HPHP {

const VarNR null_varNR;
const VarNR true_varNR(true);
const VarNR false_varNR(false);
const VarNR INF_varNR(std::numeric_limits<double>::infinity());
const VarNR NEGINF_varNR(std::numeric_limits<double>::infinity());
const VarNR NAN_varNR(std::numeric_limits<double>::quiet_NaN());
const Variant empty_string_variant_ref(staticEmptyString(),
                                       Variant::PersistentStrInit{});

using BlackHoleStorage = std::aligned_storage<
  sizeof(req::root<Variant>),
  alignof(req::root<Variant>)
>::type;

static __thread BlackHoleStorage bhStorage;

///////////////////////////////////////////////////////////////////////////////
// static strings

const StaticString
  s_scalar("scalar");

///////////////////////////////////////////////////////////////////////////////

Variant::Variant(StringData *v) noexcept {
  if (v) {
    m_data.pstr = v;
    if (!v->isRefCounted()) {
      m_type = KindOfPersistentString;
    } else {
      m_type = KindOfString;
      v->rawIncRefCount();
    }
  } else {
    m_type = KindOfNull;
  }
}

namespace {
ALWAYS_INLINE
void implCopyConstruct(TypedValue tv, Variant& v) {
  cellDup(tvToInitCell(tv), v.asTypedValue());
}
}

// the version of the high frequency function that is not inlined
NEVER_INLINE
Variant::Variant(const Variant& v) noexcept {
  implCopyConstruct(*v.asTypedValue(), *this);
}

NEVER_INLINE
Variant::Variant(const_variant_ref v) noexcept {
  implCopyConstruct(*v.rval(), *this);
}

/*
 * The destruct functions below all arbitrarily take RefData* as an
 * example of a refcounted object, then just cast to the proper type.
 * This is safe because we have compile time assertions that guarantee that
 * the _count field will always be exactly FAST_REFCOUNT_OFFSET bytes from
 * the beginning of the object for the StringData, ArrayData, ObjectData,
 * ResourceHdr, and RefData classes.
 */

static_assert(typeToDestrIdx(KindOfArray)    == 0, "Array destruct index");
static_assert(typeToDestrIdx(KindOfShape)    == 1, "Shape destruct index");
static_assert(typeToDestrIdx(KindOfKeyset)   == 2, "Keyset destruct index");
static_assert(typeToDestrIdx(KindOfDict)     == 3, "Dict destruct index");
static_assert(typeToDestrIdx(KindOfVec)      == 4, "Vec destruct index");
static_assert(typeToDestrIdx(KindOfString)   == 5, "String destruct index");
static_assert(typeToDestrIdx(KindOfObject)   == 7, "Object destruct index");
static_assert(typeToDestrIdx(KindOfResource) == 8, "Resource destruct index");
static_assert(typeToDestrIdx(KindOfRef)      == 9, "Ref destruct index");

static_assert(kDestrTableSize == 10,
              "size of g_destructors[] must be kDestrTableSize");

RawDestructor g_destructors[] = {
  (RawDestructor)getMethodPtr(&ArrayData::release),   // KindOfArray
  (RawDestructor)&MixedArray::Release,                // KindOfShape
  (RawDestructor)&SetArray::Release,                  // KindOfKeyset
  (RawDestructor)&MixedArray::Release,                // KindOfDict
  (RawDestructor)&PackedArray::Release,               // KindOfVec
  (RawDestructor)getMethodPtr(&StringData::release),  // KindOfString
  nullptr, // hole
  (RawDestructor)getMethodPtr(&ObjectData::release),  // may replace at runtime
                                                      // KindOfObject
  (RawDestructor)getMethodPtr(&ResourceHdr::release), // KindOfResource
  (RawDestructor)getMethodPtr(&RefData::release),     // KindOfRef
};

void tweak_variant_dtors() {
  if (RuntimeOption::EnableObjDestructCall) return;
  destructorForType(KindOfObject) =
    (RawDestructor)getMethodPtr(&ObjectData::releaseNoObjDestructCheck);
}

#define IMPLEMENT_SET(argType, setOp)                     \
  void Variant::set(argType v) noexcept {                 \
    if (isPrimitive()) {                                  \
      setOp;                                              \
    } else if (isRefType(m_type)) {                     \
      m_data.pref->var()->set(v);                         \
    } else {                                              \
      auto const old = *asTypedValue();                   \
      setOp;                                              \
      tvDecRefCountable(old);                             \
    }                                                     \
  }

IMPLEMENT_SET(bool, m_type = KindOfBoolean; m_data.num = v)
IMPLEMENT_SET(int, m_type = KindOfInt64; m_data.num = v)
IMPLEMENT_SET(int64_t, m_type = KindOfInt64; m_data.num = v)
IMPLEMENT_SET(double, m_type = KindOfDouble; m_data.dbl = v)
IMPLEMENT_SET(const StaticString&,
              StringData* s = v.get();
              assertx(s);
              m_type = KindOfPersistentString;
              m_data.pstr = s)

#undef IMPLEMENT_SET

#define IMPLEMENT_PTR_SET(ptr, member, dtype)                           \
  void Variant::set(ptr *v) noexcept {                                  \
    Variant *self = isRefType(m_type) ? m_data.pref->var() : this;    \
    if (UNLIKELY(!v)) {                                                 \
      self->setNull();                                                  \
    } else {                                                            \
      v->incRefCount();                                                 \
      auto const old = *self->asTypedValue();                           \
      self->m_type = dtype;                                             \
      self->m_data.member = v;                                          \
      tvDecRefGen(old);                                                    \
    }                                                                   \
  }

IMPLEMENT_PTR_SET(StringData, pstr,
                  v->isRefCounted() ? KindOfString : KindOfPersistentString);
IMPLEMENT_PTR_SET(ArrayData, parr,
                  v->isRefCounted() ?
                  v->toDataType() : v->toPersistentDataType());
IMPLEMENT_PTR_SET(ObjectData, pobj, KindOfObject)
IMPLEMENT_PTR_SET(ResourceHdr, pres, KindOfResource)

#undef IMPLEMENT_PTR_SET

#define IMPLEMENT_STEAL(ptr, member, dtype)                             \
  void Variant::steal(ptr* v) noexcept {                                \
    Variant* self = isRefType(m_type) ? m_data.pref->var() : this;  \
    if (UNLIKELY(!v)) {                                                 \
      self->setNull();                                                  \
    } else {                                                            \
      auto const old = *self->asTypedValue();                           \
      self->m_type = dtype;                                             \
      self->m_data.member = v;                                          \
      tvDecRefGen(old);                                                    \
    }                                                                   \
  }

IMPLEMENT_STEAL(StringData, pstr,
                v->isRefCounted() ? KindOfString : KindOfPersistentString)
IMPLEMENT_STEAL(ArrayData, parr,
                v->isRefCounted() ?
                v->toDataType() : v->toPersistentDataType());
IMPLEMENT_STEAL(ObjectData, pobj, KindOfObject)
IMPLEMENT_STEAL(ResourceHdr, pres, KindOfResource)

#undef IMPLEMENT_STEAL

///////////////////////////////////////////////////////////////////////////////
// informational

bool Variant::isNumeric(bool checkString /* = false */) const noexcept {
  int64_t ival;
  double dval;
  DataType t = toNumeric(ival, dval, checkString);
  return t == KindOfInt64 || t == KindOfDouble;
}

DataType Variant::toNumeric(int64_t &ival, double &dval,
                            bool checkString /* = false */) const {
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
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
      return m_type;

    case KindOfInt64:
      ival = m_data.num;
      return KindOfInt64;

    case KindOfDouble:
      dval = m_data.dbl;
      return KindOfDouble;

    case KindOfPersistentString:
    case KindOfString:
      return checkString ? m_data.pstr->toNumeric(ival, dval) : m_type;

    case KindOfRef:
      return m_data.pref->var()->toNumeric(ival, dval, checkString);
  }
  not_reached();
}

bool Variant::isScalar() const noexcept {
  switch (getType()) {
    case KindOfUninit:
    case KindOfNull:
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
      return false;

    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfFunc:
    case KindOfClass:
      return true;

    case KindOfRef:
      always_assert(false && "isScalar() called on a boxed value");
  }
  not_reached();
}

static bool isAllowedAsConstantValueImpl(TypedValue tv) {
  switch (tv.m_type) {
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfPersistentVec:
    case KindOfPersistentDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentShape:
    case KindOfPersistentArray:
    case KindOfResource:
      return true;

    case KindOfVec:
    case KindOfDict:
    case KindOfShape:
    case KindOfArray: {
      if (tv.m_data.parr->isGlobalsArray()) return false;

      bool allowed = true;
      IterateV(
        tv.m_data.parr,
        [&] (TypedValue v) {
          if (!isAllowedAsConstantValueImpl(v)) allowed = false;
          return !allowed;
        }
      );
      return allowed;
    }

    case KindOfUninit:
    case KindOfObject:
    case KindOfRef:
    case KindOfFunc:
    case KindOfClass:
      return false;
  }
  not_reached();
}

bool Variant::isAllowedAsConstantValue() const {
  return isAllowedAsConstantValueImpl(*this);
}

///////////////////////////////////////////////////////////////////////////////

inline DataType Variant::convertToNumeric(int64_t *lval, double *dval) const {
  StringData *s = getStringData();
  assertx(s);
  return s->isNumericWithVal(*lval, *dval, 1);
}

///////////////////////////////////////////////////////////////////////////////
// type conversions

bool Variant::toBooleanHelper() const {
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:         assertx(false); return m_data.num;
    case KindOfDouble:        return m_data.dbl != 0;
    case KindOfPersistentString:
    case KindOfString:        return m_data.pstr->toBoolean();
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray:         return !m_data.parr->empty();
    case KindOfObject:        return m_data.pobj->toBoolean();
    case KindOfResource:      return m_data.pres->data()->o_toBoolean();
    case KindOfFunc:
      return funcToStringHelper(m_data.pfunc)->toBoolean();
    case KindOfClass:
      return classToStringHelper(m_data.pclass)->toBoolean();
    case KindOfRef:           return m_data.pref->var()->toBoolean();
      always_assert(false);
  }
  not_reached();
}

int64_t Variant::toInt64Helper(int base /* = 10 */) const {
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:         assertx(false); return m_data.num;
    case KindOfDouble:        return double_to_int64(m_data.dbl);
    case KindOfPersistentString:
    case KindOfString:        return m_data.pstr->toInt64(base);
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray:         return m_data.parr->empty() ? 0 : 1;
    case KindOfObject:        return m_data.pobj->toInt64();
    case KindOfResource:      return m_data.pres->data()->o_toInt64();
    case KindOfFunc:
      return funcToStringHelper(m_data.pfunc)->toInt64();
    case KindOfClass:
      return classToStringHelper(m_data.pclass)->toInt64();
    case KindOfRef:           return m_data.pref->var()->toInt64(base);
      always_assert(false);
  }
  not_reached();
}

double Variant::toDoubleHelper() const {
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:          return 0.0;
    case KindOfBoolean:
    case KindOfInt64:         return (double)toInt64();
    case KindOfDouble:        return m_data.dbl;
    case KindOfPersistentString:
    case KindOfString:        return m_data.pstr->toDouble();
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray:         return (double)toInt64();
    case KindOfObject:        return m_data.pobj->toDouble();
    case KindOfResource:      return m_data.pres->data()->o_toDouble();
    case KindOfFunc:
      return funcToStringHelper(m_data.pfunc)->toDouble();
    case KindOfClass:
      return classToStringHelper(m_data.pclass)->toDouble();
    case KindOfRef:           return m_data.pref->var()->toDouble();
      always_assert(false);
  }
  not_reached();
}

Array Variant::toPHPArrayHelper() const {
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:          return empty_array();
    case KindOfBoolean:       return Array::Create(*this);
    case KindOfInt64:         return Array::Create(m_data.num);
    case KindOfDouble:        return Array::Create(*this);
    case KindOfPersistentString:
      return Array::Create(Variant{m_data.pstr, PersistentStrInit{}});
    case KindOfString:
      return Array::Create(Variant{m_data.pstr});
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentKeyset:
    case KindOfKeyset:        return ArrNR{m_data.parr}.asArray().toPHPArray();
    case KindOfPersistentArray:
    case KindOfArray:         return Array(m_data.parr);
    case KindOfObject:        return m_data.pobj->toArray();
    case KindOfResource:      return m_data.pres->data()->o_toArray();
    case KindOfFunc:
      return Array::Create(Variant{funcToStringHelper(m_data.pfunc),
                                   PersistentStrInit{}});
    case KindOfClass:
      return Array::Create(Variant{classToStringHelper(m_data.pclass),
                                   PersistentStrInit{}});
    case KindOfRef:           return m_data.pref->var()->toArray();
      always_assert(false);

  }
  not_reached();
}

Resource Variant::toResourceHelper() const {
  switch (m_type) {
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
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfFunc:
    case KindOfClass:
      return Resource(req::make<DummyResource>());

    case KindOfResource:
      return Resource{m_data.pres};

    case KindOfRef:
      return m_data.pref->var()->toResource();

  }
  not_reached();
}

req::root<Variant>* blackHolePtr() {
  void* p = &bhStorage;
  return reinterpret_cast<req::root<Variant>*>(p);
}

void initBlackHole() {
  new (blackHolePtr()) req::root<Variant>();
}

void clearBlackHole() {
  using req::root;
  blackHolePtr()->~root<Variant>();
}

Variant& lvalBlackHole() {
  blackHolePtr()->unset();
  blackHolePtr()->setNull();
  return *blackHolePtr();
}

void Variant::setEvalScalar() {
  assertx(cellIsPlausible(*this));

  auto const do_array = [this]{
    if (!m_data.parr->isStatic()) {
      ArrayData::GetScalarArray(&m_data.parr);
      assertx(m_data.parr->isStatic());
    }
  };

  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
      return;

    case KindOfString:
      m_type = KindOfPersistentString;
    case KindOfPersistentString: {
      auto pstr = m_data.pstr;
      if (!pstr->isStatic()) {
        StringData *sd = makeStaticString(pstr);
        decRefStr(pstr);
        m_data.pstr = sd;
        assertx(m_data.pstr->isStatic());
      }
      return;
    }

    case KindOfVec:
      m_type = KindOfPersistentVec;
    case KindOfPersistentVec:
      do_array();
      return;

    case KindOfDict:
      m_type = KindOfPersistentDict;
    case KindOfPersistentDict:
      do_array();
      return;

    case KindOfKeyset:
      m_type = KindOfPersistentKeyset;
    case KindOfPersistentKeyset:
      do_array();
      return;

    case KindOfShape:
      m_type = KindOfPersistentShape;
    case KindOfPersistentShape:
      do_array();
      return;

    case KindOfArray:
      m_type = KindOfPersistentArray;
    case KindOfPersistentArray:
      do_array();
      return;

    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    case KindOfFunc:
    case KindOfClass:
      break;
  }
  not_reached();
}

VarNR::VarNR(const String& v) {
  init(KindOfString);
  StringData *s = v.get();
  if (s) {
    m_data.pstr = s;
  } else {
    m_type = KindOfNull;
  }
}

VarNR::VarNR(const Array& v) {
  ArrayData *a = v.get();
  if (a) {
    init(a->toDataType());
    m_data.parr = a;
  } else {
    init(KindOfNull);
  }
}

VarNR::VarNR(const Object& v) {
  init(KindOfObject);
  ObjectData *o = v.get();
  if (o) {
    m_data.pobj = o;
  } else {
    m_type = KindOfNull;
  }
}

VarNR::VarNR(StringData *v) {
  init(KindOfString);
  if (v) {
    m_data.pstr = v;
  } else {
    m_type = KindOfNull;
  }
}

VarNR::VarNR(ArrayData *v) {
  if (v) {
    init(v->toDataType());
    m_data.parr = v;
  } else {
    init(KindOfNull);
  }
}

VarNR::VarNR(ObjectData *v) {
  init(KindOfObject);
  if (v) {
    m_data.pobj = v;
  } else {
    m_type = KindOfNull;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
