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
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/array-helpers.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/empty-array.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/member-val.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
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

namespace HPHP {

TRACE_SET_MOD(runtime);

static_assert(MixedArray::computeAllocBytes(MixedArray::SmallScale) ==
              kEmptyMixedArraySize, "");

std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyDictArray;

struct MixedArray::Initializer {
  Initializer() {
    auto const ad = reinterpret_cast<MixedArray*>(&s_theEmptyDictArray);
    ad->initHash(1);
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
  auto const ad    = reqAlloc(scale);

  // Intialize the hash table first, because the header is already in L1 cache,
  // but the hash table may not be.  So let's issue the cache request ASAP.
  ad->initHash(scale);

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
  auto const ad    = reqAlloc(scale);

  ad->m_sizeAndPos       = size; // pos=0
  ad->initHeader(HeaderKind::Mixed, 1);
  ad->m_scale_used       = scale | uint64_t{size} << 32; // used=size
  ad->m_nextKI           = 0;

  auto const table = ad->initHash(scale);
  auto const mask = ad->mask();
  auto const data = ad->data();

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
    auto ei = ad->findForNewInsert(table, mask, h);
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
  auto const ad    = reqAlloc(scale);

  ad->initHash(scale);

  ad->m_sizeAndPos       = size; // pos=0
  ad->initHeader(HeaderKind::Mixed, 1);
  ad->m_scale_used       = scale | uint64_t{size} << 32; // used=size
  ad->m_nextKI           = 0;

  // Append values by moving -- no refcounts are updated.
  auto const data = ad->data();
  for (uint32_t i = 0; i < size; i++) {
    auto& kTv = keysAndValues[i * 2];
    if (kTv.m_type == KindOfString) {
      auto k = kTv.m_data.pstr;
      auto h = k->hash();
      auto ei = ad->findForInsertUpdate(k, h);
      if (isValidPos(ei)) {
        // it's the caller's responsibility to free keysAndValues
        ad->setZombie();
        Release(ad);
        return nullptr;
      }
      data[i].setStrKeyNoIncRef(k, h);
      *ei = i;
    } else {
      assert(kTv.m_type == KindOfInt64);
      auto k = kTv.m_data.num;
      auto h = hash_int64(k);
      auto ei = ad->findForInsertUpdate(k, h);
      if (isValidPos(ei)) {
        // it's the caller's responsibility to free keysAndValues
        ad->setZombie();
        Release(ad);
        return nullptr;
      }
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
  auto const ad = mode == AllocMode::Request ? reqAlloc(scale)
                                             : staticAlloc(scale);

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
  CopyHash(ad->hashTab(), other.hashTab(), scale);

  // Bump up refcounts as needed.
  auto const elms = ad->data();
  for (uint32_t i = 0, limit = ad->m_used; i < limit; ++i) {
    auto& e = elms[i];
    if (UNLIKELY(e.isTombstone())) continue;
    if (e.hasStrKey()) e.skey->incRefCount();
    if (UNLIKELY(e.data.m_type == KindOfRef)) {
      auto ref = e.data.m_data.pref;
      // See also tvDupWithRef()
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
    tvIncRefGen(&e.data);
  }

  assert(ad->m_used == other.m_used);
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

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE static bool UncountedMixedArrayOnHugePage() {
#ifdef USE_JEMALLOC_CUSTOM_HOOKS
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
  CopyHash(ad->hashTab(), a->hashTab(), scale);

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
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().addAPCUncountedBlock();
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
      tvDecRefGen(&ptr->data);
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

    // We better not have strong iterators associated with uncounted arrays.
    assert(!has_strong_iterator(ad));
  }
  if (UncountedMixedArrayOnHugePage()) {
    free_huge(reinterpret_cast<char*>(ad) - extra);
  } else {
    free(reinterpret_cast<char*>(ad) - extra);
  }
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().removeAPCUncountedBlock();
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

size_t MixedArray::Vsize(const ArrayData*) { not_reached(); }

member_rval::ptr_u MixedArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  auto a = asMixed(ad);
  assert(a->checkInvariants());
  assert(pos != a->m_used);
  auto const& e = a->data()[pos];
  assert(!e.isTombstone());
  return &e.data;
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

NEVER_INLINE
int32_t* warnUnbalanced(MixedArray* a, size_t n, int32_t* ei) {
  if (n > size_t(RuntimeOption::MaxArrayChain)) {
    decRefArr(a->asArrayData()); // otherwise, a leaks when exn propagates
    raise_error("Array is too unbalanced (%lu)", n);
  }
  return ei;
}

MixedArray::InsertPos MixedArray::insert(int64_t k) {
  assert(!isFull());
  auto h = hash_int64(k);
  auto ei = findForInsertUpdate(k, h);
  if (isValidPos(ei)) {
    return InsertPos(true, data()[(int32_t)*ei].data);
  }
  if (k >= m_nextKI && m_nextKI >= 0) m_nextKI = k + 1;
  auto e = allocElm(ei);
  e->setIntKey(k, h);
  return InsertPos(false, e->data);
}

MixedArray::InsertPos MixedArray::insert(StringData* k) {
  assert(!isFull());
  auto const h = k->hash();
  auto ei = findForInsertUpdate(k, h);
  if (isValidPos(ei)) {
    return InsertPos(true, data()[(int32_t)*ei].data);
  }
  auto e = allocElm(ei);
  e->setStrKey(k, h);
  return InsertPos(false, e->data);
}

NEVER_INLINE
int32_t MixedArray::findForRemove(int64_t ki, inthash_t h, bool updateNext) {
  // all vector methods should work w/out touching the hashtable
  return findForRemove(ki, h,
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

int32_t MixedArray::findForRemove(const StringData* s, strhash_t h) {
  return findForRemove(s, h,
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
  tvBoxIfNeeded(v.asTypedValue());
  refDup(*v.asTypedValue(), tv);
  return this;
}

ALWAYS_INLINE
MixedArray* MixedArray::initWithRef(TypedValue& tv, TypedValue v) {
  tvWriteNull(&tv);
  tvAsVariant(&tv).setWithRef(v);
  return this;
}

ALWAYS_INLINE
MixedArray* MixedArray::initWithRef(TypedValue& tv, const Variant& v) {
  return initWithRef(tv, *v.asTypedValue());
}

ALWAYS_INLINE
ArrayData* MixedArray::zSetVal(TypedValue& tv, RefData* v) {
  // Dec ref the old value
  tvDecRefGen(tv);
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

NEVER_INLINE MixedArray*
MixedArray::InsertCheckUnbalanced(MixedArray* ad,
                                  int32_t* table,
                                  uint32_t mask,
                                  Elm* iter,
                                  Elm* stop) {
  for (uint32_t i = 0; iter != stop; ++iter, ++i) {
    auto& e = *iter;
    if (e.isTombstone()) continue;
    *ad->findForNewInsertWarn(table,
                              mask,
                              e.probe()) = i;
  }
  return ad;
}

NEVER_INLINE
MixedArray*
MixedArray::Grow(MixedArray* old, uint32_t newScale, bool copy) {
  assert(old->m_size > 0);
  assert(MixedArray::Capacity(newScale) >= old->m_size);
  assert(newScale >= 1 && (newScale & (newScale - 1)) == 0);

  auto ad            = reqAlloc(newScale);
  auto const oldUsed = old->m_used;
  ad->m_sizeAndPos   = old->m_sizeAndPos;
  ad->initHeader(*old, 1);
  ad->m_scale_used   = newScale | uint64_t{oldUsed} << 32;

  copyElmsNextUnsafe(ad, old, oldUsed);
  if (copy) {
    auto elm = ad->data();
    for (auto const end = elm + ad->m_used; elm < end; ++elm) {
      if (UNLIKELY(elm->isTombstone())) continue;
      if (elm->hasStrKey()) elm->skey->incRefCount();
      if (UNLIKELY(elm->data.m_type == KindOfRef)) {
        auto ref = elm->data.m_data.pref;
        // See also tvDupWithRef()
        if (!ref->isReferenced() && ref->tv()->m_data.parr != old) {
          cellDup(*ref->tv(), elm->data);
          continue;
        }
      }
      tvIncRefGen(&elm->data);
    }
  } else {
    if (UNLIKELY(strong_iterators_exist())) move_strong_iterators(ad, old);
    old->setZombie();
  }

  auto const table = ad->initHash(newScale);

  auto iter = ad->data();
  auto const stop = iter + oldUsed;
  assert(newScale == ad->m_scale);
  auto mask = MixedArray::Mask(newScale);

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

  assert(ad->kind() == old->kind());
  assert(ad->m_size == old->m_size);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_pos == old->m_pos);
  assert(ad->m_used == oldUsed);
  assert(ad->m_scale == newScale);
  assert(ad->checkInvariants());
  return ad;
}

ALWAYS_INLINE
MixedArray* MixedArray::prepareForInsert(bool copy) {
  assert(checkInvariants());
  if (isFull()) return Grow(this, m_scale * 2, copy);
  if (copy) return copyMixed();
  return this;
}

void MixedArray::compact(bool renumber /* = false */) {
  bool updatePosAfterCompact = false;
  ElmKey mPos;
  bool updateStrongIters = false;
  req::TinyVector<ElmKey,3> siKeys;

  // Prep work before beginning the compaction process
  if (LIKELY(!renumber)) {
    if (m_pos == m_used) {
      // If m_pos is the canonical invalid position, we need to update it to
      // what the new canonical invalid position will be after compaction
      m_pos = m_size;
    } else if (m_pos != 0) {
      // Cache key for element associated with m_pos in order to
      // update m_pos after the compaction has been performed.
      // We only need to do this if m_pos is nonzero and is not
      // the canonical invalid position.
      updatePosAfterCompact = true;
      assert(size_t(m_pos) < m_used);
      auto& e = data()[m_pos];
      mPos.hash = e.hash();
      mPos.skey = e.skey;
    }
    if (UNLIKELY(strong_iterators_exist())) {
      for_each_strong_iterator([&] (const MIterTable::Ent& miEnt) {
        if (miEnt.array != this) return;
        if (miEnt.iter->getResetFlag()) return;
        updateStrongIters = true;
        auto const ei = miEnt.iter->m_pos;
        if (ei == m_used) return;
        auto& e = data()[ei];
        siKeys.push_back(ElmKey(e.hash(), e.skey));
      });
    }
  } else {
    // To conform to PHP behavior, when array's integer keys are renumbered
    // we invalidate all strong iterators and we reset the array's internal
    // cursor (even if the array is empty or has no integer keys).
    if (UNLIKELY(strong_iterators_exist())) {
      free_strong_iterators(this);
    }
    m_pos = 0;
    // Set m_nextKI to 0 for now to prepare for renumbering integer keys
    m_nextKI = 0;
  }

  // Perform compaction
  auto elms = data();
  auto const mask = this->mask();
  auto const table = initHash(m_scale);
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

  if (LIKELY(!updateStrongIters)) {
    // In the common case there aren't any strong iterators that need updating,
    // so we can just update m_used and return
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
  assert(!isValidPos(ei));
  // Allocate and initialize a new element.
  auto e = allocElm(ei);
  e->setIntKey(ki, h);
  m_nextKI = ki + 1; // Update next free element.
  cellDup(v, e->data);
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
  auto e = allocElm(ei);
  e->setIntKey(ki, h);
  m_nextKI = ki + 1; // Update next free element.
  return initRef(e->data, data);
}

ArrayData* MixedArray::nextInsertWithRef(TypedValue data) {
  assert(!isFull());

  int64_t ki = m_nextKI;
  auto h = hash_int64(ki);
  auto ei = findForNewInsert(h);
  assert(!isValidPos(ei));

  // Allocate a new element.
  auto e = allocElm(ei);
  e->setIntKey(ki, h);
  m_nextKI = ki + 1; // Update next free element.
  return initWithRef(e->data, data);
}

ALWAYS_INLINE
ArrayData* MixedArray::nextInsertWithRef(const Variant& data) {
  return nextInsertWithRef(*data.asTypedValue());
}

template <class K> ALWAYS_INLINE
ArrayData* MixedArray::update(K k, Cell data) {
  assert(!isFull());
  auto p = insert(k);
  if (p.found) {
    // TODO(#3888164): we should restructure things so we don't have
    // to check KindOfUninit here.
    setElem(p.tv, data);
    return this;
  }
  // TODO(#3888164): we should restructure things so we don't have to
  // check KindOfUninit here.
  initElem(p.tv, data);
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
  assert(!isValidPos(ei));
  auto e = allocElm(ei);
  e->setIntKey(ki, h);
  m_nextKI = ki + 1;
  *key_ptr = ki;
  return zInitVal(e->data, data);
}

member_lval MixedArray::LvalInt(ArrayData* ad, int64_t k, bool copy) {
  return asMixed(ad)->prepareForInsert(copy)->addLvalImpl<true>(k);
}

member_lval MixedArray::LvalIntRef(ArrayData* ad, int64_t k, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);
  return LvalInt(ad, k, copy);
}

member_lval MixedArray::LvalStr(ArrayData* ad, StringData* key, bool copy) {
  return asMixed(ad)->prepareForInsert(copy)->addLvalImpl<true>(key);
}

member_lval MixedArray::LvalStrRef(ArrayData* ad, StringData* key, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(key);
  return LvalStr(ad, key, copy);
}

member_lval MixedArray::LvalSilentInt(ArrayData* ad, int64_t k, bool copy) {
  auto a = asMixed(ad);
  auto const pos = a->find(k, hash_int64(k));
  if (UNLIKELY(!validPos(pos))) return member_lval { a, nullptr };
  if (copy) a = a->copyMixed();
  return member_lval { a, &a->data()[pos].data };
}

member_lval MixedArray::LvalSilentStr(ArrayData* ad, const StringData* k,
                                  bool copy) {
  auto a = asMixed(ad);
  auto const pos = a->find(k, k->hash());
  if (UNLIKELY(!validPos(pos))) return member_lval { a, nullptr };
  if (copy) a = a->copyMixed();
  return member_lval { a, &a->data()[pos].data };
}

member_lval MixedArray::LvalNew(ArrayData* ad, bool copy) {
  auto a = asMixed(ad);
  if (UNLIKELY(a->m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return member_lval { a, lvalBlackHole().asTypedValue() };
  }

  a = a->prepareForInsert(copy);

  if (UNLIKELY(!a->nextInsert(make_tv<KindOfNull>()))) {
    return member_lval { a, lvalBlackHole().asTypedValue() };
  }

  return member_lval { a, &a->data()[a->m_used - 1].data };
}

member_lval MixedArray::LvalNewRef(ArrayData* ad, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefNew();
  return LvalNew(ad, copy);
}

ArrayData* MixedArray::SetInt(ArrayData* ad, int64_t k, Cell v, bool copy) {
  return asMixed(ad)->prepareForInsert(copy)->update(k, v);
}

ArrayData*
MixedArray::SetStr(ArrayData* ad, StringData* k, Cell v, bool copy) {
  return asMixed(ad)->prepareForInsert(copy)->update(k, v);
}

ArrayData* MixedArray::SetWithRefInt(ArrayData* ad, int64_t k,
                                     TypedValue v, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices && tvIsReferenced(v)) {
    raiseHackArrCompatRefBind(k);
  }
  return asMixed(ad)->prepareForInsert(copy)->updateWithRef(k, v);
}

ArrayData* MixedArray::SetWithRefStr(ArrayData* ad, StringData* k,
                                     TypedValue v, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices && tvIsReferenced(v)) {
    raiseHackArrCompatRefBind(k);
  }
  return asMixed(ad)->prepareForInsert(copy)->updateWithRef(k, v);
}

ArrayData*
MixedArray::SetRefInt(ArrayData* ad, int64_t k, Variant& v, bool copy) {
  auto a = asMixed(ad);
  assert(a->isMixed());
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);
  return a->prepareForInsert(copy)->updateRef(k, v);
}

ArrayData*
MixedArray::SetRefStr(ArrayData* ad, StringData* k, Variant& v, bool copy) {
  auto a = asMixed(ad);
  assert(a->isMixed());
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);
  return a->prepareForInsert(copy)->updateRef(k, v);
}

ArrayData*
MixedArray::AddInt(ArrayData* ad, int64_t k, Cell v, bool copy) {
  assert(!ad->exists(k));
  return asMixed(ad)->prepareForInsert(copy)->addVal(k, v);
}

ArrayData*
MixedArray::AddStr(ArrayData* ad, StringData* k, Cell v, bool copy) {
  assert(!ad->exists(k));
  return asMixed(ad)->prepareForInsert(copy)->addVal(k, v);
}

ArrayData*
MixedArray::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  return asMixed(ad)->prepareForInsert(false)->zSetImpl(k, v);
}

ArrayData*
MixedArray::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  return asMixed(ad)->prepareForInsert(false)->zSetImpl(k, v);
}

ArrayData*
MixedArray::ZAppend(ArrayData* ad, RefData* v, int64_t* key_ptr) {
  return asMixed(ad)->prepareForInsert(false)->zAppendImpl(v, key_ptr);
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
  auto const oldTV = *tv;
  tv->m_type = kInvalidDataType;
  --m_size;
  // Mark the hash entry as "deleted".
  assert(m_used <= capacity());

  // Finally, decref the old value
  tvDecRefGen(oldTV);
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

ArrayData* MixedArray::Append(ArrayData* ad, Cell v, bool copy) {
  auto a = asMixed(ad);
  if (UNLIKELY(a->m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return a;
  }
  a = a->prepareForInsert(copy);
  a->nextInsert(v);
  return a;
}

ArrayData* MixedArray::AppendRef(ArrayData* ad, Variant& v, bool copy) {
  auto a = asMixed(ad);
  assert(a->isMixed());

  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefNew();

  if (UNLIKELY(a->m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return a;
  }

  // TODO: maybe inline nextInsertRef manually (this is the only caller).
  return a->prepareForInsert(copy)->nextInsertRef(v);
}

ArrayData* MixedArray::AppendWithRef(ArrayData* ad, TypedValue v, bool copy) {
  auto a = asMixed(ad);
  assert(a->isMixed());

  if (RuntimeOption::EvalHackArrCompatNotices && tvIsReferenced(v)) {
    raiseHackArrCompatRefNew();
  }

  return a->prepareForInsert(copy)->nextInsertWithRef(v);
}

/*
 * Copy an array to a new array of mixed kind, with a particular
 * pre-reserved size.
 */
NEVER_INLINE
MixedArray* MixedArray::CopyReserve(const MixedArray* src,
                                    size_t expectedSize) {
  auto const scale = computeScaleFromSize(expectedSize);
  auto const ad    = reqAlloc(scale);
  auto const oldUsed = src->m_used;

  ad->m_sizeAndPos      = src->m_sizeAndPos;
  ad->initHeader(*src, 1);
  ad->m_scale           = scale; // don't set m_used yet
  ad->m_nextKI          = src->m_nextKI;

  auto const table = ad->initHash(scale);

  auto dstElm = ad->data();
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
    tvDupWithRef(srcElm->data, dstElm->data, src);
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
  assert(i == dstElm - ad->data());
  ad->m_used = i;

  assert(ad->kind() == src->kind());
  assert(ad->m_size == src->m_size);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_used <= oldUsed);
  assert(ad->m_used == dstElm - ad->data());
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
    auto const value = it.secondVal();

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
      auto const ei = ret->findForInsertUpdate(srcElem->skey, hash);
      if (isValidPos(ei)) continue;
      auto e = ret->allocElm(ei);
      e->setStrKey(srcElem->skey, hash);
      ret->initWithRef(e->data, tvAsCVarRef(&srcElem->data));
    } else {
      auto const ei = ret->findForInsertUpdate(srcElem->ikey, hash);
      if (isValidPos(ei)) continue;
      auto e = ret->allocElm(ei);
      e->setIntKey(srcElem->ikey, hash);
      ret->initWithRef(e->data, tvAsCVarRef(&srcElem->data));
    }
  }

  return ret;
}

NEVER_INLINE
ArrayData* MixedArray::ArrayMergeGeneric(MixedArray* ret,
                                         const ArrayData* elems) {
  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    auto const value = it.secondVal();
    if (key.asTypedValue()->m_type == KindOfInt64) {
      ret->nextInsertWithRef(value);
    } else {
      StringData* sd = key.getStringData();
      auto const lval = ret->addLvalImpl<false>(sd);
      tvAsVariant(lval.tv()).setWithRef(value);
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
        auto const lval = ret->addLvalImpl<false>(srcElem->skey);
        tvAsVariant(lval.tv()).setWithRef(tvAsCVarRef(&srcElem->data));
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

ArrayData* MixedArray::Prepend(ArrayData* adInput, Cell v, bool /*copy*/) {
  // TODO: why is this ignoring the copy param and calling cowCheck?
  auto a = asMixed(adInput)->prepareForInsert(adInput->cowCheck());

  auto elms = a->data();
  if (a->m_used > 0 && !isTombstone(elms[0].data.m_type)) {
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

member_rval::ptr_u MixedArray::NvTryGetIntDict(const ArrayData* ad, int64_t k) {
  assert(asMixed(ad)->checkInvariants());
  assert(ad->isDict());
  auto const ptr = MixedArray::NvGetInt(ad, k);
  if (UNLIKELY(!ptr)) throwOOBArrayKeyException(k, ad);
  return ptr;
}

member_rval::ptr_u MixedArray::NvTryGetStrDict(const ArrayData* ad,
                                               const StringData* k) {
  assert(asMixed(ad)->checkInvariants());
  assert(ad->isDict());
  auto const ptr = MixedArray::NvGetStr(ad, k);
  if (UNLIKELY(!ptr)) throwOOBArrayKeyException(k, ad);
  return ptr;
}

ArrayData* MixedArray::SetWithRefIntDict(ArrayData* ad, int64_t k,
                                         TypedValue v, bool copy) {
  if (tvIsReferenced(v)) throwRefInvalidArrayValueException(ad);
  return asMixed(ad)->prepareForInsert(copy)->updateWithRef(k, v);
}

ArrayData* MixedArray::SetWithRefStrDict(ArrayData* ad, StringData* k,
                                         TypedValue v, bool copy) {
  if (tvIsReferenced(v)) throwRefInvalidArrayValueException(ad);
  return asMixed(ad)->prepareForInsert(copy)->updateWithRef(k, v);
}

member_lval MixedArray::LvalIntRefDict(ArrayData* adIn, int64_t, bool) {
  assert(asMixed(adIn)->checkInvariants());
  assert(adIn->isDict());
  throwRefInvalidArrayValueException(adIn);
}

member_lval MixedArray::LvalStrRefDict(ArrayData* adIn, StringData*, bool) {
  assert(asMixed(adIn)->checkInvariants());
  assert(adIn->isDict());
  throwRefInvalidArrayValueException(adIn);
}

member_lval MixedArray::LvalNewRefDict(ArrayData* adIn, bool) {
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
MixedArray::AppendWithRefDict(ArrayData* adIn, TypedValue v, bool copy) {
  assert(asMixed(adIn)->checkInvariants());
  assert(adIn->isDict());
  if (tvIsReferenced(v)) throwRefInvalidArrayValueException(adIn);
  return Append(adIn, tvToInitCell(v), copy);
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
bool MixedArray::DictEqualHelper(const ArrayData* ad1, const ArrayData* ad2,
                                 bool strict) {
  assert(asMixed(ad1)->checkInvariants());
  assert(asMixed(ad2)->checkInvariants());
  assert(ad1->isDict());
  assert(ad2->isDict());

  if (ad1 == ad2) return true;
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
      auto const other_rval = [&] {
        if (elm->hasIntKey()) {
          return RvalIntDict(ad2, elm->ikey);
        } else {
          assertx(elm->hasStrKey());
          return RvalStrDict(ad2, elm->skey);
        }
      }();
      if (!other_rval ||
          !cellEqual(tvAssertCell(elm->data),
                     tvAssertCell(other_rval.tv()))) {
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
    auto copied = a->prepareForInsert(copy);
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
