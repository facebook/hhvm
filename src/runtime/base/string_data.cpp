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

IMPLEMENT_SMART_ALLOCATION_HOT(StringData, SmartAllocatorImpl::NeedSweep);
///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

// The (void *) value is not used.
typedef tbb::concurrent_hash_map<const StringData *, void *,
                                 StringDataHashCompare> StringDataMap;
static StringDataMap *s_stringDataMap;

StringData *StringData::GetStaticString(const StringData *str) {
  StringDataMap::const_accessor acc;
  if (!s_stringDataMap) s_stringDataMap = new StringDataMap();
  if (s_stringDataMap->find(acc, str)) {
    return const_cast<StringData*>(acc->first);
  }
  // Lookup failed, so do the hard work of creating a StringData with its own
  // copy of the key string, so that the atomic insert() has a permanent key.
  StringData *sd = new StringData(str->data(), str->size(), CopyString);
  sd->setStatic();
  if (!s_stringDataMap->insert(acc, sd)) {
    delete sd;
  }
  ASSERT(acc->first != NULL);
  return const_cast<StringData*>(acc->first);
}

StringData *StringData::GetStaticString(const std::string &str) {
  StringData sd(str.c_str(), str.size(), AttachLiteral);
  return GetStaticString(&sd);
}

StringData *StringData::GetStaticString(const char *str) {
  StringData sd(str, strlen(str), AttachLiteral);
  return GetStaticString(&sd);
}

void StringData::initLiteral(const char* data) {
  return initLiteral(data, strlen(data));
}

void StringData::initLiteral(const char* data, int len) {
  if (uint32_t(len) > MaxSize) {
    throw InvalidArgumentException("len>=2^30", len);
  }
  // Do not copy literals, this StringData can have a shorter lifetime than
  // the literal, and the client can count on this->data() giving back
  // the literal ptr with the longer lifetime. Sketchy!
  m_hash = 0;
  _count = 0;
  m_len = len;
  m_cdata = data;
  m_big.cap = len | IsLiteral;
  ASSERT(checkSane());
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
}

void StringData::initAttachDeprecated(const char* data) {
  return initAttachDeprecated(data, strlen(data));
}

void StringData::initAttachDeprecated(const char* data, int len) {
  if (uint32_t(len) > MaxSize) {
    throw InvalidArgumentException("len>=2^30", len);
  }
  // Don't copy small strings here either because the caller sometimes
  // assumes he can mess with data while this string is still alive,
  // and we want to free it eagerly. Sketchy!
  m_hash = 0;
  _count = 0;
  m_len = len;
  m_cdata = data;
  m_big.cap = len | IsMalloc;
  ASSERT(checkSane());
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
}

HOT_FUNC
void StringData::initAttach(const char* data) {
  return initAttach(data, strlen(data));
}

HOT_FUNC
void StringData::initAttach(const char* data, int len) {
  if (uint32_t(len) > MaxSize) {
    throw InvalidArgumentException("len>=2^30", len);
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
    m_len = len;
    m_cdata = data;
    m_big.cap = len | IsMalloc;
  }
  ASSERT(checkSane());
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
}

HOT_FUNC
void StringData::initCopy(const char* data) {
  return initCopy(data, strlen(data));
}

HOT_FUNC
void StringData::initCopy(const char* data, int len) {
  if (uint32_t(len) > MaxSize) {
    throw InvalidArgumentException("len>=2^30", len);
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
  ASSERT(checkSane());
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
}

HOT_FUNC
StringData::StringData(SharedVariant *shared)
  : _count(0) {
  ASSERT(shared);
  shared->incRef();
  m_shared = shared;
  m_len = shared->stringLength();
  m_cdata = shared->stringData();
  m_big.cap = m_len | IsShared;
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
}

HOT_FUNC
void StringData::releaseData() {
  Format f = format();
  if (f == IsSmall) return;
  if (f == IsMalloc) {
    ASSERT(checkSane());
    free(m_data);
    m_data = NULL;
    return;
  }
  if (f == IsShared) {
    ASSERT(checkSane());
    m_shared->decRef();
    return;
  }
  // Nothing to do for literals, which are rarely destructed anyway.
}

void StringData::attach(char *data, int len) {
  if (uint32_t(len) > MaxSize) {
    throw InvalidArgumentException("len>=2^30", len);
  }
  releaseData();
  m_len = len;
  m_data = data;
  m_big.cap = len | IsMalloc;
}

void StringData::initConcat(StringSlice r1, StringSlice r2) {
  m_hash = 0;
  _count = 0;
  int len = r1.len + r2.len;
  if (uint32_t(len) <= MaxSmallSize) {
    memcpy(m_small,          r1.ptr, r1.len);
    memcpy(m_small + r1.len, r2.ptr, r2.len);
    m_len = len;
    m_data = m_small;
    m_small[len] = 0;
    m_small[MaxSmallSize] = 0;
  } else {
    char* buf = string_concat(r1.ptr, r1.len, r2.ptr, r2.len, len);
    m_len = len;
    m_data = buf;
    m_big.cap = len | IsMalloc;
  }
}

// make an empty string with cap reserve bytes, plus one more for \0
StringData::StringData(int cap) {
  m_hash = 0;
  _count = 0;
  if (uint32_t(cap) <= MaxSmallSize) {
    m_len = 0;
    m_data = m_small;
    m_small[0] = 0;
    m_small[MaxSmallSize] = 0;
  } else {
    m_len = 0;
    m_data = (char*) malloc(cap + 1);
    m_big.cap = cap | IsMalloc;
  }
}

void StringData::append(const char *s, int len) {
  ASSERT(!isStatic()); // never mess around with static strings!
  if (len == 0) return;
  if (uint32_t(len) > MaxSize) {
    throw InvalidArgumentException("len>=2^30", len);
  }
  int newlen;
  // TODO: t1122987: in any of the cases below where we need a bigger buffer,
  // we can probably assume we're in a concat-loop and pick a good buffer
  // size to avoid O(N^2) copying cost.
  if (isShared() || isLiteral()) {
    // buffer is immutable, don't modify it.
    // We are mutating, so we don't need to repropagate our own taint
    if (isShared()) m_shared->decRef();
    StringSlice r = slice();
    char* newdata = string_concat(r.ptr, r.len, s, len, newlen);
    m_len = newlen;
    m_data = newdata;
    m_big.cap = newlen | IsMalloc;
    m_hash = 0;
  } else if (rawdata() == s) {
    // appending ourself to ourself, be conservative.
    // We are mutating, so we don't need to repropagate our own taint
    StringSlice r = slice();
    char *newdata = string_concat(r.ptr, r.len, s, len, newlen);
    releaseData();
    m_len = newlen;
    m_data = newdata;
    m_big.cap = newlen | IsMalloc;
    m_hash = 0;
  } else if (isSmall()) {
    // we're currently small but might not be after append.
    // We are mutating, so we don't need to repropagate our own taint
    int oldlen = m_len;
    newlen = oldlen + len;
    if (unsigned(newlen) <= MaxSmallSize) {
      // win.
      memcpy(&m_small[oldlen], s, len);
      m_small[newlen] = 0;
      m_small[MaxSmallSize] = 0;
      m_len = newlen;
      m_data = m_small;
      m_hash = 0;
    } else {
      // small->big string transition.
      char *newdata = string_concat(m_small, oldlen, s, len, newlen);
      m_len = newlen;
      m_data = newdata;
      m_big.cap = newlen | IsMalloc;
      m_hash = 0;
    }
  } else {
    // generic "big string concat" path.  realloc buffer.
    int oldlen = m_len;
    char* oldp = m_data;
    ASSERT((oldp > s && oldp - s > len) ||
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
  if (uint32_t(newlen) > MaxSize) {
    releaseData();
    m_len = 0;
    m_data = NULL;
    m_big.cap = 0;
    throw FatalErrorException(0, "String length exceeded 2^30 - 1: %d", newlen);
  }
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, rawdata());
  ASSERT(checkSane());
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
    return new StringData(data(), size(), CopyString);
  } else {
    if (isLiteral()) {
      return NEW(StringData)(data(), size(), AttachLiteral);
    }
    return NEW(StringData)(data(), size(), CopyString);
  }
}

void StringData::escalate() {
  ASSERT(isImmutable() && !isStatic() && size() > 0);
  StringSlice s = slice();
  char *buf = (char*)malloc(s.len + 1);
  memcpy(buf, s.ptr, s.len);
  buf[s.len] = 0;
  m_len = s.len;
  m_data = buf;
  m_big.cap = s.len | IsMalloc;
  // clear precomputed hashcode
  m_hash = 0;
  ASSERT(checkSane());
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
    StringData str(s, 1, CopyString);
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
  return NEW(StringData)(0);
}

// mutations
void StringData::setChar(int offset, CStrRef substring) {
  ASSERT(!isStatic());
  if (offset >= 0) {
    StringSlice s = slice();
    if (s.len == 0) {
      // PHP will treat data as an array and we don't want to follow that.
      throw OffsetOutOfRangeException();
    }
    if (uint32_t(offset) < s.len) {
      char* buf = (char*)s.ptr;
      buf[offset] = substring.empty() ? 0 : substring.data()[0];
    } else if (offset <= RuntimeOption::StringOffsetLimit) {
      // We are mutating, so we don't need to repropagate our own taint
      int newlen = offset + 1;
      char *buf = (char *)Util::safe_malloc(newlen + 1);
      memcpy(buf, s.ptr, s.len);
      memset(buf + s.len, ' ', newlen - s.len);
      buf[newlen] = 0;
      buf[offset] = substring.empty() ? 0 : substring.data()[0];
      attach(buf, newlen);
    } else {
      throw OffsetOutOfRangeException();
    }
    m_hash = 0; // since we modified the string.
  }
}

void StringData::setChar(int offset, char ch) {
  ASSERT(offset >= 0 && offset < size() && !isStatic());
  if (isImmutable()) escalate();
  ((char*)rawdata())[offset] = ch;
  m_hash = 0;
}

void StringData::inc() {
  ASSERT(!isStatic());
  ASSERT(!empty());
  if (isImmutable()) {
    escalate();
  }
  StringSlice s = slice();
  // if increment_string overflows, it returns a new ptr and updates s.len
  ASSERT(int(s.len) >= 0 && s.len <= MaxSize); // safe int/uint casting
  int len = s.len;
  char *overflowed = increment_string((char *)s.ptr, len);
  if (overflowed) attach(overflowed, len);
  m_hash = 0;
}

void StringData::negate() {
  if (empty()) return;
  // Assume we're a fresh mutable copy.
  ASSERT(!isImmutable() && _count <= 1 && m_hash == 0);
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
  ASSERT(!isShared()); // because we are gonna reuse the space!
  // We don't want to collect taint for a hash
  StringSlice s = slice();
  m_hash = hash_string(s.ptr, s.len);
  ASSERT(m_hash >= 0);
  int64 lval; double dval;
  if (isNumericWithVal(lval, dval, 1) == KindOfNull) {
    m_hash |= (1ull << 63);
  }
}

void StringData::setStatic() const {
  _count = RefCountStaticValue;
  preCompute();
}

///////////////////////////////////////////////////////////////////////////////
// type conversions

DataType StringData::isNumericWithVal(int64 &lval, double &dval,
                                      int allow_errors) const {
  if (m_hash < 0) return KindOfNull;
  DataType ret = KindOfNull;
  StringSlice s = slice();
  if (s.len) {
    // Not involved in further string construction/mutation; no taint pickup
    ret = is_numeric_string(s.ptr, s.len, &lval, &dval, allow_errors);
    if (ret == KindOfNull && !isShared() && allow_errors) {
      m_hash |= (1ull << 63);
    }
  }
  return ret;
}

bool StringData::isNumeric() const {
  if (isStatic()) return (m_hash >= 0);
  int64 lval; double dval;
  DataType ret = isNumericWithVal(lval, dval, 0);
  switch (ret) {
  case KindOfNull:   return false;
  case KindOfInt64:
  case KindOfDouble: return true;
  default:
    ASSERT(false);
    break;
  }
  return false;
}

bool StringData::isInteger() const {
  if (m_hash < 0) return false;
  int64 lval; double dval;
  DataType ret = isNumericWithVal(lval, dval, 0);
  switch (ret) {
  case KindOfNull:   return false;
  case KindOfInt64:  return true;
  case KindOfDouble: return false;
  default:
    ASSERT(false);
    break;
  }
  return false;
}

bool StringData::isValidVariableName() const {
  // Not involved in further string construction/mutation; no taint pickup
  StringSlice s = slice();
  return is_valid_var_name(s.ptr, s.len);
}

int64 StringData::hashForIntSwitch(int64 firstNonZero, int64 noMatch) const {
  int64 lval; double dval;
  DataType ret = isNumericWithVal(lval, dval, 1);
  switch (ret) {
  case KindOfNull:
    // if the string is not a number, it matches 0
    return 0;
  case KindOfInt64:
    return lval;
  case KindOfDouble:
    return Variant::DoubleHashForIntSwitch(dval, noMatch);
  default:
    break;
  }
  ASSERT(false);
  return 0;
}

int64 StringData::hashForStringSwitch(
    int64 firstTrueCaseHash,
    int64 firstNullCaseHash,
    int64 firstFalseCaseHash,
    int64 firstZeroCaseHash,
    int64 firstHash,
    int64 noMatchHash,
    bool &needsOrder) const {
  int64 lval; double dval;
  DataType ret = isNumericWithVal(lval, dval, 1);
  needsOrder = false;
  switch (ret) {
  case KindOfNull:
    return empty() ? firstNullCaseHash : hash();
  case KindOfInt64:
    return lval;
  case KindOfDouble:
    return (int64) dval;
  default:
    break;
  }
  ASSERT(false);
  return 0;
}

bool StringData::toBoolean() const {
  return !empty() && !isZero();
}

int64 StringData::toInt64(int base /* = 10 */) const {
  // Taint absorbtion unnecessary; taint is recreated later for numerics
  return strtoll(rawdata(), NULL, base);
}

double StringData::toDouble() const {
  StringSlice s = slice();
  // Taint absorbtion unnecessary; taint is recreated later for numerics
  if (s.len) return zend_strtod(s.ptr, NULL);
  return 0;
}

DataType StringData::toNumeric(int64 &lval, double &dval) const {
  if (m_hash < 0) return KindOfString;
  DataType ret = isNumericWithVal(lval, dval, 0);
  if (ret == KindOfInt64 || ret == KindOfDouble) return ret;
  return KindOfString;
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

HOT_FUNC
int StringData::numericCompare(const StringData *v2) const {
  ASSERT(v2);

  int64 lval1, lval2;
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
    ASSERT(ret2 == KindOfInt64);
    dval2 = (double)lval2;
  } else {
    ASSERT(ret1 == KindOfInt64);
    ASSERT(ret2 == KindOfDouble);
    dval1 = (double)lval1;
  }

  if (dval1 > dval2) return 1;
  if (dval1 == dval2) return 0;
  return -1;
}

HOT_FUNC
int StringData::compare(const StringData *v2) const {
  ASSERT(v2);

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
int64 StringData::getSharedStringHash() const {
  ASSERT(isShared());
  return m_shared->stringHash();
}

HOT_FUNC
int64 StringData::hashHelper() const {
  // We don't want to collect taint for a hash
  StringSlice s = slice();
  int64 h = hash_string_inline(s.ptr, s.len);
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
  static_assert(sizeof(Format) == 8, "enum Format is wrong size");
  static_assert(offsetof(StringData, _count) == FAST_REFCOUNT_OFFSET,
                "_count at wrong offset");
  static_assert(MaxSmallSize == sizeof(StringData) -
                        offsetof(StringData, m_small) - 1, "layout bustage");
  ASSERT(uint32_t(size()) <= MaxSize);
  ASSERT(uint32_t(capacity()) <= MaxSize);
  ASSERT(size() <= capacity());
  ASSERT(rawdata()[size()] == 0); // all strings must be null-terminated
  if (isSmall()) {
    ASSERT(m_data == m_small && m_len <= MaxSmallSize);
  } else {
    ASSERT(m_data && m_data != m_small);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
