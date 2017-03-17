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

#include "hphp/runtime/base/mixed-array.h"

#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/empty-array.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/vm/member-operations.h"

#include "hphp/util/alloc.h"
#include "hphp/util/hash.h"
#include "hphp/util/lock.h"
#include "hphp/util/trace.h"

#include <folly/portability/Constexpr.h>

#include <algorithm>
#include <utility>

#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/array-iterator-defs.h"

#if defined(__x86_64__) && defined(FACEBOOK) && !defined(NO_SSE42) &&\
    defined(NO_M_DATA)
#include "hphp/runtime/base/mixed-array-x64.h"
#endif

namespace HPHP {

TRACE_SET_MOD(runtime);

static_assert(computeAllocBytes(MixedArray::SmallScale) ==
              kEmptyMixedArraySize, "");

std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyDictArray;

struct MixedArray::Initializer {
  Initializer() {
    auto const ad = reinterpret_cast<MixedArray*>(&s_theEmptyDictArray);
    auto const data = mixedData(ad);
    auto const hash = mixedHash(data, 1);
    ad->initHash(hash, 1);
    ad->m_sizeAndPos = 0;
    ad->m_scale_used = 1;
    ad->m_nextKI = 0;
    ad->initHeader(HeaderKind::Dict, StaticValue);
  }
};
MixedArray::Initializer MixedArray::s_initializer;

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
ArrayData* MixedArray::MakeReserveImpl(uint32_t size, HeaderKind hk) {
  assert(hk == HeaderKind::Mixed || hk == HeaderKind::Dict);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAllocArray(scale);

  // Intialize the hash table first, because the header is already in L1 cache,
  // but the hash table may not be.  So let's issue the cache request ASAP.
  auto const data = mixedData(ad);
  auto const hash = mixedHash(data, scale);
  ad->initHash(hash, scale);

  ad->m_sizeAndPos   = 0; // size=0, pos=0
  ad->initHeader(hk, 1);
  ad->m_scale_used   = scale; // used=0
  ad->m_nextKI       = 0;

  assert(ad->kind() == kMixedKind || ad->kind() == kDictKind);
  assert(ad->m_size == 0);
  assert(ad->m_pos == 0);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_used == 0);
  assert(ad->m_nextKI == 0);
  assert(ad->m_scale == scale);
  assert(ad->checkInvariants());
  return ad;
}

ArrayData* MixedArray::MakeReserveMixed(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::Mixed);
  assert(ad->isMixed());
  return ad;
}

ArrayData* MixedArray::MakeReserveDict(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::Dict);
  assert(ad->isDict());
  return ad;
}

ArrayData* MixedArray::MakeReserveSame(const ArrayData* other,
                                       uint32_t capacity) {
  capacity = (capacity ? capacity : other->size());

  if (other->isPacked()) {
    return PackedArray::MakeReserve(capacity);
  }

  if (other->isVecArray()) {
    return PackedArray::MakeReserveVec(capacity);
  }

  if (other->isDict()) {
    return MixedArray::MakeReserveDict(capacity);
  }

  if (other->isKeyset()) {
    return SetArray::MakeReserveSet(capacity);
  }

  return MixedArray::MakeReserveMixed(capacity);
}

ArrayData* MixedArray::MakeReserveLike(const ArrayData* other,
                                       uint32_t capacity) {
  capacity = (capacity ? capacity : other->size());

  if (other->hasPackedLayout()) {
    return PackedArray::MakeReserve(capacity);
  } else {
    return MixedArray::MakeReserveMixed(capacity);
  }
}

MixedArray* MixedArray::MakeStruct(uint32_t size, const StringData* const* keys,
                                   const TypedValue* values) {
  assert(size > 0);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAllocArray(scale);

  auto const data = mixedData(ad);
  auto const hash = mixedHash(data, scale);
  ad->initHash(hash, scale);

  ad->m_sizeAndPos       = size; // pos=0
  ad->initHeader(HeaderKind::Mixed, 1);
  ad->m_scale_used       = scale | uint64_t{size} << 32; // used=size
  ad->m_nextKI           = 0;


  // Append values by moving -- Caller assumes we update refcount.
  // Values are in reverse order since they come from the stack, which
  // grows down.
  for (uint32_t i = 0; i < size; i++) {
    assert(keys[i]->isStatic());
    auto k = keys[i];
    auto h = k->hash();
    data[i].setStaticKey(const_cast<StringData*>(k), h);
    const auto& tv = values[size - i - 1];
    data[i].data.m_data = tv.m_data;
    data[i].data.m_type = tv.m_type;
    auto ei = ad->findForNewInsert(h);
    *ei = i;
  }

  assert(ad->m_size == size);
  assert(ad->m_pos == 0);
  assert(ad->kind() == kMixedKind);
  assert(ad->m_scale == scale);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_used == size);
  assert(ad->m_nextKI == 0);
  assert(ad->checkInvariants());
  return ad;
}

MixedArray* MixedArray::MakeMixed(uint32_t size,
                                  const TypedValue* keysAndValues) {
  assert(size > 0);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAllocArray(scale);

  auto const data = mixedData(ad);
  auto const hash = mixedHash(data, scale);
  ad->initHash(hash, scale);

  ad->m_sizeAndPos       = size; // pos=0
  ad->initHeader(HeaderKind::Mixed, 1);
  ad->m_scale_used       = scale | uint64_t{size} << 32; // used=size
  ad->m_nextKI           = 0;

  // Append values by moving -- Caller assumes we update refcount.
  for (uint32_t i = 0; i < size; i++) {
    auto& kTv = keysAndValues[i * 2];
    if (kTv.m_type == KindOfString) {
      auto k = kTv.m_data.pstr;
      auto h = k->hash();
      auto ei = ad->findForInsert(k, h);
      if (validPos(*ei)) return nullptr;
      data[i].setStrKey(k, h);
      *ei = i;
    } else {
      assert(kTv.m_type == KindOfInt64);
      auto k = kTv.m_data.num;
      auto h = hash_int64(k);
      auto ei = ad->findForInsert(k, h);
      if (validPos(*ei)) return nullptr;
      data[i].setIntKey(k, h);
      *ei = i;
    }
    const auto& tv = keysAndValues[(i * 2) + 1];
    data[i].data.m_data = tv.m_data;
    data[i].data.m_type = tv.m_type;
  }

  assert(ad->m_size == size);
  assert(ad->m_pos == 0);
  assert(ad->kind() == kMixedKind);
  assert(ad->m_scale == scale);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_used == size);
  assert(ad->m_nextKI == 0);
  assert(ad->checkInvariants());
  return ad;
}

// for internal use by copyStatic() and copyMixed()
ALWAYS_INLINE
MixedArray* MixedArray::CopyMixed(const MixedArray& other,
                                  AllocMode mode, HeaderKind dest_hk) {
  assert(dest_hk == HeaderKind::Mixed || dest_hk == HeaderKind::Dict);
  auto const scale = other.m_scale;
  auto const ad = mode == AllocMode::Request ? reqAllocArray(scale)
                                             : staticAllocArray(scale);

  // Copy everything including tombstones.  We want to copy the elements and
  // the hash separately, because the array may not be very full.
  assertx(reinterpret_cast<uintptr_t>(ad) % 16 == 0);
  assertx(reinterpret_cast<uintptr_t>(&other) % 16 == 0);
  // Adding 24 bytes so that we can copy in 32-byte groups. This might
  // overwrite the hash table, but won't overrun the allocated space as long as
  // `malloc' returns multiple of 16 bytes.
  bcopy32_inline(ad, &other,
                 sizeof(MixedArray) + sizeof(Elm) * other.m_used + 24);
  RefCount count = mode == AllocMode::Request ? 1 : StaticValue;
  ad->initHeader(dest_hk, count);
  copyHash(ad->hashTab(), other.hashTab(), scale);

  // Bump up refcounts as needed.
  auto const elms = ad->data();
  for (uint32_t i = 0, limit = ad->m_used; i < limit; ++i) {
    auto& e = elms[i];
    if (UNLIKELY(e.isTombstone())) continue;
    if (e.hasStrKey()) e.skey->incRefCount();
    if (UNLIKELY(e.data.m_type == KindOfRef)) {
      auto ref = e.data.m_data.pref;
      // See also tvDupFlattenVars()
      if (!ref->isReferenced() && ref->tv()->m_data.parr != &other) {
        cellDup(*ref->tv(), *reinterpret_cast<Cell*>(&e.data));
        continue;
      } else if (dest_hk == HeaderKind::Dict) {
        ad->m_used = i;
        ad->m_size = i;
        ad->m_pos = 0;
        if (ad->isRefCounted()) Release(ad);
        else if (ad->isUncounted()) ReleaseUncounted(ad);
        throwRefInvalidArrayValueException(staticEmptyDictArray());
      }
    }
    tvRefcountedIncRef(&e.data);
  }

  // We need to assert this up here before we possibly call compact (which
  // will cause m_used to change)
  assert(ad->m_used == other.m_used);

  // If the element density dropped below 50% due to indirect elements
  // being converted into tombstones, we should do a compaction
  if (ad->m_size < ad->m_used / 2) {
    ad->compact(false);
  }

  assert(ad->m_kind == dest_hk);
  assert(ad->m_size == other.m_size);
  assert(ad->m_pos == other.m_pos);
  assert(mode == AllocMode::Request ? ad->hasExactlyOneRef() :
         ad->isStatic());
  assert(ad->m_scale == scale);
  assert(ad->checkInvariants());
  return ad;
}

NEVER_INLINE ArrayData* MixedArray::CopyStatic(const ArrayData* in) {
  auto a = asMixed(in);
  assert(a->checkInvariants());
  return CopyMixed(*a, AllocMode::Static, in->m_kind);
}

NEVER_INLINE MixedArray* MixedArray::copyMixed() const {
  assert(checkInvariants());
  return CopyMixed(*this, AllocMode::Request, this->m_kind);
}

ALWAYS_INLINE
MixedArray* MixedArray::copyMixedAndResizeIfNeeded() const {
  if (LIKELY(!isFull())) return copyMixed();
  return copyMixedAndResizeIfNeededSlow();
}

NEVER_INLINE
MixedArray* MixedArray::copyMixedAndResizeIfNeededSlow() const {
  assert(isFull());

  // Note: this path will have to handle splitting strong iterators
  // later when we combine copyMixed & Grow into one operation.  For
  // now I'm just making use of copyMixed to do it for me before
  // GrowPacked happens.
  auto const copy = copyMixed();
  auto const ret = copy->resize();
  if (copy != ret) Release(copy);
  return ret;
}

//////////////////////////////////////////////////////////////////////

size_t MixedArray::computeAllocBytesFromMaxElms(uint32_t maxElms) {
  auto const scale = computeScaleFromSize(maxElms);
  return computeAllocBytes(scale);
}

//////////////////////////////////////////////////////////////////////

Variant MixedArray::CreateVarForUncountedArray(const Variant& source) {
  auto type = source.getType(); // this gets rid of the ref, if it was one
  switch (type) {
    case KindOfUninit:
    case KindOfNull:
      return init_null();

    case KindOfBoolean:
      return source.getBoolean();
    case KindOfInt64:
      return source.getInt64();
    case KindOfDouble:
      return source.getDouble();
    case KindOfPersistentString:
      return Variant{source.getStringData(), Variant::PersistentStrInit{}};
    case KindOfPersistentVec:
    case KindOfPersistentDict:
    case KindOfPersistentKeyset:
    case KindOfPersistentArray:
      return Variant{source.getArrayData()};

    case KindOfString: {
      auto src = source.getStringData();
      if (!src->isRefCounted()) {
        return Variant{src, Variant::PersistentStrInit{}};
      }
      if (auto s = lookupStaticString(src)) {
        return Variant{s, Variant::PersistentStrInit{}};
      }
      return Variant{StringData::MakeUncounted(src->slice()),
                     Variant::PersistentStrInit{}};
    }

    case KindOfVec: {
      auto const ad = source.getArrayData();
      assert(ad->isVecArray());
      if (ad->empty()) return Variant{staticEmptyVecArray()};
      return Variant{PackedArray::MakeUncounted(ad)};
    }

    case KindOfDict: {
      auto const ad = source.getArrayData();
      assert(ad->isDict());
      if (ad->empty()) return Variant{staticEmptyDictArray()};
      return Variant{MixedArray::MakeUncounted(ad)};
    }

    case KindOfKeyset: {
      auto const ad = source.getArrayData();
      assert(ad->isKeyset());
      if (ad->empty()) return Variant{staticEmptyKeysetArray()};
      return Variant{SetArray::MakeUncounted(ad)};
    }

    case KindOfArray: {
      auto const ad = source.getArrayData();
      assert(ad->isPHPArray());
      if (ad->empty()) return Variant{staticEmptyArray()};
      if (ad->hasPackedLayout()) return Variant{PackedArray::MakeUncounted(ad)};
      return Variant{MixedArray::MakeUncounted(ad)};
    }

    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      break;
  }
  not_reached();
}

ALWAYS_INLINE static bool UncountedMixedArrayOnHugePage() {
#ifdef USE_JEMALLOC_CHUNK_HOOKS
  return high_huge1g_arena && RuntimeOption::EvalUncountedMixedArrayHuge;
#else
  return false;
#endif
}

ArrayData* MixedArray::MakeUncounted(ArrayData* array, size_t extra) {
  auto a = asMixed(array);
  assertx(!a->empty());
  auto const scale = a->scale();
  auto const allocSize = extra + computeAllocBytes(scale);
  auto const mem = static_cast<char*>(
    UncountedMixedArrayOnHugePage() ? malloc_huge(allocSize) : malloc(allocSize)
  );
  auto const ad = reinterpret_cast<MixedArray*>(mem + extra);
  auto const used = a->m_used;
  // Do a raw copy first, without worrying about counted types or refcount
  // manipulation.  To copy in 32-byte chunks, we add 24 bytes to the length.
  // This might overwrite the hash table, but won't go beyond the space
  // allocated for the MixedArray, assuming `malloc()' always allocates
  // multiple of 16 bytes and extra is also a multiple of 16.
  assert((extra & 0xf) == 0);
  bcopy32_inline(ad, a, sizeof(MixedArray) + sizeof(Elm) * used + 24);
  ad->m_count = UncountedValue; // after bcopy, update count
  copyHash(ad->hashTab(), a->hashTab(), scale);

  // Need to make sure keys and values are all uncounted.
  auto dstElem = ad->data();
  for (uint32_t i = 0; i < used; ++i) {
    auto& te = dstElem[i];
    auto const type = te.data.m_type;
    if (UNLIKELY(isTombstone(type))) continue;
    if (te.hasStrKey() && !te.skey->isStatic()) {
      auto const st = lookupStaticString(te.skey);
      te.skey = st != nullptr ? st
                              : StringData::MakeUncounted(te.skey->slice());
    }
    ConvertTvToUncounted(&te.data);
  }
  return ad;
}

ArrayData* MixedArray::MakeDictFromAPC(const APCArray* apc) {
  assert(apc->isDict());
  auto const apcSize = apc->size();
  DictInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.setValidKey(apc->getKey(i), apc->getValue(i)->toLocal());
  }
  return init.create();
}

//=============================================================================
// Destruction

NEVER_INLINE
void MixedArray::Release(ArrayData* in) {
  assert(in->isRefCounted());
  assert(in->hasExactlyOneRef());
  auto const ad = asMixed(in);

  if (!ad->isZombie()) {
    auto const data = ad->data();
    auto const stop = data + ad->m_used;

    for (auto ptr = data; ptr != stop; ++ptr) {
      if (isTombstone(ptr->data.m_type)) continue;
      if (ptr->hasStrKey()) {
        decRefStr(ptr->skey);
        // Keep GC from asserting on freed string in debug mode. GC will ignore
        // pointers to freed memory gracefully in prod mode.
        if (debug) ptr->skey = nullptr;
      }
      tvRefcountedDecRef(&ptr->data);
    }

    if (UNLIKELY(strong_iterators_exist())) {
      free_strong_iterators(ad);
    }
  }
  MM().objFree(ad, ad->heapSize());
}

NEVER_INLINE
void MixedArray::ReleaseUncounted(ArrayData* in, size_t extra) {
  auto const ad = asMixed(in);
  assert(ad->isUncounted());

  if (!ad->isZombie()) {
    auto const data = ad->data();
    auto const stop = data + ad->m_used;

    for (auto ptr = data; ptr != stop; ++ptr) {
      if (isTombstone(ptr->data.m_type)) continue;
      if (ptr->hasStrKey()) {
        assert(!ptr->skey->isRefCounted());
        if (ptr->skey->isUncounted()) {
          ptr->skey->destructUncounted();
        }
      }
      ReleaseUncountedTv(ptr->data);
    }

    // We better not have strong iterators associated with uncounted
    // arrays.
    if (debug && UNLIKELY(strong_iterators_exist())) {
      for_each_strong_iterator([&] (const MIterTable::Ent& miEnt) {
        assert(miEnt.array != ad);
      });
    }
  }
  if (UncountedMixedArrayOnHugePage()) {
    free_huge(reinterpret_cast<char*>(ad) - extra);
  } else {
    free(reinterpret_cast<char*>(ad) - extra);
  }
}

//=============================================================================

/*
 * Invariants:
 *
 *  All arrays are either in a mode, or in zombie state.  The zombie
 *  state happens if an array is "moved from"---the only legal
 *  operation on a zombie array is to decref and release it.
 *
 * All arrays (zombie or not):
 *
 *   m_scale is 2^k (1/4 of the hashtable size and 1/3 of capacity)
 *   mask is 4*scale - 1 (even power of 2 required for quadratic probe)
 *   mask == folly::nextPowTwo(capacity()) - 1;
 *
 * Zombie state:
 *
 *   m_used == UINT32_MAX
 *   no MArrayIter's are pointing to this array
 *
 * Non-zombie:
 *
 *   m_size <= m_used; m_used <= capacity()
 *   last element cannot be a tombstone
 *   m_pos and all external iterators can't be on a tombstone
 *
 * kMixedKind:
 *   m_nextKI >= highest actual int key
 *   Elm.data.m_type maybe kInvalidDataType (tombstone)
 *   hash[] maybe Tombstone
 *
 * kPackedKind:
 *   m_size == m_used
 *   m_nextKI = uninitialized
 *   Elm.skey uninitialized
 *   Elm.hash uninitialized
 *   no kInvalidDataType tombstones
 */
bool MixedArray::checkInvariants() const {
  static_assert(ssize_t(Empty) == ssize_t(-1), "");
  static_assert(Tombstone < 0, "");
  static_assert((Tombstone & 1) == 0, "");
  static_assert(sizeof(Elm) == 24, "");
  static_assert(sizeof(ArrayData) == 2 * sizeof(uint64_t), "");
  static_assert(
    sizeof(MixedArray) == sizeof(ArrayData) + 2 * sizeof(uint64_t),
    "Performance is sensitive to sizeof(MixedArray)."
    " Make sure you changed it with good reason and then update this assert."
  );

  // All arrays:
  assert(hasMixedLayout());
  assert(checkCount());
  assert(m_scale >= 1 && (m_scale & (m_scale - 1)) == 0);
  assert(MixedArray::HashSize(m_scale) ==
         folly::nextPowTwo<uint64_t>(capacity()));

  if (isZombie()) return true;

  // Non-zombie:
  assert(m_size <= m_used);
  assert(m_used <= capacity());
  if (m_pos != m_used) {
    assert(size_t(m_pos) < m_used);
    assert(!isTombstone(data()[m_pos].data.m_type));
  }

  return true;
}

//=============================================================================
// Iteration.

inline ssize_t MixedArray::prevElm(Elm* elms, ssize_t ei) const {
  assert(ei < ssize_t(m_used));
  while (--ei >= 0) {
    if (!elms[ei].isTombstone()) {
      return ei;
    }
  }
  return m_used;
}

ssize_t MixedArray::IterBegin(const ArrayData* ad) {
  auto a = asMixed(ad);
  return a->nextElm(a->data(), -1);
}

ssize_t MixedArray::IterLast(const ArrayData* ad) {
  auto a = asMixed(ad);
  auto elms = a->data();
  ssize_t ei = a->m_used;
  while (--ei >= 0) {
    if (!elms[ei].isTombstone()) {
      return ei;
    }
  }
  return a->m_used;
}

ssize_t MixedArray::IterEnd(const ArrayData* ad) {
  auto a = asMixed(ad);
  return a->m_used;
}

ssize_t MixedArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  auto a = asMixed(ad);
  ++pos;
  if (pos >= a->m_used) return a->m_used;
  if (!a->data()[pos].isTombstone()) {
    return pos;
  }
  return a->iter_advance_helper(pos);
}

// caller has already incremented pos but encountered a tombstone
ssize_t MixedArray::iter_advance_helper(ssize_t next_pos) const {
  Elm* elms = data();
  for (auto limit = m_used; size_t(next_pos) < limit; ++next_pos) {
    if (!elms[next_pos].isTombstone()) {
      return next_pos;
    }
  }
  assert(next_pos == m_used);
  return next_pos;
}

ssize_t MixedArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  auto a = asMixed(ad);
  return a->prevElm(a->data(), pos);
}

size_t MixedArray::Vsize(const ArrayData*) { not_reached(); }

const Variant& MixedArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  auto a = asMixed(ad);
  assert(a->checkInvariants());
  assert(pos != a->m_used);
  auto& e = a->data()[pos];
  assert(!e.isTombstone());
  return tvAsCVarRef(&e.data);
}

bool MixedArray::IsVectorData(const ArrayData* ad) {
  auto a = asMixed(ad);
  if (a->m_size == 0) {
    // any 0-length array is "vector-like" for the sake of this
    // function, even if kind != kVector.
    return true;
  }
  auto const elms = a->data();
  int64_t i = 0;
  for (uint32_t pos = 0, limit = a->m_used; pos < limit; ++pos) {
    auto const& e = elms[pos];
    if (isTombstone(e.data.m_type)) {
      continue;
    }
    if (e.hasStrKey() || e.ikey != i) {
      return false;
    }
    ++i;
  }
  return true;
}

//=============================================================================
// Lookup.

ALWAYS_INLINE bool hitStringKey(const MixedArray::Elm& e, const StringData* s,
                                strhash_t hash) {
  // hitStringKey() should only be called on an Elm that is referenced by a
  // hash table entry. MixedArray guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!MixedArray::isTombstone(e.data.m_type));
  return hash == e.hash() && (s == e.skey || s->same(e.skey));
}

static bool hitIntKey(const MixedArray::Elm& e, int64_t ki) {
  // hitIntKey() should only be called on an Elm that is referenced by a
  // hash table entry. MixedArray guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!e.isTombstone());
  return e.ikey == ki && e.hasIntKey();
}

// Quadratic probe is:
//
//   h(k, i) = (k + c1*i + c2*(i^2)) % tableSize
//
// Use 1/2 for c1 and c2.  In combination with a table size that is a power of
// 2, this guarantees a probe sequence of length tableSize that probes all
// table elements exactly once.

template <class Hit> ALWAYS_INLINE
ssize_t MixedArray::findImpl(hash_t h0, Hit hit) const {
  uint32_t mask = this->mask();
  auto elms = data();
  auto hashtable = hashTab();
  for (uint32_t probeIndex = h0, i = 1;; ++i) {
    auto pos = hashtable[probeIndex & mask];
    if (validPos(pos)) {
      if (hit(elms[pos])) return pos;
    } else if (pos & 1) {
      assert(pos == Empty);
      return pos;
    }
    probeIndex += i;
    assertx(i <= mask);
    assertx(probeIndex == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

ssize_t MixedArray::find(int64_t ki, inthash_t h) const {
  return findImpl(h, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

ssize_t MixedArray::find(const StringData* s, strhash_t h) const {
  return findImpl(h, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

NEVER_INLINE
int32_t* warnUnbalanced(MixedArray* a, size_t n, int32_t* ei) {
  if (n > size_t(RuntimeOption::MaxArrayChain)) {
    decRefArr(a->asArrayData()); // otherwise, a leaks when exn propagates
    raise_error("Array is too unbalanced (%lu)", n);
  }
  return ei;
}

template <class Hit> ALWAYS_INLINE
int32_t* MixedArray::findForInsertImpl(hash_t h0, Hit hit) const {
  uint32_t mask = this->mask();
  auto elms = data();
  auto hashtable = hashTab();
  for (uint32_t probeIndex = h0, i = 1;; ++i) {
    auto ei = &hashtable[probeIndex & mask];
    int32_t pos = *ei;
    if (validPos(pos)) {
      if (hit(elms[pos])) {
        return ei;
      }
    } else if (pos & 1) {
      assert(pos == Empty);
      return ei;
    }
    probeIndex += i;
    assertx(i <= mask);
    assertx(probeIndex == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

int32_t* MixedArray::findForInsert(int64_t ki, inthash_t h) const {
  return findForInsertImpl(h, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

int32_t* MixedArray::findForInsert(const StringData* s, strhash_t h) const {
  return findForInsertImpl(h, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

MixedArray::InsertPos MixedArray::insert(int64_t k) {
  assert(!isFull());
  auto h = hash_int64(k);
  auto ei = findForInsert(k, h);
  if (validPos(*ei)) {
    return InsertPos(true, data()[*ei].data);
  }
  if (k >= m_nextKI && m_nextKI >= 0) m_nextKI = k + 1;
  auto& e = allocElm(ei);
  e.setIntKey(k, h);
  return InsertPos(false, e.data);
}

MixedArray::InsertPos MixedArray::insert(StringData* k) {
  assert(!isFull());
  auto const h = k->hash();
  auto ei = findForInsert(k, h);
  if (validPos(*ei)) {
    return InsertPos(true, data()[*ei].data);
  }
  auto& e = allocElm(ei);
  e.setStrKey(k, h);
  return InsertPos(false, e.data);
}

template <class Hit, class Remove> ALWAYS_INLINE
ssize_t MixedArray::findForRemoveImpl(hash_t h0, Hit hit, Remove remove) const {
  size_t mask = this->mask();
  auto elms = data();
  auto hashtable = hashTab();
  for (uint32_t probe = h0, i = 1;; ++i) {
    auto ei = &hashtable[probe & mask];
    auto pos = *ei;
    if (validPos(pos)) {
      if (hit(elms[pos])) {
        remove(elms[pos]);
        *ei = Tombstone;
        return pos;
      }
    } else if (pos & 1) {
      assert(pos == Empty);
      return pos;
    }
    probe += i;
    assertx(i <= mask);
    assertx(probe == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

NEVER_INLINE
ssize_t MixedArray::findForRemove(int64_t ki, inthash_t h, bool updateNext) {
  // all vector methods should work w/out touching the hashtable
  return findForRemoveImpl(h,
      [&] (const Elm& e) {
        return hitIntKey(e, ki);
      },
      [this, ki, updateNext] (Elm& e) {
        assert(ki == e.ikey);
        // Conform to PHP5 behavior
        // Hacky: don't removed the unsigned cast, else g++ can optimize away
        // the check for == 0x7fff..., since there is no signed int k
        // for which k-1 == 0x7fff...
        if (((uint64_t)ki == (uint64_t)m_nextKI-1) &&
            (ki >= 0) &&
            (ki == 0x7fffffffffffffffLL || updateNext)) {
          --m_nextKI;
        }
      }
  );
}

ssize_t MixedArray::findForRemove(const StringData* s, strhash_t h) {
  return findForRemoveImpl(h,
      [&] (const Elm& e) {
        return hitStringKey(e, s, h);
      },
      [] (Elm& e) {
        decRefStr(e.skey);
        e.setIntKey(0, hash_int64(0));
      }
    );
}

bool MixedArray::ExistsInt(const ArrayData* ad, int64_t k) {
  auto a = asMixed(ad);
  return validPos(a->find(k, hash_int64(k)));
}

bool MixedArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  auto a = asMixed(ad);
  return validPos(a->find(k, k->hash()));
}

//=============================================================================
// Append/insert/update.

ALWAYS_INLINE
ArrayData* MixedArray::zInitVal(TypedValue& tv, RefData* v) {
  tv.m_type = KindOfRef;
  tv.m_data.pref = v;
  return this;
}

ALWAYS_INLINE
MixedArray* MixedArray::initRef(TypedValue& tv, Variant& v) {
  tvAsUninitializedVariant(&tv).constructRefHelper(v);
  return this;
}

ALWAYS_INLINE
MixedArray* MixedArray::initWithRef(TypedValue& tv, const Variant& v) {
  tvWriteNull(&tv);
  tvAsVariant(&tv).setWithRef(v);
  return this;
}

ALWAYS_INLINE
ArrayData* MixedArray::zSetVal(TypedValue& tv, RefData* v) {
  // Dec ref the old value
  tvRefcountedDecRef(tv);
  // Store the RefData but do not increment the refcount
  tv.m_type = KindOfRef;
  tv.m_data.pref = v;
  return this;
}

/*
 * This is a streamlined copy of Variant.constructValHelper()
 * with no incref+decref because we're moving v to this array.
 */
ALWAYS_INLINE
MixedArray* MixedArray::moveVal(TypedValue& tv, TypedValue v) {
  tv.m_type = v.m_type == KindOfUninit ? KindOfNull : v.m_type;
  tv.m_data.num = v.m_data.num;
  return this;
}

ALWAYS_INLINE MixedArray* MixedArray::resizeIfNeeded() {
  if (UNLIKELY(isFull())) return resize();
  return this;
}

NEVER_INLINE MixedArray* MixedArray::resize() {
  assert(m_used <= capacity());
  uint32_t cap = capacity();
  // At a minimum, compaction is required.  If the load factor would be >0.5
  // even after compaction, grow instead, in order to avoid the possibility
  // of repeated compaction if the load factor were to hover at nearly 0.75.
  if (m_size > cap / 2) {
    assert(mask() <= 0x7fffffffU);
    return Grow(this, m_scale * 2);
  }
  compact(false);
  return this;
}

NEVER_INLINE MixedArray*
MixedArray::InsertCheckUnbalanced(MixedArray* ad,
                                  int32_t* table,
                                  uint32_t mask,
                                  Elm* iter,
                                  Elm* stop) {
  for (uint32_t i = 0; iter != stop; ++iter, ++i) {
    auto& e = *iter;
    if (e.isTombstone()) continue;
    *ad->findForNewInsertCheckUnbalanced(table, mask, e.probe()) = i;
  }
  return ad;
}

MixedArray*
MixedArray::Grow(MixedArray* old, uint32_t newScale) {
  assert(old->m_size > 0);
  assert(MixedArray::Capacity(newScale) >= old->m_size);
  assert(newScale >= 1 && (newScale & (newScale - 1)) == 0);

  auto ad            = reqAllocArray(newScale);
  auto const oldUsed = old->m_used;
  ad->m_sizeAndPos   = old->m_sizeAndPos;
  ad->initHeader(*old, 1);
  ad->m_scale_used   = newScale | uint64_t{oldUsed} << 32;

  copyElmsNextUnsafe(ad, old, oldUsed);

  auto table = mixedHash(ad->data(), newScale);
  ad->initHash(table, newScale);

  if (UNLIKELY(strong_iterators_exist())) {
    move_strong_iterators(ad, old);
  }

  auto iter = ad->data();
  auto const stop = iter + oldUsed;
  assert(newScale == ad->m_scale);
  auto mask = MixedArray::Mask(newScale);
  old->setZombie();

  if (UNLIKELY(oldUsed >= 2000)) {
    // This should be a tail call in opt build.
    ad = InsertCheckUnbalanced(ad, table, mask, iter, stop);
  } else {
    for (uint32_t i = 0; iter != stop; ++iter, ++i) {
      auto& e = *iter;
      if (e.isTombstone()) continue;
      *ad->findForNewInsert(table, mask, e.probe()) = i;
    }
  }

  assert(old->isZombie());
  assert(ad->kind() == old->kind());
  assert(ad->m_size == old->m_size);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_pos == old->m_pos);
  assert(ad->m_used == oldUsed);
  assert(ad->m_scale == newScale);
  assert(ad->checkInvariants());
  return ad;
}

void MixedArray::compact(bool renumber /* = false */) {
  bool updatePosAfterCompact = false;
  ElmKey mPos;
  bool hasStrongIters;
  req::TinyVector<ElmKey,3> siKeys;

  // Prep work before beginning the compaction process
  if (LIKELY(!renumber)) {
    if ((updatePosAfterCompact = (m_pos != 0 && m_pos != m_used))) {
      // Cache key for element associated with m_pos in order to
      // update m_pos after the compaction has been performed.
      // We only need to do this if m_pos is nonzero and is not
      // the canonical invalid position.
      assert(size_t(m_pos) < m_used);
      auto& e = data()[m_pos];
      mPos.hash = e.hash();
      mPos.skey = e.skey;
    } else {
      if (m_pos == m_used) {
        // If m_pos is the canonical invalid position, we need to update
        // it to what the new canonical invalid position will be after
        // compaction
        m_pos = m_size;
      }
    }
    if (UNLIKELY((hasStrongIters = strong_iterators_exist()))) {
      for_each_strong_iterator([&] (const MIterTable::Ent& miEnt) {
        if (miEnt.array != this) return;
        if (miEnt.iter->getResetFlag()) return;
        auto const ei = miEnt.iter->m_pos;
        if (ei == m_used) return;
        auto& e = data()[ei];
        siKeys.push_back(ElmKey(e.hash(), e.skey));
      });
    }
  } else {
    // To conform to PHP5 behavior, when array's integer keys are renumbered
    // we invalidate all strong iterators and we reset the array's internal
    // cursor (even if the array is empty or has no integer keys).
    if (UNLIKELY(strong_iterators_exist())) {
      free_strong_iterators(this);
    }
    m_pos = 0;
    updatePosAfterCompact = false;
    hasStrongIters = false;
    // Set m_nextKI to 0 for now to prepare for renumbering integer keys
    m_nextKI = 0;
  }

  // Perform compaction
  auto elms = data();
  auto mask = this->mask();
  auto table = hashTab();
  initHash(table, scale());
  for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
    while (elms[frPos].isTombstone()) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = elms[toPos];
    if (toPos != frPos) {
      toE = elms[frPos];
    }
    if (UNLIKELY(renumber && toE.hasIntKey())) {
      toE.setIntKey(m_nextKI, hash_int64(m_nextKI));
      m_nextKI++;
    }
    *findForNewInsert(table, mask, toE.probe()) = toPos;
  }

  if (updatePosAfterCompact) {
    // Update m_pos, now that compaction is complete
    m_pos = mPos.hash >= 0 ? ssize_t(find(mPos.skey, mPos.hash))
                           : ssize_t(find(mPos.ikey, mPos.hash));
    assert(m_pos >= 0 && m_pos < m_size);
  }

  if (LIKELY(!hasStrongIters)) {
    // In the common case there aren't any strong iterators, so we
    // can just update m_used and return
    m_used = m_size;
    return;
  }

  // Update strong iterators now that compaction is complete. Note
  // that we wait to update m_used until after we've updated the
  // strong iterators because we need to consult what the _old_ value
  // of m_used before compaction was performed.
  int key = 0;
  for_each_strong_iterator(
    [&] (MIterTable::Ent& miEnt) {
      if (miEnt.array != this) return;
      auto const iter = miEnt.iter;
      if (iter->getResetFlag()) return;
      if (iter->m_pos == m_used) {
        // If this iterator was set to the _old_ canonical invalid position,
        // we need to update it to the _new_ canonical invalid position after
        // compaction.
        iter->m_pos = m_size;
        return;
      }
      auto& k = siKeys[key];
      key++;
      iter->m_pos = k.hash >= 0 ? ssize_t(find(k.skey, k.hash))
                                : ssize_t(find(k.ikey, k.hash));
      assert(iter->m_pos >= 0 && iter->m_pos < m_size);
    }
  );
  // Finally, update m_used and return
  m_used = m_size;
}

bool MixedArray::nextInsert(Cell v) {
  assert(m_nextKI >= 0);
  assert(!isFull());

  int64_t ki = m_nextKI;
  auto h = hash_int64(ki);
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  auto ei = findForNewInsert(h);
  assert(!validPos(*ei));
  // Allocate and initialize a new element.
  auto& e = allocElm(ei);
  e.setIntKey(ki, h);
  m_nextKI = ki + 1; // Update next free element.
  cellDup(v, e.data);
  return true;
}

ArrayData* MixedArray::nextInsertRef(Variant& data) {
  assert(!isFull());
  assert(m_nextKI >= 0);

  int64_t ki = m_nextKI;
  auto h = hash_int64(ki);
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  auto ei = findForNewInsert(h);
  auto& e = allocElm(ei);
  e.setIntKey(ki, h);
  m_nextKI = ki + 1; // Update next free element.
  return initRef(e.data, data);
}

ArrayData* MixedArray::nextInsertWithRef(const Variant& data) {
  assert(!isFull());

  int64_t ki = m_nextKI;
  auto h = hash_int64(ki);
  auto ei = findForNewInsert(h);
  assert(!validPos(*ei));

  // Allocate a new element.
  auto& e = allocElm(ei);
  e.setIntKey(ki, h);
  m_nextKI = ki + 1; // Update next free element.
  return initWithRef(e.data, data);
}

template <class K> ALWAYS_INLINE
ArrayData* MixedArray::update(K k, Cell data) {
  assert(!isFull());
  auto p = insert(k);
  if (p.found) {
    // TODO(#3888164): we should restructure things so we don't have
    // to check KindOfUninit here.
    setVal(p.tv, data);
    return this;
  }
  // TODO(#3888164): we should restructure things so we don't have to
  // check KindOfUninit here.
  initVal(p.tv, data);
  return this;
}

template <class K> ALWAYS_INLINE
ArrayData* MixedArray::zSetImpl(K k, RefData* data) {
  auto p = insert(k);
  if (p.found) {
    return zSetVal(p.tv, data);
  }
  return zInitVal(p.tv, data);
}

ALWAYS_INLINE
ArrayData* MixedArray::zAppendImpl(RefData* data, int64_t* key_ptr) {
  if (UNLIKELY(m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return this;
  }
  int64_t ki = m_nextKI;
  auto h = hash_int64(ki);
  auto ei = findForNewInsert(h);
  assert(!validPos(*ei));
  auto& e = allocElm(ei);
  e.setIntKey(ki, h);
  m_nextKI = ki + 1;
  *key_ptr = ki;
  return zInitVal(e.data, data);
}

ArrayLval MixedArray::LvalInt(ArrayData* ad, int64_t k, bool copy) {
  auto a = asMixed(ad);
  if (copy) {
    a = a->copyMixedAndResizeIfNeeded();
  } else {
    a = a->resizeIfNeeded();
  }
  return a->addLvalImpl(k);
}

ArrayLval MixedArray::LvalIntRef(ArrayData* ad, int64_t k, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);
  return LvalInt(ad, k, copy);
}

ArrayLval MixedArray::LvalStr(ArrayData* ad, StringData* key, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->addLvalImpl(key);
}

ArrayLval MixedArray::LvalStrRef(ArrayData* ad, StringData* key, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(key);
  return LvalStr(ad, key, copy);
}

ArrayLval MixedArray::LvalSilentInt(ArrayData* ad, int64_t k, bool copy) {
  auto a = asMixed(ad);
  auto const pos = a->find(k, hash_int64(k));
  if (UNLIKELY(!validPos(pos))) return {a, nullptr};
  if (copy) a = a->copyMixed();
  return {a, &tvAsVariant(&a->data()[pos].data)};
}

ArrayLval MixedArray::LvalSilentStr(ArrayData* ad, const StringData* k,
                                     bool copy) {
  auto a = asMixed(ad);
  auto const pos = a->find(k, k->hash());
  if (UNLIKELY(!validPos(pos))) return {a, nullptr};
  if (copy) a = a->copyMixed();
  return {a, &tvAsVariant(&a->data()[pos].data)};
}

ArrayLval MixedArray::LvalNew(ArrayData* ad, bool copy) {
  auto a = asMixed(ad);
  if (UNLIKELY(a->m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return {a, &lvalBlackHole()};
  }

  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();

  if (UNLIKELY(!a->nextInsert(make_tv<KindOfNull>()))) {
    return {a, &lvalBlackHole()};
  }

  return {a, &tvAsVariant(&a->data()[a->m_used - 1].data)};
}

ArrayLval MixedArray::LvalNewRef(ArrayData* ad, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefNew();
  return LvalNew(ad, copy);
}

ArrayData* MixedArray::SetInt(ArrayData* ad, int64_t k, Cell v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->update(k, v);
}

ArrayData*
MixedArray::SetStr(ArrayData* ad, StringData* k, Cell v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->update(k, v);
}

ArrayData*
MixedArray::SetRefInt(ArrayData* ad, int64_t k, Variant& v, bool copy) {
  auto a = asMixed(ad);
  assert(a->isMixed());
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->updateRef(k, v);
}

ArrayData*
MixedArray::SetRefStr(ArrayData* ad, StringData* k, Variant& v, bool copy) {
  auto a = asMixed(ad);
  assert(a->isMixed());
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->updateRef(k, v);
}

ArrayData*
MixedArray::AddInt(ArrayData* ad, int64_t k, Cell v, bool copy) {
  assert(!ad->exists(k));
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->addVal(k, v);
}

ArrayData*
MixedArray::AddStr(ArrayData* ad, StringData* k, Cell v, bool copy) {
  assert(!ad->exists(k));
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->addVal(k, v);
}

ArrayData*
MixedArray::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  auto a = asMixed(ad);
  a = a->resizeIfNeeded();
  return a->zSetImpl(k, v);
}

ArrayData*
MixedArray::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  auto a = asMixed(ad);
  a = a->resizeIfNeeded();
  return a->zSetImpl(k, v);
}

ArrayData*
MixedArray::ZAppend(ArrayData* ad, RefData* v, int64_t* key_ptr) {
  auto a = asMixed(ad);
  a = a->resizeIfNeeded();
  return a->zAppendImpl(v, key_ptr);
}

//=============================================================================
// Delete.

NEVER_INLINE
void MixedArray::adjustMArrayIter(ssize_t pos) {
  assert(pos >= 0 && pos < m_used);
  ssize_t eIPrev = Tombstone;
  for_each_strong_iterator([&] (MIterTable::Ent& miEnt) {
    if (miEnt.array != this) return;
    auto const iter = miEnt.iter;
    if (iter->getResetFlag()) return;
    if (iter->m_pos == pos) {
      if (eIPrev == Tombstone) {
        // eIPrev will actually be used, so properly initialize it with the
        // previous element before pos (or an invalid position if pos was the
        // first element).
        eIPrev = prevElm(data(), pos);
      }

      if (eIPrev == m_used) {
        iter->setResetFlag(true);
      }
      iter->m_pos = eIPrev;
    }
  });
}

void MixedArray::eraseNoCompact(ssize_t pos) {
  assert(validPos(pos));

  // move strong iterators to the previous element
  if (UNLIKELY(strong_iterators_exist())) adjustMArrayIter(pos);

  // If the internal pointer points to this element, advance it.
  Elm* elms = data();
  if (m_pos == pos) {
    m_pos = nextElm(elms, pos);
  }

  auto& e = elms[pos];
  // Mark the value as a tombstone.
  TypedValue* tv = &e.data;
  DataType oldType = tv->m_type;
  uint64_t oldDatum = tv->m_data.num;
  tv->m_type = kInvalidDataType;
  --m_size;
  // Mark the hash entry as "deleted".
  assert(m_used <= capacity());

  // Finally, decref the old value
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

ArrayData* MixedArray::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  auto pos = a->findForRemove(k, hash_int64(k), false);
  if (validPos(pos)) a->erase(pos);
  return a;
}

ArrayData*
MixedArray::RemoveStr(ArrayData* ad, const StringData* key, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  auto pos = a->findForRemove(key, key->hash());
  if (validPos(pos)) a->erase(pos);
  return a;
}

ArrayData* MixedArray::Copy(const ArrayData* ad) {
  return asMixed(ad)->copyMixed();
}

ArrayData* MixedArray::CopyWithStrongIterators(const ArrayData* ad) {
  auto a = asMixed(ad);
  auto copied = a->copyMixed();
  if (LIKELY(strong_iterators_exist())) {
    move_strong_iterators(copied, const_cast<MixedArray*>(a));
  }
  return copied;
}

//=============================================================================
// non-variant interface

const TypedValue* MixedArray::NvGetInt(const ArrayData* ad, int64_t ki) {
  auto a = asMixed(ad);
  auto i = a->find(ki, hash_int64(ki));
  return LIKELY(validPos(i)) ? &a->data()[i].data : nullptr;
}

#if !defined(__SSE4_2__) || defined(NO_HWCRC) || !defined(NO_M_DATA) || \
  defined(_MSC_VER)
// This function is implemented directly in ASM in mixed-array-x64.S otherwise.
const TypedValue* MixedArray::NvGetStr(const ArrayData* ad,
                                       const StringData* k) {
  auto a = asMixed(ad);
  auto i = a->find(k, k->hash());
  if (LIKELY(validPos(i))) {
    return &a->data()[i].data;
  }
  return nullptr;
}
#else
  // mixed-array-x64.S depends on StringData and MixedArray layout.
  // If these fail, update the constants
  static_assert(sizeof(StringData) == SD_DATA, "");
  static_assert(StringData::sizeOff() == SD_LEN, "");
  static_assert(StringData::hashOff() == SD_HASH, "");
  static_assert(MixedArray::dataOff() == MA_DATA, "");
  static_assert(MixedArray::scaleOff() == MA_SCALE, "");
  static_assert(MixedArray::Elm::keyOff() == ELM_KEY, "");
  static_assert(MixedArray::Elm::hashOff() == ELM_HASH, "");
  static_assert(MixedArray::Elm::dataOff() == ELM_DATA, "");
#endif

Cell MixedArray::NvGetKey(const ArrayData* ad, ssize_t pos) {
  auto a = asMixed(ad);
  assert(pos != a->m_used);
  assert(!isTombstone(a->data()[pos].data.m_type));
  return getElmKey(a->data()[pos]);
}

ArrayData* MixedArray::Append(ArrayData* ad, Cell v, bool copy) {
  auto a = asMixed(ad);
  if (UNLIKELY(a->m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return a;
  }
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  a->nextInsert(v);
  return a;
}

ArrayData* MixedArray::AppendRef(ArrayData* ad, Variant& v, bool copy) {
  auto a = asMixed(ad);
  assert(a->isMixed());

  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefNew();

  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();

  // Note: preserving behavior, but I think this can leak the copy if
  // the user error handler throws.
  if (UNLIKELY(a->m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return a;
  }

  // TODO: maybe inline manually (only caller).
  return a->nextInsertRef(v);
}

ArrayData* MixedArray::AppendWithRef(ArrayData* ad, const Variant& v,
                                     bool copy) {
  auto a = asMixed(ad);
  assert(a->isMixed());

  if (RuntimeOption::EvalHackArrCompatNotices && v.isReferenced()) {
    raiseHackArrCompatRefNew();
  }

  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->nextInsertWithRef(v);
}

/*
 * Copy an array to a new array of mixed kind, with a particular
 * pre-reserved size.
 */
NEVER_INLINE
MixedArray* MixedArray::CopyReserve(const MixedArray* src,
                                    size_t expectedSize) {
  auto const scale = computeScaleFromSize(expectedSize);
  auto const ad    = reqAllocArray(scale);
  auto const oldUsed = src->m_used;

  ad->m_sizeAndPos      = src->m_sizeAndPos;
  ad->initHeader(*src, 1);
  ad->m_scale           = scale; // don't set m_used yet
  ad->m_nextKI          = src->m_nextKI;

  auto const data  = ad->data();
  auto const table = mixedHash(data, scale);
  ad->initHash(table, scale);

  auto dstElm = data;
  auto srcElm = src->data();
  auto const srcStop = src->data() + oldUsed;
  uint32_t i = 0;

  // We're not copying the tombstones over to the new array, so the
  // positions of the elements in the new array may be shifted. Cache
  // the key for element associated with src->m_pos so that we can
  // properly initialize ad->m_pos below.
  ElmKey mPos;
  bool updatePosAfterCopy = src->m_pos != 0 && src->m_pos < src->m_used;
  if (updatePosAfterCopy) {
    assert(size_t(src->m_pos) < src->m_used);
    auto& e = srcElm[src->m_pos];
    mPos.hash = e.probe();
    mPos.skey = e.skey;
  }

  // Copy the elements
  auto mask = MixedArray::Mask(scale);
  for (; srcElm != srcStop; ++srcElm) {
    if (srcElm->isTombstone()) continue;
    tvDupFlattenVars(&srcElm->data, &dstElm->data, src);
    auto const hash = static_cast<int32_t>(srcElm->probe());
    if (hash < 0) {
      dstElm->setIntKey(srcElm->ikey, hash);
    } else {
      dstElm->setStrKey(srcElm->skey, hash);
    }
    *ad->findForNewInsert(table, mask, hash) = i;
    ++dstElm;
    ++i;
  }

  // Now that we have finished copying the elements, update ad->m_pos
  if (updatePosAfterCopy) {
    ad->m_pos = mPos.hash >= 0 ? ssize_t(ad->find(mPos.skey, mPos.hash))
      : ssize_t(ad->find(mPos.ikey, mPos.hash));
    assert(ad->m_pos >=0 && ad->m_pos < ad->m_size);
  } else {
    // If src->m_pos is equal to src's canonical invalid position, then
    // set ad->m_pos to ad's canonical invalid position.
    if (src->m_pos != 0)
      ad->m_pos = ad->m_size;
  }

  // Set new used value (we've removed any tombstones).
  assert(i == dstElm - data);
  ad->m_used = i;

  assert(ad->kind() == src->kind());
  assert(ad->m_size == src->m_size);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_used <= oldUsed);
  assert(ad->m_used == dstElm - data);
  assert(ad->m_scale == scale);
  assert(ad->m_nextKI == src->m_nextKI);
  assert(ad->checkInvariants());
  return ad;
}

NEVER_INLINE
ArrayData* MixedArray::ArrayPlusEqGeneric(ArrayData* ad,
                                          MixedArray* ret,
                                          const ArrayData* elems,
                                          size_t neededSize) {
  assert(ad->isPHPArray());
  assert(elems->isPHPArray());
  assert(ret->isMixed());

  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    const Variant& value = it.secondRef();

    if (UNLIKELY(ret->isFull())) {
      assert(ret == ad);
      ret = CopyReserve(asMixed(ad), neededSize);
    }

    auto tv = key.asTypedValue();
    auto p = tv->m_type == KindOfInt64
      ? ret->insert(tv->m_data.num)
      : ret->insert(tv->m_data.pstr);
    if (!p.found) {
      ret->initWithRef(p.tv, value);
    }
  }

  return ret;
}

// Note: the logic relating to how to grow in this function is coupled
// to PackedArray::PlusEq.
ArrayData* MixedArray::PlusEq(ArrayData* ad, const ArrayData* elems) {
  assertx(asMixed(ad)->checkInvariants());

  if (!ad->isPHPArray()) throwInvalidAdditionException(ad);
  if (!elems->isPHPArray()) throwInvalidAdditionException(elems);

  auto const neededSize = ad->size() + elems->size();

  auto ret =
    ad->cowCheck() ? CopyReserve(asMixed(ad), neededSize) :
    asMixed(ad);

  if (UNLIKELY(!elems->hasMixedLayout())) {
    return ArrayPlusEqGeneric(ad, ret, elems, neededSize);
  }

  auto const rhs = asMixed(elems);

  auto srcElem = rhs->data();
  auto const srcStop = rhs->data() + rhs->m_used;
  for (; srcElem != srcStop; ++srcElem) {
    if (srcElem->isTombstone()) continue;

    if (UNLIKELY(ret->isFull())) {
      assert(ret == ad);
      ret = CopyReserve(ret, neededSize);
    }

    auto const hash = srcElem->hash();
    if (srcElem->hasStrKey()) {
      auto const ei = ret->findForInsert(srcElem->skey, hash);
      if (validPos(*ei)) continue;
      auto& e = ret->allocElm(ei);
      e.setStrKey(srcElem->skey, hash);
      ret->initWithRef(e.data, tvAsCVarRef(&srcElem->data));
    } else {
      auto const ei = ret->findForInsert(srcElem->ikey, hash);
      if (validPos(*ei)) continue;
      auto& e = ret->allocElm(ei);
      e.setIntKey(srcElem->ikey, hash);
      ret->initWithRef(e.data, tvAsCVarRef(&srcElem->data));
    }
  }

  return ret;
}

NEVER_INLINE
ArrayData* MixedArray::ArrayMergeGeneric(MixedArray* ret,
                                         const ArrayData* elems) {
  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    const Variant& value = it.secondRef();
    if (key.asTypedValue()->m_type == KindOfInt64) {
      ret->nextInsertWithRef(value);
    } else {
      StringData* sd = key.getStringData();
      auto const r = ret->addLvalImpl(sd);
      r.val->setWithRef(value);
    }
  }
  return ret;
}

ArrayData* MixedArray::Merge(ArrayData* ad, const ArrayData* elems) {
  assert(asMixed(ad)->checkInvariants());
  auto const ret = CopyReserve(asMixed(ad), ad->size() + elems->size());
  assert(ret->hasExactlyOneRef());
  ret->initHeader(HeaderKind::Mixed, 1);

  if (elems->hasMixedLayout()) {
    auto const rhs = asMixed(elems);
    auto srcElem = rhs->data();
    auto const srcStop = rhs->data() + rhs->m_used;

    for (; srcElem != srcStop; ++srcElem) {
      if (isTombstone(srcElem->data.m_type)) continue;

      if (srcElem->hasIntKey()) {
        ret->nextInsertWithRef(tvAsCVarRef(&srcElem->data));
      } else {
        auto const r = ret->addLvalImpl(srcElem->skey);
        r.val->setWithRef(tvAsCVarRef(&srcElem->data));
      }
    }
    return ret;
  }

  if (UNLIKELY(!elems->hasPackedLayout())) {
    return ArrayMergeGeneric(ret, elems);
  }

  assert(PackedArray::checkInvariants(elems));
  auto src           = packedData(elems);
  auto const srcStop = src + elems->m_size;
  for (; src != srcStop; ++src) {
    ret->nextInsertWithRef(tvAsCVarRef(src));
  }

  return ret;

  // Note: currently caller is responsible for calling renumber after
  // this.  Should refactor so we handle it (we already know things
  // about the array).
}

ArrayData* MixedArray::Pop(ArrayData* ad, Variant& value) {
  auto a = asMixed(ad);
  if (a->cowCheck()) a = a->copyMixed();
  auto elms = a->data();
  if (a->m_size) {
    ssize_t pos = IterLast(a);
    assert(pos >= 0 && pos < a->m_used);
    auto& e = elms[pos];
    assert(!isTombstone(e.data.m_type));
    value = tvAsCVarRef(&e.data);
    auto pos2 = e.hasStrKey() ? a->findForRemove(e.skey, e.hash())
                              : a->findForRemove(e.ikey, e.hash(), true);
    assert(pos2 == pos);
    a->erase(pos2);
  } else {
    value = uninit_null();
  }
  // To conform to PHP5 behavior, the pop operation resets the array's
  // internal iterator.
  a->m_pos = a->nextElm(elms, -1);
  return a;
}

ArrayData* MixedArray::Dequeue(ArrayData* adInput, Variant& value) {
  auto a = asMixed(adInput);
  if (a->cowCheck()) a = a->copyMixed();
  auto elms = a->data();
  if (a->m_size) {
    ssize_t pos = a->nextElm(elms, -1);
    assert(pos >= 0 && pos < a->m_used);
    auto& e = elms[pos];
    assert(!isTombstone(e.data.m_type));
    value = tvAsCVarRef(&e.data);
    auto pos2 = e.hasStrKey() ? a->findForRemove(e.skey, e.hash())
                              : a->findForRemove(e.ikey, e.hash(), false);
    assert(pos2 == pos);
    a->erase(pos2);
  } else {
    value = uninit_null();
  }
  // Even if the array is empty, for PHP5 conformity we need call
  // compact() because it has side-effects that are important
  a->compact(true);
  return a;
}

ArrayData* MixedArray::Prepend(ArrayData* adInput, Cell v, bool copy) {
  auto a = asMixed(adInput);
  if (a->cowCheck()) a = a->copyMixedAndResizeIfNeeded();

  auto elms = a->data();
  if (a->m_used == 0 || !isTombstone(elms[0].data.m_type)) {
    // Make sure there is room to insert an element.
    a = a->resizeIfNeeded();
    // Recompute elms, in case resizeIfNeeded() had side effects.
    elms = a->data();
    // Move the existing elements to make element 0 available.
    memmove(&elms[1], &elms[0], a->m_used * sizeof(*elms));
    ++a->m_used;
  }

  // Prepend.
  ++a->m_size;
  auto& e = elms[0];
  e.setIntKey(0, hash_int64(0));
  cellDup(v, e.data);

  // Renumber.
  a->compact(true);
  return a;
}

ArrayData* MixedArray::ToPHPArray(ArrayData* ad, bool) {
  assert(asMixed(ad)->checkInvariants());
  assert(ad->isPHPArray());
  return ad;
}

ArrayData* MixedArray::ToPHPArrayDict(ArrayData* adIn, bool copy) {
  assertx(adIn->isDict());

  auto a = asMixed(adIn);
  assertx(a->checkInvariants());

  auto const size = a->size();
  auto const elms = a->data();

  if (!size) return staticEmptyArray();

  // If we don't necessarily have to make a copy, first scan the dict looking
  // for any int-like string keys. If we don't find any, we can transform the
  // dict in place.
  if (!copy) {
    for (uint32_t i = 0, limit = a->m_used; i < limit; ++i) {
      auto& e = elms[i];
      if (e.isTombstone()) continue;
      if (e.hasStrKey()) {
        int64_t ignore;
        if (e.skey->isStrictlyInteger(ignore)) {
          copy = true;
          break;
        }
      }
    }
  }

  if (!copy) {
    // No int-like string keys, so transform in place.
    a->m_kind = HeaderKind::Mixed;
    assertx(a->checkInvariants());
    return a;
  }

  // Either we need to make a copy anyways, or we don't, but there are int-like
  // string keys. In either case, create the array from scratch, inserting each
  // element one-by-one, doing key conversion as necessary.
  auto newAd = asMixed(MakeReserveMixed(size));
  for (uint32_t i = 0, limit = a->m_used; i < limit; ++i) {
    auto& e = elms[i];
    if (e.isTombstone()) continue;
    if (e.hasIntKey()) {
      newAd->update(e.ikey, *tvAssertCell(&e.data));
    } else {
      int64_t n;
      if (e.skey->isStrictlyInteger(n)) {
        newAd->update(n, *tvAssertCell(&e.data));
      } else {
        newAd->update(e.skey, *tvAssertCell(&e.data));
      }
    }
  }

  assertx(newAd->checkInvariants());
  assertx(newAd->hasExactlyOneRef());
  return newAd;
}

MixedArray* MixedArray::ToDictInPlace(ArrayData* ad) {
  auto a = asMixed(ad);
  assert(a->isMixed());
  assert(!a->cowCheck());
  a->m_kind = HeaderKind::Dict;
  return a;
}

ArrayData* MixedArray::ToDict(ArrayData* ad, bool copy) {
  auto a = asMixed(ad);
  assert(a->isMixed());

  if (copy) {
    return CopyMixed(*a, AllocMode::Request, HeaderKind::Dict);
  } else {
    auto const result = ArrayCommon::CheckForRefs(a);
    if (LIKELY(result == ArrayCommon::RefCheckResult::Pass)) {
      return ToDictInPlace(a);
    } else if (result == ArrayCommon::RefCheckResult::Collapse) {
      // Remove unreferenced refs
      return CopyMixed(*a, AllocMode::Request, HeaderKind::Dict);
    } else {
      throwRefInvalidArrayValueException(staticEmptyDictArray());
    }
  }
}

ArrayData* MixedArray::ToDictDict(ArrayData* ad, bool) {
  assert(asMixed(ad)->checkInvariants());
  assert(ad->isDict());
  return ad;
}

void MixedArray::Renumber(ArrayData* ad) {
  asMixed(ad)->compact(true);
}

void MixedArray::OnSetEvalScalar(ArrayData* ad) {
  auto a = asMixed(ad);
  auto elms = a->data();
  for (uint32_t i = 0, limit = a->m_used; i < limit; ++i) {
    auto& e = elms[i];
    if (!isTombstone(e.data.m_type)) {
      auto key = e.skey;
      if (e.hasStrKey() && !key->isStatic()) {
        e.skey = makeStaticString(key);
        decRefStr(key);
      }
      tvAsVariant(&e.data).setEvalScalar();
    }
  }
}

bool MixedArray::AdvanceMArrayIter(ArrayData* ad, MArrayIter& fp) {
  auto a = asMixed(ad);
  Elm* elms = a->data();
  if (fp.getResetFlag()) {
    fp.setResetFlag(false);
    fp.m_pos = a->nextElm(elms, -1);
  } else if (fp.m_pos == a->m_used) {
    return false;
  } else {
    fp.m_pos = a->nextElm(elms, fp.m_pos);
  }
  if (fp.m_pos == a->m_used) {
    return false;
  }
  // To conform to PHP5 behavior, we need to set the internal
  // cursor to point to the next element.
  a->m_pos = a->nextElm(elms, fp.m_pos);
  return true;
}

//////////////////////////////////////////////////////////////////////

const TypedValue* MixedArray::NvTryGetIntDict(const ArrayData* ad, int64_t ki) {
  assert(asMixed(ad)->checkInvariants());
  assert(ad->isDict());
  auto const tv = MixedArray::NvGetInt(ad, ki);
  if (UNLIKELY(!tv)) throwOOBArrayKeyException(ki, ad);
  return tv;
}

const TypedValue* MixedArray::NvTryGetStrDict(const ArrayData* ad,
                                              const StringData* k) {
  assert(asMixed(ad)->checkInvariants());
  assert(ad->isDict());
  auto const tv = MixedArray::NvGetStr(ad, k);
  if (UNLIKELY(!tv)) throwOOBArrayKeyException(k, ad);
  return tv;
}

ArrayLval
MixedArray::LvalIntRefDict(ArrayData* adIn, int64_t, bool) {
  assert(asMixed(adIn)->checkInvariants());
  assert(adIn->isDict());
  throwRefInvalidArrayValueException(adIn);
}

ArrayLval
MixedArray::LvalStrRefDict(ArrayData* adIn, StringData*, bool) {
  assert(asMixed(adIn)->checkInvariants());
  assert(adIn->isDict());
  throwRefInvalidArrayValueException(adIn);
}

ArrayLval MixedArray::LvalNewRefDict(ArrayData* adIn, bool) {
  assert(asMixed(adIn)->checkInvariants());
  assert(adIn->isDict());
  throwRefInvalidArrayValueException(adIn);
}

ArrayData* MixedArray::SetRefIntDict(ArrayData* adIn, int64_t, Variant&, bool) {
  assert(asMixed(adIn)->checkInvariants());
  assert(adIn->isDict());
  throwRefInvalidArrayValueException(adIn);
}

ArrayData*
MixedArray::SetRefStrDict(ArrayData* adIn, StringData*, Variant&, bool) {
  assert(asMixed(adIn)->checkInvariants());
  assert(adIn->isDict());
  throwRefInvalidArrayValueException(adIn);
}

ArrayData* MixedArray::AppendRefDict(ArrayData* adIn, Variant&, bool) {
  assert(asMixed(adIn)->checkInvariants());
  assert(adIn->isDict());
  throwRefInvalidArrayValueException(adIn);
}

ArrayData*
MixedArray::AppendWithRefDict(ArrayData* adIn, const Variant& v, bool copy) {
  assert(asMixed(adIn)->checkInvariants());
  assert(adIn->isDict());
  if (v.isReferenced()) throwRefInvalidArrayValueException(adIn);
  auto const cell = LIKELY(v.getType() != KindOfUninit)
    ? *v.asCell()
    : make_tv<KindOfNull>();
  return Append(adIn, cell, copy);
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
bool MixedArray::DictEqualHelper(const ArrayData* ad1, const ArrayData* ad2,
                                 bool strict) {
  assert(asMixed(ad1)->checkInvariants());
  assert(asMixed(ad2)->checkInvariants());
  assert(ad1->isDict());
  assert(ad2->isDict());

  if (strict && ad1 == ad2) return true;
  if (ad1->size() != ad2->size()) return false;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  if (strict) {
    auto const arr1 = asMixed(ad1);
    auto const arr2 = asMixed(ad2);
    auto elm1 = arr1->data();
    auto elm2 = arr2->data();
    auto i1 = arr1->m_used;
    auto i2 = arr2->m_used;
    while (i1 > 0 && i2 > 0) {
      if (UNLIKELY(elm1->isTombstone())) {
        ++elm1;
        --i1;
        continue;
      }
      if (UNLIKELY(elm2->isTombstone())) {
        ++elm2;
        --i2;
        continue;
      }
      if (elm1->hasIntKey()) {
        if (!elm2->hasIntKey()) return false;
        if (elm1->ikey != elm2->ikey) return false;
      } else {
        assertx(elm1->hasStrKey());
        if (!elm2->hasStrKey()) return false;
        if (!same(elm1->skey, elm2->skey)) return false;
      }
      if (!cellSame(*tvAssertCell(&elm1->data), *tvAssertCell(&elm2->data))) {
        return false;
      }
      ++elm1;
      ++elm2;
      --i1;
      --i2;
    }

    if (!i1) {
      while (i2 > 0) {
        if (UNLIKELY(!elm2->isTombstone())) return false;
        ++elm2;
        --i2;
      }
    } else {
      assertx(!i2);
      while (i1 > 0) {
        if (UNLIKELY(!elm1->isTombstone())) return false;
        ++elm1;
        --i1;
      }
    }
  } else {
    auto const arr1 = asMixed(ad1);
    auto elm = arr1->data();
    for (auto i = arr1->m_used; i--; elm++) {
      if (UNLIKELY(elm->isTombstone())) continue;
      const TypedValue* other;
      if (elm->hasIntKey()) {
        other = NvGetIntDict(ad2, elm->ikey);
      } else {
        assertx(elm->hasStrKey());
        other = NvGetStrDict(ad2, elm->skey);
      }
      if (!other ||
          !cellEqual(*tvAssertCell(&elm->data), *tvAssertCell(other))) {
        return false;
      }
    }
  }

  return true;
}

bool MixedArray::DictEqual(const ArrayData* ad1, const ArrayData* ad2) {
  return DictEqualHelper(ad1, ad2, false);
}

bool MixedArray::DictNotEqual(const ArrayData* ad1, const ArrayData* ad2) {
  return !DictEqualHelper(ad1, ad2, false);
}

bool MixedArray::DictSame(const ArrayData* ad1, const ArrayData* ad2) {
  return DictEqualHelper(ad1, ad2, true);
}

bool MixedArray::DictNotSame(const ArrayData* ad1, const ArrayData* ad2) {
  return !DictEqualHelper(ad1, ad2, true);
}

//////////////////////////////////////////////////////////////////////

Cell MixedArray::MemoGet(const TypedValue* base,
                         const TypedValue* keys,
                         uint32_t nkeys) {
  assertx(nkeys > 0);

  auto const getPos = [&](const Cell& key, const MixedArray* a) {
    if (key.m_type == KindOfInt64) {
      auto const idx = key.m_data.num;
      return a->find(idx, hash_int64(idx));
    } else {
      assertx(tvIsString(&key));
      auto const str = key.m_data.pstr;
      return a->find(str, str->hash());
    }
  };

  auto const arr = tvToCell(base);
  assertx(cellIsPlausible(*arr));
  assertx(tvIsDict(arr));

  // Consume all but the last key, walking through the chain of nested
  // dictionaries.
  auto current = asMixed(arr->m_data.parr);
  assertx(current->isDict());
  for (auto i = uint32_t{0}; i < nkeys - 1; ++i) {
    auto const pos = getPos(*tvAssertCell(keys - i), current);
    if (UNLIKELY(!validPos(pos))) return make_tv<KindOfUninit>();
    auto const& tv = current->data()[pos].data;
    assertx(cellIsPlausible(tv));
    assertx(tvIsDict(&tv));
    current = asMixed(tv.m_data.parr);
    assertx(current->isDict());
  }

  // Consume the last key, which should result in the actual value (or Uninit if
  // not present).
  auto const pos = getPos(*tvAssertCell(keys - nkeys + 1), current);
  if (UNLIKELY(!validPos(pos))) return make_tv<KindOfUninit>();
  Cell out;
  cellDup(current->data()[pos].data, out);
  assertx(cellIsPlausible(out));
  return out;
}

void MixedArray::MemoSet(TypedValue* startBase,
                         const Cell* keys, uint32_t nkeys,
                         Cell val) {
  assertx(nkeys > 0);
  assertx(cellIsPlausible(val));

  auto const getInsert = [](const Cell& key, MixedArray* a) {
    if (key.m_type == KindOfInt64) {
      return a->insert(key.m_data.num);
    } else {
      assertx(tvIsString(&key));
      return a->insert(key.m_data.pstr);
    }
  };

  // If the given dict can't be updated in place, create a new copy, store it in
  // the base, and return that. For the vast majority of cases, the dict can be
  // updated in place because the only reference to it will be the memo
  // cache. However reflection can get copies of it, so its not guaranteed to
  // always have a ref-count of 1.
  auto const cow = [](bool copy, MixedArray* a, Cell& base) {
    auto copied = copy
      ? a->copyMixedAndResizeIfNeeded()
      : a->resizeIfNeeded();
    if (a != copied) {
      a = copied;
      cellMove(make_tv<KindOfDict>(a), base);
    }
    return a;
  };

  auto base = tvToCell(startBase);
  assertx(cellIsPlausible(*base));
  assertx(tvIsDict(base));

  // Consume all but the last key, walking through the chain of nested
  // dictionaries, cowing them as necessary.
  auto current = asMixed(base->m_data.parr);
  assertx(current->isDict());
  for (auto i = uint32_t{0}; i < nkeys - 1; ++i) {
    current = cow(current->cowCheck(), current, *base);
    auto const p = getInsert(*tvAssertCell(keys - i), current);
    if (!p.found) {
      // Extend the chain
      p.tv.m_type = KindOfDict;
      p.tv.m_data.parr = MakeReserveDict(0);
    }
    assertx(cellIsPlausible(p.tv));
    assertx(tvIsDict(&p.tv));
    current = asMixed(p.tv.m_data.parr);
    assertx(current->isDict());
    base = tvAssertCell(&p.tv);
  }

  current = cow(
    current->cowCheck() || (tvIsDict(&val) && val.m_data.parr == current),
    current,
    *base
  );
  // Consume the last key, storing the actual value (and overwriting a previous
  // value if present).
  auto const p = getInsert(*tvAssertCell(keys - nkeys + 1), current);
  p.found ? cellSet(val, *tvAssertCell(&p.tv)) : cellDup(val, p.tv);
}

/////////////////////////////////////////////////////////////////////

}
