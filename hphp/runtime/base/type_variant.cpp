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
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/zend/zend_functions.h"
#include "hphp/runtime/base/variable_serializer.h"
#include "hphp/runtime/base/variable_unserializer.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/ext/ext_variable.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/zend/zend_string.h"
#include "hphp/runtime/base/array/array_iterator.h"
#include "hphp/util/parser/hphp.tab.hpp"
#include "hphp/runtime/vm/translator/translator-x64.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/instance.h"
#include "hphp/system/lib/systemlib.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/util/util.h"
#include "hphp/util/logger.h"

namespace HPHP {

const Variant null_variant;                         // uninitialized variant
const Variant init_null_variant(Variant::nullInit); // php null
const VarNR null_varNR;
const VarNR true_varNR(true);
const VarNR false_varNR(false);
const VarNR INF_varNR(std::numeric_limits<double>::infinity());
const VarNR NEGINF_varNR(std::numeric_limits<double>::infinity());
const VarNR NAN_varNR(std::numeric_limits<double>::quiet_NaN());

static void unserializeProp(VariableUnserializer *uns,
                            ObjectData *obj, CStrRef key,
                            CStrRef context, CStrRef realKey,
                            int nProp) NEVER_INLINE;

IMPLEMENT_SMART_ALLOCATION_HOT(Variant);

///////////////////////////////////////////////////////////////////////////////
// static strings

static StaticString s_offsetGet("offsetGet");
static StaticString s_offsetSet("offsetSet");
static StaticString s_offsetUnset("offsetUnset");
static StaticString s_s("s");
static StaticString s_scalar("scalar");
static StaticString s_array("Array");
static StaticString s_1("1");
static StaticString s_unserialize("unserialize");
static StaticString s_PHP_Incomplete_Class("__PHP_Incomplete_Class");
static StaticString s_PHP_Incomplete_Class_Name("__PHP_Incomplete_Class_Name");

///////////////////////////////////////////////////////////////////////////////
// local helpers

static int64_t ToKey(int64_t i) { return i; }
static VarNR ToKey(CStrRef s) { return s.toKey(); }
static VarNR ToKey(CVarRef v) { return v.toKey(); }

///////////////////////////////////////////////////////////////////////////////
// private implementations

Variant::Variant(litstr  v) {
  m_type = KindOfString;
  m_data.pstr = NEW(StringData)(v);
  m_data.pstr->incRefCount();
}

HOT_FUNC
Variant::Variant(CStrRef v) {
  m_type = KindOfString;
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

Variant::Variant(const std::string & v) {
  m_type = KindOfString;
  StringData *s = NEW(StringData)(v.c_str(), v.size(), CopyString);
  assert(s);
  m_data.pstr = s;
  s->incRefCount();
}

HOT_FUNC
Variant::Variant(CArrRef v) {
  m_type = KindOfArray;
  ArrayData *a = v.get();
  if (a) {
    m_data.parr = a;
    a->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

HOT_FUNC
Variant::Variant(CObjRef v) {
  m_type = KindOfObject;
  ObjectData *o = v.get();
  if (o) {
    m_data.pobj = o;
    o->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

HOT_FUNC
Variant::Variant(StringData *v) {
  m_type = KindOfString;
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

Variant::Variant(ArrayData *v) {
  m_type = KindOfArray;
  if (v) {
    m_data.parr = v;
    v->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(ObjectData *v) {
  m_type = KindOfObject;
  if (v) {
    m_data.pobj = v;
    v->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(RefData *r) {
  m_type = KindOfRef;
  if (r) {
    m_data.pref = r;
  } else {
    m_type = KindOfNull;
  }
}

// the version of the high frequency function that is not inlined
HOT_FUNC
Variant::Variant(CVarRef v) {
  constructValHelper(v);
}

HOT_FUNC
Variant::Variant(CVarStrongBind v) {
  constructRefHelper(variant(v));
}

Variant::Variant(CVarWithRefBind v) {
  constructWithRefHelper(variant(v), 0);
}

/*
 * The destruct functions below all arbitrarily take RefData* as an
 * example of a refcounted object, then just cast to the proper type.
 * This is safe because we have compile time assertions that guarantee that
 * the _count field will always be exactly FAST_REFCOUNT_OFFSET bytes from
 * the beginning of the object for the StringData, ArrayData, ObjectData,
 * and RefData classes.
 */

static_assert(TYPE_TO_DESTR_IDX(KindOfString) == 0, "String destruct index");
static_assert(TYPE_TO_DESTR_IDX(KindOfArray)  == 1,  "Array destruct index");
static_assert(TYPE_TO_DESTR_IDX(KindOfObject) == 2, "Object destruct index");
static_assert(TYPE_TO_DESTR_IDX(KindOfRef)    == 3,    "Ref destruct index");

static_assert(kDestrTableSize == 4,
              "size of g_destructors[] must be kDestrTableSize");

const RawDestructor g_destructors[] = {
  (RawDestructor)Util::getMethodPtr(&StringData::release),
  (RawDestructor)Util::getMethodPtr(&ArrayData::release),
  (RawDestructor)Util::getMethodPtr(&ObjectData::release),
  (RawDestructor)Util::getMethodPtr(&RefData::release),
};

inline ALWAYS_INLINE void Variant::destructDataImpl(RefData* data, DataType t) {
  assert(IS_REFCOUNTED_TYPE(t));
  assert(IS_REAL_TYPE(t));
  if (data->decRefCount() == 0) {
    assert(t >= KindOfString && t <= KindOfRef);
    g_destructors[typeToDestrIndex(t)](data);
  }
}

inline ALWAYS_INLINE void Variant::destructImpl() {
  destructDataImpl(m_data.pref, m_type);
}

HOT_FUNC_VM
void tvDecRefHelper(DataType type, uintptr_t datum) {
  assert(type >= KindOfString && type <= KindOfRef);
  if (((RefData*)datum)->decRefCount() == 0) {
    g_destructors[typeToDestrIndex(type)]((void*)datum);
  }
}

HOT_FUNC
void Variant::destruct() {
  destructImpl();
}

HOT_FUNC
void Variant::destructData(RefData* data, DataType t) {
  destructDataImpl(data, t);
}

HOT_FUNC
Variant::~Variant() {
  if (IS_REFCOUNTED_TYPE(m_type)) destructImpl();
}

HOT_FUNC
Variant &Variant::assign(CVarRef v) {
  AssignValHelper(this, &v);
  return *this;
}

HOT_FUNC
Variant &Variant::assignRef(CVarRef v) {
  assignRefHelper(v);
  return *this;
}

HOT_FUNC
Variant &Variant::setWithRef(CVarRef v, const ArrayData *arr /* = NULL */) {
  setWithRefHelper(v, arr, IS_REFCOUNTED_TYPE(m_type));
  return *this;
}

#define IMPLEMENT_SET_IMPL(name, argType, argName, setOp, returnStmt)   \
  Variant::name(argType argName) {                                      \
    if (isPrimitive()) {                                                \
      setOp;                                                            \
    } else if (m_type == KindOfRef) {                                   \
      m_data.pref->var()->name(argName);                                \
      returnStmt;                                                       \
    } else {                                                            \
      RefData* d = m_data.pref;                                         \
      DataType t = m_type;                                              \
      setOp;                                                            \
      destructData(d, t);                                               \
    }                                                                   \
    returnStmt;                                                         \
  }
#define IMPLEMENT_VOID_SET(name, setOp) \
  void IMPLEMENT_SET_IMPL(name, , , setOp, return)
#define IMPLEMENT_SET(argType, setOp) \
  CVarRef IMPLEMENT_SET_IMPL(set, argType, v, setOp, return *this)

IMPLEMENT_VOID_SET(setNull, m_type = KindOfNull)
HOT_FUNC IMPLEMENT_SET(bool, m_type = KindOfBoolean; m_data.num = v)
IMPLEMENT_SET(int, m_type = KindOfInt64; m_data.num = v)
HOT_FUNC IMPLEMENT_SET(int64_t, m_type = KindOfInt64; m_data.num = v)
IMPLEMENT_SET(double, m_type = KindOfDouble; m_data.dbl = v)
IMPLEMENT_SET(const StaticString&,
              StringData* s = v.get();
              assert(s);
              m_type = KindOfStaticString;
              m_data.pstr = s)

#undef IMPLEMENT_SET_IMPL
#undef IMPLEMENT_VOID_SET
#undef IMPLEMENT_SET

#define IMPLEMENT_PTR_SET(ptr, member, dtype)                           \
  CVarRef Variant::set(ptr *v) {                                        \
    Variant *self = m_type == KindOfRef ? m_data.pref->var() : this;    \
    if (UNLIKELY(!v)) {                                                 \
      self->setNull();                                                  \
    } else {                                                            \
      v->incRefCount();                                                 \
      RefData* d = self->m_data.pref;                                   \
      DataType t = self->m_type;                                        \
      self->m_type = dtype;                                             \
      self->m_data.member = v;                                          \
      if (IS_REFCOUNTED_TYPE(t)) destructData(d, t);                    \
    }                                                                   \
    return *this;                                                       \
  }

HOT_FUNC IMPLEMENT_PTR_SET(StringData, pstr,
                           v->isStatic() ? KindOfStaticString : KindOfString);
HOT_FUNC IMPLEMENT_PTR_SET(ArrayData, parr, KindOfArray)
HOT_FUNC IMPLEMENT_PTR_SET(ObjectData, pobj, KindOfObject)

#undef IMPLEMENT_PTR_SET

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
  case KindOfRef: m_data.pref->var()->split();     break;
  // copy-on-write
  case KindOfStaticString:
  case KindOfString:
  {
    set(NEW(StringData)(m_data.pstr, CopyString));
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
  case KindOfRef: return m_data.pref->var()->getRefCount();
  default:
    break;
  }
  return 1;
}

///////////////////////////////////////////////////////////////////////////////
// informational

bool Variant::isInteger() const {
  switch (m_type) {
    case KindOfInt64:
      return true;
    case KindOfRef:
      return m_data.pref->var()->isInteger();
    default:
      break;
  }
  return false;
}

HOT_FUNC
bool Variant::isNumeric(bool checkString /* = false */) const {
  int64_t ival;
  double dval;
  DataType t = toNumeric(ival, dval, checkString);
  return t == KindOfInt64 || t == KindOfDouble;
}

DataType Variant::toNumeric(int64_t &ival, double &dval,
    bool checkString /* = false */) const {
  switch (m_type) {
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
  case KindOfRef:
    return m_data.pref->var()->toNumeric(ival, dval, checkString);
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
  TypedValueAccessor acc = getTypedAccessor();
  if (GetAccessorType(acc) == KindOfObject) {
    return GetObjectData(acc)->isResource();
  }
  return false;
}

HOT_FUNC
bool Variant::instanceof(CStrRef s) const {
  if (m_type == KindOfObject) {
    assert(m_data.pobj);
    return m_data.pobj->o_instanceof(s);
  }
  if (m_type == KindOfRef) {
    return m_data.pref->var()->instanceof(s);
  }
  return false;
}

HOT_FUNC
bool Variant::instanceof(Class* cls) const {
  if (m_type == KindOfObject) {
    assert(m_data.pobj);
    return m_data.pobj->instanceof(cls);
  }
  if (m_type == KindOfRef) {
    return m_data.pref->var()->instanceof(cls);
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// array operations

Variant Variant::pop() {
  if (m_type == KindOfRef) {
    return m_data.pref->var()->pop();
  }
  if (!is(KindOfArray)) {
    throw_bad_type_exception("expecting an array");
    return null_variant;
  }

  Variant ret;
  ArrayData* a = getArrayData();
  ArrayData* newarr = a->pop(ret);
  if (newarr != a) set(newarr);
  return ret;
}

Variant Variant::dequeue() {
  if (m_type == KindOfRef) {
    return m_data.pref->var()->dequeue();
  }
  if (!is(KindOfArray)) {
    throw_bad_type_exception("expecting an array");
    return null_variant;
  }

  Variant ret;
  ArrayData* a = getArrayData();
  ArrayData* newarr = a->dequeue(ret);
  if (newarr != a) set(newarr);
  return ret;
}

void Variant::prepend(CVarRef v) {
  if (m_type == KindOfRef) {
    m_data.pref->var()->prepend(v);
    return;
  }
  if (isNull()) set(Array::Create());
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    ArrayData *newarr = arr->prepend(v, (arr->getCount() > 1));
    if (newarr != arr) set(newarr);
  } else {
    throw_bad_type_exception("expecting an array");
  }
}

Variant Variant::array_iter_reset() {
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1 && !arr->isHead() && !arr->noCopyOnWrite()) {
      arr = arr->copy();
      set(arr);
      assert(arr == getArrayData());
    }
    return arr->reset();
  }
  throw_bad_type_exception("expecting an array");
  return false;
}

Variant Variant::array_iter_prev() {
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1 && !arr->isInvalid() && !arr->noCopyOnWrite()) {
      arr = arr->copy();
      set(arr);
      assert(arr == getArrayData());
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
    escalate();
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1 && !arr->noCopyOnWrite()) {
      arr = arr->copy();
      set(arr);
      assert(arr == getArrayData());
    }
    return strongBind(arr->currentRef());
  }
  throw_bad_type_exception("expecting an array");
  return false;
}

Variant Variant::array_iter_next() {
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1 && !arr->isInvalid() && !arr->noCopyOnWrite()) {
      arr = arr->copy();
      set(arr);
      assert(arr == getArrayData());
    }
    return arr->next();
  }
  throw_bad_type_exception("expecting an array");
  return false;
}

Variant Variant::array_iter_end() {
  if (is(KindOfArray)) {
    ArrayData *arr = getArrayData();
    if (arr->getCount() > 1 && !arr->isTail() && !arr->noCopyOnWrite()) {
      arr = arr->copy();
      set(arr);
      assert(arr == getArrayData());
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
    if (arr->getCount() > 1 && !arr->isInvalid() && !arr->noCopyOnWrite()) {
      arr = arr->copy();
      set(arr);
      assert(arr == getArrayData());
    }
    return arr->each();
  }
  throw_bad_type_exception("expecting an array");
  return null_variant;
}

inline DataType Variant::convertToNumeric(int64_t *lval, double *dval) const {
  StringData *s = getStringData();
  assert(s);
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
    int64_t lval; double dval;
    DataType ret = convertToNumeric(&lval, &dval);
    if (ret == KindOfDouble) {
      return dval;
    }
    if (ret == KindOfInt64) {
      return lval;
    }
    return toInt64();
  }

  assert(false);
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// add or array append

Variant operator+(const Variant & lhs, const Variant & rhs) {
  // Frequent case: two straight integers
  if (lhs.m_type == KindOfInt64 && rhs.m_type == KindOfInt64) {
    return lhs.m_data.num + rhs.m_data.num;
  }
  // Frequent case: two straight doubles
  if (lhs.m_type == KindOfDouble && rhs.m_type == KindOfDouble) {
    return lhs.m_data.dbl + rhs.m_data.dbl;
  }
  // Less frequent cases involving references etc.
  if (lhs.isIntVal() && rhs.isIntVal()) {
    return lhs.toInt64() + rhs.toInt64();
  }
  if (lhs.isDouble() || rhs.isDouble()) {
    return lhs.toDouble() + rhs.toDouble();
  }
  if (lhs.isString()) {
    int64_t lval; double dval;
    if (lhs.convertToNumeric(&lval, &dval) == KindOfDouble) {
      return dval + rhs.toDouble();
    }
  }
  if (rhs.isString()) {
    int64_t lval; double dval;
    if (rhs.convertToNumeric(&lval, &dval) == KindOfDouble) {
      return lhs.toDouble() + dval;
    }
  }
  int na = lhs.is(KindOfArray) + rhs.is(KindOfArray);
  if (na == 2) {
    return lhs.toCArrRef() + rhs.toCArrRef();
  } else if (na) {
    throw BadArrayMergeException();
  }
  return lhs.toInt64() + rhs.toInt64();
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
    if (arr1 == nullptr || arr2 == nullptr) {
      throw BadArrayMergeException();
    }
    if (arr2->empty() || arr1 == arr2) return *this;
    if (arr1->empty()) {
      set(arr2);
      return *this;
    }
    ArrayData *escalated = arr1->append(arr2, ArrayData::Plus,
                                        arr1->getCount() > 1);
    if (escalated != arr1) set(escalated);
    return *this;
  } else if (na) {
    throw BadArrayMergeException();
  }
  if (isDouble() || var.isDouble()) {
    set(toDouble() + var.toDouble());
    return *this;
  }
  if (isString()) {
    int64_t lval; double dval;
    DataType ret = convertToNumeric(&lval, &dval);
    if (ret == KindOfDouble) {
      set(dval + var.toDouble());
      return *this;
    }
  }
  if (var.isString()) {
    int64_t lval; double dval;
    DataType ret = var.convertToNumeric(&lval, &dval);
    if (ret == KindOfDouble) {
      set(toDouble() + dval);
      return *this;
    }
  }
  set(toInt64() + var.toInt64());
  return *this;
}

Variant &Variant::operator+=(int64_t n) {
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
    int64_t lval; double dval;
    DataType ret = convertToNumeric(&lval, &dval);
    if (ret == KindOfDouble) {
      set(dval + n);
      return *this;
    }
  } else {
    assert(false);
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
      int64_t lval; double dval;
      DataType ret = convertToNumeric(&lval, &dval);
      if (ret == KindOfDouble) {
        return -dval;
      } else if (ret == KindOfInt64) {
        return -lval;
      } else {
        return -toInt64();
      }
    } else {
      assert(false);
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
    int64_t lval; double dval;
    DataType ret = convertToNumeric(&lval, &dval);
    if (ret == KindOfDouble) {
      return dval - var.toDouble();
    }
  }
  if (var.isString()) {
    int64_t lval; double dval;
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
      int64_t lval; double dval;
      DataType ret = convertToNumeric(&lval, &dval);
      if (ret == KindOfDouble) {
        set(dval - var.toDouble());
        return *this;
      }
    }
    if (var.isString()) {
      int64_t lval; double dval;
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

Variant &Variant::operator-=(int64_t n) {
  if (is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (isDouble()) {
    set(toDouble() - n);
  } else if (isIntVal()) {
    set(toInt64() - n);
  } else {
    if (isString()) {
      int64_t lval; double dval;
      DataType ret = convertToNumeric(&lval, &dval);
      if (ret == KindOfDouble) {
        set(dval - n);
        return *this;
      }
    } else {
      assert(false);
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
    int64_t lval; double dval;
    DataType ret = convertToNumeric(&lval, &dval);
    if (ret == KindOfDouble) {
      return dval * var.toDouble();
    }
  }
  if (var.isString()) {
    int64_t lval; double dval;
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
      int64_t lval; double dval;
      DataType ret = convertToNumeric(&lval, &dval);
      if (ret == KindOfDouble) {
        set(dval * var.toDouble());
        return *this;
      }
    }
    if (var.isString()) {
      int64_t lval; double dval;
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

Variant &Variant::operator*=(int64_t n) {
  if (is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (isDouble()) {
    set(toDouble() * n);
  } else if (isIntVal()) {
    set(toInt64() * n);
  } else {
    if (isString()) {
      int64_t lval; double dval;
      DataType ret = convertToNumeric(&lval, &dval);
      if (ret == KindOfDouble) {
        set(dval * n);
        return *this;
      }
    } else {
      assert(false);
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
  int64_t lval; double dval; bool int1 = true;
  int64_t lval2; double dval2; bool int2 = true;

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
    assert(false);
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
    assert(false);
  }

  if ((int2 && lval2 == 0) || (!int2 && dval2 == 0)) {
    raise_warning(Strings::DIVISION_BY_ZERO);
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
  int64_t lval; double dval; bool int1 = true;
  int64_t lval2; double dval2; bool int2 = true;

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
    assert(false);
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
    assert(false);
  }

  if ((int2 && lval2 == 0) || (!int2 && dval2 == 0)) {
    raise_warning(Strings::DIVISION_BY_ZERO);
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

Variant &Variant::operator/=(int64_t n) {
  if (is(KindOfArray)) {
    throw BadArrayOperandException();
  }
  if (n == 0) {
    raise_warning(Strings::DIVISION_BY_ZERO);
    set(false);
    return *this;
  }

  if (isIntVal() && toInt64() % n == 0) {
    set(toInt64() / n);
  } else if (isDouble()) {
    set(toDouble() / n);
  } else {
    if (isString()) {
      int64_t lval; double dval;
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
    raise_warning(Strings::DIVISION_BY_ZERO);
    set(false);
    return *this;
  }
  set(toDouble() / n);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// modulus

int64_t Variant::operator%(CVarRef var) const {
  int64_t lval = toInt64();
  int64_t lval2 = var.toInt64();
  if (lval2 == 0) {
    raise_warning(Strings::DIVISION_BY_ZERO);
    return false;
  }
  return lval % lval2;
}

Variant &Variant::operator%=(CVarRef var) {
  int64_t lval = toInt64();
  int64_t lval2 = var.toInt64();
  if (lval2 == 0) {
    raise_warning(Strings::DIVISION_BY_ZERO);
    set(false);
    return *this;
  }
  set(lval % lval2);
  return *this;
}

Variant &Variant::operator%=(int64_t n) {
  if (n == 0) {
    raise_warning(Strings::DIVISION_BY_ZERO);
    set(false);
    return *this;
  }
  set(toInt64() % n);
  return *this;
}

Variant &Variant::operator%=(double n) {
  if ((int64_t)n == 0) {
    raise_warning(Strings::DIVISION_BY_ZERO);
    set(false);
    return *this;
  }
  set(toInt64() % (int64_t)n);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// bitwise

Variant Variant::operator~() const {
  TypedValueAccessor tva = getTypedAccessor();
  switch (GetAccessorType(tva)) {
  case KindOfInt64:
    return ~GetInt64(tva);
  case KindOfDouble:
    return ~toInt64(GetDouble(tva));
  case KindOfStaticString:
  case KindOfString:
    return ~GetAsString(tva);
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

Variant &Variant::operator<<=(int64_t n) {
  set(toInt64() << n);
  return *this;
}

Variant &Variant::operator>>=(int64_t n) {
  set(toInt64() >> n);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// increment/decrement

Variant &Variant::operator++() {
  switch (getType()) {
  case KindOfUninit:
  case KindOfNull:   set(int64_t(1)); break;
  case KindOfInt64:  set(toInt64() + 1);  break;
  case KindOfDouble: set(toDouble() + 1); break;
  case KindOfStaticString:
  case KindOfString:
    {
      if (getStringData()->empty()) {
        set(s_1);
      } else {
        int64_t lval; double dval;
        DataType ret = convertToNumeric(&lval, &dval);
        switch (ret) {
        case KindOfInt64:  set(lval + 1); break;
        case KindOfDouble: set(dval + 1); break;
        case KindOfUninit:
        case KindOfNull:
          split();
          getStringData()->inc(); break;
        default:
          assert(false);
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
  case KindOfInt64:  set(toInt64() - 1);  break;
  case KindOfDouble: set(toDouble() - 1); break;
  case KindOfStaticString:
  case KindOfString:
    {
      if (getStringData()->empty()) {
        set(int64_t(-1LL));
      } else {
        int64_t lval; double dval;
        DataType ret = convertToNumeric(&lval, &dval);
        switch (ret) {
        case KindOfInt64:  set(lval - 1);   break;
        case KindOfDouble: set(dval - 1);   break;
        case KindOfUninit:
        case KindOfNull:   /* do nothing */ break;
        default:
          assert(false);
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

HOT_FUNC
ArrayIter Variant::begin(CStrRef context /* = null_string */) const {
  if (is(KindOfArray)) {
    return ArrayIter(getArrayData());
  }
  if (is(KindOfObject)) {
    return getObjectData()->begin(context);
  }
  raise_warning("Invalid argument supplied for foreach()");
  return ArrayIter();
}

HOT_FUNC
MutableArrayIter Variant::begin(Variant *key, Variant &val,
                                CStrRef context /* = null_string */) {
  if (is(KindOfObject)) {
    return getObjectData()->begin(key, val, context);
  }
  return MutableArrayIter(this, key, val);
}

void Variant::escalate() {
  TypedValueAccessor tva = getTypedAccessor();
  if (GetAccessorType(tva) == KindOfArray) {
    ArrayData *arr = GetArrayData(tva);
    ArrayData *esc = arr->escalate();
    if (arr != esc) set(esc);
  }
}

///////////////////////////////////////////////////////////////////////////////
// type conversions

bool Variant::toBooleanHelper() const {
  assert(m_type > KindOfInt64);
  switch (m_type) {
  case KindOfDouble:  return m_data.dbl != 0;
  case KindOfStaticString:
  case KindOfString:  return m_data.pstr->toBoolean();
  case KindOfArray:   return !m_data.parr->empty();
  case KindOfObject:  return m_data.pobj->o_toBoolean();
  case KindOfRef: return m_data.pref->var()->toBoolean();
  default:
    assert(false);
    break;
  }
  return m_data.num;
}

int64_t Variant::toInt64Helper(int base /* = 10 */) const {
  assert(m_type > KindOfInt64);
  switch (m_type) {
  case KindOfDouble:  {
    return HPHP::toInt64(m_data.dbl);
  }
  case KindOfStaticString:
  case KindOfString:  return m_data.pstr->toInt64(base);
  case KindOfArray:   return m_data.parr->empty() ? 0 : 1;
  case KindOfObject:  return m_data.pobj->o_toInt64();
  case KindOfRef: return m_data.pref->var()->toInt64(base);
  default:
    assert(false);
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
  case KindOfRef: return m_data.pref->var()->toDouble();
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
    assert(false); // Should be done in caller
    return m_data.pstr;
  case KindOfArray:   return s_array;
  case KindOfObject:  return m_data.pobj->t___tostring();
  case KindOfRef: return m_data.pref->var()->toString();
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
  case KindOfRef: return m_data.pref->var()->toArray();
  default:
    break;
  }
  return Array::Create(*this);
}

Object Variant::toObjectHelper() const {
  if (m_type == KindOfRef) return m_data.pref->var()->toObject();

  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:
    break;
  case KindOfBoolean:
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
    assert(false);
    break;
  }
  return Object(SystemLib::AllocStdClassObject());
}

HOT_FUNC
VarNR Variant::toKey() const {
  if (m_type == KindOfString || m_type == KindOfStaticString) {
    int64_t n;
    if (m_data.pstr->isStrictlyInteger(n)) {
      return VarNR(n);
    } else {
      return VarNR(m_data.pstr);
    }
  }
  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:
    return VarNR(empty_string);
  case KindOfBoolean:
  case KindOfInt64:
    return VarNR(m_data.num);
  case KindOfDouble:
    return VarNR(ToKey(m_data.dbl));
  case KindOfObject:
    if (isResource()) {
      return VarNR(toInt64());
    }
    break;
  case KindOfRef:
    return m_data.pref->var()->toKey();
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
  return same((int64_t)v2);
}

bool Variant::same(int64_t v2) const {
  TypedValueAccessor acc = getTypedAccessor();
  switch (GetAccessorType(acc)) {
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
  StackStringData sd2(v2);
  return same(&sd2);
}

bool Variant::same(const StringData *v2) const {
  bool null1 = isNull();
  bool null2 = (v2 == nullptr);
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
  case KindOfInt64: {
    TypedValueAccessor acc2 = v2.getTypedAccessor();
    switch (GetAccessorType(acc2)) {
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
  case KindOfInt64:   return HPHP::reverse(v2, GetInt64(acc));             \
  case KindOfDouble:  return HPHP::reverse(v2, GetDouble(acc));            \
  case KindOfStaticString:                                                 \
  case KindOfString:  return HPHP::reverse(v2, GetStringData(acc));        \
  case KindOfArray:   return HPHP::reverse(v2, Array(GetArrayData(acc)));  \
  case KindOfObject:  return HPHP::reverse(v2, Object(GetObjectData(acc)));\
  default:                                                                 \
    assert(false);                                                         \
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
  case KindOfInt64:   return HPHP::reverse(v2, GetInt64(acc));             \
  case KindOfDouble:  return HPHP::reverse(v2, GetDouble(acc));            \
  case KindOfStaticString:                                                 \
  case KindOfString:  return HPHP::reverse(v2, GetStringData(acc));        \
  case KindOfArray:   return HPHP::reverse(v2, Array(GetArrayData(acc)));  \
  case KindOfObject:  return HPHP::reverse(v2, Object(GetObjectData(acc)));\
  default:                                                                 \
    assert(false);                                                         \
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
  case KindOfInt64:   return HPHP::reverse(v2, GetInt64(acc));             \
  case KindOfDouble:  return HPHP::reverse(v2, GetDouble(acc));            \
  case KindOfStaticString:                                                 \
  case KindOfString:  return HPHP::reverse(v2, GetStringData(acc));        \
  case KindOfArray:   return HPHP::reverse(v2, s_array);                   \
  case KindOfObject:                                                       \
    return HPHP::reverse(v2, Object(GetObjectData(acc)).toString());       \
  default:                                                                 \
    assert(false);                                                         \
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
    assert(false);                                                         \
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
  case KindOfInt64:   return HPHP::reverse(v2, GetInt64(acc));             \
  case KindOfDouble:  return HPHP::reverse(v2, GetDouble(acc));            \
  case KindOfStaticString:                                                 \
  case KindOfString:  return HPHP::reverse(v2, GetStringData(acc));        \
  case KindOfArray:   return Array(GetArrayData(acc)).forward(v2);         \
  case KindOfObject:  return HPHP::reverse(v2, Object(GetObjectData(acc)));\
  default:                                                                 \
    assert(false);                                                         \
    break;                                                                 \
  }                                                                        \
  return false;                                                            \

bool Variant::equal(bool    v2) const { UNWRAP(equal);}
bool Variant::equal(int     v2) const { UNWRAP(equal);}
HOT_FUNC
bool Variant::equal(int64_t   v2) const { UNWRAP(equal);}
bool Variant::equal(double  v2) const { UNWRAP(equal);}
bool Variant::equal(litstr  v2) const { UNWRAP_STR(equal);}
bool Variant::equal(const StringData *v2) const { UNWRAP_STR(equal);}
HOT_FUNC
bool Variant::equal(CStrRef v2) const { UNWRAP_STR(equal);}
bool Variant::equal(CArrRef v2) const { UNWRAP(equal);}
bool Variant::equal(CObjRef v2) const { UNWRAP(equal);}
HOT_FUNC
bool Variant::equal(CVarRef v2) const { UNWRAP_VAR(equal,equal);}

bool Variant::equalAsStr(bool    v2) const { UNWRAP_STRING(equalAsStr);}
bool Variant::equalAsStr(int     v2) const { UNWRAP_STRING(equalAsStr);}
bool Variant::equalAsStr(int64_t   v2) const { UNWRAP_STRING(equalAsStr);}
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
bool Variant::less(int64_t   v2) const { UNWRAP(more);}
bool Variant::less(double  v2) const { UNWRAP(more);}
bool Variant::less(litstr  v2) const { UNWRAP_STR(more);}
bool Variant::less(const StringData *v2) const { UNWRAP_STR(more);}
bool Variant::less(CStrRef v2) const { UNWRAP_STR(more);}
bool Variant::less(CArrRef v2) const { UNWRAP_ARR(less,more);}
bool Variant::less(CObjRef v2) const { UNWRAP(more);}
HOT_FUNC
bool Variant::less(CVarRef v2) const { UNWRAP_VAR(less,more);}

bool Variant::more(bool    v2) const { UNWRAP(less);}
bool Variant::more(int     v2) const { UNWRAP(less);}
HOT_FUNC
bool Variant::more(int64_t   v2) const { UNWRAP(less);}
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
  return more_or_equal(*this, v);
}

bool Variant::operator<=(CVarRef v) const {
  return less_or_equal(*this, v);
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
  assert(is(KindOfObject));
  ObjectData *obj = getObjectData();
  assert(obj);
  if (!obj->instanceof(SystemLib::s_ArrayAccessClass)) {
    throw InvalidOperandException("not ArrayAccess objects");
  }
  return obj;
}

void Variant::callOffsetUnset(CVarRef key) {
  assert(getType() == KindOfObject);
  ObjectData* obj = getObjectData();
  if (LIKELY(obj->isCollection())) {
    collectionOffsetUnset(obj, key);
  } else {
    getArrayAccess()->o_invoke_few_args(s_offsetUnset, 1, key);
  }
}

static void raise_bad_offset_notice() {
  if (RuntimeOption::EnableHipHopErrors) {
    raise_notice("taking offset [] on bool or number");
  }
}

#define IMPLEMENT_RVAL_INTEGRAL                                         \
  if (m_type == KindOfArray) {                                          \
    return m_data.parr->get(ToKey(offset), flags & AccessFlags::Error); \
  }                                                                     \
  switch (m_type) {                                                     \
    case KindOfStaticString:                                            \
    case KindOfString:                                                  \
      return m_data.pstr->getChar((int)offset);                         \
    case KindOfObject: {                                                \
      ObjectData* obj = m_data.pobj;                                    \
      if (obj->isCollection()) {                                        \
        return collectionOffsetGet(obj, offset);                        \
      } else {                                                          \
        return getArrayAccess()->o_invoke_few_args(s_offsetGet,         \
                                                   1, offset);          \
      }                                                                 \
      break;                                                            \
    }                                                                   \
    case KindOfRef:                                                     \
      return m_data.pref->var()->rvalAt(offset, flags);                 \
    case KindOfUninit:                                                  \
    case KindOfNull:                                                    \
      break;                                                            \
    default:                                                            \
      if ((flags & AccessFlags::Error) &&                               \
          !(flags & AccessFlags::NoHipHop)) {                           \
        raise_bad_offset_notice();                                      \
      }                                                                 \
      break;                                                            \
  }                                                                     \
  return null_variant;

Variant Variant::rvalAt(double offset, ACCESSPARAMS_IMPL) const {
  IMPLEMENT_RVAL_INTEGRAL
}

Variant Variant::rvalAtHelper(int64_t offset, ACCESSPARAMS_IMPL) const {
  switch (m_type) {
  case KindOfStaticString:
  case KindOfString:
    return m_data.pstr->getChar((int)offset);
  case KindOfObject: {
    ObjectData* obj = m_data.pobj;
    if (LIKELY(obj->isCollection())) {
      return collectionOffsetGet(obj, offset);
    } else {
      return getArrayAccess()->o_invoke_few_args(s_offsetGet, 1, offset);
    }
    break;
  }
  case KindOfRef:
    return m_data.pref->var()->rvalAt(offset, flags);
  case KindOfUninit:
  case KindOfNull:
    break;
  default:
    if ((flags & AccessFlags::Error) && !(flags & AccessFlags::NoHipHop)) {
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
    int64_t n;
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
  case KindOfObject: {
    ObjectData* obj = m_data.pobj;
    if (LIKELY(obj->isCollection())) {
      return collectionOffsetGet(obj, offset);
    } else {
      return getArrayAccess()->o_invoke_few_args(s_offsetGet, 1, offset);
    }
    break;
  }
  case KindOfRef:
    return m_data.pref->var()->rvalAt(offset, flags);
  case KindOfUninit:
  case KindOfNull:
    break;
  default:
    if ((flags & AccessFlags::Error) && !(flags & AccessFlags::NoHipHop)) {
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
    case KindOfInt64:
      return m_data.parr->get(offset.m_data.num, flags & AccessFlags::Error);
    case KindOfDouble:
      return m_data.parr->get((int64_t)offset.m_data.dbl,
                              flags & AccessFlags::Error);
    case KindOfStaticString:
    case KindOfString: {
      int64_t n;
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
    case KindOfRef:
      return rvalAt(*(offset.m_data.pref->var()), flags);
    default:
      assert(false);
      break;
    }
    return null_variant;
  }
  switch (m_type) {
  case KindOfStaticString:
  case KindOfString:
    return m_data.pstr->getChar(offset.toInt32());
  case KindOfObject: {
    ObjectData* obj = m_data.pobj;
    if (LIKELY(obj->isCollection())) {
      return collectionOffsetGet(obj, offset);
    } else {
      return getArrayAccess()->o_invoke_few_args(s_offsetGet, 1, offset);
    }
    break;
  }
  case KindOfRef:
    return m_data.pref->var()->rvalAt(offset, flags);
  case KindOfUninit:
  case KindOfNull:
    break;
  default:
    if ((flags & AccessFlags::Error) && !(flags & AccessFlags::NoHipHop)) {
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
  case KindOfObject: {
    ObjectData* obj = m_data.pobj;
    if (LIKELY(obj->isCollection())) {
      return collectionOffsetGet(obj, offset);
    } else {
      const_cast<Variant&>(tmp) =
        getArrayAccess()->o_invoke_few_args(s_offsetGet, 1, offset);
      return tmp;
    }
    break;
  }
  case KindOfRef:
    return m_data.pref->var()->rvalRef(offset, tmp, flags);
  case KindOfUninit:
  case KindOfNull:
    break;
  default:
    if ((flags & AccessFlags::Error) && !(flags & AccessFlags::NoHipHop)) {
      raise_bad_offset_notice();
    }
    break;
  }
  return null_variant;
}

template CVarRef
Variant::rvalRefHelper(int64_t offset, CVarRef tmp, ACCESSPARAMS_IMPL) const;

CVarRef Variant::rvalRef(double offset, CVarRef tmp, ACCESSPARAMS_IMPL) const {
  if (m_type == KindOfArray) {
    return m_data.parr->get(ToKey(offset), flags & AccessFlags::Error);
  }
  return rvalRefHelper(offset, tmp, flags);
}

CVarRef Variant::rvalRef(CStrRef offset, CVarRef tmp, ACCESSPARAMS_IMPL) const {
  if (m_type == KindOfArray) {
    bool error = flags & AccessFlags::Error;
    if (flags & AccessFlags::Key) return m_data.parr->get(offset, error);
    if (offset.isNull()) return m_data.parr->get(empty_string, error);
    int64_t n;
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
    case KindOfInt64:
      return m_data.parr->get(offset.m_data.num, flags & AccessFlags::Error);
    case KindOfDouble:
      return m_data.parr->get((int64_t)offset.m_data.dbl,
                              flags & AccessFlags::Error);
    case KindOfStaticString:
    case KindOfString: {
      int64_t n;
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
    case KindOfRef:
      return rvalRef(*(offset.m_data.pref->var()), tmp, flags);
    default:
      assert(false);
      break;
    }
    return null_variant;
  }
  return rvalRefHelper(offset, tmp, flags);
}

template <typename T>
CVarRef Variant::rvalAtRefHelper(T offset, ACCESSPARAMS_IMPL) const {
  if (LIKELY(m_type == KindOfArray)) {
    return asCArrRef().rvalAtRef(offset, flags);
  }
  if (LIKELY(m_type == KindOfRef)) {
    return m_data.pref->var()->rvalAtRefHelper<T>(offset, flags);
  }
  return null_variant;
}

template
CVarRef Variant::rvalAtRefHelper<int64_t>(int64_t offset, ACCESSPARAMS_IMPL) const;
template
CVarRef Variant::rvalAtRefHelper<CStrRef>(CStrRef offset,
                                          ACCESSPARAMS_IMPL) const;
template
CVarRef Variant::rvalAtRefHelper<CVarRef>(CVarRef offset,
                                          ACCESSPARAMS_IMPL) const;
CVarRef Variant::rvalAtRef(double offset, ACCESSPARAMS_IMPL) const {
  return rvalAtRefHelper(HPHP::toInt64(offset), flags);
}

template <typename T>
class LvalHelper {};

template<>
class LvalHelper<int64_t> {
public:
  typedef int64_t KeyType;
  static bool CheckKey(KeyType k) { return true; };
  static const bool CheckParams = false;
};

template<>
class LvalHelper<bool> : public LvalHelper<int64_t> {};

template<>
class LvalHelper<double> : public LvalHelper<int64_t> {};

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
Variant& Variant::LvalAtImpl0(
    Variant *self, T key, Variant *tmp, bool blackHole, ACCESSPARAMS_IMPL) {
head:
  if (self->m_type == KindOfArray) {
    ArrayData *arr = self->m_data.parr;
    ArrayData *escalated;
    Variant *ret = nullptr;
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
        if (blackHole) ret = &lvalBlackHole();
        else           ret = tmp;
        escalated = arr;
      }
    }
    if (escalated != arr) {
      self->set(escalated);
    }
    assert(ret);
    return *ret;
  }
  if (self->m_type == KindOfRef) {
    self = self->m_data.pref->var();
    goto head;
  }
  if (self->isObjectConvertable()) {
    self->set(Array::Create());
    goto head;
  }
  if (self->m_type == KindOfObject) {
    if (self->m_data.pobj->isCollection()) {
      return collectionOffsetGet(self->m_data.pobj, Variant(key));
    }
    if (!blackHole) {
      *tmp = self->getArrayAccess()->offsetGet(key);
      return *tmp;
    }
    Variant& retv = get_global_variables()->__lvalProxy;
    retv = self->getArrayAccess()->offsetGet(key);
    return retv;
  }
  return lvalInvalid();
}

template<typename T>
Variant& Variant::lvalAtImpl(T key, ACCESSPARAMS_IMPL) {
  return Variant::LvalAtImpl0<T>(this, key, nullptr, true, flags);
}

Variant &Variant::lvalAt(int     key, ACCESSPARAMS_IMPL) {
  return lvalAt((int64_t)key, flags);
}
Variant &Variant::lvalAt(int64_t   key, ACCESSPARAMS_IMPL) {
  return lvalAtImpl(key, flags);
}
Variant &Variant::lvalAt(double  key, ACCESSPARAMS_IMPL) {
  return lvalAtImpl(key, flags);
}
Variant &Variant::lvalAt(CStrRef key, ACCESSPARAMS_IMPL) {
  return lvalAtImpl<CStrRef>(key, flags);
}
Variant &Variant::lvalAt(CVarRef k, ACCESSPARAMS_IMPL) {
  return lvalAtImpl<CVarRef>(k, flags);
}

Variant &Variant::lvalRef(int     key, Variant& tmp, ACCESSPARAMS_IMPL) {
  return lvalRef((int64_t)key, tmp, flags);
}
Variant &Variant::lvalRef(int64_t   key, Variant& tmp, ACCESSPARAMS_IMPL) {
  return LvalAtImpl0(this, key, &tmp, false, flags);
}
Variant &Variant::lvalRef(double  key, Variant& tmp, ACCESSPARAMS_IMPL) {
  return LvalAtImpl0(this, key, &tmp, false, flags);
}
Variant &Variant::lvalRef(CStrRef key, Variant& tmp, ACCESSPARAMS_IMPL) {
  return Variant::LvalAtImpl0<CStrRef>(this, key, &tmp, false, flags);
}
Variant &Variant::lvalRef(CVarRef k, Variant& tmp, ACCESSPARAMS_IMPL) {
  return Variant::LvalAtImpl0<CVarRef>(this, k, &tmp, false, flags);
}

Variant *Variant::lvalPtr(CStrRef key, bool forWrite, bool create) {
  Variant *t = m_type == KindOfRef ? m_data.pref->var() : this;
  if (t->m_type == KindOfArray) {
    return t->asArrRef().lvalPtr(key, forWrite, create);
  }
  return nullptr;
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
  case KindOfRef:
    return m_data.pref->var()->lvalAt();
  case KindOfObject:
    {
      ObjectData* obj = m_data.pobj;
      if (obj->isCollection()) {
        raise_error("Cannot use [] for reading");
      }
      Variant& ret = lvalBlackHole();
      ret = m_data.pobj->o_invoke_few_args(s_offsetGet, 1, init_null_variant);
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

  assert(m_type == KindOfArray);
  Variant *ret = nullptr;
  ArrayData *arr = m_data.parr;
  ArrayData *escalated = arr->lvalNew(ret, arr->getCount() > 1);
  if (escalated != arr) {
    set(escalated);
  }
  assert(ret);
  return *ret;
}

Variant &Variant::lvalInvalid() {
  throw_bad_type_exception("not array objects");
  return lvalBlackHole();
}

Variant &Variant::lvalBlackHole() {
  Variant &bh = get_global_variables()->__lvalProxy;
  bh.unset();
  return bh;
}

Variant Variant::o_get(CStrRef propName, bool error /* = true */,
                       CStrRef context /* = null_string */) const {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_get(propName, error, context);
  } else if (m_type == KindOfRef) {
    return m_data.pref->var()->o_get(propName, error, context);
  } else if (error) {
    raise_notice("Trying to get property of non-object");
  }
  return null_variant;
}

Variant Variant::o_set(CStrRef propName, CVarRef val,
                       CStrRef context /* = null_string */) {
  if (m_type == KindOfObject) {
  } else if (m_type == KindOfRef) {
    return m_data.pref->var()->o_set(propName, val, context);
  } else if (isObjectConvertable()) {
    setToDefaultObject();
  } else {
    // Raise a warning
    raise_warning("Attempt to assign property of non-object");
    return uninit_null();
  }
  return m_data.pobj->o_set(propName, val, context);
}

Variant Variant::o_setRef(CStrRef propName, CVarRef val,
                          CStrRef context /* = null_string */) {
  if (m_type == KindOfObject) {
  } else if (m_type == KindOfRef) {
    return m_data.pref->var()->o_setRef(propName, val, context);
  } else if (isObjectConvertable()) {
    setToDefaultObject();
  } else {
    // Raise a warning
    raise_warning("Attempt to assign property of non-object");
    return uninit_null();
  }
  return m_data.pobj->o_setRef(propName, val, context);
}

Variant Variant::o_invoke(CStrRef s, CArrRef params) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke(s, params);
  } else if (m_type == KindOfRef) {
    return m_data.pref->var()->o_invoke(s, params);
  } else {
    throw_call_non_object(s.c_str());
  }
}

Variant Variant::o_invoke_few_args(CStrRef s, int count,
                                   INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (m_type == KindOfObject) {
    return m_data.pobj->o_invoke_few_args(s, count,
                                          INVOKE_FEW_ARGS_PASS_ARGS);
  } else if (m_type == KindOfRef) {
    return m_data.pref->var()->o_invoke_few_args(s, count,
                                                 INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    throw_call_non_object(s.c_str());
  }
}

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
    if (escalated != self->m_data.parr) {
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
  case KindOfRef:
    self = self->m_data.pref->var();
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
  case KindOfObject: {
    ObjectData* obj = self->getObjectData();
    if (obj->isCollection()) {
      collectionOffsetSet(obj, key, v);
    } else {
      self->getArrayAccess()->o_invoke_few_args(s_offsetSet, 2, key, v);
    }
    break;
  }
  default:
    throw_bad_type_exception("not array objects");
    break;
  }
  return v;
}

CVarRef Variant::set(int64_t key, CVarRef v) {
  return SetImpl(this, key, v, false);
}

CVarRef Variant::set(double key, CVarRef v) {
  return SetImpl(this, key, v, false);
}

CVarRef Variant::set(CStrRef key, CVarRef v, bool isString /* = false */) {
  return SetImpl<CStrRef>(this, key, v, isString);
}

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
      if (escalated != m_data.parr) {
        set(escalated);
      }
    }
    break;
  case KindOfRef:
    m_data.pref->var()->append(v);
    break;
  case KindOfObject:
    {
      ObjectData* obj = m_data.pobj;
      if (LIKELY(obj->isCollection())) {
        collectionOffsetAppend(obj, v);
      } else {
        obj->o_invoke_few_args(s_offsetSet, 2, init_null_variant, v);
      }
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
    if (escalated != self->m_data.parr) {
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
  case KindOfRef:
    self = self->m_data.pref->var();
    goto retry;
  case KindOfStaticString:
  case KindOfString: {
    if (self->m_data.pstr->empty()) {
      goto create;
    }
    throw_bad_type_exception("binding assignment to stringoffset");
    break;
  }
  case KindOfObject: {
    if (self->m_data.pobj->isCollection()) {
      raise_error("An element of a collection cannot be taken by reference");
    }
    self->getArrayAccess()->o_invoke_few_args(s_offsetSet, 2, key, v);
    break;
  }
  default:
    throw_bad_type_exception("not array objects");
    break;
  }
  return v;
}

CVarRef Variant::setRef(int64_t key, CVarRef v) {
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
      if (escalated != m_data.parr) {
        set(escalated);
      }
    }
    break;
  case KindOfRef:
    m_data.pref->var()->appendRef(v);
    break;
  case KindOfObject:
    {
      ObjectData* obj = m_data.pobj;
      if (LIKELY(obj->isCollection())) {
        raise_error("Collection elements cannot be taken by reference");
      } else {
        obj->o_invoke_few_args(s_offsetSet, 2, uninit_null(), v);
      }
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
        if (escalated != arr) {
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

void Variant::removeImpl(int64_t key) {
  switch (getType()) {
  case KindOfUninit:
  case KindOfNull:
    break;
  case KindOfArray:
    {
      ArrayData *arr = getArrayData();
      if (arr) {
        ArrayData *escalated = arr->remove(key, (arr->getCount() > 1));
        if (escalated != arr) {
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
          const VarNR &k = key.toKey();
          if (k.isNull()) return;
          escalated = arr->remove(k, (arr->getCount() > 1));
        }
        if (escalated != arr) {
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
        if (escalated != arr) {
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

void Variant::setEvalScalar() {
  switch (m_type) {
  case KindOfString: {
    StringData *pstr = m_data.pstr;
    if (!pstr->isStatic()) {
      StringData *sd = StringData::GetStaticString(pstr);
      decRefStr(pstr);
      m_data.pstr = sd;
      assert(m_data.pstr->isStatic());
      m_type = KindOfStaticString;
    }
    break;
  }
  case KindOfArray: {
    ArrayData *parr = m_data.parr;
    if (!parr->isStatic()) {
      ArrayData *ad = ArrayData::GetScalarArray(parr);
      decRefArr(parr);
      m_data.parr = ad;
      assert(m_data.parr->isStatic());
    }
    break;
  }
  case KindOfRef:
    not_reached();
    break;
  case KindOfObject:
    not_reached(); // object shouldn't be in a scalar array
    break;
  default:
    break;
  }
}

void Variant::setToDefaultObject() {
  raise_warning(Strings::CREATING_DEFAULT_OBJECT);
  set(Object(SystemLib::AllocStdClassObject()));
}

///////////////////////////////////////////////////////////////////////////////
// output functions

void Variant::serialize(VariableSerializer *serializer,
                        bool isArrayKey /* = false */,
                        bool skipNestCheck /* = false */) const {
  if (m_type == KindOfRef) {
    // Ugly, but behavior is different for serialize
    if (serializer->getType() == VariableSerializer::Serialize ||
        serializer->getType() == VariableSerializer::APCSerialize ||
        serializer->getType() == VariableSerializer::DebuggerSerialize) {
      if (serializer->incNestedLevel(m_data.pref->var())) {
        serializer->writeOverflow(m_data.pref->var());
      } else {
        // Tell the inner variant to skip the nesting check for data inside
        m_data.pref->var()->serialize(serializer, isArrayKey, true);
      }
      serializer->decNestedLevel(m_data.pref->var());
    } else {
      m_data.pref->var()->serialize(serializer, isArrayKey);
    }
    return;
  }

  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:
    assert(!isArrayKey);
    serializer->writeNull();                break;
  case KindOfBoolean:
    assert(!isArrayKey);
    serializer->write(m_data.num != 0);     break;
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
    assert(!isArrayKey);
    m_data.parr->serialize(serializer, skipNestCheck);
    break;
  case KindOfObject:
    assert(!isArrayKey);
    m_data.pobj->serialize(serializer);     break;
  default:
    assert(false);
    break;
  }
}

static void unserializeProp(VariableUnserializer *uns,
                            ObjectData *obj, CStrRef key,
                            CStrRef context, CStrRef realKey,
                            int nProp) {
  // Do a two-step look up
  int flags = 0;
  Variant* t = obj->o_realProp(key, flags, context);
  if (!t) {
    // Dynamic property. If this is the first, and we're using HphpArray,
    // we need to pre-allocate space in the array to ensure the elements
    // dont move during unserialization.
    obj->initProperties(nProp);
    t = obj->o_realProp(realKey, ObjectData::RealPropCreate, context);
    if (!t) {
      // When accessing protected/private property from wrong context,
      // we could get NULL for o_realProp.
      throw Exception("Error in accessing property");
    }
  }
  t->unserialize(uns);
}

void Variant::unserialize(VariableUnserializer *uns,
                          Uns::Mode mode /* = Uns::ValueMode */) {

  char type, sep;
  type = uns->readChar();
  sep = uns->readChar();

  if (type != 'R') {
    uns->add(this, mode);
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
      int64_t id = uns->readInt();
      Variant *v = uns->getByVal(id);
      if (v == nullptr) {
        throw Exception("Id %ld out of range", id);
      }
      operator=(*v);
    }
    break;
  case 'R':
    {
      int64_t id = uns->readInt();
      Variant *v = uns->getByRef(id);
      if (v == nullptr) {
        throw Exception("Id %ld out of range", id);
      }
      assignRef(*v);
    }
    break;
  case 'b': { int64_t v = uns->readInt(); operator=((bool)v); } break;
  case 'i': { int64_t v = uns->readInt(); operator=(v);       } break;
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
  case 'O':
  case 'V':
  case 'K':
    {
      String clsName;
      clsName.unserialize(uns);

      sep = uns->readChar();
      if (sep != ':') {
        throw Exception("Expected ':' but got '%c'", sep);
      }
      int64_t size = uns->readInt();
      char sep = uns->readChar();
      if (sep != ':') {
        throw Exception("Expected ':' but got '%c'", sep);
      }
      sep = uns->readChar();
      if (sep != '{') {
        throw Exception("Expected '{' but got '%c'", sep);
      }

      Class* cls = Unit::loadClass(clsName.get());
      Object obj;
      if (RuntimeOption::UnserializationWhitelistCheck &&
          !uns->isWhitelistedClass(clsName)) {
        const char* err_msg =
          "The object being unserialized with class name '%s' "
          "is not in the given whitelist. "
          "See http://fburl.com/SafeSerializable for more detail";
        if (RuntimeOption::UnserializationWhitelistCheckWarningOnly) {
          raise_warning(err_msg, clsName.c_str());
        } else {
          raise_error(err_msg, clsName.c_str());
        }
      }
      if (cls) {
        obj = Instance::newInstance(cls);
        if (UNLIKELY(cls == c_Pair::s_cls && size != 2)) {
          throw Exception("Pair objects must have exactly 2 elements");
        }
      } else {
        obj = Instance::newInstance(
          SystemLib::s___PHP_Incomplete_ClassClass);
        obj->o_set(s_PHP_Incomplete_Class_Name, clsName);
      }
      operator=(obj);

      if (size > 0) {
        if (type == 'O') {
          // Collections are not allowed
          if (obj->isCollection()) {
            if (size > 0) {
              throw Exception("%s does not support the 'O' serialization "
                              "format", clsName.data());
            }
            // Be lax and tolerate the 'O' serialization format for collection
            // classes if there are 0 properties.
            raise_warning("%s does not support the 'O' serialization "
                          "format", clsName.data());
          }
          /*
            Count backwards so that i is the number of properties
            remaining (to be used as an estimate for the total number
            of dynamic properties when we see the first dynamic prop).
            see getVariantPtr
          */
          for (int64_t i = size; i--; ) {
            String key = uns->unserializeKey().toString();
            int ksize = key.size();
            const char *kdata = key.data();
            int subLen = 0;
            if (kdata[0] == '\0') {
              if (UNLIKELY(!ksize)) {
                throw EmptyObjectPropertyException();
              }
              // private or protected
              subLen = strlen(kdata + 1) + 2;
              if (UNLIKELY(subLen >= ksize)) {
                if (subLen == ksize) {
                  throw EmptyObjectPropertyException();
                } else {
                  throw Exception("Mangled private object property");
                }
              }
              String k(kdata + subLen, ksize - subLen, AttachLiteral);
              if (kdata[1] == '*') {
                unserializeProp(uns, obj.get(), k, clsName, key, i + 1);
              } else {
                unserializeProp(uns, obj.get(), k,
                                String(kdata + 1, subLen - 2, AttachLiteral),
                                key, i + 1);
              }
            } else {
              unserializeProp(uns, obj.get(), key, empty_string, key, i + 1);
            }
          }
        } else {
          assert(type == 'V' || type == 'K');
          if (!obj->isCollection()) {
            throw Exception("%s is not a collection class", clsName.data());
          }
          collectionUnserialize(obj.get(), uns, size, type);
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
        obj = create_object_only(clsName);
        if (!obj->instanceof(SystemLib::s_SerializableClass)) {
          raise_error("%s didn't implement Serializable", clsName.data());
        }
        obj->o_invoke_few_args(s_unserialize, 1, serialized);
        obj.get()->clearNoDestruct();
      } catch (ClassNotFoundException &e) {
        if (!uns->allowUnknownSerializableClass()) {
          throw;
        }
        obj = create_object_only(s_PHP_Incomplete_Class);
        obj->o_set(s_PHP_Incomplete_Class_Name, clsName);
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
  if (m_type == KindOfRef) {
    return m_data.pref->var()->share(save);
  }

  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:    return false; // same as non-existent
  case KindOfBoolean: return (m_data.num != 0);
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
      return unserialize_from_string(m_data.pobj->o_get(s_s));
    }
    break;
  default:
    assert(false);
    break;
  }

  return false; // same as non-existent
}

SharedVariant *Variant::getSharedVariant() const {
  if (m_type == KindOfRef) {
    return m_data.pref->var()->getSharedVariant();
  }
  if (m_type == KindOfString) {
    return m_data.pstr->getSharedVariant();
  }
  if (m_type == KindOfArray) {
    return m_data.parr->getSharedVariant();
  }
  return nullptr;
}

void Variant::dump() const {
  VariableSerializer vs(VariableSerializer::VarDump);
  String ret(vs.serialize(*this, true));
  printf("Variant: %s", ret.c_str());
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

///////////////////////////////////////////////////////////////////////////////
}
