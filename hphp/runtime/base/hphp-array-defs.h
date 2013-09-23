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

#ifndef incl_HPHP_HPHP_ARRAY_DEFS_H_
#define incl_HPHP_HPHP_ARRAY_DEFS_H_

#include "hphp/runtime/base/hphp-array.h"
#include "hphp/util/stacktrace-profiler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void HphpArray::initHash(size_t tableSize) {
  assert(HphpArray::Empty == -1);
  memset(m_hash, 0xffU, tableSize * sizeof(*m_hash));
  m_hLoad = 0;
}

ALWAYS_INLINE
int32_t* HphpArray::findForNewInsert(size_t h0) const {
  assert(!isPacked());
  size_t mask = m_tableMask;
  auto table = m_hash;
  auto ei = &table[h0 & mask];
  return !validPos(*ei) ? ei :
         findForNewInsertLoop(table, h0, mask);
}

inline bool HphpArray::isTombstone(ssize_t pos) const {
  assert(size_t(pos) <= m_used);
  return isTombstone(m_data[pos].data.m_type);
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
  auto& elm = m_data[pos];
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
    TypedValue* ret = &a->m_data[ki].data;
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
         (a->m_data[ki].data.m_type != KindOfNull);
}

inline bool HphpArray::validPos(ssize_t pos) {
  return pos >= 0;
  static_assert(ssize_t(Empty) == ssize_t(-1), "");
}

inline bool HphpArray::validPos(int32_t pos) {
  return pos >= 0;
  static_assert(Empty == -1, "");
}

inline size_t HphpArray::computeTableSize(uint32_t tableMask) {
  return size_t(tableMask) + size_t(1U);
}

inline size_t HphpArray::computeMaxElms(uint32_t tableMask) {
  return size_t(tableMask) - size_t(tableMask) / LoadScale;
}

inline size_t HphpArray::computeDataSize(uint32_t tableMask) {
  return computeTableSize(tableMask) * sizeof(int32_t) +
    computeMaxElms(tableMask) * sizeof(Elm);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HPHP_ARRAY_DEFS_H_
