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

#include "hphp/runtime/base/type-variant.h"

#include "hphp/parser/hphp.tab.hpp"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"

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

const Variant null_variant;                         // uninitialized variant
const Variant init_null_variant((Variant::NullInit())); // php null
const VarNR null_varNR;
const VarNR true_varNR(true);
const VarNR false_varNR(false);
const VarNR INF_varNR(std::numeric_limits<double>::infinity());
const VarNR NEGINF_varNR(std::numeric_limits<double>::infinity());
const VarNR NAN_varNR(std::numeric_limits<double>::quiet_NaN());
const Variant empty_string_variant_ref(staticEmptyString(),
                                       Variant::PersistentStrInit{});

///////////////////////////////////////////////////////////////////////////////
// static strings

const StaticString
  s_scalar("scalar"),
  s_1("1");

///////////////////////////////////////////////////////////////////////////////

Variant::Variant(StringData *v) noexcept {
  if (v) {
    m_data.pstr = v;
    if (!v->isRefCounted()) {
      m_type = KindOfPersistentString;
    } else {
      m_type = KindOfString;
      v->incRefCount();
    }
  } else {
    m_type = KindOfNull;
  }
}

// the version of the high frequency function that is not inlined
NEVER_INLINE
Variant::Variant(const Variant& v) noexcept {
  constructValHelper(v);
}

/*
 * The destruct functions below all arbitrarily take RefData* as an
 * example of a refcounted object, then just cast to the proper type.
 * This is safe because we have compile time assertions that guarantee that
 * the _count field will always be exactly FAST_REFCOUNT_OFFSET bytes from
 * the beginning of the object for the StringData, ArrayData, ObjectData,
 * ResourceHdr, and RefData classes.
 */

static_assert(typeToDestrIdx(KindOfString) == 2, "String destruct index");
static_assert(typeToDestrIdx(KindOfArray)  == 3,  "Array destruct index");
static_assert(typeToDestrIdx(KindOfObject) == 4, "Object destruct index");
static_assert(typeToDestrIdx(KindOfResource) == 5,
              "Resource destruct index");
static_assert(typeToDestrIdx(KindOfRef)    == 6,    "Ref destruct index");

static_assert(kDestrTableSize == 7,
              "size of g_destructors[] must be kDestrTableSize");

RawDestructor g_destructors[] = {
  nullptr,
  nullptr,
  (RawDestructor)getMethodPtr(&StringData::release),
  (RawDestructor)getMethodPtr(&ArrayData::release),
  (RawDestructor)getMethodPtr(&ObjectData::release), // may replace at runtime
  (RawDestructor)getMethodPtr(&ResourceHdr::release),
  (RawDestructor)getMethodPtr(&RefData::release),
};

void tweak_variant_dtors() {
  if (RuntimeOption::EnableObjDestructCall) return;
  g_destructors[typeToDestrIdx(KindOfObject)] =
    (RawDestructor)getMethodPtr(&ObjectData::releaseNoObjDestructCheck);
}

Variant &Variant::assign(const Variant& v) noexcept {
  AssignValHelper(this, &v);
  return *this;
}

Variant& Variant::assignRef(Variant& v) noexcept {
  assignRefHelper(v);
  return *this;
}

Variant& Variant::setWithRef(const Variant& v) noexcept {
  setWithRefHelper(v, isRefcountedType(m_type));
  return *this;
}

#define IMPLEMENT_SET(argType, setOp)                     \
  void Variant::set(argType v) noexcept {                 \
    if (isPrimitive()) {                                  \
      setOp;                                              \
    } else if (m_type == KindOfRef) {                     \
      m_data.pref->var()->set(v);                         \
    } else {                                              \
      auto const d = m_data.num;                          \
      auto const t = m_type;                              \
      setOp;                                              \
      tvDecRefHelper(t, d);                               \
    }                                                     \
  }

IMPLEMENT_SET(bool, m_type = KindOfBoolean; m_data.num = v)
IMPLEMENT_SET(int, m_type = KindOfInt64; m_data.num = v)
IMPLEMENT_SET(int64_t, m_type = KindOfInt64; m_data.num = v)
IMPLEMENT_SET(double, m_type = KindOfDouble; m_data.dbl = v)
IMPLEMENT_SET(const StaticString&,
              StringData* s = v.get();
              assert(s);
              m_type = KindOfPersistentString;
              m_data.pstr = s)

#undef IMPLEMENT_SET

#define IMPLEMENT_PTR_SET(ptr, member, dtype)                           \
  void Variant::set(ptr *v) noexcept {                                  \
    Variant *self = m_type == KindOfRef ? m_data.pref->var() : this;    \
    if (UNLIKELY(!v)) {                                                 \
      self->setNull();                                                  \
    } else {                                                            \
      v->incRefCount();                                                 \
      auto const d = self->m_data.num;                                  \
      auto const t = self->m_type;                                      \
      self->m_type = dtype;                                             \
      self->m_data.member = v;                                          \
      tvRefcountedDecRefHelper(t, d);                                   \
    }                                                                   \
  }

IMPLEMENT_PTR_SET(StringData, pstr,
                  v->isRefCounted() ? KindOfString : KindOfPersistentString);
IMPLEMENT_PTR_SET(ArrayData, parr, KindOfArray)
IMPLEMENT_PTR_SET(ObjectData, pobj, KindOfObject)
IMPLEMENT_PTR_SET(ResourceHdr, pres, KindOfResource)

#undef IMPLEMENT_PTR_SET

#define IMPLEMENT_STEAL(ptr, member, dtype)                             \
  void Variant::steal(ptr* v) noexcept {                                \
    Variant* self = (m_type == KindOfRef) ? m_data.pref->var() : this;  \
    if (UNLIKELY(!v)) {                                                 \
      self->setNull();                                                  \
    } else {                                                            \
      auto const d = self->m_data.num;                                  \
      auto const t = self->m_type;                                      \
      self->m_type = dtype;                                             \
      self->m_data.member = v;                                          \
      tvRefcountedDecRefHelper(t, d);                                   \
    }                                                                   \
  }

IMPLEMENT_STEAL(StringData, pstr,
                v->isRefCounted() ? KindOfString : KindOfPersistentString)
IMPLEMENT_STEAL(ArrayData, parr, KindOfArray)
IMPLEMENT_STEAL(ObjectData, pobj, KindOfObject)
IMPLEMENT_STEAL(ResourceHdr, pres, KindOfResource)

#undef IMPLEMENT_STEAL

int Variant::getRefCount() const noexcept {
  return isRefcountedType(m_type) ? tvGetCount(asTypedValue()) : 1;
}

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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
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

    case KindOfClass:
      break;
  }
  not_reached();
}

bool Variant::isScalar() const noexcept {
  switch (getType()) {
    case KindOfUninit:
    case KindOfNull:
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
      return true;

    case KindOfRef:
      always_assert(false && "isScalar() called on a boxed value");

    case KindOfClass:
      break;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

inline DataType Variant::convertToNumeric(int64_t *lval, double *dval) const {
  StringData *s = getStringData();
  assert(s);
  return s->isNumericWithVal(*lval, *dval, 1);
}

///////////////////////////////////////////////////////////////////////////////
// type conversions

bool Variant::toBooleanHelper() const {
  assert(m_type > KindOfInt64);
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:         return m_data.num;
    case KindOfDouble:        return m_data.dbl != 0;
    case KindOfPersistentString:
    case KindOfString:        return m_data.pstr->toBoolean();
    case KindOfPersistentArray:
    case KindOfArray:         return !m_data.parr->empty();
    case KindOfObject:        return m_data.pobj->toBoolean();
    case KindOfResource:      return m_data.pres->data()->o_toBoolean();
    case KindOfRef:           return m_data.pref->var()->toBoolean();
    case KindOfClass:         break;
  }
  not_reached();
}

int64_t Variant::toInt64Helper(int base /* = 10 */) const {
  assert(m_type > KindOfInt64);
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:         return m_data.num;
    case KindOfDouble:        return HPHP::toInt64(m_data.dbl);
    case KindOfPersistentString:
    case KindOfString:        return m_data.pstr->toInt64(base);
    case KindOfPersistentArray:
    case KindOfArray:         return m_data.parr->empty() ? 0 : 1;
    case KindOfObject:        return m_data.pobj->toInt64();
    case KindOfResource:      return m_data.pres->data()->o_toInt64();
    case KindOfRef:           return m_data.pref->var()->toInt64(base);
    case KindOfClass:         break;
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
    case KindOfPersistentArray:
    case KindOfArray:         return (double)toInt64();
    case KindOfObject:        return m_data.pobj->toDouble();
    case KindOfResource:      return m_data.pres->data()->o_toDouble();
    case KindOfRef:           return m_data.pref->var()->toDouble();
    case KindOfClass:         break;
  }
  not_reached();
}

String Variant::toStringHelper() const {
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
      return empty_string();

    case KindOfBoolean:
      return m_data.num ? static_cast<String>(s_1)
                        : empty_string();

    case KindOfInt64:
      return m_data.num;

    case KindOfDouble:
      return m_data.dbl;

    case KindOfPersistentString:
    case KindOfString:
      assert(false); // Should be done in caller
      return String{m_data.pstr};

    case KindOfPersistentArray:
    case KindOfArray:
      raise_notice("Array to string conversion");
      return array_string;

    case KindOfObject:
      return m_data.pobj->invokeToString();

    case KindOfResource:
      return m_data.pres->data()->o_toString();

    case KindOfRef:
      return m_data.pref->var()->toString();

    case KindOfClass:
      break;
  }
  not_reached();
}

Array Variant::toArrayHelper() const {
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
    case KindOfPersistentArray:
    case KindOfArray:         return Array(m_data.parr);
    case KindOfObject:        return m_data.pobj->toArray();
    case KindOfResource:      return m_data.pres->data()->o_toArray();
    case KindOfRef:           return m_data.pref->var()->toArray();
    case KindOfClass:         break;
  }
  not_reached();
}

Object Variant::toObjectHelper() const {
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
      return SystemLib::AllocStdClassObject();

    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfResource: {
      auto obj = SystemLib::AllocStdClassObject();
      obj->o_set(s_scalar, *this, false);
      return obj;
    }


    case KindOfPersistentArray:
    case KindOfArray:
      return ObjectData::FromArray(m_data.parr);

    case KindOfObject:
      return Object{m_data.pobj};

    case KindOfRef:
      return m_data.pref->var()->toObject();

    case KindOfClass:
      break;
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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
      return Resource(req::make<DummyResource>());

    case KindOfResource:
      return Resource{m_data.pres};

    case KindOfRef:
      return m_data.pref->var()->toResource();

    case KindOfClass:
      break;
  }
  not_reached();
}

VarNR Variant::toKey() const {
  if (isStringType(m_type)) {
    int64_t n;
    return m_data.pstr->isStrictlyInteger(n) ? VarNR(n) :
                                               VarNR(m_data.pstr);
  }
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
      return VarNR(staticEmptyString());

    case KindOfBoolean:
    case KindOfInt64:
      return VarNR(m_data.num);

    case KindOfDouble:
    case KindOfResource:
      return VarNR(toInt64());

    case KindOfPersistentString:
    case KindOfString:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
      throw_bad_type_exception("Invalid type used as key");
      return null_varNR;

    case KindOfRef:
      return m_data.pref->var()->toKey();

    case KindOfClass:
      break;
  }
  not_reached();
}

Variant& lvalBlackHole() {
  auto& bh = get_env_constants()->lvalProxy;
  bh = uninit_null();
  return bh;
}

void Variant::setEvalScalar() {
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
      return;

    case KindOfPersistentString:
    case KindOfString: {
      auto pstr = m_data.pstr;
      if (!pstr->isStatic()) {
        StringData *sd = makeStaticString(pstr);
        decRefStr(pstr);
        m_data.pstr = sd;
        assert(m_data.pstr->isStatic());
        m_type = KindOfPersistentString;
      }
      return;
    }

    case KindOfPersistentArray:
    case KindOfArray: {
      auto parr = m_data.parr;
      if (!parr->isStatic()) {
        auto ad = ArrayData::GetScalarArray(parr);
        assert(ad->isStatic());
        m_data.parr = ad;
        m_type = KindOfPersistentArray;
        decRefArr(parr);
      }
      return;
    }

    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
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
  init(KindOfArray);
  ArrayData *a = v.get();
  if (a) {
    m_data.parr = a;
  } else {
    m_type = KindOfNull;
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
  init(KindOfArray);
  if (v) {
    m_data.parr = v;
  } else {
    m_type = KindOfNull;
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
