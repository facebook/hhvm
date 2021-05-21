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
#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-uncounted.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

#include "hphp/zend/zend-strtod.h"

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
  auto sd = static_cast<StringData*>(tl_heap->objMallocIndex(sizeIndex));
  // Refcount initialized to 1.
  sd->initHeader_16(HeaderKind::String, OneReference, sizeIndex);
  assertx(sd->capacity() >= len);
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

namespace {
std::atomic<bool> s_symbols_loaded;

SymbolPrefix* getSymbolPrefix(StringData* sd) {
  assertx(sd->isSymbol());
  return reinterpret_cast<SymbolPrefix*>(sd) - 1;
}
const SymbolPrefix* getSymbolPrefix(const StringData* sd) {
  assertx(sd->isSymbol());
  return getSymbolPrefix(const_cast<StringData*>(sd));
}
}

bool StringData::isSymbol() const {
  return (m_aux16 >> 8) & kIsSymbolMask;
}

void StringData::markSymbolsLoaded() {
  s_symbols_loaded.store(true, std::memory_order_release);
}

Class* StringData::getCachedClass() const {
  return getSymbolPrefix(this)->cls;
}

NamedEntity* StringData::getNamedEntity() const {
  return getSymbolPrefix(this)->ne;
}

void StringData::setCachedClass(Class* cls) {
  auto const prefix = getSymbolPrefix(this);
  assertx(IMPLIES(prefix->cls, prefix->cls == cls));
  prefix->cls = cls;
}

void StringData::setNamedEntity(NamedEntity* ne) {
  auto const prefix = getSymbolPrefix(this);
  assertx(IMPLIES(prefix->ne, prefix->ne == ne));
  prefix->ne = ne;
}

ptrdiff_t StringData::isSymbolOffset() {
  return offsetof(StringData, m_aux16) + 1;
}

ptrdiff_t StringData::cachedClassOffset() {
  return offsetof(SymbolPrefix, cls) - sizeof(SymbolPrefix);
}

//////////////////////////////////////////////////////////////////////

ptrdiff_t StringData::colorOffset() {
  return offsetof(StringData, m_aux16);
}

uint16_t StringData::color() const {
  return m_aux16 & kColorMask;
}

void StringData::setColor(uint16_t color) {
  assertx((color & ~kColorMask) == 0);
  m_aux16 |= color;
}

//////////////////////////////////////////////////////////////////////

// Create either a static or an uncounted string.
// Diffrence between static and uncounted is in the lifetime
// of the string. Static are alive for the lifetime of the process.
// Uncounted are not ref counted but will be deleted at some point.
template <bool trueStatic> ALWAYS_INLINE
MemBlock StringData::AllocateShared(folly::StringPiece sl) {
  if (UNLIKELY(sl.size() > StringData::MaxSize)) {
    raiseStringLengthExceededError(sl.size());
  }

  auto const symbol =
    trueStatic && !s_symbols_loaded.load(std::memory_order_acquire);

  auto const extra = symbol ? sizeof(SymbolPrefix) : 0;
  auto const bytes = sl.size() + kStringOverhead + extra;
  auto const ptr = trueStatic ? static_alloc(bytes) : AllocUncounted(bytes);
  return MemBlock{ptr, bytes};
}

template <bool trueStatic> ALWAYS_INLINE
StringData* StringData::MakeSharedAt(folly::StringPiece sl, MemBlock range) {
  assertx(range.size >= sl.size() + kStringOverhead);
  auto const symbol = trueStatic &&
    !s_symbols_loaded.load(std::memory_order_acquire) &&
    (range.size >= sl.size() + kStringOverhead + sizeof(SymbolPrefix));
  auto const extra = symbol ? sizeof(SymbolPrefix) : 0;
  StringData* sd = reinterpret_cast<StringData*>(
    reinterpret_cast<uintptr_t>(range.ptr) + extra
  );
  auto const data = reinterpret_cast<char*>(sd + 1);

#ifndef NO_M_DATA
  sd->m_data = data;
#endif
  auto const count = trueStatic ? StaticValue : UncountedValue;
  if (symbol) {
    auto constexpr aux = kIsSymbolMask << 8 | kInvalidColor;
    sd->initHeader_16(HeaderKind::String, count, aux);
    getSymbolPrefix(sd)->cls = nullptr;
    getSymbolPrefix(sd)->ne = nullptr;
  } else {
    sd->initHeader_16(HeaderKind::String, count, kInvalidColor);
  }
  sd->m_len = sl.size(); // m_hash is computed soon.

  data[sl.size()] = 0;
  auto const mcret = memcpy(data, sl.data(), sl.size());
  auto const ret = reinterpret_cast<StringData*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.
  ret->preCompute();                    // get m_hash right

  assertx(ret == sd);
  assertx(ret->isFlat());
  assertx(trueStatic ? ret->isStatic() : ret->isUncounted());
  assertx(ret->isSymbol() == symbol);
  assertx(ret->checkSane());
  return ret;
}

StringData* StringData::MakeStaticAt(folly::StringPiece sl, MemBlock range) {
  return MakeSharedAt<true>(sl, range);
}

StringData* StringData::MakeStatic(folly::StringPiece sl) {
  assertx(StaticString::s_globalInit);
  return MakeStaticAt(sl, AllocateShared<true>(sl));
}

StringData* StringData::MakeUncounted(folly::StringPiece sl) {
  return MakeSharedAt<false>(sl, AllocateShared<false>(sl));
}

StringData* StringData::MakeEmpty() {
  return MakeStaticAt(folly::StringPiece{""},
                      MemBlock{&s_theEmptyString, sizeof(s_theEmptyString)});
}

void StringData::destructStatic() {
  assertx(checkSane() && isStatic());
  assertx(isFlat());
  if (isSymbol()) {
    static_try_free(reinterpret_cast<SymbolPrefix*>(this) - 1,
                    size() + kStringOverhead + sizeof(SymbolPrefix));
  } else {
    static_try_free(this, size() + kStringOverhead);
  }
}

void StringData::ReleaseUncounted(StringData* str) {
  assertx(str->isFlat());
  assertx(str->checkSane());
  assertx(!str->uncountedCowCheck());
  FreeUncounted(str, str->size() + kStringOverhead);
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE void StringData::delist() {
  assertx(isProxy());
  auto& payload = *proxy();
  auto const next = payload.node.next;
  auto const prev = payload.node.prev;
  assertx(uintptr_t(next) != kMallocFreeWord);
  assertx(uintptr_t(prev) != kMallocFreeWord);
  next->prev = prev;
  prev->next = next;
}

unsigned StringData::sweepAll() {
  auto& head = tl_heap->getStringList();
  auto count = 0;
  for (StringDataNode *next, *n = head.next; n != &head; n = next) {
    count++;
    next = n->next;
    assertx(next && uintptr_t(next) != kSmallFreeWord);
    assertx(next && uintptr_t(next) != kMallocFreeWord);
    auto const s = node2str(n);
    assertx(s->isProxy());
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

  assertx(sd->same(s));
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

  assertx(ret == sd);
  assertx(ret->m_len == sl.size());
  assertx(ret->hasExactlyOneRef());
  assertx(ret->m_hash == 0);
  assertx(ret->isFlat());
  assertx(ret->checkSane());
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

  assertx(sd->hasExactlyOneRef());
  assertx(sd->isFlat());
  assertx(sd->checkSane());
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
  assertx(sd->checkSane());
  return sd;
}

StringData* StringData::Make(folly::StringPiece r1, folly::StringPiece r2) {
  // Undefined behavior if we pass nullptr strings into StringData::Make
  assertx(r1.data() && r2.data());
  auto const len = r1.size() + r2.size();
  auto const sd = allocFlat(len);
  sd->m_lenAndHash = len; // hash=0

  auto const data = reinterpret_cast<char*>(sd + 1);
  memcpy(data,             r1.data(), r1.size());
  memcpy(data + r1.size(), r2.data(), r2.size());
  data[len] = 0;

  assertx(sd->hasExactlyOneRef());
  assertx(sd->isFlat());
  assertx(sd->checkSane());
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

  assertx(sd->hasExactlyOneRef());
  assertx(sd->isFlat());
  assertx(sd->checkSane());
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

  assertx(sd->hasExactlyOneRef());
  assertx(sd->isFlat());
  assertx(sd->checkSane());
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

  assertx(sd->hasExactlyOneRef());
  assertx(sd->isFlat());
  assertx(sd->checkSane());
  return sd;
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE void StringData::enlist() {
  assertx(isProxy());
  auto& head = tl_heap->getStringList();
  // insert after head
  auto const next = head.next;
  auto& payload = *proxy();
  assertx(uintptr_t(next) != kMallocFreeWord);
  payload.node.next = next;
  payload.node.prev = &head;
  next->prev = head.next = &payload.node;
}

StringData* StringData::MakeProxy(const APCString* apcstr) {
#ifdef NO_M_DATA
  always_assert(false);
  not_reached();
#else
  assertx(!apcExtension::UseUncounted);
  // No need to check if len > MaxSize, because if it were we'd never
  // have made the StringData in the APCVariant without throwing.
  assertx(size_t(apcstr->getStringData()->size()) <= size_t(MaxSize));

  auto const sd = static_cast<StringData*>(
    tl_heap->mallocSmallSize(sizeof(StringData) + sizeof(Proxy))
  );
  auto const data = apcstr->getStringData();
  sd->m_data = const_cast<char*>(data->m_data);
  sd->initHeader(*data, OneReference);
  sd->m_lenAndHash = data->m_lenAndHash;
  sd->proxy()->apcstr = apcstr;
  sd->enlist();
  apcstr->reference();

  assertx(sd->m_len == data->size());
  assertx(sd->m_aux16 == data->m_aux16);
  assertx(sd->m_kind == HeaderKind::String);
  assertx(sd->hasExactlyOneRef());
  assertx(sd->m_hash == data->m_hash);
  assertx(sd->isProxy());
  assertx(sd->checkSane());
  return sd;
#endif
}

void StringData::unProxy() {
  assertx(isProxy());
  proxy()->apcstr->unreference();
  delist();
}

NEVER_INLINE
void StringData::releaseProxy() {
  unProxy();
  tl_heap->freeSmallSize(this, sizeof(StringData) + sizeof(Proxy));
}

void StringData::release() noexcept {
  fixCountForRelease();
  assertx(isRefCounted());
  assertx(checkSane());
  if (UNLIKELY(!isFlat())) {
    releaseProxy();
    AARCH64_WALKABLE_FRAME();
    return;
  }
  tl_heap->objFreeIndex(this, m_aux16);
  AARCH64_WALKABLE_FRAME();
}

//////////////////////////////////////////////////////////////////////

#define ALIASING_APPEND_ASSERT(ptr, len)                        \
  assertx(uintptr_t(ptr) <= uintptr_t(data()) ||                 \
         uintptr_t(ptr) >= uintptr_t(data() + capacity() + 1)); \
  assertx(ptr != data() || len <= m_len);

StringData* StringData::append(folly::StringPiece range) {
  assertx(!isImmutable() && !hasMultipleRefs());

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
  assertx(target->checkSane());

  return target;
}

StringData* StringData::append(folly::StringPiece r1, folly::StringPiece r2) {
  assertx(!isImmutable() && !hasMultipleRefs());

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
  assertx(target->checkSane());

  return target;
}

StringData* StringData::append(folly::StringPiece r1,
                               folly::StringPiece r2,
                               folly::StringPiece r3) {
  assertx(!isImmutable() && !hasMultipleRefs());

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
  assertx(target->checkSane());

  return target;
}

#undef ALIASING_APPEND_ASSERT

//////////////////////////////////////////////////////////////////////

StringData* StringData::reserve(size_t cap) {
  assertx(!isImmutable() && !hasMultipleRefs());
  assertx(isFlat());

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

  assertx(sd->hasExactlyOneRef());
  assertx(sd->isFlat());
  assertx(sd->checkSane());
  return sd;
}

StringData* StringData::shrinkImpl(size_t len) {
  assertx(!isImmutable() && !hasMultipleRefs());
  assertx(isFlat());
  assertx(len <= capacity());

  auto const sd = allocFlat(len);
  sd->m_lenAndHash = len;
  auto const src = static_cast<void*>(this + 1);
  auto const dst = static_cast<void*>(sd + 1);
  *memcpy8(dst, src, len) = 0;

  assertx(sd->checkSane());
  return sd;
}

StringData* StringData::shrink(size_t len) {
  assertx(!isImmutable() && !hasMultipleRefs());
  if (capacity() - len > kMinShrinkThreshold) {
    return shrinkImpl(len);
  }
  assertx(len < MaxSize);
  setSize(len);
  return this;
}

// State transition from Mode::Shared to Mode::Flat.
StringData* StringData::escalate(size_t cap) {
  assertx(isProxy() && !isStatic() && cap >= m_len);

  auto const sd = allocFlat(cap);
  sd->m_lenAndHash = m_lenAndHash;
  auto const sd_data = reinterpret_cast<char*>(sd + 1);
  *memcpy8(sd_data, data(), m_len) = 0;

  assertx(sd->hasExactlyOneRef());
  assertx(sd->isFlat());
  assertx(sd->checkSane());
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
  assertx(!isImmutable() && !hasMultipleRefs());
  assertx(!empty());

  auto const sd = UNLIKELY(isProxy())
    ? escalate(m_len + 1)
    : reserve(m_len + 1);
  sd->incrementHelper();
  return sd;
}

void StringData::incrementHelper() {
  raise_notice("Increment on string '%s'", data());
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

    assertx(len + 1 <= capacity());
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
  assertx(m_hash >= 0);
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
  assertx(!isProxy());
  strhash_t h = hash_string_i_unsafe(data(), m_len);
  assertx(h >= 0);
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
      assertx(!isProxy());
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
    case KindOfObject:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfClass:
    case KindOfLazyClass:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfRecord:
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
    case KindOfObject:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfClass:
    case KindOfLazyClass:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfRecord:
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
  assertx(s);
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
  assertx(v2);

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
    assertx(ret2 == KindOfInt64);
    if (oflow1) {
      return oflow1;
    }
    dval2 = (double)lval2;
  } else {
    assertx(ret1 == KindOfInt64);
    assertx(ret2 == KindOfDouble);
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
  assertx(v2);

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

StringData*
StringData::substr(int start, int length /* = StringData::MaxSize */) {
  if (start < 0 || start >= size() || length <= 0) {
    return staticEmptyString();
  }

  auto const max_len = size() - start;
  if (length > max_len) {
    length = max_len;
  }

  assertx(length > 0);
  if (UNLIKELY(length == size())) {
    incRefCount();
    return this;
  }
  if (UNLIKELY(length == 1)) {
    return makeStaticString(data()[start]);
  }
  return StringData::Make(data() + start, length, CopyString);
}

///////////////////////////////////////////////////////////////////////////////
// Debug

std::string StringData::toCppString() const {
  auto s = slice();
  return std::string(s.data(), s.size());
}

bool StringData::checkSane() const {
  static_assert(sizeof(StringData) == (use_lowptr ? 16 : 24),
                "StringData size changed---update assertion if you mean it");
  static_assert(size_t(MaxSize) <= size_t(INT_MAX), "Beware int wraparound");
#ifdef NO_M_DATA
  static_assert(sizeof(StringData) == SD_DATA, "");
  static_assert(offsetof(StringData, m_len) == SD_LEN, "");
  static_assert(offsetof(StringData, m_hash) == SD_HASH, "");
#endif
  assertx(kindIsValid());
  assertx(uint32_t(size()) <= MaxSize);
  assertx(size() >= 0);
  assertx(IMPLIES(isSymbol(), isStatic()));
  if (!isImmutable()) {
    assertx(size() <= capacity());
    assertx(capacity() <= MaxSize);
  }
  // isFlat() and isProxy() both check whether m_data == payload(),
  // which guarantees by definition that isFlat() != isProxy()
  return true;
}

//////////////////////////////////////////////////////////////////////

}
