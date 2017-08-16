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
#include "hphp/runtime/base/packed-array.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include <folly/Likely.h>

#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-helpers.h"
#include "hphp/runtime/base/member-val.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/tv-variant.h"

#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/array-iterator-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyVecArray;

struct PackedArray::VecInitializer {
  VecInitializer() {
    auto const ad = reinterpret_cast<ArrayData*>(&s_theEmptyVecArray);
    ad->m_sizeAndPos = 0;
    ad->initHeader(HeaderKind::VecArray, StaticValue);
  }
};
PackedArray::VecInitializer PackedArray::s_initializer;

//////////////////////////////////////////////////////////////////////

namespace {

inline ArrayData* alloc_packed_static(size_t cap) {
  auto size = sizeof(ArrayData) + cap * sizeof(TypedValue);
  auto ret = RuntimeOption::EvalLowStaticArrays ?
    low_malloc_data(size) : malloc(size);
  return static_cast<ArrayData*>(ret);
}

inline size_t packedArrayCapacityToSizeIndex(size_t cap) {
  if (cap <= PackedArray::SmallSize) {
    return PackedArray::SmallSizeIndex;
  }
  auto const sizeIndex = MemoryManager::size2Index(
    sizeof(ArrayData) + cap * sizeof(TypedValue)
  );
  assert(sizeIndex <= PackedArray::MaxSizeIndex);
  return sizeIndex;
}

}

bool PackedArray::checkInvariants(const ArrayData* arr) {
  assert(arr->hasPackedLayout());
  assert(arr->checkCount());
  assert(arr->m_size <= MixedArray::MaxSize);
  assert(arr->m_size <= capacity(arr));
  assert(arr->m_pos >= 0 && arr->m_pos <= arr->m_size);
  static_assert(ArrayData::kPackedKind == 0, "");
  // Note that m_pos < m_size is not an invariant, because an array
  // that grows will only adjust m_size to zero on the old array.

  // This loop is too slow for normal use, but can be enabled to debug
  // packed arrays.
  if (false) {
    auto ptr = packedData(arr);
    auto const stop = ptr + arr->m_size;
    for (; ptr != stop; ptr++) {
      assert(ptr->m_type != KindOfUninit);
      assert(tvIsPlausible(*ptr));
      assert(!arr->isVecArray() || ptr->m_type != KindOfRef);
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
MixedArray* PackedArray::ToMixedHeader(const ArrayData* old,
                                       size_t neededSize) {
  assert(PackedArray::checkInvariants(old));

  auto const oldSize = old->m_size;
  auto const scale   = MixedArray::computeScaleFromSize(neededSize);
  auto const ad      = MixedArray::reqAlloc(scale);
  ad->m_sizeAndPos   = oldSize | int64_t{old->m_pos} << 32;
  ad->initHeader(HeaderKind::Mixed, 1);
  ad->m_scale_used   = scale | uint64_t{oldSize} << 32; // used=oldSize
  ad->m_nextKI       = oldSize;

  assert(ad->m_size == oldSize);
  assert(ad->m_pos == old->m_pos);
  assert(ad->kind() == ArrayData::kMixedKind);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_used == oldSize);
  assert(ad->m_scale == scale);
  assert(ad->m_nextKI == oldSize);
  // Can't checkInvariants yet, since we haven't populated the payload.
  return ad;
}

/*
 * Converts a packed array to mixed, leaving the packed array in an
 * empty state.  You need ToMixedCopy in cases where the old array
 * needs to remain un-modified (usually if `copy' is true).
 *
 * The returned array is mixed, and is guaranteed not to be isFull().
 * (Note: only unset can call ToMixed when we aren't about to insert.)
 */
MixedArray* PackedArray::ToMixed(ArrayData* old) {
  auto const oldSize = old->m_size;
  auto const ad      = ToMixedHeader(old, oldSize + 1);
  auto const mask    = ad->mask();
  auto dstData       = ad->data();
  auto const srcData = packedData(old);

  auto const dstHash = ad->initHash(ad->scale());
  for (uint32_t i = 0; i < oldSize; ++i) {
    auto h = hash_int64(i);
    *ad->findForNewInsert(dstHash, mask, h) = i;
    dstData->setIntKey(i, h);
    tvCopy(srcData[i], dstData->data);
    ++dstData;
  }
  old->m_sizeAndPos = 0;

  // PHP does not have the concept of packed VS mixed, so packed to mixed
  // promotion needs to be invisible to strong iteration in order to match
  // PHP behavior; intentionally not doing the same in ToMixedCopy{,Reserve}
  // because copies _are_ supposed to be visible to strong iteration
  if (UNLIKELY(strong_iterators_exist())) move_strong_iterators(ad, old);

  assert(ad->checkInvariants());
  assert(!ad->isFull());
  assert(ad->hasExactlyOneRef());
  return ad;
}

/*
 * Convert a packed array to mixed, without moving the elements out of
 * the old packed array.  This effectively performs a Copy at the same
 * time as converting to mixed.  The returned mixed array is
 * guaranteed not to be full.
 */
MixedArray* PackedArray::ToMixedCopy(const ArrayData* old) {
  assert(PackedArray::checkInvariants(old));

  auto const oldSize = old->m_size;
  auto const ad      = ToMixedHeader(old, oldSize + 1);
  auto const mask    = ad->mask();
  auto dstData       = ad->data();
  auto const srcData = packedData(old);

  auto const dstHash = ad->initHash(ad->scale());
  for (uint32_t i = 0; i < oldSize; ++i) {
    auto h = hash_int64(i);
    *ad->findForNewInsert(dstHash, mask, h) = i;
    dstData->setIntKey(i, h);
    tvDupWithRef(srcData[i], dstData->data, old);
    ++dstData;
  }

  assert(ad->checkInvariants());
  assert(!ad->isFull());
  assert(ad->hasExactlyOneRef());
  return ad;
}

/*
 * Convert to mixed, reserving space for at least `neededSize' elems.
 * The `neededSize' should include old->size(), but may be equal to
 * it.
 */
MixedArray* PackedArray::ToMixedCopyReserve(const ArrayData* old,
                                           size_t neededSize) {
  assert(neededSize >= old->m_size);
  auto const ad      = ToMixedHeader(old, neededSize);
  auto const oldSize = old->m_size;
  auto const mask    = ad->mask();
  auto dstData       = ad->data();
  auto const srcData = packedData(old);

  auto const dstHash = ad->initHash(ad->scale());
  for (uint32_t i = 0; i < oldSize; ++i) {
    auto h = hash_int64(i);
    *ad->findForNewInsert(dstHash, mask, h) = i;
    dstData->setIntKey(i, h);
    tvDupWithRef(srcData[i], dstData->data, old);
    ++dstData;
  }

  assert(ad->checkInvariants());
  assert(ad->hasExactlyOneRef());
  return ad;
}

NEVER_INLINE
ArrayData* PackedArray::Grow(ArrayData* adIn, bool copy) {
  assert(checkInvariants(adIn));
  assert(adIn->m_size == capacity(adIn));

  auto const sizeIndex = adIn->m_aux16 + kSizeClassesPerDoubling;
  if (UNLIKELY(sizeIndex > MaxSizeIndex)) return nullptr;
  auto ad = static_cast<ArrayData*>(MM().objMallocIndex(sizeIndex));

  if (copy) {
    // CopyPackedHelper will copy the header and m_sizeAndPos; since we pass
    // convertingPackedToVec = false, it can't fail. All we have to do
    // afterwards is fix the capacity and refcount on the copy; it's easiest
    // to do that by reinitializing the whole header.
    auto const DEBUG_ONLY ok = CopyPackedHelper<false>(adIn, ad);
    assert(ok);
    ad->initHeader(uint16_t(sizeIndex), adIn->m_kind, 1);

    assert(ad->m_size == adIn->m_size);
    assert(ad->m_pos == adIn->m_pos);
  } else {
    // Copy everything from `adIn' to `ad', including header and m_sizeAndPos
    static_assert(sizeof(ArrayData) == 16 && sizeof(TypedValue) == 16, "");
    memcpy16_inline(ad, adIn, (adIn->m_size + 1) * sizeof(TypedValue));
    ad->initHeader(uint16_t(sizeIndex), adIn->m_kind, 1);

    assert(ad->m_size == adIn->m_size);
    assert(ad->m_pos == adIn->m_pos);
    adIn->m_sizeAndPos = 0; // old is a zombie now

    if (UNLIKELY(strong_iterators_exist())) move_strong_iterators(ad, adIn);
  }

  assert(ad->kind() == adIn->kind());
  assert(capacity(ad) > capacity(adIn));
  assert(ad->hasExactlyOneRef());
  assert(checkInvariants(ad));
  return ad;
}

ALWAYS_INLINE
ArrayData* PackedArray::PrepareForInsert(ArrayData* adIn, bool copy) {
  assert(checkInvariants(adIn));
  if (adIn->m_size == capacity(adIn)) return Grow(adIn, copy);
  if (copy) return Copy(adIn);
  return adIn;
}

//////////////////////////////////////////////////////////////////////

/* This helper copies everything from adIn to ad, including the header
 * (capacity, kind, and refcount) and m_sizeAndPos. It then increfs the
 * contents, if needed.
 *
 * If convertingPackedToVec is false, it will always succeed (return true).
 *
 * If convertingPackedToVec is true and adIn contains a Ref, then it will
 * return false. Refcounts of the contents will be left in a consistent state.
 * It is the callers responsibility to free ad and throw an appropriate
 * exception in this case.
 */
template<bool convertingPackedToVec>
ALWAYS_INLINE
bool PackedArray::CopyPackedHelper(const ArrayData* adIn, ArrayData* ad) {
  // Copy everything from `adIn' to `ad', including refcount, kind and cap
  auto const size = adIn->m_size;
  static_assert(sizeof(ArrayData) == 16 && sizeof(TypedValue) == 16, "");
  memcpy16_inline(ad, adIn, (size + 1) * 16);

  // Copy counted types correctly, especially RefData.
  for (auto elm = packedData(ad), end = elm + size; elm < end; ++elm) {
    if (UNLIKELY(elm->m_type == KindOfRef)) {
      assert(!adIn->isVecArray());
      auto ref = elm->m_data.pref;
      // See also tvDupWithRef()
      if (!ref->isReferenced() && ref->tv()->m_data.parr != adIn) {
        cellDup(*ref->tv(), *elm);
        continue;
      } else if (convertingPackedToVec) {
        for (--elm; elm >= packedData(ad); --elm) {
          tvDecRefGen(elm);
        }
        return false;
      }
    }
    tvIncRefGen(elm);
  }
  return true;
}

NEVER_INLINE
ArrayData* PackedArray::Copy(const ArrayData* adIn) {
  assert(checkInvariants(adIn));

  auto ad = static_cast<ArrayData*>(MM().objMallocIndex(adIn->m_aux16));

  // CopyPackedHelper will copy the header (including capacity and kind), and
  // m_sizeAndPos; since we pass convertingPackedToVec = false, it can't fail.
  // All we have to do afterwards is fix the refcount on the copy.
  auto const DEBUG_ONLY ok = CopyPackedHelper<false>(adIn, ad);
  assert(ok);
  ad->m_count = 1;

  assert(ad->kind() == adIn->kind());
  assert(capacity(ad) == capacity(adIn));
  assert(ad->m_size == adIn->m_size);
  assert(ad->m_pos == adIn->m_pos);
  assert(ad->hasExactlyOneRef());
  assert(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::CopyStatic(const ArrayData* adIn) {
  assert(checkInvariants(adIn));

  auto const sizeIndex = packedArrayCapacityToSizeIndex(adIn->m_size);
  auto ad = alloc_packed_static(adIn->m_size);
  // CopyPackedHelper will copy the header and m_sizeAndPos; since we pass
  // convertingPackedToVec = false, it can't fail. All we have to do afterwards
  // is fix the capacity and refcount on the copy; it's easiest to do that by
  // reinitializing the whole header.
  auto const DEBUG_ONLY ok = CopyPackedHelper<false>(adIn, ad);
  assert(ok);
  ad->initHeader(uint16_t(sizeIndex), adIn->m_kind, StaticValue);

  assert(ad->kind() == adIn->kind());
  assert(capacity(ad) >= adIn->m_size);
  assert(ad->m_size == adIn->m_size);
  assert(ad->m_pos == adIn->m_pos);
  assert(ad->isStatic());
  assert(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ConvertStatic(const ArrayData* arr) {
  assert(arr->isVectorData());

  auto const sizeIndex = packedArrayCapacityToSizeIndex(arr->m_size);
  auto ad = alloc_packed_static(arr->m_size);
  ad->initHeader(uint16_t(sizeIndex), HeaderKind::Packed, StaticValue);
  ad->m_sizeAndPos = arr->m_sizeAndPos;

  auto data = packedData(ad);
  auto pos_limit = arr->iter_end();
  for (auto pos = arr->iter_begin(); pos != pos_limit;
       pos = arr->iter_advance(pos), ++data) {
    tvDupWithRef(arr->atPos(pos), *data, arr);
  }

  assert(ad->isPacked());
  assert(capacity(ad) >= arr->m_size);
  assert(ad->m_size == arr->m_size);
  assert(ad->m_pos == arr->m_pos);
  assert(ad->isStatic());
  assert(checkInvariants(ad));
  return ad;
}

/* This helper allocates an ArrayData and initializes the header (including
 * capacity, kind, and refcount). The caller is responsible for initializing
 * m_sizeAndPos, and initializing array entries (if any).
 */
ALWAYS_INLINE
ArrayData* PackedArray::MakeReserveImpl(uint32_t cap, HeaderKind hk) {
  auto const sizeIndex = packedArrayCapacityToSizeIndex(cap);
  auto ad = static_cast<ArrayData*>(MM().objMallocIndex(sizeIndex));
  ad->initHeader(uint16_t(sizeIndex), hk, 1);

  assert(ad->m_kind == hk);
  assert(capacity(ad) >= cap);
  assert(ad->hasExactlyOneRef());
  return ad;
}

ArrayData* PackedArray::MakeReserve(uint32_t capacity) {
  auto ad = MakeReserveImpl(capacity, HeaderKind::Packed);
  ad->m_sizeAndPos = 0;
  assert(ad->isPacked());
  assert(ad->m_size == 0);
  assert(ad->m_pos == 0);
  assert(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakeReserveVec(uint32_t capacity) {
  auto ad = MakeReserveImpl(capacity, HeaderKind::VecArray);
  ad->m_sizeAndPos = 0;
  assert(ad->isVecArray());
  assert(ad->m_size == 0);
  assert(ad->m_pos == 0);
  assert(checkInvariants(ad));
  return ad;
}

template<bool reverse>
ALWAYS_INLINE
ArrayData* PackedArray::MakePackedImpl(uint32_t size,
                                       const TypedValue* values,
                                       HeaderKind hk) {
  assert(size > 0);
  auto ad = MakeReserveImpl(size, hk);
  ad->m_sizeAndPos = size; // pos = 0

  // Append values by moving; this function takes ownership of them.
  if (reverse) {
    auto elm = packedData(ad) + size - 1;
    for (auto end = values + size; values < end; ++values, --elm) {
      assert(hk != HeaderKind::VecArray || values->m_type != KindOfRef);
      tvCopy(*values, *elm);
    }
  } else {
    memcpy16_inline(packedData(ad), values, sizeof(TypedValue) * size);
  }

  assert(ad->m_size == size);
  assert(ad->m_pos == 0);
  assert(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakePacked(uint32_t size, const TypedValue* values) {
  // Values are in reverse order since they come from the stack, which
  // grows down.
  auto ad = MakePackedImpl<true>(size, values, HeaderKind::Packed);
  assert(ad->isPacked());
  return ad;
}

ArrayData* PackedArray::MakeVec(uint32_t size, const TypedValue* values) {
  // Values are in reverse order since they come from the stack, which
  // grows down.
  auto ad = MakePackedImpl<true>(size, values, HeaderKind::VecArray);
  assert(ad->isVecArray());
  return ad;
}

ArrayData* PackedArray::MakePackedNatural(uint32_t size,
                                          const TypedValue* values) {
  auto ad = MakePackedImpl<false>(size, values, HeaderKind::Packed);
  assert(ad->isPacked());
  return ad;
}

ArrayData* PackedArray::MakeUninitialized(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::Packed);
  ad->m_sizeAndPos = size; // pos = 0
  assert(ad->isPacked());
  assert(ad->m_size == size);
  assert(ad->m_pos == 0);
  assert(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakeUninitializedVec(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::VecArray);
  ad->m_sizeAndPos = size; // pos = 0
  assert(ad->isVecArray());
  assert(ad->m_size == size);
  assert(ad->m_pos == 0);
  assert(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakeVecFromAPC(const APCArray* apc) {
  assert(apc->isVec());
  auto const apcSize = apc->size();
  VecArrayInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.append(apc->getValue(i)->toLocal());
  }
  return init.create();
}

void PackedArray::Release(ArrayData* ad) {
  assert(checkInvariants(ad));
  assert(ad->isRefCounted());
  assert(ad->hasExactlyOneRef());

  for (auto elm = packedData(ad), end = elm + ad->m_size; elm < end; ++elm) {
    tvDecRefGen(elm);
  }
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(ad);
  }
  MM().objFreeIndex(ad, ad->m_aux16);
}

NEVER_INLINE
void PackedArray::ReleaseUncounted(ArrayData* ad, size_t extra) {
  assert(checkInvariants(ad));
  assert(ad->isUncounted());

  auto const data = packedData(ad);
  auto const stop = data + ad->m_size;
  for (auto ptr = data; ptr != stop; ++ptr) {
    ReleaseUncountedTv(*ptr);
  }

  // We better not have strong iterators associated with uncounted arrays.
  assert(!has_strong_iterator(ad));
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().removeAPCUncountedBlock();
  }

  free_huge(reinterpret_cast<char*>(ad) - extra);
}

////////////////////////////////////////////////////////////////////////////////

member_rval::ptr_u PackedArray::NvGetInt(const ArrayData* ad, int64_t ki) {
  assert(checkInvariants(ad));
  auto const data = packedData(ad);
  return LIKELY(size_t(ki) < ad->m_size) ? &data[ki] : nullptr;
}

member_rval::ptr_u
PackedArray::NvGetStr(const ArrayData* ad, const StringData* /*s*/) {
  assert(checkInvariants(ad));
  return nullptr;
}

member_rval::ptr_u PackedArray::NvTryGetIntVec(const ArrayData* ad, int64_t k) {
  assert(checkInvariants(ad));
  assert(ad->isVecArray());
  auto const data = packedData(ad);
  if (LIKELY(size_t(k) < ad->m_size)) return &data[k];
  throwOOBArrayKeyException(k, ad);
}

member_rval::ptr_u PackedArray::NvTryGetStrVec(const ArrayData* ad,
                                               const StringData* s) {
  assert(checkInvariants(ad));
  assert(ad->isVecArray());
  throwInvalidArrayKeyException(s, ad);
}

Cell PackedArray::NvGetKey(const ArrayData* ad, ssize_t pos) {
  assert(checkInvariants(ad));
  assert(pos != ad->m_size);
  return make_tv<KindOfInt64>(pos);
}

size_t PackedArray::Vsize(const ArrayData*) {
  // PackedArray always has a valid m_size so it's an error to get here.
  always_assert(false);
}

member_rval::ptr_u PackedArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  assert(checkInvariants(ad));
  assert(pos != ad->m_size);
  return &packedData(ad)[pos];
}

bool PackedArray::ExistsInt(const ArrayData* ad, int64_t k) {
  assert(checkInvariants(ad));
  return size_t(k) < ad->m_size;
}

bool PackedArray::ExistsStr(const ArrayData* ad, const StringData* /*s*/) {
  assert(checkInvariants(ad));
  return false;
}

///////////////////////////////////////////////////////////////////////////////

namespace {

template<typename FoundFn, typename AppendFn, typename PromotedFn>
auto MutableOpInt(ArrayData* adIn, int64_t k, bool copy,
                  FoundFn found, AppendFn append, PromotedFn promoted) {
  assert(PackedArray::checkInvariants(adIn));
  assert(adIn->isPacked());

  if (LIKELY(size_t(k) < adIn->getSize())) {
    auto const ad = copy ? PackedArray::Copy(adIn) : adIn;
    return found(ad);
  }

  if (size_t(k) == adIn->getSize()) return append();

  auto const mixed = copy ? PackedArray::ToMixedCopy(adIn)
                          : PackedArray::ToMixed(adIn);
  return promoted(mixed);
}

template<typename PromotedFn>
auto MutableOpStr(ArrayData* adIn, StringData* k, bool copy,
                  PromotedFn promoted) {
  assert(PackedArray::checkInvariants(adIn));
  assert(adIn->isPacked());

  auto const mixed = copy ? PackedArray::ToMixedCopy(adIn)
                          : PackedArray::ToMixed(adIn);
  return promoted(mixed);
}

template<typename FoundFn>
auto MutableOpIntVec(ArrayData* adIn, int64_t k, bool copy, FoundFn found) {
  assert(PackedArray::checkInvariants(adIn));
  assert(adIn->isVecArray());

  if (UNLIKELY(size_t(k) >= adIn->getSize())) {
    throwOOBArrayKeyException(k, adIn);
  }
  auto const ad = copy ? PackedArray::Copy(adIn) : adIn;
  return found(ad);
}

}

member_lval PackedArray::LvalInt(ArrayData* adIn, int64_t k, bool copy) {
  return MutableOpInt(adIn, k, copy,
    [&] (ArrayData* ad) { return member_lval { ad, &packedData(ad)[k] }; },
    [&] { return LvalNew(adIn, copy); },
    // TODO(#2606310): Make use of our knowledge that the key is missing.
    [&] (MixedArray* mixed) { return mixed->addLvalImpl<true>(k); }
  );
}

member_lval PackedArray::LvalIntRef(ArrayData* adIn, int64_t k, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);
  return LvalInt(adIn, k, copy);
}

member_lval PackedArray::LvalIntVec(ArrayData* adIn, int64_t k, bool copy) {
  return MutableOpIntVec(adIn, k, copy,
    [&] (ArrayData* ad) { return member_lval { ad, &packedData(ad)[k] }; }
  );
}

member_lval PackedArray::LvalSilentInt(ArrayData* adIn, int64_t k, bool copy) {
  assert(checkInvariants(adIn));
  if (UNLIKELY(size_t(k) >= adIn->m_size)) return {adIn, nullptr};
  auto const ad = copy ? Copy(adIn) : adIn;
  return member_lval { ad, &packedData(ad)[k] };
}

member_lval PackedArray::LvalStr(ArrayData* adIn, StringData* k, bool copy) {
  return MutableOpStr(adIn, k, copy,
    // TODO(#2606310): Make use of our knowledge that the key is missing.
    [&] (MixedArray* mixed) { return mixed->addLvalImpl<true>(k); }
  );
}

member_lval
PackedArray::LvalStrRef(ArrayData* adIn, StringData* key, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(key);
  return LvalStr(adIn, key, copy);
}

member_lval
PackedArray::LvalStrVec(ArrayData* adIn, StringData* key, bool) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  throwInvalidArrayKeyException(key, adIn);
}

member_lval PackedArray::LvalIntRefVec(ArrayData* adIn, int64_t /*k*/, bool) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  throwRefInvalidArrayValueException(adIn);
}

member_lval
PackedArray::LvalStrRefVec(ArrayData* adIn, StringData* key, bool) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  throwInvalidArrayKeyException(key, adIn);
}

member_lval PackedArray::LvalNew(ArrayData* adIn, bool copy) {
  assert(checkInvariants(adIn));
  auto const ad = PrepareForInsert(adIn, copy);
  auto& tv = packedData(ad)[ad->m_size++];
  tv.m_type = KindOfNull;
  return member_lval { ad, &tv };
}

member_lval PackedArray::LvalNewRef(ArrayData* adIn, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefNew();
  return LvalNew(adIn, copy);
}

member_lval PackedArray::LvalNewRefVec(ArrayData* adIn, bool) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  throwRefInvalidArrayValueException(adIn);
}

ArrayData* PackedArray::SetInt(ArrayData* adIn, int64_t k, Cell v, bool copy) {
  return MutableOpInt(adIn, k, copy,
    [&] (ArrayData* ad) { setElem(packedData(ad)[k], v); return ad; },
    [&] { return Append(adIn, v, copy); },
    [&] (MixedArray* mixed) { return mixed->addVal(k, v); }
  );
}

ArrayData*
PackedArray::SetIntVec(ArrayData* adIn, int64_t k, Cell v, bool copy) {
  return MutableOpIntVec(adIn, k, copy,
    [&] (ArrayData* ad) { setElemNoRef(packedData(ad)[k], v); return ad; }
  );
}

ArrayData* PackedArray::SetStr(ArrayData* adIn, StringData* k, Cell v,
                               bool copy) {
  return MutableOpStr(adIn, k, copy,
    [&] (MixedArray* mixed) { return mixed->addVal(k, v); }
  );
}

ArrayData* PackedArray::SetStrVec(ArrayData* adIn, StringData* k, Cell, bool) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  throwInvalidArrayKeyException(k, adIn);
}

ArrayData* PackedArray::SetWithRefInt(ArrayData* adIn, int64_t k,
                                      TypedValue v, bool copy) {
  auto const checkHackArrRef = [&] {
    if (RuntimeOption::EvalHackArrCompatNotices && tvIsReferenced(v)) {
      raiseHackArrCompatRefBind(k);
    }
  };

  return MutableOpInt(adIn, k, copy,
    [&] (ArrayData* ad) {
      checkHackArrRef();
      setElemWithRef(packedData(ad)[k], v);
      return ad;
    },
    [&] { return AppendWithRef(adIn, v, copy); },
    [&] (MixedArray* mixed) {
      checkHackArrRef();
      auto const lval = mixed->addLvalImpl<false>(k);
      tvSetWithRef(v, *lval.tv());
      return lval.arr_base();
    }
  );
}

ArrayData* PackedArray::SetWithRefIntVec(ArrayData* adIn, int64_t k,
                                         TypedValue v, bool copy) {
  if (tvIsReferenced(v)) throwRefInvalidArrayValueException(adIn);

  return MutableOpIntVec(adIn, k, copy,
    [&] (ArrayData* ad) { setElemNoRef(packedData(ad)[k], v); return ad; }
  );
}

ArrayData* PackedArray::SetWithRefStr(ArrayData* adIn, StringData* k,
                                      TypedValue v, bool copy) {
  return MutableOpStr(adIn, k, copy,
    [&] (MixedArray* mixed) {
      if (RuntimeOption::EvalHackArrCompatNotices && tvIsReferenced(v)) {
        raiseHackArrCompatRefBind(k);
      }
      auto const lval = mixed->addLvalImpl<false>(k);
      tvSetWithRef(v, *lval.tv());
      return lval.arr_base();
    }
  );
}

ArrayData* PackedArray::SetWithRefStrVec(ArrayData* adIn, StringData* k,
                                         TypedValue, bool) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  throwInvalidArrayKeyException(k, adIn);
}

ArrayData* PackedArray::SetRefInt(ArrayData* adIn, int64_t k, Variant& v,
                                  bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);

  return MutableOpInt(adIn, k, copy,
    [&] (ArrayData* ad) { tvBind(v.asRef(), &packedData(ad)[k]); return ad; },
    [&] { return AppendRef(adIn, v, copy); },
    // TODO(#2606310): Make use of our knowledge that the key is missing.
    [&] (MixedArray* mixed) { return mixed->updateRef(k, v); }
  );
}

ArrayData* PackedArray::SetRefIntVec(ArrayData* adIn, int64_t /*k*/,
                                     Variant& /*v*/, bool /*copy*/) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  throwRefInvalidArrayValueException(adIn);
}

ArrayData* PackedArray::SetRefStr(ArrayData* adIn,
                                  StringData* k,
                                  Variant& v,
                                  bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);

  return MutableOpStr(adIn, k, copy,
    // TODO(#2606310): Make use of our knowledge that the key is missing.
    [&] (MixedArray* mixed) { return mixed->updateRef(k, v); }
  );
}

ArrayData*
PackedArray::SetRefStrVec(ArrayData* adIn, StringData* k, Variant&, bool) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  throwInvalidArrayKeyException(k, adIn);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

void adjustMArrayIterAfterPop(ArrayData* ad) {
  assert(ad->hasPackedLayout());
  auto const size = ad->getSize();
  if (size) {
    for_each_strong_iterator([&] (MIterTable::Ent& miEnt) {
      if (miEnt.array != ad) return;
      auto const iter = miEnt.iter;
      if (iter->getResetFlag()) return;
      if (iter->m_pos >= size) iter->m_pos = size - 1;
    });
  } else {
    reset_strong_iterators(ad);
  }
}

}

ArrayData* PackedArray::RemoveInt(ArrayData* adIn, int64_t k, bool copy) {
  assert(checkInvariants(adIn));
  assert(adIn->isPacked());
  if (size_t(k) < adIn->m_size) {
    // Escalate to mixed for correctness; unset preserves m_nextKI.
    //
    // TODO(#2606310): if we're removing the /last/ element, we
    // probably could stay packed, but this needs to be verified.
    auto const mixed = copy ? ToMixedCopy(adIn) : ToMixed(adIn);
    auto pos = mixed->findForRemove(k, hash_int64(k), false);
    if (validPos(pos)) mixed->erase(pos);
    return mixed;
  }
  // Key doesn't exist---we're still packed.
  return copy ? Copy(adIn) : adIn;
}

ArrayData*
PackedArray::RemoveIntVec(ArrayData* adIn, int64_t k, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());

  // You're only allowed to remove an element at the end of the vec (or beyond,
  // which is a no-op).
  if (UNLIKELY(size_t(k) >= adIn->m_size)) return adIn;
  if (LIKELY(size_t(k) + 1 == adIn->m_size)) {
    auto const ad = copy ? Copy(adIn) : adIn;
    auto const oldSize = ad->m_size;
    auto& tv = packedData(ad)[oldSize - 1];
    auto const oldTV = tv;
    ad->m_size = oldSize - 1;
    ad->m_pos = 0;
    tvDecRefGen(oldTV);
    return ad;
  }
  throwVecUnsetException();
}

ArrayData*
PackedArray::RemoveStr(ArrayData* adIn, const StringData*, bool) {
  assertx(checkInvariants(adIn));
  return adIn;
}

ssize_t PackedArray::IterBegin(const ArrayData* ad) {
  assert(checkInvariants(ad));
  return 0;
}

ssize_t PackedArray::IterLast(const ArrayData* ad) {
  assert(checkInvariants(ad));
  return ad->m_size ? ad->m_size - 1 : 0;
}

ssize_t PackedArray::IterEnd(const ArrayData* ad) {
  assert(checkInvariants(ad));
  return ad->m_size;
}

ssize_t PackedArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  assert(checkInvariants(ad));
  if (pos < ad->m_size) {
    ++pos;
  }
  return pos;
}

ssize_t PackedArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  assert(checkInvariants(ad));
  if (pos > 0) {
    return pos - 1;
  }
  return ad->m_size;
}

bool PackedArray::AdvanceMArrayIter(ArrayData* ad, MArrayIter& fp) {
  assert(checkInvariants(ad));
  if (fp.getResetFlag()) {
    fp.setResetFlag(false);
    fp.m_pos = 0;
  } else if (fp.m_pos == ad->m_size) {
    return false;
  } else {
    fp.m_pos = IterAdvance(ad, fp.m_pos);
  }
  if (fp.m_pos == ad->m_size) {
    return false;
  }
  // We set ad's internal cursor to point to the next element
  // to conform with PHP5 behavior
  ad->m_pos = IterAdvance(ad, fp.m_pos);
  return true;
}

ArrayData* PackedArray::Append(ArrayData* adIn, Cell v, bool copy) {
  assert(checkInvariants(adIn));
  assertx(v.m_type != KindOfUninit);
  auto const ad = PrepareForInsert(adIn, copy);
  cellDup(v, packedData(ad)[ad->m_size++]);
  return ad;
}

ArrayData* PackedArray::AppendRef(ArrayData* adIn,
                                  Variant& v,
                                  bool copy) {
  assert(checkInvariants(adIn));
  assert(adIn->isPacked());
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefNew();
  auto const ad = PrepareForInsert(adIn, copy);
  auto& dst = packedData(ad)[ad->m_size++];
  dst.m_data.pref = v.asRef()->m_data.pref;
  dst.m_type = KindOfRef;
  dst.m_data.pref->incRefCount();
  return ad;
}

ArrayData* PackedArray::AppendRefVec(ArrayData* adIn, Variant&, bool) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  throwRefInvalidArrayValueException(adIn);
}

ArrayData*
PackedArray::AppendWithRef(ArrayData* adIn, TypedValue v, bool copy) {
  assert(checkInvariants(adIn));
  assert(adIn->isPacked());

  if (RuntimeOption::EvalHackArrCompatNotices && tvIsReferenced(v)) {
    raiseHackArrCompatRefNew();
  }

  auto const ad = PrepareForInsert(adIn, copy);
  auto& dst = packedData(ad)[ad->m_size++];
  dst.m_type = KindOfNull;
  tvAsVariant(&dst).setWithRef(v);
  return ad;
}

ArrayData*
PackedArray::AppendWithRefVec(ArrayData* adIn, TypedValue v, bool copy) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  if (tvIsReferenced(v)) throwRefInvalidArrayValueException(adIn);
  return Append(adIn, tvToInitCell(v), copy);
}

ArrayData* PackedArray::PlusEq(ArrayData* adIn, const ArrayData* elems) {
  assert(checkInvariants(adIn));
  assert(adIn->isPacked());
  if (!elems->isPHPArray()) throwInvalidAdditionException(elems);
  auto const neededSize = adIn->size() + elems->size();
  auto const mixed = ToMixedCopyReserve(adIn, neededSize);
  try {
    auto const ret = MixedArray::PlusEq(mixed, elems);
    assert(ret == mixed);
    assert(mixed->hasExactlyOneRef());
    return ret;
  } catch (...) {
    MixedArray::Release(mixed);
    throw;
  }
}

ArrayData* PackedArray::PlusEqVec(ArrayData* adIn, const ArrayData* /*elems*/) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  throwInvalidAdditionException(adIn);
}

ArrayData* PackedArray::Merge(ArrayData* adIn, const ArrayData* elems) {
  assert(checkInvariants(adIn));
  auto const neededSize = adIn->m_size + elems->size();
  auto const ret = ToMixedCopyReserve(adIn, neededSize);
  return MixedArray::ArrayMergeGeneric(ret, elems);
}

ArrayData* PackedArray::Pop(ArrayData* adIn, Variant& value) {
  assert(checkInvariants(adIn));

  auto const ad = adIn->cowCheck() ? Copy(adIn) : adIn;

  if (UNLIKELY(ad->m_size == 0)) {
    assert(ad->m_pos == 0);
    value = uninit_null();
    return ad;
  }

  auto const oldSize = ad->m_size;
  auto& tv = packedData(ad)[oldSize - 1];
  value = tvAsCVarRef(&tv);
  auto const oldTV = tv;
  ad->m_size = oldSize - 1;
  ad->m_pos = 0;
  if (UNLIKELY(strong_iterators_exist())) adjustMArrayIterAfterPop(ad);
  tvDecRefGen(oldTV);
  return ad;
}

ArrayData* PackedArray::Dequeue(ArrayData* adIn, Variant& value) {
  assert(checkInvariants(adIn));

  auto const ad = adIn->cowCheck() ? Copy(adIn) : adIn;
  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is removed from the beginning of the array.
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(ad);
  }

  if (UNLIKELY(ad->m_size == 0)) {
    value = uninit_null();
    return ad;
  }

  // This is O(N), but so is Dequeue on a mixed array, because it
  // needs to renumber keys.  So it makes sense to stay packed.
  auto n = ad->m_size - 1;
  auto const data = packedData(ad);
  value = std::move(tvAsVariant(data)); // no incref+decref
  std::memmove(data, data + 1, n * sizeof *data);
  ad->m_size = n;
  ad->m_pos = 0;
  return ad;
}

ArrayData* PackedArray::Prepend(ArrayData* adIn, Cell v, bool /*copy*/) {
  assert(checkInvariants(adIn));

  auto const ad = PrepareForInsert(adIn, adIn->cowCheck());

  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is added to the beginning of the array.
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(ad);
  }

  auto const size = ad->m_size;
  auto const data = packedData(ad);
  std::memmove(data + 1, data, sizeof *data * size);
  cellDup(v, data[0]);
  ad->m_size = size + 1;
  ad->m_pos = 0;
  return ad;
}

ArrayData* PackedArray::ToPHPArray(ArrayData* ad, bool) {
  assert(checkInvariants(ad));
  assert(ad->isPacked());
  return ad;
}

ArrayData* PackedArray::ToPHPArrayVec(ArrayData* adIn, bool copy) {
  assert(checkInvariants(adIn));
  assert(adIn->isVecArray());
  ArrayData* ad = copy ? Copy(adIn) : adIn;
  ad->m_kind = HeaderKind::Packed;
  return ad;
}

ArrayData* PackedArray::ToDict(ArrayData* ad, bool copy) {
  assert(checkInvariants(ad));
  assert(ad->isPacked());

  auto mixed = [&] {
    switch (ArrayCommon::CheckForRefs(ad)) {
      case ArrayCommon::RefCheckResult::Pass:
        return copy ? ToMixedCopy(ad) : ToMixed(ad);
      case ArrayCommon::RefCheckResult::Collapse:
        // Unconditionally copy to remove unreferenced refs
        return ToMixedCopy(ad);
      case ArrayCommon::RefCheckResult::Fail:
        throwRefInvalidArrayValueException(staticEmptyDictArray());
        break;
    }
    not_reached();
  }();
  return MixedArray::ToDictInPlace(mixed);
}

ArrayData* PackedArray::ToDictVec(ArrayData* ad, bool copy) {
  assert(checkInvariants(ad));
  assert(ad->isVecArray());
  auto mixed = copy ? ToMixedCopy(ad) : ToMixed(ad);
  return MixedArray::ToDictInPlace(mixed);
}

ArrayData* PackedArray::ToVec(ArrayData* adIn, bool copy) {
  assert(checkInvariants(adIn));
  assert(adIn->isPacked());

  auto const do_copy = [&] {
    // CopyPackedHelper will copy the header and m_sizeAndPos; since we pass
    // convertingPackedToVec = true, it can fail and we have to handle that.
    // All we have to do afterwards is fix the kind and refcount in the copy;
    // it's easiest to do that by reinitializing the whole header.
    auto ad = static_cast<ArrayData*>(MM().objMallocIndex(adIn->m_aux16));
    if (!CopyPackedHelper<true>(adIn, ad)) {
      MM().objFreeIndex(ad, adIn->m_aux16);
      SystemLib::throwInvalidArgumentExceptionObject(
        "Vecs cannot contain references");
    }
    ad->initHeader(adIn->m_aux16, HeaderKind::VecArray, 1);

    return ad;
  };

  ArrayData* ad;
  if (copy) {
    ad = do_copy();
  } else {
    auto const result = ArrayCommon::CheckForRefs(adIn);
    if (LIKELY(result == ArrayCommon::RefCheckResult::Pass)) {
      adIn->m_kind = HeaderKind::VecArray;
      ad = adIn;
    } else if (result == ArrayCommon::RefCheckResult::Collapse) {
      ad = do_copy();
    } else {
      throwRefInvalidArrayValueException(staticEmptyVecArray());
    }
  }

  assert(ad->isVecArray());
  assert(capacity(ad) == capacity(adIn));
  assert(ad->m_size == adIn->m_size);
  assert(ad->m_pos == adIn->m_pos);
  assert(ad->hasExactlyOneRef());
  assert(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ToVecVec(ArrayData* ad, bool) {
  assert(checkInvariants(ad));
  assert(ad->isVecArray());
  return ad;
}

void PackedArray::OnSetEvalScalar(ArrayData* ad) {
  assert(checkInvariants(ad));
  auto ptr = packedData(ad);
  auto const stop = ptr + ad->m_size;
  for (; ptr != stop; ++ptr) {
    tvAsVariant(ptr).setEvalScalar();
  }
}

void PackedArray::Ksort(ArrayData* ad, int /*flags*/, bool ascending) {
  assert(ad->getSize() <= 1 || ascending);
}

void PackedArray::Asort(ArrayData* ad, int, bool) {
  assert(ad->getSize() <= 1);
}

bool PackedArray::Uksort(ArrayData* ad, const Variant&) {
  assert(ad->getSize() <= 1);
  return true;
}

bool PackedArray::Uasort(ArrayData* ad, const Variant&) {
  assert(ad->getSize() <= 1);
  return true;
}

ArrayData* PackedArray::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  assert(checkInvariants(ad));
  return MixedArray::ZSetInt(ToMixedCopy(ad), k, v);
}

ArrayData* PackedArray::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  assert(checkInvariants(ad));
  return MixedArray::ZSetStr(ToMixedCopy(ad), k, v);
}

ArrayData* PackedArray::ZAppend(ArrayData* ad, RefData* v, int64_t* key_ptr) {
  assert(checkInvariants(ad));
  return MixedArray::ZAppend(ToMixedCopy(ad), v, key_ptr);
}

ArrayData* PackedArray::MakeUncounted(ArrayData* array, size_t extra) {
  assert(checkInvariants(array));
  assert(!array->empty());
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().addAPCUncountedBlock();
  }

  auto const size = array->m_size;
  auto const sizeIndex = packedArrayCapacityToSizeIndex(size);
  auto const mem = static_cast<char*>(
    malloc_huge(extra + sizeof(ArrayData) + size * sizeof(TypedValue))
  );
  auto ad = reinterpret_cast<ArrayData*>(mem + extra);
  ad->initHeader(uint16_t(sizeIndex), array->m_kind, UncountedValue);
  ad->m_sizeAndPos = array->m_sizeAndPos;

  // Do a raw copy without worrying about refcounts, and convert the values to
  // uncounted later.
  auto src = packedData(array);
  auto dst = packedData(ad);
  memcpy16_inline(dst, src, sizeof(TypedValue) * size);
  for (auto end = dst + size; dst < end; ++dst) {
    ConvertTvToUncounted(dst);
  }

  assert(ad->kind() == array->kind());
  assert(capacity(ad) >= size);
  assert(ad->m_size == size);
  assert(ad->m_pos == array->m_pos);
  assert(ad->isUncounted());
  assert(checkInvariants(ad));
  return ad;
}

ALWAYS_INLINE
bool PackedArray::VecEqualHelper(const ArrayData* ad1, const ArrayData* ad2,
                                 bool strict) {
  assert(checkInvariants(ad1));
  assert(checkInvariants(ad2));
  assert(ad1->isVecArray());
  assert(ad2->isVecArray());

  if (ad1 == ad2) return true;
  if (ad1->m_size != ad2->m_size) return false;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  auto elm1 = packedData(ad1);
  auto end = elm1 + ad1->m_size;
  auto elm2 = packedData(ad2);
  if (strict) {
    for (; elm1 < end; ++elm1, ++elm2) {
      if (!cellSame(*elm1, *elm2)) return false;
    }
  } else {
    for (; elm1 < end; ++elm1, ++elm2) {
      if (!cellEqual(*elm1, *elm2)) return false;
    }
  }

  return true;
}

ALWAYS_INLINE
int64_t PackedArray::VecCmpHelper(const ArrayData* ad1, const ArrayData* ad2) {
  assert(checkInvariants(ad1));
  assert(checkInvariants(ad2));
  assert(ad1->isVecArray());
  assert(ad2->isVecArray());

  auto const size1 = ad1->m_size;
  auto const size2 = ad2->m_size;

  if (size1 < size2) return -1;
  if (size1 > size2) return 1;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  auto elm1 = packedData(ad1);
  auto end = elm1 + size1;
  auto elm2 = packedData(ad2);
  for (; elm1 < end; ++elm1, ++elm2) {
    auto const cmp = cellCompare(*elm1, *elm2);
    if (cmp != 0) return cmp;
  }

  return 0;
}

bool PackedArray::VecEqual(const ArrayData* ad1, const ArrayData* ad2) {
  return VecEqualHelper(ad1, ad2, false);
}

bool PackedArray::VecNotEqual(const ArrayData* ad1, const ArrayData* ad2) {
  return !VecEqualHelper(ad1, ad2, false);
}

bool PackedArray::VecSame(const ArrayData* ad1, const ArrayData* ad2) {
  return VecEqualHelper(ad1, ad2, true);
}

bool PackedArray::VecNotSame(const ArrayData* ad1, const ArrayData* ad2) {
  return !VecEqualHelper(ad1, ad2, true);
}

bool PackedArray::VecLt(const ArrayData* ad1, const ArrayData* ad2) {
  return VecCmpHelper(ad1, ad2) < 0;
}

bool PackedArray::VecLte(const ArrayData* ad1, const ArrayData* ad2) {
  return VecCmpHelper(ad1, ad2) <= 0;
}

bool PackedArray::VecGt(const ArrayData* ad1, const ArrayData* ad2) {
  return VecCmpHelper(ad1, ad2) > 0;
}

bool PackedArray::VecGte(const ArrayData* ad1, const ArrayData* ad2) {
  return VecCmpHelper(ad1, ad2) >= 0;
}

int64_t PackedArray::VecCmp(const ArrayData* ad1, const ArrayData* ad2) {
  return VecCmpHelper(ad1, ad2);
}

//////////////////////////////////////////////////////////////////////

}
