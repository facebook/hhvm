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
#include "hphp/runtime/base/static-string-table.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/perf-warning.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/reverse-data-map.h"

#include "hphp/runtime/server/memory-stats.h"

#include "hphp/util/low-ptr.h"

#include <folly/AtomicHashMap.h>

#include <type_traits>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

StringData** precomputed_chars;

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
  assertx(!isMagicKey(key));
  static_assert(std::is_unsigned<StrInternKey>(), "cast must zero-extend");
  return reinterpret_cast<const StringData*>(key);
}

struct strintern_eq {
  bool operator()(StrInternKey k1, StrInternKey k2) const {
    assertx(!isMagicKey(k2)); // no magic values on rhs
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
    assertx(!isMagicKey(k)); // no magic values get here
    return operator()(to_sdata(k));
  }

  size_t operator()(const StringData* sd) const {
    return sd->hash();
  }

  size_t operator()(folly::StringPiece slice) const {
    return StringData::hash(slice.data(), slice.size());
  }
};

// The uint32_t is used to hold RDS offsets for constants
using StringDataMap = folly::AtomicHashMap<
  StrInternKey,
  rds::Link<TypedValue, rds::Mode::NonLocal>,
  strintern_hash,
  strintern_eq,
  VMAllocator<char>
>;

struct EmbeddedStringMap {
  explicit operator bool() const { return inited; }

  StringDataMap* operator->() {
    assertx(inited);
    return reinterpret_cast<StringDataMap*>(&data);
  }
  StringDataMap& operator*() { assertx(inited); return *operator->(); }

  void emplace(uint32_t size, const StringDataMap::Config& config) {
    assertx(!inited);
    new (&data) StringDataMap(size, config);
    inited = true;
  }

  void clear() {
    if (inited) {
      operator*().~StringDataMap();
      inited = false;
    }
  }

 private:
  typename std::aligned_storage<
    sizeof(StringDataMap),
    alignof(StringDataMap)
  >::type data;
  bool inited;
};

EmbeddedStringMap s_stringDataMap;

// If a string is static it better be the one in the table.
DEBUG_ONLY bool checkStaticStr(const StringData* s) {
  assertx(s->isStatic());
  assertx(s_stringDataMap);
  auto DEBUG_ONLY const it = s_stringDataMap->find(s);
  assertx(it != s_stringDataMap->end());
  assertx(to_sdata(it->first) == s);
  return true;
}

StringData** precompute_chars() {
  StringData** raw = new StringData*[256];
  for (int i = 0; i < 256; i++) {
    char s[2] = { (char)i, 0 };
    raw[i] = makeStaticString(&s[0], 1);
  }
  return raw;
}

}

ServiceData::ExportedCounter* ss_counter;

StringData* insertStaticString(StringData* sd,
                               void (*deleter)(StringData*)) {
  assertx(sd->isStatic());
  auto pair = s_stringDataMap->insert(
    safe_cast<StrInternKey>(reinterpret_cast<uintptr_t>(sd)),
    rds::Link<TypedValue, rds::Mode::NonLocal>{}
  );

  if (!pair.second) {
    if (deleter) {
      deleter(sd);
    } else {
      sd->destructStatic();
    }
  } else {
    auto const symbol = sd->isSymbol();
    auto const allocSize = sd->size() + kStringOverhead
                         + (symbol ? sizeof(SymbolPrefix) : 0);
    MemoryStats::LogAlloc(AllocKind::StaticString, allocSize);
    if (RuntimeOption::EvalEnableReverseDataMap) {
      data_map::register_start(sd);
    }
    static std::atomic<bool> signaled{false};
    checkAHMSubMaps(*s_stringDataMap, "static string table", signaled);
    ss_counter->increment();
  }
  assertx(to_sdata(pair.first->first) != nullptr);

  return const_cast<StringData*>(to_sdata(pair.first->first));
}

void create_string_data_map() {
  always_assert(!s_stringDataMap);
  ss_counter = ServiceData::createCounter("admin.static-strings");
  StringDataMap::Config config;
  config.growthFactor = 1;
  config.entryCountThreadCacheSize = 10;
  MemoryStats::ResetStaticStringSize();

  s_stringDataMap.emplace(RuntimeOption::EvalInitialStaticStringTableSize,
                          config);
  insertStaticString(StringData::MakeEmpty());
  if (!precomputed_chars) {
    precomputed_chars = precompute_chars();
  }
}

//////////////////////////////////////////////////////////////////////

size_t makeStaticStringCount() {
  if (!s_stringDataMap) return 0;
  return s_stringDataMap->size();
}

StringData* makeStaticString(const StringData* str) {
  if (str->isStatic()) {
    assertx(checkStaticStr(str));
    return const_cast<StringData*>(str);
  }
  auto const it = s_stringDataMap->find(str);
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(to_sdata(it->first));
  }
  return insertStaticString(StringData::MakeStatic(str->slice()));
}

StringData* makeStaticString(folly::StringPiece slice) {
  auto const it = s_stringDataMap->find(slice);
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(to_sdata(it->first));
  }
  return insertStaticString(StringData::MakeStatic(slice));
}

StringData* lookupStaticString(const StringData *str) {
  assertx(s_stringDataMap && !str->isStatic());
  auto const it = s_stringDataMap->find(str);
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(to_sdata(it->first));
  }
  return nullptr;
}

StringData* lookupStaticString(folly::StringPiece slice) {
  auto const it = s_stringDataMap->find(slice);
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(to_sdata(it->first));
  }
  return nullptr;
}

StringData* makeStaticString(const String& str) {
  assertx(!str.isNull());
  return makeStaticString(str.get());
}

StringData* makeStaticString(const char* str, size_t len) {
  assertx(len <= StringData::MaxSize);
  return makeStaticString(folly::StringPiece{str, len});
}

StringData* makeStaticString(const std::string& str) {
  assertx(str.size() <= StringData::MaxSize);
  return makeStaticString(folly::StringPiece{str.c_str(), str.size()});
}

StringData* makeStaticString(const char* str) {
  return makeStaticString(str, strlen(str));
}

StringData* makeStaticString(char c) {
  // TODO(#2880477): should this be inlined?
  return precomputed_chars[(uint8_t)c];
}


bool bindPersistentCns(const StringData* cnsName, const TypedValue& value) {
  assertx(s_stringDataMap);
  auto const it = s_stringDataMap->find(cnsName);
  assertx(it != s_stringDataMap->end());
  it->second.bind<kTVSimdAlign>(
    rds::Mode::Persistent,
    rds::LinkName{"Cns", cnsName},
    &value
  );
  return it->second.isPersistent();
}

rds::Handle lookupCnsHandle(const StringData* cnsName) {
  assertx(s_stringDataMap);
  auto const it = s_stringDataMap->find(cnsName);
  if (it != s_stringDataMap->end()) {
    return it->second.maybeHandle();
  }
  return rds::kUninitHandle;
}

rds::Handle makeCnsHandle(const StringData* cnsName) {
  assertx(cnsName->isStatic());
  auto const val = lookupCnsHandle(cnsName);
  if (rds::isHandleBound(val)) return val;
  auto const it = s_stringDataMap->find(cnsName);
  assertx(it != s_stringDataMap->end());
  if (!it->second.bound()) {
    it->second.bind<kTVSimdAlign>(
      rds::Mode::Normal,
      rds::LinkName{"Cns", cnsName}
    );
  }
  return it->second.handle();
}

std::vector<StringData*> lookupDefinedStaticStrings() {
  assertx(s_stringDataMap);
  std::vector<StringData*> ret;
  ret.reserve(s_stringDataMap->size());

  for (auto const& it : *s_stringDataMap) {
    ret.push_back(const_cast<StringData*>(to_sdata(it.first)));
  }

  return ret;
}


const StaticString s_user("user");
const StaticString s_Core("Core");

Array lookupDefinedConstants(bool categorize /*= false */) {
  assertx(s_stringDataMap);
  auto usr = Array::CreateDict();
  auto sys = categorize ? Array::CreateDict() : Array();

  for (auto const& it : *s_stringDataMap) {
    auto const& rval = it.second;
    if (!rval.bound() || !rval.isInit()) continue;

    auto* tbl = (categorize && rval.isPersistent()) ? &sys : &usr;
    auto const& tv = *rval;

    StrNR key{to_sdata(it.first)};
    if (type(tv) != KindOfUninit) {
      tbl->set(key, tv, true);
    } else {
      auto cns = Constant::get(key.get());
      assertx(cns.isAllowedAsConstantValue() == Variant::AllowedAsConstantValue::Allowed);
      tbl->set(key, cns, true);
    }
  }

  if (categorize) {
    return make_dict_array(
      s_user, usr,
      s_Core, sys
    );
  } else {
    return usr;
  }
}

size_t countStaticStringConstants() {
  if (!s_stringDataMap) return 0;
  size_t count = 0;

  for (auto const& it : *s_stringDataMap) {
    if (it.second.bound()) {
      ++count;
    }
  }
  return count;
}

void refineStaticStringTableSize() {
  if (RuntimeOption::EvalInitialStaticStringTableSize ==
      kDefaultInitialStaticStringTableSize ||
      !s_stringDataMap) {
    return;
  }

  std::vector<StringDataMap::value_type>
    oldStringTable(s_stringDataMap->begin(), s_stringDataMap->end());

  s_stringDataMap.clear();
  create_string_data_map();

  for (auto& kv : oldStringTable) {
    s_stringDataMap->insert(kv.first, kv.second);
  }
}

//////////////////////////////////////////////////////////////////////

}
