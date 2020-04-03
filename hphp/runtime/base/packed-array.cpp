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

#include <folly/Format.h>
#include <folly/Likely.h>

#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-helpers.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/tv-variant.h"

#include "hphp/runtime/base/mixed-array-defs.h"
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

inline ArrayData* alloc_packed_static(const ArrayData* ad) {
  auto const extra = arrprov::tagSize(ad);
  auto const size = sizeof(ArrayData)
                  + ad->getSize() * sizeof(TypedValue)
                  + extra;
  auto const ret = RuntimeOption::EvalLowStaticArrays
    ? low_malloc(size)
    : uncounted_malloc(size);
  return reinterpret_cast<ArrayData*>(reinterpret_cast<char*>(ret) + extra);
}

}

bool PackedArray::checkInvariants(const ArrayData* arr) {
  assertx(arr->hasVanillaPackedLayout());
  assertx(arr->checkCountZ());
  assertx(arr->m_size <= MixedArray::MaxSize);
  assertx(arr->m_size <= capacity(arr));
  assertx(arr->m_pos >= 0 && arr->m_pos <= arr->m_size);
  assertx(!arr->isPackedKind() || !arr->isDArray());
  assertx(!arr->isVecArrayKind() || arr->isNotDVArray());
  assertx(!RuntimeOption::EvalHackArrDVArrs || arr->isNotDVArray());
  assertx(arrprov::arrayWantsTag(arr) || !arr->hasProvenanceData());
  static_assert(ArrayData::kPackedKind == 0, "");
  // Note that m_pos < m_size is not an invariant, because an array
  // that grows will only adjust m_size to zero on the old array.

  // This loop is too slow for normal use, but can be enabled to debug
  // packed arrays.
  if (false) {
    for (uint32_t i = 0; i < arr->m_size; ++i) {
      auto const DEBUG_ONLY rval = NvGetInt(arr, i);
      assertx(type(rval) != KindOfUninit);
      assertx(tvIsPlausible(*rval));
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
MixedArray* PackedArray::ToMixedHeader(const ArrayData* old,
                                       size_t neededSize) {
  assertx(checkInvariants(old));

  auto const aux =
    MixedArrayKeys::packIntsForAux() |
    (old->isVArray() ? ArrayData::kDArray : ArrayData::kNotDVArray);

  auto const oldSize = old->m_size;
  auto const scale   = MixedArray::computeScaleFromSize(neededSize);
  auto const ad      = MixedArray::reqAlloc(scale);
  ad->m_sizeAndPos   = oldSize | int64_t{old->m_pos} << 32;
  ad->initHeader_16(HeaderKind::Mixed, OneReference, aux);
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

  auto const dstHash = ad->initHash(ad->scale());
  for (uint32_t i = 0; i < oldSize; ++i) {
    auto h = hash_int64(i);
    *ad->findForNewInsert(dstHash, mask, h) = i;
    dstData->setIntKey(i, h);
    tvCopy(GetPosVal(old, i), dstData->data);
    ++dstData;
  }
  old->m_sizeAndPos = 0;

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
  auto ad = PackedArray::ToMixedCopyReserve(old, old->m_size + 1);
  assertx(!ad->isFull());
  return ad;
}

/*
 * Convert to mixed, reserving space for at least `neededSize' elems.
 * `neededSize' must be at least old->size(), and may be equal to it.
 */
MixedArray* PackedArray::ToMixedCopyReserve(const ArrayData* old,
                                            size_t neededSize) {
  assertx(neededSize >= old->m_size);
  auto const ad      = ToMixedHeader(old, neededSize);
  auto const oldSize = old->m_size;
  auto const mask    = ad->mask();
  auto dstData       = ad->data();

  auto const dstHash = ad->initHash(ad->scale());
  for (uint32_t i = 0; i < oldSize; ++i) {
    auto const h = hash_int64(i);
    *ad->findForNewInsert(dstHash, mask, h) = i;
    dstData->setIntKey(i, h);
    tvDup(GetPosVal(old, i), dstData->data);
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
    CopyPackedHelper(adIn, ad);
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
    static_assert(PackedArray::stores_typed_values, "");
    memcpy16_inline(ad, adIn, (adIn->m_size + 1) * sizeof(TypedValue));
    ad->initHeader_16(
      adIn->m_kind,
      OneReference,
      packSizeIndexAndAuxBits(sizeIndex, adIn->auxBits())
    );

    assertx(ad->m_size == adIn->m_size);
    assertx(ad->m_pos == adIn->m_pos);
    adIn->m_sizeAndPos = 0; // old is a zombie now
  }

  ad->m_aux16 &= ~ArrayData::kHasProvenanceData;

  assertx(ad->kind() == adIn->kind());
  assertx(ad->dvArray() == adIn->dvArray());
  assertx(capacity(ad) > capacity(adIn));
  assertx(ad->hasExactlyOneRef());
  assertx(checkInvariants(ad));
  return tagArrProv(ad, adIn);
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
ALWAYS_INLINE
void PackedArray::CopyPackedHelper(const ArrayData* adIn, ArrayData* ad) {
  // Copy everything from `adIn' to `ad', including refcount, kind and cap
  auto const size = adIn->m_size;
  static_assert(sizeof(ArrayData) == 16 && sizeof(TypedValue) == 16, "");
  static_assert(PackedArray::stores_typed_values, "");
  memcpy16_inline(ad, adIn, (size + 1) * 16);
  // Clear the provenance bit if we had one set
  ad->m_aux16 &= ~ArrayData::kHasProvenanceData;

  // Copy counted types correctly
  for (uint32_t i = 0; i < size; ++i) {
    auto const elm = LvalUncheckedInt(ad, i);
    tvIncRefGen(*elm);
  }
}

NEVER_INLINE
ArrayData* PackedArray::Copy(const ArrayData* adIn) {
  assertx(checkInvariants(adIn));

  auto ad = static_cast<ArrayData*>(tl_heap->objMallocIndex(sizeClass(adIn)));

  // CopyPackedHelper will copy the header (including capacity and kind), and
  // m_sizeAndPos.  All we have to do afterwards is fix the refcount on the
  // copy.
  CopyPackedHelper(adIn, ad);
  ad->m_count = OneReference;

  assertx(ad->kind() == adIn->kind());
  assertx(ad->isLegacyArray() == adIn->isLegacyArray());
  assertx(capacity(ad) == capacity(adIn));
  assertx(ad->m_size == adIn->m_size);
  assertx(ad->m_pos == adIn->m_pos);
  assertx(ad->hasExactlyOneRef());
  assertx(checkInvariants(ad));
  return tagArrProv(ad, adIn);
}

ArrayData* PackedArray::CopyStatic(const ArrayData* adIn) {
  assertx(checkInvariants(adIn));

  auto const sizeIndex = capacityToSizeIndex(adIn->m_size);
  auto ad = alloc_packed_static(adIn);
  // CopyPackedHelper will copy the header and m_sizeAndPos. All we have to do
  // afterwards is fix the capacity and refcount on the copy; it's easiest to do
  // that by reinitializing the whole header.
  CopyPackedHelper(adIn, ad);
  ad->initHeader_16(
    adIn->m_kind,
    StaticValue,
    packSizeIndexAndAuxBits(sizeIndex, adIn->auxBits())
  );

  if (RuntimeOption::EvalArrayProvenance) {
    assertx(!ad->hasProvenanceData());
    auto const tag = arrprov::getTag(adIn);
    if (tag.valid()) {
      arrprov::setTag(ad, tag);
    }
  }

  assertx(ad->kind() == adIn->kind());
  assertx(ad->dvArray() == adIn->dvArray());
  assertx(!arrprov::arrayWantsTag(ad) ||
          arrprov::getTag(ad) == arrprov::getTag(adIn));
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
  auto ad = alloc_packed_static(arr);
  ad->initHeader_16(
    HeaderKind::Packed,
    StaticValue,
    packSizeIndexAndAuxBits(sizeIndex, arr->auxBits())
  );
  ad->m_sizeAndPos = arr->m_sizeAndPos;

  uint32_t i = 0;
  auto pos_limit = arr->iter_end();
  for (auto pos = arr->iter_begin(); pos != pos_limit;
       pos = arr->iter_advance(pos), ++i) {
    tvDup(arr->nvGetVal(pos), LvalUncheckedInt(ad, i));
  }

  if (RuntimeOption::EvalArrayProvenance) {
    assertx(!ad->hasProvenanceData());
    auto const tag = arrprov::getTag(ad);
    if (tag.valid()) {
      arrprov::setTag(ad, tag);
    }
  }

  assertx(ad->isPackedKind());
  assertx(capacity(ad) >= arr->m_size);
  assertx(ad->dvArray() == arr->dvArray());
  assertx(!arrprov::arrayWantsTag(ad) ||
          !!arrprov::getTag(ad) == !!arrprov::getTag(arr));
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
  assertx(ad->isPackedKind());
  assertx(ad->isNotDVArray());
  assertx(ad->m_size == 0);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakeReserveVArray(uint32_t capacity) {
  auto ad = MakeReserveImpl(capacity, HeaderKind::Packed, ArrayData::kVArray);
  ad->m_sizeAndPos = 0;
  assertx(ad->isPackedKind());
  assertx(ad->isVArray());
  assertx(ad->m_size == 0);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return tagArrProv(ad);
}

ArrayData* PackedArray::MakeReserveVec(uint32_t capacity) {
  auto ad =
    MakeReserveImpl(capacity, HeaderKind::VecArray, ArrayData::kNotDVArray);
  ad->m_sizeAndPos = 0;
  assertx(ad->isVecArrayKind());
  assertx(ad->m_size == 0);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return tagArrProv(ad);
}

template<bool reverse>
ALWAYS_INLINE
ArrayData* PackedArray::MakePackedImpl(uint32_t size,
                                       const TypedValue* values,
                                       HeaderKind hk,
                                       ArrayData::DVArray dv) {
  assertx(size > 0);
  auto ad = MakeReserveImpl(size, hk, dv);
  ad->m_sizeAndPos = size; // pos = 0

  // Append values by moving; this function takes ownership of them.
  if (reverse) {
    uint32_t i = size - 1;
    for (auto end = values + size; values < end; ++values, --i) {
      tvCopy(*values, LvalUncheckedInt(ad, i));
    }
  } else {
    if (debug) {
      for (uint32_t i = 0; i < size; ++i) {
        assertx(tvIsPlausible(*(values + i)));
      }
    }
    memcpy16_inline(packedData(ad), values, sizeof(TypedValue) * size);
  }

  assertx(ad->m_size == size);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakePacked(uint32_t size, const TypedValue* values) {
  // Values are in reverse order since they come from the stack, which
  // grows down.
  auto ad = MakePackedImpl<true>(size, values, HeaderKind::Packed,
                                 ArrayData::kNotDVArray);
  assertx(ad->isPackedKind());
  assertx(ad->isNotDVArray());
  return ad;
}

ArrayData* PackedArray::MakeVArray(uint32_t size, const TypedValue* values) {
  // Values are in reverse order since they come from the stack, which
  // grows down.
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto ad = MakePackedImpl<true>(size, values, HeaderKind::Packed,
                                 ArrayData::kVArray);
  assertx(ad->isPackedKind());
  assertx(ad->isVArray());
  return tagArrProv(ad);
}

ArrayData* PackedArray::MakeVec(uint32_t size, const TypedValue* values) {
  // Values are in reverse order since they come from the stack, which
  // grows down.
  auto ad = MakePackedImpl<true>(size, values, HeaderKind::VecArray,
                                 ArrayData::kNotDVArray);
  assertx(ad->isVecArrayKind());
  return tagArrProv(ad);
}

ArrayData* PackedArray::MakePackedNatural(uint32_t size, const TypedValue* values) {
  auto ad = MakePackedImpl<false>(size, values, HeaderKind::Packed,
                                  ArrayData::kNotDVArray);
  assertx(ad->isPackedKind());
  assertx(ad->isNotDVArray());
  return ad;
}

ArrayData* PackedArray::MakeVArrayNatural(uint32_t size, const TypedValue* values) {
  if (RuntimeOption::EvalHackArrDVArrs) return MakeVecNatural(size, values);

  auto ad = MakePackedImpl<false>(size, values, HeaderKind::Packed,
                                  ArrayData::kVArray);
  assertx(ad->isPackedKind());
  assertx(ad->isVArray());
  return tagArrProv(ad);
}

ArrayData* PackedArray::MakeVecNatural(uint32_t size, const TypedValue* values) {
  auto ad = MakePackedImpl<false>(size, values, HeaderKind::VecArray,
                                  ArrayData::kNotDVArray);
  assertx(ad->isVecArrayKind());
  return tagArrProv(ad);
}

ArrayData* PackedArray::MakeUninitialized(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::Packed, ArrayData::kNotDVArray);
  ad->m_sizeAndPos = size; // pos = 0
  assertx(ad->isPackedKind());
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
  assertx(ad->isPackedKind());
  assertx(ad->isVArray());
  assertx(ad->m_size == size);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return tagArrProv(ad);
}

ArrayData* PackedArray::MakeUninitializedVec(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::VecArray, ArrayData::kNotDVArray);
  ad->m_sizeAndPos = size; // pos = 0
  assertx(ad->isVecArrayKind());
  assertx(ad->m_size == size);
  assertx(ad->m_pos == 0);
  assertx(checkInvariants(ad));
  return tagArrProv(ad);
}

ArrayData* PackedArray::MakeVecFromAPC(const APCArray* apc) {
  assertx(apc->isVec());
  auto const apcSize = apc->size();
  VecArrayInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.append(apc->getValue(i)->toLocal());
  }
  auto const ad = init.create();
  return tagArrProv(ad, apc);
}

ArrayData* PackedArray::MakeVArrayFromAPC(const APCArray* apc) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  assertx(apc->isVArray());
  auto const apcSize = apc->size();
  VArrayInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.append(apc->getValue(i)->toLocal());
  }
  auto const ad = init.create();
  return tagArrProv(ad, apc);
}

void PackedArray::Release(ArrayData* ad) {
  ad->fixCountForRelease();
  assertx(checkInvariants(ad));
  assertx(ad->isRefCounted());
  assertx(ad->hasExactlyOneRef());

  if (RuntimeOption::EvalArrayProvenance) {
    arrprov::clearTag(ad);
  }
  for (uint32_t i = 0; i < ad->m_size; ++i) {
    tvDecRefGen(LvalUncheckedInt(ad, i));
  }
  tl_heap->objFreeIndex(ad, sizeClass(ad));
  AARCH64_WALKABLE_FRAME();
}

NEVER_INLINE
void PackedArray::ReleaseUncounted(ArrayData* ad) {
  assertx(checkInvariants(ad));
  if (!ad->uncountedDecRef()) return;

  if (RuntimeOption::EvalArrayProvenance) {
    arrprov::clearTag(ad);
  }
  for (uint32_t i = 0; i < ad->m_size; ++i) {
    ReleaseUncountedTv(LvalUncheckedInt(ad, i));
  }

  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().removeAPCUncountedBlock();
  }

  static_assert(PackedArray::stores_typed_values, "");
  auto const extra = uncountedAllocExtra(ad, ad->hasApcTv());
  auto const allocSize = extra + sizeof(PackedArray) +
                         ad->m_size * sizeof(TypedValue);
  uncounted_sized_free(reinterpret_cast<char*>(ad) - extra, allocSize);
}

////////////////////////////////////////////////////////////////////////////////

tv_rval PackedArray::NvGetInt(const ArrayData* ad, int64_t k) {
  assertx(checkInvariants(ad));
  return LIKELY(size_t(k) < ad->m_size) ? &packedData(ad)[k] : nullptr;
}

tv_rval
PackedArray::NvGetStr(const ArrayData* ad, const StringData* /*s*/) {
  assertx(checkInvariants(ad));
  return nullptr;
}

ssize_t PackedArray::NvGetIntPos(const ArrayData* ad, int64_t k) {
  assertx(checkInvariants(ad));
  return LIKELY(size_t(k) < ad->m_size) ? k : ad->m_size;
}

ssize_t PackedArray::NvGetStrPos(const ArrayData* ad, const StringData* k) {
  assertx(checkInvariants(ad));
  return ad->m_size;
}

TypedValue PackedArray::GetPosKey(const ArrayData* ad, ssize_t pos) {
  assertx(checkInvariants(ad));
  assertx(pos != ad->m_size);
  return make_tv<KindOfInt64>(pos);
}

TypedValue PackedArray::GetPosVal(const ArrayData* ad, ssize_t pos) {
  assertx(checkInvariants(ad));
  assertx(pos < ad->m_size);
  return *LvalUncheckedInt(const_cast<ArrayData*>(ad), pos);
}

size_t PackedArray::Vsize(const ArrayData*) {
  // PackedArray always has a valid m_size so it's an error to get here.
  always_assert(false);
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
  assertx(adIn->isPackedKind());

  if (LIKELY(size_t(k) < adIn->getSize())) {
    auto const ad = copy ? PackedArray::Copy(adIn) : adIn;
    return found(ad);
  }

  if (size_t(k) == adIn->getSize()) {
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatCheckImplicitVarrayAppend) &&
        adIn->isVArray()) {
      raise_hackarr_compat_notice("Implicit append to varray");
    }
    return append();
  }

  if (UNLIKELY(RuntimeOption::EvalHackArrCompatCheckVarrayPromote) &&
      adIn->isVArray()) {
    raise_hackarr_compat_notice(
      folly::sformat("varray promoting to darray: out of bounds key {}", k)
    );
  }

  auto const mixed = copy ? PackedArray::ToMixedCopy(adIn)
                          : PackedArray::ToMixed(adIn);
  return promoted(mixed);
}

template <typename PromotedFn>
auto MutableOpStr(ArrayData* adIn, StringData* /*k*/, bool copy,
                  PromotedFn promoted) {
  assertx(PackedArray::checkInvariants(adIn));
  assertx(adIn->isPackedKind());

  if (UNLIKELY(RuntimeOption::EvalHackArrCompatCheckVarrayPromote) &&
      adIn->isVArray()) {
    raise_hackarr_compat_notice(
      "varray promoting to darray: invalid key: expected int, got string"
    );
  }

  auto const mixed = copy ? PackedArray::ToMixedCopy(adIn)
                          : PackedArray::ToMixed(adIn);
  return promoted(mixed);
}

template<typename FoundFn>
auto MutableOpIntVec(ArrayData* adIn, int64_t k, bool copy, FoundFn found) {
  assertx(PackedArray::checkInvariants(adIn));
  assertx(adIn->isVecArrayKind());

  if (UNLIKELY(size_t(k) >= adIn->getSize())) {
    throwOOBArrayKeyException(k, adIn);
  }
  auto const ad = copy ? PackedArray::Copy(adIn) : adIn;
  return found(ad);
}

}

arr_lval PackedArray::LvalInt(ArrayData* adIn, int64_t k, bool copy) {
  return MutableOpInt(adIn, k, copy,
    [&] (ArrayData* ad) { return arr_lval { ad, LvalUncheckedInt(ad, k) }; },
    []() -> arr_lval { throwMissingElementException("Lval"); },
    // TODO(#2606310): Make use of our knowledge that the key is missing.
    [&] (MixedArray* mixed) { return mixed->addLvalImpl<true>(k); }
  );
}

arr_lval PackedArray::LvalIntVec(ArrayData* adIn, int64_t k, bool copy) {
  return MutableOpIntVec(adIn, k, copy,
    [&] (ArrayData* ad) { return arr_lval { ad, LvalUncheckedInt(ad, k) }; }
  );
}

tv_lval PackedArray::LvalUncheckedInt(ArrayData* ad, int64_t k) {
  // NOTE: We cannot check that k is less than the array's length here, because
  // the vector extension allocates the array and uses this method to fill it.
  assertx(size_t(k) < PackedArray::capacity(ad));
  return &packedData(ad)[k];
}

arr_lval PackedArray::LvalStr(ArrayData* adIn, StringData* k, bool copy) {
  return MutableOpStr(adIn, k, copy,
    // TODO(#2606310): Make use of our knowledge that the key is missing.
    [&] (MixedArray* mixed) { return mixed->addLvalImpl<true>(k); }
  );
}

arr_lval
PackedArray::LvalStrVec(ArrayData* adIn, StringData* key, bool) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArrayKind());
  throwInvalidArrayKeyException(key, adIn);
}

tv_lval PackedArray::LvalNewInPlace(ArrayData* ad) {
  assertx(checkInvariants(ad));
  assertx(ad->m_size < capacity(ad));
  assertx(!ad->cowCheck());
  auto const lval = LvalUncheckedInt(ad, ad->m_size++);
  type(lval) = KindOfNull;
  return lval;
}

ArrayData* PackedArray::SetInt(ArrayData* adIn, int64_t k, TypedValue v) {
  auto const copy = adIn->cowCheck();
  return MutableOpInt(adIn, k, copy,
    [&] (ArrayData* ad) { setElem(LvalUncheckedInt(ad, k), v); return ad; },
    [&] { return AppendImpl(adIn, v, copy); },
    [&] (MixedArray* mixed) { return mixed->addVal(k, v); }
  );
}

ArrayData* PackedArray::SetIntMove(ArrayData* adIn, int64_t k, TypedValue v) {
  auto done = false;
  auto const copy = adIn->cowCheck();
  auto const result = MutableOpInt(adIn, k, copy,
    [&] (ArrayData* ad) {
      assertx((adIn != ad) == copy);
      if (copy && adIn->decReleaseCheck()) PackedArray::Release(adIn);
      setElem(LvalUncheckedInt(ad, k), v, true);
      done = true;
      return ad;
    },
    [&] { return AppendImpl(adIn, v, copy); },
    [&] (MixedArray* mixed) { return mixed->addVal(k, v); }
  );
  if (done) return result;
  if (adIn != result && adIn->decReleaseCheck()) PackedArray::Release(adIn);
  tvDecRefGen(v);
  return result;
}

ArrayData* PackedArray::SetIntVec(ArrayData* adIn, int64_t k, TypedValue v) {
  assertx(adIn->cowCheck() || adIn->notCyclic(v));
  return MutableOpIntVec(adIn, k, adIn->cowCheck(),
    [&] (ArrayData* ad) { setElem(LvalUncheckedInt(ad, k), v); return ad; }
  );
}

ArrayData* PackedArray::SetIntMoveVec(ArrayData* adIn, int64_t k, TypedValue v) {
  assertx(adIn->cowCheck() || adIn->notCyclic(v));
  auto const copy = adIn->cowCheck();
  return MutableOpIntVec(adIn, k, copy,
    [&] (ArrayData* ad) {
      assertx((adIn != ad) == copy);
      if (copy && adIn->decReleaseCheck()) PackedArray::Release(adIn);
      setElem(LvalUncheckedInt(ad, k), v, true);
      return ad;
    }
  );
}

ArrayData* PackedArray::SetStr(ArrayData* adIn, StringData* k, TypedValue v) {
  return MutableOpStr(adIn, k, adIn->cowCheck(),
    [&] (MixedArray* mixed) { return mixed->addVal(k, v); }
  );
}

ArrayData* PackedArray::SetStrMove(ArrayData* adIn, StringData* k, TypedValue v) {
  auto const result = SetStr(adIn, k, v);
  assertx(result != adIn);
  if (adIn->decReleaseCheck()) PackedArray::Release(adIn);
  tvDecRefGen(v);
  return result;
}

ArrayData* PackedArray::SetStrVec(ArrayData* adIn, StringData* k, TypedValue v) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArrayKind());
  throwInvalidArrayKeyException(k, adIn);
}

///////////////////////////////////////////////////////////////////////////////

ArrayData* PackedArray::RemoveImpl(ArrayData* adIn, int64_t k, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isPackedKind());
  if (size_t(k) < adIn->m_size) {
    // Escalate to mixed for correctness; unset preserves m_nextKI.
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatCheckVarrayPromote) &&
        adIn->isVArray()) {
      raise_hackarr_compat_notice("varray promoting to darray: removing key");
    }
    //
    // TODO(#2606310): if we're removing the /last/ element, we
    // probably could stay packed, but this needs to be verified.
    auto const mixed = copy ? ToMixedCopy(adIn) : ToMixed(adIn);
    return MixedArray::RemoveInt(mixed, k);
  }
  // Key doesn't exist---we're still packed.
  return copy ? Copy(adIn) : adIn;
}

ArrayData* PackedArray::RemoveInt(ArrayData* adIn, int64_t k) {
  return RemoveImpl(adIn, k, adIn->cowCheck());
}

ArrayData*
PackedArray::RemoveImplVec(ArrayData* adIn, int64_t k, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArrayKind());

  // You're only allowed to remove an element at the end of the vec (or beyond,
  // which is a no-op).
  if (UNLIKELY(size_t(k) >= adIn->m_size)) return adIn;
  if (LIKELY(size_t(k) + 1 == adIn->m_size)) {
    auto const ad = copy ? Copy(adIn) : adIn;
    auto const size = ad->m_size - 1;
    ad->m_sizeAndPos = size; // pos = 0
    tvDecRefGen(LvalUncheckedInt(ad, size));
    return ad;
  }
  throwVecUnsetException();
}

ArrayData* PackedArray::RemoveIntVec(ArrayData* adIn, int64_t k) {
  return RemoveImplVec(adIn, k, adIn->cowCheck());
}

ArrayData*
PackedArray::RemoveStr(ArrayData* adIn, const StringData*) {
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

ArrayData* PackedArray::AppendImpl(ArrayData* adIn, TypedValue v, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(v.m_type != KindOfUninit);
  assertx(copy || adIn->notCyclic(v));
  auto const ad = PrepareForInsert(adIn, copy);
  tvDup(v, LvalUncheckedInt(ad, ad->m_size++));
  return ad;
}

ArrayData* PackedArray::Append(ArrayData* adIn, TypedValue v) {
  return AppendImpl(adIn, v, adIn->cowCheck());
}

ArrayData* PackedArray::AppendInPlace(ArrayData* adIn, TypedValue v) {
  assertx(!adIn->cowCheck());
  return AppendImpl(adIn, v, false);
}

ArrayData* PackedArray::PlusEq(ArrayData* adIn, const ArrayData* elems) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isPackedKind());
  if (!elems->isPHPArrayType()) throwInvalidAdditionException(elems);
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
  assertx(adIn->isVecArrayKind());
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

  auto const size = ad->m_size - 1;
  auto const tv = *LvalUncheckedInt(ad, size);
  value = tvAsCVarRef(&tv);
  ad->m_sizeAndPos = size; // pos = 0
  tvDecRefGen(tv);
  return ad;
}

ArrayData* PackedArray::Dequeue(ArrayData* adIn, Variant& value) {
  assertx(checkInvariants(adIn));

  auto const ad = adIn->cowCheck() ? Copy(adIn) : adIn;
  if (UNLIKELY(ad->m_size == 0)) {
    value = uninit_null();
    return ad;
  }

  // This is O(N), but so is Dequeue on a mixed array, because it
  // needs to renumber keys.  So it makes sense to stay packed.
  auto const size = ad->m_size - 1;
  auto const data = packedData(ad);
  value = std::move(tvAsVariant(data)); // no incref+decref
  std::memmove(data, data + 1, size * sizeof *data);
  ad->m_sizeAndPos = size; // pos = 0
  return ad;
}

ArrayData* PackedArray::Prepend(ArrayData* adIn, TypedValue v) {
  assertx(checkInvariants(adIn));

  auto const ad = PrepareForInsert(adIn, adIn->cowCheck());
  auto const size = ad->m_size;
  auto const data = packedData(ad);
  std::memmove(data + 1, data, sizeof *data * size);
  tvDup(v, data[0]);
  ad->m_size = size + 1;
  ad->m_pos = 0;
  return ad;
}

ArrayData* PackedArray::ToPHPArray(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isPackedKind());
  if (adIn->isNotDVArray()) return adIn;
  assertx(adIn->isVArray());
  if (adIn->getSize() == 0) return ArrayData::Create();
  ArrayData* ad = copy ? Copy(adIn) : adIn;
  ad->setDVArray(ArrayData::kNotDVArray);
  if (RO::EvalArrayProvenance) arrprov::clearTag(ad);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ToVArray(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isPackedKind());
  if (RuntimeOption::EvalHackArrDVArrs) return ToVec(adIn, copy);
  if (adIn->isVArray()) return adIn;
  if (adIn->getSize() == 0) return ArrayData::CreateVArray();
  ArrayData* ad = copy ? Copy(adIn) : adIn;
  ad->setDVArray(ArrayData::kVArray);
  if (RO::EvalArrayProvenance) arrprov::reassignTag(ad);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ToDArray(ArrayData* adIn, bool /*copy*/) {
  assertx(checkInvariants(adIn));

  auto const size = adIn->getSize();
  if (size == 0) return ArrayData::CreateDArray();

  DArrayInit init{size};
  for (int64_t i = 0; i < size; ++i) init.add(i, GetPosVal(adIn, i));
  return init.create();
}

ArrayData* PackedArray::ToPHPArrayVec(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArrayKind());
  if (adIn->empty()) return ArrayData::Create();
  ArrayData* ad = copy ? Copy(adIn) : adIn;
  ad->m_kind = HeaderKind::Packed;
  assertx(ad->isNotDVArray());
  if (RO::EvalArrayProvenance) arrprov::clearTag(ad);
  ad->setLegacyArray(false);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ToVArrayVec(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isVecArrayKind());
  if (RuntimeOption::EvalHackArrDVArrs) return adIn;
  if (adIn->getSize() == 0) return ArrayData::CreateVArray();
  ArrayData* ad = copy ? Copy(adIn) : adIn;
  ad->m_kind = HeaderKind::Packed;
  ad->setLegacyArray(false);
  ad->setDVArray(ArrayData::kVArray);
  if (RO::EvalArrayProvenance) arrprov::reassignTag(ad);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ToDict(ArrayData* ad, bool copy) {
  assertx(checkInvariants(ad));
  assertx(ad->isPackedKind());

  if (ad->empty()) return ArrayData::CreateDict();

  auto const mixed = copy ? ToMixedCopy(ad) : ToMixed(ad);
  return MixedArray::ToDictInPlace(mixed);
}

ArrayData* PackedArray::ToDictVec(ArrayData* ad, bool copy) {
  assertx(checkInvariants(ad));
  assertx(ad->isVecArrayKind());
  if (ad->empty()) return ArrayData::CreateDict();
  auto mixed = copy ? ToMixedCopy(ad) : ToMixed(ad);
  return MixedArray::ToDictInPlace(mixed);
}

ArrayData* PackedArray::ToVec(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  assertx(adIn->isPackedKind());

  if (adIn->empty()) return ArrayData::CreateVec();

  ArrayData* ad;
  if (copy) {
    // CopyPackedHelper will copy the header and m_sizeAndPos. All we have to do
    // afterwards is fix the kind and refcount in the copy; it's easiest to do
    // that by reinitializing the whole header.
    ad = static_cast<ArrayData*>(tl_heap->objMallocIndex(sizeClass(adIn)));
    CopyPackedHelper(adIn, ad);
    ad->initHeader_16(
      HeaderKind::VecArray,
      OneReference,
      packSizeIndexAndAuxBits(sizeClass(adIn), ArrayData::kNotDVArray)
    );
  } else {
    adIn->m_kind = HeaderKind::VecArray;
    adIn->setDVArray(ArrayData::kNotDVArray);
    ad = adIn;
  }
  if (RO::EvalArrayProvenance) arrprov::reassignTag(ad);

  assertx(ad->isVecArrayKind());
  assertx(capacity(ad) == capacity(adIn));
  assertx(ad->m_size == adIn->m_size);
  assertx(ad->m_pos == adIn->m_pos);
  assertx(ad->hasExactlyOneRef());
  assertx(checkInvariants(ad));
  return tagArrProv(ad);
}

ArrayData* PackedArray::ToVecVec(ArrayData* ad, bool) {
  assertx(checkInvariants(ad));
  assertx(ad->isVecArrayKind());
  return ad;
}

void PackedArray::OnSetEvalScalar(ArrayData* ad) {
  assertx(checkInvariants(ad));
  auto const size = ad->m_size;
  for (uint32_t i = 0; i < size; ++i) {
    auto lval = LvalUncheckedInt(ad, i);
    auto tv = *lval;
    tvAsVariant(&tv).setEvalScalar();
    tvCopy(tv, lval);
  }
}

void PackedArray::Ksort(ArrayData* ad, int /*flags*/, bool ascending) {
  assertx(ascending ||
          (ad->getSize() <= 1 && !(ad->isVecArrayKind() || ad->isVArray())));
}

void PackedArray::Asort(ArrayData* ad, int, bool) {
  assertx(ad->getSize() <= 1 && !(ad->isVecArrayKind() || ad->isVArray()));
}

bool PackedArray::Uksort(ArrayData* ad, const Variant&) {
  assertx(ad->getSize() <= 1 && !(ad->isVecArrayKind() || ad->isVArray()));
  return true;
}

bool PackedArray::Uasort(ArrayData* ad, const Variant&) {
  assertx(ad->getSize() <= 1 && !(ad->isVecArrayKind() || ad->isVArray()));
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

  auto const extra = uncountedAllocExtra(array, withApcTypedValue);
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

  ad->m_aux16 &= ~ArrayData::kHasProvenanceData;

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
  return tagArrProv(ad, array);
}

ALWAYS_INLINE
bool PackedArray::VecEqualHelper(const ArrayData* ad1, const ArrayData* ad2,
                                 bool strict) {
  assertx(checkInvariants(ad1));
  assertx(checkInvariants(ad2));
  assertx(ad1->isVecArrayKind());
  assertx(ad2->isVecArrayKind());

  if (ad1 == ad2) return true;
  if (ad1->m_size != ad2->m_size) return false;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  auto const size = ad1->m_size;
  for (uint32_t i = 0; i < size; ++i) {
    auto const elm1 = GetPosVal(ad1, i);
    auto const elm2 = GetPosVal(ad2, i);
    auto const cmp = strict ? tvSame(elm1, elm2) : tvEqual(elm1, elm2);
    if (!cmp) return false;
  }

  return true;
}

ALWAYS_INLINE
int64_t PackedArray::VecCmpHelper(const ArrayData* ad1, const ArrayData* ad2) {
  assertx(checkInvariants(ad1));
  assertx(checkInvariants(ad2));
  assertx(ad1->isVecArrayKind());
  assertx(ad2->isVecArrayKind());

  auto const size1 = ad1->m_size;
  auto const size2 = ad2->m_size;

  if (size1 < size2) return -1;
  if (size1 > size2) return 1;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  for (uint32_t i = 0; i < size1; ++i) {
    auto const cmp = tvCompare(GetPosVal(ad1, i), GetPosVal(ad2, i));
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
