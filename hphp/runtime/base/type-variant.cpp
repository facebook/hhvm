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
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/ext/ext_variable.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/util/util.h"
#include "hphp/util/abi-cxx.h"
#include "hphp/util/logger.h"

namespace HPHP {

const Variant null_variant;                         // uninitialized variant
const Variant init_null_variant((Variant::NullInit())); // php null
const VarNR null_varNR;
const VarNR true_varNR(true);
const VarNR false_varNR(false);
const VarNR INF_varNR(std::numeric_limits<double>::infinity());
const VarNR NEGINF_varNR(std::numeric_limits<double>::infinity());
const VarNR NAN_varNR(std::numeric_limits<double>::quiet_NaN());

static void unserializeProp(VariableUnserializer *uns,
                            ObjectData *obj, const String& key,
                            const String& context, const String& realKey,
                            int nProp) NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
// static strings

const StaticString
  s_offsetGet("offsetGet"),
  s_offsetSet("offsetSet"),
  s_offsetUnset("offsetUnset"),
  s_s("s"),
  s_scalar("scalar"),
  s_array("Array"),
  s_1("1"),
  s_unserialize("unserialize"),
  s_PHP_Incomplete_Class("__PHP_Incomplete_Class"),
  s_PHP_Incomplete_Class_Name("__PHP_Incomplete_Class_Name"),
  s_PHP_Unserializable_Class_Name("__PHP_Unserializable_Class_Name");

///////////////////////////////////////////////////////////////////////////////
// local helpers

static int64_t ToKey(int64_t i) { return i; }
static int64_t ToKey(double d) {
  return d > std::numeric_limits<uint64_t>::max() ? 0u : uint64_t(d);
}
static VarNR ToKey(const String& s) { return s.toKey(); }
static VarNR ToKey(CVarRef v) { return v.toKey(); }

///////////////////////////////////////////////////////////////////////////////
// private implementations

Variant::Variant(litstr  v) {
  m_type = KindOfString;
  m_data.pstr = StringData::Make(v);
  m_data.pstr->incRefCount();
}

Variant::Variant(const String& v) {
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
  StringData *s = StringData::Make(v.c_str(), v.size(), CopyString);
  assert(s);
  m_data.pstr = s;
  s->incRefCount();
}

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

Variant::Variant(CResRef v) {
  m_type = KindOfResource;
  ResourceData* o = v.get();
  if (o) {
    m_data.pres = o;
    o->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(StringData *v) {
  if (v) {
    m_data.pstr = v;
    if (v->isStatic()) {
      m_type = KindOfStaticString;
    } else {
      m_type = KindOfString;
      v->incRefCount();
    }
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(const StringData *v) {
  if (v) {
    assert(v->isStatic());
    m_data.pstr = const_cast<StringData*>(v);
    m_type = KindOfStaticString;
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

Variant::Variant(ResourceData *v) {
  m_type = KindOfResource;
  if (v) {
    m_data.pres = v;
    v->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(RefData *r) {
  m_type = KindOfRef;
  if (r) {
    m_data.pref = r;
    r->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(RefData *r, NoInc) {
  m_type = KindOfRef;
  if (r) {
    m_data.pref = r;
  } else {
    m_type = KindOfNull;
  }
}

// the version of the high frequency function that is not inlined
Variant::Variant(CVarRef v) {
  constructValHelper(v);
}

Variant::Variant(CVarStrongBind v) {
  constructRefHelper(variant(v));
}

Variant::Variant(CVarWithRefBind v) {
  constructWithRefHelper(variant(v));
}

/*
 * The destruct functions below all arbitrarily take RefData* as an
 * example of a refcounted object, then just cast to the proper type.
 * This is safe because we have compile time assertions that guarantee that
 * the _count field will always be exactly FAST_REFCOUNT_OFFSET bytes from
 * the beginning of the object for the StringData, ArrayData, ObjectData,
 * ResourceData, and RefData classes.
 */

static_assert(TYPE_TO_DESTR_IDX(KindOfString) == 1, "String destruct index");
static_assert(TYPE_TO_DESTR_IDX(KindOfArray)  == 2,  "Array destruct index");
static_assert(TYPE_TO_DESTR_IDX(KindOfObject) == 3, "Object destruct index");
static_assert(TYPE_TO_DESTR_IDX(KindOfResource) == 4,
              "Resource destruct index");
static_assert(TYPE_TO_DESTR_IDX(KindOfRef)    == 5,    "Ref destruct index");

static_assert(kDestrTableSize == 6,
              "size of g_destructors[] must be kDestrTableSize");

const RawDestructor g_destructors[] = {
  nullptr,
  (RawDestructor)getMethodPtr(&StringData::release),
  (RawDestructor)getMethodPtr(&ArrayData::release),
  (RawDestructor)getMethodPtr(&ObjectData::release),
  (RawDestructor)getMethodPtr(&ResourceData::release),
  (RawDestructor)getMethodPtr(&RefData::release),
};

Variant::~Variant() {
  if (IS_REFCOUNTED_TYPE(m_type)) {
    tvDecRefHelper(m_type, uint64_t(m_data.pref));
  }
}

void tvDecRefHelper(DataType type, uint64_t datum) {
  assert(type == KindOfString || type == KindOfArray ||
         type == KindOfObject || type == KindOfResource ||
         type == KindOfRef);
  DECREF_AND_RELEASE_MAYBE_STATIC(
    ((RefData*)datum),
    g_destructors[typeToDestrIndex(type)]((void*)datum));
}

Variant &Variant::assign(CVarRef v) {
  AssignValHelper(this, &v);
  return *this;
}

Variant &Variant::assignRef(CVarRef v) {
  assignRefHelper(v);
  return *this;
}

Variant &Variant::setWithRef(CVarRef v) {
  setWithRefHelper(v, IS_REFCOUNTED_TYPE(m_type));
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
IMPLEMENT_SET(bool, m_type = KindOfBoolean; m_data.num = v)
IMPLEMENT_SET(int, m_type = KindOfInt64; m_data.num = v)
IMPLEMENT_SET(int64_t, m_type = KindOfInt64; m_data.num = v)
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

IMPLEMENT_PTR_SET(StringData, pstr,
                           v->isStatic() ? KindOfStaticString : KindOfString);
IMPLEMENT_PTR_SET(ArrayData, parr, KindOfArray)
IMPLEMENT_PTR_SET(ObjectData, pobj, KindOfObject)
IMPLEMENT_PTR_SET(ResourceData, pres, KindOfResource)

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

int Variant::getRefCount() const {
  switch (m_type) {
  case KindOfString:  return m_data.pstr->getCount();
  case KindOfArray:   return m_data.parr->getCount();
  case KindOfObject:  return m_data.pobj->getCount();
  case KindOfResource: return m_data.pres->getCount();
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
  case KindOfResource:
    return false;
  default:
    break;
  }
  return true;
}

bool Variant::isResource() const {
  auto const cell = asCell();
  return (cell->m_type == KindOfResource);
}

bool Variant::instanceof(const String& s) const {
  if (m_type == KindOfObject) {
    assert(m_data.pobj);
    return m_data.pobj->o_instanceof(s);
  }
  if (m_type == KindOfRef) {
    return m_data.pref->var()->instanceof(s);
  }
  return false;
}

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
    ArrayData *newarr = arr->prepend(v, (arr->hasMultipleRefs()));
    if (newarr != arr) set(newarr);
  } else {
    throw_bad_type_exception("expecting an array");
  }
}

inline DataType Variant::convertToNumeric(int64_t *lval, double *dval) const {
  StringData *s = getStringData();
  assert(s);
  return s->isNumericWithVal(*lval, *dval, 1);
}

///////////////////////////////////////////////////////////////////////////////
// iterator functions

ArrayIter Variant::begin(const String& context /* = null_string */) const {
  if (is(KindOfArray)) {
    return ArrayIter(getArrayData());
  }
  if (is(KindOfObject)) {
    return getObjectData()->begin(context);
  }
  raise_warning("Invalid argument supplied for foreach()");
  return ArrayIter();
}

MutableArrayIter Variant::begin(Variant *key, Variant &val,
                                const String& context /* = null_string */) {
  if (is(KindOfObject)) {
    return getObjectData()->begin(key, val, context);
  }
  return MutableArrayIter(this, key, val);
}

void Variant::escalate() {
  auto const cell = asCell();
  if (cell->m_type == KindOfArray) {
    ArrayData *arr = cell->m_data.parr;
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
  case KindOfResource: return m_data.pres->o_toBoolean();
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
  case KindOfResource: return m_data.pres->o_toInt64();
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
  case KindOfResource: return m_data.pres->o_toDouble();
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
  case KindOfArray:   raise_notice("Array to string conversion");
                      return s_array;
  case KindOfObject:  return m_data.pobj->invokeToString();
  case KindOfResource: return m_data.pres->o_toString();
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
  case KindOfResource: return m_data.pres->o_toArray();
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
  case KindOfResource:
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

Resource Variant::toResourceHelper() const {
  if (m_type == KindOfRef) return m_data.pref->var()->toResource();

  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfStaticString:
  case KindOfString:
  case KindOfArray:
  case KindOfObject:
    break;
  case KindOfResource:  return m_data.pres;
  default:
    assert(false);
    break;
  }
  return Resource(NEWOBJ(DummyResource));
}

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
    break;
  case KindOfResource:
    return VarNR(toInt64());
  case KindOfRef:
    return m_data.pref->var()->toKey();
  default:
    break;
  }
  throw_bad_type_exception("Invalid type used as key");
  return null_varNR;
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
    collectionUnset(obj, cvarToCell(&key));
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
      if (flags & AccessFlags::Error) {                                 \
        raise_bad_offset_notice();                                      \
      }                                                                 \
      break;                                                            \
  }                                                                     \
  return null_variant;

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
    if (flags & AccessFlags::Error) {
      raise_bad_offset_notice();
    }
    break;
  }
  return null_variant;
}

Variant Variant::rvalAt(const String& offset, ACCESSPARAMS_IMPL) const {
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
      throw_bad_type_exception("Invalid type used as key");
      break;
    case KindOfResource:
      return m_data.parr->get(offset.toInt64(), flags & AccessFlags::Error);
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
    if (flags & AccessFlags::Error) {
      raise_bad_offset_notice();
    }
    break;
  }
  return null_variant;
}

template CVarRef
Variant::rvalRefHelper(int64_t offset, CVarRef tmp, ACCESSPARAMS_IMPL) const;

CVarRef Variant::rvalRef(const String& offset, CVarRef tmp,
                         ACCESSPARAMS_IMPL) const {
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
      throw_bad_type_exception("Invalid type used as key");
      break;
    case KindOfResource:
      return m_data.parr->get(offset.toInt64(), flags & AccessFlags::Error);
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
CVarRef Variant::rvalAtRefHelper<const String&>(const String& offset,
                                          ACCESSPARAMS_IMPL) const;
template
CVarRef Variant::rvalAtRefHelper<CVarRef>(CVarRef offset,
                                          ACCESSPARAMS_IMPL) const;

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
class LvalHelper<const String&> {
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
  assert(!(flags & AccessFlags::CheckExist));
  if (self->m_type == KindOfArray) {
    ArrayData *arr = self->m_data.parr;
    ArrayData *escalated;
    Variant *ret = nullptr;
    if (LvalHelper<T>::CheckParams && flags & AccessFlags::Key) {
      escalated = arr->lval(key, ret, arr->hasMultipleRefs());
    } else {
      typename LvalHelper<T>::KeyType k(ToKey(key));
      if (LvalHelper<T>::CheckKey(k)) {
        escalated = arr->lval(k, ret, arr->hasMultipleRefs());
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
    Variant& retv = get_env_constants()->__lvalProxy;
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
Variant &Variant::lvalAt(const String& key, ACCESSPARAMS_IMPL) {
  return lvalAtImpl<const String&>(key, flags);
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
Variant &Variant::lvalRef(const String& key, Variant& tmp, ACCESSPARAMS_IMPL) {
  return Variant::LvalAtImpl0<const String&>(this, key, &tmp, false, flags);
}
Variant &Variant::lvalRef(CVarRef k, Variant& tmp, ACCESSPARAMS_IMPL) {
  return Variant::LvalAtImpl0<CVarRef>(this, k, &tmp, false, flags);
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
  ArrayData *escalated = arr->lvalNew(ret, arr->hasMultipleRefs());
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
  Variant &bh = get_env_constants()->__lvalProxy;
  bh.unset();
  return bh;
}

template <typename T>
ALWAYS_INLINE
CVarRef Variant::SetImpl(Variant *self, T key, CVarRef v, bool isKey) {
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
    auto const s = self->m_data.pstr;
    if (s->empty()) goto create;

    auto const es = [&]() -> StringData* {
      auto const offset = HPHP::toInt64(key);
      auto const r = s->slice();
      if (offset < 0) return s;
      if (r.len == 0) throw OffsetOutOfRangeException();

      String str = v.toString();
      auto const ch = str.empty() ? 0 : str.data()[0];
      if (offset < r.len && !s->hasMultipleRefs()) {
        return s->modifyChar(offset, ch);
      }
      if (offset > RuntimeOption::StringOffsetLimit) {
        throw OffsetOutOfRangeException();
      }
      uint32_t newlen = offset + 1;
      auto const sd = StringData::Make(newlen);
      auto const mslice = sd->bufferSlice();
      memcpy(mslice.ptr, r.ptr, r.len);
      memset(mslice.ptr + r.len, ' ', newlen - r.len);
      mslice.ptr[offset] = ch;
      sd->setSize(newlen);
      return sd;
    }();
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

CVarRef Variant::set(const String& key, CVarRef v,
                     bool isString /* = false */) {
  return SetImpl<const String&>(this, key, v, isString);
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
        collectionAppend(obj, cvarToCell(&v));
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
ALWAYS_INLINE
CVarRef Variant::SetRefImpl(Variant *self, T key, CVarRef v, bool isKey) {
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

CVarRef Variant::setRef(const String& key, CVarRef v,
                        bool isString /* = false */) {
  return SetRefImpl<const String&>(this, key, v, isString);
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

void Variant::removeImpl(int64_t key) {
  switch (getType()) {
  case KindOfUninit:
  case KindOfNull:
    break;
  case KindOfArray:
    {
      ArrayData *arr = getArrayData();
      if (arr) {
        ArrayData *escalated = arr->remove(key, (arr->hasMultipleRefs()));
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
          escalated = arr->remove(key, (arr->hasMultipleRefs()));
        } else {
          const VarNR &k = key.toKey();
          if (k.isNull()) return;
          escalated = arr->remove(k, (arr->hasMultipleRefs()));
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

void Variant::removeImpl(const String& key, bool isString /* false */) {
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
          escalated = arr->remove(key, (arr->hasMultipleRefs()));
        } else {
          escalated = arr->remove(key.toKey(), (arr->hasMultipleRefs()));
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
      StringData *sd = makeStaticString(pstr);
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
  case KindOfResource:
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
    if (serializer->getType() == VariableSerializer::Type::Serialize ||
        serializer->getType() == VariableSerializer::Type::APCSerialize ||
        serializer->getType() == VariableSerializer::Type::DebuggerSerialize) {
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
  case KindOfResource:
    assert(!isArrayKey);
    m_data.pres->serialize(serializer);     break;
  default:
    assert(false);
    break;
  }
}

static void unserializeProp(VariableUnserializer *uns,
                            ObjectData *obj, const String& key,
                            const String& context, const String& realKey,
                            int nProp) {
  // Do a two-step look up
  int flags = 0;
  Variant* t = obj->o_realProp(key, flags, context);
  if (!t) {
    // Dynamic property. If this is the first, and we're using HphpArray,
    // we need to pre-allocate space in the array to ensure the elements
    // dont move during unserialization.
    //
    // TODO(#2881866): this assumption means we can't do reallocations
    // when promoting kPackedKind -> kMixedKind.
    obj->reserveProperties(nProp);
    t = obj->o_realProp(realKey, ObjectData::RealPropCreate, context);
    if (!t) {
      // When accessing protected/private property from wrong context,
      // we could get NULL for o_realProp.
      throw Exception("Error in accessing property");
    }
  }
  t->unserialize(uns);
}

/*
 * For namespaced collections, returns an "alternate" name, which is a
 * collection name with or without the namespace qualifier, depending on
 * what's passed.
 * If no alternate name is found, returns nullptr.
 */
static const StringData* getAlternateName(const StringData* clsName) {
  typedef hphp_hash_map<const StringData*, const StringData*,
                        string_data_hash, string_data_isame> ClsNameMap;

  auto getAltMap = [] {
    typedef std::pair<StaticString, StaticString> SStringPair;

    static ClsNameMap m;

    static std::vector<SStringPair> mappings {
      std::make_pair(StaticString("Vector"), StaticString("HH\\Vector")),
      std::make_pair(StaticString("Map"), StaticString("HH\\Map")),
      std::make_pair(StaticString("StableMap"), StaticString("HH\\StableMap")),
      std::make_pair(StaticString("Set"), StaticString("HH\\Set")),
      std::make_pair(StaticString("Pair"), StaticString("HH\\Pair"))
    };

    for (const auto& p : mappings) {
      m[p.first.get()] = p.second.get();
      m[p.second.get()] = p.first.get();
    }

    return &m;
  };

  static const ClsNameMap* altMap = getAltMap();

  auto it = altMap->find(clsName);
  return it != altMap->end() ? it->second : nullptr;
}

void Variant::unserialize(VariableUnserializer *uns,
                          Uns::Mode mode /* = Uns::Mode::Value */) {

  // NOTE: If you make changes to how serialization and unserialization work,
  // make sure to update the reserialize() method in "runtime/ext/ext_apc.cpp"
  // and to update test_apc_reserialize() in "test/ext/test_ext_apc.cpp".

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
        throw Exception("Id %" PRId64 " out of range", id);
      }
      operator=(*v);
    }
    break;
  case 'R':
    {
      int64_t id = uns->readInt();
      Variant *v = uns->getByRef(id);
      if (v == nullptr) {
        throw Exception("Id %" PRId64 " out of range", id);
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
    if (uns->getType() == VariableUnserializer::Type::APCSerialize) {
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
  case 'L':
    {
      int64_t id = uns->readInt();
      sep = uns->readChar();
      if (sep != ':') {
        throw Exception("Expected ':' but got '%c'", sep);
      }
      String rsrcName;
      rsrcName.unserialize(uns);
      sep = uns->readChar();
      if (sep != '{') {
        throw Exception("Expected '{' but got '%c'", sep);
      }
      sep = uns->readChar();
      if (sep != '}') {
        throw Exception("Expected '}' but got '%c'", sep);
      }
      DummyResource* rsrc = NEWOBJ(DummyResource);
      rsrc->o_setResourceId(id);
      rsrc->m_class_name = rsrcName;
      operator=(rsrc);
      return; // resource has '}' terminating
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

      // If we can't load the class, try an alternate name.
      // This is so we can tolerate stale data while the migration of
      // collections takes place.
      const StringData* altName;
      if (!cls) {
        altName = getAlternateName(clsName.get());
        if (altName) cls = Unit::loadClass(altName);
      }

      Object obj;
      if (RuntimeOption::UnserializationWhitelistCheck &&
          !uns->isWhitelistedClass(clsName)) {

        if (!altName) altName = getAlternateName(clsName.get());

        if (!altName || !uns->isWhitelistedClass(String(altName))) {
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
      }
      if (cls) {
        // Only unserialize CPP extension types which can actually
        // support it. Otherwise, we risk creating a CPP object
        // without having it intialized completely.
        if (cls->instanceCtor() && !cls->isCppSerializable()) {
          obj = ObjectData::newInstance(
            SystemLib::s___PHP_Unserializable_ClassClass);
          obj->o_set(s_PHP_Unserializable_Class_Name, clsName);
        } else {
          obj = ObjectData::newInstance(cls);
          if (UNLIKELY(cls == c_Pair::classof() && size != 2)) {
            throw Exception("Pair objects must have exactly 2 elements");
          }
        }
      } else {
        obj = ObjectData::newInstance(
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
              String k(kdata + subLen, ksize - subLen, CopyString);
              if (kdata[1] == '*') {
                unserializeProp(uns, obj.get(), k, clsName, key, i + 1);
              } else {
                unserializeProp(uns, obj.get(), k,
                                String(kdata + 1, subLen - 2, CopyString),
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

      obj->invokeWakeup();
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

void Variant::dump() const {
  VariableSerializer vs(VariableSerializer::Type::VarDump);
  String ret(vs.serialize(*this, true));
  printf("Variant: %s", ret.c_str());
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
