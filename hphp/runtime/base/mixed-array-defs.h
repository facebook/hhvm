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

ALWAYS_INLINE
MixedArray* getArrayFromMixedData(const MixedArray::Elm* elms) {
  auto* a = const_cast<MixedArray*>(
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
  assert(m_used <= m_cap);
  return m_used == m_cap;
}

inline void MixedArray::initHash(int32_t* hash, size_t tableSize) {
  wordfill(hash, Empty, tableSize);
}

inline int32_t*
MixedArray::copyHash(int32_t* to, const int32_t* from, size_t count) {
  return wordcpy(to, from, count);
}

inline MixedArray::Elm*
MixedArray::copyElms(Elm* to, const Elm* from, size_t count) {
  return wordcpy(to, from, count);
}

extern int32_t* warnUnbalanced(size_t n, int32_t* ei);

ALWAYS_INLINE int32_t*
MixedArray::findForNewInsertCheckUnbalanced(int32_t* table, size_t mask,
                                            size_t h0) const {
  assert(!isPacked());
  size_t balanceLimit = size_t(RuntimeOption::MaxArrayChain);
  for (size_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) {
      return LIKELY(i <= balanceLimit) ? ei : warnUnbalanced(i, ei);
    }
    probe += i;
    assert(i <= mask && probe == h0 + ((i + i * i) / 2));
  }
}

ALWAYS_INLINE int32_t*
MixedArray::findForNewInsert(int32_t* table, size_t mask, size_t h0) const {
  assert(!isPacked());
  for (size_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) return ei;
    probe += i;
    assert(i <= mask && probe == h0 + ((i + i * i) / 2));
  }
}

ALWAYS_INLINE
int32_t* MixedArray::findForNewInsert(size_t h0) const {
  return findForNewInsert(hashTab(), m_tableMask, h0);
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
  return m_tableMask + 1;
}

inline size_t MixedArray::computeMaxElms(uint32_t tableMask) {
  return size_t(tableMask) - size_t(tableMask) / LoadScale;
}

inline size_t MixedArray::computeDataSize(uint32_t tableMask) {
  return (tableMask + 1) * sizeof(int32_t) +
         computeMaxElms(tableMask) * sizeof(Elm);
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
    return kind == ArrayData::kMixedKind ||
      kind == ArrayData::kIntMapKind;
  }

  explicit ValIter(ArrayData* arr)
    : m_arr(arr)
    , m_kind(arr->m_kind)
  {
    assert(isMixed(m_kind) || m_kind == kPackedKind);
    if (isMixed(m_kind)) {
      m_iterMixed = asMixed(arr)->data();
      m_stopMixed = m_iterMixed + asMixed(arr)->m_used;
     } else {
       m_iterPacked = reinterpret_cast<TypedValue*>(arr + 1);
       m_stopPacked = m_iterPacked + arr->m_size;
     }
   }

   explicit ValIter(ArrayData* arr, ssize_t start_pos)
     : m_arr(arr)
     , m_kind(arr->m_kind)
   {
     assert(isMixed(m_kind) || m_kind == kPackedKind);
     if (isMixed(m_kind)) {
       m_iterMixed = asMixed(arr)->data() + start_pos;
       m_stopMixed = asMixed(arr)->data() + asMixed(arr)->m_used;
       assert(m_iterMixed <= m_stopMixed);
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
    return m_iterPacked - reinterpret_cast<TypedValue*>(m_arr + 1);
  }

private:
  ArrayData* const m_arr;
  ArrayData::ArrayKind const m_kind;
  union { Elm* m_iterMixed; TypedValue* m_iterPacked; };
  union { Elm* m_stopMixed; TypedValue* m_stopPacked; };
};

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
uint32_t computeMaskFromNumElms(uint32_t n) {
  assert(n <= 0x7fffffffU);
  auto lgSize = MixedArray::MinLgTableSize;
  auto maxElms = MixedArray::SmallSize;
  assert(lgSize >= 2);

  // Note: it's tempting to convert this loop into something involving
  // x64 bsr and a shift.  Naive attempts currently actually add more
  // branches, because we need to initially check whether `n' is less
  // than SmallSize, and after finding the next power of two we need a
  // branch to see if it was big enough for the desired load factor.
  // This is probably still worth revisiting (e.g., MakeReserve could
  // have a precondition that n is at least SmallSize).
  while (maxElms < n) {
    ++lgSize;
    maxElms <<= 1;
  }
  assert(lgSize <= 32);

  // return 2^lgSize - 1
  return ((size_t(1U)) << lgSize) - 1;
  static_assert(MixedArray::MinLgTableSize >= 2,
                "lower limit for 0.75 load factor");
}

ALWAYS_INLINE
std::pair<uint32_t,uint32_t> computeCapAndMask(uint32_t minimumMaxElms) {
  auto const mask = computeMaskFromNumElms(minimumMaxElms);
  auto const cap  = MixedArray::computeMaxElms(mask);
  return std::make_pair(cap, mask);
}

ALWAYS_INLINE
size_t computeAllocBytes(uint32_t cap, uint32_t mask) {
  auto const tabSize    = mask + 1;
  auto const tabBytes   = tabSize * sizeof(int32_t);
  auto const dataBytes  = cap * sizeof(MixedArray::Elm);
  return sizeof(MixedArray) + tabBytes + dataBytes;
}

ALWAYS_INLINE
MixedArray* smartAllocArray(uint32_t cap, uint32_t mask) {
  /*
   * Note: we're currently still allocating the memory for the hash
   * for a packed array even if we aren't going to use it yet.
   */
  auto const allocBytes = computeAllocBytes(cap, mask);
  return static_cast<MixedArray*>(MM().objMallocLogged(allocBytes));
}

ALWAYS_INLINE
MixedArray* mallocArray(uint32_t cap, uint32_t mask) {
  auto const allocBytes = computeAllocBytes(cap, mask);
  return static_cast<MixedArray*>(std::malloc(allocBytes));
}

//////////////////////////////////////////////////////////////////////

}

#endif
