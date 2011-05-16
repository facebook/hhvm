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
#include <runtime/base/complex_types.h>
#include <runtime/base/comparisons.h>
#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/variable_unserializer.h>
#include <runtime/base/externals.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/fiber_reference_map.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/array/array_iterator.h>
#include <util/parser/hphp.tab.hpp>

#include <system/lib/systemlib.h>

using namespace std;

namespace HPHP {

const Variant null_variant = Variant();
const VarNR null_varNR = VarNR();

IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(Variant);

///////////////////////////////////////////////////////////////////////////////
// static strings

static StaticString s_offsetGet("offsetGet");
static StaticString s_offsetSet("offsetSet");
static StaticString s_offsetUnset("offsetUnset");
static StaticString s_s("s");
static StaticString s_scalar("scalar");
static StaticString s_array("Array");
static StaticString s_1("1");

///////////////////////////////////////////////////////////////////////////////
// local helpers

static int64 ToKey(bool i) { return (int64)i; }
static int64 ToKey(int64 i) { return i; }
static int64 ToKey(double d) { return (int64)d; }
static VarNR ToKey(CStrRef s) { return s.toKey(); }
static VarNR ToKey(CVarRef v) { return v.toKey(); }

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
    if (s->isStatic()) {
      m_type = KindOfStaticString;
    } else {
      s->incRefCount();
    }
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(const std::string & v) : _count(0), m_type(KindOfString) {
  StringData *s = NEW(StringData)(v.c_str(), v.size(), CopyString);
  ASSERT(s);
  m_data.pstr = s;
  s->incRefCount();
}

__attribute__ ((section (".text.hot")))
Variant::Variant(CArrRef v) : _count(0), m_type(KindOfArray) {
  ArrayData *a = v.get();
  if (a) {
    m_data.parr = a;
    a->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(CObjRef v) : _count(0), m_type(KindOfObject) {
  ObjectData *o = v.get();
  if (o) {
    m_data.pobj = o;
    o->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(StringData *v) : _count(0), m_type(KindOfString) {
  if (v) {
    m_data.pstr = v;
    if (v->isStatic()) {
      m_type = KindOfStaticString;
    } else {
      v->incRefCount();
    }
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(ArrayData *v) : _count(0), m_type(KindOfArray) {
  if (v) {
    m_data.parr = v;
    v->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(ObjectData *v) : _count(0), m_type(KindOfObject) {
  if (v) {
    m_data.pobj = v;
    v->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(Variant *v) : _count(0), m_type(KindOfVariant) {
  if (v) {
    m_data.pvar = v;
  } else {
    m_type = KindOfNull;
  }
}

// the version of the high frequency function that is not inlined
__attribute__ ((section (".text.hot")))
Variant::Variant(CVarRef v) {
  constructValHelper(v);
}

Variant::Variant(CVarStrongBind v) {
  constructRefHelper(variant(v));
}

Variant::Variant(CVarWithRefBind v) {
  constructWithRefHelper(variant(v), 0);
}

void Variant::reset() {
  m_type = KindOfNull;
}

#ifdef FAST_REFCOUNT_FOR_VARIANT
static void destructString(void *p)  { ((StringData *)p)->release(); }
static void destructArray(void *p)   { ((ArrayData *)p)->release();  }
static void destructObject(void *p)  { ((ObjectData *)p)->release(); }
static void destructVariant(void *p) { ((Variant *)p)->release();    }

static void (*destructors[4])(void *) =
  {destructString, destructArray, destructObject, destructVariant};
#endif

__attribute__ ((section (".text.hot")))
void Variant::destruct() {
  ASSERT(!isPrimitive());
#ifdef FAST_REFCOUNT_FOR_VARIANT
  /**
   * This is safe because we have compile time assertions that guarantee that
   * the _count field will always be exactly FAST_REFCOUNT_OFFSET bytes from
   * the beginning of the object for the StringData, ArrayData, ObjectData,
   * and Variant classes.
   */
  CT_ASSERT(KindOfString + 1 == KindOfArray &&
            KindOfArray + 1 == KindOfObject &&
            KindOfObject + 1 == KindOfVariant);
  if (m_data.pvar->decRefCount() == 0) {
    ASSERT(m_type >= KindOfString && m_type <= KindOfVariant);
    destructors[m_type - KindOfString]((void *)m_data.pvar);
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

__attribute__ ((section (".text.hot")))
Variant &Variant::assign(CVarRef v) {
  AssignValHelper(this, &v);
  return *this;
}

Variant &Variant::assignRef(CVarRef v) {
  assignRefHelper(v);
  return *this;
}

Variant &Variant::setWithRef(CVarRef v, const ArrayData *arr /* = NULL */) {
  setWithRefHelper(v, arr, IS_REFCOUNTED_TYPE(m_type));
  return *this;
}

void Variant::setNull() {
  if (isPrimitive()) {
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
    destruct();
  }
  m_type = KindOfBoolean;
  m_data.num = (v ? 1 : 0);
  return *this;
}

CVarRef Variant::set(int v) {
  if (isPrimitive()) {
    // do nothing
  } else if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  } else {
    destruct();
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
    destruct();
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
    destruct();
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
    destruct();
  }
  m_type = KindOfString;
  m_data.pstr = NEW(StringData)(v);
  m_data.pstr->incRefCount();
  return *this;
}

CVarRef Variant::set(StringData *v) {
  Variant *self = m_type == KindOfVariant ? m_data.pvar : this;
  if (UNLIKELY(!v)) {
    self->setNull();
  } else {
    v->incRefCount();
    if (IS_REFCOUNTED_TYPE(self->m_type)) self->destruct();
    self->m_type = v->isStatic() ? KindOfStaticString : KindOfString;
    self->m_data.pstr = v;
  }
  return *this;
}

CVarRef Variant::set(const StaticString & v) {
  if (isPrimitive()) {
    // do nothing
  } else if (m_type == KindOfVariant) {
    m_data.pvar->set(v);
    return *this;
  } else {
    destruct();
  }
  StringData *s = v.get();
  ASSERT(s);
  m_type = KindOfStaticString;
  m_data.pstr = s;
  return *this;
}

CVarRef Variant::set(ArrayData *v) {
  Variant *self = m_type == KindOfVariant ? m_data.pvar : this;
  if (UNLIKELY(!v)) {
    self->setNull();
  } else {
    v->incRefCount();
    if (IS_REFCOUNTED_TYPE(self->m_type)) self->destruct();
    self->m_type = KindOfArray;
    self->m_data.parr = v;
  }
  return *this;
}

CVarRef Variant::set(ObjectData *v) {
  Variant *self = m_type == KindOfVariant ? m_data.pvar : this;
  if (UNLIKELY(!v)) {
    self->setNull();
  } else {
    v->incRefCount();
    if (IS_REFCOUNTED_TYPE(self->m_type)) self->destruct();
    self->m_type = KindOfObject;
    self->m_data.pobj = v;
  }
  return *this;
}

void Variant::init(ObjectData *v) {
  if (v) {
    m_type = KindOfObject;
    m_data.pobj = v;
    v->incRefCount();
  } else {
    m_type = KindOfNull;
  }
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

int64 Variant::hashForIntSwitch(int64 firstNonZero, int64 noMatch) const {
  switch (m_type) {
  case KindOfInt32:
  case KindOfInt64:
    return m_data.num;
  case KindOfBoolean:
    return m_data.num ? firstNonZero : 0;
  case KindOfDouble:
    return Variant::DoubleHashForIntSwitch(m_data.dbl, noMatch);
  case KindOfUninit:
  case KindOfNull:
    // take care of the NULLs here, so below we can assume
    // a non null m_data field
    return 0;
  case KindOfStaticString:
  case KindOfString:
    return m_data.pstr->hashForIntSwitch(firstNonZero, noMatch);
  case KindOfArray:
    return noMatch;
  case KindOfObject:
    return m_data.pobj->o_toInt64();
  case KindOfVariant:
    return m_data.pvar->hashForIntSwitch(firstNonZero, noMatch);
  default:
    break;
  }
  ASSERT(false);
  return 0;
}

int64 Variant::DoubleHashForIntSwitch(double dbl, int64 noMatch) {
  // only matches an int if it is integral, ie
  // "50.00" -> 50
  // "50.12" -> no match
  int64 t = (int64) dbl;
  return t == dbl ? t : noMatch;
}

int64 Variant::hashForStringSwitch(
    int64 firstTrueCaseHash,
    int64 firstNullCaseHash,
    int64 firstFalseCaseHash,
    int64 firstZeroCaseHash,
    int64 firstHash,
    int64 noMatchHash,
    bool &needsOrder) const {
  switch (m_type) {
  case KindOfInt32:
  case KindOfInt64:
    needsOrder = false;
    return m_data.num == 0 ? firstZeroCaseHash : m_data.num;
  case KindOfBoolean:
    needsOrder = false;
    return m_data.num ? firstTrueCaseHash : firstFalseCaseHash;
  case KindOfDouble:
    needsOrder = false;
    return m_data.dbl == 0 ? firstZeroCaseHash : toInt64();
  case KindOfUninit:
  case KindOfNull:
    // take care of the NULLs here, so below we can assume
    // a non null m_data field
    needsOrder = false;
    return firstNullCaseHash;
  case KindOfStaticString:
  case KindOfString:
    return m_data.pstr->hashForStringSwitch(
        firstTrueCaseHash, firstNullCaseHash, firstFalseCaseHash,
        firstZeroCaseHash, firstHash, noMatchHash, needsOrder);
  case KindOfArray:
    needsOrder = false;
    return noMatchHash;
  case KindOfObject:
    needsOrder = true;
    return firstHash;
  case KindOfVariant:
    return m_data.pvar->hashForStringSwitch(
        firstTrueCaseHash, firstNullCaseHash, firstFalseCaseHash,
        firstZeroCaseHash, firstHash, noMatchHash, needsOrder);
  default:
    break;
  }
  ASSERT(false);
  return 0;
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
  case KindOfUninit:
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

bool Variant::instanceof(CStrRef s) const {
  if (m_type == KindOfObject) {
    ASSERT(m_data.pobj);
    return m_data.pobj->o_instanceof(s);
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
    if (arr->getCount() > 1 && !arr->isHead()
     && !arr->isGlobalArrayWrapper()) {
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
    if (arr->getCount() > 1 && !arr->isInvalid()
     && !arr->isGlobalArrayWrapper()) {
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

Variant Variant::array_iter_current_ref() {
  if (is(KindOfArray)) {
    escalate(true);
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1 && !arr->isGlobalArrayWrapper()) {
      arr = arr->copy();
      set(arr);
      ASSERT(arr == getArrayData());
    }
    return strongBind(arr->currentRef());
  }
  throw_bad_type_exception("expecting an array");
  return false;
}

Variant Variant::array_iter_next() {
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1 && !arr->isInvalid()
     && !arr->isGlobalArrayWrapper()) {
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
    if (arr->getCount() > 1 && !arr->isTail()
     && !arr->isGlobalArrayWrapper()) {
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
    if (arr->getCount() > 1 && !arr->isInvalid()
     && !arr->isGlobalArrayWrapper()) {
      arr = arr->copy();
      set(arr);
      ASSERT(arr == getArrayData());
    }
    return arr->each();
  }
  throw_bad_type_exception("expecting an array");
  return null_variant;
}

void Variant::array_iter_dirty_set() const {
  if (is(KindOfArray)) {
    getArrayData()->iter_dirty_set();
  }
}

void Variant::array_iter_dirty_reset() const {
  if (is(KindOfArray)) {
    getArrayData()->iter_dirty_reset();
  }
}

void Variant::array_iter_dirty_check() const {
  if (is(KindOfArray)) {
    getArrayData()->iter_dirty_check();
  }
}

inline DataType Variant::convertToNumeric(int64 *lval, double *dval) const {
  StringData *s = getStringData();
  ASSERT(s);
  return s->isNumericWithVal(*lval, *dval, 1);
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
    int64 lval; double dval;
    DataType ret = convertToNumeric(&lval, &dval);
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
  int na = is(KindOfArray) + var.is(KindOfArray);
  if (na == 2) {
    return toArray() + var.toArray();
  } else if (na) {
    throw BadArrayMergeException();
  }
  if (isDouble() || var.isDouble()) {
    return toDouble() + var.toDouble();
  }
  if (isString()) {
    int64 lval; double dval;
    DataType ret = convertToNumeric(&lval, &dval);
    if (ret == KindOfDouble) {
      return dval + var.toDouble();
    }
  }
  if (var.isString()) {
    int64 lval; double dval;
    DataType ret = var.convertToNumeric(&lval, &dval);
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
  int na = is(KindOfArray) + var.is(KindOfArray);
  if (na == 2) {
    ArrayData *arr1 = getArrayData();
    ArrayData *arr2 = var.getArrayData();
    if (arr1 == NULL || arr2 == NULL) {
      throw BadArrayMergeException();
    }
    if (arr2->empty() || arr1 == arr2) return *this;
    if (arr1->empty()) {
      set(arr2);
      return *this;
    }
    ArrayData *escalated = arr1->append(arr2, ArrayData::Plus,
                                        arr1->getCount() > 1);
    if (escalated) set(escalated);
    return *this;
  } else if (na) {
    throw BadArrayMergeException();
  }
  if (isDouble() || var.isDouble()) {
    set(toDouble() + var.toDouble());
    return *this;
  }
  if (isString()) {
    int64 lval; double dval;
    DataType ret = convertToNumeric(&lval, &dval);
    if (ret == KindOfDouble) {
      set(dval + var.toDouble());
      return *this;
    }
  }
  if (var.isString()) {
    int64 lval; double dval;
    DataType ret = var.convertToNumeric(&lval, &dval);
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
    int64 lval; double dval;
    DataType ret = convertToNumeric(&lval, &dval);
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
      int64 lval; double dval;
      DataType ret = convertToNumeric(&lval, &dval);
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
    int64 lval; double dval;
    DataType ret = convertToNumeric(&lval, &dval);
    if (ret == KindOfDouble) {
      return dval - var.toDouble();
    }
  }
  if (var.isString()) {
    int64 lval; double dval;
    DataType ret = var.convertToNumeric(&lval, &dval);
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
      int64 lval; double dval;
      DataType ret = convertToNumeric(&lval, &dval);
      if (ret == KindOfDouble) {
        set(dval - var.toDouble());
        return *this;
      }
    }
    if (var.isString()) {
      int64 lval; double dval;
      DataType ret = var.convertToNumeric(&lval, &dval);
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
      int64 lval; double dval;
      DataType ret = convertToNumeric(&lval, &dval);
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
    int64 lval; double dval;
    DataType ret = convertToNumeric(&lval, &dval);
    if (ret == KindOfDouble) {
      return dval * var.toDouble();
    }
  }
  if (var.isString()) {
    int64 lval; double dval;
    DataType ret = var.convertToNumeric(&lval, &dval);
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
      int64 lval; double dval;
      DataType ret = convertToNumeric(&lval, &dval);
      if (ret == KindOfDouble) {
        set(dval * var.toDouble());
        return *this;
      }
    }
    if (var.isString()) {
      int64 lval; double dval;
      DataType ret = var.convertToNumeric(&lval, &dval);
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
      int64 lval; double dval;
      DataType ret = convertToNumeric(&lval, &dval);
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
    DataType ret = convertToNumeric(&lval, &dval);
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
    DataType ret = var.convertToNumeric(&lval2, &dval2);
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
    DataType ret = convertToNumeric(&lval, &dval);
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
    DataType ret = var.convertToNumeric(&lval2, &dval2);
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
      int64 lval; double dval;
      DataType ret = convertToNumeric(&lval, &dval);
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
  case KindOfUninit:
  case KindOfNull:   set(1LL); break;
  case KindOfInt32:
  case KindOfInt64:  set(toInt64() + 1);  break;
  case KindOfDouble: set(toDouble() + 1); break;
  case KindOfStaticString:
  case KindOfString:
    {
      if (getStringData()->empty()) {
        set("1");
      } else {
        int64 lval; double dval;
        DataType ret = convertToNumeric(&lval, &dval);
        switch (ret) {
        case KindOfInt64:  set(lval + 1); break;
        case KindOfDouble: set(dval + 1); break;
        case KindOfUninit:
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
  case KindOfInt32:
  case KindOfInt64:  set(toInt64() - 1);  break;
  case KindOfDouble: set(toDouble() - 1); break;
  case KindOfStaticString:
  case KindOfString:
    {
      if (getStringData()->empty()) {
        set(-1LL);
      } else {
        int64 lval; double dval;
        DataType ret = convertToNumeric(&lval, &dval);
        switch (ret) {
        case KindOfInt64:  set(lval - 1);   break;
        case KindOfDouble: set(dval - 1);   break;
        case KindOfUninit:
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

ArrayIter Variant::begin(CStrRef context /* = null_string */,
                         bool setIterDirty /* = false */) const {
  if (is(KindOfArray)) {
    if (setIterDirty) array_iter_dirty_set();
    return ArrayIter(getArrayData());
  }
  if (is(KindOfObject)) {
    return getObjectData()->begin(context);
  }
  raise_warning("Invalid argument supplied for foreach()");
  return ArrayIter();
}

MutableArrayIter Variant::begin(Variant *key, Variant &val,
                                CStrRef context /* = null_string */,
                                bool setIterDirty /* = false */) {
  if (is(KindOfObject)) {
    return getObjectData()->begin(key, val, context);
  }
  // we are about to modify an array that has other weak references, so
  // we have to make a copy to preserve other instances
  if (is(KindOfArray)) {
    if (setIterDirty) array_iter_dirty_set();
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1 && !arr->isGlobalArrayWrapper()) {
      set(arr->copy());
    }
  }
  return MutableArrayIter(this, key, val);
}

void Variant::escalate(bool mutableIteration /* = false */) {
  TypedValueAccessor tva = getTypedAccessor();
  if (GetAccessorType(tva) == KindOfArray) {
    ArrayData *arr = GetArrayData(tva);
    ArrayData *esc = arr->escalate(mutableIteration);
    if (arr != esc) set(esc);
  }
}

///////////////////////////////////////////////////////////////////////////////
// type conversions

__attribute__ ((section (".text.hot")))
bool Variant::toBooleanHelper() const {
  ASSERT(m_type > KindOfInt64);
  switch (m_type) {
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
  ASSERT(m_type > KindOfInt64);
  switch (m_type) {
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

double Variant::toDoubleHelper() const {
  switch (m_type) {
  case KindOfUninit:
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

String Variant::toStringHelper() const {
  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:    return empty_string;
  case KindOfBoolean: return m_data.num ? s_1 : empty_string;
  case KindOfDouble:  return m_data.dbl;
  case KindOfStaticString:
  case KindOfString:
    ASSERT(false); // Should be done in caller
    return m_data.pstr;
  case KindOfArray:   return s_array;
  case KindOfObject:  return m_data.pobj->t___tostring();
  case KindOfVariant: return m_data.pvar->toString();
  default:
    break;
  }
  return m_data.num;
}

Array Variant::toArrayHelper() const {
  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:    return Array::Create();
  case KindOfInt64:   return Array::Create(m_data.num);
  case KindOfStaticString:
  case KindOfString:  return Array::Create(m_data.pstr);
  case KindOfArray:   return m_data.parr;
  case KindOfObject:  return m_data.pobj->o_toArray();
  case KindOfVariant: return m_data.pvar->toArray();
  default:
    break;
  }
  return Array::Create(*this);
}

Object Variant::toObjectHelper() const {
  if (m_type == KindOfVariant) return m_data.pvar->toObject();

  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:
    break;
  case KindOfBoolean:
  case KindOfInt32:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfStaticString:
  case KindOfString:
    {
      ObjectData *obj = SystemLib::AllocStdClassObject();
      obj->o_set(s_scalar, *this, false);
      return obj;
    }
  case KindOfArray:   return m_data.parr->toObject();
  case KindOfObject:  return m_data.pobj;
  default:
    ASSERT(false);
    break;
  }
  return Object(SystemLib::AllocStdClassObject());
}

__attribute__ ((section (".text.hot")))
VarNR Variant::toKey() const {
  if (m_type == KindOfString || m_type == KindOfStaticString) {
    int64 n;
    if (m_data.pstr->isStrictlyInteger(n)) {
      return n;
    } else {
      return m_data.pstr;
    }
  }
  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:
    return empty_string;
  case KindOfBoolean:
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
  return null_varNR;
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
  return isBoolean() && HPHP::equal(v2, getBoolean());
}

bool Variant::same(int v2) const {
  return same((int64)v2);
}

bool Variant::same(int64 v2) const {
  TypedValueAccessor acc = getTypedAccessor();
  switch (GetAccessorType(acc)) {
  case KindOfInt32:
  case KindOfInt64:
    return HPHP::equal(v2, GetInt64(acc));
  default:
    break;
  }
  return false;
}

bool Variant::same(double v2) const {
  return isDouble() && HPHP::equal(v2, getDouble());
}

bool Variant::same(litstr v2) const {
  StringData sd2(v2);
  return same(&sd2);
}

bool Variant::same(const StringData *v2) const {
  bool null1 = isNull();
  bool null2 = (v2 == NULL);
  if (null1 && null2) return true;
  if (null1 || null2) return false;
  return isString() && HPHP::same(getStringData(), v2);
}

bool Variant::same(CStrRef v2) const {
  return same(v2.get());
}

bool Variant::same(CArrRef v2) const {
  bool null1 = isNull();
  bool null2 = v2.isNull();
  if (null1 && null2) return true;
  if (null1 || null2) return false;
  return is(KindOfArray) && Array(getArrayData()).same(v2);
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

  TypedValueAccessor acc = getTypedAccessor();
  switch (GetAccessorType(acc)) {
  case KindOfInt32:
  case KindOfInt64: {
    TypedValueAccessor acc2 = v2.getTypedAccessor();
    switch (GetAccessorType(acc2)) {
    case KindOfInt32:
    case KindOfInt64:
      return HPHP::equal(GetInt64(acc), GetInt64(acc2));
    default:
      break;
    }
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    TypedValueAccessor acc2 = v2.getTypedAccessor();
    switch (GetAccessorType(acc2)) {
    case KindOfStaticString:
    case KindOfString:
      return GetStringData(acc)->same(v2.GetStringData(acc2));
    default:
      return false;
    }
  }
  case KindOfArray:
    if (v2.is(KindOfArray)) {
      return Array(getArrayData()).same(Array(v2.getArrayData()));
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

#define UNWRAP(reverse)                                                    \
  TypedValueAccessor acc = getTypedAccessor();                             \
  switch (GetAccessorType(acc)) {                                          \
  case KindOfUninit:                                                       \
  case KindOfNull:    return HPHP::reverse(v2, false);                     \
  case KindOfBoolean: return HPHP::reverse(v2, GetBoolean(acc));           \
  case KindOfInt32:                                                        \
  case KindOfInt64:   return HPHP::reverse(v2, GetInt64(acc));             \
  case KindOfDouble:  return HPHP::reverse(v2, GetDouble(acc));            \
  case KindOfStaticString:                                                 \
  case KindOfString:  return HPHP::reverse(v2, GetStringData(acc));        \
  case KindOfArray:   return HPHP::reverse(v2, Array(GetArrayData(acc)));  \
  case KindOfObject:  return HPHP::reverse(v2, Object(GetObjectData(acc)));\
  default:                                                                 \
    ASSERT(false);                                                         \
    break;                                                                 \
  }                                                                        \
  return false;                                                            \

// "null" needs to convert to "" before comparing with a string
#define UNWRAP_STR(reverse)                                                \
  TypedValueAccessor acc = getTypedAccessor();                             \
  switch (GetAccessorType(acc)) {                                          \
  case KindOfUninit:                                                       \
  case KindOfNull:    return HPHP::reverse(v2, empty_string);              \
  case KindOfBoolean: return HPHP::reverse(v2, GetBoolean(acc));           \
  case KindOfInt32:                                                        \
  case KindOfInt64:   return HPHP::reverse(v2, GetInt64(acc));             \
  case KindOfDouble:  return HPHP::reverse(v2, GetDouble(acc));            \
  case KindOfStaticString:                                                 \
  case KindOfString:  return HPHP::reverse(v2, GetStringData(acc));        \
  case KindOfArray:   return HPHP::reverse(v2, Array(GetArrayData(acc)));  \
  case KindOfObject:  return HPHP::reverse(v2, Object(GetObjectData(acc)));\
  default:                                                                 \
    ASSERT(false);                                                         \
    break;                                                                 \
  }                                                                        \
  return false;                                                            \

// Array needs to convert to "Array" and Object to String
#define UNWRAP_STRING(reverse)                                             \
  TypedValueAccessor acc = getTypedAccessor();                             \
  switch (GetAccessorType(acc)) {                                          \
  case KindOfUninit:                                                       \
  case KindOfNull:    return HPHP::reverse(v2, empty_string);              \
  case KindOfBoolean: return HPHP::reverse(v2, GetBoolean(acc));           \
  case KindOfInt32:                                                        \
  case KindOfInt64:   return HPHP::reverse(v2, GetInt64(acc));             \
  case KindOfDouble:  return HPHP::reverse(v2, GetDouble(acc));            \
  case KindOfStaticString:                                                 \
  case KindOfString:  return HPHP::reverse(v2, GetStringData(acc));        \
  case KindOfArray:   return HPHP::reverse(v2, s_array);                   \
  case KindOfObject:                                                       \
    return HPHP::reverse(v2, Object(GetObjectData(acc)).toString());       \
  default:                                                                 \
    ASSERT(false);                                                         \
    break;                                                                 \
  }                                                                        \
  return false;                                                            \

// "null" needs to convert to "" before comparing with a string
#define UNWRAP_VAR(forward, reverse)                                       \
  TypedValueAccessor acc = getTypedAccessor();                             \
  switch (GetAccessorType(acc)) {                                          \
  case KindOfUninit:                                                       \
  case KindOfNull:                                                         \
    if (v2.isString()) {                                                   \
      return HPHP::reverse(v2.getStringData(), empty_string);              \
    }                                                                      \
    return HPHP::reverse(v2, false);                                       \
  case KindOfBoolean: return HPHP::reverse(v2, GetBoolean(acc));           \
  case KindOfInt32:                                                        \
  case KindOfInt64:   return HPHP::reverse(v2, GetInt64(acc));             \
  case KindOfDouble:  return HPHP::reverse(v2, GetDouble(acc));            \
  case KindOfStaticString:                                                 \
  case KindOfString:  return HPHP::reverse(v2, GetStringData(acc));        \
  case KindOfArray:                                                        \
    if (v2.isArray()) {                                                    \
      return Array(GetArrayData(acc)).forward(Array(v2.getArrayData()));   \
    }                                                                      \
    return HPHP::reverse(v2, Array(GetArrayData(acc)));                    \
  case KindOfObject:  return HPHP::reverse(v2, Object(GetObjectData(acc)));\
  default:                                                                 \
    ASSERT(false);                                                         \
    break;                                                                 \
  }                                                                        \
  return false;                                                            \

// array comparison is directional when they are uncomparable
// also, ">" is implemented as "!<=" in Zend
#define UNWRAP_ARR(forward, reverse)                                       \
  TypedValueAccessor acc = getTypedAccessor();                             \
  switch (GetAccessorType(acc)) {                                          \
  case KindOfUninit:                                                       \
  case KindOfNull:    return HPHP::reverse(v2, false);                     \
  case KindOfBoolean: return HPHP::reverse(v2, GetBoolean(acc));           \
  case KindOfInt32:                                                        \
  case KindOfInt64:   return HPHP::reverse(v2, GetInt64(acc));             \
  case KindOfDouble:  return HPHP::reverse(v2, GetDouble(acc));            \
  case KindOfStaticString:                                                 \
  case KindOfString:  return HPHP::reverse(v2, GetStringData(acc));        \
  case KindOfArray:   return Array(GetArrayData(acc)).forward(v2);         \
  case KindOfObject:  return HPHP::reverse(v2, Object(GetObjectData(acc)));\
  default:                                                                 \
    ASSERT(false);                                                         \
    break;                                                                 \
  }                                                                        \
  return false;                                                            \

bool Variant::equal(bool    v2) const { UNWRAP(equal);}
bool Variant::equal(int     v2) const { UNWRAP(equal);}
bool Variant::equal(int64   v2) const { UNWRAP(equal);}
bool Variant::equal(double  v2) const { UNWRAP(equal);}
bool Variant::equal(litstr  v2) const { UNWRAP_STR(equal);}
bool Variant::equal(const StringData *v2) const { UNWRAP_STR(equal);}
bool Variant::equal(CStrRef v2) const { UNWRAP_STR(equal);}
bool Variant::equal(CArrRef v2) const { UNWRAP(equal);}
bool Variant::equal(CObjRef v2) const { UNWRAP(equal);}
bool Variant::equal(CVarRef v2) const { UNWRAP_VAR(equal,equal);}

bool Variant::equalAsStr(bool    v2) const { UNWRAP_STRING(equalAsStr);}
bool Variant::equalAsStr(int     v2) const { UNWRAP_STRING(equalAsStr);}
bool Variant::equalAsStr(int64   v2) const { UNWRAP_STRING(equalAsStr);}
bool Variant::equalAsStr(double  v2) const { UNWRAP_STRING(equalAsStr);}
bool Variant::equalAsStr(litstr  v2) const { UNWRAP_STRING(equalAsStr);}
bool Variant::equalAsStr(const StringData *v2) const {
  UNWRAP_STRING(equalAsStr);
}
bool Variant::equalAsStr(CStrRef  v2) const { UNWRAP_STRING(equalAsStr);}
bool Variant::equalAsStr(CArrRef  v2) const { UNWRAP_STRING(equalAsStr);}
bool Variant::equalAsStr(CObjRef  v2) const { UNWRAP_STRING(equalAsStr);}
bool Variant::equalAsStr(CVarRef  v2) const { UNWRAP_STRING(equalAsStr);}

bool Variant::less(bool    v2) const { UNWRAP(more);}
bool Variant::less(int     v2) const { UNWRAP(more);}
bool Variant::less(int64   v2) const { UNWRAP(more);}
bool Variant::less(double  v2) const { UNWRAP(more);}
bool Variant::less(litstr  v2) const { UNWRAP_STR(more);}
bool Variant::less(const StringData *v2) const { UNWRAP_STR(more);}
bool Variant::less(CStrRef v2) const { UNWRAP_STR(more);}
bool Variant::less(CArrRef v2) const { UNWRAP_ARR(less,more);}
bool Variant::less(CObjRef v2) const { UNWRAP(more);}
bool Variant::less(CVarRef v2) const { UNWRAP_VAR(less,more);}

bool Variant::more(bool    v2) const { UNWRAP(less);}
bool Variant::more(int     v2) const { UNWRAP(less);}
bool Variant::more(int64   v2) const { UNWRAP(less);}
bool Variant::more(double  v2) const { UNWRAP(less);}
bool Variant::more(litstr  v2) const { UNWRAP_STR(less);}
bool Variant::more(const StringData *v2) const { UNWRAP_STR(less);}
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

static void raise_bad_offset_notice() {
  if (RuntimeOption::EnableHipHopErrors) {
    raise_notice("taking offset [] on bool or number");
  }
}

#define IMPLEMENT_RVAL_INTEGRAL                                         \
  if (m_type == KindOfArray) {                                          \
    return m_data.parr->get((int64)offset, flags & AccessFlags::Error); \
  }                                                                     \
  switch (m_type) {                                                     \
    case KindOfStaticString:                                            \
    case KindOfString:                                                  \
      return m_data.pstr->getChar((int)offset);                         \
    case KindOfObject:                                                  \
      return getArrayAccess()->o_invoke(s_offsetGet,                    \
                                        Array::Create(offset));         \
    case KindOfVariant:                                                 \
      return m_data.pvar->rvalAt(offset, flags);                        \
    case KindOfUninit:                                                  \
    case KindOfNull:                                                    \
      break;                                                            \
    default:                                                            \
      if (flags & AccessFlags::Error) {                                 \
        raise_bad_offset_notice();                                      \
      }                                                                 \
      break;                                                            \
  }                                                                     \
  return null_variant;

Variant Variant::rvalAt(bool offset, ACCESSPARAMS_IMPL) const {
  IMPLEMENT_RVAL_INTEGRAL
}
Variant Variant::rvalAt(double offset, ACCESSPARAMS_IMPL) const {
  IMPLEMENT_RVAL_INTEGRAL
}

Variant Variant::rvalAtHelper(int64 offset, ACCESSPARAMS_IMPL) const {
  switch (m_type) {
  case KindOfStaticString:
  case KindOfString:
    return m_data.pstr->getChar((int)offset);
  case KindOfObject:
    return getArrayAccess()->o_invoke(s_offsetGet, Array::Create(offset));
  case KindOfVariant:
    return m_data.pvar->rvalAt(offset, flags);
  case KindOfUninit:
  case KindOfNull:
    break;
  default:
    if (flags & AccessFlags::Error) {
      raise_bad_offset_notice();
    }
    break;
  }
  return null_variant;
}

Variant Variant::rvalAt(litstr offset, ACCESSPARAMS_IMPL) const {
  if (m_type == KindOfArray) {
    bool error = flags & AccessFlags::Error;
    if (flags & AccessFlags::Key) {
      return m_data.parr->get(offset, error);
    }
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
    return m_data.pstr->getChar(StringData(offset).toInt32());
  case KindOfObject:
    return getArrayAccess()->o_invoke(s_offsetGet, Array::Create(offset));
  case KindOfVariant:
    return m_data.pvar->rvalAt(offset, flags);
  case KindOfUninit:
  case KindOfNull:
    break;
  default:
    if (flags & AccessFlags::Error) {
      raise_bad_offset_notice();
    }
    break;
  }
  return null_variant;
}

Variant Variant::rvalAt(CStrRef offset, ACCESSPARAMS_IMPL) const {
  if (m_type == KindOfArray) {
    bool error = flags & AccessFlags::Error;
    if (flags & AccessFlags::Key) {
      return m_data.parr->get(offset, error);
    }
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
    return m_data.pvar->rvalAt(offset, flags);
  case KindOfUninit:
  case KindOfNull:
    break;
  default:
    if (flags & AccessFlags::Error) {
      raise_bad_offset_notice();
    }
    break;
  }
  return null_variant;
}

Variant Variant::rvalAt(CVarRef offset, ACCESSPARAMS_IMPL) const {
  if (m_type == KindOfArray) {
    // Fast path for KindOfArray
    switch (offset.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return m_data.parr->get(empty_string, flags & AccessFlags::Error);
    case KindOfBoolean:
    case KindOfInt32:
    case KindOfInt64:
      return m_data.parr->get(offset.m_data.num, flags & AccessFlags::Error);
    case KindOfDouble:
      return m_data.parr->get((int64)offset.m_data.dbl,
                              flags & AccessFlags::Error);
    case KindOfStaticString:
    case KindOfString: {
      int64 n;
      if (offset.m_data.pstr->isStrictlyInteger(n)) {
        return m_data.parr->get(n, flags & AccessFlags::Error);
      } else {
        return m_data.parr->get(offset.asCStrRef(), flags & AccessFlags::Error);
      }
    }
    case KindOfArray:
      throw_bad_type_exception("Invalid type used as key");
      break;
    case KindOfObject:
      if (offset.isResource()) {
        return m_data.parr->get(offset.toInt64(), flags & AccessFlags::Error);
      }
      throw_bad_type_exception("Invalid type used as key");
      break;
    case KindOfVariant:
      return rvalAt(*(offset.m_data.pvar), flags);
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
    return m_data.pvar->rvalAt(offset, flags);
  case KindOfUninit:
  case KindOfNull:
    break;
  default:
    if (flags & AccessFlags::Error) {
      raise_bad_offset_notice();
    }
    break;
  }
  return null_variant;
}

template <typename T>
CVarRef Variant::rvalRefHelper(T offset, CVarRef tmp, ACCESSPARAMS_IMPL) const {
  switch (m_type) {
  case KindOfStaticString:
  case KindOfString:
    const_cast<Variant&>(tmp) = m_data.pstr->getChar(HPHP::toInt32(offset));
    return tmp;
  case KindOfObject:
    const_cast<Variant&>(tmp) =
      getArrayAccess()->o_invoke(s_offsetGet, Array::Create(offset));
    return tmp;
  case KindOfVariant:
    return m_data.pvar->rvalRef(offset, tmp, flags);
  case KindOfUninit:
  case KindOfNull:
    break;
  default:
    if (flags & AccessFlags::Error) {
      raise_bad_offset_notice();
    }
    break;
  }
  return null_variant;
}

template CVarRef
Variant::rvalRefHelper(int64 offset, CVarRef tmp, ACCESSPARAMS_IMPL) const;

CVarRef Variant::rvalRef(double offset, CVarRef tmp, ACCESSPARAMS_IMPL) const {
  return rvalRef((int64)offset, tmp, flags);
}

CVarRef Variant::rvalRef(litstr offset, CVarRef tmp, ACCESSPARAMS_IMPL) const {
  if (m_type == KindOfArray) {
    bool error = flags & AccessFlags::Error;
    if (flags & AccessFlags::Key) return m_data.parr->get(offset, error);
    int64 n;
    int len = strlen(offset);
    if (!is_strictly_integer(offset, len, n)) {
      return m_data.parr->get(offset, error);
    } else {
      return m_data.parr->get(n, error);
    }
  }
  return rvalRefHelper(offset, tmp, flags);
}

CVarRef Variant::rvalRef(CStrRef offset, CVarRef tmp, ACCESSPARAMS_IMPL) const {
  if (m_type == KindOfArray) {
    bool error = flags & AccessFlags::Error;
    if (flags & AccessFlags::Key) return m_data.parr->get(offset, error);
    if (offset.isNull()) return m_data.parr->get(empty_string, error);
    int64 n;
    if (!offset->isStrictlyInteger(n)) {
      return m_data.parr->get(offset, error);
    } else {
      return m_data.parr->get(n, error);
    }
  }
  return rvalRefHelper(offset, tmp, flags);
}

CVarRef Variant::rvalRef(CVarRef offset, CVarRef tmp, ACCESSPARAMS_IMPL) const {
  if (m_type == KindOfArray) {
    // Fast path for KindOfArray
    switch (offset.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return m_data.parr->get(empty_string, flags & AccessFlags::Error);
    case KindOfBoolean:
    case KindOfInt32:
    case KindOfInt64:
      return m_data.parr->get(offset.m_data.num, flags & AccessFlags::Error);
    case KindOfDouble:
      return m_data.parr->get((int64)offset.m_data.dbl,
                              flags & AccessFlags::Error);
    case KindOfStaticString:
    case KindOfString: {
      int64 n;
      if (offset.m_data.pstr->isStrictlyInteger(n)) {
        return m_data.parr->get(n, flags & AccessFlags::Error);
      } else {
        return m_data.parr->get(offset.asCStrRef(), flags & AccessFlags::Error);
      }
    }
    case KindOfArray:
      throw_bad_type_exception("Invalid type used as key");
      break;
    case KindOfObject:
      if (offset.isResource()) {
        return m_data.parr->get(offset.toInt64(), flags & AccessFlags::Error);
      }
      throw_bad_type_exception("Invalid type used as key");
      break;
    case KindOfVariant:
      return rvalRef(*(offset.m_data.pvar), tmp, flags);
    default:
      ASSERT(false);
      break;
    }
    return null_variant;
  }
  return rvalRefHelper(offset, tmp, flags);
}

template <typename T>
class LvalHelper {};

template<>
class LvalHelper<int64> {
public:
  typedef int64 KeyType;
  static bool CheckKey(KeyType k) { return true; };
  static const bool CheckParams = false;
};

template<>
class LvalHelper<bool> : public LvalHelper<int64> {};

template<>
class LvalHelper<double> : public LvalHelper<int64> {};

template<>
class LvalHelper<CStrRef> {
public:
  typedef VarNR KeyType;
  static bool CheckKey(const KeyType &k) { return true; };
  static const bool CheckParams = true;
};

template<>
class LvalHelper<CVarRef> {
public:
  typedef VarNR KeyType;
  static bool CheckKey(const KeyType &k) { return !k.isNull(); };
  static const bool CheckParams = true;
};


template<typename T>
Variant& Variant::lvalAtImpl(T key, ACCESSPARAMS_IMPL) {
  if (m_type == KindOfArray) {
    Variant *ret = NULL;
    ArrayData *arr = m_data.parr;
    ArrayData *escalated;
    if (LvalHelper<T>::CheckParams && flags & AccessFlags::Key) {
      escalated = arr->lval(key, ret, arr->getCount() > 1,
                            flags & AccessFlags::CheckExist);
    } else {
      typename LvalHelper<T>::KeyType k(ToKey(key));
      if (LvalHelper<T>::CheckKey(k)) {
        escalated =
          arr->lval(k, ret, arr->getCount() > 1,
                    flags & AccessFlags::CheckExist);
      } else {
        ret = &lvalBlackHole();
        escalated = 0;
      }
    }
    if (escalated) {
      set(escalated);
    }
    ASSERT(ret);
    return *ret;
  }
  if (m_type == KindOfVariant) {
    return m_data.pvar->lvalAtImpl<T>(key, flags);
  }
  if (isObjectConvertable()) {
    set(Array::Create());
    return lvalAtImpl<T>(key, flags);
  }
  if (m_type == KindOfObject) {
    return getArrayAccess()->___offsetget_lval(key);
  }
  return lvalInvalid();
}

Variant &Variant::lvalAt(bool    key, ACCESSPARAMS_IMPL) {
  return lvalAtImpl(key, flags);
}
Variant &Variant::lvalAt(int     key, ACCESSPARAMS_IMPL) {
  return lvalAt((int64)key, flags);
}
Variant &Variant::lvalAt(int64   key, ACCESSPARAMS_IMPL) {
  return lvalAtImpl(key, flags);
}
Variant &Variant::lvalAt(double  key, ACCESSPARAMS_IMPL) {
  return lvalAtImpl(key, flags);
}
Variant &Variant::lvalAt(litstr  ckey, ACCESSPARAMS_IMPL) {
  String key(ckey);
  return lvalAt(key, flags);
}
Variant &Variant::lvalAt(CStrRef key, ACCESSPARAMS_IMPL) {
  return Variant::lvalAtImpl<CStrRef>(key, flags);
}
Variant &Variant::lvalAt(CVarRef k, ACCESSPARAMS_IMPL) {
  return Variant::lvalAtImpl<CVarRef>(k, flags);
}

Variant *Variant::lvalPtr(CStrRef key, bool forWrite, bool create) {
  Variant *t = m_type == KindOfVariant ? m_data.pvar : this;
  if (t->m_type == KindOfArray) {
    return t->asArrRef().lvalPtr(key, forWrite, create);
  }
  return NULL;
}

Variant &Variant::lvalAt() {
  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:
    set(ArrayData::Create());
    break;
  case KindOfBoolean:
    if (!toBoolean()) {
      set(ArrayData::Create());
    } else {
      throw_bad_type_exception("[] operator not supported for this type");
      return lvalBlackHole();
    }
    break;
  case KindOfArray:
    break;
  case KindOfVariant:
    return m_data.pvar->lvalAt();
  case KindOfObject:
    {
      Array params = CREATE_VECTOR1(null);
      Variant& ret = lvalBlackHole();
      ret = m_data.pobj->o_invoke(s_offsetGet, params);
      raise_warning("Indirect modification of overloaded element of %s has "
                    "no effect", m_data.pobj->o_getClassName().data());
      return ret;
    }
  case KindOfStaticString:
  case KindOfString:
    if (getStringData()->empty()) {
      set(ArrayData::Create());
      break;
    }
    // fall through to throw
  default:
    throw_bad_type_exception("[] operator not supported for this type");
    return lvalBlackHole();
  }

  ASSERT(m_type == KindOfArray);
  Variant *ret = NULL;
  ArrayData *arr = m_data.parr;
  ArrayData *escalated = arr->lvalNew(ret, arr->getCount() > 1);
  if (escalated) {
    set(escalated);
  }
  ASSERT(ret);
  return *ret;
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
    return strongBind(lvalAt(key, AccessFlags::IsKey(isString)));
  } else {
    return rvalAt(key, AccessFlags::IsKey(isString));
  }
}

Variant Variant::argvalAt(bool byRef, bool key) {
  return argvalAtImpl(byRef, key);
}
Variant Variant::argvalAt(bool byRef, int key) {
  return argvalAtImpl(byRef, key);
}
Variant Variant::argvalAt(bool byRef, int64 key) {
  return argvalAtImpl(byRef, key);
}
Variant Variant::argvalAt(bool byRef, double key) {
  return argvalAtImpl(byRef, (int64)key);
}
Variant Variant::argvalAt(bool byRef, litstr key,
    bool isString /* = false */) {
  return argvalAtImpl(byRef, key, isString);
}
Variant Variant::argvalAt(bool byRef, CStrRef key,
    bool isString /* = false */) {
  return argvalAtImpl(byRef, key, isString);
}
Variant Variant::argvalAt(bool byRef, CVarRef key) {
  return argvalAtImpl(byRef, key);
}

Variant Variant::argvalAtImpl(bool byRef, CStrRef key,
    bool isString /* = false */) {
  if (m_type == KindOfVariant) {
    return m_data.pvar->argvalAtImpl(byRef, key, isString);
  }
  if (byRef && (is(KindOfArray) || isObjectConvertable())) {
    return strongBind(lvalAt(key, AccessFlags::IsKey(isString)));
  } else {
    return rvalAt(key, AccessFlags::IsKey(isString));
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

bool Variant::o_empty(CStrRef propName,
                      CStrRef context /* = null_string */) const {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_empty(propName, context);
  }
  if (m_type == KindOfVariant) {
    return m_data.pvar->o_empty(propName, context);
  }
  if (m_type == KindOfArray) {
    return empty(rvalAt(propName));
  }
  return true;
}

bool Variant::o_isset(CStrRef propName,
                      CStrRef context /* = null_string */) const {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_isset(propName, context);
  }
  if (m_type == KindOfVariant) {
    return m_data.pvar->o_isset(propName, context);
  }
  if (m_type == KindOfArray) {
    return isset(rvalAt(propName));
  }
  return false;
}

void Variant::o_unset(CStrRef propName, CStrRef context /* = null_string */) {
  if (m_type == KindOfObject) {
    m_data.pobj->o_unset(propName, context);
  }
  if (m_type == KindOfVariant) {
    m_data.pvar->o_unset(propName, context);
  }
}

Variant Variant::o_argval(bool byRef, CStrRef propName,
    bool error /* = true */, CStrRef context /* = null_string */) const {
  if (m_type == KindOfObject) {
    if (byRef) {
      return strongBind(m_data.pobj->o_lval(propName, context));
    } else {
      return m_data.pobj->o_get(propName, error, context);
    }
  } else if (m_type == KindOfVariant) {
    if (byRef) {
      return strongBind(m_data.pvar->o_lval(propName, context));
    } else {
      return m_data.pvar->o_get(propName, error, context);
    }
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
    set(Object(SystemLib::AllocStdClassObject()));
  } else {
    // Raise a warning
    raise_warning("Attempt to assign property of non-object");
    return val;
  }
  return m_data.pobj->o_set(propName, val, false, context);
}

Variant Variant::o_setRef(CStrRef propName, CVarRef val,
                          CStrRef context /* = null_string */) {
  if (propName.empty()) {
    throw EmptyObjectPropertyException();
  }

  if (m_type == KindOfObject) {
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_setRef(propName, val, context);
  } else if (isObjectConvertable()) {
    set(Object(SystemLib::AllocStdClassObject()));
  } else {
    // Raise a warning
    raise_warning("Attempt to assign property of non-object");
    return val;
  }
  return m_data.pobj->o_setRef(propName, val, false, context);
}

Variant Variant::o_setPublic(CStrRef propName, CVarRef val) {
  if (propName.empty()) {
    throw EmptyObjectPropertyException();
  }

  if (m_type == KindOfObject) {
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_setPublic(propName, val);
  } else if (isObjectConvertable()) {
    set(Object(SystemLib::AllocStdClassObject()));
  } else {
    // Raise a warning
    raise_warning("Attempt to assign property of non-object");
    return val;
  }
  return m_data.pobj->o_setPublic(propName, val, false);
}

Variant Variant::o_setPublicRef(CStrRef propName, CVarRef val) {
  if (propName.empty()) {
    throw EmptyObjectPropertyException();
  }

  if (m_type == KindOfObject) {
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_setPublicRef(propName, val);
  } else if (isObjectConvertable()) {
    set(Object(SystemLib::AllocStdClassObject()));
  } else {
    // Raise a warning
    raise_warning("Attempt to assign property of non-object");
    return val;
  }
  return m_data.pobj->o_setPublicRef(propName, val, false);
}

Variant Variant::o_invoke(const char *s, CArrRef params,
                          int64 hash /* = -1 */) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke(s, params, hash);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_invoke(s, params, hash);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_invoke(CStrRef s, CArrRef params, int64 hash /* = -1 */) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke(s, params, hash);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_invoke(s, params, hash);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_root_invoke(const char *s, CArrRef params,
                               int64 hash /* = -1 */) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_root_invoke(s, params, hash);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_root_invoke(s, params, hash);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_root_invoke(CStrRef s, CArrRef params,
                               int64 hash /* = -1 */) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_root_invoke(s, params, hash);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_root_invoke(s, params, hash);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_invoke_ex(CStrRef clsname, CStrRef s, CArrRef params) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke_ex(clsname, s, params);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_invoke_ex(clsname, s, params);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_invoke_few_args(const char *s, int64 hash, int count,
                                   INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke_few_args(s, hash, count,
                                          INVOKE_FEW_ARGS_PASS_ARGS);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_invoke_few_args(s, hash, count,
                                          INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_invoke_few_args(CStrRef s, int64 hash, int count,
                                   INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke_few_args(s, hash, count,
                                          INVOKE_FEW_ARGS_PASS_ARGS);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_invoke_few_args(s, hash, count,
                                          INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_root_invoke_few_args(const char *s, int64 hash, int count,
                                        INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_root_invoke_few_args(s, hash, count,
                                               INVOKE_FEW_ARGS_PASS_ARGS);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_root_invoke_few_args(s, hash, count,
                                               INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant Variant::o_root_invoke_few_args(CStrRef s, int64 hash, int count,
                                        INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_root_invoke_few_args(s, hash, count,
                                               INVOKE_FEW_ARGS_PASS_ARGS);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_root_invoke_few_args(s, hash, count,
                                               INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

bool Variant::o_get_call_info(MethodCallPackage &info, int64 hash /* = -1 */) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_get_call_info(info, hash);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_get_call_info(info, hash);
  } else {
    throw InvalidOperandException(
        "Call to a member function on a non-object");
  }
}

Variant &Variant::o_lval(CStrRef propName, CVarRef tmpForGet,
                         CStrRef context /* = null_string */) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_lval(propName, tmpForGet, context);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_lval(propName, tmpForGet, context);
  } else if (isObjectConvertable()) {
    set(Object(SystemLib::AllocStdClassObject()));
    return m_data.pobj->o_lval(propName, tmpForGet, context);
  } else {
    // Raise a warning
    raise_warning("Attempt to assign property of non-object");
    return const_cast<Variant&>(tmpForGet);
  }
}

Variant &Variant::o_unsetLval(CStrRef propName, CVarRef tmpForGet,
                              CStrRef context /* = null_string */) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_lval(propName, tmpForGet, context);
  } else if (m_type == KindOfVariant) {
    return m_data.pvar->o_unsetLval(propName, tmpForGet, context);
  } else {
    return const_cast<Variant&>(tmpForGet);
  }
}

#define OPEQUAL(op, l, r)                                               \
  switch (op) {                                                         \
  case T_CONCAT_EQUAL: concat_assign((l), r); break;                    \
  case T_PLUS_EQUAL:   ((l) += r);            break;                    \
  case T_MINUS_EQUAL:  ((l) -= r);            break;                    \
  case T_MUL_EQUAL:    ((l) *= r);            break;                    \
  case T_DIV_EQUAL:    ((l) /= r);            break;                    \
  case T_MOD_EQUAL:    ((l) %= r);            break;                    \
  case T_AND_EQUAL:    ((l) &= r);            break;                    \
  case T_OR_EQUAL:     ((l) |= r);            break;                    \
  case T_XOR_EQUAL:    ((l) ^= r);            break;                    \
  case T_SL_EQUAL:     ((l) <<= r);           break;                    \
  case T_SR_EQUAL:     ((l) >>= r);           break;                    \
  default:                                                              \
    throw FatalErrorException(0, "invalid operator %d", op);            \
  }                                                                     \

#define IMPLEMENT_SETAT_OPEQUAL                                         \
check_array:                                                            \
  if (m_type == KindOfArray) {                                          \
    Variant *cv = NULL;                                                 \
    ArrayData *escalated =                                              \
      m_data.parr->lval(ToKey(key), cv, (m_data.parr->getCount() > 1)); \
    if (escalated) {                                                    \
      set(escalated);                                                   \
    }                                                                   \
    ASSERT(cv);                                                         \
    OPEQUAL(op, *cv, v);                                                \
    return *cv;                                                         \
  }                                                                     \
  switch (m_type) {                                                     \
  case KindOfBoolean:                                                   \
    if (toBoolean()) {                                                  \
      throw_bad_type_exception("not array objects");                    \
      break;                                                            \
    }                                                                   \
    /* Fall through */                                                  \
  case KindOfUninit:                                                    \
  case KindOfNull:                                                      \
    set(ArrayData::Create(ToKey(key), null));                           \
    goto check_array;                                                   \
  case KindOfVariant:                                                   \
    m_data.pvar->setOpEqual(op, key, v);                                \
    break;                                                              \
  case KindOfStaticString:                                              \
  case KindOfString: {                                                  \
    String s = toString();                                              \
    if (s.empty()) {                                                    \
      set(ArrayData::Create(ToKey(key), null));                         \
      goto check_array;                                                 \
    }                                                                   \
    throw_bad_type_exception("not array objects");                      \
    break;                                                              \
  }                                                                     \
  case KindOfObject: {                                                  \
    ObjectData *aa = getArrayAccess();                                  \
    Variant &cv = aa->___offsetget_lval(key);                           \
    OPEQUAL(op, cv, v);                                                 \
    aa->o_invoke(s_offsetSet, CREATE_VECTOR2(key, cv), -1);             \
    return cv;                                                          \
  }                                                                     \
  default:                                                              \
    throw_bad_type_exception("not array objects");                      \
    break;                                                              \
  }                                                                     \
  return v;                                                             \

template <typename T>
inline ALWAYS_INLINE CVarRef Variant::SetImpl(Variant *self, T key,
                                              CVarRef v, bool isKey) {
  retry:
  if (LIKELY(self->m_type == KindOfArray)) {
    ArrayData *escalated;
    if (LvalHelper<T>::CheckParams && isKey) {
      escalated = self->m_data.parr->set(key, v, self->needCopyForSet(v));
    } else {
      typename LvalHelper<T>::KeyType k(ToKey(key));
      if (!LvalHelper<T>::CheckKey(k)) return lvalBlackHole();
      escalated = self->m_data.parr->set(k, v, self->needCopyForSet(v));
    }
    if (escalated) {
      self->set(escalated);
    }
    return v;
  }
  switch (self->m_type) {
  case KindOfBoolean:
    if (self->m_data.num) {
      throw_bad_type_exception("not array objects");
      break;
    }
    /* Fall through */
  case KindOfUninit:
  case KindOfNull:
  create:
    if (LvalHelper<T>::CheckParams && isKey) {
      self->set(ArrayData::Create(key, v));
    } else {
      typename LvalHelper<T>::KeyType k(ToKey(key));
      if (!LvalHelper<T>::CheckKey(k)) return lvalBlackHole();
      self->set(ArrayData::Create(k, v));
    }
    break;
  case KindOfVariant:
    self = self->m_data.pvar;
    goto retry;
  case KindOfStaticString:
  case KindOfString: {
    StringData *s = self->m_data.pstr;
    if (s->empty()) {
      goto create;
    }
    StringData *es = StringData::Escalate(s);
    es->set(key, v.toString());
    if (es != s) self->set(es);
    break;
  }
  case KindOfObject:
    self->getArrayAccess()->o_invoke_few_args(s_offsetSet, -1, 2, key, v);
    break;
  default:
    throw_bad_type_exception("not array objects");
    break;
  }
  return v;
}

CVarRef Variant::set(bool key, CVarRef v) {
  return SetImpl(this, key, v, false);
}

CVarRef Variant::set(int64 key, CVarRef v) {
  return SetImpl(this, key, v, false);
}

CVarRef Variant::set(double key, CVarRef v) {
  return SetImpl(this, key, v, false);
}

CVarRef Variant::set(CStrRef key, CVarRef v, bool isString /* = false */) {
  return SetImpl<CStrRef>(this, key, v, isString);
}

__attribute__ ((section (".text.hot")))
CVarRef Variant::set(CVarRef key, CVarRef v) {
  return SetImpl<CVarRef>(this, key, v, false);
}

CVarRef Variant::append(CVarRef v) {
  switch (m_type) {
  case KindOfUninit:
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
      ArrayData *escalated = m_data.parr->append(v, needCopyForSet(v));
      if (escalated) {
        set(escalated);
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
    if (getStringData()->empty()) {
      set(ArrayData::Create(v));
      return v;
    }
    // fall through to throw
  default:
    throw_bad_type_exception("[] operator not supported for this type");
  }
  return v;
}

template <typename T>
inline ALWAYS_INLINE CVarRef Variant::SetRefImpl(Variant *self, T key,
                                                 CVarRef v, bool isKey) {
  retry:
  if (LIKELY(self->m_type == KindOfArray)) {
    ArrayData *escalated;
    if (LvalHelper<T>::CheckParams && isKey) {
      escalated = self->m_data.parr->setRef(key, v, self->needCopyForSetRef(v));
    } else {
      typename LvalHelper<T>::KeyType k(ToKey(key));
      if (!LvalHelper<T>::CheckKey(k)) return lvalBlackHole();
      escalated = self->m_data.parr->setRef(k, v, self->needCopyForSetRef(v));
    }
    if (escalated) {
      self->set(escalated);
    }
    return v;
  }
  switch (self->m_type) {
  case KindOfBoolean:
    if (self->m_data.num) {
      throw_bad_type_exception("not array objects");
      break;
    }
    /* Fall through */
  case KindOfUninit:
  case KindOfNull:
  create:
    if (LvalHelper<T>::CheckParams && isKey) {
      self->set(ArrayData::CreateRef(key, v));
    } else {
      typename LvalHelper<T>::KeyType k(ToKey(key));
      if (!LvalHelper<T>::CheckKey(k)) return lvalBlackHole();
      self->set(ArrayData::CreateRef(k, v));
    }
    break;
  case KindOfVariant:
    self = self->m_data.pvar;
    goto retry;
  case KindOfStaticString:
  case KindOfString: {
    if (self->m_data.pstr->empty()) {
      goto create;
    }
    throw_bad_type_exception("binding assignment to stringoffset");
    break;
  }
  case KindOfObject:
    self->getArrayAccess()->o_invoke_few_args(s_offsetSet, -1, 2, key, v);
    break;
  default:
    throw_bad_type_exception("not array objects");
    break;
  }
  return v;
}

CVarRef Variant::setRef(bool key, CVarRef v) {
  return SetRefImpl(this, key, v, false);
}

CVarRef Variant::setRef(int64 key, CVarRef v) {
  return SetRefImpl(this, key, v, false);
}

CVarRef Variant::setRef(double key, CVarRef v) {
  return SetRefImpl(this, key, v, false);
}

CVarRef Variant::setRef(CStrRef key, CVarRef v, bool isString /* = false */) {
  return SetRefImpl<CStrRef>(this, key, v, isString);
}

CVarRef Variant::setRef(CVarRef key, CVarRef v) {
  return SetRefImpl<CVarRef>(this, key, v, false);
}

CVarRef Variant::appendRef(CVarRef v) {
  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:
    set(ArrayData::CreateRef(v));
    break;
  case KindOfBoolean:
    if (!toBoolean()) {
      set(ArrayData::CreateRef(v));
    } else {
      throw_bad_type_exception("[] operator not supported for this type");
    }
    break;
  case KindOfArray:
    {
      ArrayData *escalated = m_data.parr->appendRef(v, needCopyForSetRef(v));
      if (escalated) {
        set(escalated);
      }
    }
    break;
  case KindOfVariant:
    m_data.pvar->appendRef(v);
    break;
  case KindOfObject:
    {
      m_data.pobj->o_invoke_few_args(s_offsetSet, -1, 2, null, v);
      break;
    }
  case KindOfStaticString:
  case KindOfString:
    if (getStringData()->empty()) {
      set(ArrayData::CreateRef(v));
      return v;
    }
    // fall through to throw
  default:
    throw_bad_type_exception("[] operator not supported for this type");
  }
  return v;
}

CVarRef Variant::setOpEqual(int op, bool key, CVarRef v) {
  IMPLEMENT_SETAT_OPEQUAL;
}

CVarRef Variant::setOpEqual(int op, int64 key, CVarRef v) {
  IMPLEMENT_SETAT_OPEQUAL;
}

CVarRef Variant::setOpEqual(int op, double key, CVarRef v) {
  IMPLEMENT_SETAT_OPEQUAL;
}

CVarRef Variant::setOpEqual(int op, CStrRef key, CVarRef v,
                            bool isString /* = false */) {
check_array:
  if (m_type == KindOfArray) {
    Variant *cv = NULL;
    ArrayData *escalated;
    if (isString) {
      escalated =
        m_data.parr->lval(key, cv, (m_data.parr->getCount() > 1));
    } else {
      escalated =
        m_data.parr->lval(ToKey(key), cv, (m_data.parr->getCount() > 1));
    }
    if (escalated) {
      set(escalated);
    }
    ASSERT(cv);
    OPEQUAL(op, *cv, v);
    return *cv;
  }
  switch (m_type) {
  case KindOfBoolean:
    if (toBoolean()) {
      throw_bad_type_exception("not array objects");
      break;
    }
    /* Fall through */
  case KindOfUninit:
  case KindOfNull:
    if (isString) {
      set(ArrayData::Create(key, null));
    } else {
      set(ArrayData::Create(ToKey(key), null));
    }
    goto check_array;
  case KindOfVariant:
    return m_data.pvar->setOpEqual(op, key, v, isString);
  case KindOfStaticString:
  case KindOfString: {
    String s = toString();
    if (s.empty()) {
      if (isString) {
        set(ArrayData::Create(key, null));
      } else {
        set(ArrayData::Create(ToKey(key), null));
      }
      goto check_array;
    }
    throw_bad_type_exception("not array objects");
    break;
  }
  case KindOfObject: {
    ObjectData *aa = getArrayAccess();
    Variant &cv = aa->___offsetget_lval(key);
    OPEQUAL(op, cv, v);
    aa->o_invoke(s_offsetSet, CREATE_VECTOR2(key, cv), -1);
    return cv;
  }
  default:
    throw_bad_type_exception("not array objects");
    break;
  }
  return v;
}

CVarRef Variant::setOpEqual(int op, CVarRef key, CVarRef v) {
check_array:
  if (m_type == KindOfArray) {
    Variant *cv = NULL;
    VarNR k(ToKey(key));
    if (k.isNull()) return lvalBlackHole();
    ArrayData *escalated =
      m_data.parr->lval(k, cv, (m_data.parr->getCount() > 1));
    if (escalated) {
      set(escalated);
    }
    ASSERT(cv);
    OPEQUAL(op, *cv, v);
    return *cv;
  }
  switch (m_type) {
  case KindOfBoolean:
    if (toBoolean()) {
      throw_bad_type_exception("not array objects");
      break;
    }
    /* Fall through */
  case KindOfUninit:
  case KindOfNull: {
    VarNR k(ToKey(key));
    if (k.isNull()) return lvalBlackHole();
    set(ArrayData::Create(k, null));
    goto check_array;
  }
  case KindOfVariant:
    return m_data.pvar->setOpEqual(op, key, v);
  case KindOfStaticString:
  case KindOfString: {
    String s = toString();
    if (s.empty()) {
      VarNR k(ToKey(key));
      if (k.isNull()) return lvalBlackHole();
      set(ArrayData::Create(k, null));
      goto check_array;
    }
    throw_bad_type_exception("not array objects");
    break;
  }
  case KindOfObject: {
    ObjectData *aa = getArrayAccess();
    Variant &cv = aa->___offsetget_lval(key);
    OPEQUAL(op, cv, v);
    aa->o_invoke(s_offsetSet, CREATE_VECTOR2(key, cv), -1);
    return cv;
  }
  default:
    throw_bad_type_exception("not array objects");
    break;
  }
  return v;
}

CVarRef Variant::appendOpEqual(int op, CVarRef v) {
check_array:
  if (m_type == KindOfArray) {
    Variant *cv = NULL;
    ArrayData *escalated =
      m_data.parr->lvalNew(cv, m_data.parr->getCount() > 1);
    if (escalated) {
      set(escalated);
    }
    ASSERT(cv);
    switch (op) {
    case T_CONCAT_EQUAL: return concat_assign((*cv), v);
    case T_PLUS_EQUAL:   return ((*cv) += v);
    case T_MINUS_EQUAL:  return ((*cv) -= v);
    case T_MUL_EQUAL:    return ((*cv) *= v);
    case T_DIV_EQUAL:    return ((*cv) /= v);
    case T_MOD_EQUAL:    return ((*cv) %= v);
    case T_AND_EQUAL:    return ((*cv) &= v);
    case T_OR_EQUAL:     return ((*cv) |= v);
    case T_XOR_EQUAL:    return ((*cv) ^= v);
    case T_SL_EQUAL:     return ((*cv) <<= v);
    case T_SR_EQUAL:     return ((*cv) >>= v);
    default:
      throw FatalErrorException(0, "invalid operator %d", op);
    }
    return v;
  }
  switch (m_type) {
  case KindOfUninit:
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
      throw FatalErrorException(0, "invalid operator %d", op);
    }
    aa->o_invoke(s_offsetSet, CREATE_VECTOR2(null_variant, cv));
    return cv;
  }
  case KindOfStaticString:
  case KindOfString:
    if (getStringData()->empty()) {
      set(ArrayData::Create());
      goto check_array;
    }
    // fall through to throw
  default:
    throw_bad_type_exception("[] operator not supported for this type");
  }
  return v;
}

void Variant::removeImpl(double key) {
  switch (getType()) {
  case KindOfUninit:
  case KindOfNull:
    break;
  case KindOfArray:
    {
      ArrayData *arr = getArrayData();
      if (arr) {
        ArrayData *escalated = arr->remove(ToKey(key), (arr->getCount() > 1));
        if (escalated) {
          set(escalated);
        }
      }
    }
    break;
  case KindOfObject:
    callOffsetUnset(key);
    break;
  default:
    lvalInvalid();
    break;
  }
}

void Variant::removeImpl(int64 key) {
  switch (getType()) {
  case KindOfUninit:
  case KindOfNull:
    break;
  case KindOfArray:
    {
      ArrayData *arr = getArrayData();
      if (arr) {
        ArrayData *escalated = arr->remove(key, (arr->getCount() > 1));
        if (escalated) {
          set(escalated);
        }
      }
    }
    break;
  case KindOfObject:
    callOffsetUnset(key);
    break;
  default:
    lvalInvalid();
    break;
  }
}

void Variant::removeImpl(bool key) {
  switch (getType()) {
  case KindOfUninit:
  case KindOfNull:
    break;
  case KindOfArray:
    {
      ArrayData *arr = getArrayData();
      if (arr) {
        ArrayData *escalated = arr->remove(ToKey(key), (arr->getCount() > 1));
        if (escalated) {
          set(escalated);
        }
      }
    }
    break;
  case KindOfObject:
    callOffsetUnset(key);
    break;
  default:
    lvalInvalid();
    break;
  }
}

void Variant::removeImpl(CVarRef key, bool isString /* false */) {
  switch (getType()) {
  case KindOfUninit:
  case KindOfNull:
    break;
  case KindOfArray:
    {
      ArrayData *arr = getArrayData();
      if (arr) {
        ArrayData *escalated;
        if (isString) {
          escalated = arr->remove(key, (arr->getCount() > 1));
        } else {
          escalated = arr->remove(key.toKey(), (arr->getCount() > 1));
        }
        if (escalated) {
          set(escalated);
        }
      }
    }
    break;
  case KindOfObject:
    callOffsetUnset(key);
    break;
  default:
    lvalInvalid();
    break;
  }
}

void Variant::removeImpl(CStrRef key, bool isString /* false */) {
  switch (getType()) {
  case KindOfUninit:
  case KindOfNull:
    break;
  case KindOfArray:
    {
      ArrayData *arr = getArrayData();
      if (arr) {
        ArrayData *escalated;
        if (isString) {
          escalated = arr->remove(key, (arr->getCount() > 1));
        } else {
          escalated = arr->remove(key.toKey(), (arr->getCount() > 1));
        }
        if (escalated) {
          set(escalated);
        }
      }
    }
    break;
  case KindOfObject:
    callOffsetUnset(key);
    break;
  default:
    lvalInvalid();
    break;
  }
}

void Variant::remove(CVarRef key) {
  switch(key.getType()) {
  case KindOfInt32:
  case KindOfInt64:
    removeImpl(key.toInt64());
    return;
  case KindOfString:
  case KindOfStaticString:
    removeImpl(key.toString());
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
  case KindOfUninit:
  case KindOfNull:
    ASSERT(!isArrayKey);
    serializer->writeNull();                break;
  case KindOfBoolean:
    ASSERT(!isArrayKey);
    serializer->write(m_data.num != 0);     break;
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

void Variant::unserialize(VariableUnserializer *uns) {
  char type, sep;
  type = uns->readChar();
  sep = uns->readChar();

  if (type != 'R') {
    uns->add(this);
  }

  if (type == 'N') {
    if(sep != ';') throw Exception("Expected ';' but got '%c'", sep);
    setNull(); // NULL *IS* the value, without we get undefined warnings
    return;
  }
  if (sep != ':') {
    throw Exception("Expected ':' but got '%c'", sep);
  }

  switch (type) {
  case 'r':
    {
      int64 id = uns->readInt();
      Variant *v = uns->get(id);
      if (v == NULL) {
        throw Exception("Id %ld out of range", id);
      }
      operator=(*v);
    }
    break;
  case 'R':
    {
      int64 id = uns->readInt();
      Variant *v = uns->get(id);
      if (v == NULL) {
        throw Exception("Id %ld out of range", id);
      }
      assignRef(*v);
    }
    break;
  case 'b': { int64 v = uns->readInt(); operator=((bool)v); } break;
  case 'i': { int64 v = uns->readInt(); operator=(v);       } break;
  case 'd':
    {
      double v;
      char ch = uns->peek();
      bool negative = false;
      char buf[4];
      if (ch == '-') {
        negative = true;
        ch = uns->readChar();
        ch = uns->peek();
      }
      if (ch == 'I') {
        uns->read(buf, 3); buf[3] = '\0';
        if (strcmp(buf, "INF")) {
          throw Exception("Expected 'INF' but got '%s'", buf);
        }
        v = atof("inf");
      } else if (ch == 'N') {
        uns->read(buf, 3); buf[3] = '\0';
        if (strcmp(buf, "NAN")) {
          throw Exception("Expected 'NAN' but got '%s'", buf);
        }
        v = atof("nan");
      } else {
        v = uns->readDouble();
      }
      operator=(negative ? -v : v);
    }
    break;
  case 's':
    {
      String v;
      v.unserialize(uns);
      operator=(v);
    }
    break;
  case 'S':
    if (uns->getType() == VariableUnserializer::APCSerialize) {
      union {
        char buf[8];
        StringData *sd;
      } u;
      uns->read(u.buf, 8);
      operator=(u.sd);
    } else {
      throw Exception("Unknown type '%c'", type);
    }
    break;
  case 'a':
    {
      Array v = Array::Create();
      v.unserialize(uns);
      operator=(v);
      return; // array has '}' terminating
    }
    break;
  case 'A':
    if (uns->getType() == VariableUnserializer::APCSerialize) {
      union {
        char buf[8];
        ArrayData *ad;
      } u;
      uns->read(u.buf, 8);
      operator=(u.ad);
    } else {
      throw Exception("Unknown type '%c'", type);
    }
    break;
  case 'o':
    {
      String clsName;
      clsName.unserialize(uns);

      sep = uns->readChar();
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
      v.unserialize(uns);
      obj->o_setArray(v);

      obj->t___wakeup();
      return; // array has '}' terminating
    }
    break;
  case 'O':
    {
      String clsName;
      clsName.unserialize(uns);

      sep = uns->readChar();
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
      int64 size = uns->readInt();
      char sep = uns->readChar();
      if (sep != ':') {
        throw Exception("Expected ':' but got '%c'", sep);
      }
      sep = uns->readChar();
      if (sep != '{') {
        throw Exception("Expected '{' but got '%c'", sep);
      }
      if (size > 0) {
        for (int64 i = 0; i < size; i++) {
          String key = uns->unserializeKey().toString();
          int subLen = 0;
          if (key.size() > 0 && key.charAt(0) == '\00') {
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
          value.unserialize(uns);
        }
      }
      sep = uns->readChar();
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
      clsName.unserialize(uns);

      sep = uns->readChar();
      if (sep != ':') {
        throw Exception("Expected ':' but got '%c'", sep);
      }
      String serialized;
      serialized.unserialize(uns, '{', '}');

      Object obj;
      try {
        obj = create_object(clsName.data(), Array::Create(), false);
        if (!obj->o_instanceof("Serializable")) {
          raise_error("%s didn't implement Serializable", clsName.data());
        }
        obj->o_invoke("unserialize", CREATE_VECTOR1(serialized), -1);
      } catch (ClassNotFoundException &e) {
        if (!uns->allowUnknownSerializableClass()) {
          throw;
        }
        obj = create_object("__PHP_Incomplete_Class", Array::Create(), false);
        obj->o_set("__PHP_Incomplete_Class_Name", clsName);
        obj->o_set("serialized", serialized);
      }
      operator=(obj);
      return; // object has '}' terminating
    }
    break;
  default:
    throw Exception("Unknown type '%c'", type);
  }
  sep = uns->readChar();
  if (sep != ';') {
    throw Exception("Expected ';' but got '%c'", sep);
  }
}

Variant Variant::share(bool save) const {
  if (m_type == KindOfVariant) {
    return m_data.pvar->share(save);
  }

  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:    return false; // same as non-existent
  case KindOfBoolean: return (m_data.num != 0);
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
      ObjectData *obj = SystemLib::AllocStdClassObject();
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
        refMap.insert(mpvar, pvar, true); // ahead of deep copy
        *pvar = mpvar->fiberMarshal(refMap);
      }
      pvar->incRefCount();
      return pvar;
    }
    return mpvar->fiberMarshal(refMap);
  }

  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:    return Variant();
  case KindOfBoolean: return (m_data.num != 0);
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
        refMap.insert(mpvar, pvar, false); // ahead of deep copy
        *pvar = mpvar->fiberUnmarshal(refMap);
      }
      pvar->incRefCount();
      return pvar;
    }

    // i'm actually a weakly bound variant
    return mpvar->fiberUnmarshal(refMap);
  }

  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:    return Variant();
  case KindOfBoolean: return (m_data.num != 0);
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
  case KindOfUninit:
  case KindOfNull:    return "KindOfNull";
  case KindOfBoolean: return "KindOfBoolean";
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

VarNR::VarNR(CStrRef v) {
  init(KindOfString);
  StringData *s = v.get();
  if (s) {
    m_data.pstr = s;
  } else {
    m_type = KindOfNull;
  }
}

VarNR::VarNR(CArrRef v) {
  init(KindOfArray);
  ArrayData *a = v.get();
  if (a) {
    m_data.parr = a;
  } else {
    m_type = KindOfNull;
  }
}

VarNR::VarNR(CObjRef v) {
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
    set(Object(SystemLib::AllocStdClassObject()));
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
    obj = SystemLib::AllocStdClassObject();
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
