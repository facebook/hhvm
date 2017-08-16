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

#ifndef incl_HPHP_HPHP_ARRAY_DEFS_H_
#define incl_HPHP_HPHP_ARRAY_DEFS_H_

#include "hphp/runtime/base/mixed-array.h"

#include "hphp/runtime/base/array-helpers.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-iterator-defs.h"
#include "hphp/runtime/base/member-val.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/thread-info.h"

#include "hphp/util/stacktrace-profiler.h"
#include "hphp/util/word-mem.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline void MixedArray::scan(type_scan::Scanner& scanner) const {
  if (isZombie()) return;
  auto data = this->data();
  scanner.scan(*data, m_used * sizeof(*data));
}

ALWAYS_INLINE
void MixedArray::InitSmall(MixedArray* a, RefCount count, uint32_t size,
                           int64_t nextIntKey) {
  assert(count != 0);
  InitSmallHash(a);
  a->m_sizeAndPos = size; // pos=0
  a->initHeader(HeaderKind::Mixed, count);
  a->m_scale_used = MixedArray::SmallScale | uint64_t(size) << 32;
  a->m_nextKI = nextIntKey;
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

extern int32_t* warnUnbalanced(MixedArray*, size_t n, int32_t* ei);

inline bool MixedArray::isTombstone(ssize_t pos) const {
  assert(size_t(pos) <= m_used);
  return isTombstone(data()[pos].data.m_type);
}

ALWAYS_INLINE
Cell MixedArray::getElmKey(const Elm& e) {
  if (e.hasIntKey()) {
    return make_tv<KindOfInt64>(e.ikey);
  }
  auto str = e.skey;
  if (str->isRefCounted()) {
    str->rawIncRefCount();
    return make_tv<KindOfString>(str);
  }
  return make_tv<KindOfPersistentString>(str);
}

ALWAYS_INLINE
void MixedArray::getArrayElm(ssize_t pos,
                            TypedValue* valOut,
                            TypedValue* keyOut) const {
  assert(size_t(pos) < m_used);
  auto& elm = data()[pos];
  auto const cur = tvToCell(&elm.data);
  cellDup(*cur, *valOut);
  cellCopy(getElmKey(elm), *keyOut);
}

ALWAYS_INLINE
void MixedArray::getArrayElm(ssize_t pos, TypedValue* valOut) const {
  assert(size_t(pos) < m_used);
  auto& elm = data()[pos];
  auto const cur = tvToCell(&elm.data);
  cellDup(*cur, *valOut);
}

ALWAYS_INLINE
const TypedValue* MixedArray::getArrayElmPtr(ssize_t pos) const {
  assert(validPos(pos));
  if (size_t(pos) >= m_used) return nullptr;
  auto& elm = data()[pos];
  return !isTombstone(elm.data.m_type) ? &elm.data : nullptr;
}

ALWAYS_INLINE
TypedValue MixedArray::getArrayElmKey(ssize_t pos) const {
  assert(validPos(pos));
  if (size_t(pos) >= m_used) return make_tv<KindOfUninit>();
  auto& elm = data()[pos];
  if (isTombstone(elm.data.m_type)) return make_tv<KindOfUninit>();
  return getElmKey(elm);
}

ALWAYS_INLINE
void MixedArray::dupArrayElmWithRef(ssize_t pos,
                                   TypedValue* valOut,
                                   TypedValue* keyOut) const {
  auto& elm = data()[pos];
  tvDupWithRef(elm.data, *valOut);
  cellCopy(getElmKey(elm), *keyOut);
}

inline ArrayData* MixedArray::addVal(int64_t ki, Cell data) {
  assert(!exists(ki));
  assert(!isFull());
  auto h = hash_int64(ki);
  auto ei = findForNewInsert(h);
  auto e = allocElm(ei);
  e->setIntKey(ki, h);
  if (ki >= m_nextKI && m_nextKI >= 0) m_nextKI = ki + 1;
  cellDup(data, e->data);
  // TODO(#3888164): should avoid needing these KindOfUninit checks.
  if (UNLIKELY(e->data.m_type == KindOfUninit)) {
    e->data.m_type = KindOfNull;
  }
  return this;
}

inline ArrayData* MixedArray::addVal(StringData* key, Cell data) {
  assert(!exists(key));
  assert(!isFull());
  return addValNoAsserts(key, data);
}

inline ArrayData* MixedArray::addValNoAsserts(StringData* key, Cell data) {
  strhash_t h = key->hash();
  auto ei = findForNewInsert(h);
  auto e = allocElm(ei);
  e->setStrKey(key, h);
  // TODO(#3888164): we should restructure things so we don't have to check
  // KindOfUninit here.
  initElem(e->data, data);
  return this;
}

inline MixedArray::Elm& MixedArray::addKeyAndGetElem(StringData* key) {
  strhash_t h = key->hash();
  auto ei = findForNewInsert(h);
  auto e = allocElm(ei);
  e->setStrKey(key, h);
  return *e;
}

template <class K>
ArrayData* MixedArray::updateWithRef(K k, TypedValue data) {
  assert(!isFull());
  auto p = insert(k);
  if (p.found) {
    // TODO(#3888164): We should restructure things so we don't have to check
    // KindOfUninit here.
    setElemWithRef(p.tv, data);
    return this;
  }
  // TODO(#3888164): We should restructure things so we don't have to check
  // KindOfUninit here.
  tvDupWithRef(data, p.tv);
  if (p.tv.m_type == KindOfUninit) p.tv.m_type = KindOfNull;
  return this;
}

template <class K>
ArrayData* MixedArray::updateRef(K k, Variant& data) {
  assert(!isFull());
  auto p = insert(k);
  if (p.found) {
    tvBind(data.asRef(), &p.tv);
    return this;
  }
  tvBoxIfNeeded(data.asTypedValue());
  refDup(*data.asTypedValue(), p.tv);
  return this;
}

template <bool warn, class K>
member_lval MixedArray::addLvalImpl(K k) {
  assert(!isFull());
  auto p = insert(k);
  if (!p.found) {
    tvWriteNull(&p.tv);
  }
  return member_lval { this, &p.tv };
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
    assert(isMixed(m_kind) || m_kind == kPackedKind || m_kind == kVecKind);
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
    , m_kind(arr->kind())
  {
    assert(isMixed(m_kind) || m_kind == kPackedKind || m_kind == kVecKind);
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
  union {
    Elm* m_iterMixed;
    TypedValue* m_iterPacked;
  };
  union {
    Elm* m_stopMixed;
    TypedValue* m_stopPacked;
  };
};

//////////////////////////////////////////////////////////////////////

// Converts a TypedValue `source' to its uncounted form, so that its lifetime
// can go beyond the current request.  It is used after doing a raw copy of the
// array elements (without manipulating refcounts, as an uncounted won't hold
// any reference to refcounted values.
ALWAYS_INLINE
void ConvertTvToUncounted(TypedValue* source) {
  if (source->m_type == KindOfRef) {
    // unbox
    auto const inner = source->m_data.pref->tv();
    tvCopy(*inner, *source);
  }
  auto type = source->m_type;
  // `source' cannot be Ref here as we already did an unbox.  It won't be
  // Object or Resource, as these should never appear in an uncounted array.
  // Thus we only need to deal with strings/arrays.  Note that even if the
  // string/array is already uncounted but not static, we still have to make a
  // copy, as we have no idea about the lifetime of the other uncounted item
  // here.
  switch (type) {
    case KindOfString:
      source->m_type = KindOfPersistentString;
      // Fall-through.
    case KindOfPersistentString: {
      auto& str = source->m_data.pstr;
      if (str->isStatic()) break;
      else if (str->empty()) str = staticEmptyString();
      else if (auto const st = lookupStaticString(str)) str = st;
      else str = StringData::MakeUncounted(str->slice());
      break;
    }
    case KindOfVec:
      source->m_type = KindOfPersistentVec;
      // Fall-through.
    case KindOfPersistentVec: {
      auto& ad = source->m_data.parr;
      assert(ad->isVecArray());
      if (ad->isStatic()) break;
      else if (ad->empty()) ad = staticEmptyVecArray();
      else ad = PackedArray::MakeUncounted(ad);
      break;
    }

    case KindOfDict:
      source->m_type = KindOfPersistentDict;
      // Fall-through.
    case KindOfPersistentDict: {
      auto& ad = source->m_data.parr;
      assert(ad->isDict());
      if (ad->isStatic()) break;
      else if (ad->empty()) ad = staticEmptyDictArray();
      else ad = MixedArray::MakeUncounted(ad);
      break;
    }

    case KindOfKeyset:
      source->m_type = KindOfPersistentKeyset;
      // Fall-through.
    case KindOfPersistentKeyset: {
      auto& ad = source->m_data.parr;
      assert(ad->isKeyset());
      if (ad->isStatic()) break;
      else if (ad->empty()) ad = staticEmptyKeysetArray();
      else ad = SetArray::MakeUncounted(ad);
      break;
    }

    case KindOfArray:
      source->m_type = KindOfPersistentArray;
      // Fall-through.
    case KindOfPersistentArray: {
      auto& ad = source->m_data.parr;
      assert(ad->isPHPArray());
      if (ad->isStatic()) break;
      else if (ad->empty()) ad = staticEmptyArray();
      else if (ad->hasPackedLayout()) ad = PackedArray::MakeUncounted(ad);
      else ad = MixedArray::MakeUncounted(ad);
      break;
    }
    case KindOfUninit: {
      source->m_type = KindOfNull;
      break;
    }
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble: {
      break;
    }
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      not_reached();
  }
}

ALWAYS_INLINE
void ReleaseUncountedTv(TypedValue& tv) {
  if (isStringType(tv.m_type)) {
    assert(!tv.m_data.pstr->isRefCounted());
    if (tv.m_data.pstr->isUncounted()) {
      tv.m_data.pstr->destructUncounted();
    }
    return;
  }
  if (isArrayLikeType(tv.m_type)) {
    auto arr = tv.m_data.parr;
    assert(!arr->isRefCounted());
    if (!arr->isStatic()) {
      if (arr->hasPackedLayout()) PackedArray::ReleaseUncounted(arr);
      else MixedArray::ReleaseUncounted(arr);
    }
    return;
  }
  assertx(!isRefcountedType(tv.m_type));
}

//////////////////////////////////////////////////////////////////////

}

#endif
