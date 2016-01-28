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
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/server/memory-stats.h"
#include "hphp/util/low-ptr.h"

#include <folly/AtomicHashMap.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

// the string key will one of these values:
//  * a valid LowPtr<StringData>, or
//  * -1, -2, or -3 AHM magic values.
// Note that only the magic values have 1s in the low 3 bits
// since StringData's are at least 8-aligned.

using StrInternKey = LowStringPtr::storage_type;

// Return true if k is one of AHM's magic values. Valid pointers are
// 8-aligned, so test the low 3 bits.
bool isMagicKey(StrInternKey k) {
  return (k & 7) != 0;
}

const StringData* to_sdata(StrInternKey key) {
  assert(!isMagicKey(key));
  static_assert(std::is_unsigned<StrInternKey>(), "cast must zero-extend");
  return reinterpret_cast<const StringData*>(key);
}

struct strintern_eq {
  bool operator()(StrInternKey k1, StrInternKey k2) const {
    assert(!isMagicKey(k2)); // no magic values on rhs
    return operator()(k1, to_sdata(k2));
  }

  bool operator()(StrInternKey k1, const StringData* string2) const {
    if (isMagicKey(k1)) return false; // magic values
    auto const sd1 = to_sdata(k1);
    auto const len1 = sd1->size();
    auto const data1 = sd1->data();
    if (len1 != string2->size()) return false;
    // only use wordsame on 8-byte aligned addresses
    return wordsame(data1, string2->data(), len1);
  }

  bool operator()(StrInternKey k1, folly::StringPiece slice2) const {
    if (isMagicKey(k1)) return false; // magic values
    auto const sd1 = to_sdata(k1);
    auto const len1 = sd1->size();
    auto const data1 = sd1->data();
    if (len1 != slice2.size()) return false;
    return !memcmp(data1, slice2.begin(), len1);
  }
};

struct strintern_hash {
  size_t operator()(StrInternKey k) const {
    assert(!isMagicKey(k)); // no magic values get here
    return operator()(to_sdata(k));
  }

  size_t operator()(const StringData* sd) const {
    return sd->hash();
  }

  size_t operator()(folly::StringPiece slice) const {
    return hash_string(slice.data(), slice.size());
  }
};

// The uint32_t is used to hold RDS offsets for constants
typedef folly::AtomicHashMap<
  StrInternKey,
  rds::Link<TypedValue>,
  strintern_hash,
  strintern_eq
> StringDataMap;
StringDataMap* s_stringDataMap;

// If a string is static it better be the one in the table.
DEBUG_ONLY bool checkStaticStr(const StringData* s) {
  assert(s->isStatic());
  assert(s_stringDataMap);
  auto DEBUG_ONLY const it = s_stringDataMap->find(s);
  assert(it != s_stringDataMap->end());
  assert(to_sdata(it->first) == s);
  return true;
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

StringData* insertStaticString(StringData* sd) {
  assert(sd->isStatic());
  auto pair = s_stringDataMap->insert(
    safe_cast<StrInternKey>(reinterpret_cast<uintptr_t>(sd)),
    rds::Link<TypedValue>(rds::kInvalidHandle)
  );

  if (!pair.second) {
    sd->destructStatic();
  } else {
    MemoryStats::GetInstance()->LogStaticStringAlloc(sd->size()
        + sizeof(StringData));
  }
  assert(to_sdata(pair.first->first) != nullptr);

  return const_cast<StringData*>(to_sdata(pair.first->first));
}

inline StringData* insertStaticStringSlice(folly::StringPiece slice) {
  return insertStaticString(StringData::MakeStatic(slice));
}

void create_string_data_map() {
  StringDataMap::Config config;
  config.growthFactor = 1;
  MemoryStats::GetInstance()->ResetStaticStringSize();

  s_stringDataMap =
    new StringDataMap(RuntimeOption::EvalInitialStaticStringTableSize,
                      config);
  insertStaticString(StringData::MakeEmpty());
}

}

//////////////////////////////////////////////////////////////////////

size_t makeStaticStringCount() {
  if (!s_stringDataMap) return 0;
  return s_stringDataMap->size();
}

StringData* makeStaticString(const StringData* str) {
  if (str->isStatic()) {
    assert(checkStaticStr(str));
    return const_cast<StringData*>(str);
  }
  if (UNLIKELY(!s_stringDataMap)) {
    create_string_data_map();
  }
  auto const it = s_stringDataMap->find(str);
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(to_sdata(it->first));
  }
  return insertStaticStringSlice(str->slice());
}

StringData* makeStaticString(folly::StringPiece slice) {
  if (UNLIKELY(!s_stringDataMap)) {
    create_string_data_map();
  }
  auto const it = s_stringDataMap->find(slice);
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(to_sdata(it->first));
  }
  return insertStaticStringSlice(slice);
}

StringData* lookupStaticString(const StringData *str) {
  assert(s_stringDataMap && !str->isStatic());
  auto const it = s_stringDataMap->find(str);
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
  return makeStaticString(folly::StringPiece{str, len});
}

StringData* makeStaticString(const std::string& str) {
  assert(str.size() <= StringData::MaxSize);
  return makeStaticString(folly::StringPiece{str.c_str(), str.size()});
}

StringData* makeStaticString(const char* str) {
  return makeStaticString(str, strlen(str));
}

StringData* makeStaticString(char c) {
  // TODO(#2880477): should this be inlined?
  return precomputed_chars[(uint8_t)c];
}

rds::Handle lookupCnsHandle(const StringData* cnsName) {
  assert(s_stringDataMap);
  auto const it = s_stringDataMap->find(cnsName);
  if (it != s_stringDataMap->end()) {
    return it->second.handle();
  }
  return 0;
}

rds::Handle makeCnsHandle(const StringData* cnsName, bool persistent) {
  auto const val = lookupCnsHandle(cnsName);
  if (val) return val;
  if (!cnsName->isStatic()) {
    // Its a dynamic constant, that doesn't correspond to
    // an already allocated handle. We'll allocate it in
    // the request local rds::s_constants instead.
    return 0;
  }
  auto const it = s_stringDataMap->find(cnsName);
  assert(it != s_stringDataMap->end());
  if (!it->second.bound()) {
    it->second.bind<kTVSimdAlign>(persistent ? rds::Mode::Persistent
                                             : rds::Mode::Normal);

    rds::recordRds(it->second.handle(), sizeof(TypedValue),
                   "Cns", cnsName->data());
  }
  return it->second.handle();
}

std::vector<StringData*> lookupDefinedStaticStrings() {
  assert(s_stringDataMap);
  std::vector<StringData*> ret;

  for (auto it = s_stringDataMap->begin();
       it != s_stringDataMap->end(); ++it) {
    ret.push_back(const_cast<StringData*>(to_sdata(it->first)));
  }

  return ret;
}


const StaticString s_user("user");
const StaticString s_Core("Core");

Array lookupDefinedConstants(bool categorize /*= false */) {
  assert(s_stringDataMap);
  Array usr(rds::s_constants());
  Array sys;

  for (auto it = s_stringDataMap->begin();
       it != s_stringDataMap->end(); ++it) {
    if (it->second.bound()) {
      Array *tbl = (categorize &&
                    rds::isPersistentHandle(it->second.handle()))
                 ? &sys : &usr;
      auto& tv = *it->second;
      if (tv.m_type != KindOfUninit) {
        StrNR key(const_cast<StringData*>(to_sdata(it->first)));
        tbl->set(key, tvAsVariant(&tv), true);
      } else if (tv.m_data.pref) {
        StrNR key(const_cast<StringData*>(to_sdata(it->first)));
        auto callback =
          reinterpret_cast<Unit::SystemConstantCallback>(tv.m_data.pref);
        auto cns = callback();
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

size_t countStaticStringConstants() {
  if (!s_stringDataMap) return 0;
  size_t count = 0;
  for (auto it = s_stringDataMap->begin();
       it != s_stringDataMap->end(); ++it) {
    if (it->second.bound()) {
      ++count;
    }
  }
  return count;
}

void refineStaticStringTableSize() {
  if (RuntimeOption::EvalInitialStaticStringTableSize ==
      kDefaultInitialStaticStringTableSize) {
    return;
  }
  auto oldStringTable = s_stringDataMap;
  if (!oldStringTable) return;

  s_stringDataMap = nullptr;
  create_string_data_map();
  SCOPE_EXIT { delete oldStringTable; };

  for (auto& kv : *oldStringTable) {
    s_stringDataMap->insert(kv.first, kv.second);
  }
}

//////////////////////////////////////////////////////////////////////

}
