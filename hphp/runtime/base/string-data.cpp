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

#include <cmath>

#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/util/alloc.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-strtod.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/util/stacktrace-profiler.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

NEVER_INLINE void throw_string_too_large(uint32_t len);
NEVER_INLINE void throw_string_too_large(uint32_t len) {
  throw InvalidArgumentException("len > 2^31-2", len);
}

NEVER_INLINE void throw_string_too_large2(size_t len);
NEVER_INLINE void throw_string_too_large2(size_t len) {
  throw FatalErrorException(0, "String length exceeded 2^31-2: %zu", len);
}

ALWAYS_INLINE
std::pair<StringData*,uint32_t> allocFlatForLen(uint32_t len) {
  auto const needed = static_cast<uint32_t>(sizeof(StringData) + len + 1);
  if (LIKELY(needed <= kMaxSmartSize)) {
    auto const cap = MemoryManager::smartSizeClass(needed);
    auto const sd  = static_cast<StringData*>(MM().smartMallocSizeLogged(cap));
    return std::make_pair(sd, cap);
  }

  if (UNLIKELY(needed > StringData::MaxSize + sizeof(StringData) + 1)) {
    throw_string_too_large(len);
  }

  auto const cap = needed;
  auto const ret = MM().smartMallocSizeBigLogged(cap);
  return std::make_pair(static_cast<StringData*>(ret.first),
                        static_cast<uint32_t>(ret.second));
}

ALWAYS_INLINE
void freeForSize(void* vp, uint32_t size) {
  if (LIKELY(size <= kMaxSmartSize)) {
    return MM().smartFreeSizeLogged(vp, size);
  }
  return MM().smartFreeSizeBigLogged(vp, size);
}

}

//////////////////////////////////////////////////////////////////////

StringData* StringData::MakeStatic(StringSlice sl) {
  if (UNLIKELY(sl.len > MaxSize)) {
    throw_string_too_large(sl.len);
  }

  auto const sd = static_cast<StringData*>(
    Util::low_malloc(sizeof(StringData) + sl.len + 1)
  );
  auto const data = reinterpret_cast<char*>(sd + 1);

  sd->m_data        = data;
  sd->m_lenAndCount = sl.len;
  sd->m_capAndHash  = sl.len + 1;

  data[sl.len] = 0;
  auto const mcret = memcpy(data, sl.ptr, sl.len);
  auto const ret   = reinterpret_cast<StringData*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.

  assert(ret->m_hash == 0);
  assert(ret->m_count == 0);
  ret->setStatic();

  assert(ret == sd);
  assert(ret->isFlat());
  assert(ret->isStatic());
  assert(ret->checkSane());
  return ret;
}

void StringData::destructStatic() {
  assert(checkSane());
  assert(isFlat());
  Util::low_free(this);
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE void StringData::delist() {
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
  auto& head = MM().m_strings;
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

StringData* StringData::Make(StringSlice sl, CopyStringMode) {
  auto const allocRet = allocFlatForLen(sl.len);
  auto const sd       = allocRet.first;
  auto const cap      = allocRet.second;
  auto const data     = reinterpret_cast<char*>(sd + 1);

  sd->m_data         = data;
  sd->m_lenAndCount  = sl.len;
  sd->m_capAndHash   = cap - sizeof(StringData);

  data[sl.len] = 0;
  auto const mcret = memcpy(data, sl.ptr, sl.len);
  auto const ret   = reinterpret_cast<StringData*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.

  assert(ret == sd);
  assert(ret->m_len == sl.len);
  assert(ret->m_count == 0);
  assert(ret->m_cap == cap - sizeof(StringData));
  assert(ret->m_hash == 0);
  assert(ret->isFlat());
  assert(ret->checkSane());
  return ret;
}

StringData* StringData::Make(const char* data, CopyStringMode) {
  return Make(StringSlice(data, strlen(data)), CopyString);
}

StringData* StringData::MakeMalloced(const char* data, int len) {
  if (UNLIKELY(uint32_t(len) > MaxSize)) {
    throw_string_too_large(len);
  }

  auto const cap = static_cast<uint32_t>(len) + 1;
  auto const sd = static_cast<StringData*>(
    std::malloc(sizeof(StringData) + cap)
  );

  sd->m_lenAndCount = len;
  sd->m_cap         = cap;
  sd->m_data        = reinterpret_cast<char*>(sd + 1);

  sd->m_data[len] = 0;
  auto const mcret = memcpy(sd->m_data, data, len);
  auto const ret   = reinterpret_cast<StringData*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.

  ret->preCompute();

  assert(ret == sd);
  assert(ret->m_hash != 0);
  assert(ret->m_count == 0);
  assert(ret->isFlat());
  assert(ret->checkSane());
  return ret;
}

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

StringData* StringData::Make(StringSlice s1, const char* lit2) {
  return Make(s1, StringSlice(lit2, strlen(lit2)));
}

NEVER_INLINE
void StringData::releaseDataSlowPath() {
  assert(!isFlat());
  assert(isShared());
  assert(checkSane());

  sharedPayload()->shared->decRef();
  delist();
  freeForSize(this, sizeof(StringData) + sizeof(SharedPayload));
}

void StringData::release() {
  assert(checkSane());

  if (UNLIKELY(!isFlat())) {
    return releaseDataSlowPath();
  }
  freeForSize(this, sizeof(StringData) + m_cap);
}

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
  assert(!hasMultipleRefs());

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
  assert(uintptr_t(s) <= uintptr_t(data()) ||
         uintptr_t(s) >= uintptr_t(data() + capacity()));
  assert(s != data() || len <= m_len);

  auto const target = UNLIKELY(isShared()) ? escalate(newLen)
                                           : reserve(newLen);
  auto const mslice = target->bufferSlice();

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
  assert(!isImmutable() && !hasMultipleRefs() && cap >= 0);
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
  // Recalculating ret from mcret avoids a spill.

  assert(ret == sd);
  assert(ret->checkSane());
  return ret;
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
  // Recalculating ret from mcret avoids a spill.

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

StringData *StringData::getChar(int offset) const {
  if (offset >= 0 && offset < size()) {
    return makeStaticString(m_data[offset]);
  }
  raise_notice("Uninitialized string offset: %d", offset);
  return makeStaticString("");
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
  StringSlice s = slice();
  m_hash = hash_string(s.ptr, s.len);
  assert(m_hash >= 0);
  int64_t lval; double dval;
  if (isNumericWithVal(lval, dval, 1) == KindOfNull) {
    m_hash |= STRHASH_MSB;
  }
}

void StringData::setStatic() const {
  m_count = StaticValue;
  preCompute();
}

void StringData::setUncounted() const {
  m_count = UncountedValue;
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
  return strtoll(data(), nullptr, base);
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

int StringData::compare(const StringData *v2) const {
  assert(v2);

  if (v2 == this) return 0;

  int ret = numericCompare(v2);
  if (ret < -1) {
    int len1 = size();
    int len2 = v2->size();
    int len = len1 < len2 ? len1 : len2;
    ret = memcmp(data(), v2->data(), len);
    if (ret) return ret;
    if (len1 == len2) return 0;
    return len < len1 ? 1 : -1;
  }
  return ret;
}

strhash_t StringData::hashHelper() const {
  assert(!isShared());
  strhash_t h = hash_string_inline(m_data, m_len);
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
