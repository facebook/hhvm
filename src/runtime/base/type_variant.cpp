/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/complex_types.h>
#include <runtime/base/comparisons.h>
#include <runtime/base/zend/zend_functions.h>
#include <system/gen/php/classes/stdclass.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/variable_unserializer.h>
#include <runtime/base/externals.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/fiber_reference_map.h>
#include <runtime/base/zend/zend_string.h>
#include <compiler/parser/hphp.tab.hpp>

using namespace std;

namespace HPHP {

const Variant null_variant = Variant();

IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(Variant);

///////////////////////////////////////////////////////////////////////////////
// static strings

static StaticString s_offsetGet("offsetGet");
static StaticString s_offsetSet("offsetSet");
static StaticString s_offsetUnset("offsetUnset");
static StaticString s_s("s");
static StaticString s_scalar("scalar");

///////////////////////////////////////////////////////////////////////////////
// private implementations

Variant::Variant(litstr  v) : _count(0), m_type(KindOfString) {
  m_data.pstr = NEW(StringData)(v);
  m_data.pstr->incRefCount();
}

Variant::Variant(CStrRef v) : _count(0), m_type(KindOfString) {
  StringData *s = v.get();
  if (s) {
    m_data.pstr = s;
    s->incRefCount();
  } else {
    m_data.num = 0;
    m_type = KindOfNull;
  }
}

Variant::Variant(const std::string & v) : _count(0), m_type(KindOfString) {
  StringData *s = NEW(StringData)(v.c_str(), v.size(), CopyString);
  ASSERT(s);
  m_data.pstr = s;
  s->incRefCount();
}

Variant::Variant(const StaticString & v) :
  _count(0), m_type(KindOfStaticString) {
  StringData *s = v.get();
  if (s) {
    m_data.pstr = s;
  } else {
    m_data.num = 0;
    m_type = KindOfNull;
  }
}

Variant::Variant(CArrRef v) : _count(0), m_type(KindOfArray) {
  ArrayData *a = v.get();
  if (a) {
    m_data.parr = a;
    a->incRefCount();
  } else {
    m_data.num = 0;
    m_type = KindOfNull;
  }
}

Variant::Variant(CObjRef v) : _count(0), m_type(KindOfObject) {
  ObjectData *o = v.get();
  if (o) {
    m_data.pobj = o;
    o->incRefCount();
  } else {
    m_data.num = 0;
    m_type = KindOfNull;
  }
}

Variant::Variant(StringData *v) : _count(0), m_type(KindOfString) {
  if (v) {
    m_data.pstr = v;
    v->incRefCount();
  } else {
    m_data.num = 0;
    m_type = KindOfNull;
  }
}

Variant::Variant(ArrayData *v) : _count(0), m_type(KindOfArray) {
  if (v) {
    m_data.parr = v;
    v->incRefCount();
  } else {
    m_data.num = 0;
    m_type = KindOfNull;
  }
}

Variant::Variant(ObjectData *v) : _count(0), m_type(KindOfObject) {
  if (v) {
    m_data.pobj = v;
    v->incRefCount();
  } else {
    m_data.num = 0;
    m_type = KindOfNull;
  }
}

Variant::Variant(Variant *v) : _count(0), m_type(KindOfVariant) {
  if (v) {
    m_data.pvar = v;
  } else {
    m_data.num = 0;
    m_type = KindOfNull;
  }
}

Variant::Variant(CVarRef v) : _count(0), m_type(KindOfNull) {
  m_data.num = 0;
  if (v.isContagious()) {
    assignContagious(v);
    return;
  }
  bind(v);
}

void Variant::reset() {
  m_data.num = 0;
  m_type = KindOfNull;
}

void Variant::destruct() {
  ASSERT(!isPrimitive());
#ifdef FAST_REFCOUNT_FOR_VARIANT
  /**
   * This is safe because we have compile time assertions that guarantee that
   * the _count field will always be exactly FAST_REFCOUNT_OFFSET bytes from
   * the beginning of the object for the StringData, ArrayData, ObjectData,
   * and Variant classes.
   */
  if (m_data.pvar->decRefCount() == 0) {
    switch (m_type) {
    case KindOfString:
      m_data.pstr->release();
      break;
    case KindOfArray:
      m_data.parr->release();
      break;
    case KindOfObject:
      m_data.pobj->release();
      break;
    case KindOfVariant:
      m_data.pvar->release();
      break;
    default:
      ASSERT(false);
      break;
    }
  }
#else
  switch (m_type) {
  case KindOfString:
    if (m_data.pstr->decRefCount() == 0) {
      m_data.pstr->release();
    }
    break;
  case KindOfArray:
    if (m_data.parr->decRefCount() == 0) {
      m_data.parr->release();
    }
    break;
  case KindOfObject:
    if (m_data.pobj->decRefCount() == 0) {
      m_data.pobj->release();
    }
    break;
  case KindOfVariant:
    if (m_data.pvar->decRefCount() == 0) {
      m_data.pvar->release();
    }
    break;
  default:
    ASSERT(false);
    break;
  }
#endif
}

Variant& Variant::assign(CVarRef v) {
  // otherwise our code generation is wrong
  ASSERT(!isContagious() || this == &v);
  if (v.isContagious()) {
    assignContagious(v);
    return *this;
  }
  if (m_type == KindOfVariant) {
    Variant * innerVar = m_data.pvar;
    if (innerVar->getCount() > 1) {
      if (!v.isReferenced() || v.getVariantData() != innerVar) {
        innerVar->assign(v);
      }
    } else if (this != &v) {
      // We need to release whatever value innerVar holds before
      // calling bind()
      if (IS_REFCOUNTED_TYPE(innerVar->m_type)) innerVar->destruct();
      innerVar->bind(v);
    }
    return *this;
  }
  if (this == &v) {
    return *this;
  }
  if (IS_REFCOUNTED_TYPE(m_type)) destruct();
  bind(v);
  return *this;
}

void Variant::setNull() {
  if (isPrimitive()) {
    m_data.num = 0;
    m_type = KindOfNull;
  } else if (m_type == KindOfVariant) {
    m_data.pvar->setNull();
  } else {
    unset();
  }
}

CVarRef Variant::set(bool v) {
  if (isPrimitive()) {
    // do nothing
  } else if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  } else {
    if (IS_REFCOUNTED_TYPE(m_type)) destruct();
  }
  m_type = KindOfBoolean;
  m_data.num = (v ? 1 : 0);
  return *this;
}

CVarRef Variant::set(char v) {
  if (isPrimitive()) {
    // do nothing
  } else if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  } else {
    if (IS_REFCOUNTED_TYPE(m_type)) destruct();
  }
  m_type = KindOfByte;
  m_data.num = v;
  return *this;
}

CVarRef Variant::set(short v) {
  if (isPrimitive()) {
    // do nothing
  } else if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  } else {
    if (IS_REFCOUNTED_TYPE(m_type)) destruct();
  }
  m_type = KindOfInt16;
  m_data.num = v;
  return *this;
}

CVarRef Variant::set(int v) {
  if (isPrimitive()) {
    // do nothing
  } else if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  } else {
    if (IS_REFCOUNTED_TYPE(m_type)) destruct();
  }
  m_type = KindOfInt32;
  m_data.num = v;
  return *this;
}

CVarRef Variant::set(int64 v) {
  if (isPrimitive()) {
    // do nothing
  } else if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  } else {
    if (IS_REFCOUNTED_TYPE(m_type)) destruct();
  }
  m_type = KindOfInt64;
  m_data.num = v;
  return *this;
}

CVarRef Variant::set(double v) {
  if (isPrimitive()) {
    // do nothing
  } else if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  } else {
    if (IS_REFCOUNTED_TYPE(m_type)) destruct();
  }
  m_type = KindOfDouble;
  m_data.dbl = v;
  return *this;
}

CVarRef Variant::set(litstr v) {
  if (isPrimitive()) {
    // do nothing
  } else if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  } else {
    if (IS_REFCOUNTED_TYPE(m_type)) destruct();
  }
  m_type = KindOfString;
  m_data.pstr = NEW(StringData)(v);
  m_data.pstr->incRefCount();
  return *this;
}

CVarRef Variant::set(StringData *v) {
  if (isPrimitive()) {
    // do nothing
  } else if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  } else {
    if (IS_REFCOUNTED_TYPE(m_type)) destruct();
  }
  if (v) {
    m_type = KindOfString;
    m_data.pstr = v;
    v->incRefCount();
  } else {
    m_data.num = 0;
    m_type = KindOfNull;
  }
  return *this;
}

CVarRef Variant::set(ArrayData *v) {
  if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  }
  if (IS_REFCOUNTED_TYPE(m_type)) destruct();
  if (v) {
    m_type = KindOfArray;
    m_data.parr = v;
    v->incRefCount();
  } else {
    m_data.num = 0;
    m_type = KindOfNull;
  }
  return *this;
}

CVarRef Variant::set(ObjectData *v) {
  if (isPrimitive()) {
    // do nothing
  } else if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  } else {
    if (IS_REFCOUNTED_TYPE(m_type)) destruct();
  }
  if (v) {
    m_type = KindOfObject;
    m_data.pobj = v;
    v->incRefCount();
  } else {
    m_data.num = 0;
    m_type = KindOfNull;
  }
  return *this;
}

void Variant::split() {
  switch (m_type) {
  case KindOfVariant: m_data.pvar->split();     break;
  // copy-on-write
  case KindOfStaticString:
  case KindOfString:
  {
    int len = m_data.pstr->size();
    const char *copy = string_duplicate(m_data.pstr->data(), len);
    set(NEW(StringData)(copy, len, AttachString));
    break;
  }
  case KindOfArray:   set(m_data.parr->copy()); break;
  default:
    break;
  }
}

int Variant::getRefCount() const {
  switch (m_type) {
  case KindOfString:  return m_data.pstr->getCount();
  case KindOfArray:   return m_data.parr->getCount();
  case KindOfObject:  return m_data.pobj->getCount();
  case KindOfVariant: return m_data.pvar->getRefCount();
  default:
    break;
  }
  return 1;
}

///////////////////////////////////////////////////////////////////////////////
// informational

bool Variant::isInteger() const {
  switch (m_type) {
    case KindOfByte:
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
      return true;
    case KindOfVariant:
      return m_data.pvar->isInteger();
    default:
      break;
  }
  return false;
}

bool Variant::isNumeric(bool checkString /* = false */) const {
  int64 ival;
  double dval;
  DataType t = toNumeric(ival, dval, checkString);
  return t == KindOfInt64 || t == KindOfDouble;
}

DataType Variant::toNumeric(int64 &ival, double &dval,
    bool checkString /* = false */) const {
  switch (m_type) {
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    ival = m_data.num;
    return KindOfInt64;
  case KindOfDouble:
    dval = m_data.dbl;
    return KindOfDouble;
  case KindOfStaticString:
  case KindOfString:
    if (checkString) {
      return m_data.pstr->toNumeric(ival, dval);
    }
    break;
  case KindOfVariant:
    return m_data.pvar->toNumeric(ival, dval, checkString);
  default:
    break;
  }
  return m_type;
}

bool Variant::isScalar() const {
  switch (getType()) {
  case KindOfNull:
  case KindOfArray:
  case KindOfObject:
    return false;
  default:
    break;
  }
  return true;
}

bool Variant::isResource() const {
  if (is(KindOfObject)) {
    return toObject()->isResource();
  }
  return false;
}

bool Variant::instanceof(const char *s) const {
  if (m_type == KindOfObject) {
    return toObject().instanceof(s);
  }
  if (m_type == KindOfVariant) {
    return m_data.pvar->instanceof(s);
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// array operations

Variant Variant::pop() {
  if (m_type == KindOfVariant) {
    return m_data.pvar->pop();
  }
  if (!is(KindOfArray)) {
    throw_bad_type_exception("expecting an array");
    return null_variant;
  }

  Variant ret;
  ArrayData *newarr = getArrayData()->pop(ret);
  if (newarr) {
    set(newarr);
  }
  return ret;
}

Variant Variant::dequeue() {
  if (m_type == KindOfVariant) {
    return m_data.pvar->dequeue();
  }
  if (!is(KindOfArray)) {
    throw_bad_type_exception("expecting an array");
    return null_variant;
  }

  Variant ret;
  ArrayData *newarr = getArrayData()->dequeue(ret);
  if (newarr) {
    set(newarr);
  }
  return ret;
}

void Variant::prepend(CVarRef v) {
  if (m_type == KindOfVariant) {
    m_data.pvar->prepend(v);
    return;
  }
  if (isNull()) {
    set(Array::Create());
  }

  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    ArrayData *newarr = arr->prepend(v, (arr->getCount() > 1));
    if (newarr) {
      set(newarr);
    }
  } else {
    throw_bad_type_exception("expecting an array");
  }
}

Variant Variant::array_iter_reset() {
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1) {
      arr = arr->copy();
      set(arr);
      ASSERT(arr == getArrayData());
    }
    return arr->reset();
  }
  throw_bad_type_exception("expecting an array");
  return false;
}

Variant Variant::array_iter_prev() {
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1) {
      arr = arr->copy();
      set(arr);
      ASSERT(arr == getArrayData());
    }
    return arr->prev();
  }
  throw_bad_type_exception("expecting an array");
  return false;
}

Variant Variant::array_iter_current() const {
  if (is(KindOfArray)) {
    return getArrayData()->current();
  }
  throw_bad_type_exception("expecting an array");
  return false;
}

Variant Variant::array_iter_next() {
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1) {
      arr = arr->copy();
      set(arr);
      ASSERT(arr == getArrayData());
    }
    return arr->next();
  }
  throw_bad_type_exception("expecting an array");
  return false;
}

Variant Variant::array_iter_end() {
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1) {
      arr = arr->copy();
      set(arr);
      ASSERT(arr == getArrayData());
    }
    return arr->end();
  }
  throw_bad_type_exception("expecting an array");
  return false;
}

Variant Variant::array_iter_key() const {
  if (is(KindOfArray)) {
    return getArrayData()->key();
  }
  throw_bad_type_exception("expecting an array");
  return false;
}

Variant Variant::array_iter_each() {
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1) {
      arr = arr->copy();
      set(arr);
      ASSERT(arr == getArrayData());
    }
    return arr->each();
  }
  throw_bad_type_exception("expecting an array");
  return null_variant;
}

///////////////////////////////////////////////////////////////////////////////
// unary plus
Variant Variant::operator+() const {
  if (is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (isDouble()) {
    return toDouble();
  }
  if (isIntVal()) {
    return toInt64();
  }
  if (isString()) {
    String s = toString();
    DataType ret = KindOfNull;
    int64 lval; double dval;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      return dval;
    }
    if (ret == KindOfInt64) {
      return lval;
    }
    return toInt64();
  }

  ASSERT(false);
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// add or array append

Variant Variant::operator+(CVarRef var) const {
  if (m_type == KindOfInt64 && var.m_type == KindOfInt64) {
    return m_data.num + var.m_data.num;
  }
  if (isIntVal() && var.isIntVal()) {
    return toInt64() + var.toInt64();
  }
  if (is(KindOfArray) && var.is(KindOfArray)) {
    return toArray() + var.toArray();
  }
  if (is(KindOfArray) || var.is(KindOfArray)) {
    throw BadArrayMergeException();
  }
  if (isDouble() || var.isDouble()) {
    return toDouble() + var.toDouble();
  }
  if (isString()) {
    String s = toString();
    DataType ret = KindOfNull;
    int64 lval; double dval;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      return dval + var.toDouble();
    }
  }
  if (var.isString()) {
    String s = var.toString();
    DataType ret = KindOfNull;
    int64 lval; double dval;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      return toDouble() + dval;
    }
  }
  return toInt64() + var.toInt64();
}

Variant &Variant::operator+=(CVarRef var) {
  if (m_type == KindOfInt64 && var.m_type == KindOfInt64) {
    m_data.num += var.m_data.num;
    return *this;
  }
  if (isIntVal() && var.isIntVal()) {
    set(toInt64() + var.toInt64());
    return *this;
  }
  if (is(KindOfArray) && var.is(KindOfArray)) {
    set(toArray() + var.toArray());
    return *this;
  }
  if (is(KindOfArray) || var.is(KindOfArray)) {
    throw BadArrayMergeException();
  }
  if (isDouble() || var.isDouble()) {
    set(toDouble() + var.toDouble());
    return *this;
  }
  if (isString()) {
    String s = toString();
    DataType ret = KindOfNull;
    int64 lval; double dval;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      set(dval + var.toDouble());
      return *this;
    }
  }
  if (var.isString()) {
    String s = var.toString();
    DataType ret = KindOfNull;
    int64 lval; double dval;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      set(toDouble() + dval);
      return *this;
    }
  }
  set(toInt64() + var.toInt64());
  return *this;
}

Variant &Variant::operator+=(int64 n) {
  if (m_type == KindOfInt64) {
    m_data.num += n;
    return *this;
  }
  if (isIntVal()) {
    set(toInt64() + n);
    return *this;
  }
  if (isDouble()) {
    set(toDouble() + n);
    return *this;
  }
  if (is(KindOfArray)) {
    throw BadArrayMergeException();
  }
  if (isString()) {
    String s = toString();
    DataType ret = KindOfNull;
    int64 lval; double dval;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      set(dval + n);
      return *this;
    }
  } else {
    ASSERT(false);
  }
  set(toInt64() + n);
  return *this;
}

Variant &Variant::operator+=(double n) {
  if (is(KindOfArray)) {
    throw BadArrayMergeException();
  }
  set(toDouble() + n);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// minus

Variant Variant::operator-() const {
  if (is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (isDouble()) {
    return -toDouble();
  } else if (isIntVal()) {
    return -toInt64();
  } else {
    if (isString()) {
      String s = toString();
      DataType ret = KindOfNull;
      int64 lval; double dval;
      ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
      if (ret == KindOfDouble) {
        return -dval;
      } else if (ret == KindOfInt64) {
        return -lval;
      } else {
        return -toInt64();
      }
    } else {
      ASSERT(false);
    }
  }
  return *this;
}

Variant Variant::operator-(CVarRef var) const {
  if (is(KindOfArray) || var.is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (isDouble() || var.isDouble()) {
    return toDouble() - var.toDouble();
  }
  if (isIntVal() && var.isIntVal()) {
    return toInt64() - var.toInt64();
  }
  if (isString()) {
    String s = toString();
    DataType ret = KindOfNull;
    int64 lval; double dval;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      return dval - var.toDouble();
    }
  }
  if (var.isString()) {
    String s = var.toString();
    DataType ret = KindOfNull;
    int64 lval; double dval;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      return toDouble() - dval;
    }
  }
  return toInt64() - var.toInt64();
}

Variant &Variant::operator-=(CVarRef var) {
  if (is(KindOfArray) || var.is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (isDouble() || var.isDouble()) {
    set(toDouble() - var.toDouble());
  } else if (isIntVal() && var.isIntVal()) {
    set(toInt64() - var.toInt64());
  } else {
    if (isString()) {
      String s = toString();
      DataType ret = KindOfNull;
      int64 lval; double dval;
      ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
      if (ret == KindOfDouble) {
        set(dval - var.toDouble());
        return *this;
      }
    }
    if (var.isString()) {
      String s = var.toString();
      DataType ret = KindOfNull;
      int64 lval; double dval;
      ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
      if (ret == KindOfDouble) {
        set(toDouble() - dval);
        return *this;
      }
    }
    set(toInt64() - var.toInt64());
  }
  return *this;
}

Variant &Variant::operator-=(int64 n) {
  if (is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (isDouble()) {
    set(toDouble() - n);
  } else if (isIntVal()) {
    set(toInt64() - n);
  } else {
    if (isString()) {
      String s = toString();
      DataType ret = KindOfNull;
      int64 lval; double dval;
      ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
      if (ret == KindOfDouble) {
        set(dval - n);
        return *this;
      }
    } else {
      ASSERT(false);
    }
    set(toInt64() - n);
  }
  return *this;
}

Variant &Variant::operator-=(double n) {
  if (is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  set(toDouble() - n);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// multiply

Variant Variant::operator*(CVarRef var) const {
  if (is(KindOfArray) || var.is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (isDouble() || var.isDouble()) {
    return toDouble() * var.toDouble();
  }
  if (isIntVal() && var.isIntVal()) {
    return toInt64() * var.toInt64();
  }
  if (isString()) {
    String s = toString();
    DataType ret = KindOfNull;
    int64 lval; double dval;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      return dval * var.toDouble();
    }
  }
  if (var.isString()) {
    String s = var.toString();
    DataType ret = KindOfNull;
    int64 lval; double dval;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      return toDouble() * dval;
    }
  }
  return toInt64() * var.toInt64();
}

Variant &Variant::operator*=(CVarRef var) {
  if (is(KindOfArray) || var.is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (isDouble() || var.isDouble()) {
    set(toDouble() * var.toDouble());
  } else if (isIntVal() && var.isIntVal()) {
    set(toInt64() * var.toInt64());
  } else {
    if (isString()) {
      String s = toString();
      DataType ret = KindOfNull;
      int64 lval; double dval;
      ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
      if (ret == KindOfDouble) {
        set(dval * var.toDouble());
        return *this;
      }
    }
    if (var.isString()) {
      String s = var.toString();
      DataType ret = KindOfNull;
      int64 lval; double dval;
      ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
      if (ret == KindOfDouble) {
        set(toDouble() * dval);
        return *this;
      }
    }
    set(toInt64() * var.toInt64());
  }
  return *this;
}

Variant &Variant::operator*=(int64 n) {
  if (is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (isDouble()) {
    set(toDouble() * n);
  } else if (isIntVal()) {
    set(toInt64() * n);
  } else {
    if (isString()) {
      String s = toString();
      DataType ret = KindOfNull;
      int64 lval; double dval;
      ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
      if (ret == KindOfDouble) {
        set(dval * n);
        return *this;
      }
    } else {
      ASSERT(false);
    }
    set(toInt64() * n);
  }
  return *this;
}

Variant &Variant::operator*=(double n) {
  if (is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  set(toDouble() * n);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// divide

Variant Variant::operator/(CVarRef var) const {
  if (is(KindOfArray) || var.is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  int64 lval; double dval; bool int1 = true;
  int64 lval2; double dval2; bool int2 = true;

  if (isDouble()) {
    dval = toDouble();
    int1 = false;
  } else if (isIntVal()) {
    lval = toInt64();
  } else if (isString()) {
    String s = toString();
    DataType ret = KindOfNull;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      int1 = false;
    } else if (ret != KindOfInt64) {
      lval = 0;
    }
  } else {
    ASSERT(false);
  }
  if (var.isDouble()) {
    dval2 = var.toDouble();
    int2 = false;
  } else if (var.isIntVal()) {
    lval2 = var.toInt64();
  } else if (var.isString()) {
    String s = var.toString();
    DataType ret = KindOfNull;
    ret = is_numeric_string(s.data(), s.size(), &lval2, &dval2, 1);
    if (ret == KindOfDouble) {
      int2 = false;
    } else if (ret != KindOfInt64) {
      lval2 = 0;
    }
  } else {
    ASSERT(false);
  }

  if ((int2 && lval2 == 0) || (!int2 && dval2 == 0)) {
    raise_warning("Division by zero");
    return false;
  }

  if (int1 && int2) {
    if (lval % lval2 == 0) {
      return lval / lval2;
    } else {
      return (double)lval / lval2;
    }
  } else if (int1 && !int2) {
    return lval / dval2;
  } else if (!int1 && int2) {
    return dval / lval2;
  } else {
    return dval / dval2;
  }
}

Variant &Variant::operator/=(CVarRef var) {
  if (is(KindOfArray) || var.is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  int64 lval; double dval; bool int1 = true;
  int64 lval2; double dval2; bool int2 = true;

  if (isDouble()) {
    dval = toDouble();
    int1 = false;
  } else if (isIntVal()) {
    lval = toInt64();
  } else if (isString()) {
    String s = toString();
    DataType ret = KindOfNull;
    ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
    if (ret == KindOfDouble) {
      int1 = false;
    } else if (ret != KindOfInt64) {
      lval = 0;
    }
  } else {
    ASSERT(false);
  }
  if (var.isDouble()) {
    dval2 = var.toDouble();
    int2 = false;
  } else if (var.isIntVal()) {
    lval2 = var.toInt64();
  } else if (var.isString()) {
    String s = var.toString();
    DataType ret = KindOfNull;
    ret = is_numeric_string(s.data(), s.size(), &lval2, &dval2, 1);
    if (ret == KindOfDouble) {
      int2 = false;
    } else if (ret != KindOfInt64) {
      lval2 = 0;
    }
  } else {
    ASSERT(false);
  }

  if ((int2 && lval2 == 0) || (!int2 && dval2 == 0)) {
    raise_warning("Division by zero");
    set(false);
    return *this;
  }

  if (int1 && int2) {
    if (lval % lval2 == 0) {
      set(lval / lval2);
    } else {
      set((double)lval / lval2);
    }
  } else if (int1 && !int2) {
    set(lval / dval2);
  } else if (!int1 && int2) {
    set(dval / lval2);
  } else {
    set(dval / dval2);
  }
  return *this;
}

Variant &Variant::operator/=(int64 n) {
  if (is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (n == 0) {
    raise_warning("Division by zero");
    set(false);
    return *this;
  }

  if (isIntVal() && toInt64() % n == 0) {
    set(toInt64() / n);
  } else if (isDouble()) {
    set(toDouble() / n);
  } else {
    if (isString()) {
      String s = toString();
      DataType ret = KindOfNull;
      int64 lval; double dval;
      ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
      if (ret == KindOfInt64 && lval % n == 0) {
        set(lval / n);
        return *this;
      }
    }
    set(toDouble() / n);
  }
  return *this;
}

Variant &Variant::operator/=(double n) {
  if (is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (n == 0.0) {
    raise_warning("Division by zero");
    set(false);
    return *this;
  }
  set(toDouble() / n);
  return *this;
}

double operator/(double n, CVarRef v) {
  if (v.is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  double dval = v.toDouble();
  if (dval == 0.0) {
    raise_warning("Division by zero");
    return false;
  }
  return n / dval;
}

double operator/(int n, CVarRef v) {
  if (v.is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  double dval = v.toDouble();
  if (dval == 0.0) {
    raise_warning("Division by zero");
    return false;
  }
  return n / dval;
}

///////////////////////////////////////////////////////////////////////////////
// modulus

int64 Variant::operator%(CVarRef var) const {
  int64 lval = toInt64();
  int64 lval2 = var.toInt64();
  if (lval2 == 0) {
    raise_warning("Division by zero");
    return false;
  }
  return lval % lval2;
}

Variant &Variant::operator%=(CVarRef var) {
  int64 lval = toInt64();
  int64 lval2 = var.toInt64();
  if (lval2 == 0) {
    raise_warning("Division by zero");
    set(false);
    return *this;
  }
  set(lval % lval2);
  return *this;
}

Variant &Variant::operator%=(int64 n) {
  if (n == 0) {
    raise_warning("Division by zero");
    set(false);
    return *this;
  }
  set(toInt64() % n);
  return *this;
}

Variant &Variant::operator%=(double n) {
  if ((int64)n == 0) {
    raise_warning("Division by zero");
    set(false);
    return *this;
  }
  set(toInt64() % (int64)n);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// bitwise

Variant Variant::operator~() const {
  switch (getType()) {
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    return ~toInt64();
  case KindOfDouble:
    return ~(int64)(toDouble());
  case KindOfStaticString:
  case KindOfString:
    return ~toString();
  default:
    break;
  }
  throw InvalidOperandException("only numerics and strings can be negated");
}

Variant Variant::operator|(CVarRef v) const {
  if (isString() && v.isString()) {
    return toString() | v.toString();
  }
  return toInt64() | v.toInt64();
}

Variant Variant::operator&(CVarRef v) const {
  if (isString() && v.isString()) {
    return toString() & v.toString();
  }
  return toInt64() & v.toInt64();
}

Variant Variant::operator^(CVarRef v) const {
  if (isString() && v.isString()) {
    return toString() ^ v.toString();
  }
  return toInt64() ^ v.toInt64();
}

Variant &Variant::operator|=(CVarRef v) {
  if (isString() && v.isString()) {
    set(toString() | v.toString());
  } else {
    set(toInt64() | v.toInt64());
  }
  return *this;
}

Variant &Variant::operator&=(CVarRef v) {
  if (isString() && v.isString()) {
    set(toString() & v.toString());
  } else {
    set(toInt64() & v.toInt64());
  }
  return *this;
}

Variant &Variant::operator^=(CVarRef v) {
  if (isString() && v.isString()) {
    set(toString() ^ v.toString());
  } else {
    set(toInt64() ^ v.toInt64());
  }
  return *this;
}

Variant &Variant::operator<<=(int64 n) {
  set(toInt64() << n);
  return *this;
}

Variant &Variant::operator>>=(int64 n) {
  set(toInt64() >> n);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// increment/decrement

Variant &Variant::operator++() {
  switch (getType()) {
  case KindOfNull:   set(1LL); break;
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:  set(toInt64() + 1);  break;
  case KindOfDouble: set(toDouble() + 1); break;
  case KindOfStaticString:
  case KindOfString:
    {
      String s = toString();
      if (s.empty()) {
        set(1LL);
      } else {
        int64 lval; double dval;
        DataType ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
        switch (ret) {
        case KindOfInt64:  set(lval + 1); break;
        case KindOfDouble: set(dval + 1); break;
        case KindOfNull:
          split();
          getStringData()->inc(); break;
        default:
          ASSERT(false);
          break;
        }
      }
    }
    break;
  default:
    break;
  }
  return *this;
}

Variant Variant::operator++(int) {
  Variant ret(*this);
  operator++();
  return ret;
}

Variant &Variant::operator--() {
  switch (getType()) {
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:  set(toInt64() - 1);  break;
  case KindOfDouble: set(toDouble() - 1); break;
  case KindOfStaticString:
  case KindOfString:
    {
      String s = toString();
      if (s.empty()) {
        set(-1LL);
      } else {
        int64 lval; double dval;
        DataType ret = is_numeric_string(s.data(), s.size(), &lval, &dval, 1);
        switch (ret) {
        case KindOfInt64:  set(lval - 1);   break;
        case KindOfDouble: set(dval - 1);   break;
        case KindOfNull:   /* do nothing */ break;
        default:
          ASSERT(false);
          break;
        }
      }
    }
    break;
  default:
    break;
  }
  return *this;
}

Variant Variant::operator--(int) {
  Variant ret(*this);
  operator--();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// iterator functions

ArrayIterPtr Variant::begin(CStrRef context /* = null_string */) const {
  if (is(KindOfArray)) {
    return new ArrayIter(getArrayData());
  }
  if (is(KindOfObject)) {
    ObjectData *obj = getObjectData();
    if (obj->o_instanceof("Iterator")) {
      return new ObjectArrayIter(obj);
    }
    while (obj->o_instanceof("IteratorAggregate")) {
      Variant iterator = obj->o_invoke_mil(
                                       "getiterator", Array(), -1);
      if (!iterator.isObject()) break;
      if (iterator.instanceof("Iterator")) {
        return new ObjectArrayIter(iterator.getObjectData(), &iterator);
      }
      obj = iterator.getObjectData();
    }
    return new ArrayIter(obj->o_toIterArray(context));
  }
  raise_warning("Invalid argument supplied for foreach()");
  return new ArrayIter(NULL);
}

MutableArrayIterPtr Variant::begin(Variant *key, Variant &val) {
  if (is(KindOfObject)) {
    ObjectData *obj = getObjectData();
    if (obj->o_instanceof("Iterator")) {
      throw FatalErrorException("An iterator cannot be used with "
                                "foreach by reference");
    }
    while (obj->o_instanceof("IteratorAggregate")) {
      Variant iterator = obj->o_invoke("getiterator", Array(), -1);
      if (!iterator.isObject()) break;
      if (iterator.instanceof("Iterator")) {
        throw FatalErrorException("An iterator cannot be used with "
                                  "foreach by reference");
      }
      obj = iterator.getObjectData();
    }
    Array properties = obj->o_toIterArray(null_string, true);
    properties.escalate(true);
    ArrayData *arr = properties.getArrayData();
    return new MutableArrayIter(arr, key, val);
  }
  // we are about to modify an array that has other weak references, so
  // we have to make a copy to preserve other instances
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1) {
      set(arr->copy());
    }
  }
  return new MutableArrayIter(this, key, val);
}

void Variant::escalate(bool mutableIteration /* = false */) {
  if (is(KindOfArray)) {
    Array arr = toArray();
    arr.escalate(mutableIteration);
    set(arr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// type conversions

bool Variant::toBooleanHelper() const {
  switch (m_type) {
  case KindOfNull:    return false;
  case KindOfDouble:  return m_data.dbl != 0;
  case KindOfStaticString:
  case KindOfString:  return m_data.pstr->toBoolean();
  case KindOfArray:   return !m_data.parr->empty();
  case KindOfObject:  return m_data.pobj->o_toBoolean();
  case KindOfVariant: return m_data.pvar->toBoolean();
  default:
    ASSERT(false);
    break;
  }
  return m_data.num;
}

int64 Variant::toInt64Helper(int base /* = 10 */) const {
  switch (m_type) {
  case KindOfNull:    return 0;
  case KindOfDouble:  {
    return (m_data.dbl > LONG_MAX) ? (uint64)m_data.dbl : (int64)m_data.dbl;
  }
  case KindOfStaticString:
  case KindOfString:  return m_data.pstr->toInt64(base);
  case KindOfArray:   return m_data.parr->empty() ? 0 : 1;
  case KindOfObject:  return m_data.pobj->o_toInt64();
  case KindOfVariant: return m_data.pvar->toInt64(base);
  default:
    ASSERT(false);
    break;
  }
  return m_data.num;
}

double Variant::toDouble() const {
  switch (m_type) {
  case KindOfNull:    return 0.0;
  case KindOfDouble:  return m_data.dbl;
  case KindOfStaticString:
  case KindOfString:  return m_data.pstr->toDouble();
  case KindOfObject:  return m_data.pobj->o_toDouble();
  case KindOfVariant: return m_data.pvar->toDouble();
  default:
    break;
  }
  return (double)toInt64();
}

String Variant::toString() const {
  switch (m_type) {
  case KindOfNull:    return empty_string;
  case KindOfBoolean: return m_data.num ? "1" : "";
  case KindOfDouble:  return m_data.dbl;
  case KindOfStaticString:
  case KindOfString:  return m_data.pstr;
  case KindOfArray:   return "Array";
  case KindOfObject:  return m_data.pobj->t___tostring();
  case KindOfVariant: return m_data.pvar->toString();
  default:
    break;
  }
  return m_data.num;
}

Array Variant::toArray() const {
  switch (m_type) {
  case KindOfNull:    return Array::Create();
  case KindOfInt64:   return Array::Create(m_data.num);
  case KindOfStaticString:
  case KindOfString:  return Array::Create(String(m_data.pstr));
  case KindOfArray:   return m_data.parr;
  case KindOfObject:  return m_data.pobj->o_toArray();
  case KindOfVariant: return m_data.pvar->toArray();
  default:
    break;
  }
  return Array::Create(*this);
}

Object Variant::toObject() const {
  if (m_type == KindOfObject) return m_data.pobj;
  if (m_type == KindOfVariant) return m_data.pvar->toObject();

  switch (m_type) {
  case KindOfNull:
    break;
  case KindOfBoolean:
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfStaticString:
  case KindOfString:
    {
      c_stdClass *obj = NEW(c_stdClass)();
      obj->o_set(s_scalar, *this, false);
      return obj;
    }
  case KindOfArray:   return m_data.parr->toObject();
  default:
    ASSERT(false);
    break;
  }
  return Object(NEW(c_stdClass)());
}

Variant Variant::toKey() const {
  if (m_type == KindOfString || m_type == KindOfStaticString) {
    int64 n;
    if (m_data.pstr->isStrictlyInteger(n)) {
      return n;
    } else {
      return *this;
    }
  }
  switch (m_type) {
  case KindOfNull:
    return empty_string;
  case KindOfBoolean:
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    return m_data.num;
  case KindOfDouble:
    return (int64)m_data.dbl;
  case KindOfObject:
    if (isResource()) {
      return toInt64();
    }
    break;
  case KindOfVariant:
    return m_data.pvar->toKey();
  default:
    break;
  }
  throw_bad_type_exception("Invalid type used as key");
  return null_variant;
}

Variant::operator String() const {
  return toString();
}

Variant::operator Array() const {
  return toArray();
}

Variant::operator Object() const {
  return toObject();
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

bool Variant::same(bool v2) const {
  return is(KindOfBoolean) && equal(v2);
}

bool Variant::same(char v2) const {
  return same((int64)v2);
}

bool Variant::same(short v2) const {
  return same((int64)v2);
}

bool Variant::same(int v2) const {
  return same((int64)v2);
}

bool Variant::same(int64 v2) const {
  switch (getType()) {
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    return equal(v2);
  default:
    break;
  }
  return false;
}

bool Variant::same(double v2) const {
  return isDouble() && equal(v2);
}

bool Variant::same(litstr v2) const {
  return same(String(v2));
}

bool Variant::same(CStrRef v2) const {
  bool null1 = isNull();
  bool null2 = v2.isNull();
  if (null1 && null2) return true;
  if (null1 || null2) return false;
  return isString() && equal(v2);
}

bool Variant::same(CArrRef v2) const {
  bool null1 = isNull();
  bool null2 = v2.isNull();
  if (null1 && null2) return true;
  if (null1 || null2) return false;
  return is(KindOfArray) && toArray().same(v2);
}

bool Variant::same(CObjRef v2) const {
  bool null1 = isNull();
  bool null2 = v2.isNull();
  if (null1 && null2) return true;
  if (null1 || null2) return false;
  return is(KindOfObject) && getObjectData() == v2.get();
}

bool Variant::same(CVarRef v2) const {
  bool null1 = isNull();
  bool null2 = v2.isNull();
  if (null1 && null2) return true;
  if (null1 || null2) return false;

  switch (getType()) {
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    switch (v2.getType()) {
    case KindOfByte:
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
      return equal(v2);
    default:
      break;
    }
    break;
  case KindOfStaticString:
  case KindOfString: {
    switch (v2.getType()) {
    case KindOfStaticString:
    case KindOfString:
      return getStringData()->same(v2.getStringData());
    default:
      return false;
    }
  }
  case KindOfArray:
    if (v2.is(KindOfArray)) {
      return toArray().same(v2.toArray());
    }
    break;
  case KindOfObject:
    return v2.is(KindOfObject) && getObjectData() == v2.getObjectData();
  default:
    break;
  }
  return getType() == v2.getType() && equal(v2);
}

///////////////////////////////////////////////////////////////////////////////

#define UNWRAP(reverse)                                         \
  switch (getType()) {                                          \
  case KindOfNull:                                              \
  case KindOfBoolean: return HPHP::reverse(v2, toBoolean());    \
  case KindOfByte:                                              \
  case KindOfInt16:                                             \
  case KindOfInt32:                                             \
  case KindOfInt64:   return HPHP::reverse(v2, toInt64());      \
  case KindOfDouble:  return HPHP::reverse(v2, toDouble());     \
  case KindOfStaticString:                                      \
  case KindOfString:  return HPHP::reverse(v2, toString());     \
  case KindOfArray:   return HPHP::reverse(v2, toArray());      \
  case KindOfObject:  return HPHP::reverse(v2, toObject());     \
  default:                                                      \
    ASSERT(false);                                              \
    break;                                                      \
  }                                                             \
  return false;                                                 \

// "null" needs to convert to "" before comparing with a string
#define UNWRAP_STR(reverse)                                     \
  switch (getType()) {                                          \
  case KindOfNull:    return HPHP::reverse(v2, "");             \
  case KindOfBoolean: return HPHP::reverse(v2, toBoolean());    \
  case KindOfByte:                                              \
  case KindOfInt16:                                             \
  case KindOfInt32:                                             \
  case KindOfInt64:   return HPHP::reverse(v2, toInt64());      \
  case KindOfDouble:  return HPHP::reverse(v2, toDouble());     \
  case KindOfStaticString:                                      \
  case KindOfString:  return HPHP::reverse(v2, toString());     \
  case KindOfArray:   return HPHP::reverse(v2, toArray());      \
  case KindOfObject:  return HPHP::reverse(v2, toObject());     \
  default:                                                      \
    ASSERT(false);                                              \
    break;                                                      \
  }                                                             \
  return false;                                                 \

// "null" needs to convert to "" before comparing with a string
#define UNWRAP_VAR(forward, reverse)                            \
  switch (getType()) {                                          \
  case KindOfNull:                                              \
    if (v2.isString()) {                                        \
      return HPHP::reverse(v2, "");                             \
    } /* otherwise fall through */                              \
  case KindOfBoolean: return HPHP::reverse(v2, toBoolean());    \
  case KindOfByte:                                              \
  case KindOfInt16:                                             \
  case KindOfInt32:                                             \
  case KindOfInt64:   return HPHP::reverse(v2, toInt64());      \
  case KindOfDouble:  return HPHP::reverse(v2, toDouble());     \
  case KindOfStaticString:                                      \
  case KindOfString:  return HPHP::reverse(v2, toString());     \
  case KindOfArray:                                             \
    if (v2.is(KindOfArray)) {                                   \
      return toArray().forward(v2.toArray());                   \
    }                                                           \
    return HPHP::reverse(v2, toArray());                        \
  case KindOfObject:  return HPHP::reverse(v2, toObject());     \
  default:                                                      \
    ASSERT(false);                                              \
    break;                                                      \
  }                                                             \
  return false;                                                 \

// array comparison is directional when they are uncomparable
// also, ">" is implemented as "!<=" in Zend
#define UNWRAP_ARR(forward, reverse)                            \
  switch (getType()) {                                          \
  case KindOfNull:                                              \
  case KindOfBoolean: return HPHP::reverse(v2, toBoolean());    \
  case KindOfByte:                                              \
  case KindOfInt16:                                             \
  case KindOfInt32:                                             \
  case KindOfInt64:   return HPHP::reverse(v2, toInt64());      \
  case KindOfDouble:  return HPHP::reverse(v2, toDouble());     \
  case KindOfStaticString:                                      \
  case KindOfString:  return HPHP::reverse(v2, toString());     \
  case KindOfArray:   return toArray().forward(v2);             \
  case KindOfObject:  return HPHP::reverse(v2, toObject());     \
  default:                                                      \
    ASSERT(false);                                              \
    break;                                                      \
  }                                                             \
  return false;                                                 \

bool Variant::equal(bool    v2) const { UNWRAP(equal);}
bool Variant::equal(char    v2) const { UNWRAP(equal);}
bool Variant::equal(short   v2) const { UNWRAP(equal);}
bool Variant::equal(int     v2) const { UNWRAP(equal);}
bool Variant::equal(int64   v2) const { UNWRAP(equal);}
bool Variant::equal(double  v2) const { UNWRAP(equal);}
bool Variant::equal(litstr  v2) const { UNWRAP_STR(equal);}
bool Variant::equal(CStrRef v2) const { UNWRAP_STR(equal);}
bool Variant::equal(CArrRef v2) const { UNWRAP(equal);}
bool Variant::equal(CObjRef v2) const { UNWRAP(equal);}
bool Variant::equal(CVarRef v2) const { UNWRAP_VAR(equal,equal);}

bool Variant::less(bool    v2) const { UNWRAP(more);}
bool Variant::less(char    v2) const { UNWRAP(more);}
bool Variant::less(short   v2) const { UNWRAP(more);}
bool Variant::less(int     v2) const { UNWRAP(more);}
bool Variant::less(int64   v2) const { UNWRAP(more);}
bool Variant::less(double  v2) const { UNWRAP(more);}
bool Variant::less(litstr  v2) const { UNWRAP_STR(more);}
bool Variant::less(CStrRef v2) const { UNWRAP_STR(more);}
bool Variant::less(CArrRef v2) const { UNWRAP_ARR(less,more);}
bool Variant::less(CObjRef v2) const { UNWRAP(more);}
bool Variant::less(CVarRef v2) const { UNWRAP_VAR(less,more);}

bool Variant::more(bool    v2) const { UNWRAP(less);}
bool Variant::more(char    v2) const { UNWRAP(less);}
bool Variant::more(short   v2) const { UNWRAP(less);}
bool Variant::more(int     v2) const { UNWRAP(less);}
bool Variant::more(int64   v2) const { UNWRAP(less);}
bool Variant::more(double  v2) const { UNWRAP(less);}
bool Variant::more(litstr  v2) const { UNWRAP_STR(less);}
bool Variant::more(CStrRef v2) const { UNWRAP_STR(less);}
bool Variant::more(CArrRef v2) const { UNWRAP_ARR(more,less);}
bool Variant::more(CObjRef v2) const { UNWRAP(less);}
bool Variant::more(CVarRef v2) const { UNWRAP_VAR(more,less);}

///////////////////////////////////////////////////////////////////////////////
// comparison operators

bool Variant::operator==(CVarRef v) const {
  return HPHP::equal(*this, v);
}

bool Variant::operator!=(CVarRef v) const {
  return !HPHP::equal(*this, v);
}

bool Variant::operator>=(CVarRef v) const {
  return not_less(*this, v);
}

bool Variant::operator<=(CVarRef v) const {
  return not_more(*this, v);
}

bool Variant::operator>(CVarRef v) const {
  return HPHP::more(*this, v);
}

bool Variant::operator<(CVarRef v) const {
  return HPHP::less(*this, v);
}

///////////////////////////////////////////////////////////////////////////////
// offset functions

ObjectData *Variant::getArrayAccess() const {
  ASSERT(is(KindOfObject));
  ObjectData *obj = getObjectData();
  ASSERT(obj);
  if (!obj->o_instanceof("ArrayAccess")) {
    throw InvalidOperandException("not ArrayAccess objects");
  }
  return obj;
}

void Variant::callOffsetUnset(CVarRef key) {
  getArrayAccess()->o_invoke(s_offsetUnset, Array::Create(key));
}

#define IMPLEMENT_RVAL_INTEGRAL                                         \
  if (m_type == KindOfArray) {                                          \
    return m_data.parr->get((int64)offset, error);                      \
  }                                                                     \
  switch (m_type) {                                                     \
    case KindOfStaticString:                                            \
    case KindOfString:                                                  \
      return m_data.pstr->getChar((int)offset);                         \
    case KindOfObject:                                                  \
      return getArrayAccess()->o_invoke(s_offsetGet,                    \
                                        Array::Create(offset));         \
    case KindOfVariant:                                                 \
      return m_data.pvar->rvalAt(offset, error);                        \
    case KindOfNull:                                                    \
      break;                                                            \
    default:                                                            \
      if (error) {                                                      \
        raise_notice("taking offset [] on bool or number");             \
      }                                                                 \
      break;                                                            \
  }                                                                     \
  return null_variant;

Variant Variant::rvalAt(bool offset, bool error /* = false */) const {
  IMPLEMENT_RVAL_INTEGRAL
}
Variant Variant::rvalAt(double offset, bool error /* = false */) const {
  IMPLEMENT_RVAL_INTEGRAL
}

Variant Variant::rvalAtHelper(int64 offset, bool error /* = false */) const {
  switch (m_type) {
  case KindOfStaticString:
  case KindOfString:
    return m_data.pstr->getChar((int)offset);
  case KindOfObject:
    return getArrayAccess()->o_invoke(s_offsetGet, Array::Create(offset));
  case KindOfVariant:
    return m_data.pvar->rvalAt(offset, error);
  case KindOfNull:
    break;
  default:
    if (error) {
      raise_notice("taking offset [] on bool or number");
    }
    break;
  }
  return null_variant;
}

Variant Variant::rvalAt(litstr offset, bool error /* = false */,
                        bool isString /* = false */) const {
  if (m_type == KindOfArray) {
    if (isString) return m_data.parr->get(offset, error);
    int64 n;
    int len = strlen(offset);
    if (!is_strictly_integer(offset, len, n)) {
      return m_data.parr->get(offset, error);
    } else {
      return m_data.parr->get(n, error);
    }
  }
  switch (m_type) {
  case KindOfStaticString:
  case KindOfString:
    return m_data.pstr->getChar(String(offset).toInt32());
  case KindOfObject:
    return getArrayAccess()->o_invoke(s_offsetGet, Array::Create(offset));
  case KindOfVariant:
    return m_data.pvar->rvalAt(offset, error);
  case KindOfNull:
    break;
  default:
    if (error) {
      raise_notice("taking offset [] on bool or number");
    }
    break;
  }
  return null_variant;
}

Variant Variant::rvalAt(CStrRef offset, bool error /* = false */,
                        bool isString /* = false */) const {
  if (m_type == KindOfArray) {
    if (isString) return m_data.parr->get(offset, error);
    if (offset.isNull()) return m_data.parr->get(empty_string, error);
    int64 n;
    if (!offset->isStrictlyInteger(n)) {
      return m_data.parr->get(offset, error);
    } else {
      return m_data.parr->get(n, error);
    }
  }
  switch (m_type) {
  case KindOfStaticString:
  case KindOfString:
    return m_data.pstr->getChar(offset.toInt32());
  case KindOfObject:
    return getArrayAccess()->o_invoke(s_offsetGet, Array::Create(offset));
  case KindOfVariant:
    return m_data.pvar->rvalAt(offset, error, isString);
  case KindOfNull:
    break;
  default:
    if (error) {
      raise_notice("taking offset [] on bool or number");
    }
    break;
  }
  return null_variant;
}

Variant Variant::rvalAt(CVarRef offset, bool error /* = false */) const {
  if (m_type == KindOfArray) {
    // Fast path for KindOfArray
    switch (offset.m_type) {
    case KindOfNull:
      return m_data.parr->get(empty_string, error);
    case KindOfBoolean:
    case KindOfByte:
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
      return m_data.parr->get(offset.m_data.num, error);
    case KindOfDouble:
      return m_data.parr->get((int64)offset.m_data.dbl, error);
    case KindOfStaticString:
    case KindOfString: {
      int64 n;
      if (offset.m_data.pstr->isStrictlyInteger(n)) {
        return m_data.parr->get(n, error);
      } else {
        return m_data.parr->get(String(offset.m_data.pstr), error);
      }
    }
    case KindOfArray:
      throw_bad_type_exception("Invalid type used as key");
      break;
    case KindOfObject:
      if (offset.isResource()) {
        return m_data.parr->get(offset.toInt64(), error);
      }
      throw_bad_type_exception("Invalid type used as key");
      break;
    case KindOfVariant:
      return rvalAt(*(offset.m_data.pvar), error);
    default:
      ASSERT(false);
      break;
    }
    return null_variant;
  }
  switch (m_type) {
  case KindOfStaticString:
  case KindOfString:
    return m_data.pstr->getChar(offset.toInt32());
  case KindOfObject:
    return getArrayAccess()->o_invoke(s_offsetGet, Array::Create(offset));
  case KindOfVariant:
    return m_data.pvar->rvalAt(offset, error);
  case KindOfNull:
    break;
  default:
    if (error) {
      raise_notice("taking offset [] on bool or number");
    }
    break;
  }
  return null_variant;
}

Variant &Variant::lvalAt(bool    key, bool    checkExist /* = false */) {
  return lvalAtImpl(key, checkExist);
}
Variant &Variant::lvalAt(char    key, bool    checkExist /* = false */) {
  return lvalAtImpl(key, checkExist);
}
Variant &Variant::lvalAt(short   key, bool    checkExist /* = false */) {
  return lvalAtImpl(key, checkExist);
}
Variant &Variant::lvalAt(int     key, bool    checkExist /* = false */) {
  return lvalAtImpl(key, checkExist);
}
Variant &Variant::lvalAt(int64   key, bool    checkExist /* = false */) {
  if (m_type == KindOfArray) {
    Variant *ret = NULL;
    ArrayData *arr = m_data.parr;
    ArrayData *escalated =
      arr->lval(key, ret, arr->getCount() > 1, checkExist);
    if (escalated) {
      set(escalated);
    }
    ASSERT(ret);
    return *ret;
  }
  return lvalAtImpl(key, checkExist);
}
Variant &Variant::lvalAt(double  key, bool    checkExist /* = false */) {
  return lvalAtImpl((int64)key, checkExist);
}
Variant &Variant::lvalAt(litstr  key, bool    checkExist /* = false */,
                         bool    isString /* = false */) {
  if (m_type == KindOfArray) {
    Variant *ret = NULL;
    ArrayData *arr = m_data.parr;
    ArrayData *escalated;
    if (isString) {
      escalated = arr->lval(key, ret, arr->getCount() > 1, checkExist);
    } else {
      escalated = arr->lval(String(key).toKey(), ret, arr->getCount() > 1,
                            checkExist);
    }
    if (escalated) {
      set(escalated);
    }
    ASSERT(ret);
    return *ret;
  }
  return lvalAtImpl(key, checkExist);
}
Variant &Variant::lvalAt(CStrRef key, bool    checkExist /* = false */,
                         bool    isString /* = false */) {
  if (m_type == KindOfArray) {
    Variant *ret = NULL;
    ArrayData *arr = m_data.parr;
    ArrayData *escalated;
    if (isString) {
      escalated = arr->lval(key, ret, arr->getCount() > 1, checkExist);
    } else {
      escalated = arr->lval(key.toKey(), ret, arr->getCount() > 1, checkExist);
    }
    if (escalated) {
      set(escalated);
    }
    ASSERT(ret);
    return *ret;
  }
  return lvalAtImpl(key, checkExist);
}
Variant &Variant::lvalAt(CVarRef k, bool checkExist /* = false */) {
  if (m_type == KindOfArray) {
    Variant *ret = NULL;
    ArrayData *arr = m_data.parr;
    ArrayData *escalated = arr->lval(k.toKey(), ret, arr->getCount() > 1,
        checkExist);
    if (escalated) {
      set(escalated);
    }
    ASSERT(ret);
    return *ret;
  }
  return lvalAtImpl(k, checkExist);
}

Variant *Variant::lvalPtr(CStrRef key, bool forWrite, bool create) {
  Variant *t = m_type == KindOfVariant ? m_data.pvar : this;
  if (t->m_type == KindOfArray) {
    return t->asArrRef().lvalPtr(key, forWrite, create);
  }
  return NULL;
}

Variant &Variant::lvalInvalid() {
  throw_bad_type_exception("not array objects");
  return lvalBlackHole();
}

Variant &Variant::lvalBlackHole() {
  Variant &bh = ((Globals*)get_global_variables())->__lvalProxy;
  bh.unset();
  return bh;
}

Variant Variant::refvalAt(bool    key) {
  return refvalAtImpl(key);
}
Variant Variant::refvalAt(char    key) {
  return refvalAtImpl(key);
}
Variant Variant::refvalAt(short   key) {
  return refvalAtImpl(key);
}
Variant Variant::refvalAt(int     key) {
  return refvalAtImpl(key);
}
Variant Variant::refvalAt(int64   key) {
  return refvalAtImpl(key);
}
Variant Variant::refvalAt(double  key) {
  return refvalAtImpl((int64)key);
}
Variant Variant::refvalAt(litstr  key, bool isString /* = false */) {
  return refvalAtImpl(key, isString);
}
Variant Variant::refvalAt(CStrRef key, bool isString /* = false */) {
  return refvalAtImpl(key, isString);
}
Variant Variant::refvalAt(CVarRef key) {
  return refvalAtImpl(key);
}

Variant Variant::refvalAtImpl(CStrRef key, bool isString /* = false */) {
  if (m_type == KindOfVariant) {
    return m_data.pvar->refvalAtImpl(key, isString);
  }
  if (is(KindOfArray) || isObjectConvertable()) {
    return ref(lvalAt(key, false, isString));
  } else {
    return rvalAt(key, isString);
  }
}

Variant Variant::o_get(CStrRef propName, bool error /* = true */,
                       CStrRef context /* = null_string */) const {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_get(propName, error, context);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_get(propName, error, context);
  } else if (error) {
    raise_notice("Trying to get property of non-object");
  }
  return null_variant;
}

Variant Variant::o_getPublic(CStrRef propName, bool error /* = true */) const {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_getPublic(propName, error);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_getPublic(propName, error);
  } else if (error) {
    raise_notice("Trying to get property of non-object");
  }
  return null_variant;
}

Variant Variant::o_set(CStrRef propName, CVarRef val,
                       CStrRef context /* = null_string */) {
  if (propName.empty()) {
    throw EmptyObjectPropertyException();
  }

  if (m_type == KindOfObject) {
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_set(propName, val, context);
  } else if (isObjectConvertable()) {
    set(Object(NEW(c_stdClass)()));
  } else {
    // Raise a warning
    raise_warning("Attempt to assign property of non-object");
    return val;
  }
  return m_data.pobj->o_set(propName, val, false, context);
}

Variant Variant::o_invoke(MethodIndex methodIndex, const char *s,
                          CArrRef params, int64 hash) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke(methodIndex, s, params, hash);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_invoke(methodIndex, s, params, hash);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_invoke(CStrRef s, CArrRef params, int64 hash /* = -1 */) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke_mil(s, params, hash);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_invoke_mil(s, params, hash);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_invoke_mil(const char *s,
                              CArrRef params, int64 hash) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      if (m_type == KindOfObject) {
        return m_data.pobj->o_invoke_mil(s, params, hash);
      } else if (m_type == KindOfVariant) {
        return m_data.pvar->o_invoke_mil(s, params, hash);
      }
    }
  }
  return o_invoke(methodIndex, s, params, hash);
}

Variant Variant::o_root_invoke(MethodIndex methodIndex, const char *s,
                               CArrRef params, int64 hash) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_root_invoke(methodIndex, s, params, hash);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_root_invoke(methodIndex, s, params, hash);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_root_invoke(CStrRef s, CArrRef params,
                               int64 hash /* = -1 */) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_root_invoke_mil(s, params, hash);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_root_invoke_mil(s, params, hash);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_root_invoke_mil(const char *s,
                                   CArrRef params, int64 hash) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      if (m_type == KindOfObject) {
        return m_data.pobj->doRootCall(s, params, true);
      } else if (m_type == KindOfVariant) {
        return m_data.pvar->o_root_invoke_mil(s, params, hash);
      }
    }
  }
  return o_root_invoke( methodIndex, s, params, hash);
}

Variant Variant::o_invoke_ex(const char *clsname, MethodIndex methodIndex,
                             const char *s,
                             CArrRef params, int64 hash) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke_ex(clsname, methodIndex, s, params, hash);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_invoke_ex(clsname, methodIndex, s, params, hash);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_invoke_ex_mil(const char *clsname, const char *s,
                                 CArrRef params, int64 hash) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      if (m_type == KindOfObject) {
        return m_data.pobj->doRootCall(s, params, true);
      } else if (m_type == KindOfVariant) {
        return m_data.pvar->o_invoke_ex_mil(clsname, s, params, hash);
      }
    }
  }
  return o_invoke_ex(clsname, methodIndex, s, params, hash);
}

Variant Variant::o_invoke_few_args(CStrRef s, int64 hash, int count,
                                   INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke_few_args_mil(s, hash, count,
                                          INVOKE_FEW_ARGS_PASS_ARGS);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_invoke_few_args_mil(s, hash, count,
                                          INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_invoke_few_args(MethodIndex methodIndex, const char *s,
                                   int64 hash, int count,
                                   INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke_few_args(methodIndex, s, hash, count,
                                          INVOKE_FEW_ARGS_PASS_ARGS);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_invoke_few_args(methodIndex, s, hash, count,
                                          INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_invoke_few_args_mil(const char *s,
                                   int64 hash, int count,
                                   INVOKE_FEW_ARGS_IMPL_ARGS) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      if (m_type == KindOfObject) {
        return m_data.pobj->o_invoke_few_args_mil(s, hash, count,
                                                  INVOKE_FEW_ARGS_PASS_ARGS);
      } else if (m_type == KindOfVariant) {
        return m_data.pvar->o_invoke_few_args_mil(s, hash, count,
                                                  INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
      }
    }
  }
  return o_invoke_few_args(methodIndex, s, hash, count,
                           INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant Variant::o_root_invoke_few_args(MethodIndex methodIndex,
                                        const char *s, int64 hash, int count,
                                        INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_root_invoke_few_args(methodIndex, s, hash, count,
                                               INVOKE_FEW_ARGS_PASS_ARGS);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_root_invoke_few_args(methodIndex, s, hash, count,
                                               INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_root_invoke_few_args(CStrRef s, int64 hash, int count,
                                        INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_root_invoke_few_args_mil(s, hash, count,
                                                   INVOKE_FEW_ARGS_PASS_ARGS);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_root_invoke_few_args_mil(s, hash, count,
                                                   INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_root_invoke_few_args_mil(const char *s, int64 hash,
                                            int count,
                                            INVOKE_FEW_ARGS_IMPL_ARGS) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      if (m_type == KindOfObject) {
        return m_data.pobj->
          o_root_invoke_few_args_mil(s, hash, count,
                                     INVOKE_FEW_ARGS_PASS_ARGS);
      } else if (m_type == KindOfVariant) {
        return m_data.pvar->
          o_root_invoke_few_args_mil(s, hash, count,
                                     INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
      }
    }
  }
  return o_root_invoke_few_args(methodIndex, s, hash, count,
                                INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant &Variant::o_lval(CStrRef propName, CVarRef tmpForGet,
                         CStrRef context /* = null_string */) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_lval(propName, tmpForGet, context);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_lval(propName, tmpForGet, context);
  } else if (isObjectConvertable()) {
    set(Object(NEW(c_stdClass)()));
    return m_data.pobj->o_lval(propName, tmpForGet, context);
  } else {
    // Raise a warning
    raise_warning("Attempt to assign property of non-object");
    return const_cast<Variant&>(tmpForGet);
  }
}

#define IMPLEMENT_SETAT                                                 \
  if (m_type == KindOfArray) {                                          \
    if (v.isContagious()) {                                             \
      escalate();                                                       \
    }                                                                   \
    ArrayData *escalated =                                              \
      m_data.parr->set(ToKey(key), v, (m_data.parr->getCount() > 1));   \
    if (escalated) {                                                    \
      set(escalated);                                                   \
    }                                                                   \
    return v;                                                           \
  }                                                                     \
  switch (m_type) {                                                     \
  case KindOfBoolean:                                                   \
    if (toBoolean()) {                                                  \
      throw_bad_type_exception("not array objects");                    \
      break;                                                            \
    }                                                                   \
    /* Fall through */                                                  \
  case KindOfNull:                                                      \
    set(ArrayData::Create(ToKey(key), v));                              \
    break;                                                              \
  case KindOfVariant:                                                   \
    m_data.pvar->set(key, v);                                           \
    break;                                                              \
  case KindOfStaticString:                                              \
  case KindOfString:                                                    \
    {                                                                   \
      String s = toString();                                            \
      if (s.empty()) {                                                  \
        set(Array::Create(ToKey(key), v));                              \
      } else {                                                          \
        s.lvalAt(key) = v;                                              \
        set(s);                                                         \
      }                                                                 \
      break;                                                            \
    }                                                                   \
  case KindOfObject:                                                    \
    getArrayAccess()->o_invoke(s_offsetSet,                             \
                               CREATE_VECTOR2(key, v));                 \
    break;                                                              \
  default:                                                              \
    throw_bad_type_exception("not array objects");                      \
    break;                                                              \
  }                                                                     \
  return v;                                                             \

#define IMPLEMENT_SETAT_OPEQUAL                                         \
check_array:                                                            \
  if (m_type == KindOfArray) {                                          \
    Variant *cv = NULL;                                                 \
    ASSERT(!v.isContagious());                                          \
    ArrayData *escalated =                                              \
      m_data.parr->lval(ToKey(key), cv, (m_data.parr->getCount() > 1)); \
    if (escalated) {                                                    \
      set(escalated);                                                   \
    }                                                                   \
    ASSERT(cv);                                                         \
    switch (op) {                                                       \
    case T_CONCAT_EQUAL: return concat_assign((*cv), v);                \
    case T_PLUS_EQUAL:  return ((*cv) += v);                            \
    case T_MINUS_EQUAL: return ((*cv) -= v);                            \
    case T_MUL_EQUAL:   return ((*cv) *= v);                            \
    case T_DIV_EQUAL:   return ((*cv) /= v);                            \
    case T_MOD_EQUAL:   return ((*cv) %= v);                            \
    case T_AND_EQUAL:   return ((*cv) &= v);                            \
    case T_OR_EQUAL:    return ((*cv) |= v);                            \
    case T_XOR_EQUAL:   return ((*cv) ^= v);                            \
    case T_SL_EQUAL:    return ((*cv) <<= v);                           \
    case T_SR_EQUAL:    return ((*cv) >>= v);                           \
    default:                                                            \
      throw FatalErrorException("invalid operator %d", op);             \
    }                                                                   \
  }                                                                     \
  switch (m_type) {                                                     \
  case KindOfBoolean:                                                   \
    if (toBoolean()) {                                                  \
      throw_bad_type_exception("not array objects");                    \
      break;                                                            \
    }                                                                   \
    /* Fall through */                                                  \
  case KindOfNull:                                                      \
    set(ArrayData::Create(ToKey(key), null));                           \
    goto check_array;                                                   \
  case KindOfVariant:                                                   \
    m_data.pvar->setOpEqual(op, key, v);                                \
    break;                                                              \
  case KindOfStaticString:                                              \
  case KindOfString:                                                    \
    throw_bad_type_exception("not array objects");                      \
    break;                                                              \
  case KindOfObject: {                                                  \
    ObjectData *aa = getArrayAccess();                                  \
    Variant &cv = aa->___offsetget_lval(key);                           \
    switch (op) {                                                       \
    case T_CONCAT_EQUAL: concat_assign(cv, v); break;                   \
    case T_PLUS_EQUAL:   cv += v;              break;                   \
    case T_MINUS_EQUAL:  cv -= v;              break;                   \
    case T_MUL_EQUAL:    cv *= v;              break;                   \
    case T_DIV_EQUAL:    cv /= v;              break;                   \
    case T_MOD_EQUAL:    cv %= v;              break;                   \
    case T_AND_EQUAL:    cv &= v;              break;                   \
    case T_OR_EQUAL:     cv |= v;              break;                   \
    case T_XOR_EQUAL:    cv ^= v;              break;                   \
    case T_SL_EQUAL:     cv <<= v;             break;                   \
    case T_SR_EQUAL:     cv >= v;              break;                   \
    default:                                                            \
      throw FatalErrorException("invalid operator %d", op);             \
    }                                                                   \
    aa->o_invoke(s_offsetSet, CREATE_VECTOR2(key, cv), -1);             \
    return cv;                                                          \
  }                                                                     \
  default:                                                              \
    throw_bad_type_exception("not array objects");                      \
    break;                                                              \
  }                                                                     \
  return v;                                                             \

CVarRef Variant::set(bool key, CVarRef v) {
  IMPLEMENT_SETAT;
}

CVarRef Variant::set(int64 key, CVarRef v) {
  IMPLEMENT_SETAT;
}
CVarRef Variant::set(double  key, CVarRef v) {
  IMPLEMENT_SETAT;
}

CVarRef Variant::set(litstr key, CVarRef v, bool isString /* = false */) {
  IMPLEMENT_SETAT;
}

CVarRef Variant::set(CStrRef key, CVarRef v, bool isString /* = false */) {
  IMPLEMENT_SETAT;
}

CVarRef Variant::set(CVarRef key, CVarRef v) {
  IMPLEMENT_SETAT;
}

CVarRef Variant::setOpEqual(int op, bool key, CVarRef v) {
  IMPLEMENT_SETAT_OPEQUAL;
}

CVarRef Variant::setOpEqual(int op, int64 key, CVarRef v) {
  IMPLEMENT_SETAT_OPEQUAL;
}

CVarRef Variant::setOpEqual(int op, double  key, CVarRef v) {
  IMPLEMENT_SETAT_OPEQUAL;
}

CVarRef Variant::setOpEqual(int op, litstr key, CVarRef v,
                            bool isString /* = false */) {
  IMPLEMENT_SETAT_OPEQUAL;
}

CVarRef Variant::setOpEqual(int op, CStrRef key, CVarRef v,
                            bool isString /* = false */) {
  IMPLEMENT_SETAT_OPEQUAL;
}

CVarRef Variant::setOpEqual(int op, CVarRef key, CVarRef v) {
  IMPLEMENT_SETAT_OPEQUAL;
}

CVarRef Variant::append(CVarRef v) {
  switch (m_type) {
  case KindOfNull:
    set(ArrayData::Create(v));
    break;
  case KindOfBoolean:
    if (!toBoolean()) {
      set(ArrayData::Create(v));
    } else {
      throw_bad_type_exception("[] operator not supported for this type");
    }
    break;
  case KindOfArray:
    {
      bool contagious = false;
      if (v.isContagious()) {
        escalate();
        contagious = true;
      }
      ArrayData *escalated =
        m_data.parr->append(v, (m_data.parr->getCount() > 1));
      if (escalated) {
        set(escalated);
        // special case "$a[] = $a;" to match PHP's weird semantics
        if (!contagious &&
            (this == &v ||
             (v.is(KindOfArray) && getArrayData() == v.getArrayData()))) {
          const_cast<Variant&>(escalated->endRef()).set(escalated);
        }
      }
    }
    break;
  case KindOfVariant:
    m_data.pvar->append(v);
    break;
  case KindOfObject:
    {
      Array params = CREATE_VECTOR2(null, v);
      m_data.pobj->o_invoke(s_offsetSet, params);
      break;
    }
  case KindOfStaticString:
  case KindOfString:
    if (toString().empty()) {
      set(ArrayData::Create(v));
      return v;
    }
    // fall through to throw
  default:
    throw_bad_type_exception("[] operator not supported for this type");
  }
  return v;
}

CVarRef Variant::appendOpEqual(int op, CVarRef v) {
check_array:
  if (m_type == KindOfArray) {
    bool contagious = false;
    if (v.isContagious()) {
      escalate();
      contagious = true;
    }
    ArrayData *escalated =
      m_data.parr->append(null_variant, (m_data.parr->getCount() > 1));
    if (escalated) {
      set(escalated);
    }
    Variant *cv = NULL;
    m_data.parr->lval(cv, (m_data.parr->getCount() > 1));
    ASSERT(cv);
    switch (op) {
    case T_CONCAT_EQUAL: return concat_assign((*cv), v);
    case T_PLUS_EQUAL:  return ((*cv) += v);
    case T_MINUS_EQUAL: return ((*cv) -= v);
    case T_MUL_EQUAL:   return ((*cv) *= v);
    case T_DIV_EQUAL:   return ((*cv) /= v);
    case T_MOD_EQUAL:   return ((*cv) %= v);
    case T_AND_EQUAL:   return ((*cv) &= v);
    case T_OR_EQUAL:    return ((*cv) |= v);
    case T_XOR_EQUAL:   return ((*cv) ^= v);
    case T_SL_EQUAL:    return ((*cv) <<= v);
    case T_SR_EQUAL:    return ((*cv) >>= v);
    default:
      throw FatalErrorException("invalid operator %d", op);
    }
    return v;
  }
  switch (m_type) {
  case KindOfNull:
    set(ArrayData::Create());
    goto check_array;
  case KindOfBoolean:
    if (!toBoolean()) {
      set(ArrayData::Create());
      goto check_array;
    } else {
      throw_bad_type_exception("[] operator not supported for this type");
    }
    break;
  case KindOfVariant:
    m_data.pvar->appendOpEqual(op, v);
    break;
  case KindOfObject: {
    ObjectData *aa = getArrayAccess();
    Variant &cv = aa->___offsetget_lval(null_variant);
    switch (op) {
    case T_CONCAT_EQUAL: concat_assign(cv, v); break;
    case T_PLUS_EQUAL:   cv += v;              break;
    case T_MINUS_EQUAL:  cv -= v;              break;
    case T_MUL_EQUAL:    cv *= v;              break;
    case T_DIV_EQUAL:    cv /= v;              break;
    case T_MOD_EQUAL:    cv %= v;              break;
    case T_AND_EQUAL:    cv &= v;              break;
    case T_OR_EQUAL:     cv |= v;              break;
    case T_XOR_EQUAL:    cv ^= v;              break;
    case T_SL_EQUAL:     cv <<= v;             break;
    case T_SR_EQUAL:     cv >= v;              break;
      break;
    default:
      throw FatalErrorException("invalid operator %d", op);
    }
    aa->o_invoke(s_offsetSet, CREATE_VECTOR2(null_variant, cv));
    return cv;
  }
  case KindOfStaticString:
  case KindOfString:
    if (toString().empty()) {
      set(ArrayData::Create());
      goto check_array;
    }
    // fall through to throw
  default:
    throw_bad_type_exception("[] operator not supported for this type");
  }
  return v;
}

void Variant::remove(CVarRef key) {
  switch(key.getType()) {
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    removeImpl(key.toInt64());
    return;
  default:
    break;
  }
  // Trouble cases: Array, Object
  removeImpl(key);
}

void Variant::setStatic() const {
  switch (m_type) {
  case KindOfString:
    m_data.pstr->setStatic();
    break;
  case KindOfArray:
    m_data.parr->setStatic();
    m_data.parr->onSetStatic();
    break;
  case KindOfVariant:
    m_data.pvar->setStatic();
    break;
  case KindOfObject:
    ASSERT(false); // object shouldn't be in a scalar array
    break;
  default:
    break;
  }
}

Variant &Variant::bindClass(ThreadInfo *info) const {
  if (m_type == KindOfObject) {
    m_data.pobj->bindThis(info);
  } else if (m_type == KindOfVariant) {
    m_data.pvar->bindClass(info);
  } else {
    throw InvalidOperandException("Call to a member function on a non-object");
  }
  return const_cast<Variant&>(*this);
}

///////////////////////////////////////////////////////////////////////////////
// output functions

void Variant::serialize(VariableSerializer *serializer,
                        bool isArrayKey /* = false */) const {
  if (m_type == KindOfVariant) {
    // Ugly, but behavior is different for serialize
    if (serializer->getType() == VariableSerializer::Serialize ||
        serializer->getType() == VariableSerializer::APCSerialize) {
      if (serializer->incNestedLevel(m_data.pvar)) {
        serializer->writeOverflow(m_data.pvar);
      } else {
        m_data.pvar->serialize(serializer, isArrayKey);
      }
      serializer->decNestedLevel(m_data.pvar);
    } else {
      m_data.pvar->serialize(serializer, isArrayKey);
    }
    return;
  }

  switch (m_type) {
  case KindOfNull:
    ASSERT(!isArrayKey);
    serializer->writeNull();                break;
  case KindOfBoolean:
    ASSERT(!isArrayKey);
    serializer->write(m_data.num != 0);     break;
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    serializer->write(m_data.num);          break;
  case KindOfDouble:
    serializer->write(m_data.dbl);          break;
  case KindOfStaticString:
  case KindOfString:
    serializer->write(m_data.pstr->data(),
                      m_data.pstr->size(), isArrayKey);
    break;
  case KindOfArray:
    ASSERT(!isArrayKey);
    m_data.parr->serialize(serializer);     break;
  case KindOfObject:
    ASSERT(!isArrayKey);
    m_data.pobj->serialize(serializer);     break;
  default:
    ASSERT(false);
    break;
  }
}

void Variant::unserialize(VariableUnserializer *unserializer) {
  std::istream &in = unserializer->in();
  char type, sep;
  in >> type >> sep;

  if (type != 'R') {
    unserializer->add(this);
  }

  if (type == 'N') {
    // ASSERT(isNull());
    if(sep != ';') throw Exception("Expected ';' but got '%c'", sep);
    return;
  }
  if (sep != ':') {
    throw Exception("Expected ':' but got '%c'", sep);
  }

  switch (type) {
  case 'r':
    {
      int64 id;
      in >> id;
      Variant *v = unserializer->get(id);
      if (v == NULL) {
        throw Exception("Id %ld out of range", id);
      }
      operator=(*v);
    }
    break;
  case 'R':
    {
      int64 id;
      in >> id;
      Variant *v = unserializer->get(id);
      if (v == NULL) {
        throw Exception("Id %ld out of range", id);
      }
      operator=(ref(*v));
    }
    break;
  case 'b': { int64 v;  in >> v; operator=((bool)v); } break;
  case 'i': { int64 v;  in >> v; operator=(v);       } break;
  case 'd':
    {
      double v;
      char ch = in.peek();
      bool negative = false;
      char buf[4];
      if (ch == '-') {
        negative = true;
        in >> ch;
        ch = in.peek();
      }
      if (ch == 'I') {
        in.read(buf, 3); buf[3] = '\0';
        if (strcmp(buf, "INF")) {
          throw Exception("Expected 'INF' but got '%s'", buf);
        }
        v = atof("inf");
      } else if (ch == 'N') {
        in.read(buf, 3); buf[3] = '\0';
        if (strcmp(buf, "NAN")) {
          throw Exception("Expected 'NAN' but got '%s'", buf);
        }
        v = atof("nan");
      } else {
        in >> v;
      }
      operator=(negative ? -v : v);
    }
    break;
  case 's':
    {
      String v;
      v.unserialize(in);
      operator=(v);
    }
    break;
  case 'S':
    if (unserializer->getType() == VariableUnserializer::APCSerialize) {
      union {
        char buf[8];
        StringData *sd;
      } u;
      in.read(u.buf, 8);
      operator=(u.sd);
    } else {
      throw Exception("Unknown type '%c'", type);
    }
    break;
  case 'a':
    {
      Array v = Array::Create();
      v.unserialize(unserializer);
      operator=(v);
      return; // array has '}' terminating
    }
    break;
  case 'A':
    if (unserializer->getType() == VariableUnserializer::APCSerialize) {
      union {
        char buf[8];
        ArrayData *ad;
      } u;
      in.read(u.buf, 8);
      operator=(u.ad);
    } else {
      throw Exception("Unknown type '%c'", type);
    }
    break;
  case 'o':
    {
      String clsName;
      clsName.unserialize(in);

      in >> sep;
      if (sep != ':') {
        throw Exception("Expected ':' but got '%c'", sep);
      }

      Object obj;
      try {
        obj = create_object(clsName.data(), Array::Create(), false);
      } catch (ClassNotFoundException &e) {
        ASSERT(false);
      }
      operator=(obj);

      Array v = Array::Create();
      v.unserialize(unserializer);
      obj->o_setArray(v);

      obj->t___wakeup();
      return; // array has '}' terminating
    }
    break;
  case 'O':
    {
      String clsName;
      clsName.unserialize(in);

      in >> sep;
      if (sep != ':') {
        throw Exception("Expected ':' but got '%c'", sep);
      }

      Object obj;
      try {
        obj = create_object(clsName.data(), Array::Create(), false);
      } catch (ClassNotFoundException &e) {
        obj = create_object("__PHP_Incomplete_Class", Array::Create(), false);
        obj->o_set("__PHP_Incomplete_Class_Name", clsName);
      }
      operator=(obj);
      int64 size;
      char sep;
      in >> size >> sep;
      if (sep != ':') {
        throw Exception("Expected ':' but got '%c'", sep);
      }
      in >> sep;
      if (sep != '{') {
        throw Exception("Expected '{' but got '%c'", sep);
      }
      if (size > 0) {
        for (int64 i = 0; i < size; i++) {
          String key = unserializer->unserializeKey().toString();
          int subLen = 0;
          if (key.charAt(0) == '\00') {
            if (key.charAt(1) == '*') {
              subLen = 3; // protected
            } else {
              subLen = key.find('\00', 1) + 1; // private, skipping class name
              if (subLen == String::npos) {
                throw Exception("Mangled private object property");
              }
            }
          }
          Variant tmp;
          Variant &value = subLen != 0 ?
            (key.charAt(1) == '*' ?
             obj->o_lval(key.substr(subLen), tmp, clsName) :
             obj->o_lval(key.substr(subLen), tmp,
                         String(key.data() + 1, subLen - 2, AttachLiteral)))
            : obj->o_lval(key, tmp);
          value.unserialize(unserializer);
        }
      }
      in >> sep;
      if (sep != '}') {
        throw Exception("Expected '}' but got '%c'", sep);
      }

      obj->t___wakeup();
      return; // object has '}' terminating
    }
    break;
  case 'C':
    {
      String clsName;
      clsName.unserialize(in);

      in >> sep;
      if (sep != ':') {
        throw Exception("Expected ':' but got '%c'", sep);
      }

      Object obj = create_object(clsName.data(), Array::Create(), false);
      if (!obj->o_instanceof("Serializable")) {
        raise_error("%s didn't implement Serializable", clsName.data());
      }
      operator=(obj);

      String serialized;
      serialized.unserialize(in, '{', '}');
      obj->o_invoke_mil("unserialize",
                    CREATE_VECTOR1(serialized), -1);

      return; // object has '}' terminating
    }
    break;
  default:
    throw Exception("Unknown type '%c'", type);
  }
  in >> sep;
  if (sep != ';') {
    throw Exception("Expected ';' but got '%c'", sep);
  }
}

Variant Variant::share(bool save) const {
  if (m_type == KindOfVariant) {
    return m_data.pvar->share(save);
  }

  switch (m_type) {
  case KindOfNull:    return false; // same as non-existent
  case KindOfBoolean: return (m_data.num != 0);
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:   return m_data.num;
  case KindOfDouble:  return m_data.dbl;
  case KindOfStaticString:
  case KindOfString:
    return String(m_data.pstr->data(), m_data.pstr->size(), CopyString);
  case KindOfArray:
    {
      Array ret;
      for (ArrayIter iter(m_data.parr); iter; ++iter) {
        ret.set(iter.first().share(save), iter.second().share(save));
      }
      return ret;
    }
    break;
  case KindOfObject:
    if (save) {
      // we have to return an object so to remember its type
      c_stdClass *obj = NEW(c_stdClass)();
      obj->o_set(s_s, f_serialize(*this));
      return obj;
    } else {
      return f_unserialize(m_data.pobj->o_get(s_s));
    }
    break;
  default:
    ASSERT(false);
    break;
  }

  return false; // same as non-existent
}

SharedVariant *Variant::getSharedVariant() const {
  if (m_type == KindOfVariant) {
    return m_data.pvar->getSharedVariant();
  }
  if (m_type == KindOfString) {
    return m_data.pstr->getSharedVariant();
  }
  if (m_type == KindOfArray) {
    return m_data.parr->getSharedVariant();
  }
  return NULL;
}

Variant Variant::fiberMarshal(FiberReferenceMap &refMap) const {
  if (m_type == KindOfVariant) {
    Variant *mpvar = m_data.pvar;
    if (mpvar->getCount() > 1) {
      Variant *pvar = (Variant*)refMap.lookup(mpvar);
      if (pvar == NULL) {
        pvar = NEW(Variant)();
        refMap.insert(mpvar, pvar); // ahead of deep copy
        *pvar = mpvar->fiberMarshal(refMap);
      }
      pvar->incRefCount();
      return pvar;
    }
    return mpvar->fiberMarshal(refMap);
  }

  switch (m_type) {
  case KindOfNull:    return Variant();
  case KindOfBoolean: return (m_data.num != 0);
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:   return m_data.num;
  case KindOfDouble:  return m_data.dbl;
  case KindOfStaticString:
  case KindOfString:
    return String(m_data.pstr).fiberCopy();
  case KindOfArray:
    return Array(m_data.parr).fiberMarshal(refMap);
  case KindOfObject:
    return m_data.pobj->fiberMarshal(refMap);
  default:
    ASSERT(false);
    break;
  }

  return Variant();
}

Variant Variant::fiberUnmarshal(FiberReferenceMap &refMap) const {
  if (m_type == KindOfVariant) {
    Variant *mpvar = m_data.pvar;
    if (mpvar->getCount() > 1) {
      // marshaling back to original thread
      Variant *pvar = (Variant*)refMap.lookup(mpvar);
      if (pvar == NULL) {
        // was i in original thread?
        pvar = (Variant*)refMap.reverseLookup(mpvar);
        if (pvar == NULL) {
          pvar = NEW(Variant)();
        }
        refMap.insert(mpvar, pvar); // ahead of deep copy
        *pvar = mpvar->fiberUnmarshal(refMap);
      }
      pvar->incRefCount();
      return pvar;
    }

    // i'm actually a weakly bound variant
    return mpvar->fiberUnmarshal(refMap);
  }

  switch (m_type) {
  case KindOfNull:    return Variant();
  case KindOfBoolean: return (m_data.num != 0);
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:   return m_data.num;
  case KindOfDouble:  return m_data.dbl;
  case KindOfStaticString:
  case KindOfString:
    return String(m_data.pstr).fiberCopy();
  case KindOfArray:
    return Array(m_data.parr).fiberUnmarshal(refMap);
  case KindOfObject:
    return m_data.pobj->fiberUnmarshal(refMap);
  default:
    ASSERT(false);
    break;
  }

  return Variant();
}

const char *Variant::getTypeString(DataType type) {
  switch (type) {
  case KindOfNull:    return "KindOfNull";
  case KindOfBoolean: return "KindOfBoolean";
  case KindOfByte:    return "KindOfByte";
  case KindOfInt16:   return "KindOfInt16";
  case KindOfInt32:   return "KindOfInt32";
  case KindOfInt64:   return "KindOfInt64";
  case KindOfDouble:  return "KindOfDouble";
  case KindOfStaticString:  return "KindOfStaticString";
  case KindOfString:  return "KindOfString";
  case KindOfArray:   return "KindOfArray";
  case KindOfObject:  return "KindOfObject";
  case KindOfVariant: return "KindOfVariant";
  default:
    ASSERT(false);
    break;
  }
  return "";
}

std::string Variant::getDebugDump() const {
  char buf[1024];
  snprintf(buf, sizeof(buf), "[%s: %p]", getTypeString(m_type), m_data.pstr);
  return buf;
}

void Variant::dump() const {
  VariableSerializer vs(VariableSerializer::VarDump);
  Variant ret(vs.serialize(*this, true));
  printf("Variant: %s", ret.toString().data());
}

template<>
Variant AssignOp<T_CONCAT_EQUAL>::assign(Variant &var, CVarRef val) {
  return concat_assign(var, val);
}

template<>
Variant AssignOp<T_PLUS_EQUAL>::assign(Variant &var, CVarRef val) {
  return var += val;
}

template<>
Variant AssignOp<T_MINUS_EQUAL>::assign(Variant &var, CVarRef val) {
  return var -= val;
}

template<>
Variant AssignOp<T_MUL_EQUAL>::assign(Variant &var, CVarRef val) {
  return var *= val;
}

template<>
Variant AssignOp<T_DIV_EQUAL>::assign(Variant &var, CVarRef val) {
  return var /= val;
}

template<>
Variant AssignOp<T_MOD_EQUAL>::assign(Variant &var, CVarRef val) {
  return var %= val;
}

template<>
Variant AssignOp<T_AND_EQUAL>::assign(Variant &var, CVarRef val) {
  return var &= val;
}

template<>
Variant AssignOp<T_OR_EQUAL>::assign(Variant &var, CVarRef val) {
  return var |= val;
}

template<>
Variant AssignOp<T_XOR_EQUAL>::assign(Variant &var, CVarRef val) {
  return var ^= val;
}

template<>
Variant AssignOp<T_SL_EQUAL>::assign(Variant &var, CVarRef val) {
  return var <<= val;
}

template<>
Variant AssignOp<T_SR_EQUAL>::assign(Variant &var, CVarRef val) {
  return var >>= val;
}

template<>
Variant AssignOp<T_INC>::assign(Variant &var, CVarRef val) {
  return val.isNull() ? ++var : var++;
}

template<>
Variant AssignOp<T_DEC>::assign(Variant &var, CVarRef val) {
  return val.isNull() ? --var : var--;
}

template<typename T, int op>
T Variant::o_assign_op(CStrRef propName, CVarRef val,
                       CStrRef context /* = null_string */) {
  if (propName.empty()) {
    throw EmptyObjectPropertyException();
  }

  if (m_type == KindOfObject) {
  } else if (m_type == KindOfVariant) {
    return (T)m_data.pvar->template o_assign_op<T,op>(propName, val, context);
  } else if (isObjectConvertable()) {
    set(Object(NEW(c_stdClass)()));
  } else {
    // Raise a warning
    raise_warning("Attempt to assign property of non-object");
    Variant tmp;
    return (T)tmp.template o_assign_op<T,op>(propName, val, context);
  }
  return (T)m_data.pobj->template o_assign_op<T,op>(propName, val, context);
}

template<typename T, int op>
T Object::o_assign_op(CStrRef propName, CVarRef val,
                      CStrRef context /* = null_string */) {
  if (propName.empty()) {
    throw EmptyObjectPropertyException();
  }
  ObjectData *obj = m_px;
  if (!obj) {
    obj = NEW(c_stdClass)();
    SmartPtr<ObjectData>::operator=(obj);
  }

  return obj->template o_assign_op<T,op>(propName, val, context);
}

template<typename T, int op>
T ObjectData::o_assign_op(CStrRef propName, CVarRef val,
                          CStrRef context /* = null_string */) {
  bool useGet = getAttribute(ObjectData::UseGet);
  bool useSet = getAttribute(ObjectData::UseSet);
  int flags = useSet ? ObjectData::RealPropWrite :
    ObjectData::RealPropCreate | ObjectData::RealPropWrite;

  if (Variant *t = o_realProp(propName, flags, context)) {
    if (useGet && !t->isInitialized()) {
      AttributeClearer a(ObjectData::UseGet, this);
      *t = t___get(propName);
    }

    return (T)AssignOp<op>::assign(*t, val);
  }

  ASSERT(useSet);
  Variant var;
  if (useGet) {
    AttributeClearer a(ObjectData::UseGet, this);
    var = t___get(propName);
  }

  Variant ret = AssignOp<op>::assign(var, val);
  AttributeClearer a(ObjectData::UseSet, this);
  t___set(propName, var);
  return (T)ret;
}

#define DECLARE_O_ASSIGN_OP_ONE(C,T,op)                         \
  template T                                                  \
  C::o_assign_op<T,op>(CStrRef propName, CVarRef val,           \
                       CStrRef context /* = null_string */)

#define DECLARE_O_ASSIGN_OP(op)                 \
  DECLARE_O_ASSIGN_OP_ONE(Object,void,op);      \
  DECLARE_O_ASSIGN_OP_ONE(Object,Variant,op);   \
  DECLARE_O_ASSIGN_OP_ONE(Variant,void,op);     \
  DECLARE_O_ASSIGN_OP_ONE(Variant,Variant,op)

DECLARE_O_ASSIGN_OP(T_CONCAT_EQUAL);
DECLARE_O_ASSIGN_OP(T_PLUS_EQUAL);
DECLARE_O_ASSIGN_OP(T_MINUS_EQUAL);
DECLARE_O_ASSIGN_OP(T_MUL_EQUAL);
DECLARE_O_ASSIGN_OP(T_DIV_EQUAL);
DECLARE_O_ASSIGN_OP(T_MOD_EQUAL);
DECLARE_O_ASSIGN_OP(T_AND_EQUAL);
DECLARE_O_ASSIGN_OP(T_OR_EQUAL);
DECLARE_O_ASSIGN_OP(T_XOR_EQUAL);
DECLARE_O_ASSIGN_OP(T_SL_EQUAL);
DECLARE_O_ASSIGN_OP(T_SR_EQUAL);
DECLARE_O_ASSIGN_OP(T_INC);
DECLARE_O_ASSIGN_OP(T_DEC);
///////////////////////////////////////////////////////////////////////////////
}
