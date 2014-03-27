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

#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-iterator-defs.h"

#include "hphp/util/stacktrace-profiler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline ArrayData::~ArrayData() {
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(this);
  }
}

inline void HphpArray::initHash(int32_t* hash, size_t tableSize) {
  wordfill(hash, Empty, tableSize);
}

inline int32_t*
HphpArray::copyHash(int32_t* to, const int32_t* from, size_t count) {
  return wordcpy(to, from, count);
}

inline HphpArray::Elm*
HphpArray::copyElms(Elm* to, const Elm* from, size_t count) {
  return wordcpy(to, from, count);
}

ALWAYS_INLINE int32_t*
HphpArray::findForNewInsert(int32_t* table, size_t mask, size_t h0) const {
  assert(!isPacked());
  for (size_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) return ei;
    probe += i;
    assert(i <= mask && probe == h0 + ((i + i * i) / 2));
  }
}

ALWAYS_INLINE
int32_t* HphpArray::findForNewInsert(size_t h0) const {
  return findForNewInsert(hashTab(), m_tableMask, h0);
}

inline bool HphpArray::isTombstone(ssize_t pos) const {
  assert(size_t(pos) <= m_used);
  return isTombstone(data()[pos].data.m_type);
}

inline void HphpArray::getElmKey(const Elm& e, TypedValue* out) {
  if (e.hasIntKey()) {
    out->m_data.num = e.ikey;
    out->m_type = KindOfInt64;
    return;
  }
  auto str = e.key;
  out->m_data.pstr = str;
  out->m_type = KindOfString;
  str->incRefCount();
}

template <bool withRef> ALWAYS_INLINE
void HphpArray::getArrayElm(ssize_t pos, TypedValue* valOut,
                            TypedValue* keyOut) const {
  assert(size_t(pos) < m_used);
  auto& elm = data()[pos];
  if (withRef) {
    tvAsVariant(valOut) = withRefBind(tvAsVariant(&elm.data));
    if (LIKELY(keyOut != nullptr)) {
      DataType t = keyOut->m_type;
      uint64_t d = keyOut->m_data.num;
      if (isPacked()) {
        keyOut->m_data.num = pos;
        keyOut->m_type = KindOfInt64;
      } else {
        HphpArray::getElmKey(elm, keyOut);
      }
      tvRefcountedDecRefHelper(t, d);
    }
  } else {
    TypedValue* cur = tvToCell(&elm.data);
    cellDup(*cur, *valOut);
    if (keyOut) {
      if (isPacked()) {
        keyOut->m_data.num = pos;
        keyOut->m_type = KindOfInt64;
      } else {
        HphpArray::getElmKey(elm, keyOut);
      }
    }
  }
}

ALWAYS_INLINE
void HphpArray::getArrayElm(ssize_t pos, TypedValue* valOut) const {
  assert(size_t(pos) < m_used);
  auto& elm = data()[pos];
  TypedValue* cur = tvToCell(&elm.data);
  cellDup(*cur, *valOut);
}

inline HphpArray* HphpArray::asHphpArray(ArrayData* ad) {
  assert(ad->isHphpArray());
  auto a = static_cast<HphpArray*>(ad);
  assert(a->checkInvariants());
  return a;
}

inline const HphpArray* HphpArray::asHphpArray(const ArrayData* ad) {
  assert(ad->isHphpArray());
  auto a = static_cast<const HphpArray*>(ad);
  assert(a->checkInvariants());
  return a;
}

inline HphpArray* HphpArray::asPacked(ArrayData* ad) {
  assert(ad->kind() == kPackedKind);
  auto a = static_cast<HphpArray*>(ad);
  assert(a->checkInvariants());
  return a;
}

inline const HphpArray* HphpArray::asPacked(const ArrayData* ad) {
  assert(ad->kind() == kPackedKind);
  auto a = static_cast<const HphpArray*>(ad);
  assert(a->checkInvariants());
  return a;
}

inline HphpArray* HphpArray::asMixed(ArrayData* ad) {
  assert(ad->kind() == kMixedKind);
  auto a = static_cast<HphpArray*>(ad);
  assert(a->checkInvariants());
  return a;
}

inline const HphpArray* HphpArray::asMixed(const ArrayData* ad) {
  assert(ad->kind() == kMixedKind);
  auto a = static_cast<const HphpArray*>(ad);
  assert(a->checkInvariants());
  return a;
}

inline HphpArray* HphpArray::copyImpl() const {
  return isPacked() ? copyPacked() : copyMixed();
}

inline TypedValue
HphpArray::GetCellIntPacked(const ArrayData* ad, int64_t ki) {
  auto a = asPacked(ad);
  if (LIKELY(size_t(ki) < a->m_size)) {
    TypedValue* ret = &a->data()[ki].data;
    ret = tvToCell(ret);
    tvRefcountedIncRef(ret);
    return *ret;
  }
  Variant v = getNotFound(ki);
  return *v.asTypedValue();
}

inline uint64_t
HphpArray::IssetIntPacked(const ArrayData* ad, int64_t ki) {
  auto a = asPacked(ad);
  return (size_t(ki) < a->m_size) &&
         (a->data()[ki].data.m_type != KindOfNull);
}

inline bool HphpArray::validPos(ssize_t pos) {
  return pos >= 0;
  static_assert(ssize_t(Empty) == ssize_t(-1), "");
}

inline bool HphpArray::validPos(int32_t pos) {
  return pos >= 0;
  static_assert(Empty == -1, "");
}

inline size_t HphpArray::hashSize() const {
  return m_tableMask + 1;
}

inline size_t HphpArray::computeMaxElms(uint32_t tableMask) {
  return size_t(tableMask) - size_t(tableMask) / LoadScale;
}

inline size_t HphpArray::computeDataSize(uint32_t tableMask) {
  return (tableMask + 1) * sizeof(int32_t) +
         computeMaxElms(tableMask) * sizeof(Elm);
}

//////////////////////////////////////////////////////////////////////

struct HphpArray::ValIter {
  explicit ValIter(HphpArray* arr)
    : m_arr(arr)
    , m_iter(arr->data())
    , m_stop(m_iter + arr->m_used)
  {
    assert(arr->m_kind == kMixedKind || arr->m_kind == kPackedKind);
  }

  explicit ValIter(HphpArray* arr, ssize_t start_pos)
    : m_arr(arr)
    , m_iter(arr->data() + start_pos)
    , m_stop(arr->data() + arr->m_used)
  {
    assert(arr->m_kind == kMixedKind || arr->m_kind == kPackedKind);
    assert(m_iter <= m_stop);
  }

  Elm* current() const { return m_iter; }
  bool empty() const { return m_iter == m_stop; }

  void advance() {
    do {
      ++m_iter;
    } while (!empty() && HphpArray::isTombstone(m_iter->data.m_type));
  }

  ssize_t currentPos() const { return m_iter - m_arr->data(); }

private:
  HphpArray* m_arr;
  Elm* m_iter;
  Elm* const m_stop;
};

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
uint32_t computeMaskFromNumElms(uint32_t n) {
  assert(n <= 0x7fffffffU);
  auto lgSize = HphpArray::MinLgTableSize;
  auto maxElms = HphpArray::SmallSize;
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
  static_assert(HphpArray::MinLgTableSize >= 2,
                "lower limit for 0.75 load factor");
}

ALWAYS_INLINE
std::pair<uint32_t,uint32_t> computeCapAndMask(uint32_t minimumMaxElms) {
  auto const mask = computeMaskFromNumElms(minimumMaxElms);
  auto const cap  = HphpArray::computeMaxElms(mask);
  return std::make_pair(cap, mask);
}

ALWAYS_INLINE
size_t computeAllocBytes(uint32_t cap, uint32_t mask) {
  auto const tabSize    = mask + 1;
  auto const tabBytes   = tabSize * sizeof(int32_t);
  auto const dataBytes  = cap * sizeof(HphpArray::Elm);
  return sizeof(HphpArray) + tabBytes + dataBytes;
}

ALWAYS_INLINE
HphpArray* smartAllocArray(uint32_t cap, uint32_t mask) {
  /*
   * Note: we're currently still allocating the memory for the hash
   * for a packed array even if we aren't going to use it yet.
   */
  auto const allocBytes = computeAllocBytes(cap, mask);
  return static_cast<HphpArray*>(MM().objMallocLogged(allocBytes));
}

//////////////////////////////////////////////////////////////////////

}

#endif
