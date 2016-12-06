/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
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

ALWAYS_INLINE int32_t* mixedHash(MixedArray::Elm* data, uint32_t scale) {
  return reinterpret_cast<int32_t*>(data + static_cast<size_t>(scale) * 3);
}

inline void MixedArray::scan(type_scan::Scanner& scanner) const {
  if (isZombie()) return;
  auto data = this->data();
  scanner.scan(*data, m_used * sizeof(*data));
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
  auto const hash = mixedHash(mixedData(a), MixedArray::SmallScale);
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
  static_assert(Empty == -1, "Cannot use wordfillones().");
  wordfillones(hash, HashSize(scale));
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

extern int32_t* warnUnbalanced(MixedArray*, size_t n, int32_t* ei);

ALWAYS_INLINE int32_t*
MixedArray::findForNewInsertCheckUnbalanced(int32_t* table, size_t mask,
                                            hash_t h0) {
  uint32_t balanceLimit = RuntimeOption::MaxArrayChain;
  for (uint32_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) {
      return LIKELY(i <= balanceLimit) ? ei : warnUnbalanced(this, i, ei);
    }
    probe += i;
    assertx(i <= mask);
    assertx(probe == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

ALWAYS_INLINE int32_t*
MixedArray::findForNewInsert(int32_t* table, size_t mask, hash_t h0) const {
  for (uint32_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) return ei;
    probe += i;
    assertx(i <= mask);
    assertx(probe == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

ALWAYS_INLINE
int32_t* MixedArray::findForNewInsert(hash_t h0) const {
  return findForNewInsert(hashTab(), mask(), h0);
}

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

inline size_t MixedArray::hashSize() const {
  return HashSize(m_scale);
}

inline ArrayData* MixedArray::addVal(int64_t ki, Cell data) {
  assert(!exists(ki));
  assert(!isFull());
  auto h = hash_int64(ki);
  auto ei = findForNewInsert(h);
  auto& e = allocElm(ei);
  e.setIntKey(ki, h);
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
  assert(!isFull());
  return addValNoAsserts(key, data);
}

inline ArrayData* MixedArray::addValNoAsserts(StringData* key, Cell data) {
  strhash_t h = key->hash();
  auto ei = findForNewInsert(h);
  auto& e = allocElm(ei);
  e.setStrKey(key, h);
  // TODO(#3888164): we should restructure things so we don't have to check
  // KindOfUninit here.
  initVal(e.data, data);
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
ArrayLval MixedArray::addLvalImpl(K k) {
  assert(!isFull());
  auto p = insert(k);
  if (!p.found) tvWriteNull(&p.tv);
  return {this, &tvAsVariant(&p.tv)};
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
  return static_cast<MixedArray*>(low_malloc_data(allocBytes));
}

ALWAYS_INLINE
size_t MixedArray::heapSize() const {
  return computeAllocBytes(m_scale);
}

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
    case KindOfClass:
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
