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

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/shared-variant.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/util/alloc.h"
#include <math.h>
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-strtod.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "tbb/concurrent_hash_map.h"
#include "hphp/util/stacktrace-profiler.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

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

// The uint32_t is used to hold TargetCache offsets for constants
typedef folly::AtomicHashMap<StrInternKey,uint32_t,strintern_hash,strintern_eq>
        StringDataMap;
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

//////////////////////////////////////////////////////////////////////

NEVER_INLINE void throw_string_too_large(uint32_t len) ATTRIBUTE_COLD;
NEVER_INLINE void throw_string_too_large(uint32_t len) {
  throw InvalidArgumentException("len > 2^31-2", len);
}

NEVER_INLINE void throw_string_too_large2(size_t len) ATTRIBUTE_COLD;
NEVER_INLINE void throw_string_too_large2(size_t len) {
  throw FatalErrorException(0, "String length exceeded 2^31-2: %zu", len);
}

ALWAYS_INLINE
std::pair<StringData*,uint32_t> allocFlatForLen(uint32_t len) {
  auto const needed = static_cast<uint32_t>(sizeof(StringData) + len + 1);
  if (LIKELY(needed <= MemoryManager::kMaxSmartSize)) {
    auto const cap = MemoryManager::smartSizeClass(needed);
    auto const sd  = static_cast<StringData*>(MM().smartMallocSize(cap));
    return std::make_pair(sd, cap);
  }

  if (UNLIKELY(needed > StringData::MaxSize + sizeof(StringData) + 1)) {
    throw_string_too_large(len);
  }

  auto const cap = needed;
  auto const sd = static_cast<StringData*>(MM().smartMallocSizeBig(cap));
  return std::make_pair(sd, cap);
}

ALWAYS_INLINE
void freeForSize(void* vp, uint32_t size) {
  if (LIKELY(size <= MemoryManager::kMaxSmartSize)) {
    return MM().smartFreeSize(vp, size);
  }
  return MM().smartFreeSizeBig(vp, size);
}

//////////////////////////////////////////////////////////////////////

}

size_t StringData::GetStaticStringCount() {
  if (!s_stringDataMap) return 0;
  return s_stringDataMap->size();
}

StringData* StringData::InsertStaticString(StringSlice slice) {
  auto const sd = MakeLowMalloced(slice);
  sd->setStatic();
  auto pair = s_stringDataMap->insert(make_intern_key(sd), 0);
  if (!pair.second) {
    sd->destructLowMalloc();
  }
  assert(to_sdata(pair.first->first) != nullptr);
  return const_cast<StringData*>(to_sdata(pair.first->first));
}

StringData* StringData::GetStaticString(const StringData* str) {
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
  return InsertStaticString(str->slice());
}

StringData* StringData::GetStaticString(StringSlice slice) {
  if (UNLIKELY(!s_stringDataMap)) {
    create_string_data_map();
  }
  auto const it = s_stringDataMap->find(make_intern_key(&slice));
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(to_sdata(it->first));
  }
  return InsertStaticString(slice);
}

StringData* StringData::LookupStaticString(const StringData *str) {
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

StringData* StringData::GetStaticString(const String& str) {
  assert(!str.isNull());
  return GetStaticString(str.get());
}

StringData* StringData::GetStaticString(const char* str, size_t len) {
  assert(len <= MaxSize);
  return GetStaticString(StringSlice{str, static_cast<uint32_t>(len)});
}

StringData* StringData::GetStaticString(const std::string& str) {
  assert(str.size() <= MaxSize);
  return GetStaticString(
    StringSlice{str.c_str(), static_cast<uint32_t>(str.size())}
  );
}

StringData* StringData::GetStaticString(const char* str) {
  return GetStaticString(str, strlen(str));
}

uint32_t StringData::GetCnsHandle(const StringData* cnsName) {
  assert(s_stringDataMap);
  auto const it = s_stringDataMap->find(make_intern_key(cnsName));
  if (it != s_stringDataMap->end()) {
    return it->second;
  }
  return 0;
}

uint32_t StringData::DefCnsHandle(const StringData* cnsName, bool persistent) {
  uint32_t val = GetCnsHandle(cnsName);
  if (val) return val;
  if (!cnsName->isStatic()) {
    // Its a dynamic constant, that doesn't correspond to
    // an already allocated handle. We'll allocate it in
    // the request local TargetCache::s_constants instead.
    return 0;
  }
  auto const it = s_stringDataMap->find(make_intern_key(cnsName));
  assert(it != s_stringDataMap->end());
  if (!it->second) {
    Transl::TargetCache::allocConstant(&it->second, persistent);
  }
  return it->second;
}

Array StringData::GetConstants() {
  // Return an array of all defined constants.
  assert(s_stringDataMap);
  Array a(Transl::TargetCache::s_constants);

  for (StringDataMap::const_iterator it = s_stringDataMap->begin();
       it != s_stringDataMap->end(); ++it) {
    if (it->second) {
      auto& tv =
        Transl::TargetCache::handleToRef<TypedValue>(it->second);
      if (tv.m_type != KindOfUninit) {
        StrNR key(const_cast<StringData*>(to_sdata(it->first)));
        a.set(key, tvAsVariant(&tv), true);
      } else if (tv.m_data.pref) {
        StrNR key(const_cast<StringData*>(to_sdata(it->first)));
        ClassInfo::ConstantInfo* ci =
          (ClassInfo::ConstantInfo*)(void*)tv.m_data.pref;
        a.set(key, ci->getDeferredValue(), true);
      }
    }
  }

  return a;
}

StringData* StringData::MakeLowMalloced(StringSlice sl) {
  if (UNLIKELY(sl.len > MaxSize)) {
    throw_string_too_large(sl.len);
  }

  auto const sd = static_cast<StringData*>(
    Util::low_malloc(sizeof(StringData) + sl.len + 1)
  );

  sd->m_data        = reinterpret_cast<char*>(sd + 1);
  sd->m_lenAndCount = sl.len;
  sd->m_capAndHash  = sl.len + 1;

  sd->m_data[sl.len] = 0;
  auto const mcret = memcpy(sd->m_data, sl.ptr, sl.len);
  auto const ret   = reinterpret_cast<StringData*>(mcret) - 1;

  assert(ret == sd);
  assert(ret->m_hash == 0);
  assert(ret->m_count == 0);
  assert(ret->isFlat());
  assert(ret->checkSane());
  return ret;
}

void StringData::destructLowMalloc() {
  assert(checkSane());
  assert(isFlat());
  Util::low_free(this);
}

//////////////////////////////////////////////////////////////////////

inline void StringData::enlist() {
  assert(isShared());
  auto& head = MemoryManager::TheMemoryManager()->m_strings;
  // insert after head
  auto const next = head.next;
  auto& payload = *sharedPayload();
  assert(uintptr_t(next) != kMallocFreeWord);
  payload.node.next = next;
  payload.node.prev = &head;
  next->prev = head.next = &payload.node;
}

inline void StringData::delist() {
  assert(isShared());
  auto& payload = *sharedPayload();
  auto const next = payload.node.next;
  auto const prev = payload.node.prev;
  assert(uintptr_t(next) != kMallocFreeWord);
  assert(uintptr_t(prev) != kMallocFreeWord);
  next->prev = prev;
  prev->next = next;
}

void StringData::sweepAll() {
  auto& head = MemoryManager::TheMemoryManager()->m_strings;
  for (SweepNode *next, *n = head.next; n != &head; n = next) {
    next = n->next;
    assert(next && uintptr_t(next) != kSmartFreeWord);
    assert(next && uintptr_t(next) != kMallocFreeWord);
    auto const s = reinterpret_cast<StringData*>(
      uintptr_t(n) - offsetof(SharedPayload, node)
                   - sizeof(StringData)
    );
    assert(s->isShared());
    s->sharedPayload()->shared->decRef();
  }
  head.next = head.prev = &head;
}

//////////////////////////////////////////////////////////////////////

HOT_FUNC
StringData* StringData::Make(StringSlice sl, CopyStringMode) {
  auto const allocRet = allocFlatForLen(sl.len);
  auto const sd       = allocRet.first;
  auto const cap      = allocRet.second;

  sd->m_data         = reinterpret_cast<char*>(sd + 1);
  sd->m_lenAndCount  = sl.len;
  sd->m_capAndHash   = cap - sizeof(StringData);

  sd->m_data[sl.len] = 0;
  auto const mcret = memcpy(sd->m_data, sl.ptr, sl.len);
  auto const ret   = reinterpret_cast<StringData*>(mcret) - 1;

  assert(ret == sd);
  assert(ret->m_hash == 0);
  assert(ret->m_count == 0);
  assert(ret->isFlat());
  assert(ret->checkSane());
  return ret;
}

HOT_FUNC
StringData* StringData::Make(const char* data, CopyStringMode) {
  return Make(StringSlice(data, strlen(data)), CopyString);
}

HOT_FUNC
StringData* StringData::MakeMalloced(const char* data, int len) {
  if (UNLIKELY(uint32_t(len) > MaxSize)) {
    throw_string_too_large(len);
  }

  auto const cap = static_cast<uint32_t>(len) + 1;
  auto const sd = static_cast<StringData*>(
    std::malloc(sizeof(StringData) + cap)
  );

  sd->m_lenAndCount = len;
  sd->m_capAndHash  = cap;
  sd->m_data        = reinterpret_cast<char*>(sd + 1);

  sd->m_data[len] = 0;
  auto const mcret = memcpy(sd->m_data, data, len);
  auto const ret   = reinterpret_cast<StringData*>(mcret) - 1;

  assert(ret == sd);
  assert(ret->m_hash == 0);
  assert(ret->m_count == 0);
  assert(ret->isFlat());
  assert(ret->checkSane());
  return ret;
}

HOT_FUNC
StringData* StringData::Make(SharedVariant* shared) {
  assert(shared && size_t(shared->stringLength()) <= size_t(MaxSize));

  auto const len = shared->stringLength();
  auto const data = shared->stringData();
  if (LIKELY(len <= SmallStringReserve)) {
    return Make(StringSlice(data, len), CopyString);
  }

  auto const sd = static_cast<StringData*>(
    MM().smartMallocSize(sizeof(StringData) + sizeof(SharedPayload))
  );

  sd->m_data        = const_cast<char*>(data);
  sd->m_lenAndCount = len;
  sd->m_capAndHash  = 0;   // cap == 0 means Shared.

  sd->sharedPayload()->shared = shared;
  sd->enlist();
  shared->incRef();

  assert(sd->checkSane());
  return sd;
}

HOT_FUNC
StringData* StringData::Make(StringSlice r1, StringSlice r2) {
  auto const len      = r1.len + r2.len;
  auto const allocRet = allocFlatForLen(len);
  auto const sd       = allocRet.first;
  auto const cap      = allocRet.second;
  auto const data     = reinterpret_cast<char*>(sd + 1);

  sd->m_data        = data;
  sd->m_lenAndCount = len;
  sd->m_capAndHash  = cap - sizeof(StringData);

  memcpy(data, r1.ptr, r1.len);
  memcpy(data + r1.len, r2.ptr, r2.len);
  data[len] = 0;

  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

HOT_FUNC
StringData* StringData::Make(StringSlice s1, const char* lit2) {
  return Make(s1, StringSlice(lit2, strlen(lit2)));
}

HOT_FUNC NEVER_INLINE
void StringData::releaseDataSlowPath() {
  assert(!isFlat());
  assert(isShared());
  assert(checkSane());

  sharedPayload()->shared->decRef();
  delist();
  freeForSize(this, sizeof(StringData) + sizeof(SharedPayload));
}

HOT_FUNC
void StringData::release() {
  assert(checkSane());

  if (UNLIKELY(!isFlat())) {
    return releaseDataSlowPath();
  }
  freeForSize(this, sizeof(StringData) + m_cap);
}

HOT_FUNC
StringData* StringData::Make(int reserveLen) {
  auto const allocRet = allocFlatForLen(reserveLen);
  auto const sd       = allocRet.first;
  auto const cap      = allocRet.second;
  auto const data     = reinterpret_cast<char*>(sd + 1);

  data[0] = 0;

  sd->m_data        = data;
  sd->m_lenAndCount = 0;
  sd->m_capAndHash  = cap - sizeof(StringData);

  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::append(StringSlice range) {
  assert(!isStatic() && getCount() <= 1);

  auto s = range.ptr;
  auto const len = range.len;

  if (len == 0) return this;
  if (UNLIKELY(uint32_t(len) > MaxSize)) {
    throw_string_too_large(len);
  }
  if (UNLIKELY(size_t(m_len) + size_t(len) > MaxSize)) {
    throw_string_too_large2(size_t(len) + size_t(m_len));
  }

  auto const newLen = m_len + len;

  /*
   * We may have an aliasing append.  We don't allow appending with an
   * interior pointer, although we may be asked to append less than
   * the whole string in an aliasing situation.
   */
  assert(uintptr_t(s) <= uintptr_t(rawdata()) ||
         uintptr_t(s) >= uintptr_t(rawdata() + capacity()));
  assert(s != rawdata() || len <= m_len);

  auto const target = UNLIKELY(isShared()) ? escalate(newLen)
                                           : reserve(newLen);
  auto const mslice = target->mutableSlice();

  /*
   * memcpy is safe even if it's a self append---the regions will be
   * disjoint, since s can't point past the start of our source
   * pointer, and len is smaller than the old length.
   */
  memcpy(mslice.ptr + m_len, s, len);

  target->setSize(newLen);
  assert(target->checkSane());

  return target;
}

StringData* StringData::reserve(int cap) {
  assert(!isImmutable() && m_count <= 1 && cap >= 0);
  assert(isFlat());

  if (cap + 1 <= capacity()) return this;

  cap += cap >> 2;
  if (cap > MaxCap) cap = MaxCap;
  auto const sd = Make(cap);
  auto const src = slice();
  auto const dst = sd->mutableData();
  sd->setSize(src.len);
  auto const mcret = memcpy(dst, src.ptr, src.len);
  auto const ret = static_cast<StringData*>(mcret) - 1;
  assert(ret == sd);
  assert(ret->checkSane());
  return ret;
}

StringData* StringData::copy(bool sharedMemory /* = false */) const {
  if (isStatic()) {
    // Static strings cannot change, and are always available.
    return const_cast<StringData*>(this);
  }
  if (sharedMemory) {
    // Even if it's literal, it might come from hphpi's class info
    // which will be freed at the end of the request, and so must be
    // copied.
    return StringData::MakeMalloced(data(), size());
  }
  return StringData::Make(slice(), CopyString);
}

// State transition from Mode::Shared to Mode::Flat.
StringData* StringData::escalate(uint32_t cap) {
  assert(isShared() && !isStatic() && cap >= m_len);

  auto const sd = Make(cap);
  auto const src = slice();
  auto const dst = sd->mutableData();
  sd->setSize(src.len);
  auto const mcret = memcpy(dst, src.ptr, src.len);
  auto const ret = static_cast<StringData*>(mcret) - 1;
  assert(ret == sd);
  assert(ret->checkSane());
  return ret;
}

void StringData::dump() const {
  StringSlice s = slice();

  printf("StringData(%d) (%s%s%d): [", m_count,
         isShared() ? "shared " : "",
         isStatic() ? "static " : "",
         s.len);
  for (uint32_t i = 0; i < s.len; i++) {
    char ch = s.ptr[i];
    if (isprint(ch)) {
      std::cout << ch;
    } else {
      printf("\\x%02x", ch);
    }
  }
  printf("]\n");
}

static StringData** precompute_chars() ATTRIBUTE_COLD;
static StringData** precompute_chars() {
  StringData** raw = new StringData*[256];
  for (int i = 0; i < 256; i++) {
    char s[2] = { (char)i, 0 };
    raw[i] = StringData::GetStaticString(&s[0], 1);
  }
  return raw;
}

static StringData** precomputed_chars = precompute_chars();

HOT_FUNC
StringData* StringData::GetStaticString(char c) {
  return precomputed_chars[(uint8_t)c];
}

HOT_FUNC
StringData *StringData::getChar(int offset) const {
  if (offset >= 0 && offset < size()) {
    return GetStaticString(m_data[offset]);
  }
  raise_notice("Uninitialized string offset: %d", offset);
  return GetStaticString("");
}

StringData* StringData::increment() {
  assert(!isStatic());
  assert(!empty());

  auto const sd = UNLIKELY(isShared())
    ? escalate(m_len + 1)
    : reserve(m_len + 1);
  sd->incrementHelper();
  return sd;
}

void StringData::incrementHelper() {
  m_hash = 0;

  enum class CharKind {
    UNKNOWN,
    LOWER_CASE,
    UPPER_CASE,
    NUMERIC
  };

  auto const len = m_len;
  auto const s = m_data;
  int carry = 0;
  int pos = len - 1;
  auto last = CharKind::UNKNOWN; // Shut up the compiler warning
  int ch;

  while (pos >= 0) {
    ch = s[pos];
    if (ch >= 'a' && ch <= 'z') {
      if (ch == 'z') {
        s[pos] = 'a';
        carry=1;
      } else {
        s[pos]++;
        carry=0;
      }
      last = CharKind::LOWER_CASE;
    } else if (ch >= 'A' && ch <= 'Z') {
      if (ch == 'Z') {
        s[pos] = 'A';
        carry=1;
      } else {
        s[pos]++;
        carry=0;
      }
      last = CharKind::UPPER_CASE;
    } else if (ch >= '0' && ch <= '9') {
      if (ch == '9') {
        s[pos] = '0';
        carry=1;
      } else {
        s[pos]++;
        carry=0;
      }
      last = CharKind::NUMERIC;
    } else {
      carry=0;
      break;
    }
    if (carry == 0) {
      break;
    }
    pos--;
  }

  if (carry) {
    if (UNLIKELY(len + 1 > MaxSize)) {
      throw_string_too_large(len);
    }

    assert(len + 2 <= capacity());
    memmove(s + 1, s, len);
    s[len + 1] = '\0';
    m_len = len + 1;

    switch (last) {
    case CharKind::NUMERIC:
      s[0] = '1';
      break;
    case CharKind::UPPER_CASE:
      s[0] = 'A';
      break;
    case CharKind::LOWER_CASE:
      s[0] = 'a';
      break;
    default:
      break;
    }
  }
}

void StringData::preCompute() const {
  assert(!isShared()); // because we are gonna reuse the space!
  StringSlice s = slice();
  m_hash = hash_string(s.ptr, s.len);
  assert(m_hash >= 0);
  int64_t lval; double dval;
  if (isNumericWithVal(lval, dval, 1) == KindOfNull) {
    m_hash |= STRHASH_MSB;
  }
}

void StringData::setStatic() const {
  m_count = RefCountStaticValue;
  preCompute();
}

///////////////////////////////////////////////////////////////////////////////
// type conversions

DataType StringData::isNumericWithVal(int64_t &lval, double &dval,
                                      int allow_errors) const {
  if (m_hash < 0) return KindOfNull;
  DataType ret = KindOfNull;
  StringSlice s = slice();
  if (s.len) {
    ret = is_numeric_string(s.ptr, s.len, &lval, &dval, allow_errors);
    if (ret == KindOfNull && !isShared() && allow_errors) {
      m_hash |= STRHASH_MSB;
    }
  }
  return ret;
}

bool StringData::isNumeric() const {
  if (m_hash < 0) return false;
  int64_t lval; double dval;
  DataType ret = isNumericWithVal(lval, dval, 0);
  switch (ret) {
  case KindOfNull:   return false;
  case KindOfInt64:
  case KindOfDouble: return true;
  default:
    assert(false);
    break;
  }
  return false;
}

bool StringData::isInteger() const {
  if (m_hash < 0) return false;
  int64_t lval; double dval;
  DataType ret = isNumericWithVal(lval, dval, 0);
  switch (ret) {
  case KindOfNull:   return false;
  case KindOfInt64:  return true;
  case KindOfDouble: return false;
  default:
    assert(false);
    break;
  }
  return false;
}

bool StringData::toBoolean() const {
  return !empty() && !isZero();
}

int64_t StringData::toInt64(int base /* = 10 */) const {
  return strtoll(rawdata(), nullptr, base);
}

double StringData::toDouble() const {
  StringSlice s = slice();
  if (s.len) return zend_strtod(s.ptr, nullptr);
  return 0;
}

DataType StringData::toNumeric(int64_t &lval, double &dval) const {
  if (m_hash < 0) return KindOfString;
  DataType ret = isNumericWithVal(lval, dval, 0);
  if (ret == KindOfInt64 || ret == KindOfDouble) return ret;
  return KindOfString;
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

HOT_FUNC
bool StringData::equal(const StringData *s) const {
  assert(s);
  if (s == this) return true;
  int ret;

  if (!(m_hash < 0 || s->m_hash < 0)) {
    ret = numericCompare(s);
    if (ret >= -1) {
      return ret == 0;
    }
  }
  return same(s);
}

HOT_FUNC
int StringData::numericCompare(const StringData *v2) const {
  assert(v2);

  int64_t lval1, lval2;
  double dval1, dval2;
  DataType ret1, ret2;
  if ((ret1 = isNumericWithVal(lval1, dval1, 0)) == KindOfNull ||
      (ret1 == KindOfDouble && !finite(dval1)) ||
      (ret2 = v2->isNumericWithVal(lval2, dval2, 0)) == KindOfNull ||
      (ret2 == KindOfDouble && !finite(dval2))) {
    return -2;
  }
  if (ret1 == KindOfInt64 && ret2 == KindOfInt64) {
    if (lval1 > lval2) return 1;
    if (lval1 == lval2) return 0;
    return -1;
  }
  if (ret1 == KindOfDouble && ret2 == KindOfDouble) {
    if (dval1 > dval2) return 1;
    if (dval1 == dval2) return 0;
    return -1;
  }
  if (ret1 == KindOfDouble) {
    assert(ret2 == KindOfInt64);
    dval2 = (double)lval2;
  } else {
    assert(ret1 == KindOfInt64);
    assert(ret2 == KindOfDouble);
    dval1 = (double)lval1;
  }

  if (dval1 > dval2) return 1;
  if (dval1 == dval2) return 0;
  return -1;
}

HOT_FUNC
int StringData::compare(const StringData *v2) const {
  assert(v2);

  if (v2 == this) return 0;

  int ret = numericCompare(v2);
  if (ret < -1) {
    int len1 = size();
    int len2 = v2->size();
    int len = len1 < len2 ? len1 : len2;
    ret = memcmp(rawdata(), v2->rawdata(), len);
    if (ret) return ret;
    if (len1 == len2) return 0;
    return len < len1 ? 1 : -1;
  }
  return ret;
}

HOT_FUNC
strhash_t StringData::hashHelper() const {
  strhash_t h = isShared() ? sharedPayload()->shared->stringHash()
                           : hash_string_inline(m_data, m_len);
  assert(h >= 0);
  m_hash |= h;
  return h;
}

///////////////////////////////////////////////////////////////////////////////
// Debug

std::string StringData::toCPPString() const {
  StringSlice s = slice();
  return std::string(s.ptr, s.len);
}

bool StringData::checkSane() const {
  static_assert(sizeof(StringData) == 24,
                "StringData size changed---update assertion if you mean it");
  static_assert(size_t(MaxSize) <= size_t(INT_MAX), "Beware int wraparound");
  static_assert(offsetof(StringData, m_count) == FAST_REFCOUNT_OFFSET,
                "m_count at wrong offset");
  static_assert(offsetof(StringSlice, ptr) == offsetof(StringData, m_data) &&
                offsetof(StringSlice, len) == offsetof(StringData, m_len),
                "StringSlice and StringData must have same pointer and size "
                "layout for the StaticString map");

  assert(uint32_t(size()) <= MaxSize);
  assert(uint32_t(capacity()) <= MaxCap);
  assert(size() >= 0);

  if (!isShared()) {
    assert(size() < capacity());
  } else {
    assert(capacity() == 0);
  }

  if (isFlat()) {
    assert(m_data == voidPayload());
  } else {
    assert(m_data && m_data != voidPayload());
  }

  return true;
}

//////////////////////////////////////////////////////////////////////

}
