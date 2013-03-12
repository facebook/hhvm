/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/string_data.h>
#include <runtime/base/shared/shared_variant.h>
#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/util/exceptions.h>
#include <util/alloc.h>
#include <math.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_strtod.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <tbb/concurrent_hash_map.h>

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION_HOT(StringData);
///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

// The (void *) value is not used.
typedef tbb::concurrent_hash_map<const StringData *, void *,
                                 StringDataHashCompare> StringDataMap;
static StringDataMap *s_stringDataMap;

size_t StringData::GetStaticStringCount() {
  if (!s_stringDataMap) return 0;
  return s_stringDataMap->size();
}

StringData *StringData::GetStaticString(const StringData *str) {
  StringDataMap::const_accessor acc;
  if (UNLIKELY(!s_stringDataMap)) s_stringDataMap = new StringDataMap();
  if (s_stringDataMap->find(acc, str)) {
    return const_cast<StringData*>(acc->first);
  }
  // Lookup failed, so do the hard work of creating a StringData with its own
  // copy of the key string, so that the atomic insert() has a permanent key.
  StringData *sd = (StringData*)Util::low_malloc(sizeof(StringData));
  new (sd) StringData(str->data(), str->size(), CopyMalloc);
  sd->setStatic();
  if (!s_stringDataMap->insert(acc, sd)) {
    sd->~StringData();
    Util::low_free(sd);
  }
  assert(acc->first != nullptr);
  return const_cast<StringData*>(acc->first);
}

StringData* StringData::FindStaticString(const StringData* str) {
  StringDataMap::const_accessor acc;
  if (UNLIKELY(!s_stringDataMap)) s_stringDataMap = new StringDataMap();
  if (s_stringDataMap->find(acc, str)) {
    return const_cast<StringData*>(acc->first);
  }
  return nullptr;
}

StringData *StringData::GetStaticString(const std::string &str) {
  StackStringData sd(str.c_str(), str.size(), AttachLiteral);
  return GetStaticString(&sd);
}

StringData *StringData::GetStaticString(const char *str) {
  StackStringData sd(str, strlen(str), AttachLiteral);
  return GetStaticString(&sd);
}

void StringData::initLiteral(const char* data) {
  return initLiteral(data, strlen(data));
}

void StringData::initLiteral(const char* data, int len) {
  if (uint32_t(len) > MaxSize) {
    throw InvalidArgumentException("len > 2^31-2", len);
  }
  // Do not copy literals, this StringData can have a shorter lifetime than
  // the literal, and the client can count on this->data() giving back
  // the literal ptr with the longer lifetime. Sketchy!
  m_hash = 0;
  _count = 0;
  m_len = len;
  m_cdata = data;
  m_big.cap = len | IsLiteral;
  assert(checkSane());
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
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
  _count = 0;
  if (uint32_t(len) <= MaxSmallSize) {
    memcpy(m_small, data, len);
    m_len = len;
    m_data = m_small;
    m_small[len] = 0;
    m_small[MaxSmallSize] = 0;
    free((void*)data);
  } else {
    char* buf = (char*)smart_malloc(len + 1);
    memcpy(buf, data, len);
    buf[len] = 0;
    m_len = len;
    m_cdata = buf;
    m_big.cap = len | IsSmart;
    free((void*)data);
  }
  assert(checkSane());
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
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
  _count = 0;
  if (uint32_t(len) <= MaxSmallSize) {
    memcpy(m_small, data, len);
    m_len = len;
    m_data = m_small;
    m_small[len] = 0;
    m_small[MaxSmallSize] = 0;
  } else {
    char *buf = (char*)smart_malloc(len + 1);
    memcpy(buf, data, len);
    buf[len] = 0;
    m_len = len;
    m_cdata = buf;
    m_big.cap = len | IsSmart;
  }
  assert(checkSane());
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
}

HOT_FUNC
void StringData::initMalloc(const char* data, int len) {
  if (uint32_t(len) > MaxSize) {
    throw InvalidArgumentException("len > 2^31-2", len);
  }
  m_hash = 0;
  _count = 0;
  if (uint32_t(len) <= MaxSmallSize) {
    memcpy(m_small, data, len);
    m_len = len;
    m_data = m_small;
    m_small[len] = 0;
    m_small[MaxSmallSize] = 0;
  } else {
    char *buf = (char*)malloc(len + 1);
    memcpy(buf, data, len);
    buf[len] = 0;
    m_len = len;
    m_cdata = buf;
    m_big.cap = len | IsMalloc;
  }
  assert(checkSane());
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
}

HOT_FUNC
StringData::StringData(SharedVariant *shared)
  : _count(0) {
  assert(shared && size_t(shared->stringLength()) <= size_t(MaxSize));
  shared->incRef();
  m_hash = 0;
  m_len = shared->stringLength();
  m_cdata = shared->stringData();
  m_big.shared = shared;
  m_big.cap = m_len | IsShared;
  enlist();
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
}

HOT_FUNC
void StringData::releaseData() {
  switch (format()) {
  case IsMalloc:
    assert(checkSane());
    free(m_data);
    break;
  case IsShared:
    assert(checkSane());
    m_big.shared->decRef();
    delist();
    break;
  case IsSmart:
    assert(checkSane());
    smart_free(m_data);
    break;
  default:
    break;
  }
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
  _count = 0;
  uint32_t len = r1.len + r2.len;
  if (len <= MaxSmallSize) {
    memcpy(m_small,          r1.ptr, r1.len);
    memcpy(m_small + r1.len, r2.ptr, r2.len);
    m_len = len;
    m_data = m_small;
    m_small[len] = 0;
    m_small[MaxSmallSize] = 0;
  } else if (UNLIKELY(len > MaxSize)) {
    throw FatalErrorException(0, "String length exceeded 2^31-2: %u", len);
  } else {
    char* buf = smart_concat(r1.ptr, r1.len, r2.ptr, r2.len);
    m_len = len;
    m_data = buf;
    m_big.cap = len | IsSmart;
  }
}

// make an empty string with cap reserve bytes, plus one more for \0
HOT_FUNC
StringData::StringData(int cap) {
  m_hash = 0;
  _count = 0;
  if (uint32_t(cap) <= MaxSmallSize) {
    m_len = 0;
    m_data = m_small;
    m_small[0] = 0;
    m_small[MaxSmallSize] = 0;
  } else {
    if (UNLIKELY(uint32_t(cap) > MaxSize)) {
      throw InvalidArgumentException("len > 2^31-2", cap);
    }
    m_len = 0;
    m_data = (char*) smart_malloc(cap + 1);
    m_data[0] = 0;
    m_big.cap = cap | IsSmart;
  }
}

void StringData::append(const char *s, int len) {
  assert(!isStatic()); // never mess around with static strings!
  if (len == 0) return;
  if (UNLIKELY(uint32_t(len) > MaxSize)) {
    throw InvalidArgumentException("len > 2^31-2", len);
  }
  if (UNLIKELY(size_t(m_len) + size_t(len) > MaxSize)) {
    throw FatalErrorException(0, "String length exceeded 2^31-2: %ul",
                              size_t(len) + size_t(m_len));
  }
  uint32_t newlen = m_len + len;
  // TODO: t1122987: in any of the cases below where we need a bigger buffer,
  // we can probably assume we're in a concat-loop and pick a good buffer
  // size to avoid O(N^2) copying cost.
  if (isShared() || isLiteral()) {
    // buffer is immutable, don't modify it.
    // We are mutating, so we don't need to repropagate our own taint
    StringSlice r = slice();
    char* newdata = smart_concat(r.ptr, r.len, s, len);
    if (isShared()) {
      m_big.shared->decRef();
      delist();
    }
    m_len = newlen;
    m_data = newdata;
    m_big.cap = newlen | IsSmart;
    m_hash = 0;
  } else if (rawdata() == s) {
    // appending ourself to ourself, be conservative.
    // We are mutating, so we don't need to repropagate our own taint
    StringSlice r = slice();
    char *newdata = smart_concat(r.ptr, r.len, s, len);
    releaseData();
    m_len = newlen;
    m_data = newdata;
    m_big.cap = newlen | IsSmart;
    m_hash = 0;
  } else if (isSmall()) {
    // we're currently small but might not be after append.
    // We are mutating, so we don't need to repropagate our own taint
    uint32_t oldlen = m_len;
    newlen = oldlen + len;
    if (newlen <= MaxSmallSize) {
      // win.
      memcpy(&m_small[oldlen], s, len);
      m_small[newlen] = 0;
      m_small[MaxSmallSize] = 0;
      m_len = newlen;
      m_data = m_small;
      m_hash = 0;
    } else {
      // small->big string transition.
      char *newdata = smart_concat(m_small, oldlen, s, len);
      m_len = newlen;
      m_data = newdata;
      m_big.cap = newlen | IsSmart;
      m_hash = 0;
    }
  } else if (format() == IsSmart) {
    // generic "big string concat" path.  smart_realloc buffer.
    uint32_t oldlen = m_len;
    char* oldp = m_data;
    assert((oldp > s && oldp - s > len) ||
           (oldp < s && s - oldp > oldlen)); // no overlapping
    newlen = oldlen + len;
    char* newdata;
    if ((int)newlen <= capacity()) {
      newdata = oldp;
    } else {
      uint32_t nlen = newlen + (newlen >> 2);
      newdata = (char*) smart_realloc(oldp, nlen + 1);
      m_big.cap = nlen | IsSmart;
    }
    memcpy(newdata + oldlen, s, len);
    newdata[newlen] = 0;
    m_len = newlen;
    m_data = newdata;
    m_hash = 0;
  } else {
    // generic "big string concat" path.  realloc buffer.
    uint32_t oldlen = m_len;
    char* oldp = m_data;
    assert((oldp > s && oldp - s > len) ||
           (oldp < s && s - oldp > oldlen)); // no overlapping
    newlen = oldlen + len;
    char* newdata = (char*) realloc(oldp, newlen + 1);
    memcpy(newdata + oldlen, s, len);
    newdata[newlen] = 0;
    m_len = newlen;
    m_data = newdata;
    m_big.cap = newlen | IsMalloc;
    m_hash = 0;
  }
  assert(newlen <= MaxSize);
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
  assert(checkSane());
}

MutableSlice StringData::reserve(int cap) {
  assert(!isImmutable() && _count <= 1 && cap >= 0);
  if (cap <= capacity()) return mutableSlice();
  switch (format()) {
    default: assert(false);
    case IsSmall:
      m_data = (char*) smart_malloc(cap + 1);
      memcpy(m_data, m_small, m_len + 1); // includes \0
      m_big.cap = cap | IsSmart;
      break;
    case IsSmart:
      m_data = (char*) smart_realloc(m_data, cap + 1);
      m_big.cap = cap | IsSmart;
      break;
    case IsMalloc:
      m_data = (char*) realloc(m_data, cap + 1);
      m_big.cap = cap | IsMalloc;
      break;
  }
  return MutableSlice(m_data, cap);
}

StringData* StringData::shrink(int len) {
  setSize(len);
  switch (format()) {
    case IsSmart:
      m_data = (char*) smart_realloc(m_data, len + 1);
      m_big.cap = len | IsSmart;
      break;
    case IsMalloc:
      m_data = (char*) realloc(m_data, len + 1);
      m_big.cap = len | IsMalloc;
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
  } else {
    if (isLiteral()) {
      return NEW(StringData)(data(), size(), AttachLiteral);
    }
    return NEW(StringData)(data(), size(), CopyString);
  }
}

MutableSlice StringData::escalate(uint32_t cap) {
  assert(isImmutable() && !isStatic() && cap >= m_len);
  char *buf = (char*)smart_malloc(cap + 1);
  StringSlice s = slice();
  memcpy(buf, s.ptr, s.len);
  buf[s.len] = 0;
  m_data = buf;
  m_big.cap = s.len | IsSmart;
  // clear precomputed hashcode
  m_hash = 0;
  assert(checkSane());
  return MutableSlice(buf, cap);
}

StringData *StringData::Escalate(StringData *in) {
  if (!in) return NEW(StringData)();
  if (in->_count != 1 || in->isImmutable()) {
    StringData *ret = NEW(StringData)(in->data(), in->size(), CopyString);
    return ret;
  }
  in->m_hash = 0;
  return in;
}

void StringData::dump() const {
  StringSlice s = slice();

  printf("StringData(%d) (%s%s%s%d): [", _count,
         isLiteral() ? "literal " : "",
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
#ifdef TAINTED
  printf("\n");
  this->getTaintDataRefConst().dump();
#endif
  printf("]\n");
}

static StringData** precompute_chars() ATTRIBUTE_COLD;
static StringData** precompute_chars() {
  StringData** raw = new StringData*[256];
  for (int i = 0; i < 256; i++) {
    char s[2] = { (char)i, 0 };
    StackStringData str(s, 1, AttachLiteral);
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
      // We are mutating, so we don't need to repropagate our own taint
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

/**
 * Zend's way of incrementing a string. Definitely something we want to get rid
 * of in the future.  If the incremented string is longer, use smart_malloc
 * to allocate the new string, update len, and return the new string.
 * Otherwise return null.
 */
char *increment_string(char *s, uint32_t &len) {
  assert(s && *s);
  enum CharKind {
    UNKNOWN_KIND,
    LOWER_CASE,
    UPPER_CASE,
    NUMERIC
  };

  int carry = 0;
  int pos = len - 1;
  int last = UNKNOWN_KIND; // Shut up the compiler warning
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
      last=LOWER_CASE;
    } else if (ch >= 'A' && ch <= 'Z') {
      if (ch == 'Z') {
        s[pos] = 'A';
        carry=1;
      } else {
        s[pos]++;
        carry=0;
      }
      last=UPPER_CASE;
    } else if (ch >= '0' && ch <= '9') {
      if (ch == '9') {
        s[pos] = '0';
        carry=1;
      } else {
        s[pos]++;
        carry=0;
      }
      last = NUMERIC;
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
    char *t = (char *) smart_malloc(len+1+1);
    memcpy(t+1, s, len);
    t[++len] = '\0';
    switch (last) {
    case NUMERIC:
      t[0] = '1';
      break;
    case UPPER_CASE:
      t[0] = 'A';
      break;
    case LOWER_CASE:
      t[0] = 'a';
      break;
    }
    return t;
  }
  return nullptr;
}

void StringData::inc() {
  assert(!isStatic());
  assert(!empty());
  StringSlice s = slice();
  if (isImmutable()) escalate(s.len);
  // if increment_string overflows, it returns a new ptr and updates len
  assert(s.len <= MaxSize); // safe int/uint casting
  auto len = s.len;
  char *overflowed = increment_string((char *)s.ptr, len);
  if (overflowed) {
    if (len > MaxSize) {
      throw InvalidArgumentException("len > 2^31-2", len);
    }
    releaseData();
    m_len = len;
    m_data = overflowed;
    m_big.cap = len | IsSmart;
  }
  m_hash = 0;
}

void StringData::negate() {
  if (empty()) return;
  // Assume we're a fresh mutable copy.
  assert(!isImmutable() && _count <= 1 && m_hash == 0);
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
  // We don't want to collect taint for a hash
  StringSlice s = slice();
  m_hash = hash_string(s.ptr, s.len);
  assert(m_hash >= 0);
  int64_t lval; double dval;
  if (isNumericWithVal(lval, dval, 1) == KindOfNull) {
    m_hash |= STRHASH_MSB;
  }
}

void StringData::setStatic() const {
  _count = RefCountStaticValue;
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
    // Not involved in further string construction/mutation; no taint pickup
    ret = is_numeric_string(s.ptr, s.len, &lval, &dval, allow_errors);
    if (ret == KindOfNull && !isShared() && allow_errors) {
      m_hash |= STRHASH_MSB;
    }
  }
  return ret;
}

bool StringData::isNumeric() const {
  if (isStatic()) return (m_hash >= 0);
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

bool StringData::isValidVariableName() const {
  // Not involved in further string construction/mutation; no taint pickup
  StringSlice s = slice();
  return is_valid_var_name(s.ptr, s.len);
}

bool StringData::toBoolean() const {
  return !empty() && !isZero();
}

int64_t StringData::toInt64(int base /* = 10 */) const {
  // Taint absorbtion unnecessary; taint is recreated later for numerics
  return strtoll(rawdata(), nullptr, base);
}

double StringData::toDouble() const {
  StringSlice s = slice();
  // Taint absorbtion unnecessary; taint is recreated later for numerics
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
    // No taint absorption on self-contained string ops like compare
    ret = memcmp(rawdata(), v2->rawdata(), len);
    if (ret) return ret;
    if (len1 == len2) return 0;
    return len < len1 ? 1 : -1;
  }
  return ret;
}

HOT_FUNC
strhash_t StringData::hashHelper() const {
  // We don't want to collect taint for a hash
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
  static_assert(sizeof(Format) == 8, "enum Format is wrong size");
  static_assert(offsetof(StringData, _count) == FAST_REFCOUNT_OFFSET,
                "_count at wrong offset");
  static_assert(MaxSmallSize == sizeof(StringData) -
                        offsetof(StringData, m_small) - 1, "layout bustage");
  assert(uint32_t(size()) <= MaxSize);
  assert(uint32_t(capacity()) <= MaxSize);
  assert(size() <= capacity());
  if (isSmall()) {
    assert(m_data == m_small && m_len <= MaxSmallSize);
  } else {
    assert(m_data && m_data != m_small);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
