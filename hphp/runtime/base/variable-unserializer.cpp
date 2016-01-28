/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/zend-strtod.h"
#include "hphp/runtime/ext/std/ext_std_classobj.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP {

static void unserializeVariant(Variant&, VariableUnserializer* unserializer,
                               UnserializeMode mode = UnserializeMode::Value);
static Array unserializeArray(VariableUnserializer*);
static String unserializeString(VariableUnserializer*, char delimiter0 = '"',
                                char delimiter1 = '"');
static void unserializeCollection(ObjectData* obj, VariableUnserializer* uns,
                                  int64_t sz, char type);
static void unserializeVector(ObjectData*, VariableUnserializer*, int64_t sz,
                              char type);
static void unserializeMap(ObjectData*, VariableUnserializer*, int64_t sz,
                           char type);
static void unserializeSet(ObjectData*, VariableUnserializer*, int64_t sz,
                           char type);
static void unserializePair(ObjectData*, VariableUnserializer*, int64_t sz,
                            char type);

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwUnexpectedSep(char expect, char actual) {
  throw Exception("Expected '%c' but got '%c'", expect, actual);
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwOutOfRange(int64_t id) {
  throw Exception("Id %" PRId64 " out of range", id);
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwUnexpectedStr(const char* expect, folly::StringPiece& actual) {
  throw Exception("Expected '%s' but got '%.*s'", expect,
                  (int)actual.size(), actual.data());
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwUnknownType(char type) {
  throw Exception("Unknown type '%c'", type);
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwInvalidPair() {
  throw Exception("Pair objects must have exactly 2 elements");
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwInvalidOFormat(const String& clsName) {
  throw Exception("%s does not support the 'O' serialization format",
                  clsName.data());
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwMangledPrivateProperty() {
  throw Exception("Mangled private object property");
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwUnterminatedProperty() {
  throw Exception("Object property not terminated properly");
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwNotCollection(const String& clsName) {
  throw Exception("%s is not a collection class", clsName.data());
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwUnexpectedType(const String& key, const ObjectData* obj,
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

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwUnexpectedType(const StringData* key, const ObjectData* obj,
                                const Variant& type) {
  String str(key->data(), key->size(), CopyString);
  throwUnexpectedType(str, obj, type);
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwArraySizeOutOfBounds() {
  throw Exception("Array size out of bounds");
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwInvalidKey() {
  throw Exception("Invalid key");
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwUnterminatedElement() {
  throw Exception("Array element not terminated properly");
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwLargeStringSize(int64_t size) {
  throw Exception("Size of serialized string (%ld) exceeds max", size);
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwNegativeStringSize(int64_t size) {
  throw Exception("Size of serialized string (%ld) must not be negative", size);
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwBadFormat(const ObjectData* obj, char type) {
  throw Exception("%s does not support the '%c' serialization format",
                  header_names[(int)obj->headerKind()], type);
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwInvalidHashKey(const ObjectData* obj) {
  throw Exception("%s values must be integers or strings",
                  header_names[(int)obj->headerKind()]);
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwColRKey() {
  throw Exception("Referring to collection keys using the 'r' encoding "
                    "is not supported");
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwColRefValue() {
  throw Exception("Collection values cannot be taken by reference");
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwColRefKey() {
  throw Exception("Collection keys cannot be taken by reference");
}

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwUnexpectedEOB() {
  throw Exception("Unexpected end of buffer during unserialization");
}

const StaticString
  s_unserialize("unserialize"),
  s_PHP_Incomplete_Class("__PHP_Incomplete_Class"),
  s_PHP_Incomplete_Class_Name("__PHP_Incomplete_Class_Name");

///////////////////////////////////////////////////////////////////////////////

VariableUnserializer::RefInfo::RefInfo(Variant* v)
    : m_data(reinterpret_cast<uintptr_t>(v))
{}

VariableUnserializer::RefInfo
VariableUnserializer::RefInfo::makeNonRefable(Variant* v) {
  RefInfo r(v);
  r.m_data |= 1;
  return r;
}

Variant* VariableUnserializer::RefInfo::var() const  {
  return reinterpret_cast<Variant*>(m_data & ~1);
}

bool VariableUnserializer::RefInfo::canBeReferenced() const {
  return !(m_data & 1);
}

VariableUnserializer::VariableUnserializer(
  const char* str,
  size_t len,
  Type type,
  bool allowUnknownSerializableClass,
  const Array& options)
    : m_type(type)
    , m_buf(str)
    , m_end(str + len)
    , m_unknownSerializable(allowUnknownSerializableClass)
    , m_options(options)
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
  } else if (mode == UnserializeMode::ColValue) {
    m_refs.emplace_back(RefInfo::makeNonRefable(v));
  } else {
    assert(mode == UnserializeMode::ColKey);
    // We don't currently support using the 'r' encoding to refer
    // to collection keys, but eventually we'll need to make this
    // work to allow objects as keys. For now we encode collections
    // keys in m_refs using a null pointer.
    m_refs.emplace_back(RefInfo(nullptr));
  }
}

Variant* VariableUnserializer::getByVal(int id) {
  if (id <= 0 || id > (int)m_refs.size()) return nullptr;
  Variant* ret = m_refs[id-1].var();
  if (!ret) throwColRKey();
  return ret;
}

Variant* VariableUnserializer::getByRef(int id) {
  if (id <= 0 || id > (int)m_refs.size()) return nullptr;
  if (!m_refs[id-1].canBeReferenced()) {
    throwColRefValue();
  }
  Variant* ret = m_refs[id-1].var();
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
  unserializeVariant(v, this);
  return v;
}

static std::pair<int64_t,const char*> hh_strtoll_base10(const char* p) {
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

static bool isWhitelistClass(const String& clsName, const Array& list) {
  if (!list.empty()) {
    for (ArrayIter iter(list); iter; ++iter) {
      auto val = iter.second().toString();
      if (val.get()->isame(clsName.get())) {
        return true;
      }
    }
  }
  return false;
}

const StaticString s_allowed_classes("allowed_classes");

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
      return isWhitelistClass(clsName, allowed_classes.toArray());
    }
    return allowed_classes.toBoolean();
  }

  if (!RuntimeOption::UnserializationWhitelistCheck) {
    // No need for BC HHVM-style whitelist check,
    // since the check isn't enabled.
    // Go with PHP5 default behavior of allowing all
    return true;
  }

  // Check for old-style whitelist
  if (isWhitelistClass(clsName, m_options)) {
    return true;
  }

  // Non-whitelisted class with a check enabled,
  // are we willing to hard-error over it?
  const char* err_msg =
    "The object being unserialized with class name '%s' "
    "is not in the given whitelist.";

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
static void unserializePropertyValue(Variant& v, VariableUnserializer* uns,
                                     int remainingProps) {
  assert(remainingProps > 0);
  unserializeVariant(v, uns);
  if (--remainingProps > 0) {
    auto lastChar = uns->peekBack();
    if (lastChar != ';' && lastChar != '}') {
      throwUnterminatedProperty();
    }
  }
}

// nProp should include the current property being unserialized.
NEVER_INLINE
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

  if (UNLIKELY(isRefcountedType(t->getRawType()))) {
    uns->putInOverwrittenList(*t);
  }

  unserializePropertyValue(*t, uns, nProp);
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
static void unserializeRemainingProps(VariableUnserializer* uns,
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
    unserializeVariant(v, uns, UnserializeMode::Key);
    String key = v.toString();
    int ksize = key.size();
    const char *kdata = key.data();
    int subLen = 0;
    if (key == s_serializedNativeDataKey) {
      unserializePropertyValue(serializedNativeData,
                               uns, remainingProps--);
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
      unserializeProp(uns, obj.get(), k, ctx, key,
                      remainingProps--);
    } else {
      unserializeProp(uns, obj.get(), key, nullptr, key,
                      remainingProps--);
    }
  }
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

NEVER_INLINE static
void unserializeVariant(Variant& self, VariableUnserializer* uns,
                        UnserializeMode mode /* = UnserializeMode::Value */) {

  // NOTE: If you make changes to how serialization and unserialization work,
  // make sure to update reserialize() here and test_apc_reserialize()
  // in "test/ext/test_ext_apc.cpp".

  char type = uns->readChar();
  char sep = uns->readChar();

  if (type != 'R') {
    uns->add(&self, mode);
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
      int64_t id = uns->readInt();
      Variant *v = uns->getByVal(id);
      if (!v) throwOutOfRange(id);
      Variant::AssignValHelper(&self, v);
    }
    break;
  case 'R':
    {
      int64_t id = uns->readInt();
      Variant *v = uns->getByRef(id);
      if (!v) throwOutOfRange(id);
      self.assignRefHelper(*v);
    }
    break;
  case 'b':
    {
      int64_t v = uns->readInt();
      tvSetBool((bool)v, *self.asTypedValue());
      break;
    }
  case 'i':
    {
      int64_t v = uns->readInt();
      tvSetInt(v, *self.asTypedValue());
      break;
    }
  case 'd':
    {
      char ch = uns->peek();
      bool negative = false;
      if (ch == '-') {
        negative = true;
        uns->readChar();
        ch = uns->peek();
      }
      double v;
      if (ch == 'I') {
        auto str = uns->readStr(3);
        if (str.size() != 3 || str[1] != 'N' || str[2] != 'F') {
          throwUnexpectedStr("INF", str);
        }
        v = std::numeric_limits<double>::infinity();
      } else if (ch == 'N') {
        auto str = uns->readStr(3);
        if (str.size() != 3 || str[1] != 'A' || str[2] != 'N') {
          throwUnexpectedStr("NAN", str);
        }
        v = std::numeric_limits<double>::quiet_NaN();
      } else {
        v = uns->readDouble();
      }
      tvSetDouble(negative ? -v : v, *self.asTypedValue());
    }
    break;
  case 's':
    {
      String v = unserializeString(uns);
      tvMove(make_tv<KindOfString>(v.detach()), *self.asTypedValue());
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
      auto str = uns->readStr(8);
      assert(str.size() == 8);
      auto sdp = reinterpret_cast<StringData*const*>(&str[0]);
      assert((*sdp)->isStatic());
      tvMove(make_tv<KindOfPersistentString>(*sdp), *self.asTypedValue());
    } else {
      throwUnknownType(type);
    }
    break;
  case 'a':
    {
      // Check stack depth to avoid overflow.
      check_recursion_throw();
      auto a = unserializeArray(uns);
      tvMove(make_tv<KindOfArray>(a.detach()), *self.asTypedValue());
    }
    return; // array has '}' terminating
  case 'L':
    {
      int64_t id = uns->readInt();
      uns->expectChar(':');
      String rsrcName = unserializeString(uns);
      uns->expectChar('{');
      uns->expectChar('}');
      auto rsrc = req::make<DummyResource>();
      rsrc->o_setResourceId(id);
      rsrc->m_class_name = rsrcName;
      tvMove(make_tv<KindOfResource>(rsrc.detach()->hdr()),
             *self.asTypedValue());
    }
    return; // resource has '}' terminating
  case 'O':
  case 'V':
  case 'K':
    {
      String clsName = unserializeString(uns);

      uns->expectChar(':');
      const int64_t size = uns->readInt();
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

      if ((type == 'O') && !uns->whitelistCheck(clsName)) {
        cls = nullptr;
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
          obj = Object{cls};
          if (UNLIKELY(collections::isType(cls, CollectionType::Pair) &&
                       (size != 2))) {
            throwInvalidPair();
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
              if (!uns->matchString(prop.mangledName->slice())) {
                mismatch = true;
                break;
              }
              // don't need to worry about overwritten list, because
              // this is definitely the first time we're setting this
              // property.
              TypedValue* tv = objProps++;
              Variant& t = tvAsVariant(tv);
              unserializePropertyValue(t, uns, remainingProps--);

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
                unserializeVariant(v, uns, UnserializeMode::Key);
                String key = v.toString();
                if (key == s_serializedNativeDataKey) {
                  unserializePropertyValue(serializedNativeData,
                                           uns, remainingProps--);
                  hasSerializedNativeData = true;
                } else {
                  auto t = &arr.lvalAt(key, AccessFlags::Key);
                  if (UNLIKELY(isRefcountedType(t->getRawType()))) {
                    uns->putInOverwrittenList(*t);
                  }
                  unserializePropertyValue(*t, uns, remainingProps--);
                }
              }
            }
          }
          if (remainingProps > 0) {
            INC_TPC(unser_prop_slow);
            unserializeRemainingProps(uns, obj, remainingProps,
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
          unserializeCollection(obj.get(), uns, size, type);
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
      String clsName = unserializeString(uns);

      uns->expectChar(':');
      String serialized = unserializeString(uns, '{', '}');

      auto obj = [&]() -> Object {
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

      tvMove(make_tv<KindOfObject>(obj.detach()), *self.asTypedValue());
    }
    return; // object has '}' terminating
  default:
    throwUnknownType(type);
  }
  uns->expectChar(';');
}

Array unserializeArray(VariableUnserializer* uns) {
  int64_t size = uns->readInt();
  uns->expectChar(':');
  uns->expectChar('{');
  if (size == 0) {
    uns->expectChar('}');
    return Array::Create(); // static empty array
  }
  if (UNLIKELY(size < 0 || size > std::numeric_limits<int>::max())) {
    throwArraySizeOutOfBounds();
  }
  auto const scale = computeScaleFromSize(size);
  auto const allocsz = computeAllocBytes(scale);

  // For large arrays, do a naive pre-check for OOM.
  if (UNLIKELY(allocsz > kMaxSmallSize && MM().preAllocOOM(allocsz))) {
    check_request_surprise_unlikely();
  }

  // Pre-allocate an ArrayData of the given size, to avoid escalation in the
  // middle, which breaks references.
  Array arr = ArrayInit(size, ArrayInit::Mixed{}).toArray();
  for (int64_t i = 0; i < size; i++) {
    Variant key;
    unserializeVariant(key, uns, UnserializeMode::Key);
    if (!key.isString() && !key.isInteger()) {
      throwInvalidKey();
    }
    // for apc, we know the key can't exist, but ignore that optimization
    assert(uns->type() != VariableUnserializer::Type::APCSerialize ||
           !arr.exists(key, true));

    Variant& value = arr.lvalAt(key, AccessFlags::Key);
    if (UNLIKELY(isRefcountedType(value.getRawType()))) {
      uns->putInOverwrittenList(value);
    }
    unserializeVariant(value, uns);

    if (i < (size - 1)) {
      auto lastChar = uns->peekBack();
      if ((lastChar != ';' && lastChar != '}')) {
        throwUnterminatedElement();
      }
    }
  }
  check_request_surprise_unlikely();
  uns->expectChar('}');
  return arr;
}

static
String unserializeString(VariableUnserializer* uns, char delimiter0 /* = '"' */,
                         char delimiter1 /* = '"' */) {
  int64_t size = uns->readInt();
  if (size >= RuntimeOption::MaxSerializedStringSize) {
    throwLargeStringSize(size);
  }
  if (size < 0) {
    throwNegativeStringSize(size);
  }
  uns->expectChar(':');
  uns->expectChar(delimiter0);
  auto r = uns->readStr(size);
  auto str = String::attach(StringData::Make(r, CopyString));
  uns->expectChar(delimiter1);
  return str;
}

static
void unserializeCollection(ObjectData* obj, VariableUnserializer* uns,
                           int64_t sz, char type) {
  switch (obj->collectionType()) {
    case CollectionType::Pair:
      unserializePair(obj, uns, sz, type);
      break;
    case CollectionType::Vector:
    case CollectionType::ImmVector:
      unserializeVector(obj, uns, sz, type);
      break;
    case CollectionType::Map:
    case CollectionType::ImmMap:
      unserializeMap(obj, uns, sz, type);
      break;
    case CollectionType::Set:
    case CollectionType::ImmSet:
      unserializeSet(obj, uns, sz, type);
      break;
  }
}

static
void unserializeVector(ObjectData* obj, VariableUnserializer* uns,
                       int64_t sz, char type) {
  if (type != 'V') throwBadFormat(obj, type);
  auto bvec = static_cast<BaseVector*>(obj);
  bvec->reserve(sz);
  assert(bvec->canMutateBuffer());
  for (int64_t i = 0; i < sz; ++i) {
    auto tv = bvec->appendForUnserialize(i);
    tv->m_type = KindOfNull;
    unserializeVariant(tvAsVariant(tv), uns, UnserializeMode::ColValue);
  }
}

static
void unserializeMap(ObjectData* obj, VariableUnserializer* uns,
                    int64_t sz, char type) {
  if (type != 'K') throwBadFormat(obj, type);
  auto map = static_cast<BaseMap*>(obj);
  map->reserve(sz);
  for (int64_t i = 0; i < sz; ++i) {
    Variant k;
    unserializeVariant(k, uns, UnserializeMode::ColKey);
    TypedValue* tv = nullptr;
    if (k.isInteger()) {
      auto h = k.toInt64();
      tv = map->findForUnserialize(h);
      // Be robust against manually crafted inputs with conflicting elements
      if (UNLIKELY(!tv)) goto do_unserialize;
    } else if (k.isString()) {
      auto key = k.getStringData();
      tv = map->findForUnserialize(key);
      // Be robust against manually crafted inputs with conflicting elements
      if (UNLIKELY(!tv)) goto do_unserialize;
    } else {
      throwInvalidKey();
    }
    tv->m_type = KindOfNull;
do_unserialize:
    unserializeVariant(tvAsVariant(tv), uns, UnserializeMode::ColValue);
  }
}

static
void unserializeSet(ObjectData* obj, VariableUnserializer* uns, int64_t sz,
                    char type) {
  if (type != 'V') throwBadFormat(obj, type);
  auto set = static_cast<BaseSet*>(obj);
  set->reserve(sz);
  for (int64_t i = 0; i < sz; ++i) {
    // When unserializing an element of a Set, we use Mode::ColKey for now.
    // This will make the unserializer to reserve an id for the element
    // but won't allow referencing the element via 'r' or 'R'.
    Variant k;
    unserializeVariant(k, uns, UnserializeMode::ColKey);
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

static
void unserializePair(ObjectData* obj, VariableUnserializer* uns,
                     int64_t sz, char type) {
  assert(sz == 2);
  if (type != 'V') throwBadFormat(obj, type);
  auto pair = static_cast<c_Pair*>(obj);
  auto elms = pair->initForUnserialize();
  unserializeVariant(tvAsVariant(&elms[0]), uns, UnserializeMode::ColValue);
  unserializeVariant(tvAsVariant(&elms[1]), uns, UnserializeMode::ColValue);
}

void reserialize(VariableUnserializer* uns, StringBuffer& buf) {
  char type = uns->readChar();
  char sep = uns->readChar();

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
      while (uns->peek() != ';') {
        char ch;
        ch = uns->readChar();
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
      auto str = uns->readStr(8);
      buf.append(str.data(), str.size());
    }
    break;
  case 's':
    {
      String v = unserializeString(uns);
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
      sep = uns->readChar();
      return;
    }
    break;
  case 'a':
    {
      buf.append("a:");
      int64_t size = uns->readInt();
      char sep2 = uns->readChar();
      buf.append(size);
      buf.append(sep2);
      sep2 = uns->readChar();
      buf.append(sep2);
      for (int64_t i = 0; i < size; i++) {
        reserialize(uns, buf); // key
        reserialize(uns, buf); // value
      }
      sep2 = uns->readChar(); // '}'
      buf.append(sep2);
      return;
    }
    break;
  case 'o':
  case 'O':
  case 'V':
  case 'K':
    {
      buf.append(type);
      buf.append(sep);

      String clsName = unserializeString(uns);
      buf.append(clsName.size());
      buf.append(":\"");
      buf.append(clsName.data(), clsName.size());
      buf.append("\":");

      uns->readChar();
      int64_t size = uns->readInt();
      char sep2 = uns->readChar();

      buf.append(size);
      buf.append(sep2);
      sep2 = uns->readChar(); // '{'
      buf.append(sep2);
      // 'V' type is a series with values only, while all other
      // types are series with keys and values
      int64_t i = type == 'V' ? size : size * 2;
      while (i--) {
        reserialize(uns, buf);
      }
      sep2 = uns->readChar(); // '}'
      buf.append(sep2);
      return;
    }
    break;
  case 'C':
    {
      buf.append(type);
      buf.append(sep);

      String clsName = unserializeString(uns);
      buf.append(clsName.size());
      buf.append(":\"");
      buf.append(clsName.data(), clsName.size());
      buf.append("\":");

      sep = uns->readChar(); // ':'
      String serialized = unserializeString(uns, '{', '}');
      buf.append(serialized.size());
      buf.append(":{");
      buf.append(serialized.data(), serialized.size());
      buf.append('}');
      return;
    }
    break;
  default:
    throwUnknownType(type);
  }

  sep = uns->readChar(); // the last ';'
  buf.append(sep);
}

}
