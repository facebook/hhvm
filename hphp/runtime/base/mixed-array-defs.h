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

#ifndef incl_HPHP_HPHP_ARRAY_DEFS_H_
#define incl_HPHP_HPHP_ARRAY_DEFS_H_

#include "hphp/runtime/base/mixed-array.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-iterator-defs.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/struct-array.h"
#include "hphp/runtime/base/struct-array-defs.h"

#include "hphp/util/stacktrace-profiler.h"
#include "hphp/util/word-mem.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Return the payload from a ArrayData* that is kMixedKind.
 */
ALWAYS_INLINE
MixedArray::Elm* mixedData(const MixedArray* arr) {
  return reinterpret_cast<MixedArray::Elm*>(
    const_cast<MixedArray*>(arr) + 1
  );
}

ALWAYS_INLINE int32_t* mixedHash(MixedArray::Elm* data, uint32_t scale) {
  return reinterpret_cast<int32_t*>(data + static_cast<size_t>(scale) * 3);
}

template<class F> void MixedArray::scan(F& mark) const {
  if (isZombie()) return;
  auto data = this->data();
  for (unsigned i = 0, n = m_used; i < n; i++) {
    auto& e = data[i];
    if (MixedArray::isTombstone(e.data.m_type)) continue;
    if (e.hasStrKey()) mark(e.skey);
    mark(e.data);
  }
}

ALWAYS_INLINE
MixedArray* getArrayFromMixedData(const MixedArray::Elm* elms) {
  // Note: changes to this scheme will require changes in the JIT for
  // LdColArray.
  auto a = const_cast<MixedArray*>(
    reinterpret_cast<const MixedArray*>(elms) - 1
  );
  assert(mixedData(a) == elms);
  return a;
}

inline ArrayData::~ArrayData() {
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(this);
  }
}

inline bool validPos(ssize_t pos) {
  return pos >= 0;
}

inline bool validPos(int32_t pos) {
  return pos >= 0;
}

ALWAYS_INLINE
bool MixedArray::isFull() const {
  assert(!isPacked());
  assert(m_used <= capacity());
  return m_used == capacity();
}

ALWAYS_INLINE
void MixedArray::InitSmall(MixedArray* a, RefCount count, uint32_t size,
                           int64_t nextIntKey) {
  assert(count != 0);
  // Intentionally initialize hash table before header.
#ifdef __x86_64__
  static_assert(MixedArray::Empty == -1, "");
  static_assert(MixedArray::SmallSize == 3, "");
  static_assert(sizeof(MixedArray) +
                MixedArray::SmallSize * sizeof(MixedArray::Elm) == 104, "");
  __asm__ __volatile__(
    "pcmpeqd    %%xmm0, %%xmm0\n"          // xmm0 <- 11111....
    "movdqu     %%xmm0, 104(%0)\n"
    : : "r"(a) : "xmm0"
  );
#else
  auto const hash = a->hashTab();
  auto const emptyVal = int64_t{MixedArray::Empty};
  reinterpret_cast<int64_t*>(hash)[0] = emptyVal;
  reinterpret_cast<int64_t*>(hash)[1] = emptyVal;
#endif
  a->m_sizeAndPos = size; // pos=0
  a->m_hdr.init(HeaderKind::Mixed, count);
  a->m_scale_used = MixedArray::SmallScale | uint64_t(size) << 32;
  a->m_nextKI = nextIntKey;
}

inline void MixedArray::initHash(int32_t* hash, uint32_t scale) {
#if defined(__x86_64__)
  static_assert(Empty == -1, "The following fills with all 1's.");
  assertx(HashSize(scale) == scale * 4);

  uint64_t offset = scale * 16;
  __asm__ __volatile__(
    "pcmpeqd    %%xmm0, %%xmm0\n"          // xmm0 <- 11111....
    ".l%=:\n"
    "sub        $0x10, %0\n"
    "movdqu     %%xmm0, (%1, %0)\n"
    "ja         .l%=\n"
    : "+r"(offset) : "r"(hash) : "xmm0"
  );
#else
  wordfill(hash, Empty, HashSize(scale));
#endif
}

inline void
MixedArray::copyHash(int32_t* to, const int32_t* from, uint32_t scale) {
  assertx(HashSize(scale) == scale * 4);
  uint64_t nBytes = scale * 16;
  memcpy16_inline(to, from, nBytes);
}

inline void
MixedArray::copyElmsNextUnsafe(MixedArray* to, const MixedArray* from,
                               uint32_t nElems) {
  static_assert(offsetof(MixedArray, m_nextKI) + 8 == sizeof(MixedArray),
                "Revisit this if MixedArray layout changes");
  static_assert(sizeof(Elm) == 24, "");
  // Copy `m_nextKI' (8 bytes), data (oldUsed * 24), and optionally 24 more
  // bytes to make sure we can use bcopy32(), which rounds the length down to
  // 32-byte chunks. The additional bytes are guaranteed not to exceed the
  // space allocated for the array, because the hash table has at least 16
  // bytes, and when it is only 16 bytes (capacity = 3), we overrun the buffer
  // by only 16 bytes instead of 24.
  bcopy32_inline(&(to->m_nextKI), &(from->m_nextKI), sizeof(Elm) * nElems + 32);
}

extern int32_t* warnUnbalanced(size_t n, int32_t* ei);

ALWAYS_INLINE int32_t*
MixedArray::findForNewInsertCheckUnbalanced(int32_t* table, size_t mask,
                                            size_t h0) const {
  assert(!isPacked());
  uint32_t balanceLimit = RuntimeOption::MaxArrayChain;
  for (uint32_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) {
      return LIKELY(i <= balanceLimit) ? ei : warnUnbalanced(i, ei);
    }
    probe += i;
    assertx(i <= mask);
    assertx(probe == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

ALWAYS_INLINE int32_t*
MixedArray::findForNewInsert(int32_t* table, size_t mask, size_t h0) const {
  assert(!isPacked());
  for (uint32_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) return ei;
    probe += i;
    assertx(i <= mask);
    assertx(probe == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

ALWAYS_INLINE
int32_t* MixedArray::findForNewInsert(size_t h0) const {
  return findForNewInsert(hashTab(), mask(), h0);
}

inline bool MixedArray::isTombstone(ssize_t pos) const {
  assert(size_t(pos) <= m_used);
  return isTombstone(data()[pos].data.m_type);
}

ALWAYS_INLINE
void MixedArray::getElmKey(const Elm& e, TypedValue* out) {
  if (e.hasIntKey()) {
    out->m_data.num = e.ikey;
    out->m_type = KindOfInt64;
    return;
  }
  auto str = e.skey;
  out->m_data.pstr = str;
  out->m_type = KindOfString;
  str->incRefCount();
}

ALWAYS_INLINE
void MixedArray::getArrayElm(ssize_t pos,
                            TypedValue* valOut,
                            TypedValue* keyOut) const {
  assert(size_t(pos) < m_used);
  assert(!isPacked());
  auto& elm = data()[pos];
  TypedValue* cur = tvToCell(&elm.data);
  cellDup(*cur, *valOut);
  getElmKey(elm, keyOut);
}

ALWAYS_INLINE
void MixedArray::getArrayElm(ssize_t pos, TypedValue* valOut) const {
  assert(size_t(pos) < m_used);
  auto& elm = data()[pos];
  TypedValue* cur = tvToCell(&elm.data);
  cellDup(*cur, *valOut);
}

ALWAYS_INLINE
void MixedArray::dupArrayElmWithRef(ssize_t pos,
                                   TypedValue* valOut,
                                   TypedValue* keyOut) const {
  auto& elm = data()[pos];
  tvDupWithRef(elm.data, *valOut);
  getElmKey(elm, keyOut);
}

ALWAYS_INLINE
MixedArray::Elm& MixedArray::allocElm(int32_t* ei) {
  assert(!validPos(*ei) && !isFull());
  assert(m_size == 0 || m_used != 0);
  ++m_size;
  size_t i = m_used;
  (*ei) = i;
  m_used = i + 1;
  return data()[i];
}

inline MixedArray* MixedArray::asMixed(ArrayData* ad) {
  assert(ad->isMixed());
  auto a = static_cast<MixedArray*>(ad);
  assert(a->checkInvariants());
  return a;
}

inline const MixedArray* MixedArray::asMixed(const ArrayData* ad) {
  assert(ad->isMixed());
  auto a = static_cast<const MixedArray*>(ad);
  assert(a->checkInvariants());
  return a;
}

inline size_t MixedArray::hashSize() const {
  return HashSize(m_scale);
}

inline ArrayData* MixedArray::addVal(int64_t ki, Cell data) {
  assert(!exists(ki));
  assert(!isPacked());
  assert(!isFull());
  auto ei = findForNewInsert(ki);
  auto& e = allocElm(ei);
  e.setIntKey(ki);
  if (ki >= m_nextKI && m_nextKI >= 0) m_nextKI = ki + 1;
  cellDup(data, e.data);
  // TODO(#3888164): should avoid needing these KindOfUninit checks.
  if (UNLIKELY(e.data.m_type == KindOfUninit)) {
    e.data.m_type = KindOfNull;
  }
  return this;
}

inline ArrayData* MixedArray::addVal(StringData* key, Cell data) {
  assert(!exists(key));
  assert(!isPacked());
  assert(!isFull());
  return addValNoAsserts(key, data);
}

inline ArrayData* MixedArray::addValNoAsserts(StringData* key, Cell data) {
  strhash_t h = key->hash();
  auto ei = findForNewInsert(h);
  auto& e = allocElm(ei);
  e.setStrKey(key, h);
  cellDup(data, e.data);
  // TODO(#3888164): should refactor to avoid making KindOfUninit checks.
  if (UNLIKELY(e.data.m_type == KindOfUninit)) {
    e.data.m_type = KindOfNull;
  }
  return this;
}

inline MixedArray::Elm& MixedArray::addKeyAndGetElem(StringData* key) {
  strhash_t h = key->hash();
  auto ei = findForNewInsert(h);
  auto& e = allocElm(ei);
  e.setStrKey(key, h);
  return e;
}

template <class K>
ArrayData* MixedArray::updateRef(K k, Variant& data) {
  assert(!isPacked());
  assert(!isFull());
  auto p = insert(k);
  if (p.found) {
    tvBind(data.asRef(), &p.tv);
    return this;
  }
  tvAsUninitializedVariant(&p.tv).constructRefHelper(data);
  return this;
}

template <class K>
ArrayData* MixedArray::addLvalImpl(K k, Variant*& ret) {
  assert(!isPacked());
  assert(!isFull());
  auto p = insert(k);
  if (!p.found) tvWriteNull(&p.tv);
  ret = &tvAsVariant(&p.tv);
  return this;
}

//////////////////////////////////////////////////////////////////////

struct MixedArray::ValIter {

  ALWAYS_INLINE
  static bool isMixed(const ArrayData::ArrayKind& kind) {
    return kind == ArrayData::kMixedKind;
  }

  explicit ValIter(ArrayData* arr)
    : m_arr(arr)
    , m_kind(arr->kind())
  {
    assert(isMixed(m_kind) || m_kind == kPackedKind || m_kind == kStructKind);
    if (isMixed(m_kind)) {
      m_iterMixed = asMixed(arr)->data();
      m_stopMixed = m_iterMixed + asMixed(arr)->m_used;
    } else if (m_kind == kStructKind) {
      auto structArray = StructArray::asStructArray(arr);
      m_iterStruct = structArray->data();
      m_stopStruct = m_iterStruct + structArray->size();
    } else {
      m_iterPacked = reinterpret_cast<TypedValue*>(arr + 1);
      m_stopPacked = m_iterPacked + arr->m_size;
    }
  }

  explicit ValIter(ArrayData* arr, ssize_t start_pos)
    : m_arr(arr)
    , m_kind(arr->kind())
  {
    assert(isMixed(m_kind) || m_kind == kPackedKind || m_kind == kStructKind);
    if (isMixed(m_kind)) {
      m_iterMixed = asMixed(arr)->data() + start_pos;
      m_stopMixed = asMixed(arr)->data() + asMixed(arr)->m_used;
      assert(m_iterMixed <= m_stopMixed);
     } else if (m_kind == kStructKind) {
      auto structArray = StructArray::asStructArray(arr);
      m_iterStruct = structArray->data() + start_pos;
      m_stopStruct = structArray->data() + arr->size();
      assert(m_iterStruct <= m_stopStruct);
    } else {
      m_iterPacked = reinterpret_cast<TypedValue*>(arr + 1) + start_pos;
      m_stopPacked = reinterpret_cast<TypedValue*>(arr + 1) + arr->m_size;
      assert(m_iterPacked <= m_stopPacked);
    }
  }

   TypedValue* current() const {
     return UNLIKELY(isMixed(m_kind)) ? &currentElm()->data
                                      : m_iterPacked;
   }

   Elm* currentElm() const {
     assert(isMixed(m_kind));
     return m_iterMixed;
   }

   bool empty() const {
     return isMixed(m_kind) ? m_iterMixed == m_stopMixed
                            : m_iterPacked == m_stopPacked;
   }

   void advance() {
     if (UNLIKELY(isMixed(m_kind))) {
       do {
         ++m_iterMixed;
       } while (!empty() && MixedArray::isTombstone(m_iterMixed->data.m_type));
      return;
    }
    ++m_iterPacked;
  }

  ssize_t currentPos() const {
    if (isMixed(m_kind)) return m_iterMixed - asMixed(m_arr)->data();
    if (m_kind == kStructKind) {
      return m_iterStruct - StructArray::asStructArray(m_arr)->data();
    }
    return m_iterPacked - reinterpret_cast<TypedValue*>(m_arr + 1);
  }

private:
  ArrayData* const m_arr;
  ArrayData::ArrayKind const m_kind;
  union {
    Elm* m_iterMixed;
    TypedValue* m_iterPacked;
    TypedValue* m_iterStruct;
  };
  union {
    Elm* m_stopMixed;
    TypedValue* m_stopPacked;
    TypedValue* m_stopStruct;
  };
};

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
uint32_t computeScaleFromSize(uint32_t n) {
  assert(n <= 0x7fffffffU);
  auto scale = MixedArray::SmallScale;
  while (MixedArray::Capacity(scale) < n) scale *= 2;
  return scale;
  static_assert(MixedArray::SmallHashSize >= 4,
                "lower limit for 0.75 load factor");
}

ALWAYS_INLINE
MixedArray* reqAllocArray(uint32_t scale) {
  auto const allocBytes = computeAllocBytes(scale);
  return static_cast<MixedArray*>(MM().objMalloc(allocBytes));
}

ALWAYS_INLINE
MixedArray* staticAllocArray(uint32_t scale) {
  auto const allocBytes = computeAllocBytes(scale);
  return static_cast<MixedArray*>(std::malloc(allocBytes));
}

ALWAYS_INLINE
size_t MixedArray::heapSize() const {
  return computeAllocBytes(m_scale);
}

//////////////////////////////////////////////////////////////////////

}

#endif
