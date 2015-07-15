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
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"

#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
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
                                       Variant::StaticStrInit{});

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
  s_1("1"),
  s_unserialize("unserialize"),
  s_PHP_Incomplete_Class("__PHP_Incomplete_Class"),
  s_PHP_Incomplete_Class_Name("__PHP_Incomplete_Class_Name");

///////////////////////////////////////////////////////////////////////////////

Variant::Variant(StringData *v) noexcept {
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

RawDestructor g_destructors[] = {
  nullptr,
  (RawDestructor)getMethodPtr(&StringData::release),
  (RawDestructor)getMethodPtr(&ArrayData::release),
  (RawDestructor)getMethodPtr(&ObjectData::release), // may replace at runtime
  (RawDestructor)getMethodPtr(&ResourceData::release),
  (RawDestructor)getMethodPtr(&RefData::release),
};

void tweak_variant_dtors() {
  if (RuntimeOption::EnableObjDestructCall) return;
  g_destructors[TYPE_TO_DESTR_IDX(KindOfObject)] =
    (RawDestructor)getMethodPtr(&ObjectData::releaseNoObjDestructCheck);
}

Variant::~Variant() noexcept {
  tvRefcountedDecRef(asTypedValue());
  if (debug) {
    memset(this, kTVTrashFill2, sizeof(*this));
  }
}

void tvDecRefHelper(DataType type, uint64_t datum) noexcept {
  assert(type == KindOfString || type == KindOfArray ||
         type == KindOfObject || type == KindOfResource ||
         type == KindOfRef);
  if (((ArrayData*)datum)->decReleaseCheck()) {
    g_destructors[typeToDestrIndex(type)]((void*)datum);
  }
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
  setWithRefHelper(v, IS_REFCOUNTED_TYPE(m_type));
  return *this;
}

#define IMPLEMENT_SET_IMPL(name, argType, argName, setOp) \
  void Variant::name(argType argName) noexcept {          \
    if (isPrimitive()) {                                  \
      setOp;                                              \
    } else if (m_type == KindOfRef) {                     \
      m_data.pref->var()->name(argName);                  \
    } else {                                              \
      auto const d = m_data.num;                          \
      auto const t = m_type;                              \
      setOp;                                              \
      tvDecRefHelper(t, d);                               \
    }                                                     \
  }
#define IMPLEMENT_VOID_SET(name, setOp) \
  IMPLEMENT_SET_IMPL(name, , , setOp)
#define IMPLEMENT_SET(argType, setOp) \
  IMPLEMENT_SET_IMPL(set, argType, v, setOp)

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
                           v->isStatic() ? KindOfStaticString : KindOfString);
IMPLEMENT_PTR_SET(ArrayData, parr, KindOfArray)
IMPLEMENT_PTR_SET(ObjectData, pobj, KindOfObject)
IMPLEMENT_PTR_SET(ResourceData, pres, KindOfResource)

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
                v->isStatic() ? KindOfStaticString : KindOfString)
IMPLEMENT_STEAL(ArrayData, parr, KindOfArray)
IMPLEMENT_STEAL(ObjectData, pobj, KindOfObject)
IMPLEMENT_STEAL(ResourceData, pres, KindOfResource)

#undef IMPLEMENT_STEAL

int Variant::getRefCount() const noexcept {
  switch (m_type) {
    DT_UNCOUNTED_CASE:
      return 1;
    case KindOfString:    return m_data.pstr->getCount();
    case KindOfArray:     return m_data.parr->getCount();
    case KindOfObject:    return m_data.pobj->getCount();
    case KindOfResource:  return m_data.pres->getCount();
    case KindOfRef:       return m_data.pref->var()->getRefCount();
    case KindOfClass:     break;
  }
  not_reached();
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

    case KindOfStaticString:
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
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
      return false;

    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfStaticString:
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
    case KindOfStaticString:
    case KindOfString:        return m_data.pstr->toBoolean();
    case KindOfArray:         return !m_data.parr->empty();
    case KindOfObject:        return m_data.pobj->toBoolean();
    case KindOfResource:      return m_data.pres->o_toBoolean();
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
    case KindOfStaticString:
    case KindOfString:        return m_data.pstr->toInt64(base);
    case KindOfArray:         return m_data.parr->empty() ? 0 : 1;
    case KindOfObject:        return m_data.pobj->toInt64();
    case KindOfResource:      return m_data.pres->o_toInt64();
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
    case KindOfStaticString:
    case KindOfString:        return m_data.pstr->toDouble();
    case KindOfArray:         return (double)toInt64();
    case KindOfObject:        return m_data.pobj->toDouble();
    case KindOfResource:      return m_data.pres->o_toDouble();
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

    case KindOfStaticString:
    case KindOfString:
      assert(false); // Should be done in caller
      return m_data.pstr;

    case KindOfArray:
      raise_notice("Array to string conversion");
      return array_string;

    case KindOfObject:
      return m_data.pobj->invokeToString();

    case KindOfResource:
      return m_data.pres->o_toString();

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
    case KindOfStaticString:
    case KindOfString:        return Array::Create(m_data.pstr);
    case KindOfArray:         return Array(m_data.parr);
    case KindOfObject:        return m_data.pobj->toArray();
    case KindOfResource:      return m_data.pres->o_toArray();
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
    case KindOfStaticString:
    case KindOfString:
    case KindOfResource: {
      auto obj = SystemLib::AllocStdClassObject();
      obj->o_set(s_scalar, *this, false);
      return obj;
    }

    case KindOfArray:
      return ObjectData::FromArray(m_data.parr);

    case KindOfObject:
      return m_data.pobj;

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
    case KindOfStaticString:
    case KindOfString:
    case KindOfArray:
    case KindOfObject:
      return Resource(req::make<DummyResource>());

    case KindOfResource:
      return m_data.pres;

    case KindOfRef:
      return m_data.pref->var()->toResource();

    case KindOfClass:
      break;
  }
  not_reached();
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
      return VarNR(staticEmptyString());

    case KindOfBoolean:
    case KindOfInt64:
      return VarNR(m_data.num);

    case KindOfDouble:
    case KindOfResource:
      return VarNR(toInt64());

    case KindOfStaticString:
    case KindOfString:
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

///////////////////////////////////////////////////////////////////////////////
// offset functions

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

Variant& lvalBlackHole() {
  auto& bh = get_env_constants()->lvalProxy;
  bh = uninit_null();
  return bh;
}

void Variant::setEvalScalar() {
  switch (m_type) {
    DT_UNCOUNTED_CASE:
      return;

    case KindOfString: {
      StringData *pstr = m_data.pstr;
      if (!pstr->isStatic()) {
        StringData *sd = makeStaticString(pstr);
        decRefStr(pstr);
        m_data.pstr = sd;
        assert(m_data.pstr->isStatic());
        m_type = KindOfStaticString;
      }
      return;
    }

    case KindOfArray: {
      ArrayData *parr = m_data.parr;
      if (!parr->isStatic()) {
        ArrayData *ad = ArrayData::GetScalarArray(parr);
        decRefArr(parr);
        m_data.parr = ad;
        assert(m_data.parr->isStatic());
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


namespace {
static void serializeRef(const TypedValue* tv,
                         VariableSerializer* serializer,
                         bool isArrayKey) {
  assert(tv->m_type == KindOfRef);
  // Ugly, but behavior is different for serialize
  if (serializer->getType() == VariableSerializer::Type::Serialize ||
      serializer->getType() == VariableSerializer::Type::APCSerialize ||
      serializer->getType() == VariableSerializer::Type::DebuggerSerialize) {
    if (serializer->incNestedLevel(tv->m_data.pref->var())) {
      serializer->writeOverflow(tv->m_data.pref->var());
    } else {
      // Tell the inner variant to skip the nesting check for data inside
      serializeVariant(*tv->m_data.pref->var(), serializer, isArrayKey, true);
    }
    serializer->decNestedLevel(tv->m_data.pref->var());
  } else {
    serializeVariant(*tv->m_data.pref->var(), serializer, isArrayKey);
  }
}
}

///////////////////////////////////////////////////////////////////////////////
// output functions

void serializeVariant(const Variant& self, VariableSerializer *serializer,
                      bool isArrayKey /* = false */,
                      bool skipNestCheck /* = false */,
                      bool noQuotes /* = false */) {
  auto tv = self.asTypedValue();

  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
      assert(!isArrayKey);
      serializer->writeNull();
      return;

    case KindOfBoolean:
      assert(!isArrayKey);
      serializer->write(tv->m_data.num != 0);
      return;

    case KindOfInt64:
      serializer->write(tv->m_data.num);
      return;

    case KindOfDouble:
      serializer->write(tv->m_data.dbl);
      return;

    case KindOfStaticString:
    case KindOfString:
      serializer->write(tv->m_data.pstr->data(),
                        tv->m_data.pstr->size(), isArrayKey, noQuotes);
      return;

    case KindOfArray:
      assert(!isArrayKey);
      tv->m_data.parr->serialize(serializer, skipNestCheck);
      return;

    case KindOfObject:
      assert(!isArrayKey);
      tv->m_data.pobj->serialize(serializer);
      return;

    case KindOfResource:
      assert(!isArrayKey);
      tv->m_data.pres->serialize(serializer);
      return;

    case KindOfRef:
      serializeRef(tv, serializer, isArrayKey);
      return;

    case KindOfClass:
      break;
  }
  not_reached();
}

static void unserializeProp(VariableUnserializer* uns,
                            ObjectData* obj,
                            const String& key,
                            Class* ctx,
                            const String& realKey,
                            int nProp) {
  // Do a two-step look up
  auto const lookup = obj->getProp(ctx, key.get());
  Variant* t;

  if (!lookup.prop || !lookup.accessible) {
    // Dynamic property. If this is the first, and we're using MixedArray,
    // we need to pre-allocate space in the array to ensure the elements
    // dont move during unserialization.
    //
    // TODO(#2881866): this assumption means we can't do reallocations
    // when promoting kPackedKind -> kMixedKind.
    t = &obj->reserveProperties(nProp).lvalAt(realKey, AccessFlags::Key);
  } else {
    t = &tvAsVariant(lookup.prop);
  }

  if (UNLIKELY(IS_REFCOUNTED_TYPE(t->getRawType()))) {
    uns->putInOverwrittenList(*t);
  }

  unserializeVariant(*t, uns);
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
    key,
    obj->getVMClass()->name(),
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

void unserializeVariant(Variant& self, VariableUnserializer *uns,
                        UnserializeMode mode /* = UnserializeMode::Value */) {

  // NOTE: If you make changes to how serialization and unserialization work,
  // make sure to update the reserialize() method in "runtime/ext/ext_apc.cpp"
  // and to update test_apc_reserialize() in "test/ext/test_ext_apc.cpp".

  char type = uns->readChar();
  char sep = uns->readChar();

  if (type != 'R') {
    uns->add(&self, mode);
  }

  if (type == 'N') {
    if (sep != ';') throw Exception("Expected ';' but got '%c'", sep);
    self.setNull(); // NULL *IS* the value, without we get undefined warnings
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
      self = *v;
    }
    break;
  case 'R':
    {
      int64_t id = uns->readInt();
      Variant *v = uns->getByRef(id);
      if (v == nullptr) {
        throw Exception("Id %" PRId64 " out of range", id);
      }
      self.assignRef(*v);
    }
    break;
  case 'b': { int64_t v = uns->readInt(); self = (bool)v; } break;
  case 'i': { int64_t v = uns->readInt(); self = v;       } break;
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
      self = negative ? -v : v;
    }
    break;
  case 's':
    {
      String v;
      v.unserialize(uns);
      self = std::move(v);
      if (!uns->endOfBuffer()) {
        // Semicolon *should* always be required,
        // but PHP's implementation allows omitting it
        // and still functioning.
        // Worse, it throws it away without any check.
        // So we'll do the same.  Sigh.
        uns->readChar();
      }
    }
    return;
  case 'S':
    if (uns->type() == VariableUnserializer::Type::APCSerialize) {
      union {
        char buf[8];
        StringData *sd;
      } u;
      uns->read(u.buf, 8);
      self = u.sd;
    } else {
      throw Exception("Unknown type '%c'", type);
    }
    break;
  case 'a':
    {
      // Check stack depth to avoid overflow.
      check_recursion_throw();
      auto v = Array::Create();
      v.unserialize(uns);
      self = std::move(v);
    }
    return; // array has '}' terminating
  case 'L':
    {
      int64_t id = uns->readInt();
      uns->expectChar(':');
      String rsrcName;
      rsrcName.unserialize(uns);
      uns->expectChar('{');
      uns->expectChar('}');
      auto rsrc = req::make<DummyResource>();
      rsrc->o_setResourceId(id);
      rsrc->m_class_name = rsrcName;
      self = std::move(rsrc);
    }
    return; // resource has '}' terminating
  case 'O':
  case 'V':
  case 'K':
    {
      String clsName;
      clsName.unserialize(uns);

      uns->expectChar(':');
      int64_t size = uns->readInt();
      uns->expectChar(':');
      uns->expectChar('{');

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
          assert(obj.isNull());
          throw_null_pointer_exception();
        } else {
          obj = Object{cls};
          if (UNLIKELY(collections::isType(cls, CollectionType::Pair) &&
                       (size != 2))) {
            throw Exception("Pair objects must have exactly 2 elements");
          }
        }
      } else {
        obj = Object{SystemLib::s___PHP_Incomplete_ClassClass};
        obj->o_set(s_PHP_Incomplete_Class_Name, clsName);
      }
      assert(!obj.isNull());
      self = obj;

      if (size > 0) {
        // Check stack depth to avoid overflow.
        check_recursion_throw();

        if (type == 'O') {
          // Collections are not allowed
          if (obj->isCollection()) {
            throw Exception("%s does not support the 'O' serialization "
                            "format", clsName.data());
          }

          Variant serializedNativeData = init_null();
          bool hasSerializedNativeData = false;

          /*
            Count backwards so that i is the number of properties
            remaining (to be used as an estimate for the total number
            of dynamic properties when we see the first dynamic prop).
            see getVariantPtr
          */
          for (int64_t i = size; i--; ) {
            Variant v;
            unserializeVariant(v, uns, UnserializeMode::Key);
            String key = v.toString();
            int ksize = key.size();
            const char *kdata = key.data();
            int subLen = 0;
            if (key == ObjectData::s_serializedNativeDataKey) {
              unserializeVariant(serializedNativeData, uns);
              hasSerializedNativeData = true;
            } else if (kdata[0] == '\0') {
              if (UNLIKELY(!ksize)) {
                raise_error("Cannot access empty property");
              }
              // private or protected
              subLen = strlen(kdata + 1) + 2;
              if (UNLIKELY(subLen >= ksize)) {
                if (subLen == ksize) {
                  raise_error("Cannot access empty property");
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

            if (i > 0) {
              auto lastChar = uns->peekBack();
              if ((lastChar != ';') && (lastChar != '}')) {
                throw Exception("Object property not terminated properly");
              }
            }
          }

          // nativeDataWakeup is called last to ensure that all properties are
          // already unserialized. We also ensure that nativeDataWakeup is
          // invoked regardless of whether or not serialized native data exists
          // within the serialized content.
          if (obj->getAttribute(ObjectData::HasNativeData) &&
              obj->getVMClass()->getNativeDataInfo()->isSerializable()) {
            Native::nativeDataWakeup(obj.get(), serializedNativeData);
          } else if (hasSerializedNativeData) {
            raise_warning("%s does not expect any serialized native data.",
                          clsName.data());
          }
        } else {
          assert(type == 'V' || type == 'K');
          if (!obj->isCollection()) {
            throw Exception("%s is not a collection class", clsName.data());
          }
          collections::unserialize(obj.get(), uns, size, type);
        }
      }
      uns->expectChar('}');

      if (uns->type() != VariableUnserializer::Type::DebuggerSerialize ||
          (cls && cls->instanceCtor() && cls->isCppSerializable())) {
        // Don't call wakeup when unserializing for the debugger, except for
        // natively implemented classes.
        obj->invokeWakeup();
      }

      check_request_surprise_unlikely();
    }
    return; // object has '}' terminating
  case 'C':
    {
      if (uns->type() == VariableUnserializer::Type::DebuggerSerialize) {
        raise_error("Debugger shouldn't call custom unserialize method");
      }
      String clsName;
      clsName.unserialize(uns);

      uns->expectChar(':');
      String serialized;
      serialized.unserialize(uns, '{', '}');

      auto const obj = [&]() -> Object {
        if (auto const cls = Unit::loadClass(clsName.get())) {
          return Object::attach(g_context->createObject(cls, init_null_variant,
                                                        false /* init */));
        }
        if (!uns->allowUnknownSerializableClass()) {
          raise_error("unknown class %s", clsName.data());
        }
        Object ret = create_object_only(s_PHP_Incomplete_Class);
        ret->o_set(s_PHP_Incomplete_Class_Name, clsName);
        ret->o_set("serialized", serialized);
        return ret;
      }();

      if (!obj->instanceof(SystemLib::s_SerializableClass)) {
        raise_warning("Class %s has no unserializer",
                      obj->getClassName().data());
      } else {
        obj->o_invoke_few_args(s_unserialize, 1, serialized);
        obj.get()->clearNoDestruct();
      }

      self = std::move(obj);
    }
    return; // object has '}' terminating
  default:
    throw Exception("Unknown type '%c'", type);
  }
  uns->expectChar(';');
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
