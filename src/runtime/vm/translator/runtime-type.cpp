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

#include <assert.h>
#include <stdint.h>
#include <stdarg.h>
#include "runtime/vm/bytecode.h"
#include "runtime/base/types.h"
#include "runtime-type.h"
#include "translator.h"

namespace HPHP {
namespace VM {
namespace Transl {

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

RuntimeType::RuntimeType(DataType outer, DataType inner /* = KindOfInvalid */,
                         const Class* klass /* = NULL */)
  : m_kind(VALUE) {
  m_value.outerType = normalizeDataType(outer);
  m_value.innerType = normalizeDataType(inner);
  m_value.klass = klass;
  consistencyCheck();
}

RuntimeType::RuntimeType(const StringData* sd)
  : m_kind(VALUE) {
  m_value.outerType = KindOfString;
  m_value.innerType = KindOfInvalid;
  m_value.string = sd;
  consistencyCheck();
}

RuntimeType::RuntimeType(const ArrayData* ad)
  : m_kind(VALUE) {
  m_value.outerType = KindOfArray;
  m_value.innerType = KindOfInvalid;
  m_value.array = ad;
  consistencyCheck();
}

RuntimeType::RuntimeType(bool value)
  : m_kind(VALUE) {
  m_value.outerType = KindOfBoolean;
  m_value.innerType = KindOfInvalid;
  m_value.klass = NULL;
  m_value.boolean = value;
  m_value.boolValid = true;
  consistencyCheck();
}

RuntimeType::RuntimeType(int64 value)
  : m_kind(VALUE) {
  m_value.outerType = KindOfInt64;
  m_value.innerType = KindOfInvalid;
  m_value.intval = value;
  consistencyCheck();
}

RuntimeType::RuntimeType(const Class* klass)
  : m_kind(VALUE) {
  m_value.outerType = KindOfClass;
  m_value.innerType = KindOfInvalid;
  m_value.klass = klass;
  consistencyCheck();
}

RuntimeType::RuntimeType(const RuntimeType& source) {
  *this = source;
}

RuntimeType::RuntimeType() :
  m_kind(VALUE) {
  m_value.outerType = KindOfInvalid;
  m_value.innerType = KindOfInvalid;
  m_value.klass = NULL;
}

RuntimeType::RuntimeType(const Iter* it) :
  m_kind(ITER) {
  m_iter.type = it->m_itype;
}

RuntimeType::RuntimeType(Iter::Type type) :
  m_kind(ITER) {
  m_iter.type = type;
}

RuntimeType RuntimeType::box() const {
  ASSERT(m_kind == VALUE);
  if (m_value.outerType == KindOfRef) {
    consistencyCheck();
    return *this;
  }
  RuntimeType rtt;
  rtt.m_value.outerType = KindOfRef;
  rtt.m_value.innerType = m_value.outerType;
  rtt.consistencyCheck();
  return rtt;
}

RuntimeType RuntimeType::unbox() const {
  ASSERT(m_kind == VALUE);
  if (m_value.outerType != KindOfRef) {
    consistencyCheck();
    return *this;
  }
  RuntimeType rtt;
  rtt.m_value.outerType = m_value.innerType;
  rtt.m_value.innerType = KindOfInvalid;
  rtt.consistencyCheck();
  return rtt;
}

DataType RuntimeType::valueType() const {
  ASSERT(m_kind != ITER);
  if (outerType() == KindOfRef) {
    return m_value.innerType;
  }
  return m_value.outerType;
}

const Class*
RuntimeType::valueClass() const {
  consistencyCheck();
  ASSERT(m_kind != ITER);
  ASSERT(valueType() == KindOfObject || valueType() == KindOfClass);
  return m_value.klass;
}

const StringData*
RuntimeType::valueString() const {
  consistencyCheck();
  ASSERT(m_kind != ITER);
  ASSERT(isString());
  return m_value.string;
}

const StringData*
RuntimeType::valueStringOrNull() const {
  if (!isString()) return NULL;
  return valueString();
}

const ArrayData*
RuntimeType::valueArray() const {
  consistencyCheck();
  ASSERT(m_kind != ITER);
  ASSERT(isArray());
  return m_value.array;
}

// -1 for unknown, 0 for false, 1 for true
int
RuntimeType::valueBoolean() const {
  consistencyCheck();
  ASSERT(m_kind != ITER);
  ASSERT(isBoolean());
  return m_value.boolValid ? m_value.boolean : -1;
}

int64
RuntimeType::valueInt() const {
  consistencyCheck();
  ASSERT(m_kind == VALUE);
  ASSERT(isInt());
  return m_value.intval;
}

// Get the value as a blob. Use with care.
int64
RuntimeType::valueGeneric() const {
  consistencyCheck();
  ASSERT(m_kind == VALUE);
  return m_value.intval;
}

RuntimeType
RuntimeType::setValueType(DataType newInner) const {
  ASSERT(m_kind == VALUE);
  RuntimeType rtt;
  rtt.m_kind = VALUE;
  rtt.m_value.outerType = outerType();
  if (outerType() == KindOfRef) {
    rtt.m_value.innerType = newInner;
  } else {
    rtt.m_value.outerType = newInner;
  }
  ASSERT(rtt.valueType() == newInner);
  rtt.m_value.klass = NULL;
  rtt.consistencyCheck();
  return rtt;
}

// Accessors
DataType RuntimeType::outerType() const {
  consistencyCheck();
  ASSERT(m_kind == VALUE);
  return m_value.outerType;
}

DataType RuntimeType::innerType() const {
  consistencyCheck();
  ASSERT(m_kind == VALUE);
  return m_value.innerType;
}

bool RuntimeType::isValue() const {
  consistencyCheck();
  return m_kind == VALUE;
}

Iter::Type RuntimeType::iterType() const {
  ASSERT(isIter());
  return m_iter.type;
}

int RuntimeType::typeCheckOffset() const {
  if (isIter()) return offsetof(Iter, m_itype);
  return offsetof(Cell, m_type);
}

DataType RuntimeType::typeCheckValue() const {
  if (isIter()) return DataType(m_iter.type);
  return outerType();
}

bool RuntimeType::isIter() const {
  consistencyCheck();
  return m_kind == ITER;
}

bool RuntimeType::isVariant() const {
  ASSERT(m_kind == VALUE);
  return outerType() == KindOfRef;
}

bool RuntimeType::isVagueValue() const {
  return m_kind == VALUE && outerType() == KindOfInvalid;
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
      return r.m_iter.type == m_iter.type;
    case VALUE:
      return r.m_value.innerType == m_value.innerType &&
             r.m_value.outerType == m_value.outerType &&
             r.m_value.klass == m_value.klass;
    default:
      ASSERT(false);
      return false;
  }
}

RuntimeType& RuntimeType::operator=(const RuntimeType& r) {
  m_kind            = r.m_kind;
  m_iter            = r.m_iter;
  m_value.innerType = r.m_value.innerType;
  m_value.outerType = r.m_value.outerType;
  m_value.klass     = r.m_value.klass;
  consistencyCheck();
  ASSERT(*this == r);
  return *this;
}

size_t
RuntimeType::operator()(const RuntimeType& r) const {
  uint64 p1 = HPHP::hash_int64(m_kind);
  uint64 p2 = 0;
  // We can't just hash the whole blob of memory, because
  // C++ will leave padding uninitialized. The shifts are to
  // make the final hash order-dependent, so that
  //    { field1: 0, field2: 1 }
  // has a different hash than
  //    { field1: 1, field2: 0 }
  switch(m_kind) {
    case ITER:
      p2 = HPHP::hash_int64(m_iter.type);
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
    sprintf(buf, "(Iter %s)",
            m_iter.type == Iter::TypeMutableArray ? "mutableArray" : "array");
    return std::string(buf);
  }
  if (m_value.outerType == KindOfRef) {
    sprintf(buf, "(Value (Var %s))", tname(m_value.innerType).c_str());
  } else {
    sprintf(buf, "(Value %s)", tname(m_value.outerType).c_str());
  }
  string retval = buf;
  if (valueType() == KindOfObject && valueClass() != NULL) {
    char buf2[1024];
    sprintf(buf2, "(OfClass %s)", valueClass()->preClass()->name()->data());
    retval += string(buf2);
  }
  if (valueType() == KindOfClass && valueClass() != NULL) {
    char buf2[1024];
    sprintf(buf2, "(Class %s)", valueClass()->preClass()->name()->data());
    retval += string(buf2);
  }
  return retval;
}


} } }
