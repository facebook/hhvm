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

#include "hphp/runtime/base/string-data.h"

#include <cmath>
#include <utility>

#include "hphp/util/alloc.h"
#include "hphp/util/safe-cast.h"
#include "hphp/util/stacktrace-profiler.h"

#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-strtod.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

NEVER_INLINE void raiseStringLengthExceededError(size_t len) {
  raise_error("String length exceeded: %zu > %u", len, StringData::MaxSize);
}

// Allocate, initialize `m_data' and HeapObject, but not `m_lenAndHash'.
ALWAYS_INLINE StringData* allocFlat(size_t len) {
  if (UNLIKELY(len > StringData::MaxSize)) {
    raiseStringLengthExceededError(len);
  }
  auto const sizeIndex = MemoryManager::size2Index(len + kStringOverhead);
  auto sd = static_cast<StringData*>(MM().objMallocIndex(sizeIndex));
  // Refcount initialized to 1.
  sd->initHeader_16(HeaderKind::String, InitialValue, sizeIndex);
  assert(sd->capacity() >= len);
#ifndef NO_M_DATA
  sd->m_data = reinterpret_cast<char*>(sd + 1);
#endif
  return sd;
}

//////////////////////////////////////////////////////////////////////

std::aligned_storage<
  kStringOverhead,
  alignof(StringData)
>::type s_theEmptyString;

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE static bool UncountedStringOnHugePage() {
#ifdef USE_JEMALLOC_CUSTOM_HOOKS
  return high_huge1g_arena && RuntimeOption::EvalUncountedStringHuge;
#else
  return false;
#endif
}

// Create either a static or an uncounted string.
// Diffrence between static and uncounted is in the lifetime
// of the string. Static are alive for the lifetime of the process.
// Uncounted are not ref counted but will be deleted at some point.
template <bool trueStatic> ALWAYS_INLINE
StringData* StringData::MakeShared(folly::StringPiece sl) {
  if (UNLIKELY(sl.size() > StringData::MaxSize)) {
    raiseStringLengthExceededError(sl.size());
  }

  auto const allocSize = sl.size() + kStringOverhead;
  StringData* sd = reinterpret_cast<StringData*>(
    trueStatic ? low_malloc_data(allocSize)
               : UncountedStringOnHugePage() ? malloc_huge(allocSize)
               : malloc(allocSize));
  auto const data = reinterpret_cast<char*>(sd + 1);

#ifndef NO_M_DATA
  sd->m_data = data;
#endif
  auto const count = trueStatic ? StaticValue : UncountedValue;
  sd->initHeader(HeaderKind::String, count);
  sd->m_len = sl.size(); // m_hash is computed soon.

  data[sl.size()] = 0;
  auto const mcret = memcpy(data, sl.data(), sl.size());
  auto const ret = reinterpret_cast<StringData*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.
  ret->preCompute();                    // get m_hash right

  assert(ret == sd);
  assert(ret->isFlat());
  assert(trueStatic ? ret->isStatic() : ret->isUncounted());
  assert(ret->checkSane());
  return ret;
}

StringData* StringData::MakeStatic(folly::StringPiece sl) {
  return MakeShared<true>(sl);
}

StringData* StringData::MakeUncounted(folly::StringPiece sl) {
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().addAPCUncountedBlock();
  }
  return MakeShared<false>(sl);
}

StringData* StringData::MakeEmpty() {
  void* vpEmpty = &s_theEmptyString;

  auto const sd = static_cast<StringData*>(vpEmpty);
  auto const data = reinterpret_cast<char*>(sd + 1);

#ifndef NO_M_DATA
  sd->m_data        = data;
#endif
  sd->initHeader(HeaderKind::String, StaticValue);
  sd->m_lenAndHash  = 0; // len=0, hash=0
  data[0] = 0;
  sd->preCompute();

  assert(sd->m_len == 0);
  assert(sd->capacity() == 0);
  assert(sd->m_kind == HeaderKind::String);
  assert(sd->isFlat());
  assert(sd->isStatic());
  assert(sd->checkSane());
  return sd;
}

void StringData::destructStatic() {
  assert(checkSane() && isStatic());
  assert(isFlat());
  low_free_data(this);
}

void StringData::destructUncounted() {
  assert(checkSane() && isUncounted());
  assert(isFlat());

  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().removeAPCUncountedBlock();
  }
  if (UncountedStringOnHugePage()) {
    free_huge(this);
  } else {
    free(this);
  }
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE void StringData::delist() {
  assert(isProxy());
  auto& payload = *proxy();
  auto const next = payload.node.next;
  auto const prev = payload.node.prev;
  assert(uintptr_t(next) != kMallocFreeWord);
  assert(uintptr_t(prev) != kMallocFreeWord);
  next->prev = prev;
  prev->next = next;
}

unsigned StringData::sweepAll() {
  auto& head = MM().getStringList();
  auto count = 0;
  for (StringDataNode *next, *n = head.next; n != &head; n = next) {
    count++;
    next = n->next;
    assert(next && uintptr_t(next) != kSmallFreeWord);
    assert(next && uintptr_t(next) != kMallocFreeWord);
    auto const s = node2str(n);
    assert(s->isProxy());
    s->proxy()->apcstr->unreference();
  }
  head.next = head.prev = &head;
  return count;
}

//////////////////////////////////////////////////////////////////////

StringData* StringData::Make(const StringData* s, CopyStringMode) {
  auto const sd = allocFlat(s->m_len);
  sd->m_lenAndHash = s->m_lenAndHash;
  auto const data = static_cast<void*>(sd + 1);
  *memcpy8(data, s->data(), s->m_len) = 0;

  assert(sd->same(s));
  return sd;
}

StringData* StringData::Make(folly::StringPiece sl, CopyStringMode) {
  auto const sd = allocFlat(sl.size());
  sd->m_lenAndHash = sl.size(); // hash=0
  auto const data = reinterpret_cast<char*>(sd + 1);

  data[sl.size()] = 0;
  auto const mcret = memcpy(data, sl.data(), sl.size());
  auto const ret = reinterpret_cast<StringData*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.

  assert(ret == sd);
  assert(ret->m_len == sl.size());
  assert(ret->hasExactlyOneRef());
  assert(ret->m_hash == 0);
  assert(ret->isFlat());
  assert(ret->checkSane());
  return ret;
}

StringData* StringData::Make(const char* data, size_t len, CopyStringMode) {
  if (UNLIKELY(len > StringData::MaxSize)) {
    raiseStringLengthExceededError(len);
  }

  return Make(folly::StringPiece(data, len), CopyString);
}

StringData* StringData::Make(size_t reserveLen) {
  auto const sd = allocFlat(reserveLen);
  sd->setSize(0);

  assert(sd->hasExactlyOneRef());
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::Make() {
  return Make(SmallStringReserve);
}

//////////////////////////////////////////////////////////////////////

StringData* StringData::Make(char* data, size_t len, AttachStringMode) {
  if (UNLIKELY(len > StringData::MaxSize)) {
    raiseStringLengthExceededError(len);
  }
  auto const sd = Make(folly::StringPiece(data, len), CopyString);
  free(data);
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::Make(folly::StringPiece r1, folly::StringPiece r2) {
  auto const len = r1.size() + r2.size();
  auto const sd = allocFlat(len);
  sd->m_lenAndHash = len; // hash=0

  auto const data = reinterpret_cast<char*>(sd + 1);
  memcpy(data,             r1.data(), r1.size());
  memcpy(data + r1.size(), r2.data(), r2.size());
  data[len] = 0;

  assert(sd->hasExactlyOneRef());
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::Make(const StringData* s1, const StringData* s2) {
  auto const len = s1->m_len + s2->m_len;
  // `memcpy8()' could overrun the buffer by at most 7 bytes, so we allocate 6
  // more bytes here, which (together with the trailing 0) makes it safe.
  auto const sd = allocFlat(len + 6);
  sd->m_lenAndHash = len; // hash=0

  auto const data = reinterpret_cast<char*>(sd + 1);
  auto const next = memcpy8(data, s1->data(), s1->m_len);
  *memcpy8(next, s2->data(), s2->m_len) = 0;

  assert(sd->hasExactlyOneRef());
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::Make(folly::StringPiece s1, const char* lit2) {
  return Make(s1, folly::StringPiece(lit2, strlen(lit2)));
}

StringData* StringData::Make(folly::StringPiece r1, folly::StringPiece r2,
                             folly::StringPiece r3) {
  auto const len = r1.size() + r2.size() + r3.size();
  auto const sd = allocFlat(len);
  sd->m_lenAndHash  = len; // hash=0

  auto p = reinterpret_cast<char*>(sd + 1);
  p = static_cast<char*>(memcpy(p,             r1.data(), r1.size()));
  p = static_cast<char*>(memcpy(p + r1.size(), r2.data(), r2.size()));
  p = static_cast<char*>(memcpy(p + r2.size(), r3.data(), r3.size()));
  p[r3.size()] = 0;

  assert(sd->hasExactlyOneRef());
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::Make(folly::StringPiece r1, folly::StringPiece r2,
                             folly::StringPiece r3, folly::StringPiece r4) {
  auto const len = r1.size() + r2.size() + r3.size() + r4.size();
  auto const sd = allocFlat(len);
  sd->m_lenAndHash = len; // hash=0

  auto p = reinterpret_cast<char*>(sd + 1);
  p = static_cast<char*>(memcpy(p,             r1.data(), r1.size()));
  p = static_cast<char*>(memcpy(p + r1.size(), r2.data(), r2.size()));
  p = static_cast<char*>(memcpy(p + r2.size(), r3.data(), r3.size()));
  p = static_cast<char*>(memcpy(p + r3.size(), r4.data(), r4.size()));
  p[r4.size()] = 0;

  assert(sd->hasExactlyOneRef());
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE void StringData::enlist() {
  assert(isProxy());
  auto& head = MM().getStringList();
  // insert after head
  auto const next = head.next;
  auto& payload = *proxy();
  assert(uintptr_t(next) != kMallocFreeWord);
  payload.node.next = next;
  payload.node.prev = &head;
  next->prev = head.next = &payload.node;
}

StringData* StringData::MakeProxy(const APCString* apcstr) {
#ifdef NO_M_DATA
  always_assert(false);
  not_reached();
#else
  assert(!apcExtension::UseUncounted);
  // No need to check if len > MaxSize, because if it were we'd never
  // have made the StringData in the APCVariant without throwing.
  assert(size_t(apcstr->getStringData()->size()) <= size_t(MaxSize));

  auto const sd = static_cast<StringData*>(
    MM().mallocSmallSize(sizeof(StringData) + sizeof(Proxy))
  );
  auto const data = apcstr->getStringData();
  sd->m_data = const_cast<char*>(data->m_data);
  sd->initHeader(*data, InitialValue);
  sd->m_lenAndHash = data->m_lenAndHash;
  sd->proxy()->apcstr = apcstr;
  sd->enlist();
  apcstr->reference();

  assert(sd->m_len == data->size());
  assert(sd->m_aux16 == data->m_aux16);
  assert(sd->m_kind == HeaderKind::String);
  assert(sd->hasExactlyOneRef());
  assert(sd->m_hash == data->m_hash);
  assert(sd->isProxy());
  assert(sd->checkSane());
  return sd;
#endif
}

void StringData::unProxy() {
  assert(isProxy());
  proxy()->apcstr->unreference();
  delist();
}

NEVER_INLINE
void StringData::releaseProxy() {
  unProxy();
  MM().freeSmallSize(this, sizeof(StringData) + sizeof(Proxy));
}

void StringData::release() noexcept {
  assert(isRefCounted());
  assert(checkSane());
  if (UNLIKELY(!isFlat())) {
    releaseProxy();
    AARCH64_WALKABLE_FRAME();
    return;
  }
  MM().objFreeIndex(this, m_aux16);
  AARCH64_WALKABLE_FRAME();
}

//////////////////////////////////////////////////////////////////////

#define ALIASING_APPEND_ASSERT(ptr, len)                        \
  assert(uintptr_t(ptr) <= uintptr_t(data()) ||                 \
         uintptr_t(ptr) >= uintptr_t(data() + capacity() + 1)); \
  assert(ptr != data() || len <= m_len);

StringData* StringData::append(folly::StringPiece range) {
  assert(!isImmutable() && !hasMultipleRefs());

  auto s = range.data();
  auto const len = range.size();
  if (len == 0) return this;
  auto const newLen = size_t(m_len) + size_t(len);

  if (UNLIKELY(newLen > MaxSize)) {
    raiseStringLengthExceededError(newLen);
  }

  /*
   * We may have an aliasing append.  We don't allow appending with an
   * interior pointer, although we may be asked to append less than
   * the whole string in an aliasing situation.
   */
  ALIASING_APPEND_ASSERT(s, len);

  auto const requestLen = static_cast<uint32_t>(newLen);
  auto const target = UNLIKELY(isProxy()) ? escalate(requestLen)
                                           : reserve(requestLen);
  memcpy(target->mutableData() + m_len, s, len);
  target->setSize(newLen);
  assert(target->checkSane());

  return target;
}

StringData* StringData::append(folly::StringPiece r1, folly::StringPiece r2) {
  assert(!isImmutable() && !hasMultipleRefs());

  auto const len = r1.size() + r2.size();

  if (len == 0) return this;
  if (UNLIKELY(size_t(m_len) + size_t(len) > MaxSize)) {
    raiseStringLengthExceededError(size_t(len) + size_t(m_len));
  }

  auto const newLen = m_len + len;

  /*
   * We may have an aliasing append.  We don't allow appending with an
   * interior pointer, although we may be asked to append less than
   * the whole string in an aliasing situation.
   */
  ALIASING_APPEND_ASSERT(r1.data(), r1.size());
  ALIASING_APPEND_ASSERT(r2.data(), r2.size());

  auto const target = UNLIKELY(isProxy()) ? escalate(newLen)
                                          : reserve(newLen);

  /*
   * memcpy is safe even if it's a self append---the regions will be
   * disjoint, since rN.data() can't point past the start of our source
   * pointer, and rN.size() is smaller than the old length.
   */
  void* p = target->mutableData();
  p = memcpy((char*)p + m_len,     r1.data(), r1.size());
      memcpy((char*)p + r1.size(), r2.data(), r2.size());

  target->setSize(newLen);
  assert(target->checkSane());

  return target;
}

StringData* StringData::append(folly::StringPiece r1,
                               folly::StringPiece r2,
                               folly::StringPiece r3) {
  assert(!isImmutable() && !hasMultipleRefs());

  auto const len = r1.size() + r2.size() + r3.size();

  if (len == 0) return this;
  if (UNLIKELY(size_t(m_len) + size_t(len) > MaxSize)) {
    raiseStringLengthExceededError(size_t(len) + size_t(m_len));
  }

  auto const newLen = m_len + len;

  /*
   * We may have an aliasing append.  We don't allow appending with an
   * interior pointer, although we may be asked to append less than
   * the whole string in an aliasing situation.
   */
  ALIASING_APPEND_ASSERT(r1.data(), r1.size());
  ALIASING_APPEND_ASSERT(r2.data(), r2.size());
  ALIASING_APPEND_ASSERT(r3.data(), r3.size());

  auto const target = UNLIKELY(isProxy()) ? escalate(newLen)
                                          : reserve(newLen);

  /*
   * memcpy is safe even if it's a self append---the regions will be
   * disjoint, since rN.data() can't point past the start of our source
   * pointer, and rN.size() is smaller than the old length.
   */
  void* p = target->mutableData();
  p = memcpy((char*)p + m_len,     r1.data(), r1.size());
  p = memcpy((char*)p + r1.size(), r2.data(), r2.size());
      memcpy((char*)p + r2.size(), r3.data(), r3.size());

  target->setSize(newLen);
  assert(target->checkSane());

  return target;
}

#undef ALIASING_APPEND_ASSERT

//////////////////////////////////////////////////////////////////////

StringData* StringData::reserve(size_t cap) {
  assert(!isImmutable() && !hasMultipleRefs());
  assert(isFlat());

  if (cap <= capacity()) return this;

  cap = std::min(cap + cap / 4, size_t(MaxSize));
  auto const sd = allocFlat(cap);

  // Request-allocated StringData are always aligned at 16 bytes, thus it is
  // safe to copy in 16-byte groups.
#ifdef NO_M_DATA
  // layout: [header][m_lenAndHash][...data]
  sd->m_lenAndHash = m_lenAndHash;
  // This copies the characters (m_len bytes), and the trailing zero (1 byte)
  memcpy16_inline(sd+1, this+1, (m_len + 1 + 15) & ~0xF);
  assertx(reinterpret_cast<uintptr_t>(this+1) % 16 == 0);
#else
  // layout: [header][m_data][m_lenAndHash][...data]
  // This copies m_lenAndHash (8 bytes), the characters (m_len bytes),
  // and the trailing zero (1 byte).
  memcpy16_inline(&sd->m_lenAndHash, &m_lenAndHash,
                  (m_len + 8 + 1 + 15) & ~0xF);
  assertx(reinterpret_cast<uintptr_t>(&m_lenAndHash) + 8 ==
          reinterpret_cast<uintptr_t>(m_data));
  assertx(reinterpret_cast<uintptr_t>(&m_lenAndHash) % 16 == 0);
#endif

  assert(sd->hasExactlyOneRef());
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::shrinkImpl(size_t len) {
  assert(!isImmutable() && !hasMultipleRefs());
  assert(isFlat());
  assert(len <= capacity());

  auto const sd = allocFlat(len);
  sd->m_lenAndHash = len;
  auto const src = static_cast<void*>(this + 1);
  auto const dst = static_cast<void*>(sd + 1);
  *memcpy8(dst, src, len) = 0;

  assert(sd->checkSane());
  return sd;
}

StringData* StringData::shrink(size_t len) {
  assert(!isImmutable() && !hasMultipleRefs());
  if (capacity() - len > kMinShrinkThreshold) {
    return shrinkImpl(len);
  }
  assert(len < MaxSize);
  setSize(len);
  return this;
}

// State transition from Mode::Shared to Mode::Flat.
StringData* StringData::escalate(size_t cap) {
  assert(isProxy() && !isStatic() && cap >= m_len);

  auto const sd = allocFlat(cap);
  sd->m_lenAndHash = m_lenAndHash;
  auto const sd_data = reinterpret_cast<char*>(sd + 1);
  *memcpy8(sd_data, data(), m_len) = 0;

  assert(sd->hasExactlyOneRef());
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

void StringData::dump() const {
  auto s = slice();

  printf("StringData(%d) (%s%s%s%d): [", m_count,
         isProxy() ? "proxy " : "",
         isStatic() ? "static " : "",
         isUncounted() ? "uncounted " : "",
         static_cast<int>(s.size()));
  for (uint32_t i = 0; i < s.size(); i++) {
    char ch = s.data()[i];
    if (isprint(ch)) {
      printf("%c", ch);
    } else {
      printf("\\x%02x", ch);
    }
  }
  printf("]\n");
}

StringData* StringData::getChar(int offset) const {
  if (offset >= 0 && offset < size()) {
    return makeStaticString(data()[offset]);
  }
  raise_notice("Uninitialized string offset: %d", offset);
  return staticEmptyString();
}

StringData* StringData::increment() {
  assert(!isImmutable() && !hasMultipleRefs());
  assert(!empty());

  auto const sd = UNLIKELY(isProxy())
    ? escalate(m_len + 1)
    : reserve(m_len + 1);
  sd->incrementHelper();
  return sd;
}

void StringData::incrementHelper() {
  if (RuntimeOption::EnableHipHopSyntax) {
    raise_notice("Increment on string '%s'", data());
  }
  m_hash = 0;

  enum class CharKind {
    UNKNOWN,
    LOWER_CASE,
    UPPER_CASE,
    NUMERIC
  };

  auto const len = m_len;
  auto const s = mutableData();
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
      raiseStringLengthExceededError(len + 1);
    }

    assert(len + 1 <= capacity());
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

void StringData::preCompute() {
  auto s = slice();
  m_hash = hash_string_i_unsafe(s.data(), s.size());
  assert(m_hash >= 0);
  if (s.size() > 0 &&
      (is_numeric_string(s.data(), s.size(), nullptr, nullptr,
                         1, nullptr) == KindOfNull)) {
    m_hash |= STRHASH_MSB;
  }
}

#if (!defined(__SSE4_2__) && !defined(ENABLE_AARCH64_CRC)) || \
     defined(NO_HWCRC) || !defined(NO_M_DATA) || defined(_MSC_VER)
// This function is implemented directly in ASM in string-data-*.S otherwise.
NEVER_INLINE strhash_t StringData::hashHelper() const {
  assert(!isProxy());
  strhash_t h = hash_string_i_unsafe(data(), m_len);
  assert(h >= 0);
  m_hash |= h;
  return h;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// type conversions

DataType StringData::isNumericWithVal(int64_t &lval, double &dval,
                                      int allow_errors, int* overflow) const {
  if (m_hash < 0) return KindOfNull;
  DataType ret = KindOfNull;
  auto s = slice();
  if (s.size()) {
    ret = is_numeric_string(
      s.data(),
      s.size(),
      &lval,
      &dval,
      allow_errors,
      overflow
    );
    if (ret == KindOfNull && allow_errors) {
      // a proxy string has its hash precomputed - so it can't
      // suddenly go from being numeric to not-numeric
      assert(!isProxy());
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
    case KindOfNull:
      return false;
    case KindOfInt64:
    case KindOfDouble:
      return true;
    case KindOfUninit:
    case KindOfBoolean:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      break;
  }
  not_reached();
}

bool StringData::isInteger() const {
  if (m_hash < 0) return false;
  int64_t lval; double dval;
  DataType ret = isNumericWithVal(lval, dval, 0);
  switch (ret) {
    case KindOfNull:
    case KindOfDouble:
      return false;
    case KindOfInt64:
      return true;
    case KindOfUninit:
    case KindOfBoolean:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      break;
  }
  not_reached();
}

bool StringData::toBoolean() const {
  return !empty() && !isZero();
}

int64_t StringData::toInt64(int base /* = 10 */) const {
  return strtoll(data(), nullptr, base);
}

double StringData::toDouble() const {
  auto s = slice();
  if (s.size()) return zend_strtod(s.data(), nullptr);
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

  int oflow1, oflow2;
  int64_t lval1, lval2;
  double dval1, dval2;
  DataType ret1, ret2;
  if ((ret1 = isNumericWithVal(lval1, dval1, 0, &oflow1)) == KindOfNull ||
      (ret1 == KindOfDouble && !std::isfinite(dval1)) ||
      (ret2 = v2->isNumericWithVal(lval2, dval2, 0, &oflow2)) == KindOfNull ||
      (ret2 == KindOfDouble && !std::isfinite(dval2))) {
    return -2;
  }
  if (oflow1 && oflow1 == oflow2 && dval1 == dval2) {
    return -2; // overflow in same direction, comparison will be inaccurate
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
    if (oflow1) {
      return oflow1;
    }
    dval2 = (double)lval2;
  } else {
    assert(ret1 == KindOfInt64);
    assert(ret2 == KindOfDouble);
    if (oflow2) {
      return -oflow2;
    }
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

///////////////////////////////////////////////////////////////////////////////
// Debug

std::string StringData::toCppString() const {
  auto s = slice();
  return std::string(s.data(), s.size());
}

bool StringData::checkSane() const {
  static_assert(sizeof(StringData) == use_lowptr ? 16 : 24,
                "StringData size changed---update assertion if you mean it");
  static_assert(size_t(MaxSize) <= size_t(INT_MAX), "Beware int wraparound");
#ifdef NO_M_DATA
  static_assert(sizeof(StringData) == SD_DATA, "");
  static_assert(offsetof(StringData, m_len) == SD_LEN, "");
  static_assert(offsetof(StringData, m_hash) == SD_HASH, "");
#endif
  assert(kindIsValid());
  assert(uint32_t(size()) <= MaxSize);
  assert(size() >= 0);
  if (isImmutable()) {
    assert(capacity() == 0);
  } else {
    assert(size() <= capacity());
    assert(capacity() <= MaxSize);
  }
  // isFlat() and isProxy() both check whether m_data == payload(),
  // which guarantees by definition that isFlat() != isProxy()
  return true;
}

//////////////////////////////////////////////////////////////////////

}
