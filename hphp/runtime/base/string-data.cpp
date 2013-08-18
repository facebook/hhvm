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
#include "hphp/util/stacktrace_profiler.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

void init_stringdata_allocator() {
  StringData::Allocator allocator;
  StringData::Allocator::getCheck();
}

void StringData::release() {
  this->~StringData();
  Allocator::getNoCheck()->dealloc(this);
}

//////////////////////////////////////////////////////////////////////

// equality checker for AtomicHashMap
struct ahm_string_data_same {
  bool operator()(const StringData *s1, const StringData *s2) const {
    // ahm uses -1, -2, -3 as magic values
    return int64_t(s1) > 0 && s1->same(s2);
  }
};

// The uint32_t is used to hold TargetCache offsets for constants
typedef folly::AtomicHashMap<const StringData *, uint32_t,
                             string_data_hash,
                             ahm_string_data_same> StringDataMap;
static StringDataMap *s_stringDataMap;

// If a string is static it better be the one in the table.
#ifndef NDEBUG
static bool checkStaticStr(const StringData* s) {
  assert(s->isStatic());
  StringDataMap::const_iterator it = s_stringDataMap->find(s);
  assert(it != s_stringDataMap->end());
  assert(it->first == s);
  return true;
}
#endif

size_t StringData::GetStaticStringCount() {
  if (!s_stringDataMap) return 0;
  return s_stringDataMap->size();
}

StringData *StringData::GetStaticString(const StringData *str) {
  if (UNLIKELY(!s_stringDataMap)) {
    StringDataMap::Config config;
    config.growthFactor = 1;
    s_stringDataMap =
      new StringDataMap(RuntimeOption::EvalInitialStaticStringTableSize,
                        config);
  }
  if (str->isStatic()) {
    assert(checkStaticStr(str));
    return const_cast<StringData*>(str);
  }
  StringDataMap::const_iterator it = s_stringDataMap->find(str);
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(it->first);
  }
  // Lookup failed, so do the hard work of creating a StringData with its own
  // copy of the key string, so that the atomic insert() has a permanent key.
  StringData *sd = (StringData*)Util::low_malloc(sizeof(StringData));
  new (sd) StringData(str->data(), str->size(), CopyMalloc);
  sd->setStatic();
  auto pair = s_stringDataMap->insert(sd, 0);
  if (!pair.second) {
    sd->~StringData();
    Util::low_free(sd);
  }
  assert(pair.first->first != nullptr);
  return const_cast<StringData*>(pair.first->first);
}

StringData *StringData::LookupStaticString(const StringData *str) {
  if (UNLIKELY(!s_stringDataMap)) return nullptr;
  if (str->isStatic()) {
    assert(checkStaticStr(str));
    return const_cast<StringData*>(str);
  }
  StringDataMap::const_iterator it = s_stringDataMap->find(str);
  if (it != s_stringDataMap->end()) {
    return const_cast<StringData*>(it->first);
  }
  return nullptr;
}

StringData* StringData::GetStaticString(const String& str) {
  assert(!str.isNull());
  return GetStaticString(str.get());
}

StringData *StringData::GetStaticString(const char *str, size_t len) {
  StackStringData sd(str, len, CopyString);
  return GetStaticString(&sd);
}

StringData *StringData::GetStaticString(const std::string &str) {
  return GetStaticString(str.c_str(), str.size());
}

StringData *StringData::GetStaticString(const char *str) {
  return GetStaticString(str, strlen(str));
}

uint32_t StringData::GetCnsHandle(const StringData* cnsName) {
  assert(s_stringDataMap);
  StringDataMap::const_iterator it = s_stringDataMap->find(cnsName);
  if (it != s_stringDataMap->end()) {
    return it->second;
  }
  return 0;
}

uint32_t StringData::DefCnsHandle(const StringData* cnsName, bool persistent) {
  uint32_t val = GetCnsHandle(cnsName);
  if (val) return val;
  if (!cnsName->isStatic()) {
    // Its a dynamic constant, that doesnt correspond to
    // an already allocated handle. We'll allocate it in
    // the request local TargetCache::s_constants instead.
    return 0;
  }
  StringDataMap::iterator it = s_stringDataMap->find(cnsName);
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
      TypedValue& tv =
        Transl::TargetCache::handleToRef<TypedValue>(it->second);
      if (tv.m_type != KindOfUninit) {
        StrNR key(const_cast<StringData*>(it->first));
        a.set(key, tvAsVariant(&tv), true);
      } else if (tv.m_data.pref) {
        StrNR key(const_cast<StringData*>(it->first));
        ClassInfo::ConstantInfo* ci =
          (ClassInfo::ConstantInfo*)(void*)tv.m_data.pref;
        a.set(key, ci->getDeferredValue(), true);
      }
    }
  }

  return a;
}

void StringData::enlist() {
  assert(isShared());
  SweepNode& head = MemoryManager::TheMemoryManager()->m_strings;
  // insert after head
  SweepNode* next = head.next;
  assert(uintptr_t(next) != kMallocFreeWord);
  m_big.node.next = next;
  m_big.node.prev = &head;
  next->prev = head.next = &m_big.node;
}

void StringData::delist() {
  assert(isShared());
  SweepNode* next = m_big.node.next;
  SweepNode* prev = m_big.node.prev;
  assert(uintptr_t(next) != kMallocFreeWord);
  assert(uintptr_t(prev) != kMallocFreeWord);
  next->prev = prev;
  prev->next = next;
}

void StringData::sweepAll() {
  SweepNode& head = MemoryManager::TheMemoryManager()->m_strings;
  for (SweepNode *next, *n = head.next; n != &head; n = next) {
    next = n->next;
    assert(next && uintptr_t(next) != kSmartFreeWord);
    assert(next && uintptr_t(next) != kMallocFreeWord);
    StringData* s = (StringData*)(uintptr_t(n) -
                                  offsetof(StringData, m_big.node));
    assert(s->isShared());
    s->m_big.shared->decRef();
  }
  head.next = head.prev = &head;
}

HOT_FUNC
void StringData::initAttach(const char* data) {
  return initAttach(data, strlen(data));
}

HOT_FUNC
void StringData::initAttach(const char* data, int len) {
  if (uint32_t(len) > MaxSize) {
    throw InvalidArgumentException("len > 2^31-2", len);
  }
  m_hash = 0;
  m_count = 0;
  if (uint32_t(len) <= MaxSmallSize) {
    memcpy(m_small, data, len);
    m_len = len;
    m_data = m_small;
    m_small[len] = 0;
    setSmall();
    free(const_cast<char*>(data)); // XXX
  } else {
    char* buf = (char*)smart_malloc(len + 1);
    memcpy(buf, data, len);
    buf[len] = 0;
    m_len = len;
    m_cdata = buf;
    setModeAndCap(Mode::Smart, len + 1);
    free(const_cast<char*>(data)); // XXX
  }
  assert(checkSane());
}

HOT_FUNC
void StringData::initCopy(const char* data) {
  return initCopy(data, strlen(data));
}

HOT_FUNC
void StringData::initCopy(const char* data, int len) {
  if (uint32_t(len) > MaxSize) {
    throw InvalidArgumentException("len > 2^31-2", len);
  }
  m_hash = 0;
  m_count = 0;
  if (uint32_t(len) <= MaxSmallSize) {
    memcpy(m_small, data, len);
    m_len = len;
    m_data = m_small;
    m_small[len] = 0;
    setSmall();
  } else {
    char *buf = (char*)smart_malloc(len + 1);
    memcpy(buf, data, len);
    buf[len] = 0;
    m_len = len;
    m_cdata = buf;
    setModeAndCap(Mode::Smart, len + 1);
  }
  assert(checkSane());
}

HOT_FUNC
void StringData::initMalloc(const char* data, int len) {
  if (uint32_t(len) > MaxSize) {
    throw InvalidArgumentException("len > 2^31-2", len);
  }
  m_hash = 0;
  m_count = 0;
  if (uint32_t(len) <= MaxSmallSize) {
    memcpy(m_small, data, len);
    m_len = len;
    m_data = m_small;
    m_small[len] = 0;
    setSmall();
  } else {
    char *buf = (char*)malloc(len + 1);
    memcpy(buf, data, len);
    buf[len] = 0;
    m_len = len;
    m_cdata = buf;
    setModeAndCap(Mode::Malloc, len + 1);
  }
  assert(checkSane());
}

HOT_FUNC
StringData::StringData(SharedVariant* shared)
  : m_count(0) {
  assert(shared && size_t(shared->stringLength()) <= size_t(MaxSize));
  m_hash = 0;
  auto len = shared->stringLength();
  auto data = shared->stringData();
  if (len <= MaxSmallSize) {
    memcpy(m_small, data, len + 1); // include \0
    m_len = len;
    m_data = m_small;
    setSmall();
    assert(m_small[len] == 0);
  } else {
    shared->incRef();
    m_len = len;
    m_cdata = data;
    m_big.shared = shared;
    setModeAndCap(Mode::Shared, len + 1);
    enlist();
  }
}

HOT_FUNC
void StringData::releaseDataSlowPath() {
  assert(!isSmall());
  assert(checkSane());

  auto const loadedMode = mode();

  if (LIKELY(loadedMode == Mode::Smart)) {
    smart_free(m_data);
    return;
  }

  if (loadedMode == Mode::Shared) {
    assert(checkSane());
    m_big.shared->decRef();
    delist();
    return;
  }

  assert(loadedMode == Mode::Malloc);
  assert(checkSane());
  free(m_data);
}

char* smart_concat(const char* s1, uint32_t len1, const char* s2, uint32_t len2) {
  uint32_t len = len1 + len2;
  char* s = (char*)smart_malloc(len + 1);
  memcpy(s, s1, len1);
  memcpy(s + len1, s2, len2);
  s[len] = 0;
  return s;
}

void StringData::initConcat(StringSlice r1, StringSlice r2) {
  m_hash = 0;
  m_count = 0;
  uint32_t len = r1.len + r2.len;
  if (len <= MaxSmallSize) {
    memcpy(m_small,          r1.ptr, r1.len);
    memcpy(m_small + r1.len, r2.ptr, r2.len);
    m_len = len;
    m_data = m_small;
    m_small[len] = 0;
    setSmall();
  } else if (UNLIKELY(len > MaxSize)) {
    throw FatalErrorException(0, "String length exceeded 2^31-2: %u", len);
  } else {
    char* buf = smart_concat(r1.ptr, r1.len, r2.ptr, r2.len);
    m_len = len;
    m_data = buf;
    setModeAndCap(Mode::Smart, len + 1);
  }
}

// make an empty string with cap reserve bytes, plus one more for \0
HOT_FUNC
StringData::StringData(int cap) {
  m_hash = 0;
  m_count = 0;
  if (uint32_t(cap) <= MaxSmallSize) {
    m_len = 0;
    m_data = m_small;
    m_small[0] = 0;
    setSmall();
  } else {
    if (UNLIKELY(uint32_t(cap) > MaxSize)) {
      throw InvalidArgumentException("len > 2^31-2", cap);
    }
    m_len = 0;
    m_data = (char*) smart_malloc(cap + 1);
    m_data[0] = 0;
    setModeAndCap(Mode::Smart, cap + 1);
  }
}

void StringData::append(const char* s, int len) {
  assert(!isStatic() && getCount() <= 1);

  if (len == 0) return;
  if (UNLIKELY(uint32_t(len) > MaxSize)) {
    throw InvalidArgumentException("len > 2^31-2", len);
  }
  if (UNLIKELY(size_t(m_len) + size_t(len) > MaxSize)) {
    throw FatalErrorException(0, "String length exceeded 2^31-2: %zu",
                              size_t(len) + size_t(m_len));
  }

  const uint32_t newLen = m_len + len;

  /*
   * In case we're being to asked to append our own string, we need to
   * load the old pointer value (it might change when we reserve
   * below).
   *
   * We don't allow appending with an interior pointers here, although
   * we may be asked to append less than the whole string.
   */
  auto const oldDataPtr = rawdata();
  assert(uintptr_t(s) <= uintptr_t(rawdata()) ||
         uintptr_t(s) >= uintptr_t(rawdata() + capacity()));
  assert(s != rawdata() || len <= m_len);

  auto const mslice = UNLIKELY(isShared()) ? escalate(newLen)
                                           : reserve(newLen);
  if (UNLIKELY(s == oldDataPtr)) s = mslice.ptr;

  /*
   * memcpy is safe even if it's a self append---the regions will be
   * disjoint, since s can't point past our oldDataPtr, and len is
   * smaller than the old length.
   */
  memcpy(mslice.ptr + m_len, s, len);

  setSize(newLen);
  assert(checkSane());
}

MutableSlice StringData::reserve(int cap) {
  assert(!isImmutable() && m_count <= 1 && cap >= 0);
  if (cap + 1 <= capacity()) return mutableSlice();

  switch (mode()) {
    default: assert(false);
    case Mode::Small:
      m_data = (char*) smart_malloc(cap + 1);
      memcpy(m_data, m_small, m_len + 1); // includes \0
      setModeAndCap(Mode::Smart, cap + 1);
      break;
    case Mode::Smart:
      // We only use geometric growth when we're heading to the smart
      // allocator.  This is mostly because it was what was tested as
      // a perf win, but it might make sense to do it for Mode::Malloc
      // as well.  Will be revisited soon.
      cap += cap >> 2;
      m_data = (char*) smart_realloc(m_data, cap + 1);
      setModeAndCap(Mode::Smart, cap + 1);
      break;
    case Mode::Malloc:
      m_data = (char*) realloc(m_data, cap + 1);
      setModeAndCap(Mode::Malloc, cap + 1);
      break;
  }
  return MutableSlice(m_data, cap);
}

StringData* StringData::shrink(int len) {
  setSize(len);
  switch (mode()) {
    case Mode::Smart:
      m_data = (char*) smart_realloc(m_data, len + 1);
      setModeAndCap(Mode::Smart, len + 1);
      break;
    case Mode::Malloc:
      m_data = (char*) realloc(m_data, len + 1);
      setModeAndCap(Mode::Malloc, len + 1);
      break;
    default:
      // don't shrink
      break;
  }
  return this;
}

StringData *StringData::copy(bool sharedMemory /* = false */) const {
  if (isStatic()) {
    // Static strings cannot change, and are always available.
    return const_cast<StringData *>(this);
  }
  if (sharedMemory) {
    // Even if it's literal, it might come from hphpi's class info
    // which will be freed at the end of the request, and so must be
    // copied.
    return new StringData(data(), size(), CopyMalloc);
  }
  return StringData::Make(data(), size(), CopyString);
}

/*
 * Change to smart-malloced string.  Then returns a mutable slice of
 * the usable string buffer (minus space for the null terminator).
 */
MutableSlice StringData::escalate(uint32_t cap) {
  assert(isShared() && !isStatic() && cap >= m_len);

  char *buf = (char*)smart_malloc(cap + 1);
  StringSlice s = slice();
  memcpy(buf, s.ptr, s.len);
  buf[s.len] = 0;

  m_big.shared->decRef();
  delist();

  m_data = buf;
  setModeAndCap(Mode::Smart, cap + 1);
  // clear precomputed hashcode
  m_hash = 0;
  assert(checkSane());
  return MutableSlice(buf, cap);
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
    StackStringData str(s, 1, CopyString);
    raw[i] = StringData::GetStaticString(&str);
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

// mutations
void StringData::setChar(int offset, CStrRef substring) {
  assert(!isStatic());
  if (offset >= 0) {
    StringSlice s = slice();
    if (s.len == 0) {
      // PHP will treat data as an array and we don't want to follow that.
      throw OffsetOutOfRangeException();
    }
    char c = substring.empty() ? 0 : substring.data()[0];
    if (uint32_t(offset) < s.len) {
      ((char*)s.ptr)[offset] = c;
    } else if (offset <= RuntimeOption::StringOffsetLimit) {
      uint32_t newlen = offset + 1;
      MutableSlice buf = isImmutable() ? escalate(newlen) : reserve(newlen);
      memset(buf.ptr + s.len, ' ', newlen - s.len);
      buf.ptr[offset] = c;
      setSize(newlen);
    } else {
      throw OffsetOutOfRangeException();
    }
    m_hash = 0; // since we modified the string.
  }
}

void StringData::setChar(int offset, char ch) {
  assert(offset >= 0 && offset < size() && !isStatic());
  if (isImmutable()) escalate(size());
  ((char*)rawdata())[offset] = ch;
  m_hash = 0;
}

void StringData::inc() {
  assert(!isStatic());
  assert(!empty());

  if (isImmutable()) {
    escalate(m_len + 1);
  } else {
    reserve(m_len + 1);
  }
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
      throw InvalidArgumentException("len > 2^31-2", len);
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

void StringData::negate() {
  if (empty()) return;
  // Assume we're a fresh mutable copy.
  assert(!isImmutable() && m_count <= 1 && m_hash == 0);
  StringSlice s = slice();
  char *buf = (char*)s.ptr;
  for (int i = 0, len = s.len; i < len; i++) {
    buf[i] = ~(buf[i]);
  }
  m_hash = 0;
}

void StringData::set(CStrRef key, CStrRef v) {
  setChar(key.toInt32(), v);
}

void StringData::set(CVarRef key, CStrRef v) {
  setChar(key.toInt32(), v);
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
  if (m_len != s->m_len) return false;
  ret = memcmp(rawdata(), s->rawdata(), m_len);
  return ret == 0;
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
  strhash_t h = isShared() ? m_big.shared->stringHash() :
                             hash_string_inline(m_data, m_len);
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
  static_assert(size_t(MaxSize) <= size_t(INT_MAX), "Beware int wraparound");
  static_assert(offsetof(StringData, m_count) == FAST_REFCOUNT_OFFSET,
                "m_count at wrong offset");
  static_assert(MaxSmallSize == sizeof(StringData) -
                        offsetof(StringData, m_small) - 1, "layout bustage");
  assert(uint32_t(size()) <= MaxSize);
  assert(uint32_t(capacity()) < MaxSize);
  assert(size() < capacity());
  if (isSmall()) {
    assert(m_data == m_small && m_len <= MaxSmallSize);
  } else {
    assert(m_data && m_data != m_small);
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

}
