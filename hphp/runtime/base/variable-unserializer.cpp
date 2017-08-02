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
#include "hphp/runtime/base/variable-unserializer.h"

#include <algorithm>
#include <utility>

#include <folly/Conv.h>
#include <folly/Range.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/struct-log-util.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/zend-strtod.h"

#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/std/ext_std_classobj.h"

#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/repo.h"

#include "hphp/runtime/vm/jit/perf-counters.h"

namespace HPHP {

namespace {

enum class ArrayKind { PHP, Dict, Vec, Keyset };

[[noreturn]] NEVER_INLINE
void throwUnexpectedSep(char expect, char actual) {
  throw Exception("Expected '%c' but got '%c'", expect, actual);
}

[[noreturn]] NEVER_INLINE
void throwOutOfRange(int64_t id) {
  throw Exception("Id %" PRId64 " out of range", id);
}

[[noreturn]] NEVER_INLINE
void throwUnexpectedStr(const char* expect, folly::StringPiece& actual) {
  throw Exception("Expected '%s' but got '%.*s'", expect,
                  (int)actual.size(), actual.data());
}

[[noreturn]] NEVER_INLINE
void throwUnknownType(char type) {
  throw Exception("Unknown type '%c'", type);
}

[[noreturn]] NEVER_INLINE
void throwInvalidPair() {
  throw Exception("Pair objects must have exactly 2 elements");
}

[[noreturn]] NEVER_INLINE
void throwInvalidOFormat(const String& clsName) {
  throw Exception("%s does not support the 'O' serialization format",
                  clsName.data());
}

[[noreturn]] NEVER_INLINE
void throwMangledPrivateProperty() {
  throw Exception("Mangled private object property");
}

[[noreturn]] NEVER_INLINE
void throwUnterminatedProperty() {
  throw Exception("Object property not terminated properly");
}

[[noreturn]] NEVER_INLINE
void throwNotCollection(const String& clsName) {
  throw Exception("%s is not a collection class", clsName.data());
}

[[noreturn]] NEVER_INLINE
void throwUnexpectedType(const String& key, const ObjectData* obj,
                         const Variant& type) {
  auto msg = folly::format(
    "Property {} for class {} was deserialized with type ({}) that "
    "didn't match what we inferred in static analysis",
    key,
    obj->getVMClass()->name(),
    tname(type.asTypedValue()->m_type)
  ).str();
  throw Exception(msg);
}

[[noreturn]] NEVER_INLINE
void throwUnexpectedType(const StringData* key, const ObjectData* obj,
                         const Variant& type) {
  String str(key->data(), key->size(), CopyString);
  throwUnexpectedType(str, obj, type);
}

[[noreturn]] NEVER_INLINE
void throwArraySizeOutOfBounds() {
  throw Exception("Array size out of bounds");
}

[[noreturn]] NEVER_INLINE
void throwInvalidKey() {
  throw Exception("Invalid key");
}

[[noreturn]] NEVER_INLINE
void throwUnterminatedElement() {
  throw Exception("Array element not terminated properly");
}

[[noreturn]] NEVER_INLINE
void throwLargeStringSize(int64_t size) {
  throw Exception("Size of serialized string (%ld) exceeds max", size);
}

[[noreturn]] NEVER_INLINE
void throwNegativeStringSize(int64_t size) {
  throw Exception("Size of serialized string (%ld) must not be negative", size);
}

[[noreturn]] NEVER_INLINE
void throwBadFormat(const ObjectData* obj, char type) {
  throw Exception("%s does not support the '%c' serialization format",
                  header_names[(int)obj->headerKind()], type);
}

[[noreturn]] NEVER_INLINE
void throwInvalidHashKey(const ObjectData* obj) {
  throw Exception("%s values must be integers or strings",
                  header_names[(int)obj->headerKind()]);
}

[[noreturn]] NEVER_INLINE
void throwColRKey() {
  throw Exception("Referring to collection keys using the 'r' encoding "
                    "is not supported");
}

[[noreturn]] NEVER_INLINE
void throwColRefValue() {
  throw Exception("Collection values cannot be taken by reference");
}

[[noreturn]] NEVER_INLINE
void throwColRefKey() {
  throw Exception("Collection keys cannot be taken by reference");
}

[[noreturn]] NEVER_INLINE
void throwUnexpectedEOB() {
  throw Exception("Unexpected end of buffer during unserialization");
}

[[noreturn]] NEVER_INLINE
void throwVecRefValue() {
  throw Exception("Vecs cannot contain references");
}

[[noreturn]] NEVER_INLINE
void throwDictRefValue() {
  throw Exception("Dicts cannot contain references");
}

[[noreturn]] NEVER_INLINE
void throwKeysetValue() {
  throw Exception("Keysets can only contain integers and strings");
}

[[noreturn]] NEVER_INLINE
void throwInvalidClassName() {
  throw Exception("Provided class name is invalid");
}
}

const StaticString
  s_unserialize("unserialize"),
  s_PHP_Incomplete_Class("__PHP_Incomplete_Class"),
  s_PHP_Incomplete_Class_Name("__PHP_Incomplete_Class_Name"),
  s___wakeup("__wakeup");

///////////////////////////////////////////////////////////////////////////////

VariableUnserializer::RefInfo::RefInfo(Variant* v) : RefInfo(v, Type::Value) {}
VariableUnserializer::RefInfo::RefInfo(Variant* v, Type t) {
  m_data.set(t, v);
}

VariableUnserializer::RefInfo
VariableUnserializer::RefInfo::makeColValue(Variant* v) {
  return RefInfo{v, Type::ColValue};
}
VariableUnserializer::RefInfo
VariableUnserializer::RefInfo::makeVecValue(Variant* v) {
  return RefInfo{v, Type::VecValue};
}
VariableUnserializer::RefInfo
VariableUnserializer::RefInfo::makeDictValue(Variant* v) {
  return RefInfo{v, Type::DictValue};
}

Variant* VariableUnserializer::RefInfo::var() const {
  return const_cast<Variant*>(m_data.ptr());
}

bool VariableUnserializer::RefInfo::canBeReferenced() const {
  return m_data.tag() == Type::Value;
}
bool VariableUnserializer::RefInfo::isVecValue() const {
  return m_data.tag() == Type::VecValue;
}
bool VariableUnserializer::RefInfo::isDictValue() const {
  return m_data.tag() == Type::DictValue;
}
bool VariableUnserializer::RefInfo::isColValue() const {
  return m_data.tag() == Type::ColValue;
}

VariableUnserializer::VariableUnserializer(
  const char* str,
  size_t len,
  Type type,
  bool allowUnknownSerializableClass,
  const Array& options)
    : m_type(type)
    , m_readOnly(false)
    , m_buf(str)
    , m_end(str + len)
    , m_unknownSerializable(allowUnknownSerializableClass)
    , m_options(options)
    , m_begin(str)
{}

VariableUnserializer::Type VariableUnserializer::type() const {
  return m_type;
}

bool VariableUnserializer::allowUnknownSerializableClass() const {
  return m_unknownSerializable;
}

const char* VariableUnserializer::head() const {
  return m_buf;
}

const char* VariableUnserializer::begin() const {
  return m_begin;
}

const char* VariableUnserializer::end() const {
  return m_end;
}

char VariableUnserializer::peek() const {
  check();
  return *m_buf;
}

char VariableUnserializer::peekBack() const {
  return m_buf[-1];
}

bool VariableUnserializer::endOfBuffer() const {
  return m_buf >= m_end;
}

char VariableUnserializer::readChar() {
  check();
  return *(m_buf++);
}

void VariableUnserializer::add(Variant* v, UnserializeMode mode) {
  if (mode == UnserializeMode::Value) {
    m_refs.emplace_back(RefInfo(v));
  } else if (mode == UnserializeMode::Key) {
    // do nothing
  } else if (mode == UnserializeMode::VecValue) {
    m_refs.emplace_back(RefInfo::makeVecValue(v));
  } else if (mode == UnserializeMode::DictValue) {
    m_refs.emplace_back(RefInfo::makeDictValue(v));
  } else if (mode == UnserializeMode::ColValue) {
    m_refs.emplace_back(RefInfo::makeColValue(v));
  } else {
    assert(mode == UnserializeMode::ColKey);
    // We don't currently support using the 'r' encoding to refer
    // to collection keys, but eventually we'll need to make this
    // work to allow objects as keys. For now we encode collections
    // keys in m_refs using a null pointer.
    m_refs.emplace_back(RefInfo(nullptr));
  }
}

void VariableUnserializer::reserveForAdd(size_t count) {
  m_refs.reserve(m_refs.size() + count);
}

Variant* VariableUnserializer::getByVal(int id) {
  if (id <= 0 || id > (int)m_refs.size()) return nullptr;
  Variant* ret = m_refs[id-1].var();
  if (!ret) throwColRKey();
  return ret;
}

Variant* VariableUnserializer::getByRef(int id) {
  if (id <= 0 || id > (int)m_refs.size()) return nullptr;
  auto const& info = m_refs[id-1];
  if (UNLIKELY(!info.canBeReferenced())) {
    if (info.isColValue()) {
      throwColRefValue();
    } else if (info.isVecValue()) {
      throwVecRefValue();
    } else {
      assert(info.isDictValue());
      throwDictRefValue();
    }
  } else if (RuntimeOption::EvalHackArrCompatNotices) {
    raiseHackArrCompatRefNew();
  }
  Variant* ret = info.var();
  if (!ret) throwColRefKey();
  return ret;
}

void VariableUnserializer::check() const {
  if (m_buf >= m_end) throwUnexpectedEOB();
}

void VariableUnserializer::set(const char* buf, const char* end) {
  m_buf = buf;
  m_end = end;
}

Variant VariableUnserializer::unserialize() {
  Variant v;
  unserializeVariant(v);
  if (UNLIKELY(StructuredLog::coinflip(RuntimeOption::EvalSerDesSampleRate))) {
    String ser(m_begin, m_end - m_begin, CopyString);
    auto const fmt = folly::sformat("VU{}", (int)m_type);
    StructuredLog::logSerDes(fmt.c_str(), "des", ser, v);
  }

  for (auto& obj : m_sleepingObjects) {
    obj->invokeWakeup();
  }

  return v;
}

namespace {
std::pair<int64_t,const char*> hh_strtoll_base10(const char* p) {
  int64_t x = 0;
  bool neg = false;
  if (*p == '-') {
    neg = true;
    ++p;
  }
  while (*p >= '0' && *p <= '9') {
    x = (x * 10) + ('0' - *p);
    ++p;
  }
  if (!neg) {
    x = -x;
  }
  return std::pair<int64_t,const char*>(x, p);
}
}

int64_t VariableUnserializer::readInt() {
  check();
  auto r = hh_strtoll_base10(m_buf);
  m_buf = r.second;
  return r.first;
}

double VariableUnserializer::readDouble() {
  check();
  const char* newBuf;
  double r = zend_strtod(m_buf, &newBuf);
  m_buf = newBuf;
  return r;
}

folly::StringPiece VariableUnserializer::readStr(unsigned n) {
  check();
  auto const bufferLimit = std::min(size_t(m_end - m_buf), size_t(n));
  auto str = folly::StringPiece(m_buf, bufferLimit);
  m_buf += bufferLimit;
  return str;
}

void VariableUnserializer::expectChar(char expected) {
  char ch = readChar();
  if (UNLIKELY(ch != expected)) {
    throwUnexpectedSep(expected, ch);
  }
}

namespace {
bool isWhitelistClass(const String& requestedClassName,
                      const Array& list,
                      bool includeSubclasses) {
  if (!list.empty()) {
    for (ArrayIter iter(list); iter; ++iter) {
      auto allowedClassName = iter.second().toString();
      auto const matches = includeSubclasses
        ? HHVM_FN(is_a)(requestedClassName, allowedClassName, true)
        : allowedClassName.get()->isame(requestedClassName.get());
      if (matches) return true;
    }
  }
  return false;
}
}

const StaticString s_allowed_classes("allowed_classes");
const StaticString s_include_subclasses("include_subclasses");

bool VariableUnserializer::whitelistCheck(const String& clsName) const {
  if (m_type != Type::Serialize || m_options.isNull()) {
    return true;
  }

  // PHP7-style class whitelisting
  // Allowed classes are allowed,
  // all others result in __Incomplete_PHP_Class
  if (m_options.exists(s_allowed_classes)) {
    auto allowed_classes = m_options[s_allowed_classes];
    if (allowed_classes.isArray()) {
      bool subs = m_options.exists(s_include_subclasses)
        && m_options[s_include_subclasses].toBoolean();
      return isWhitelistClass(clsName,
                              allowed_classes.toArray(),
                              subs);
    } else if (allowed_classes.isBoolean()) {
      return allowed_classes.toBoolean();
    } else {
      throw InvalidAllowedClassesException();
    }
  }

  if (!RuntimeOption::UnserializationWhitelistCheck) {
    // No need for BC HHVM-style whitelist check,
    // since the check isn't enabled.
    // Go with PHP5 default behavior of allowing all
    return true;
  }

  // Check for old-style whitelist
  if (isWhitelistClass(clsName, m_options, false)) {
    return true;
  }

  // Non-whitelisted class with a check enabled,
  // are we willing to hard-error over it?
  const char* err_msg =
    "The object being unserialized with class name '%s' "
    "is not in the given whitelist"; // followed by ' in <filename> on line %d'.

  if (RuntimeOption::UnserializationWhitelistCheckWarningOnly) {
    // Nope, just whine to the user and let it through
    raise_warning(err_msg, clsName.c_str());
    return true;
  } else {
    // Yes, shut it down.
    raise_error(err_msg, clsName.c_str());
    return false;
  }
}

void VariableUnserializer::putInOverwrittenList(const Variant& v) {
  m_overwrittenList.append(v);
}

void VariableUnserializer::addSleepingObject(const Object& o) {
  m_sleepingObjects.emplace_back(o);
}

bool VariableUnserializer::matchString(folly::StringPiece str) {
  const char* p = m_buf;
  const auto ss = str.size();
  if (ss >= 100) return false;
  int digs = ss >= 10 ? 2 : 1;
  int total = 2 + digs + 2 + ss + 2;
  if (p + total > m_end) return false;
  if (*p++ != 's') return false;
  if (*p++ != ':') return false;
  if (digs == 2) {
    if (*p++ != '0' + ss/10) return false;
    if (*p++ != '0' + ss%10) return false;
  } else {
    if (*p++ != '0' + ss) return false;
  }
  if (*p++ != ':') return false;
  if (*p++ != '\"') return false;
  if (memcmp(p, str.data(), ss)) return false;
  p += ss;
  if (*p++ != '\"') return false;
  if (*p++ != ';') return false;
  assert(m_buf + total == p);
  m_buf = p;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

// remainingProps should include the current property being unserialized.
void VariableUnserializer::unserializePropertyValue(Variant& v,
                                                    int remainingProps) {
  assert(remainingProps > 0);
  unserializeVariant(v);
  if (--remainingProps > 0) {
    auto lastChar = peekBack();
    if (lastChar != ';' && lastChar != '}') {
      throwUnterminatedProperty();
    }
  }
}

// nProp should include the current property being unserialized.
NEVER_INLINE
void VariableUnserializer::unserializeProp(ObjectData* obj,
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
    // Ignore fields which are marked as NoSerialize
    auto const cls = obj->getVMClass();
    auto const propIdx = cls->getDeclPropIndex(ctx, key.get()).prop;
    assertx(propIdx != kInvalidSlot);
    if (UNLIKELY(cls->declProperties()[propIdx].attrs & AttrNoSerialize)) {
      Variant temp;
      return unserializePropertyValue(temp, nProp);
    }

    t = &tvAsVariant(lookup.prop);
  }

  if (UNLIKELY(isRefcountedType(t->getRawType()))) {
    putInOverwrittenList(*t);
  }

  unserializePropertyValue(*t, nProp);
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
  throwUnexpectedType(key, obj, *t);
}


NEVER_INLINE
void VariableUnserializer::unserializeRemainingProps(
  Object& obj,
  int remainingProps,
  Variant& serializedNativeData,
  bool& hasSerializedNativeData) {
  while (remainingProps > 0) {
    /*
      use the number of properties remaining as an estimate for
      the total number of dynamic properties when we see the
      first dynamic prop.  see getVariantPtr
    */
    Variant v;
    unserializeVariant(v, UnserializeMode::Key);
    String key = v.toString();
    int ksize = key.size();
    const char *kdata = key.data();
    int subLen = 0;
    if (key == s_serializedNativeDataKey) {
      unserializePropertyValue(serializedNativeData,
                               remainingProps--);
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
          throwMangledPrivateProperty();
        }
      }
      String k(kdata + subLen, ksize - subLen, CopyString);
      Class* ctx = (Class*)-1;
      if (kdata[1] != '*') {
        ctx = Unit::lookupClass(
          String(kdata + 1, subLen - 2, CopyString).get());
      }
      unserializeProp(obj.get(), k, ctx, key,
                      remainingProps--);
    } else {
      unserializeProp(obj.get(), key, nullptr, key,
                      remainingProps--);
    }
  }
}

namespace {

static const StaticString
  s_Vector("Vector"), s_HH_Vector("HH\\Vector"),
  s_Map("Map"), s_HH_Map("HH\\Map"),
  s_Set("Set"), s_HH_Set("HH\\Set"),
  s_Pair("Pair"), s_HH_Pair("HH\\Pair"),
  s_StableMap("StableMap");

/*
 * For namespaced collections, returns an "alternate" name, which is a
 * collection name with or without the namespace qualifier, depending on
 * what's passed.
 * If no alternate name is found, returns nullptr.
 */
const StringData* getAlternateCollectionName(const StringData* clsName) {
  typedef hphp_hash_map<const StringData*, const StringData*,
                        string_data_hash, string_data_isame> ClsNameMap;

  auto getAltMap = [] {
    typedef std::pair<StaticString, StaticString> SStringPair;

    static ClsNameMap m;

    static std::vector<SStringPair> mappings {
      std::make_pair(s_Vector, s_HH_Vector),
      std::make_pair(s_Map, s_HH_Map),
      std::make_pair(s_Set, s_HH_Set),
      std::make_pair(s_Pair, s_HH_Pair)
    };

    for (const auto& p : mappings) {
      m[p.first.get()] = p.second.get();
      m[p.second.get()] = p.first.get();
    }

    // As part of StableMap merging into Map, StableMap is an alias for HH\\Map,
    // but Map is the sole alias for HH\\Map
    m[s_StableMap.get()] = s_HH_Map.get();
    return &m;
  };

  static const ClsNameMap* altMap = getAltMap();

  auto it = altMap->find(clsName);
  return it != altMap->end() ? it->second : nullptr;
}

Class* tryAlternateCollectionClass(const StringData* clsName) {
  auto altName = getAlternateCollectionName(clsName);
  return altName ? Unit::getClass(altName, /* autoload */ false) : nullptr;
}

/*
 * Try to read 'str' while advancing 'cur' without reaching 'end'.
 */
ALWAYS_INLINE
static bool match(const char*& cur,
                  const char* expected,
                  const char* const end) {
  if (cur + strlen(expected) >= end) return false;
  while (*expected) {
    if (*cur++ != *expected++) return false;
  }
  return true;
}

ALWAYS_INLINE
static int64_t read64(const char*& cur) {
  auto p = hh_strtoll_base10(cur);
  cur = p.second;
  return p.first;
}

/*
 * Read an int64 from 'cur' into 'out'. Returns false on unexpected
 * (but possibly still legal) format or if 'end' is reached.
 */
ALWAYS_INLINE
bool readInt64(const char*& cur, const char* const end, int64_t& out) {
  if (!match(cur, "i:", end)) return false;
  out = read64(cur);
  return match(cur, ";", end);
}

/*
 * Read, allocate, and return a string from 'cur'. Returns null on unexpected
 * (but possibly still legal) format or if 'end' is reached, without allocating.
 */
ALWAYS_INLINE
static StringData* readStringData(const char*& cur, const char* const end,
                                  int maxLen) {
  if (!match(cur, "s:", end)) return nullptr;
  auto len = read64(cur);
  if (len < 0 || len >= maxLen) return nullptr;
  if (!match(cur, ":\"", end)) return nullptr;
  auto const slice = folly::StringPiece(cur, len);
  if ((cur += len) >= end) return nullptr;
  if (!match(cur, "\";", end)) return nullptr;
  // TODO(11398853): Consider streaming/non-temporal stores here.
  auto sd = StringData::Make(slice, CopyString);
  return sd;
}
}

NEVER_INLINE
void VariableUnserializer::unserializeVariant(
  Variant& self,
  UnserializeMode mode /* = UnserializeMode::Value */) {

  // NOTE: If you make changes to how serialization and unserialization work,
  // make sure to update reserialize() here and test_apc_reserialize()
  // in "test/ext/test_ext_apc.cpp".

  char type = readChar();
  char sep = readChar();

  if (type != 'R') {
    add(&self, mode);
  }

  if (type == 'N') {
    if (sep != ';') throwUnexpectedSep(';', sep);
    self.setNull(); // NULL *IS* the value, without we get undefined warnings
    return;
  }
  if (sep != ':') throwUnexpectedSep(':', sep);

  switch (type) {
  case 'r':
    {
      int64_t id = readInt();
      Variant *v = getByVal(id);
      if (!v) throwOutOfRange(id);
      tvSet(tvToInitCell(v->asTypedValue()), *self.asTypedValue());
    }
    break;
  case 'R':
    {
      if (UNLIKELY(mode == UnserializeMode::VecValue)) {
        throwVecRefValue();
      } else if (UNLIKELY(mode == UnserializeMode::DictValue)) {
        throwDictRefValue();
      } else if (RuntimeOption::EvalHackArrCompatNotices) {
        raiseHackArrCompatRefNew();
      }

      int64_t id = readInt();
      Variant *v = getByRef(id);
      if (!v) throwOutOfRange(id);
      self.assignRef(*v);
    }
    break;
  case 'b':
    {
      int64_t v = readInt();
      tvSetBool((bool)v, *self.asTypedValue());
      break;
    }
  case 'i':
    {
      int64_t v = readInt();
      tvSetInt(v, *self.asTypedValue());
      break;
    }
  case 'd':
    {
      char ch = peek();
      bool negative = false;
      if (ch == '-') {
        negative = true;
        readChar();
        ch = peek();
      }
      double v;
      if (ch == 'I') {
        auto str = readStr(3);
        if (str.size() != 3 || str[1] != 'N' || str[2] != 'F') {
          throwUnexpectedStr("INF", str);
        }
        v = std::numeric_limits<double>::infinity();
      } else if (ch == 'N') {
        auto str = readStr(3);
        if (str.size() != 3 || str[1] != 'A' || str[2] != 'N') {
          throwUnexpectedStr("NAN", str);
        }
        v = std::numeric_limits<double>::quiet_NaN();
      } else {
        v = readDouble();
      }
      tvSetDouble(negative ? -v : v, *self.asTypedValue());
    }
    break;
  case 's':
    {
      String v = unserializeString();
      tvMove(make_tv<KindOfString>(v.detach()), *self.asTypedValue());
      if (!endOfBuffer()) {
        // Semicolon *should* always be required,
        // but PHP's implementation allows omitting it
        // and still functioning.
        // Worse, it throws it away without any check.
        // So we'll do the same.  Sigh.
        readChar();
      }
    }
    return;
  case 'S':
    if (this->type() == VariableUnserializer::Type::APCSerialize) {
      auto str = readStr(8);
      assert(str.size() == 8);
      auto sdp = reinterpret_cast<StringData*const*>(&str[0]);
      assert((*sdp)->isStatic());
      tvMove(make_tv<KindOfPersistentString>(*sdp), *self.asTypedValue());
    } else {
      throwUnknownType(type);
    }
    break;
  case 'a':
  case 'D':
    {
      // Check stack depth to avoid overflow.
      check_recursion_throw();
      // It seems silly to check this here, but GCC actually generates much
      // better code this way.
      auto a = (type == 'a') ? unserializeArray() : unserializeDict();
      tvMove(make_array_like_tv(a.detach()), *self.asTypedValue());
    }
    return; // array has '}' terminating
  case 'v':
    {
      // Check stack depth to avoid overflow.
      check_recursion_throw();
      auto a = unserializeVec();
      tvMove(make_tv<KindOfVec>(a.detach()), *self.asTypedValue());
    }
    return; // array has '}' terminating
  case 'k':
    {
      // Check stack depth to avoid overflow.
      check_recursion_throw();
      auto a = unserializeKeyset();
      tvMove(make_tv<KindOfKeyset>(a.detach()), *self.asTypedValue());
    }
    return; // array has '}' terminating
  case 'L':
    {
      int64_t id = readInt();
      expectChar(':');
      String rsrcName = unserializeString();
      expectChar('{');
      expectChar('}');
      auto rsrc = req::make<DummyResource>();
      rsrc->o_setResourceId(id);
      rsrc->m_class_name = std::move(rsrcName);
      tvMove(make_tv<KindOfResource>(rsrc.detach()->hdr()),
             *self.asTypedValue());
    }
    return; // resource has '}' terminating
  case 'O':
  case 'V':
  case 'K':
    {
      String clsName = unserializeString();

      expectChar(':');
      const int64_t size = readInt();
      expectChar(':');
      expectChar('{');

      const bool allowObjectFormatForCollections = true;

      Class* cls = nullptr;

      // If we are potentially dealing with a collection, we need to try to
      // load the collection class under an alternate name so that we can
      // deserialize data that was serialized before the migration of
      // collections to the HH namespace.

      if (type == 'O') {
        if (whitelistCheck(clsName)) {
          if (allowObjectFormatForCollections) {
            // In order to support the legacy {O|V}:{Set|Vector|Map}
            // serialization, we defer autoloading until we know that there's
            // no alternate (builtin) collection class.
            cls = Unit::getClass(clsName.get(), /* autoload */ false);
            if (!cls) {
              cls = tryAlternateCollectionClass(clsName.get());
            }
          }

          // No valid class was found, lets try the autoloader.
          if (!cls) {
            if (!is_valid_class_name(clsName.slice())) {
              throwInvalidClassName();
            }
            cls = Unit::loadClass(clsName.get()); // with autoloading
          }
        }
      } else {
        // Collections are CPP builtins; don't attempt to autoload
        cls = Unit::getClass(clsName.get(), /* autoload */ false);
        if (!cls) {
          cls = tryAlternateCollectionClass(clsName.get());
        }
        if (!cls || !cls->isCollectionClass()) {
          throwNotCollection(clsName);
        }
      }

      Object obj;
      if (cls) {
        // Only unserialize CPP extension types which can actually
        // support it. Otherwise, we risk creating a CPP object
        // without having it initialized completely.
        if (cls->instanceCtor() && !cls->isCppSerializable() &&
            !cls->isCollectionClass()) {
          assert(obj.isNull());
          throw_null_pointer_exception();
        } else {
          if (UNLIKELY(collections::isType(cls, CollectionType::Pair))) {
            if (UNLIKELY(size != 2)) {
              throwInvalidPair();
            }
            // pairs can't be constructed without elements
            obj = Object{req::make<c_Pair>(make_tv<KindOfNull>(),
                                           make_tv<KindOfNull>(),
                                           c_Pair::NoIncRef{})};
          } else {
            obj = Object{cls};
          }
        }
      } else {
        obj = Object{SystemLib::s___PHP_Incomplete_ClassClass};
        obj->o_set(s_PHP_Incomplete_Class_Name, clsName);
      }
      assert(!obj.isNull());
      tvSet(make_tv<KindOfObject>(obj.get()), *self.asTypedValue());

      if (size > 0) {
        // Check stack depth to avoid overflow.
        check_recursion_throw();

        if (type == 'O') {
          // Collections are not allowed
          if (obj->isCollection()) {
            throwInvalidOFormat(clsName);
          }

          Variant serializedNativeData = init_null();
          bool hasSerializedNativeData = false;
          bool checkRepoAuthType =
            RuntimeOption::RepoAuthoritative &&
            Repo::get().global().HardPrivatePropInference;
          Class* objCls = obj->getVMClass();
          auto remainingProps = size;
          // Try fast case.
          if (size >= objCls->numDeclProperties()) {
            bool mismatch = false;
            auto objProps = obj->propVec();

            for (auto prop : objCls->declProperties()) {
              if (!matchString(prop.mangledName->slice())) {
                mismatch = true;
                break;
              }

              // Ignore NoSerialize props if present
              if (UNLIKELY(prop.attrs & AttrNoSerialize)) {
                ++objProps;
                Variant temp;
                unserializePropertyValue(temp, remainingProps--);
                continue;
              }

              // don't need to worry about overwritten list, because
              // this is definitely the first time we're setting this
              // property.
              TypedValue* tv = objProps++;
              Variant& t = tvAsVariant(tv);
              unserializePropertyValue(t, remainingProps--);

              if (UNLIKELY(checkRepoAuthType &&
                           !tvMatchesRepoAuthType(*tv, prop.repoAuthType))) {
                throwUnexpectedType(prop.name, obj.get(), t);
              }
            }
            // If everything matched, all remaining properties are dynamic.
            if (!mismatch && remainingProps > 0) {
              auto& arr = obj->reserveProperties(remainingProps);
              while (remainingProps > 0) {
                Variant v;
                unserializeVariant(v, UnserializeMode::Key);
                String key = v.toString();
                if (key == s_serializedNativeDataKey) {
                  unserializePropertyValue(serializedNativeData,
                                           remainingProps--);
                  hasSerializedNativeData = true;
                } else {
                  auto t = &arr.lvalAt(key, AccessFlags::Key);
                  if (UNLIKELY(isRefcountedType(t->getRawType()))) {
                    putInOverwrittenList(*t);
                  }
                  unserializePropertyValue(*t, remainingProps--);
                }
              }
            }
          }
          if (remainingProps > 0) {
            INC_TPC(unser_prop_slow);
            unserializeRemainingProps(obj, remainingProps,
                                      serializedNativeData,
                                      hasSerializedNativeData);
            remainingProps = 0;
          } else {
            INC_TPC(unser_prop_fast);
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
            throwNotCollection(clsName);
          }
          unserializeCollection(obj.get(), size, type);
        }
      }
      expectChar('}');

      if (cls &&
          cls->lookupMethod(s___wakeup.get()) &&
          (this->type() != VariableUnserializer::Type::DebuggerSerialize ||
           (cls->instanceCtor() && cls->isCppSerializable()))) {
        // Don't call wakeup when unserializing for the debugger, except for
        // natively implemented classes.
        addSleepingObject(obj);
      }

      check_non_safepoint_surprise();
    }
    return; // object has '}' terminating
  case 'C':
    {
      if (this->type() == VariableUnserializer::Type::DebuggerSerialize) {
        raise_error("Debugger shouldn't call custom unserialize method");
      }
      String clsName = unserializeString();

      expectChar(':');
      String serialized = unserializeString('{', '}');

      auto obj = [&]() -> Object {
        if (whitelistCheck(clsName)) {
          // Try loading without the autoloader first
          auto cls = Unit::getClass(clsName.get(), /* autoload */ false);
          if (!cls) {
            if (!is_valid_class_name(clsName.slice())) {
              throwInvalidClassName();
            }
            cls = Unit::loadClass(clsName.get());
          }
          if (cls) {
            return Object::attach(g_context->createObject(cls, init_null_variant,
                                                          false /* init */));
          }
        }
        if (!allowUnknownSerializableClass()) {
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

      tvMove(make_tv<KindOfObject>(obj.detach()), *self.asTypedValue());
    }
    return; // object has '}' terminating
  default:
    throwUnknownType(type);
  }
  expectChar(';');
}

Array VariableUnserializer::unserializeArray() {
  int64_t size = readInt();
  expectChar(':');
  expectChar('{');
  if (size == 0) {
    expectChar('}');
    return Array::Create();
  }
  if (UNLIKELY(size < 0 || size > std::numeric_limits<int>::max())) {
    throwArraySizeOutOfBounds();
  }
  auto const scale = MixedArray::computeScaleFromSize(size);
  auto const allocsz = MixedArray::computeAllocBytes(scale);

  // For large arrays, do a naive pre-check for OOM.
  if (UNLIKELY(allocsz > kMaxSmallSize && MM().preAllocOOM(allocsz))) {
    check_non_safepoint_surprise();
  }

  // Pre-allocate an ArrayData of the given size, to avoid escalation in the
  // middle, which breaks references.
  Array arr = ArrayInit(size, ArrayInit::Mixed{}).toArray();
  reserveForAdd(size);

  for (int64_t i = 0; i < size; i++) {
    Variant key;
    unserializeVariant(key, UnserializeMode::Key);
    if (!key.isString() && !key.isInteger()) {
      throwInvalidKey();
    }

    // for apc, we know the key can't exist, but ignore that optimization
    assert(type() != VariableUnserializer::Type::APCSerialize ||
           !arr.exists(key, true));

    auto& value = [&]() -> decltype(auto) {
      SuppressHackArrCompatNotices shacn;
      return arr.lvalAt(key, AccessFlags::Key);
    }();
    if (UNLIKELY(isRefcountedType(value.getRawType()))) {
      putInOverwrittenList(value);
    }
    unserializeVariant(value);

    if (i < (size - 1)) {
      auto lastChar = peekBack();
      if ((lastChar != ';' && lastChar != '}')) {
        throwUnterminatedElement();
      }
    }
  }

  check_non_safepoint_surprise();
  expectChar('}');
  return arr;
}

Array VariableUnserializer::unserializeDict() {
  int64_t size = readInt();
  expectChar(':');
  expectChar('{');
  if (size == 0) {
    expectChar('}');
    return Array::CreateDict();
  }
  if (UNLIKELY(size < 0 || size > std::numeric_limits<int>::max())) {
    throwArraySizeOutOfBounds();
  }
  auto const scale = MixedArray::computeScaleFromSize(size);
  auto const allocsz = MixedArray::computeAllocBytes(scale);

  // For large arrays, do a naive pre-check for OOM.
  if (UNLIKELY(allocsz > kMaxSmallSize && MM().preAllocOOM(allocsz))) {
    check_non_safepoint_surprise();
  }

  Array arr = DictInit(size).toArray();
  for (int64_t i = 0; i < size; i++) {
    Variant key;
    unserializeVariant(key, UnserializeMode::Key);
    auto const rawType = key.getRawType();
    if (UNLIKELY(!isIntType(rawType) && !isStringType(rawType))) {
      throwInvalidKey();
    }

    // for apc, we know the key can't exist, but ignore that optimization
    assert(type() != VariableUnserializer::Type::APCSerialize ||
           !arr.exists(key, true));

    auto const lval = [&] {
      SuppressHackArrCompatNotices shacn;
      return key.isInteger()
        ? MixedArray::LvalIntDict(arr.get(), key.asInt64Val(), false)
        : MixedArray::LvalStrDict(arr.get(), key.asCStrRef().get(), false);
    }();
    assertx(lval.arr_base() == arr.get());

    auto& val = tvAsVariant(lval.tv());

    if (UNLIKELY(isRefcountedType(val.getRawType()))) {
      putInOverwrittenList(val);
    }
    unserializeVariant(val, UnserializeMode::DictValue);
    assertx(val.getRawType() != KindOfRef);

    if (i < (size - 1)) {
      auto lastChar = peekBack();
      if ((lastChar != ';' && lastChar != '}')) {
        throwUnterminatedElement();
      }
    }
  }

  check_non_safepoint_surprise();
  expectChar('}');
  return arr;
}

Array VariableUnserializer::unserializeVec() {
  int64_t size = readInt();
  expectChar(':');
  expectChar('{');
  if (size == 0) {
    expectChar('}');
    return Array::CreateVec();
  }
  if (UNLIKELY(size < 0 || size > std::numeric_limits<int>::max())) {
    throwArraySizeOutOfBounds();
  }
  auto const scale = MixedArray::computeScaleFromSize(size);
  auto const allocsz = MixedArray::computeAllocBytes(scale);

  // For large arrays, do a naive pre-check for OOM.
  if (UNLIKELY(allocsz > kMaxSmallSize && MM().preAllocOOM(allocsz))) {
    check_non_safepoint_surprise();
  }

  Array arr = VecArrayInit(size).toArray();
  reserveForAdd(size);

  for (int64_t i = 0; i < size; i++) {
    auto const lval = PackedArray::LvalNewVec(arr.get(), false);
    assertx(lval.arr_base() == arr.get());
    auto& val = tvAsVariant(lval.tv());
    unserializeVariant(val, UnserializeMode::VecValue);
    assertx(val.getRawType() != KindOfRef);

    if (i < (size - 1)) {
      auto lastChar = peekBack();
      if ((lastChar != ';' && lastChar != '}')) {
        throwUnterminatedElement();
      }
    }
  }
  check_non_safepoint_surprise();
  expectChar('}');
  return arr;
}

Array VariableUnserializer::unserializeKeyset() {
  int64_t size = readInt();
  expectChar(':');
  expectChar('{');
  if (size == 0) {
    expectChar('}');
    return Array::CreateKeyset();
  }
  if (UNLIKELY(size < 0 || size > std::numeric_limits<int>::max())) {
    throwArraySizeOutOfBounds();
  }
  auto const scale = SetArray::computeScaleFromSize(size);
  auto const allocsz = SetArray::computeAllocBytes(scale);

  // For large arrays, do a naive pre-check for OOM.
  if (UNLIKELY(allocsz > kMaxSmallSize && MM().preAllocOOM(allocsz))) {
    check_non_safepoint_surprise();
  }

  KeysetInit init(size);
  for (int64_t i = 0; i < size; i++) {
    Variant key;
    // Use key mode to stop the unserializer from keeping a pointer to this
    // variant (since its stack-allocated).
    unserializeVariant(key, UnserializeMode::Key);

    auto const type = key.getRawType();
    if (UNLIKELY(!isStringType(type) && !isIntType(type))) {
      throwKeysetValue();
    }

    init.add(key);

    if (i < (size - 1)) {
      auto lastChar = peekBack();
      if ((lastChar != ';' && lastChar != '}')) {
        throwUnterminatedElement();
      }
    }
  }
  check_non_safepoint_surprise();
  expectChar('}');
  return init.toArray();
}


folly::StringPiece
VariableUnserializer::unserializeStringPiece(char delimiter0, char delimiter1) {
  int64_t size = readInt();
  if (size >= RuntimeOption::MaxSerializedStringSize) {
    throwLargeStringSize(size);
  }
  if (size < 0) {
    throwNegativeStringSize(size);
  }
  expectChar(':');
  expectChar(delimiter0);
  auto const piece = readStr(size);
  expectChar(delimiter1);
  return piece;
}

String VariableUnserializer::unserializeString(char delimiter0,
                                               char delimiter1) {
  auto const piece = unserializeStringPiece(delimiter0, delimiter1);
  return String::attach(readOnly() ?
                        makeStaticString(piece) :
                        StringData::Make(piece, CopyString));
}

void VariableUnserializer::unserializeCollection(ObjectData* obj, int64_t sz,
                                                 char type) {
  switch (obj->collectionType()) {
    case CollectionType::Pair:
      unserializePair(obj, sz, type);
      break;
    case CollectionType::Vector:
    case CollectionType::ImmVector:
      unserializeVector(obj, sz, type);
      break;
    case CollectionType::Map:
    case CollectionType::ImmMap:
      unserializeMap(obj, sz, type);
      break;
    case CollectionType::Set:
    case CollectionType::ImmSet:
      unserializeSet(obj, sz, type);
      break;
  }
}

void VariableUnserializer::unserializeVector(ObjectData* obj, int64_t sz,
                                             char type) {
  if (type != 'V') throwBadFormat(obj, type);
  auto bvec = static_cast<BaseVector*>(obj);
  bvec->reserve(sz);
  reserveForAdd(sz);
  for (int64_t i = 0; i < sz; ++i) {
    auto tv = bvec->appendForUnserialize(i);
    tv->m_type = KindOfNull;
    unserializeVariant(tvAsVariant(tv), UnserializeMode::ColValue);
  }
}

/*
 * Attempts to unserialize into an initially empty HH\Map of string->int/string.
 * Returns false and leaves both 'map' and 'uns' untouched on failure, including
 * unexpected types and possibly legal, but uncommon, encodings.
 */
NEVER_INLINE
bool VariableUnserializer::tryUnserializeStrIntMap(BaseMap* map, int64_t sz) {
  auto b = head();
  /*
   * For efficiency, we don't add the keys/values to m_refs, so don't support
   * back-references appearing after this point. For simplicity, we thus require
   * this map to be the root object being unserialized.
   */
  if (folly::StringPiece(begin(), b) !=
      folly::to<std::string>("K:6:\"HH\\Map\":", sz, ":{")) {
    return false;
  }
  auto const end = this->end();
  auto const maxKeyLen = RuntimeOption::MaxSerializedStringSize;
  /*
   * First, parse the entire input and allocate the keys (accessing lots of
   * data, but mostly sequentially).
   */
  auto checkPoint = map->batchInsertBegin(sz);
  int64_t i = 0;
  for (; i < sz; ++i) {
    auto sd = readStringData(b, end, maxKeyLen);
    if (!sd) break;
    String key = String::attach(sd);
    auto tv = map->batchInsert(key.get());
    tv->m_type = KindOfNull;
    if (*b == 'i') {
      if (!readInt64(b, end, tv->m_data.num)) break;
      tv->m_type = KindOfInt64;
    } else if (*b == 's') {
      auto sd = readStringData(b, end, maxKeyLen);
      if (!sd) break;
      tv->m_data.pstr = sd;
      tv->m_type = KindOfString;
    } else {
      break;
    }
  }
  /*
   * On success, finalize the hash table insertion (very random access).
   */
  if (i == sz && map->tryBatchInsertEnd(checkPoint)) {
    set(b, end);
    return true;
  }
  map->batchInsertAbort(checkPoint);
  return false;
}

void VariableUnserializer::unserializeMap(ObjectData* obj, int64_t sz,
                                          char type) {
  if (type != 'K') throwBadFormat(obj, type);
  auto map = static_cast<BaseMap*>(obj);
  map->reserve(sz);
  if (sz >= RuntimeOption::UnserializationBigMapThreshold &&
      tryUnserializeStrIntMap(map, sz)) {
    return;
  }
  reserveForAdd(sz + sz); // keys + values
  for (int64_t i = 0; i < sz; ++i) {
    Variant k;
    unserializeVariant(k, UnserializeMode::ColKey);
    TypedValue* tv = nullptr;
    if (k.isInteger()) {
      auto h = k.toInt64();
      tv = map->findForUnserialize(h);
      // Be robust against manually crafted inputs with conflicting elements
      if (UNLIKELY(!tv)) {
        tv = k.asTypedValue();
        goto do_unserialize;
      }
    } else if (k.isString()) {
      auto key = k.getStringData();
      tv = map->findForUnserialize(key);
      // Be robust against manually crafted inputs with conflicting elements
      if (UNLIKELY(!tv)) {
        tv = k.asTypedValue();
        goto do_unserialize;
      }
    } else {
      throwInvalidKey();
    }
    tv->m_type = KindOfNull;
do_unserialize:
    unserializeVariant(tvAsVariant(tv), UnserializeMode::ColValue);
  }
}

void VariableUnserializer::unserializeSet(ObjectData* obj, int64_t sz,
                                          char type) {
  if (type != 'V') throwBadFormat(obj, type);
  auto set = static_cast<BaseSet*>(obj);
  set->reserve(sz);
  reserveForAdd(sz);
  for (int64_t i = 0; i < sz; ++i) {
    // When unserializing an element of a Set, we use Mode::ColKey for now.
    // This will make the unserializer to reserve an id for the element
    // but won't allow referencing the element via 'r' or 'R'.
    Variant k;
    unserializeVariant(k, UnserializeMode::ColKey);
    if (k.isInteger()) {
      auto h = k.toInt64();
      auto tv = set->findForUnserialize(h);
      // Be robust against manually crafted inputs with conflicting elements
      if (UNLIKELY(!tv)) continue;
      tv->m_type = KindOfInt64;
      tv->m_data.num = h;
    } else if (k.isString()) {
      auto key = k.getStringData();
      auto tv = set->findForUnserialize(key);
      if (UNLIKELY(!tv)) continue;
      // This increments the string's refcount twice, once for
      // the key and once for the value
      cellDup(make_tv<KindOfString>(key), *tv);
    } else {
      throwInvalidHashKey(obj);
    }
  }
}

void VariableUnserializer::unserializePair(ObjectData* obj, int64_t sz,
                                           char type) {
  assert(sz == 2);
  if (type != 'V') throwBadFormat(obj, type);
  auto pair = static_cast<c_Pair*>(obj);
  unserializeVariant(tvAsVariant(pair->at(0)), UnserializeMode::ColValue);
  unserializeVariant(tvAsVariant(pair->at(1)), UnserializeMode::ColValue);
}

////////////////////////////////////////////////////////////////////////////////

void VariableUnserializer::reserialize(StringBuffer& buf) {

  char type = readChar();
  char sep = readChar();

  if (type == 'N') {
    buf.append(type);
    buf.append(sep);
    return;
  }

  switch (type) {
  case 'r':
  case 'R':
  case 'b':
  case 'i':
  case 'd':
    {
      buf.append(type);
      buf.append(sep);
      while (peek() != ';') {
        char ch;
        ch = readChar();
        buf.append(ch);
      }
    }
    break;
  case 'S':
  case 'A':
    {
      // shouldn't happen, but keep the code here anyway.
      buf.append(type);
      buf.append(sep);
      auto str = readStr(8);
      buf.append(str.data(), str.size());
    }
    break;
  case 's':
    {
      String v = unserializeString();
      assert(!v.isNull());
      if (v.get()->isStatic()) {
        union {
          char pointer[8];
          StringData *sd;
        } u;
        u.sd = v.get();
        buf.append("S:");
        buf.append(u.pointer, 8);
        buf.append(';');
      } else {
        buf.append("s:");
        buf.append(v.size());
        buf.append(":\"");
        buf.append(v.data(), v.size());
        buf.append("\";");
      }
      sep = readChar();
      return;
    }
    break;
  case 'a':
  case 'D':
    {
      buf.append(type == 'a' ? "a:" : "D:");
      int64_t size = readInt();
      char sep2 = readChar();
      buf.append(size);
      buf.append(sep2);
      sep2 = readChar();
      buf.append(sep2);
      for (int64_t i = 0; i < size; i++) {
        reserialize(buf); // key
        reserialize(buf); // value
      }
      sep2 = readChar(); // '}'
      buf.append(sep2);
      return;
    }
    break;
  case 'v':
  case 'k':
    {
      buf.append(type == 'v' ? "v:" : "k:");
      int64_t size = readInt();
      char sep2 = readChar();
      buf.append(size);
      buf.append(sep2);
      sep2 = readChar();
      buf.append(sep2);
      for (int64_t i = 0; i < size; ++i) {
        reserialize(buf);
      }
      sep2 = readChar(); // '}'
      buf.append(sep2);
      return;
    }
  case 'o':
  case 'O':
  case 'V':
  case 'K':
    {
      buf.append(type);
      buf.append(sep);

      auto const clsName = unserializeStringPiece();
      buf.append(static_cast<int>(clsName.size()));
      buf.append(":\"");
      buf.append(clsName.data(), clsName.size());
      buf.append("\":");

      readChar();
      int64_t size = readInt();
      char sep2 = readChar();

      buf.append(size);
      buf.append(sep2);
      sep2 = readChar(); // '{'
      buf.append(sep2);
      // 'V' type is a series with values only, while all other
      // types are series with keys and values
      int64_t i = type == 'V' ? size : size * 2;
      while (i--) {
        reserialize(buf);
      }
      sep2 = readChar(); // '}'
      buf.append(sep2);
      return;
    }
    break;
  case 'C':
    {
      buf.append(type);
      buf.append(sep);

      auto const clsName = unserializeStringPiece();
      buf.append(static_cast<int>(clsName.size()));
      buf.append(":\"");
      buf.append(clsName.data(), clsName.size());
      buf.append("\":");

      sep = readChar(); // ':'
      auto const serialized = unserializeStringPiece('{', '}');
      buf.append(static_cast<int>(serialized.size()));
      buf.append(":{");
      buf.append(serialized.data(), serialized.size());
      buf.append('}');
      return;
    }
    break;
  default:
    throwUnknownType(type);
  }

  sep = readChar(); // the last ';'
  buf.append(sep);
}

}
