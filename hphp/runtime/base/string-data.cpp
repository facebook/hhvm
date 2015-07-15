/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-strtod.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

// how many bytes are not included in capacity()'s return value.
auto constexpr kCapOverhead = 1 + sizeof(StringData);

NEVER_INLINE void throw_string_too_large(size_t len) {
  raise_error("String length exceeded 2^31-2: %zu", len);
}

NEVER_INLINE StringData* allocFlatForLenSlow(size_t len);

// For request-local allocation, we make a fast path to handle cases where no
// CapCode encoding is needed and the corresponding size class is fully
// utilized.  Since `CapCode::Threshold + 1' is a small size class (it should
// always be a power of 2), the threshold for string length is
auto constexpr kMaxStringSimpleLen = CapCode::Threshold + 1 - kCapOverhead;

// The following static assertions should ensure `kMaxStringSimpleLen' has the
// properties we want.
static_assert(kMaxStringSimpleLen + kCapOverhead < kMaxSmallSizeLookup, "");
static_assert(kMaxStringSimpleLen <= CapCode::Threshold, "");
auto constexpr maxSimpleAlloc = kMaxStringSimpleLen + kCapOverhead;
auto constexpr sizeClass =
  kSmallSize2Index[(maxSimpleAlloc - 1) >> kLgSmallSizeQuantum];
static_assert(kSmallIndex2Size[sizeClass] == maxSimpleAlloc,
              "kMaxStringSimpleLen should be maximized");

}

// Allocate a string with length <= kMaxStringSimpleLen, initialize `m_data'
// and `m_hdr', but not `m_lenAndHash'.
ALWAYS_INLINE StringData* allocFlatSmallImpl(size_t len) {
  assertx(len <= kMaxStringSimpleLen);
  static_assert(kMaxStringSimpleLen + kCapOverhead <= kMaxSmallSizeLookup, "");

  auto const requestSize = len + kCapOverhead;
  auto const sizeClass = MemoryManager::lookupSmallSize2Index(requestSize);
  auto const allocSize = kSmallIndex2Size[sizeClass];
  auto sd = static_cast<StringData*>(
    MM().mallocSmallIndex(sizeClass, allocSize)
  );

  auto const cap = allocSize - kCapOverhead;

  sd->m_data = reinterpret_cast<char*>(sd + 1);
  // Refcount initialized to 1.
  sd->m_hdr.init(CapCode::exact(cap), HeaderKind::String, 1, sizeClass);
  return sd;
}

// Allocate a string with length > kMaxStringSimpleLen, initialize `m_data'
// and `m_hdr', but not `m_lenAndHash'.  We sometimes want to inline this slow
// path, too.
ALWAYS_INLINE StringData* allocFlatSlowImpl(size_t len) {
  // Slow path when length is large enough to need the real CapCode encoding.
  if (UNLIKELY(len > StringData::MaxSize)) {
    throw_string_too_large(len);
  }
  auto const need = CapCode::roundUp(len) + kCapOverhead;
  StringData* sd;
  CapCode cc;
  size_t sizeClass = 0;
  static_assert(kSmallIndex2Size[0] < kCapOverhead,
                "Size class 0 indicates shared or big allocations");
  if (LIKELY(need <= kMaxSmallSize)) {
    sizeClass = MemoryManager::computeSmallSize2Index(need);
    auto const sz = MemoryManager::smallIndex2Size(sizeClass);
    cc = CapCode::floor(sz - kCapOverhead);
    sd = static_cast<StringData*>(MM().mallocSmallIndex(sizeClass, sz));
  } else {
    auto const block = MM().mallocBigSize<true>(need);
    size_t actualCap = block.size - kCapOverhead;
    if (UNLIKELY(actualCap > StringData::MaxSize)) {
      actualCap = StringData::MaxSize;
    }
    cc = CapCode::floor(static_cast<uint32_t>(actualCap));
    sd = static_cast<StringData*>(block.ptr);
  }
  assert(cc.decode() >= len);
  sd->m_data = reinterpret_cast<char*>(sd + 1);
  // Refcount initialized to 1.
  sd->m_hdr.init(cc, HeaderKind::String, 1, sizeClass);
  return sd;
}

namespace {

// Use this if the `len' is expected to be small. For long strings it falls
// back to the non-inlined slow path.
ALWAYS_INLINE UNUSED StringData* allocFlatForLenSmall(size_t len) {
  if (LIKELY(len <= kMaxStringSimpleLen)) {
    return allocFlatSmallImpl(len);
  }
  return allocFlatForLenSlow(len);
}

// This version has the slow path inlined. Use it if `len' can often be large
// enough to require cap code encoding.
ALWAYS_INLINE UNUSED StringData* allocFlatForLen(size_t len) {
  if (LIKELY(len <= kMaxStringSimpleLen)) {
    return allocFlatSmallImpl(len);
  }
  return allocFlatSlowImpl(len);
}

NEVER_INLINE StringData* allocFlatForLenSlow(size_t len) {
  return allocFlatSlowImpl(len);
}

}

//////////////////////////////////////////////////////////////////////

std::aligned_storage<
  sizeof(StringData) + 1,
  alignof(StringData)
>::type s_theEmptyString;

//////////////////////////////////////////////////////////////////////

// Create either a static or an uncounted string.
// Diffrence between static and uncounted is in the lifetime
// of the string. Static are alive for the lifetime of the process.
// Uncounted are not ref counted but will be deleted at some point.
ALWAYS_INLINE
StringData* StringData::MakeShared(StringSlice sl, bool trueStatic) {
  if (UNLIKELY(sl.len > StringData::MaxSize)) {
    throw_string_too_large(sl.len);
  }

  auto const cc = CapCode::ceil(sl.len);
  auto const need = cc.decode() + kCapOverhead;
  auto const sd = static_cast<StringData*>(
    trueStatic ? low_malloc(need) : malloc(need)
  );
  auto const data = reinterpret_cast<char*>(sd + 1);

  sd->m_data = data;
  auto const count = trueStatic ? StaticValue : UncountedValue;
  sd->m_hdr.init(cc, HeaderKind::String, count);
  sd->m_len = sl.len; // m_hash is computed soon.

  data[sl.len] = 0;
  auto const mcret = memcpy(data, sl.ptr, sl.len);
  auto const ret = reinterpret_cast<StringData*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.
  ret->preCompute();                    // get m_hash right

  assert(ret == sd);
  assert(ret->isFlat());
  assert(trueStatic ? ret->isStatic() : ret->isUncounted());
  assert(ret->checkSane());
  return ret;
}

StringData* StringData::MakeStatic(StringSlice sl) {
  return MakeShared(sl, true);
}

StringData* StringData::MakeUncounted(StringSlice sl) {
  return MakeShared(sl, false);
}

StringData* StringData::MakeEmpty() {
  void* vpEmpty = &s_theEmptyString;

  auto const sd = static_cast<StringData*>(vpEmpty);
  auto const data = reinterpret_cast<char*>(sd + 1);

  sd->m_data        = data;
  sd->m_hdr.init(HeaderKind::String, 0);
  sd->m_lenAndHash  = 0; // len=0, hash=0
  data[0] = 0;

  sd->setStatic();

  assert(sd->m_len == 0);
  assert(sd->capacity() == 0);
  assert(sd->m_hdr.kind == HeaderKind::String);
  assert(sd->isFlat());
  assert(sd->isStatic());
  assert(sd->checkSane());
  return sd;
}

void StringData::destructStatic() {
  assert(checkSane() && isStatic());
  assert(isFlat());
  low_free(this);
}

void StringData::destructUncounted() {
  assert(checkSane() && isUncounted());
  assert(isFlat());
  free(this);
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

unsigned StringData::sweepAll() {
  auto& head = MM().getStringList();
  auto count = 0;
  for (StringDataNode *next, *n = head.next; n != &head; n = next) {
    count++;
    next = n->next;
    assert(next && uintptr_t(next) != kSmallFreeWord);
    assert(next && uintptr_t(next) != kMallocFreeWord);
    auto const s = node2str(n);
    assert(s->isShared());
    s->sharedPayload()->shared->getHandle()->unreference();
  }
  head.next = head.prev = &head;
  return count;
}

//////////////////////////////////////////////////////////////////////

StringData* StringData::Make(const StringData* s, CopyStringMode) {
  auto const sd = allocFlatForLenSmall(s->m_len);
  sd->m_lenAndHash = s->m_lenAndHash;
  auto const data = static_cast<void*>(sd + 1);
  *memcpy8(data, s->data(), s->m_len) = 0;

  assert(sd->same(s));
  return sd;
}

StringData* StringData::Make(StringSlice sl, CopyStringMode) {
  auto const sd = allocFlatForLenSmall(sl.len);
  sd->m_lenAndHash = sl.len; // hash=0
  auto const data = reinterpret_cast<char*>(sd + 1);

  data[sl.len] = 0;
  auto const mcret = memcpy(data, sl.ptr, sl.len);
  auto const ret = reinterpret_cast<StringData*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.

  assert(ret == sd);
  assert(ret->m_len == sl.len);
  assert(ret->hasExactlyOneRef());
  assert(ret->m_hash == 0);
  assert(ret->isFlat());
  assert(ret->checkSane());
  return ret;
}

StringData* StringData::Make(const char* data, size_t len, CopyStringMode) {
  if (UNLIKELY(len > StringData::MaxSize)) {
    throw_string_too_large(len);
  }

  return Make(StringSlice(data, len), CopyString);
}

StringData* StringData::Make(size_t reserveLen) {
  auto const sd = allocFlatForLenSmall(reserveLen);
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
    throw_string_too_large(len);
  }
  auto const sd = Make(StringSlice(data, len), CopyString);
  free(data);
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::Make(StringSlice r1, StringSlice r2) {
  auto const len = r1.len + r2.len;
  auto const sd = allocFlatForLenSmall(len);
  sd->m_lenAndHash = len; // hash=0

  auto const data = reinterpret_cast<char*>(sd + 1);
  memcpy(data, r1.ptr, r1.len);
  memcpy(data + r1.len, r2.ptr, r2.len);
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
  auto const sd = allocFlatForLenSmall(len + 6);
  sd->m_lenAndHash = len; // hash=0

  auto const data = reinterpret_cast<char*>(sd + 1);
  auto const next = memcpy8(data, s1->m_data, s1->m_len);
  *memcpy8(next, s2->m_data, s2->m_len) = 0;

  assert(sd->getCount() == 1);
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::Make(StringSlice s1, const char* lit2) {
  return Make(s1, StringSlice(lit2, strlen(lit2)));
}

StringData* StringData::Make(StringSlice r1, StringSlice r2,
                             StringSlice r3) {
  auto const len = r1.len + r2.len + r3.len;
  auto const sd = allocFlatForLenSmall(len);
  sd->m_lenAndHash  = len; // hash=0

  char* p = reinterpret_cast<char*>(sd + 1);
  p = static_cast<char*>(memcpy(p, r1.ptr, r1.len));
  p = static_cast<char*>(memcpy(p + r1.len, r2.ptr, r2.len));
  p = static_cast<char*>(memcpy(p + r2.len, r3.ptr, r3.len));
  p[r3.len] = 0;

  assert(sd->hasExactlyOneRef());
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::Make(StringSlice r1, StringSlice r2,
                             StringSlice r3, StringSlice r4) {
  auto const len = r1.len + r2.len + r3.len + r4.len;
  auto const sd = allocFlatForLenSmall(len);
  sd->m_lenAndHash = len; // hash=0

  char* p = reinterpret_cast<char*>(sd + 1);
  p = static_cast<char*>(memcpy(p, r1.ptr, r1.len));
  p = static_cast<char*>(memcpy(p + r1.len, r2.ptr, r2.len));
  p = static_cast<char*>(memcpy(p + r2.len, r3.ptr, r3.len));
  p = static_cast<char*>(memcpy(p + r3.len, r4.ptr, r4.len));
  p[r4.len] = 0;

  assert(sd->hasExactlyOneRef());
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE void StringData::enlist() {
  assert(isShared());
  auto& head = MM().getStringList();
  // insert after head
  auto const next = head.next;
  auto& payload = *sharedPayload();
  assert(uintptr_t(next) != kMallocFreeWord);
  payload.node.next = next;
  payload.node.prev = &head;
  next->prev = head.next = &payload.node;
}

NEVER_INLINE
StringData* StringData::MakeAPCSlowPath(const APCString* shared) {
  auto const sd = static_cast<StringData*>(
    MM().mallocSmallSize(sizeof(StringData) + sizeof(SharedPayload))
  );
  auto const data = shared->getStringData();
  sd->m_data = const_cast<char*>(data->m_data);
  sd->m_hdr.init(data->m_hdr, 1);
  sd->m_lenAndHash = data->m_lenAndHash;
  sd->sharedPayload()->shared = shared;
  sd->enlist();
  shared->getHandle()->reference();

  assert(sd->m_len == data->size());
  assert(sd->m_hdr.aux == data->m_hdr.aux);
  assert(sd->m_hdr.kind == HeaderKind::String);
  assert(sd->hasExactlyOneRef());
  assert(sd->m_hash == data->m_hash);
  assert(sd->isShared());
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::Make(const APCString* shared) {
  // No need to check if len > MaxSize, because if it were we'd never
  // have made the StringData in the APCVariant without throwing.
  assert(size_t(shared->getStringData()->size()) <= size_t(MaxSize));

  auto const data = shared->getStringData();
  auto const len = data->size();
  if (UNLIKELY(len > SmallStringReserve)) {
    return MakeAPCSlowPath(shared);
  }

  // small-string path: make a flat copy.
  static_assert(SmallStringReserve + kCapOverhead <= CapCode::Threshold, "");
  static_assert(SmallStringReserve + kCapOverhead == 64, "");
  auto const sd = allocFlatSmallImpl(SmallStringReserve);
  sd->m_lenAndHash = data->m_lenAndHash;

  auto const psrc = data->data();
  auto const pdst = reinterpret_cast<char*>(sd + 1);
  auto const mcret = memcpy(pdst, psrc, len + 1); // also copy the tailing 0
  auto const ret = reinterpret_cast<StringData*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.

  assert(ret == sd);
  assert(ret->m_len == len);
  assert(ret->hasExactlyOneRef());
  assert(ret->m_hash == data->m_hash);
  assert(ret->isFlat());
  assert(ret->checkSane());
  return ret;
}

NEVER_INLINE
void StringData::releaseDataSlowPath() {
  assert(!isFlat());
  assert(isShared());
  assert(checkSane());

  sharedPayload()->shared->getHandle()->unreference();
  delist();
  MM().freeSmallSize(this, sizeof(StringData) + sizeof(SharedPayload));
}

void StringData::release() noexcept {
  assert(checkSane());
  if (auto const sizeClass = m_hdr.smallSizeClass) {
    assert(isFlat());
    MM().freeSmallIndex(this, sizeClass,
                        MemoryManager::smallIndex2Size(sizeClass));
  } else {
    // We know that the string is request local.  A `smallSizeClass' of 0 in
    // its header indicates that the StringData is either not flat, or too big
    // to fit in a small size class.
    if (!isFlat()) return releaseDataSlowPath();
    auto const size = capacity() + kCapOverhead;
    assertx(size > kMaxSmallSize);
    MM().freeBigSize(this, size);
  }
}

//////////////////////////////////////////////////////////////////////

#define ALIASING_APPEND_ASSERT(ptr, len)                        \
  assert(uintptr_t(ptr) <= uintptr_t(data()) ||                 \
         uintptr_t(ptr) >= uintptr_t(data() + capacity() + 1)); \
  assert(ptr != data() || len <= m_len);

StringData* StringData::append(StringSlice range) {
  assert(!hasMultipleRefs());

  auto s = range.ptr;
  auto const len = range.len;
  if (len == 0) return this;
  auto const newLen = size_t(m_len) + size_t(len);

  if (UNLIKELY(newLen > MaxSize)) {
    throw_string_too_large(newLen);
  }

  /*
   * We may have an aliasing append.  We don't allow appending with an
   * interior pointer, although we may be asked to append less than
   * the whole string in an aliasing situation.
   */
  ALIASING_APPEND_ASSERT(s, len);

  auto const requestLen = static_cast<uint32_t>(newLen);
  auto const target = UNLIKELY(isShared()) ? escalate(requestLen)
                                           : reserve(requestLen);
  memcpy(target->mutableData() + m_len, s, len);
  target->setSize(newLen);
  assert(target->checkSane());

  return target;
}

StringData* StringData::append(StringSlice r1, StringSlice r2) {
  assert(!hasMultipleRefs());

  auto const len = r1.len + r2.len;

  if (len == 0) return this;
  if (UNLIKELY(size_t(m_len) + size_t(len) > MaxSize)) {
    throw_string_too_large(size_t(len) + size_t(m_len));
  }

  auto const newLen = m_len + len;

  /*
   * We may have an aliasing append.  We don't allow appending with an
   * interior pointer, although we may be asked to append less than
   * the whole string in an aliasing situation.
   */
  ALIASING_APPEND_ASSERT(r1.ptr, r1.len);
  ALIASING_APPEND_ASSERT(r2.ptr, r2.len);

  auto const target = UNLIKELY(isShared()) ? escalate(newLen)
                                           : reserve(newLen);

  /*
   * memcpy is safe even if it's a self append---the regions will be
   * disjoint, since rN.ptr can't point past the start of our source
   * pointer, and rN.len is smaller than the old length.
   */
  void* p = target->mutableData();
  p = memcpy((char*)p + m_len,  r1.ptr, r1.len);
      memcpy((char*)p + r1.len, r2.ptr, r2.len);

  target->setSize(newLen);
  assert(target->checkSane());

  return target;
}

StringData* StringData::append(StringSlice r1,
                               StringSlice r2,
                               StringSlice r3) {
  assert(!hasMultipleRefs());

  auto const len = r1.len + r2.len + r3.len;

  if (len == 0) return this;
  if (UNLIKELY(size_t(m_len) + size_t(len) > MaxSize)) {
    throw_string_too_large(size_t(len) + size_t(m_len));
  }

  auto const newLen = m_len + len;

  /*
   * We may have an aliasing append.  We don't allow appending with an
   * interior pointer, although we may be asked to append less than
   * the whole string in an aliasing situation.
   */
  ALIASING_APPEND_ASSERT(r1.ptr, r1.len);
  ALIASING_APPEND_ASSERT(r2.ptr, r2.len);
  ALIASING_APPEND_ASSERT(r3.ptr, r3.len);

  auto const target = UNLIKELY(isShared()) ? escalate(newLen)
                                           : reserve(newLen);

  /*
   * memcpy is safe even if it's a self append---the regions will be
   * disjoint, since rN.ptr can't point past the start of our source
   * pointer, and rN.len is smaller than the old length.
   */
  void* p = target->mutableData();
  p = memcpy((char*)p + m_len,  r1.ptr, r1.len);
  p = memcpy((char*)p + r1.len, r2.ptr, r2.len);
      memcpy((char*)p + r2.len, r3.ptr, r3.len);

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
  auto const sd = allocFlatForLenSmall(cap);

  // Request-allocated StringData are always aligned at 16 bytes, thus it is
  // safe to copy in 16-byte groups. This copies m_lenAndHash (8 bytes), the
  // characters (m_len bytes), add the trailing zero (1 byte).
  memcpy16_inline(&sd->m_lenAndHash, &m_lenAndHash,
                  (m_len + 8 + 1 + 15) & ~0xF);
  assertx(reinterpret_cast<uintptr_t>(&m_lenAndHash) + 8 ==
          reinterpret_cast<uintptr_t>(m_data));
  assertx(reinterpret_cast<uintptr_t>(&m_lenAndHash) % 16 == 0);

  assert(sd->hasExactlyOneRef());
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::shrinkImpl(size_t len) {
  assert(!isImmutable() && !hasMultipleRefs());
  assert(isFlat());
  assert(len <= capacity());

  auto const sd = allocFlatForLenSmall(len);
  sd->m_lenAndHash = len;
  auto const src = static_cast<void*>(this + 1);
  auto const dst = static_cast<void*>(sd + 1);
  *memcpy8(dst, src, len) = 0;

  assert(sd->checkSane());
  return sd;
}

StringData* StringData::shrink(size_t len) {
  if (capacity() - len > kMinShrinkThreshold) {
    return shrinkImpl(len);
  }
  assert(len < MaxSize);
  setSize(len);
  return this;
}

// State transition from Mode::Shared to Mode::Flat.
StringData* StringData::escalate(size_t cap) {
  assert(isShared() && !isStatic() && cap >= m_len);

  auto const sd = allocFlatForLenSmall(cap);
  sd->m_lenAndHash = m_lenAndHash;
  auto const data = reinterpret_cast<char*>(sd + 1);
  *memcpy8(data, m_data, m_len) = 0;

  assert(sd->hasExactlyOneRef());
  assert(sd->isFlat());
  assert(sd->checkSane());
  return sd;
}

void StringData::dump() const {
  StringSlice s = slice();

  printf("StringData(%d) (%s%s%d): [", getCount(),
         isShared() ? "shared " : "",
         isStatic() ? "static " : "",
         s.len);
  for (uint32_t i = 0; i < s.len; i++) {
    char ch = s.ptr[i];
    if (isprint(ch)) {
      printf("%c", ch);
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
  StringSlice s = slice();
  m_hash = hash_string_unsafe(s.ptr, s.len);
  assert(m_hash >= 0);
  if (s.len > 0 &&
      (is_numeric_string(s.ptr, s.len, nullptr, nullptr,
                         1, nullptr) == KindOfNull)) {
    m_hash |= STRHASH_MSB;
  }
}

void StringData::setStatic() {
  setRefCount(StaticValue);
  preCompute();
}

void StringData::setUncounted() {
  setRefCount(UncountedValue);
  preCompute();
}

NEVER_INLINE strhash_t StringData::hashHelper() const {
  assert(!isShared());
  strhash_t h = hash_string_i_unsafe(m_data, m_len);
  assert(h >= 0);
  m_hash |= h;
  return h;
}

///////////////////////////////////////////////////////////////////////////////
// type conversions

DataType StringData::isNumericWithVal(int64_t &lval, double &dval,
                                      int allow_errors, int* overflow) const {
  if (m_hash < 0) return KindOfNull;
  DataType ret = KindOfNull;
  StringSlice s = slice();
  if (s.len) {
    ret = is_numeric_string(s.ptr, s.len, &lval, &dval, allow_errors, overflow);
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
    case KindOfNull:
      return false;
    case KindOfInt64:
    case KindOfDouble:
      return true;
    case KindOfUninit:
    case KindOfBoolean:
    case KindOfStaticString:
    case KindOfString:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
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
    case KindOfStaticString:
    case KindOfString:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
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

  int oflow1, oflow2;
  int64_t lval1, lval2;
  double dval1, dval2;
  DataType ret1, ret2;
  if ((ret1 = isNumericWithVal(lval1, dval1, 0, &oflow1)) == KindOfNull ||
      (ret1 == KindOfDouble && !finite(dval1)) ||
      (ret2 = v2->isNumericWithVal(lval2, dval2, 0, &oflow2)) == KindOfNull ||
      (ret2 == KindOfDouble && !finite(dval2))) {
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
  StringSlice s = slice();
  return std::string(s.ptr, s.len);
}

bool StringData::checkSane() const {
  static_assert(sizeof(StringData) == 24,
                "StringData size changed---update assertion if you mean it");
  static_assert(size_t(MaxSize) <= size_t(INT_MAX), "Beware int wraparound");
  static_assert(offsetof(StringData, m_hdr) == HeaderOffset, "");

  assert(uint32_t(size()) <= MaxSize);
  assert(capacity() <= MaxSize);
  assert(size() >= 0);
  assert(size() <= capacity());
  // isFlat() and isShared() both check whether m_data == voidPayload,
  // which guarantees by definition that isFlat() != isShared()
  return true;
}

//////////////////////////////////////////////////////////////////////

}
