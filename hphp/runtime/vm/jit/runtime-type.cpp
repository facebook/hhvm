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
#include "hphp/runtime/vm/jit/runtime-type.h"

#include <assert.h>
#include <stdint.h>
#include <stdarg.h>
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/translator.h"

#define KindOfUnknown DontUseKindOfUnknownInThisFile
#define KindOfInvalid DontUseKindOfInvalidInThisFile

namespace HPHP {
namespace JIT {

static inline DataType
normalizeDataType(DataType dt) {
  // The translator treats both KindOfStaticString and KindOfString
  // identically, and uses translation-time IS_REFCOUNTED_TYPE checks
  // to determine how to handle refcounting. This means that an old
  // KindOfStaticstring translation can get reused with KindOfString
  // values. Since we emit static checks regardless, just prevent
  // KindOfStaticString from entering into the translator's awareness.
  return dt == KindOfStaticString ? KindOfString : dt;
}

void RuntimeType::init(DataType outer,
                       DataType inner /* = KindOfNone */,
                       const Class* klass /*= nullptr*/) {
  m_value.outerType = outer;
  m_value.innerType = inner;
  m_value.klass = klass;
  m_value.knownClass = nullptr;
  consistencyCheck();
}

RuntimeType::RuntimeType(DataType outer, DataType inner /* = KindOfNone */,
                         const Class* klass /* = NULL */)
  : m_kind(VALUE) {
  init(normalizeDataType(outer), normalizeDataType(inner), klass);
}

RuntimeType::RuntimeType(const StringData* sd)
  : m_kind(VALUE) {
  init(KindOfString);
  m_value.string = sd;
}

RuntimeType::RuntimeType(const ArrayData* ad)
  : m_kind(VALUE) {
  init(KindOfArray);
  m_value.array = ad;
}

RuntimeType::RuntimeType(bool value)
  : m_kind(VALUE) {
  init(KindOfBoolean);
  m_value.boolean = value;
  m_value.boolValid = true;
}

RuntimeType::RuntimeType(int64_t value)
  : m_kind(VALUE) {
  init(KindOfInt64);
  m_value.intval = value;
}

RuntimeType::RuntimeType(const Class* klass)
  : m_kind(VALUE) {
  init(KindOfClass, KindOfNone, klass);
}

RuntimeType::RuntimeType() :
  m_kind(VALUE) {
  init(KindOfNone);
}

RuntimeType::RuntimeType(const Iter* it) :
  m_kind(ITER) {
}

RuntimeType::RuntimeType(ArrayIter::Type type) :
  m_kind(ITER) {
}

RuntimeType RuntimeType::box() const {
  assert(m_kind == VALUE);
  if (m_value.outerType == KindOfRef) {
    consistencyCheck();
    return *this;
  }
  RuntimeType rtt(KindOfRef, m_value.outerType);
  return rtt;
}

RuntimeType RuntimeType::unbox() const {
  assert(m_kind == VALUE);
  if (m_value.outerType != KindOfRef) {
    consistencyCheck();
    return *this;
  }
  RuntimeType rtt(m_value.innerType);
  return rtt;
}

DataType RuntimeType::valueType() const {
  assert(m_kind != ITER);
  if (outerType() == KindOfRef) {
    return m_value.innerType;
  }
  return m_value.outerType;
}

const Class*
RuntimeType::valueClass() const {
  consistencyCheck();
  assert(m_kind != ITER);
  assert(valueType() == KindOfObject || valueType() == KindOfClass);
  return m_value.klass;
}

const StringData*
RuntimeType::valueString() const {
  consistencyCheck();
  assert(m_kind != ITER);
  assert(isString());
  return m_value.string;
}

const StringData*
RuntimeType::valueStringOrNull() const {
  if (!isString()) return nullptr;
  return valueString();
}

const ArrayData*
RuntimeType::valueArray() const {
  consistencyCheck();
  assert(m_kind != ITER);
  assert(isArray());
  return m_value.array;
}

// -1 for unknown, 0 for false, 1 for true
int
RuntimeType::valueBoolean() const {
  consistencyCheck();
  assert(m_kind != ITER);
  assert(isBoolean());
  return m_value.boolValid ? m_value.boolean : -1;
}

int64_t
RuntimeType::valueInt() const {
  consistencyCheck();
  assert(m_kind == VALUE);
  assert(isInt());
  return m_value.intval;
}

// Get the value as a blob. Use with care.
int64_t
RuntimeType::valueGeneric() const {
  consistencyCheck();
  assert(m_kind == VALUE);
  return m_value.intval;
}

const Class*
RuntimeType::knownClass() const {
  consistencyCheck();
  assert(hasKnownClass());
  return m_value.knownClass;
}

bool
RuntimeType::hasArrayKind() const {
  consistencyCheck();
  return m_value.arrayKindValid;
}

ArrayData::ArrayKind
RuntimeType::arrayKind() const {
  consistencyCheck();
  assert(hasArrayKind());
  return m_value.arrayKind;
}

RuntimeType
RuntimeType::setValueType(DataType newInner) const {
  assert(m_kind == VALUE);
  if (outerType() == KindOfRef) {
    RuntimeType rtt(KindOfRef, newInner);
    assert(rtt.valueType() == newInner);
    return rtt;
  }
  RuntimeType rtt(newInner);
  assert(rtt.valueType() == newInner);
  return rtt;
}

RuntimeType
RuntimeType::setKnownClass(const Class* klass) const {
  assert(isObject() || (isRef() && innerType() == KindOfObject));
  RuntimeType rtt(outerType(), innerType(), m_value.klass);
  rtt.m_kind = this->m_kind;
  rtt.m_value.knownClass = klass;
  rtt.consistencyCheck();
  return rtt;
}

RuntimeType
RuntimeType::setArrayKind(ArrayData::ArrayKind arrayKind) const {
  assert(isArray() || (isRef() && innerType() == KindOfArray));
  RuntimeType rtt;
  rtt.m_kind = this->m_kind;
  rtt.m_value.outerType = outerType();
  rtt.m_value.innerType = innerType();
  rtt.m_value.arrayKindValid = true;
  rtt.m_value.arrayKind = arrayKind;
  rtt.consistencyCheck();
  return rtt;
}

// Accessors
DataType RuntimeType::outerType() const {
  consistencyCheck();
  assert(m_kind == VALUE);
  return m_value.outerType;
}

DataType RuntimeType::innerType() const {
  consistencyCheck();
  assert(m_kind == VALUE);
  return m_value.innerType;
}

bool RuntimeType::isValue() const {
  consistencyCheck();
  return m_kind == VALUE;
}

DataType RuntimeType::typeCheckValue() const {
  if (isIter()) return (DataType)0;
  return outerType();
}

bool RuntimeType::isIter() const {
  consistencyCheck();
  return m_kind == ITER;
}

bool RuntimeType::isRef() const {
  assert(m_kind == VALUE);
  return outerType() == KindOfRef;
}

bool RuntimeType::isVagueValue() const {
  assert(IMPLIES(m_kind == VALUE, outerType() != KindOfNone));
  return m_kind == VALUE && outerType() == KindOfAny;
}

bool RuntimeType::isRefCounted() const {
  return isValue() && IS_REFCOUNTED_TYPE(outerType());
}

bool RuntimeType::isUninit() const {
  return isValue() && outerType() == KindOfUninit;
}

bool RuntimeType::isNull() const {
  return isValue() && IS_NULL_TYPE(outerType());
}

bool RuntimeType::isInt() const {
  return isValue() && IS_INT_TYPE(outerType());
}

bool RuntimeType::isDouble() const {
  return isValue() && IS_DOUBLE_TYPE(outerType());
}

bool RuntimeType::isBoolean() const {
  return isValue() && outerType() == KindOfBoolean;
}

bool RuntimeType::isString() const {
  return isValue() && IS_STRING_TYPE(outerType());
}

bool RuntimeType::isObject() const {
  return isValue() && outerType() == KindOfObject;
}

bool RuntimeType::isClass() const {
  return isValue() && outerType() == KindOfClass;
}

bool RuntimeType::hasKnownClass() const {
  return isObject() && m_value.knownClass != nullptr;
}

bool RuntimeType::isArray() const {
  return isValue() && outerType() == KindOfArray;
}

bool RuntimeType::operator==(const RuntimeType& r) const {
  consistencyCheck();
  if (m_kind != r.m_kind) {
    return false;
  }
  switch (m_kind) {
    case ITER:
      return true;
    case VALUE:
      return r.m_value.innerType == m_value.innerType &&
             r.m_value.outerType == m_value.outerType &&
             r.m_value.klass == m_value.klass;
    default:
      assert(false);
      return false;
  }
}

size_t
RuntimeType::operator()(const RuntimeType& r) const {
  uint64_t p1 = HPHP::hash_int64(m_kind);
  uint64_t p2 = 0;
  // We can't just hash the whole blob of memory, because
  // C++ will leave padding uninitialized. The shifts are to
  // make the final hash order-dependent, so that
  //    { field1: 0, field2: 1 }
  // has a different hash than
  //    { field1: 1, field2: 0 }
  switch(m_kind) {
    case ITER:
      p2 = 0;
      break;
    case VALUE:
      p2 = HPHP::hash_int64_pair(uintptr_t(m_value.klass),
                                 HPHP::hash_int64_pair(m_value.outerType,
                                                       m_value.innerType));
      break;
  }
  return p1 ^ (p2 << 1);
}

using std::string;

string RuntimeType::pretty() const {
  char buf[1024];
  if (isIter()) {
    sprintf(buf, "(Iter)");
    return std::string(buf);
  }
  if (m_value.outerType == KindOfRef) {
    sprintf(buf, "(Value (Var %s))", tname(m_value.innerType).c_str());
  } else {
    sprintf(buf, "(Value %s)", tname(m_value.outerType).c_str());
  }
  string retval = buf;
  if (valueType() == KindOfObject) {
    if (valueClass() != nullptr) {
      retval += folly::format("(OfClass {})",
                valueClass()->name()->data()).str();
    } else if (hasKnownClass()) {
      retval += folly::format("(Known Class {})",
                knownClass()->name()->data()).str();
    }
  }
  if (valueType() == KindOfClass && valueClass() != nullptr) {
    retval += folly::format("(Class {})",
              valueClass()->name()->data()).str();
  }
  if (valueType() == KindOfArray && hasArrayKind()) {
    retval += folly::format("(Kind {})",
                            ArrayData::kindToString(arrayKind())).str();
  }
  return retval;
}

std::string Location::pretty() const {
  char buf[1024];
  sprintf(buf, "(Location %s %" PRId64 ")", spaceName(), offset);
  return std::string(buf);
}

} }
