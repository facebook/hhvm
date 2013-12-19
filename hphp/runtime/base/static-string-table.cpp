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
#include "hphp/runtime/base/static-string-table.h"

#include "folly/AtomicHashMap.h"

#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/rds.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

// Pointer to StringData, or pointer to StringSlice.
typedef intptr_t StrInternKey;

constexpr intptr_t kAhmMagicThreshold = -3;

StrInternKey make_intern_key(const StringData* sd) {
  auto const ret = reinterpret_cast<StrInternKey>(sd);
  assert(ret > 0);
  return ret;
}

StrInternKey make_intern_key(const StringSlice* sl) {
  auto const ret = -reinterpret_cast<StrInternKey>(sl);
  assert(ret < 0 && ret < kAhmMagicThreshold);
  return ret;
}

const StringData* to_sdata(StrInternKey key) {
  assert(key > 0);
  return reinterpret_cast<const StringData*>(key);
}

const StringSlice* to_sslice(StrInternKey key) {
  assert(key < 0 && key < kAhmMagicThreshold);
  return reinterpret_cast<const StringSlice*>(-key);
}

// To avoid extra instructions in strintern_eq, we currently are
// making use of the fact that StringSlice and StringData have the
// same initial layout.  See the static_asserts in checkSane.
const StringSlice* to_sslice_punned(StrInternKey key) {
  if (UNLIKELY(key < 0)) {
    return reinterpret_cast<const StringSlice*>(-key);
  }
  // Actually a StringData*, but same layout.
  return reinterpret_cast<const StringSlice*>(key);
}

struct strintern_eq {
  bool operator()(StrInternKey k1, StrInternKey k2) const {
    if (k1 < 0) {
      // AHM only gives lookup keys on the rhs of the equal operator
      assert(k1 >= kAhmMagicThreshold);
      return false;
    }
    assert(k2 >= 0 || k2 < kAhmMagicThreshold);
    auto const sd1 = to_sdata(k1);
    auto const s2 = to_sslice_punned(k2);
    return sd1->size() == s2->len &&
           wordsame(sd1->data(), s2->ptr, s2->len);
  }
};

struct strintern_hash {
  size_t operator()(StrInternKey k) const {
    assert(k > 0 || k < kAhmMagicThreshold);
    if (LIKELY(k > 0)) {
      return to_sdata(k)->hash();
    }
    auto const slice = *to_sslice(k);
    return hash_string_inline(slice.ptr, slice.len);
  }
};

// The uint32_t is used to hold RDS offsets for constants
typedef folly::AtomicHashMap<
  StrInternKey,
  RDS::Link<TypedValue>,
  strintern_hash,
  strintern_eq
> StringDataMap;
StringDataMap* s_stringDataMap;

// If a string is static it better be the one in the table.
DEBUG_ONLY bool checkStaticStr(const StringData* s) {
  assert(s->isStatic());
  auto DEBUG_ONLY const it = s_stringDataMap->find(make_intern_key(s));
  assert(it != s_stringDataMap->end());
  assert(to_sdata(it->first) == s);
  return true;
}

void create_string_data_map() {
  StringDataMap::Config config;
  config.growthFactor = 1;
  s_stringDataMap =
    new StringDataMap(RuntimeOption::EvalInitialStaticStringTableSize,
                      config);
}

StringData** precompute_chars();
StringData** precompute_chars() {
  StringData** raw = new StringData*[256];
  for (int i = 0; i < 256; i++) {
    char s[2] = { (char)i, 0 };
    raw[i] = makeStaticString(&s[0], 1);
  }
  return raw;
}

StringData** precomputed_chars = precompute_chars();

StringData* insertStaticString(StringSlice slice) {
  auto const sd = StringData::MakeStatic(slice);
  auto pair = s_stringDataMap->insert(
    make_intern_key(sd),
    RDS::Link<TypedValue>(RDS::kInvalidHandle)
  );
  if (!pair.second) {
    sd->destructStatic();
  }
  assert(to_sdata(pair.first->first) != nullptr);
  return const_cast<StringData*>(to_sdata(pair.first->first));
}

}

//////////////////////////////////////////////////////////////////////

size_t makeStaticStringCount() {
  if (!s_stringDataMap) return 0;
  return s_stringDataMap->size();
}

StringData* makeStaticString(const StringData* str) {
  if (UNLIKELY(!s_stringDataMap)) {
    create_string_data_map();
  }
  if (str->isStatic()) {
    assert(checkStaticStr(str));
    return const_cast<StringData*>(str);
  }
  auto const it = s_stringDataMap->find(make_intern_key(str));
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(to_sdata(it->first));
  }
  return insertStaticString(str->slice());
}

StringData* makeStaticString(StringSlice slice) {
  if (UNLIKELY(!s_stringDataMap)) {
    create_string_data_map();
  }
  auto const it = s_stringDataMap->find(make_intern_key(&slice));
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(to_sdata(it->first));
  }
  return insertStaticString(slice);
}

StringData* lookupStaticString(const StringData *str) {
  if (UNLIKELY(!s_stringDataMap)) return nullptr;
  if (str->isStatic()) {
    assert(checkStaticStr(str));
    return const_cast<StringData*>(str);
  }
  auto const it = s_stringDataMap->find(make_intern_key(str));
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(to_sdata(it->first));
  }
  return nullptr;
}

StringData* makeStaticString(const String& str) {
  assert(!str.isNull());
  return makeStaticString(str.get());
}

StringData* makeStaticString(const char* str, size_t len) {
  assert(len <= StringData::MaxSize);
  return makeStaticString(StringSlice{str, static_cast<uint32_t>(len)});
}

StringData* makeStaticString(const std::string& str) {
  assert(str.size() <= StringData::MaxSize);
  return makeStaticString(
    StringSlice{str.c_str(), static_cast<uint32_t>(str.size())}
  );
}

StringData* makeStaticString(const char* str) {
  return makeStaticString(str, strlen(str));
}

StringData* makeStaticString(char c) {
  // TODO(#2880477): should this be inlined?
  return precomputed_chars[(uint8_t)c];
}

RDS::Handle lookupCnsHandle(const StringData* cnsName) {
  assert(s_stringDataMap);
  auto const it = s_stringDataMap->find(make_intern_key(cnsName));
  if (it != s_stringDataMap->end()) {
    return it->second.handle();
  }
  return 0;
}

RDS::Handle makeCnsHandle(const StringData* cnsName, bool persistent) {
  auto const val = lookupCnsHandle(cnsName);
  if (val) return val;
  if (!cnsName->isStatic()) {
    // Its a dynamic constant, that doesn't correspond to
    // an already allocated handle. We'll allocate it in
    // the request local RDS::s_constants instead.
    return 0;
  }
  auto const it = s_stringDataMap->find(make_intern_key(cnsName));
  assert(it != s_stringDataMap->end());
  it->second.bind<kTVSimdAlign>(persistent ? RDS::Mode::Persistent
                                           : RDS::Mode::Normal);
  return it->second.handle();
}

const StaticString s_user("user");
const StaticString s_Core("Core");
Array lookupDefinedConstants(bool categorize /*= false */) {
  assert(s_stringDataMap);
  Array usr(RDS::s_constants());
  Array sys;

  for (StringDataMap::const_iterator it = s_stringDataMap->begin();
       it != s_stringDataMap->end(); ++it) {
    if (it->second.bound()) {
      Array *tbl = (categorize &&
                    RDS::isPersistentHandle(it->second.handle()))
                 ? &sys : &usr;
      auto& tv = *it->second;
      if (tv.m_type != KindOfUninit) {
        StrNR key(const_cast<StringData*>(to_sdata(it->first)));
        tbl->set(key, tvAsVariant(&tv), true);
      } else if (tv.m_data.pref) {
        StrNR key(const_cast<StringData*>(to_sdata(it->first)));
        ClassInfo::ConstantInfo* ci =
          (ClassInfo::ConstantInfo*)(void*)tv.m_data.pref;
        auto cns = ci->getDeferredValue();
        if (cns.isInitialized()) {
          tbl->set(key, cns, true);
        }
      }
    }
  }

  if (categorize) {
    Array ret;
    ret.set(s_user, usr);
    ret.set(s_Core, sys);
    return ret;
  } else {
    return usr;
  }
}

//////////////////////////////////////////////////////////////////////

}
