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
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/system/systemlib.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/logger.h"
#include "hphp/util/low-ptr.h"

#include <limits>
#include <utility>
#include <vector>

namespace HPHP {

using BlackHoleStorage = std::aligned_storage<
  sizeof(req::root<Variant>),
  alignof(req::root<Variant>)
>::type;

static RDS_LOCAL(BlackHoleStorage, bhStorage);

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
  tvDup(tvToInit(tv), v.asTypedValue());
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

namespace {

void objReleaseWrapper(ObjectData* obj) noexcept {
  auto const cls = obj->getVMClass();
  cls->releaseFunc()(obj, cls);
}

}

static_assert(typeToDestrIdx(KindOfDArray)   == 0, "Array destruct index");
static_assert(typeToDestrIdx(KindOfVArray)   == 1, "Array destruct index");
static_assert(typeToDestrIdx(KindOfArray)    == 2, "Array destruct index");
static_assert(typeToDestrIdx(KindOfKeyset)   == 3, "Keyset destruct index");
static_assert(typeToDestrIdx(KindOfDict)     == 4, "Dict destruct index");
static_assert(typeToDestrIdx(KindOfVec)      == 5, "Vec destruct index");
static_assert(typeToDestrIdx(KindOfRecord)   == 6, "Record destruct index");
static_assert(typeToDestrIdx(KindOfString)   == 7, "String destruct index");
static_assert(typeToDestrIdx(KindOfObject)   == 9, "Object destruct index");
static_assert(typeToDestrIdx(KindOfResource) == 10, "Resource destruct index");
#ifndef USE_LOWPTR
static_assert(typeToDestrIdx(KindOfClsMeth)  == 11, "ClsMeth destruct index");
#endif

static_assert(kDestrTableSize == (use_lowptr ? 11 : 12),
              "size of g_destructors[] must be kDestrTableSize");

RawDestructor g_destructors[] = {
  (RawDestructor)getMethodPtr(&ArrayData::release),   // KindOfDArray
  (RawDestructor)getMethodPtr(&ArrayData::release),   // KindOfVArray
  (RawDestructor)getMethodPtr(&ArrayData::release),   // KindOfArray
  (RawDestructor)&SetArray::Release,                  // KindOfKeyset
  (RawDestructor)&MixedArray::Release,                // KindOfDict
  (RawDestructor)&PackedArray::Release,               // KindOfVec
  (RawDestructor)getMethodPtr(&RecordData::release),  // KindOfRecord
  (RawDestructor)getMethodPtr(&StringData::release),  // KindOfString
  nullptr, // hole
  (RawDestructor)&objReleaseWrapper,                  // KindOfObject
  (RawDestructor)getMethodPtr(&ResourceHdr::release), // KindOfResource
#ifndef USE_LOWPTR
  (RawDestructor)&ClsMethDataRef::Release,            // KindOfClsMeth
#endif
};

#define IMPLEMENT_SET(argType, setOp)                     \
  void Variant::set(argType v) noexcept {                 \
    if (isPrimitive()) {                                  \
      setOp;                                              \
    } else {                                              \
      auto const old = *asTypedValue();                   \
      setOp;                                              \
      tvDecRefCountable(old);                             \
    }                                                     \
  }                                                       \
  void variant_ref::set(argType v) noexcept {             \
    Value& m_data = val(m_val);                           \
    DataType& m_type = type(m_val);                       \
    if (isPrimitive()) {                                  \
      setOp;                                              \
    } else {                                              \
      auto const old = m_val.tv();                        \
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
    if (UNLIKELY(!v)) {                                                 \
      this->setNull();                                                  \
    } else {                                                            \
      v->incRefCount();                                                 \
      const TypedValue old = *this->asTypedValue();                     \
      this->m_type = dtype;                                             \
      this->m_data.member = v;                                          \
      tvDecRefGen(old);                                                 \
    }                                                                   \
  }                                                                     \
  void variant_ref::set(ptr *v) noexcept {                              \
    if (UNLIKELY(!v)) {                                                 \
      this->setNull();                                                  \
    } else {                                                            \
      v->incRefCount();                                                 \
      const TypedValue old = this->m_val.tv();                          \
      type(this->m_val) = dtype;                                        \
      val(this->m_val).member = v;                                      \
      tvDecRefGen(old);                                                 \
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
    if (UNLIKELY(!v)) {                                                 \
      this->setNull();                                                  \
    } else {                                                            \
      const TypedValue old = *this->asTypedValue();                     \
      this->m_type = dtype;                                             \
      this->m_data.member = v;                                          \
      tvDecRefGen(old);                                                 \
    }                                                                   \
  }                                                                     \
  void variant_ref::steal(ptr* v) noexcept {                            \
    if (UNLIKELY(!v)) {                                                 \
      this->setNull();                                                  \
    } else {                                                            \
      const TypedValue old = this->m_val.tv();                          \
      type(this->m_val) = dtype;                                        \
      val(this->m_val).member = v;                                      \
      tvDecRefGen(old);                                                 \
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
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfRecord:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:
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
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfClsMeth:
    case KindOfRecord:
      return false;

    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfFunc:
    case KindOfClass:
      return true;
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
    case KindOfPersistentDArray:
    case KindOfPersistentVArray:
    case KindOfPersistentArray:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClsMeth:
      return true;

    case KindOfDArray:
    case KindOfVArray:
    case KindOfVec:
    case KindOfDict:
    case KindOfArray: {
      if (tv.m_data.parr->isGlobalsArrayKind()) return false;

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
    case KindOfClass:
    case KindOfRecord:
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
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:         return !m_data.parr->empty();
    case KindOfObject:        return m_data.pobj->toBoolean();
    case KindOfResource:      return m_data.pres->data()->o_toBoolean();
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:       return true;
    case KindOfRecord:
      raise_convert_record_to_type("bool");
      return false;
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
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:         return m_data.parr->empty() ? 0 : 1;
    case KindOfObject:        return m_data.pobj->toInt64();
    case KindOfResource:      return m_data.pres->data()->o_toInt64();
    case KindOfFunc:
      return funcToInt64Helper(m_data.pfunc);
    case KindOfClass:
      return classToStringHelper(m_data.pclass)->toInt64();
    case KindOfClsMeth:
      raiseClsMethConvertWarningHelper("int");
      return 1;
    case KindOfRecord:
      raise_convert_record_to_type("int");
      return 0;
  }
  not_reached();
}

Array Variant::toPHPArrayHelper() const {
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:          return empty_array();
    case KindOfBoolean:       return make_packed_array(*this);
    case KindOfInt64:         return make_packed_array(m_data.num);
    case KindOfDouble:        return make_packed_array(*this);
    case KindOfPersistentString:
      return make_packed_array(Variant{m_data.pstr, PersistentStrInit{}});
    case KindOfString:
      return make_packed_array(Variant{m_data.pstr});
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:        return ArrNR{m_data.parr}.asArray().toPHPArray();
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:         return Array(m_data.parr);
    case KindOfObject:        return m_data.pobj->toArray();
    case KindOfResource:      return m_data.pres->data()->o_toArray();
    case KindOfFunc:
      return make_packed_array(Variant{funcToStringHelper(m_data.pfunc),
                                       PersistentStrInit{}});
    case KindOfClass:
      return make_packed_array(Variant{classToStringHelper(m_data.pclass),
                                       PersistentStrInit{}});
    case KindOfClsMeth:
      raiseClsMethConvertWarningHelper("array");
      return make_packed_array(
        m_data.pclsmeth->getClsStr(), m_data.pclsmeth->getFuncStr());
    case KindOfRecord:
      raise_convert_record_to_type("array");
      return empty_array();
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
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:
    case KindOfRecord:
      return Resource(req::make<DummyResource>());

    case KindOfResource:
      return Resource{m_data.pres};
  }
  not_reached();
}

req::root<Variant>* blackHolePtr() {
  void* p = bhStorage.get();
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
  assertx(tvIsPlausible(*this));

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

    case KindOfDArray:
      m_type = KindOfPersistentDArray;
    case KindOfPersistentDArray:
      do_array();
      return;

    case KindOfVArray:
      m_type = KindOfPersistentVArray;
    case KindOfPersistentVArray:
      do_array();
      return;

    case KindOfArray:
      m_type = KindOfPersistentArray;
    case KindOfPersistentArray:
      do_array();
      return;

    case KindOfObject:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      break;

    case KindOfRecord:
      raise_error(Strings::RECORD_NOT_SUPPORTED);

    case KindOfClsMeth:
      raiseClsMethToVecWarningHelper();
      auto const clsMeth = m_data.pclsmeth;
      m_data.parr = clsMethToVecHelper(clsMeth).detach();
      m_type = RuntimeOption::EvalHackArrDVArrs ?
               KindOfPersistentVec : KindOfPersistentArray;
      decRefClsMeth(clsMeth);
      do_array();
      return;
  }
  not_reached();
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
