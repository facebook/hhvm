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
#include "hphp/runtime/base/tv-val.h"
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
std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyVArray;

struct PackedArray::VecInitializer {
  VecInitializer() {
    auto const ad = reinterpret_cast<ArrayData*>(&s_theEmptyVecArray);
    ad->m_sizeAndPos = 0;
    ad->initHeader_16(
      HeaderKind::VecArray,
      StaticValue,
      packSizeIndexAndAuxBits(0, ArrayData::kNotDVArray)
    );
    assertx(checkInvariants(ad));
  }
};
PackedArray::VecInitializer PackedArray::s_vec_initializer;

struct PackedArray::VArrayInitializer {
  VArrayInitializer() {
    auto const ad = reinterpret_cast<ArrayData*>(&s_theEmptyVArray);
    ad->m_sizeAndPos = 0;
    ad->initHeader_16(
      HeaderKind::Packed,
      StaticValue,
      packSizeIndexAndAuxBits(0, ArrayData::kVArray)
    );
    assertx(RuntimeOption::EvalHackArrDVArrs || checkInvariants(ad));
  }
};
PackedArray::VArrayInitializer PackedArray::s_varr_initializer;

//////////////////////////////////////////////////////////////////////

namespace {

inline ArrayData* alloc_packed_static(size_t cap) {
  auto size = sizeof(ArrayData) + cap * sizeof(TypedValue);
  auto ret = RuntimeOption::EvalLowStaticArrays ? low_malloc(size)
                                                : uncounted_malloc(size);
  return static_cast<ArrayData*>(ret);
}

}

bool PackedArray::checkInvariants(const ArrayData* arr) {
  assertx(arr->hasPackedLayout());
  assertx(arr->checkCount());
  assertx(arr->m_size <= MixedArray::MaxSize);
  assertx(arr->m_size <= capacity(arr));
  assertx(arr->m_pos >= 0 && arr->m_pos <= arr->m_size);
  assertx(!arr->isPacked() || !arr->isDArray());
  assertx(!arr->isVecArray() || arr->isNotDVArray());
  assertx(!RuntimeOption::EvalHackArrDVArrs || arr->isNotDVArray());
  static_assert(ArrayData::kPackedKind == 0, "");
  // Note that m_pos < m_size is not an invariant, because an array
  // that grows will only adjust m_size to zero on the old array.

  // This loop is too slow for normal use, but can be enabled to debug
  // packed arrays.
  if (false) {
    auto ptr = packedData(arr);
    auto const stop = ptr + arr->m_size;
    for (; ptr != stop; ptr++) {
      assertx(ptr->m_type != KindOfUninit);
      assertx(tvIsPlausible(*ptr));
      assertx(!arr->isVecArray() || !isRefType(ptr->m_type));
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
MixedArray* PackedArray::ToMixedHeader(const ArrayData* old,
                                       size_t neededSize) {
  assertx(checkInvariants(old));

  if (UNLIKELY(RuntimeOption::EvalHackArrCompatPromoteNotices) &&
      old->isVArray()) {
    raise_hackarr_compat_notice("varray promoting to darray");
  }

  auto const oldSize = old->m_size;
  auto const scale   = MixedArray::computeScaleFromSize(neededSize);
  auto const ad      = MixedArray::reqAlloc(scale);
  ad->m_sizeAndPos   = oldSize | int64_t{old->m_pos} << 32;
  ad->initHeader_16(
    HeaderKind::Mixed,
    OneReference,
    old->isVArray() ? ArrayData::kDArray : ArrayData::kNotDVArray
  );
  ad->m_scale_used   = scale | uint64_t{oldSize} << 32; // used=oldSize
  ad->m_nextKI       = oldSize;

  assertx(ad->m_size == oldSize);
  assertx(ad->m_pos == old->m_pos);
  assertx(ad->kind() == ArrayData::kMixedKind);
  assertx(ad->isDArray() == old->isVArray());
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == oldSize);
  assertx(ad->m_scale == scale);
  assertx(ad->m_nextKI == oldSize);
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

  assertx(ad->checkInvariants());
  assertx(!ad->isFull());
  assertx(ad->hasExactlyOneRef());
  return ad;
}

/*
 * Convert a packed array to mixed, without moving the elements out of
 * the old packed array.  This effectively performs a Copy at the same
 * time as converting to mixed.  The returned mixed array is
 * guaranteed not to be full.
 */
MixedArray* PackedArray::ToMixedCopy(const ArrayData* old) {
  assertx(checkInvariants(old));

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

  assertx(ad->checkInvariants());
  assertx(!ad->isFull());
  assertx(ad->hasExactlyOneRef());
  return ad;
}

/*
 * Convert to mixed, reserving space for at least `neededSize' elems.
 * The `neededSize' should include old->size(), but may be equal to
 * it.
 */
MixedArray* PackedArray::ToMixedCopyReserve(const ArrayData* old,
                                           size_t neededSize) {
  assertx(neededSize >= old->m_size);
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

  assertx(ad->checkInvariants());
  assertx(ad->hasExactlyOneRef());
  return ad;
}

NEVER_INLINE
ArrayData* PackedArray::Grow(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->m_size == capacity(adIn));

  auto const sizeIndex = sizeClass(adIn) + kSizeClassesPerDoubling;
  if (UNLIKELY(sizeIndex > MaxSizeIndex)) return nullptr;
  auto ad = static_cast<ArrayData*>(tl_heap->objMallocIndex(sizeIndex));

  if (copy) {
    // CopyPackedHelper will copy the header and m_sizeAndPos; since we pass
    // convertingPackedToVec = false, it can't fail. All we have to do
    // afterwards is fix the capacity and refcount on the copy; it's easiest
    // to do that by reinitializing the whole header.
    auto const DEBUG_ONLY ok = CopyPackedHelper<false>(adIn, ad);
    assertx(ok);
    ad->initHeader_16(
      adIn->m_kind,
      OneReference,
      packSizeIndexAndAuxBits(sizeIndex, adIn->auxBits())
    );

    assertx(ad->m_size == adIn->m_size);
    assertx(ad->m_pos == adIn->m_pos);
  } else {
    // Copy everything from `adIn' to `ad', including header and m_sizeAndPos
    static_assert(sizeof(ArrayData) == 16 && sizeof(TypedValue) == 16, "");
    memcpy16_inline(ad, adIn, (adIn->m_size + 1) * sizeof(TypedValue));
    ad->initHeader_16(
      adIn->m_kind,
      OneReference,
      packSizeIndexAndAuxBits(sizeIndex, adIn->auxBits())
    );

    assertx(ad->m_size == adIn->m_size);
    assertx(ad->m_pos == adIn->m_pos);
    adIn->m_sizeAndPos = 0; // old is a zombie now

    if (UNLIKELY(strong_iterators_exist())) move_strong_iterators(ad, adIn);
  }

  assertx(ad->kind() == adIn->kind());
  assertx(ad->dvArray() == adIn->dvArray());
  assertx(capacity(ad) > capacity(adIn));
  assertx(ad->hasExactlyOneRef());
  assertx(checkInvariants(ad));
  return ad;
}

ALWAYS_INLINE
ArrayData* PackedArray::PrepareForInsert(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
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
    if (UNLIKELY(isRefType(elm->m_type))) {
      assertx(!adIn->isVecArray());
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
    tvIncRefGen(*elm);
  }
  return true;
}

NEVER_INLINE
ArrayData* PackedArray::Copy(const ArrayData* adIn) {
  assertx(checkInvariants(adIn));

  auto ad = static_cast<ArrayData*>(tl_heap->objMallocIndex(sizeClass(adIn)));

  // CopyPackedHelper will copy the header (including capacity and kind), and
  // m_sizeAndPos; since we pass convertingPackedToVec = false, it can't fail.
  // All we have to do afterwards is fix the refcount on the copy.
  auto const DEBUG_ONLY ok = CopyPackedHelper<false>(adIn, ad);
  assertx(ok);
  ad->m_count = OneReference;

  assertx(ad->kind() == adIn->kind());
  assertx(ad->isLegacyArray() == adIn->isLegacyArray());
  assertx(capacity(ad) == capacity(adIn));
  assertx(ad->m_size == adIn->m_size);
  assertx(ad->m_pos == adIn->m_pos);
  assertx(ad->hasExactlyOneRef());
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::CopyStatic(const ArrayData* adIn) {
  assertx(checkInvariants(adIn));

  auto const sizeIndex = capacityToSizeIndex(adIn->m_size);
  auto ad = alloc_packed_static(adIn->m_size);
  // CopyPackedHelper will copy the header and m_sizeAndPos; since we pass
  // convertingPackedToVec = false, it can't fail. All we have to do afterwards
  // is fix the capacity and refcount on the copy; it's easiest to do that by
  // reinitializing the whole header.
  auto const DEBUG_ONLY ok = CopyPackedHelper<false>(adIn, ad);
  assertx(ok);
  ad->initHeader_16(
    adIn->m_kind,
    StaticValue,
    packSizeIndexAndAuxBits(sizeIndex, adIn->auxBits())
  );

  assertx(ad->kind() == adIn->kind());
  assertx(ad->dvArray() == adIn->dvArray());
  assertx(capacity(ad) >= adIn->m_size);
  assertx(ad->m_size == adIn->m_size);
  assertx(ad->m_pos == adIn->m_pos);
  assertx(ad->isStatic());
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ConvertStatic(const ArrayData* arr) {
  assertx(arr->isVectorData());
  assertx(!RuntimeOption::EvalHackArrDVArrs || arr->isNotDVArray());
  assertx(!arr->isDArray());

  auto const sizeIndex = capacityToSizeIndex(arr->m_size);
  auto ad = alloc_packed_static(arr->m_size);
  ad->initHeader_16(
    HeaderKind::Packed,
    StaticValue,
    packSizeIndexAndAuxBits(sizeIndex, arr->auxBits())
  );
  ad->m_sizeAndPos = arr->m_sizeAndPos;

  auto data = packedData(ad);
  auto pos_limit = arr->iter_end();
  for (auto pos = arr->iter_begin(); pos != pos_limit;
       pos = arr->iter_advance(pos), ++data) {
    tvDupWithRef(arr->atPos(pos), *data, arr);
  }

  assertx(ad->isPacked());
  assertx(capacity(ad) >= arr->m_size);
  assertx(ad->dvArray() == arr->dvArray());
  assertx(ad->m_size == arr->m_size);
  assertx(ad->m_pos == arr->m_pos);
  assertx(ad->isStatic());
  assertx(checkInvariants(ad));
  return ad;
}

/* This helper allocates an ArrayData and initializes the header (including
 * capacity, kind, and refcount). The caller is responsible for initializing
 * m_sizeAndPos, and initializing array entries (if any).
 */
ALWAYS_INLINE
ArrayData* PackedArray::MakeReserveImpl(uint32_t cap,
                                        HeaderKind hk,
                                        ArrayData::DVArray dvarray) {
  assertx(!RuntimeOption::EvalHackArrDVArrs ||
         dvarray == ArrayData::kNotDVArray);
  auto const sizeIndex = capacityToSizeIndex(cap);
  auto ad = static_cast<ArrayData*>(tl_heap->objMallocIndex(sizeIndex));
  ad->initHeader_16(
    hk,
    OneReference,
    packSizeIndexAndAuxBits(sizeIndex, dvarray)
  );
  assertx(ad->m_kind == hk);
  assertx(ad->dvArray() == dvarray);
  assertx(capacity(ad) >= cap);
  assertx(ad->hasExactlyOneRef());
  return ad;
}

ArrayData* PackedArray::MakeReserve(uint32_t capacity) {
  auto ad =
    MakeReserveImpl(capacity, HeaderKind::Packed, ArrayData::kNotDVArray);
  ad->m_sizeAndPos = 0;
  assertx(ad->isPacked());
  assertx(ad->isNotDVArray());
  assertx(ad->m_size == 0);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakeReserveVArray(uint32_t capacity) {
  auto ad = MakeReserveImpl(capacity, HeaderKind::Packed, ArrayData::kVArray);
  ad->m_sizeAndPos = 0;
  assertx(ad->isPacked());
  assertx(ad->isVArray());
  assertx(ad->m_size == 0);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakeReserveVec(uint32_t capacity) {
  auto ad =
    MakeReserveImpl(capacity, HeaderKind::VecArray, ArrayData::kNotDVArray);
  ad->m_sizeAndPos = 0;
  assertx(ad->isVecArray());
  assertx(ad->m_size == 0);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return ad;
}

template<bool reverse>
ALWAYS_INLINE
ArrayData* PackedArray::MakePackedImpl(uint32_t size,
                                       const Cell* values,
                                       HeaderKind hk,
                                       ArrayData::DVArray dv) {
  assertx(size > 0);
  auto ad = MakeReserveImpl(size, hk, dv);
  ad->m_sizeAndPos = size; // pos = 0

  // Append values by moving; this function takes ownership of them.
  if (reverse) {
    auto elm = packedData(ad) + size - 1;
    for (auto end = values + size; values < end; ++values, --elm) {
      cellCopy(*values, *elm);
    }
  } else {
    if (debug) {
      for (uint32_t i = 0; i < size; ++i) {
        assertx(cellIsPlausible(*(values + i)));
      }
    }
    memcpy16_inline(packedData(ad), values, sizeof(Cell) * size);
  }

  assertx(ad->m_size == size);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakePacked(uint32_t size, const Cell* values) {
  // Values are in reverse order since they come from the stack, which
  // grows down.
  auto ad = MakePackedImpl<true>(size, values, HeaderKind::Packed,
                                 ArrayData::kNotDVArray);
  assertx(ad->isPacked());
  assertx(ad->isNotDVArray());
  return ad;
}

ArrayData* PackedArray::MakeVArray(uint32_t size, const Cell* values) {
  // Values are in reverse order since they come from the stack, which
  // grows down.
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto ad = MakePackedImpl<true>(size, values, HeaderKind::Packed,
                                 ArrayData::kVArray);
  assertx(ad->isPacked());
  assertx(ad->isVArray());
  return ad;
}

ArrayData* PackedArray::MakeVec(uint32_t size, const Cell* values) {
  // Values are in reverse order since they come from the stack, which
  // grows down.
  auto ad = MakePackedImpl<true>(size, values, HeaderKind::VecArray,
                                 ArrayData::kNotDVArray);
  assertx(ad->isVecArray());
  return ad;
}

ArrayData* PackedArray::MakePackedNatural(uint32_t size, const Cell* values) {
  auto ad = MakePackedImpl<false>(size, values, HeaderKind::Packed,
                                  ArrayData::kNotDVArray);
  assertx(ad->isPacked());
  assertx(ad->isNotDVArray());
  return ad;
}

ArrayData* PackedArray::MakeUninitialized(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::Packed, ArrayData::kNotDVArray);
  ad->m_sizeAndPos = size; // pos = 0
  assertx(ad->isPacked());
  assertx(ad->isNotDVArray());
  assertx(ad->m_size == size);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakeUninitializedVArray(uint32_t size) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto ad = MakeReserveImpl(size, HeaderKind::Packed, ArrayData::kVArray);
  ad->m_sizeAndPos = size; // pos = 0
  assertx(ad->isPacked());
  assertx(ad->isVArray());
  assertx(ad->m_size == size);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakeUninitializedVec(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::VecArray, ArrayData::kNotDVArray);
  ad->m_sizeAndPos = size; // pos = 0
  assertx(ad->isVecArray());
  assertx(ad->m_size == size);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakeVecFromAPC(const APCArray* apc) {
  assertx(apc->isVec());
  auto const apcSize = apc->size();
  VecArrayInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.append(apc->getValue(i)->toLocal());
  }
  return init.create();
}

ArrayData* PackedArray::MakeVArrayFromAPC(const APCArray* apc) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  assertx(apc->isVArray());
  auto const apcSize = apc->size();
  VArrayInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.append(apc->getValue(i)->toLocal());
  }
  return init.create();
}

void PackedArray::Release(ArrayData* ad) {
  ad->fixCountForRelease();
  assertx(checkInvariants(ad));
  assertx(ad->isRefCounted());
  assertx(ad->hasExactlyOneRef());

  for (auto elm = packedData(ad), end = elm + ad->m_size; elm < end; ++elm) {
    tvDecRefGen(elm);
  }
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(ad);
  }
  tl_heap->objFreeIndex(ad, sizeClass(ad));
  AARCH64_WALKABLE_FRAME();
}

NEVER_INLINE
void PackedArray::ReleaseUncounted(ArrayData* ad) {
  assertx(checkInvariants(ad));
  if (!ad->uncountedDecRef()) return;

  auto const data = packedData(ad);
  auto const stop = data + ad->m_size;
  for (auto ptr = data; ptr != stop; ++ptr) {
    ReleaseUncountedTv(*ptr);
  }

  // We better not have strong iterators associated with uncounted arrays.
  assertx(!has_strong_iterator(ad));
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().removeAPCUncountedBlock();
  }

  auto const extra = (ad->hasApcTv() ? sizeof(APCTypedValue) : 0);
  auto const allocSize = extra + sizeof(PackedArray) +
                         ad->m_size * sizeof(TypedValue);
  uncounted_sized_free(reinterpret_cast<char*>(ad) - extra, allocSize);
}

////////////////////////////////////////////////////////////////////////////////

tv_rval PackedArray::NvGetInt(const ArrayData* ad, int64_t ki) {
  assertx(checkInvariants(ad));
  auto const data = packedData(ad);
  return LIKELY(size_t(ki) < ad->m_size) ? &data[ki] : nullptr;
}

tv_rval
PackedArray::NvGetStr(const ArrayData* ad, const StringData* /*s*/) {
  assertx(checkInvariants(ad));
  return nullptr;
}

tv_rval PackedArray::NvTryGetIntVec(const ArrayData* ad, int64_t k) {
  assertx(checkInvariants(ad));
  assertx(ad->isVecArray());
  auto const data = packedData(ad);
  if (LIKELY(size_t(k) < ad->m_size)) return &data[k];
  throwOOBArrayKeyException(k, ad);
}

tv_rval PackedArray::NvTryGetStrVec(const ArrayData* ad,
                                               const StringData* s) {
  assertx(checkInvariants(ad));
  assertx(ad->isVecArray());
  throwInvalidArrayKeyException(s, ad);
}

Cell PackedArray::NvGetKey(const ArrayData* ad, ssize_t pos) {
  assertx(checkInvariants(ad));
  assertx(pos != ad->m_size);
  return make_tv<KindOfInt64>(pos);
}

size_t PackedArray::Vsize(const ArrayData*) {
  // PackedArray always has a valid m_size so it's an error to get here.
  always_assert(false);
}

tv_rval PackedArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  assertx(checkInvariants(ad));
  assertx(pos != ad->m_size);
  return &packedData(ad)[pos];
}

bool PackedArray::ExistsInt(const ArrayData* ad, int64_t k) {
  assertx(checkInvariants(ad));
  return size_t(k) < ad->m_size;
}

bool PackedArray::ExistsStr(const ArrayData* ad, const StringData* /*s*/) {
  assertx(checkInvariants(ad));
  return false;
}

///////////////////////////////////////////////////////////////////////////////

namespace {

template<typename FoundFn, typename AppendFn, typename PromotedFn>
auto MutableOpInt(ArrayData* adIn, int64_t k, bool copy,
                  FoundFn found, AppendFn append, PromotedFn promoted) {
  assertx(PackedArray::checkInvariants(adIn));
  assertx(adIn->isPacked());

  if (LIKELY(size_t(k) < adIn->getSize())) {
    auto const ad = copy ? PackedArray::Copy(adIn) : adIn;
    return found(ad);
  }

  if (size_t(k) == adIn->getSize()) {
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatPromoteNotices) &&
        adIn->isVArray()) {
      raise_hackarr_compat_notice("Implicit append to varray");
    }
    return append();
  }

  auto const mixed = copy ? PackedArray::ToMixedCopy(adIn)
                          : PackedArray::ToMixed(adIn);
  return promoted(mixed);
}

template <typename PromotedFn>
auto MutableOpStr(ArrayData* adIn, StringData* /*k*/, bool copy,
                  PromotedFn promoted) {
  assertx(PackedArray::checkInvariants(adIn));
  assertx(adIn->isPacked());

  auto const mixed = copy ? PackedArray::ToMixedCopy(adIn)
                          : PackedArray::ToMixed(adIn);
  return promoted(mixed);
}

template<typename FoundFn>
auto MutableOpIntVec(ArrayData* adIn, int64_t k, bool copy, FoundFn found) {
  assertx(PackedArray::checkInvariants(adIn));
  assertx(adIn->isVecArray());

  if (UNLIKELY(size_t(k) >= adIn->getSize())) {
    throwOOBArrayKeyException(k, adIn);
  }
  auto const ad = copy ? PackedArray::Copy(adIn) : adIn;
  return found(ad);
}

}

arr_lval PackedArray::LvalInt(ArrayData* adIn, int64_t k, bool copy) {
  return MutableOpInt(adIn, k, copy,
    [&] (ArrayData* ad) { return arr_lval { ad, &packedData(ad)[k] }; },
    [&] { return LvalNew(adIn, copy); },
    // TODO(#2606310): Make use of our knowledge that the key is missing.
    [&] (MixedArray* mixed) { return mixed->addLvalImpl<true>(k); }
  );
}

arr_lval PackedArray::LvalIntRef(ArrayData* adIn, int64_t k, bool copy) {
  if (checkHACRefBind()) raiseHackArrCompatRefBind(k);
  return LvalInt(adIn, k, copy);
}

arr_lval PackedArray::LvalIntVec(ArrayData* adIn, int64_t k, bool copy) {
  return MutableOpIntVec(adIn, k, copy,
    [&] (ArrayData* ad) { return arr_lval { ad, &packedData(ad)[k] }; }
  );
}

arr_lval PackedArray::LvalSilentInt(ArrayData* adIn, int64_t k, bool copy) {
  assertx(checkInvariants(adIn));
  if (UNLIKELY(size_t(k) >= adIn->m_size)) return {adIn, nullptr};
  auto const ad = copy ? Copy(adIn) : adIn;
  return arr_lval { ad, &packedData(ad)[k] };
}

arr_lval PackedArray::LvalStr(ArrayData* adIn, StringData* k, bool copy) {
  return MutableOpStr(adIn, k, copy,
    // TODO(#2606310): Make use of our knowledge that the key is missing.
    [&] (MixedArray* mixed) { return mixed->addLvalImpl<true>(k); }
  );
}

arr_lval
PackedArray::LvalStrRef(ArrayData* adIn, StringData* key, bool copy) {
  if (checkHACRefBind()) raiseHackArrCompatRefBind(key);
  return LvalStr(adIn, key, copy);
}

arr_lval
PackedArray::LvalStrVec(ArrayData* adIn, StringData* key, bool) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  throwInvalidArrayKeyException(key, adIn);
}

arr_lval PackedArray::LvalIntRefVec(ArrayData* adIn, int64_t /*k*/, bool) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  throwRefInvalidArrayValueException(adIn);
}

arr_lval
PackedArray::LvalStrRefVec(ArrayData* adIn, StringData* key, bool) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  throwInvalidArrayKeyException(key, adIn);
}

arr_lval PackedArray::LvalNew(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  auto const ad = PrepareForInsert(adIn, copy);
  auto& tv = packedData(ad)[ad->m_size++];
  tv.m_type = KindOfNull;
  return arr_lval { ad, &tv };
}

arr_lval PackedArray::LvalNewRef(ArrayData* adIn, bool copy) {
  if (checkHACRefBind()) raiseHackArrCompatRefNew();
  return LvalNew(adIn, copy);
}

arr_lval PackedArray::LvalNewRefVec(ArrayData* adIn, bool) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
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
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  throwInvalidArrayKeyException(k, adIn);
}

ArrayData* PackedArray::SetWithRefInt(ArrayData* adIn, int64_t k,
                                      TypedValue v, bool copy) {
  auto const checkHackArrRef = [&] {
    if (checkHACRefBind() && tvIsReferenced(v)) {
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
      tvSetWithRef(v, lval);
      return lval.arr;
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
      if (checkHACRefBind() && tvIsReferenced(v)) {
        raiseHackArrCompatRefBind(k);
      }
      auto const lval = mixed->addLvalImpl<false>(k);
      tvSetWithRef(v, lval);
      return lval.arr;
    }
  );
}

ArrayData* PackedArray::SetWithRefStrVec(ArrayData* adIn, StringData* k,
                                         TypedValue, bool) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  throwInvalidArrayKeyException(k, adIn);
}

ArrayData* PackedArray::SetRefInt(ArrayData* adIn, int64_t k,
                                  tv_lval v, bool copy) {
  if (checkHACRefBind()) raiseHackArrCompatRefBind(k);

  return MutableOpInt(adIn, k, copy,
    [&] (ArrayData* ad) {
      tvBoxIfNeeded(v);
      tvBind(v.tv(), packedData(ad)[k]);
      return ad;
    },
    [&] { return AppendRef(adIn, v, copy); },
    // TODO(#2606310): Make use of our knowledge that the key is missing.
    [&] (MixedArray* mixed) { return mixed->updateRef(k, v); }
  );
}

ArrayData* PackedArray::SetRefIntVec(ArrayData* adIn, int64_t,
                                     tv_lval, bool) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  throwRefInvalidArrayValueException(adIn);
}

ArrayData* PackedArray::SetRefStr(ArrayData* adIn, StringData* k,
                                  tv_lval v, bool copy) {
  if (checkHACRefBind()) raiseHackArrCompatRefBind(k);

  return MutableOpStr(adIn, k, copy,
    // TODO(#2606310): Make use of our knowledge that the key is missing.
    [&] (MixedArray* mixed) { return mixed->updateRef(k, v); }
  );
}

ArrayData* PackedArray::SetRefStrVec(ArrayData* adIn, StringData* k,
                                     tv_lval, bool) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  throwInvalidArrayKeyException(k, adIn);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

void adjustMArrayIterAfterPop(ArrayData* ad) {
  assertx(ad->hasPackedLayout());
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
  assertx(checkInvariants(adIn));
  assertx(adIn->isPacked());
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
  assertx(checkInvariants(ad));
  return 0;
}

ssize_t PackedArray::IterLast(const ArrayData* ad) {
  assertx(checkInvariants(ad));
  return ad->m_size ? ad->m_size - 1 : 0;
}

ssize_t PackedArray::IterEnd(const ArrayData* ad) {
  assertx(checkInvariants(ad));
  return ad->m_size;
}

ssize_t PackedArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  assertx(checkInvariants(ad));
  if (pos < ad->m_size) {
    ++pos;
  }
  return pos;
}

ssize_t PackedArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  assertx(checkInvariants(ad));
  if (pos > 0) {
    return pos - 1;
  }
  return ad->m_size;
}

bool PackedArray::AdvanceMArrayIter(ArrayData* ad, MArrayIter& fp) {
  assertx(checkInvariants(ad));
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
  assertx(checkInvariants(adIn));
  assertx(v.m_type != KindOfUninit);
  auto const ad = PrepareForInsert(adIn, copy);
  cellDup(v, packedData(ad)[ad->m_size++]);
  return ad;
}

ArrayData* PackedArray::AppendRef(ArrayData* adIn, tv_lval v, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isPacked());
  if (checkHACRefBind()) raiseHackArrCompatRefNew();
  auto const ad = PrepareForInsert(adIn, copy);
  auto& dst = packedData(ad)[ad->m_size++];
  tvBoxIfNeeded(v);
  dst.m_data.pref = v.val().pref;
  dst.m_type = KindOfRef;
  dst.m_data.pref->incRefCount();
  return ad;
}

ArrayData* PackedArray::AppendRefVec(ArrayData* adIn, tv_lval, bool) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  throwRefInvalidArrayValueException(adIn);
}

ArrayData*
PackedArray::AppendWithRef(ArrayData* adIn, TypedValue v, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isPacked());

  if (checkHACRefBind() && tvIsReferenced(v)) {
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
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  if (tvIsReferenced(v)) throwRefInvalidArrayValueException(adIn);
  return Append(adIn, tvToInitCell(v), copy);
}

ArrayData* PackedArray::PlusEq(ArrayData* adIn, const ArrayData* elems) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isPacked());
  if (!elems->isPHPArray()) throwInvalidAdditionException(elems);
  auto const neededSize = adIn->size() + elems->size();
  auto const mixed = ToMixedCopyReserve(adIn, neededSize);
  try {
    auto const ret = MixedArray::PlusEq(mixed, elems);
    assertx(ret == mixed);
    assertx(mixed->hasExactlyOneRef());
    return ret;
  } catch (...) {
    MixedArray::Release(mixed);
    throw;
  }
}

ArrayData* PackedArray::PlusEqVec(ArrayData* adIn, const ArrayData* /*elems*/) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  throwInvalidAdditionException(adIn);
}

ArrayData* PackedArray::Merge(ArrayData* adIn, const ArrayData* elems) {
  assertx(checkInvariants(adIn));
  auto const neededSize = adIn->m_size + elems->size();
  auto const ret = ToMixedCopyReserve(adIn, neededSize);
  ret->setDVArray(ArrayData::kNotDVArray);
  return MixedArray::ArrayMergeGeneric(ret, elems);
}

ArrayData* PackedArray::Pop(ArrayData* adIn, Variant& value) {
  assertx(checkInvariants(adIn));

  auto const ad = adIn->cowCheck() ? Copy(adIn) : adIn;

  if (UNLIKELY(ad->m_size == 0)) {
    assertx(ad->m_pos == 0);
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
  assertx(checkInvariants(adIn));

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
  assertx(checkInvariants(adIn));

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

ArrayData* PackedArray::ToPHPArray(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isPacked());
  if (adIn->isNotDVArray()) return adIn;
  assertx(adIn->isVArray());
  if (adIn->getSize() == 0) return staticEmptyArray();
  ArrayData* ad = copy ? Copy(adIn) : adIn;
  ad->setDVArray(ArrayData::kNotDVArray);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ToVArray(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isPacked());
  if (RuntimeOption::EvalHackArrDVArrs) return ToVec(adIn, copy);
  if (adIn->isVArray()) return adIn;
  if (adIn->getSize() == 0) return staticEmptyVArray();
  ArrayData* ad = copy ? Copy(adIn) : adIn;
  ad->setDVArray(ArrayData::kVArray);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ToDArray(ArrayData* adIn, bool /*copy*/) {
  assertx(checkInvariants(adIn));

  auto const size = adIn->getSize();
  if (size == 0) return staticEmptyDArray();

  DArrayInit init{size};
  auto const elms = packedData(adIn);
  for (int64_t i = 0; i < size; ++i) init.add(i, elms[i]);
  return init.create();
}

ArrayData* PackedArray::ToPHPArrayVec(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  ArrayData* ad = copy ? Copy(adIn) : adIn;
  ad->m_kind = HeaderKind::Packed;
  assertx(ad->isNotDVArray());
  ad->setLegacyArray(false);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ToVArrayVec(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArray());
  if (RuntimeOption::EvalHackArrDVArrs) return adIn;
  if (adIn->getSize() == 0) return staticEmptyVArray();
  ArrayData* ad = copy ? Copy(adIn) : adIn;
  ad->m_kind = HeaderKind::Packed;
  ad->setDVArray(ArrayData::kVArray);
  ad->setLegacyArray(false);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ToDict(ArrayData* ad, bool copy) {
  assertx(checkInvariants(ad));
  assertx(ad->isPacked());

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
  assertx(checkInvariants(ad));
  assertx(ad->isVecArray());
  auto mixed = copy ? ToMixedCopy(ad) : ToMixed(ad);
  return MixedArray::ToDictInPlace(mixed);
}

ArrayData* PackedArray::ToVec(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isPacked());

  auto const do_copy = [&] {
    // CopyPackedHelper will copy the header and m_sizeAndPos; since we pass
    // convertingPackedToVec = true, it can fail and we have to handle that.
    // All we have to do afterwards is fix the kind and refcount in the copy;
    // it's easiest to do that by reinitializing the whole header.
    auto ad = static_cast<ArrayData*>(tl_heap->objMallocIndex(sizeClass(adIn)));
    if (!CopyPackedHelper<true>(adIn, ad)) {
      tl_heap->objFreeIndex(ad, sizeClass(adIn));
      SystemLib::throwInvalidArgumentExceptionObject(
        "Vecs cannot contain references");
    }
    ad->initHeader_16(
      HeaderKind::VecArray,
      OneReference,
      packSizeIndexAndAuxBits(sizeClass(adIn), ArrayData::kNotDVArray)
    );
    return ad;
  };

  ArrayData* ad;
  if (copy) {
    ad = do_copy();
  } else {
    auto const result = ArrayCommon::CheckForRefs(adIn);
    if (LIKELY(result == ArrayCommon::RefCheckResult::Pass)) {
      adIn->m_kind = HeaderKind::VecArray;
      adIn->setDVArray(ArrayData::kNotDVArray);
      ad = adIn;
    } else if (result == ArrayCommon::RefCheckResult::Collapse) {
      ad = do_copy();
    } else {
      throwRefInvalidArrayValueException(staticEmptyVecArray());
    }
  }

  assertx(ad->isVecArray());
  assertx(capacity(ad) == capacity(adIn));
  assertx(ad->m_size == adIn->m_size);
  assertx(ad->m_pos == adIn->m_pos);
  assertx(ad->hasExactlyOneRef());
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ToVecVec(ArrayData* ad, bool) {
  assertx(checkInvariants(ad));
  assertx(ad->isVecArray());
  return ad;
}

void PackedArray::OnSetEvalScalar(ArrayData* ad) {
  assertx(checkInvariants(ad));
  auto ptr = packedData(ad);
  auto const stop = ptr + ad->m_size;
  for (; ptr != stop; ++ptr) {
    tvAsVariant(ptr).setEvalScalar();
  }
}

void PackedArray::Ksort(ArrayData* ad, int /*flags*/, bool ascending) {
  assertx(ad->getSize() <= 1 || ascending);
}

void PackedArray::Asort(ArrayData* ad, int, bool) {
  assertx(ad->getSize() <= 1);
}

bool PackedArray::Uksort(ArrayData* ad, const Variant&) {
  assertx(ad->getSize() <= 1);
  return true;
}

bool PackedArray::Uasort(ArrayData* ad, const Variant&) {
  assertx(ad->getSize() <= 1);
  return true;
}

ArrayData* PackedArray::MakeUncounted(ArrayData* array,
                                      bool withApcTypedValue,
                                      DataWalker::PointerMap* seen) {
  auto const updateSeen = seen && array->hasMultipleRefs();
  if (updateSeen) {
    auto it = seen->find(array);
    assertx(it != seen->end());
    if (auto const arr = static_cast<ArrayData*>(it->second)) {
      if (arr->uncountedIncRef()) {
        return arr;
      }
    }
  }
  assertx(checkInvariants(array));
  assertx(!array->empty());
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().addAPCUncountedBlock();
  }

  auto const extra = withApcTypedValue ? sizeof(APCTypedValue) : 0;
  auto const size = array->m_size;
  auto const sizeIndex = capacityToSizeIndex(size);
  auto const mem = static_cast<char*>(
    uncounted_malloc(extra + sizeof(ArrayData) + size * sizeof(TypedValue))
  );
  auto ad = reinterpret_cast<ArrayData*>(mem + extra);
  ad->initHeader_16(
    array->m_kind,
    UncountedValue,
    packSizeIndexAndAuxBits(sizeIndex, array->auxBits()) |
    (withApcTypedValue ? ArrayData::kHasApcTv : 0)
  );
  ad->m_sizeAndPos = array->m_sizeAndPos;

  // Do a raw copy without worrying about refcounts, and convert the values to
  // uncounted later.
  auto src = packedData(array);
  auto dst = packedData(ad);
  memcpy16_inline(dst, src, sizeof(TypedValue) * size);
  for (auto end = dst + size; dst < end; ++dst) {
    ConvertTvToUncounted(dst, seen);
  }

  assertx(ad->kind() == array->kind());
  assertx(ad->dvArray() == array->dvArray());
  assertx(capacity(ad) >= size);
  assertx(ad->m_size == size);
  assertx(ad->m_pos == array->m_pos);
  assertx(ad->isUncounted());
  assertx(checkInvariants(ad));
  if (updateSeen) (*seen)[array] = ad;
  return ad;
}

ALWAYS_INLINE
bool PackedArray::VecEqualHelper(const ArrayData* ad1, const ArrayData* ad2,
                                 bool strict) {
  assertx(checkInvariants(ad1));
  assertx(checkInvariants(ad2));
  assertx(ad1->isVecArray());
  assertx(ad2->isVecArray());

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
  assertx(checkInvariants(ad1));
  assertx(checkInvariants(ad2));
  assertx(ad1->isVecArray());
  assertx(ad2->isVecArray());

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
