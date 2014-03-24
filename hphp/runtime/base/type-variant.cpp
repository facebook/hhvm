/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"

#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/repo.h"
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

static void unserializeProp(VariableUnserializer *uns,
                            ObjectData *obj, const String& key,
                            Class* ctx, const String& realKey,
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

Variant::Variant(const Array& v) {
  m_type = KindOfArray;
  ArrayData *a = v.get();
  if (a) {
    m_data.parr = a;
    a->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(const Object& v) {
  m_type = KindOfObject;
  ObjectData *o = v.get();
  if (o) {
    m_data.pobj = o;
    o->incRefCount();
  } else {
    m_type = KindOfNull;
  }
}

Variant::Variant(const Resource& v) {
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

// the version of the high frequency function that is not inlined
Variant::Variant(const Variant& v) {
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

Variant &Variant::assign(const Variant& v) {
  AssignValHelper(this, &v);
  return *this;
}

Variant &Variant::assignRef(const Variant& v) {
  assignRefHelper(v);
  return *this;
}

Variant &Variant::setWithRef(const Variant& v) {
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
      auto const d = m_data.num;                                        \
      auto const t = m_type;                                            \
      setOp;                                                            \
      tvDecRefHelper(t, d);                                             \
    }                                                                   \
    returnStmt;                                                         \
  }
#define IMPLEMENT_VOID_SET(name, setOp) \
  void IMPLEMENT_SET_IMPL(name, , , setOp, return)
#define IMPLEMENT_SET(argType, setOp) \
  const Variant& IMPLEMENT_SET_IMPL(set, argType, v, setOp, return *this)

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
  const Variant& Variant::set(ptr *v) {                                        \
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
    return *this;                                                       \
  }

IMPLEMENT_PTR_SET(StringData, pstr,
                           v->isStatic() ? KindOfStaticString : KindOfString);
IMPLEMENT_PTR_SET(ArrayData, parr, KindOfArray)
IMPLEMENT_PTR_SET(ObjectData, pobj, KindOfObject)
IMPLEMENT_PTR_SET(ResourceData, pres, KindOfResource)

#undef IMPLEMENT_PTR_SET

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
  case KindOfArray:   return ObjectData::FromArray(m_data.parr);
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
    {
      auto val = m_data.dbl > std::numeric_limits<uint64_t>::max()
        ? 0u
        : uint64_t(m_data.dbl);
      return VarNR(val);
    }
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
class LvalHelper<const Variant&> {
public:
  typedef VarNR KeyType;
  static bool CheckKey(const KeyType &k) { return !k.isNull(); };
  static const bool CheckParams = true;
};

Variant &Variant::lvalInvalid() {
  throw_bad_type_exception("not array objects");
  return lvalBlackHole();
}

Variant &Variant::lvalBlackHole() {
  Variant &bh = get_env_constants()->__lvalProxy;
  bh.unset();
  return bh;
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
                            Class* ctx, const String& realKey,
                            int nProp) {
  // Do a two-step look up
  bool visible, accessible, unset;
  auto t = &tvAsVariant(obj->getProp(ctx, key.get(),
                                     visible, accessible, unset));
  assert(!unset);
  if (!t || !accessible) {
    // Dynamic property. If this is the first, and we're using HphpArray,
    // we need to pre-allocate space in the array to ensure the elements
    // dont move during unserialization.
    //
    // TODO(#2881866): this assumption means we can't do reallocations
    // when promoting kPackedKind -> kMixedKind.
    t = &obj->reserveProperties(nProp).lvalAt(realKey, AccessFlags::Key);
  }

  t->unserialize(uns);

  if (!RuntimeOption::EvalCheckRepoAuthDeserialize) return;
  if (!RuntimeOption::RepoAuthoritative) return;
  if (!Repo::get().global().HardPrivatePropInference) return;

  /*
   * We assume for performance reasons in repo authoriative mode that
   * we can see all the sets to private properties in a class.
   *
   * It's a hole in this if we don't check unserialization doesn't
   * violate what we've seen, which we handle by throwing if the repo
   * was built with this option.
   */
  auto const cls  = obj->getVMClass();
  auto const slot = cls->lookupDeclProp(key.get());
  if (UNLIKELY(slot == kInvalidSlot)) return;
  auto const repoTy = obj->getVMClass()->declPropRepoAuthType(slot);
  if (LIKELY(tvMatchesRepoAuthType(*t->asTypedValue(), repoTy))) {
    return;
  }

  auto msg = folly::format(
    "Property {} for class {} was deserialized with type ({}) that "
    "didn't match what we inferred in static analysis",
    key.data(),
    obj->getVMClass()->name()->data(),
    tname(t->asTypedValue()->m_type)
  ).str();
  throw Exception(msg);
}

/*
 * For namespaced collections, returns an "alternate" name, which is a
 * collection name with or without the namespace qualifier, depending on
 * what's passed.
 * If no alternate name is found, returns nullptr.
 */
static const StringData* getAlternateCollectionName(const StringData* clsName) {
  typedef hphp_hash_map<const StringData*, const StringData*,
                        string_data_hash, string_data_isame> ClsNameMap;

  auto getAltMap = [] {
    typedef std::pair<StaticString, StaticString> SStringPair;

    static ClsNameMap m;

    static std::vector<SStringPair> mappings {
      std::make_pair(StaticString("Vector"), StaticString("HH\\Vector")),
      std::make_pair(StaticString("Map"), StaticString("HH\\Map")),
      std::make_pair(StaticString("Set"), StaticString("HH\\Set")),
      std::make_pair(StaticString("Pair"), StaticString("HH\\Pair"))
    };

    for (const auto& p : mappings) {
      m[p.first.get()] = p.second.get();
      m[p.second.get()] = p.first.get();
    }

    // As part of StableMap merging into Map, StableMap is an alias for HH\\Map,
    // but Map is the sole alias for HH\\Map
    m[StaticString("StableMap").get()] = StaticString("HH\\Map").get();
    return &m;
  };

  static const ClsNameMap* altMap = getAltMap();

  auto it = altMap->find(clsName);
  return it != altMap->end() ? it->second : nullptr;
}

static Class* tryAlternateCollectionClass(const StringData* clsName) {
  auto altName = getAlternateCollectionName(clsName);
  return altName ? Unit::getClass(altName, /* autoload */ false) : nullptr;
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
    if (sep != ';') throw Exception("Expected ';' but got '%c'", sep);
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

      const bool allowObjectFormatForCollections = true;

      Class* cls;
      // If we are potentially dealing with a collection, we need to try to
      // load the collection class under an alternate name so that we can
      // deserialize data that was serialized before the migration of
      // collections to the HH namespace.

      if (type != 'O') {
        // Collections are CPP builtins; don't attempt to autoload
        cls = Unit::getClass(clsName.get(), /* autoload */ false);
        if (!cls) {
          cls = tryAlternateCollectionClass(clsName.get());
        }
      } else if (allowObjectFormatForCollections) {
        // In order to support the legacy {O|V}:{Set|Vector|Map}
        // serialization, we defer autoloading until we know that there's
        // no alternate (builtin) collection class.
        cls = Unit::getClass(clsName.get(), /* autoload */ false);
        if (!cls) {
          cls = tryAlternateCollectionClass(clsName.get());
        }
        if (!cls) {
          cls = Unit::loadClass(clsName.get()); // with autoloading
        }
      } else {
        cls = Unit::loadClass(clsName.get()); // with autoloading
      }

      Object obj;
      if (RuntimeOption::UnserializationWhitelistCheck &&
          (type == 'O') &&
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
        // Only unserialize CPP extension types which can actually
        // support it. Otherwise, we risk creating a CPP object
        // without having it initialized completely.
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
              Class* ctx = (Class*)-1;
              if (kdata[1] != '*') {
                ctx = Unit::lookupClass(
                  String(kdata + 1, subLen - 2, CopyString).get());
              }
              unserializeProp(uns, obj.get(), k, ctx, key, i + 1);
            } else {
              unserializeProp(uns, obj.get(), key, nullptr, key, i + 1);
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
      } catch (ClassNotFoundException &e) {
        if (!uns->allowUnknownSerializableClass()) {
          throw;
        }
        obj = create_object_only(s_PHP_Incomplete_Class);
        obj->o_set(s_PHP_Incomplete_Class_Name, clsName);
        obj->o_set("serialized", serialized);
      }

      if (!obj->instanceof(SystemLib::s_SerializableClass)) {
        raise_warning("Class %s has no unserializer",
                      obj->o_getClassName().data());
      } else {
        obj->o_invoke_few_args(s_unserialize, 1, serialized);
        obj.get()->clearNoDestruct();
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
