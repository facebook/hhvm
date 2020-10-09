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

std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyVec;
std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyVArray;
std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyMarkedVec;
std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyMarkedVArray;

struct PackedArray::VecInitializer {
  VecInitializer() {
    auto const aux = packSizeIndexAndAuxBits(0, 0);
    auto const ad = reinterpret_cast<ArrayData*>(&s_theEmptyVec);
    ad->m_size = 0;
    ad->m_extra = 0;
    ad->initHeader_16(HeaderKind::Vec, StaticValue, aux);
    assertx(checkInvariants(ad));
  }
};
PackedArray::VecInitializer PackedArray::s_vec_initializer;

struct PackedArray::VArrayInitializer {
  VArrayInitializer() {
    auto const aux = packSizeIndexAndAuxBits(0, 0);
    auto const ad = reinterpret_cast<ArrayData*>(&s_theEmptyVArray);
    ad->m_size = 0;
    ad->m_extra = 0;
    ad->initHeader_16(HeaderKind::Packed, StaticValue, aux);
    assertx(RuntimeOption::EvalHackArrDVArrs || checkInvariants(ad));
  }
};
PackedArray::VArrayInitializer PackedArray::s_varr_initializer;

struct PackedArray::MarkedVecInitializer {
  MarkedVecInitializer() {
    auto const aux = packSizeIndexAndAuxBits(0, ArrayData::kLegacyArray);
    auto const ad = reinterpret_cast<ArrayData*>(&s_theEmptyMarkedVec);
    ad->m_size = 0;
    ad->m_extra = 0;
    ad->initHeader_16(HeaderKind::Vec, StaticValue, aux);
    assertx(!RuntimeOption::EvalHackArrDVArrs || checkInvariants(ad));
  }
};
PackedArray::MarkedVecInitializer PackedArray::s_marked_vec_initializer;

struct PackedArray::MarkedVArrayInitializer {
  MarkedVArrayInitializer() {
    auto const aux = packSizeIndexAndAuxBits(0, ArrayData::kLegacyArray);
    auto const ad = reinterpret_cast<ArrayData*>(&s_theEmptyMarkedVArray);
    ad->m_size = 0;
    ad->m_extra = 0;
    ad->initHeader_16(HeaderKind::Packed, StaticValue, aux);
    assertx(RuntimeOption::EvalHackArrDVArrs || checkInvariants(ad));
  }
};
PackedArray::MarkedVArrayInitializer PackedArray::s_marked_varr_initializer;

//////////////////////////////////////////////////////////////////////

namespace {

inline ArrayData* alloc_packed_static(const ArrayData* ad) {
  auto const size = sizeof(ArrayData) + ad->size() * sizeof(TypedValue);
  auto const ret = RuntimeOption::EvalLowStaticArrays
    ? low_malloc(size)
    : uncounted_malloc(size);
  return reinterpret_cast<ArrayData*>(reinterpret_cast<char*>(ret));
}

}

bool PackedArray::checkInvariants(const ArrayData* arr) {
  assertx(arr->hasVanillaPackedLayout());
  assertx(arr->checkCountZ());
  assertx(arr->m_size <= MixedArray::MaxSize);
  assertx(arr->m_size <= capacity(arr));
  assertx(IMPLIES(arr->isVArray(), arr->isPackedKind()));
  assertx(IMPLIES(arr->isNotDVArray(), arr->isVecKind()));
  assertx(IMPLIES(arr->isLegacyArray(), arr->isHAMSafeVArray()));
  assertx(!RO::EvalHackArrDVArrs || arr->isVecKind());
  assertx(IMPLIES(!arrprov::arrayWantsTag(arr),
                  arr->m_extra == 0 &&
                  IMPLIES(RO::EvalArrayProvenance,
                          !arrprov::getTag(arr).valid())));

  // This loop is too slow for normal use, but can be enabled to debug
  // packed arrays.
  if (false) {
    for (uint32_t i = 0; i < arr->m_size; ++i) {
      auto const DEBUG_ONLY tv = NvGetInt(arr, i);
      assertx(tv.is_init());
      assertx(tvIsPlausible(tv));
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
MixedArray* PackedArray::ToMixedHeader(const ArrayData* old,
                                       size_t neededSize) {
  assertx(checkInvariants(old));

  auto const oldSize = old->m_size;
  auto const scale   = MixedArray::computeScaleFromSize(neededSize);
  auto const ad      = MixedArray::reqAlloc(scale);
  auto const kind    = old->isVArray() ? HeaderKind::Mixed : HeaderKind::Dict;
  ad->initHeader_16(kind, OneReference, MixedArrayKeys::packIntsForAux());
  ad->m_size         = oldSize;
  ad->m_extra        = old->m_extra;
  ad->m_scale_used   = scale | uint64_t{oldSize} << 32; // used=oldSize
  ad->m_nextKI       = oldSize;

  assertx(ad->m_size == oldSize);
  assertx(ad->hasVanillaMixedLayout());
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
  old->m_size = 0;

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
    // CopyPackedHelper will copy the header and m_size; since we pass
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
  } else {
    // Copy everything from `adIn' to `ad', including header and m_size
    static_assert(sizeof(ArrayData) == 16 && sizeof(TypedValue) == 16, "");
    static_assert(PackedArray::stores_typed_values, "");
    memcpy16_inline(ad, adIn, (adIn->m_size + 1) * sizeof(TypedValue));
    ad->initHeader_16(
      adIn->m_kind,
      OneReference,
      packSizeIndexAndAuxBits(sizeIndex, adIn->auxBits())
    );

    assertx(ad->m_size == adIn->m_size);
    adIn->m_size = 0; // old is a zombie now
  }

  assertx(ad->kind() == adIn->kind());
  assertx(ArrayData::dvArrayEqual(ad, adIn));
  assertx(capacity(ad) > capacity(adIn));
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_extra == adIn->m_extra);
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
 * (capacity, kind, and refcount) and m_size. It then increfs the
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
  // m_size.  All we have to do afterwards is fix the refcount on the copy.
  CopyPackedHelper(adIn, ad);
  ad->m_count = OneReference;

  assertx(ad->kind() == adIn->kind());
  assertx(ad->isLegacyArray() == adIn->isLegacyArray());
  assertx(capacity(ad) == capacity(adIn));
  assertx(ad->m_size == adIn->m_size);
  assertx(ad->m_extra == adIn->m_extra);
  assertx(ad->hasExactlyOneRef());
  assertx(checkInvariants(ad));
  return tagArrProv(ad, adIn);
}

ArrayData* PackedArray::CopyStatic(const ArrayData* adIn) {
  assertx(checkInvariants(adIn));

  auto const sizeIndex = capacityToSizeIndex(adIn->m_size);
  auto ad = alloc_packed_static(adIn);
  // CopyPackedHelper will copy the header and m_size. All we have to do
  // afterwards is fix the capacity and refcount on the copy; it's easiest to do
  // that by reinitializing the whole header.
  CopyPackedHelper(adIn, ad);
  ad->initHeader_16(
    adIn->m_kind,
    StaticValue,
    packSizeIndexAndAuxBits(sizeIndex, adIn->auxBits())
  );

  assertx(ad->kind() == adIn->kind());
  assertx(ArrayData::dvArrayEqual(ad, adIn));
  assertx(!arrprov::arrayWantsTag(ad) ||
          arrprov::getTag(ad) == arrprov::getTag(adIn));
  assertx(capacity(ad) >= adIn->m_size);
  assertx(ad->m_size == adIn->m_size);
  assertx(ad->m_extra == adIn->m_extra);
  assertx(ad->isStatic());
  assertx(checkInvariants(ad));
  return ad;
}

/* This helper allocates an ArrayData and initializes the header (including
 * capacity, kind, and refcount). The caller is responsible for initializing
 * m_size, and initializing array entries (if any).
 */
ALWAYS_INLINE
ArrayData* PackedArray::MakeReserveImpl(uint32_t cap, HeaderKind hk) {
  auto const sizeIndex = capacityToSizeIndex(cap);
  auto ad = static_cast<ArrayData*>(tl_heap->objMallocIndex(sizeIndex));
  ad->initHeader_16(hk, OneReference, packSizeIndexAndAuxBits(sizeIndex, 0));

  assertx(ad->m_kind == hk);
  assertx(capacity(ad) >= cap);
  assertx(ad->hasExactlyOneRef());
  return ad;
}

ArrayData* PackedArray::MakeReserveVArray(uint32_t capacity) {
  if (RuntimeOption::EvalHackArrDVArrs) {
    auto const ad =  MakeReserveVec(capacity);
    ad->setLegacyArrayInPlace(RuntimeOption::EvalHackArrDVArrMark);
    return ad;
  }

  auto ad = MakeReserveImpl(capacity, HeaderKind::Packed);
  ad->m_size = 0;
  ad->m_extra = 0;
  assertx(ad->isPackedKind());
  assertx(ad->isVArray());
  assertx(ad->m_size == 0);
  assertx(checkInvariants(ad));
  return tagArrProv(ad);
}

ArrayData* PackedArray::MakeReserveVec(uint32_t capacity) {
  auto ad = MakeReserveImpl(capacity, HeaderKind::Vec);
  ad->m_size = 0;
  ad->m_extra = 0;
  assertx(ad->isVecKind());
  assertx(ad->m_size == 0);
  assertx(checkInvariants(ad));
  return ad;
}

template<bool reverse>
ALWAYS_INLINE
ArrayData* PackedArray::MakePackedImpl(uint32_t size,
                                       const TypedValue* values,
                                       HeaderKind hk) {
  assertx(size > 0);
  auto ad = MakeReserveImpl(size, hk);
  ad->m_size = size;
  ad->m_extra = 0;

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
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakeVArray(uint32_t size, const TypedValue* values) {
  // Values are in reverse order since they come from the stack, which
  // grows down.
  if (RuntimeOption::EvalHackArrDVArrs) {
    auto const ad = MakeVec(size, values);
    ad->setLegacyArrayInPlace(RuntimeOption::EvalHackArrDVArrMark);
    return ad;
  }
  auto ad = MakePackedImpl<true>(size, values, HeaderKind::Packed);
  assertx(ad->isPackedKind());
  assertx(ad->isVArray());
  return tagArrProv(ad);
}

ArrayData* PackedArray::MakeVec(uint32_t size, const TypedValue* values) {
  // Values are in reverse order since they come from the stack, which
  // grows down.
  auto ad = MakePackedImpl<true>(size, values, HeaderKind::Vec);
  assertx(ad->isVecKind());
  return ad;
}

ArrayData* PackedArray::MakeVArrayNatural(uint32_t size, const TypedValue* values) {
  if (RuntimeOption::EvalHackArrDVArrs) {
    auto const ad = MakeVecNatural(size, values);
    ad->setLegacyArrayInPlace(RuntimeOption::EvalHackArrDVArrMark);
    return ad;
  }

  auto ad = MakePackedImpl<false>(size, values, HeaderKind::Packed);
  assertx(ad->isPackedKind());
  assertx(ad->isVArray());
  return tagArrProv(ad);
}

ArrayData* PackedArray::MakeVecNatural(uint32_t size, const TypedValue* values) {
  auto ad = MakePackedImpl<false>(size, values, HeaderKind::Vec);
  assertx(ad->isVecKind());
  return ad;
}

ArrayData* PackedArray::MakeUninitializedVArray(uint32_t size) {
  if (RuntimeOption::EvalHackArrDVArrs) {
    auto const ad = MakeUninitializedVec(size);
    ad->setLegacyArrayInPlace(RuntimeOption::EvalHackArrDVArrMark);
    return ad;
  }
  auto ad = MakeReserveImpl(size, HeaderKind::Packed);
  ad->m_size = size;
  ad->m_extra = 0;
  assertx(ad->isPackedKind());
  assertx(ad->isVArray());
  assertx(ad->m_size == size);
  assertx(checkInvariants(ad));
  return tagArrProv(ad);
}

ArrayData* PackedArray::MakeUninitializedVec(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::Vec);
  ad->m_size = size;
  ad->m_extra = 0;
  assertx(ad->isVecKind());
  assertx(ad->m_size == size);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::MakeVecFromAPC(const APCArray* apc, bool isLegacy) {
  assertx(apc->isPacked());
  auto const apcSize = apc->size();
  VecInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.append(apc->getValue(i)->toLocal());
  }
  auto const ad = init.create();
  ad->setLegacyArrayInPlace(isLegacy);
  return ad;
}

ArrayData* PackedArray::MakeVArrayFromAPC(const APCArray* apc, bool isMarked) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  assertx(apc->isVArray());
  auto const apcSize = apc->size();
  VArrayInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.append(apc->getValue(i)->toLocal());
  }
  auto const ad = init.create();
  ad->setLegacyArrayInPlace(isMarked);
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

TypedValue PackedArray::NvGetInt(const ArrayData* ad, int64_t k) {
  assertx(checkInvariants(ad));
  return LIKELY(size_t(k) < ad->m_size) ? packedData(ad)[k]
                                        : make_tv<KindOfUninit>();
}

TypedValue PackedArray::NvGetStr(const ArrayData* ad, const StringData* /*s*/) {
  assertx(checkInvariants(ad));
  return make_tv<KindOfUninit>();
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

template<typename FoundFn>
auto MutableOpInt(ArrayData* adIn, int64_t k, bool copy, FoundFn found) {
  assertx(PackedArray::checkInvariants(adIn));
  if (UNLIKELY(size_t(k) >= adIn->size())) {
    throwOOBArrayKeyException(k, adIn);
  }
  auto const ad = copy ? PackedArray::Copy(adIn) : adIn;
  return found(ad);
}

}

arr_lval PackedArray::LvalInt(ArrayData* adIn, int64_t k) {
  assertx(checkInvariants(adIn));
  return MutableOpInt(adIn, k, adIn->cowCheck(),
    [&] (ArrayData* ad) { return arr_lval { ad, LvalUncheckedInt(ad, k) }; }
  );
}

tv_lval PackedArray::LvalUncheckedInt(ArrayData* ad, int64_t k) {
  // NOTE: We cannot check that k is less than the array's length here, because
  // the vector extension allocates the array and uses this method to fill it.
  assertx(size_t(k) < PackedArray::capacity(ad));
  return &packedData(ad)[k];
}

arr_lval PackedArray::LvalStr(ArrayData* adIn, StringData* key) {
  assertx(checkInvariants(adIn));
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
  assertx(adIn->cowCheck() || adIn->notCyclic(v));
  return MutableOpInt(adIn, k, adIn->cowCheck(),
    [&] (ArrayData* ad) { setElem(LvalUncheckedInt(ad, k), v); return ad; }
  );
}

ArrayData* PackedArray::SetIntMove(ArrayData* adIn, int64_t k, TypedValue v) {
  assertx(adIn->cowCheck() || adIn->notCyclic(v));
  auto const copy = adIn->cowCheck();
  return MutableOpInt(adIn, k, copy,
    [&] (ArrayData* ad) {
      assertx((adIn != ad) == copy);
      if (copy && adIn->decReleaseCheck()) PackedArray::Release(adIn);
      setElem(LvalUncheckedInt(ad, k), v, true);
      return ad;
    }
  );
}

ArrayData* PackedArray::SetStr(ArrayData* adIn, StringData* k, TypedValue v) {
  assertx(checkInvariants(adIn));
  throwInvalidArrayKeyException(k, adIn);
}

///////////////////////////////////////////////////////////////////////////////

ArrayData* PackedArray::RemoveInt(ArrayData* adIn, int64_t k) {
  assertx(checkInvariants(adIn));

  // You're only allowed to remove an element at the end of the varray or
  // vec (or beyond the end, which is a no-op).
  if (UNLIKELY(size_t(k) >= adIn->m_size)) return adIn;
  if (LIKELY(size_t(k) + 1 == adIn->m_size)) {
    auto const ad = adIn->cowCheck() ? Copy(adIn) : adIn;
    auto const size = ad->m_size - 1;
    ad->m_size = size;
    tvDecRefGen(LvalUncheckedInt(ad, size));
    return ad;
  }

  if (adIn->isVecKind()) {
    throwVecUnsetException();
  } else {
    throwVarrayUnsetException();
  }
}

ArrayData* PackedArray::RemoveStr(ArrayData* adIn, const StringData*) {
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

ArrayData* PackedArray::AppendImpl(ArrayData* adIn, TypedValue v, bool copy, bool move) {
  assertx(checkInvariants(adIn));
  assertx(v.m_type != KindOfUninit);
  assertx(copy || adIn->notCyclic(v));
  auto const ad = PrepareForInsert(adIn, copy);
  if (move) {
    tvCopy(v, LvalUncheckedInt(ad, ad->m_size++));
  } else {
    tvDup(v, LvalUncheckedInt(ad, ad->m_size++));
  }
  return ad;
}

ArrayData* PackedArray::Append(ArrayData* adIn, TypedValue v) {
  return AppendImpl(adIn, v, adIn->cowCheck());
}

ArrayData* PackedArray::AppendMove(ArrayData* adIn, TypedValue v) {
  return AppendImpl(adIn, v, adIn->cowCheck(), true);
}

ArrayData* PackedArray::AppendInPlace(ArrayData* adIn, TypedValue v) {
  assertx(!adIn->cowCheck());
  return AppendImpl(adIn, v, false);
}

ArrayData* PackedArray::Pop(ArrayData* adIn, Variant& value) {
  assertx(checkInvariants(adIn));

  auto const ad = adIn->cowCheck() ? Copy(adIn) : adIn;

  if (UNLIKELY(ad->m_size == 0)) {
    value = uninit_null();
    return ad;
  }

  auto const size = ad->m_size - 1;
  auto const tv = *LvalUncheckedInt(ad, size);
  value = tvAsCVarRef(&tv);
  ad->m_size = size;
  tvDecRefGen(tv);
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
  return ad;
}

ArrayData* PackedArray::ToDVArray(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  if (adIn->empty()) return ArrayData::CreateVArray();
  auto const ad = copy ? Copy(adIn) : adIn;
  ad->m_kind = HeaderKind::Packed;
  if (RO::EvalArrayProvenance) arrprov::reassignTag(ad);
  assertx(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::ToHackArr(ArrayData* adIn, bool copy) {
  assertx(checkInvariants(adIn));
  if (adIn->empty()) return ArrayData::CreateVec();
  auto const ad = copy ? Copy(adIn) : adIn;
  ad->m_kind = HeaderKind::Vec;
  ad->setLegacyArrayInPlace(false);
  if (RO::EvalArrayProvenance) arrprov::clearTag(ad);
  assertx(checkInvariants(ad));
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
  assertx(ascending);
  assertx(ad->isVecKind() || ad->isVArray());
}

void PackedArray::Asort(ArrayData* ad, int, bool) {
  always_assert(false);
}

bool PackedArray::Uksort(ArrayData* ad, const Variant&) {
  always_assert(false);
}

bool PackedArray::Uasort(ArrayData* ad, const Variant&) {
  always_assert(false);
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
  ad->m_size = array->m_size;
  ad->m_extra = array->m_extra;

  // Do a raw copy without worrying about refcounts, and convert the values to
  // uncounted later.
  auto src = packedData(array);
  auto dst = packedData(ad);
  memcpy16_inline(dst, src, sizeof(TypedValue) * size);
  for (auto end = dst + size; dst < end; ++dst) {
    ConvertTvToUncounted(dst, seen);
  }

  assertx(ad->kind() == array->kind());
  assertx(ArrayData::dvArrayEqual(ad, array));
  assertx(capacity(ad) >= size);
  assertx(ad->m_size == size);
  assertx(ad->m_extra == array->m_extra);
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
  assertx(ad1->isVecKind());
  assertx(ad2->isVecKind());

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

//////////////////////////////////////////////////////////////////////

}
