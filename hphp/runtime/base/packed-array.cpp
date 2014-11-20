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
#include "hphp/runtime/base/packed-array.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "folly/Likely.h"

#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/runtime-error.h"

#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/array-iterator-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

bool PackedArray::checkInvariants(const ArrayData* arr) {
  assert(arr->isPacked());
  assert(arr->m_size <= packedCodeToCap(arr->m_packedCapCode));
  assert(arr->m_pos >= 0 && arr->m_pos <= arr->m_size);
  static_assert(ArrayData::kPackedKind == 0, "");
  // Note that m_pos < m_size is not an invariant, because an array
  // that grows will only adjust m_size to zero on the old array.

  // This loop is too slow for normal use, but can be enabled to debug
  // packed arrays.
  if (false) {
    auto ptr = reinterpret_cast<const TypedValue*>(arr + 1);
    auto const stop = ptr + arr->m_size;
    for (; ptr != stop; ptr++) {
      assert(ptr->m_type != KindOfUninit);
      assert(tvIsPlausible(*ptr));
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
  auto const cmret   = computeCapAndMask(neededSize);
  auto const cap     = cmret.first;
  auto const mask    = cmret.second;
  auto const ad      = smartAllocArray(cap, mask);

  ad->m_sizeAndPos       = oldSize | int64_t{old->m_pos} << 32;
  ad->m_kindAndCount     = ArrayData::kMixedKind << 24; // capcode=0, count=0
  ad->m_capAndUsed       = uint64_t{oldSize} << 32 | cap; // used=oldSize
  ad->m_tableMask        = mask;
  ad->m_nextKI           = oldSize;

  assert(ad->m_kind == ArrayData::kMixedKind);
  assert(ad->m_size == oldSize);
  assert(ad->m_pos == old->m_pos);
  assert(ad->m_count == 0);
  assert(ad->m_used == oldSize);
  assert(ad->m_cap == cap);
  assert(ad->m_tableMask == mask);
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
  auto const mask    = ad->m_tableMask;
  auto dstData       = ad->data();
  auto dstHash       = ad->hashTab();
  auto const srcData = packedData(old);

  auto i = uint32_t{0};
  for (; i < oldSize; ++i) {
    dstData->setIntKey(i);
    tvCopy(srcData[i], dstData->data);
    *dstHash = i;
    ++dstData;
    ++dstHash;
  }
  for (; i <= mask; ++i) {
    *dstHash++ = MixedArray::Empty;
  }

  old->m_size = 0;
  if (debug) {
    // For debug builds, set m_pos to 0 as well to make the
    // asserts in checkInvariants() happy.
    old->m_pos = 0;
  }

  assert(ad->checkInvariants());
  assert(!ad->isFull());
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
  auto dstData       = ad->data();
  auto dstHash       = ad->hashTab();
  auto const srcData = packedData(old);

  auto i = uint32_t{0};
  for (; i < oldSize; ++i) {
    dstData->setIntKey(i);
    tvDupFlattenVars(&srcData[i], &dstData->data, old);
    *dstHash = i;
    ++dstData;
    ++dstHash;
  }
  auto const mask = ad->m_tableMask;
  for (; i <= mask; ++i) {
    *dstHash++ = MixedArray::Empty;
  }

  assert(ad->checkInvariants());
  assert(!ad->isFull());
  return ad;
}

/*
 * Convert to mixed, reserving space for at least `neededSize' elems.
 * The `neededSize' should include old->size(), but may be equal to
 * it.
 *
 * Unlike the other ToMixed functions, the returned array already has
 * a reference count of 1.
 */
MixedArray* PackedArray::ToMixedCopyReserve(const ArrayData* old,
                                           size_t neededSize) {
  assert(neededSize >= old->m_size);
  auto const ad      = ToMixedHeader(old, neededSize);
  ad->m_count = 1;
  auto const oldSize = old->m_size;
  auto const mask    = ad->m_tableMask;
  auto dstData       = ad->data();
  auto dstHash       = ad->hashTab();
  auto const srcData = packedData(old);

  auto i = uint32_t{0};
  for (; i < oldSize; ++i) {
    dstData->setIntKey(i);
    tvDupFlattenVars(&srcData[i], &dstData->data, old);
    *dstHash = i;
    ++dstData;
    ++dstHash;
  }
  for (; i <= mask; ++i) {
    *dstHash++ = MixedArray::Empty;
  }

  assert(ad->checkInvariants());
  return ad;
}

NEVER_INLINE
ArrayData* PackedArray::Grow(ArrayData* old) {
  assert(checkInvariants(old));
  assert(old->m_size == packedCodeToCap(old->m_packedCapCode));
  DEBUG_ONLY auto const oldPos = old->m_pos;

  ArrayData* ad;
  uint32_t oldCapCode = old->m_packedCapCode & 0xFFFFFFul;
  auto cap = oldCapCode * 2;
  if (LIKELY(cap <= kPackedCapCodeThreshold)) {
    assert(oldCapCode == packedCodeToCap(old->m_packedCapCode));
    // We add 1 to the cap, to make it use up all the memory to be allocated, if
    // the original cap has been maximized.
    if (auto capUpdated = getMaxCapInPlaceFast(++cap)) {
      cap = capUpdated;
    }
    ad = static_cast<ArrayData*>(
      MM().objMallocLogged(sizeof(ArrayData) + cap * sizeof(TypedValue))
    );
    assert(cap == packedCodeToCap(cap));
    ad->m_sizeAndPos = old->m_sizeAndPos;
    ad->m_kindAndCount = cap | uint64_t{old->m_kind} << 24; // count=0
    assert(ad->isPacked());
    assert(ad->m_size == old->m_size);
    assert(packedCodeToCap(ad->m_packedCapCode) == cap);
  } else {
    ad = GrowHelper(old);
  }

  if (UNLIKELY(strong_iterators_exist())) {
    move_strong_iterators(ad, old);
  }

  // Steal the old array payload.  At the time of this writing, it was
  // better not to reuse the memcpy return value here because gcc had
  // `ad' in a callee saved register anyway.  The reg-to-reg move was
  // smaller than subtracting sizeof(ArrayData) from rax to return.
  auto const oldSize = old->m_size;
  old->m_size = 0;
  if (debug) {
    // For debug builds, set m_pos to 0 as well to make the
    // asserts in checkInvariants() happy.
    old->m_pos = 0;
  }
  std::memcpy(packedData(ad), packedData(old), oldSize * sizeof(TypedValue));

  // TODO(#2926276): it would be good to refactor callers to expect
  // our refcount to start at 1.

  assert(ad->m_pos == oldPos);
  assert(ad->m_count == 0);
  assert(checkInvariants(ad));
  return ad;
}

NEVER_INLINE
ArrayData* PackedArray::GrowHelper(ArrayData* old) {
  auto const oldCap = packedCodeToCap(old->m_packedCapCode);
  static_assert(kMaxPackedCap >= MixedArray::MaxSize, "");
  if (UNLIKELY(oldCap > MixedArray::MaxSize / 2)) return nullptr;
  auto cap = roundUpPackedCap(oldCap * 2);
  assert(cap > kPackedCapCodeThreshold);
  ArrayData* ad = MixedArray::MakeReserveSlow(cap); // pos=count=size=kind=0
  if (UNLIKELY(ad == nullptr)) return nullptr;
  // ad->m_packedCapCode is already set correctly in MakeReserveSlow
  // Assuming VPackedArray and PackedArray are the same in implementation
  ad->m_sizeAndPos = old->m_sizeAndPos;
  ad->m_kind = old->m_kind;
  assert(ad->isPacked());
  assert(ad->m_size == old->m_size);
  assert(packedCodeToCap(ad->m_packedCapCode) == cap);
  return ad;
}

/*
 * Get the maximum possible capacity without reallocation. Return 0 if we don't
 * have a quick way to get a better cap. Currently this only works for caps
 * within kPackedCapCodeThreshold. It should be pretty fast.
 */
uint32_t PackedArray::getMaxCapInPlaceFast(uint32_t cap) {
  if (UNLIKELY(cap > kPackedCapCodeThreshold)) {
    return 0;
  }
  static_assert(sizeof(TypedValue) == 16, "sizeof TypedValue changed?");
  static_assert(sizeof(ArrayData) == 16, "sizeof ArrayData changed?");
  assert((cap + 1) * 16U <= kMaxSmartSize);
  uint32_t newCap = (MemoryManager::smartSizeClass((cap + 1) << 4) >> 4) - 1;
  if (UNLIKELY(newCap > kPackedCapCodeThreshold)) {
    newCap &= ~0xFFU;
  }
  assert(newCap >= cap);
  return newCap > cap ? newCap : 0;
}

NEVER_INLINE
ArrayData* PackedArray::CopyAndResizeIfNeededSlow(const ArrayData* adIn) {
  assert(adIn->m_size == packedCodeToCap(adIn->m_packedCapCode));
  // Note: this path will have to handle splitting strong iterators
  // later when we combine copy & grow into one operation.
  // For now I'm just making use of copyPacked to do it for me before
  // GrowPacked happens.
  auto const copy = PackedArray::Copy(adIn);
  auto const ret  = PackedArray::Grow(copy);
  assert(ret != copy);
  assert(copy->getCount() == 0);
  PackedArray::Release(copy);
  return ret;
}

ALWAYS_INLINE
ArrayData* PackedArray::CopyAndResizeIfNeeded(const ArrayData* adIn) {
  if (LIKELY(sizeLessThanPackedCapCode(adIn->m_size, adIn->m_packedCapCode))) {
    return Copy(adIn);
  }
  return CopyAndResizeIfNeededSlow(adIn);
}

ALWAYS_INLINE
ArrayData* PackedArray::ResizeIfNeeded(ArrayData* adIn) {
  if (LIKELY(sizeLessThanPackedCapCode(adIn->m_size, adIn->m_packedCapCode))) {
    return adIn;
  }
  return Grow(adIn);
}

//////////////////////////////////////////////////////////////////////

NEVER_INLINE
ArrayData* PackedArray::Copy(const ArrayData* adIn) {
  assert(checkInvariants(adIn));

  auto const cap = packedCodeToCap(adIn->m_packedCapCode);

  auto const ad = static_cast<ArrayData*>(
    MM().objMallocLogged(sizeof(ArrayData) + cap * sizeof(TypedValue))
  );
  auto const size = adIn->m_size;
  auto const capCode = (adIn->m_packedCapCode & 0xFFFFFFul);
  ad->m_sizeAndPos = adIn->m_sizeAndPos;
  ad->m_kindAndCount = uint64_t{adIn->m_kind} << 24 | capCode; // count=0

  auto const srcData = packedData(adIn);
  auto const stop    = srcData + size;
  auto targetData    = reinterpret_cast<TypedValue*>(ad + 1);
  for (auto ptr = srcData; ptr != stop; ++ptr, ++targetData) {
    tvDupFlattenVars(ptr, targetData, adIn);
  }

  assert(ad->isPacked());
  assert(packedCodeToCap(ad->m_packedCapCode) == cap);
  assert(ad->m_size == size);
  assert(ad->m_pos == adIn->m_pos);
  assert(ad->m_count == 0);
  assert(checkInvariants(ad));
  return ad;
}

ArrayData* PackedArray::CopyWithStrongIterators(const ArrayData* ad) {
  auto const cpy = Copy(ad);
  if (LIKELY(strong_iterators_exist())) {
    // This returns its first argument just so we can tail call it.
    return move_strong_iterators(cpy, const_cast<ArrayData*>(ad));
  }
  return cpy;
}

ArrayData* PackedArray::NonSmartCopy(const ArrayData* adIn) {
  assert(checkInvariants(adIn));

  ArrayData* ad;
  if (LIKELY(adIn->m_size <= kPackedCapCodeThreshold)) {
    // There's no reason to use the full capacity, since non-smart
    // arrays are not mutable.
    auto const cap = adIn->m_size;
    ad = static_cast<ArrayData*>(
      std::malloc(sizeof(ArrayData) + cap * sizeof(TypedValue))
    );
    assert(cap == packedCodeToCap(cap));
    ad->m_sizeAndPos = adIn->m_sizeAndPos;
    ad->m_kindAndCount = cap; // kind=0, count=0
    assert(ad->isPacked());
    assert(packedCodeToCap(ad->m_packedCapCode) == cap);
    assert(ad->m_size == adIn->m_size);
  } else {
    ad = NonSmartCopyHelper(adIn);
  }

  auto const srcData = packedData(adIn);
  auto const size    = adIn->m_size;
  auto const stop    = srcData + size;
  auto targetData    = reinterpret_cast<TypedValue*>(ad + 1);
  for (auto ptr = srcData; ptr != stop; ++ptr, ++targetData) {
    tvDupFlattenVars(ptr, targetData, adIn);
  }

  assert(ad->m_pos == adIn->m_pos);
  assert(ad->m_count == 0);
  assert(checkInvariants(ad));
  return ad;
}

NEVER_INLINE
ArrayData* PackedArray::NonSmartCopyHelper(const ArrayData* adIn) {
  auto const cap = roundUpPackedCap(adIn->m_size);
  auto const ad = static_cast<ArrayData*>(
    std::malloc(sizeof(ArrayData) + cap * sizeof(TypedValue))
  );
  auto const capCode = packedCapToCode(cap);
  ad->m_sizeAndPos = adIn->m_sizeAndPos;
  ad->m_kindAndCount = capCode; // kind=0, count=0
  assert(ad->isPacked());
  assert(packedCodeToCap(ad->m_packedCapCode) == cap);
  assert(ad->m_size == adIn->m_size);
  return ad;
}

ArrayData* PackedArray::NonSmartConvert(const ArrayData* arr) {
  assert(arr->isVectorData());

  ArrayData* ad;
  if (LIKELY(arr->m_size <= kPackedCapCodeThreshold)) {
    auto const cap = arr->m_size;
    ad = static_cast<ArrayData*>(
      std::malloc(sizeof(ArrayData) + cap * sizeof(TypedValue))
    );
    assert(cap == packedCodeToCap(cap));
    ad->m_sizeAndPos = arr->m_sizeAndPos;
    ad->m_kindAndCount = cap; // kind=0, cap=0
    assert(ad->isPacked());
    assert(packedCodeToCap(ad->m_packedCapCode) == cap);
    assert(ad->m_size == arr->m_size);
  } else {
    ad = NonSmartConvertHelper(arr);
  }
  auto data = reinterpret_cast<TypedValue*>(ad + 1);
  auto pos_limit = arr->iter_end();
  for (auto pos = arr->iter_begin(); pos != pos_limit;
       pos = arr->iter_advance(pos), ++data) {
    tvDupFlattenVars(arr->getValueRef(pos).asTypedValue(), data, arr);
  }

  assert(ad->m_pos == arr->m_pos);
  assert(ad->m_count == 0);
  assert(checkInvariants(ad));
  return ad;
}

NEVER_INLINE
ArrayData* PackedArray::NonSmartConvertHelper(const ArrayData* arr) {
  auto const cap = roundUpPackedCap(arr->m_size);
  auto const ad = static_cast<ArrayData*>(
    std::malloc(sizeof(ArrayData) + cap * sizeof(TypedValue))
  );
  auto const capCode = packedCapToCode(cap);
  ad->m_sizeAndPos = arr->m_sizeAndPos;
  ad->m_kindAndCount = capCode; // kind=0, count=0
  assert(ad->isPacked());
  assert(packedCodeToCap(ad->m_packedCapCode) == cap);
  assert(ad->m_size == arr->m_size);
  return ad;
}

//////////////////////////////////////////////////////////////////////

void PackedArray::downgradeAndWarn(ArrayData* ad, const Reason r) {
  assert(ad->isVPackedArray());
  PackedArray::warnUsage(r);
  ad->m_kind = ArrayData::kPackedKind;
}

void PackedArray::warnUsage(const Reason r) {
  if (!RuntimeOption::EvalHackArrayWarnFrequency) {
    return;
  }
  static __thread uint32_t numWarnings = 0;
  numWarnings++;
  if (numWarnings % RuntimeOption::EvalHackArrayWarnFrequency != 0) {
    return;
  }
  switch (r) {
  case Reason::kForeachByRef:
    raise_warning("Foreach by reference over a varray, converting to array");
    break;
  case Reason::kTakeByRef:
    raise_warning("Taking an element by reference from a varray, converting "
                  "to array");
    break;
  case Reason::kSetRef:
    raise_warning("Adding a reference to a varray, converting to array");
    break;
  case Reason::kAppendRef:
    raise_warning("Appending a reference to a varray, converting to array");
    break;
  case Reason::kOutOfOrderIntKey:
    raise_warning("Adding out-of-order integer key to a varray, converting "
                  "array");
    break;
  case Reason::kRemoveInt:
    raise_warning("Removing an integer key from a varray, converting to array");
    break;
  case Reason::kGetStr:
    raise_warning("Trying to read a string key from a varray");
    break;
  case Reason::kSetStr:
    raise_warning("Adding a string key to a varray, converting to array");
    break;
  case Reason::kRemoveStr:
    raise_warning("Trying to remove a string key from a varray");
    break;
  case Reason::kNumericString:
    raise_warning("Using an integer-like string key with a varray");
    break;
  case Reason::kPlusNotSupported:
    raise_warning("+ operator not supported for varray, converting to array");
    break;
  case Reason::kMergeNotSupported:
    raise_warning("array_merge() not supported for varray, converting to "
                  "array");
    break;
  case Reason::kSortNotSupported:
    raise_warning("sort builtins are not supported for varray at present, "
                  "converting to array");
    break;
  }
}

ArrayData* MixedArray::MakeReserve(uint32_t capacity) {
  ArrayData* ad;
  if (LIKELY(capacity <= kPackedCapCodeThreshold)) {
    auto const kSmallSize = MixedArray::SmallSize;
    auto const cap = std::max(capacity, kSmallSize);
    ad = static_cast<ArrayData*>(
      MM().objMallocLogged(sizeof(ArrayData) + sizeof(TypedValue) * cap)
    );
    assert(cap == packedCodeToCap(cap));
    ad->m_sizeAndPos = 0; // size=0, pos=0
    ad->m_kindAndCount = cap | uint64_t{1} << 32; // kind=0, count=1
    assert(ad->isPacked());
    assert(packedCodeToCap(ad->m_packedCapCode) == cap);
    assert(ad->m_size == 0);
  } else {
    ad = MakeReserveSlow(capacity); // size=pos=kind=count=0
    ad->m_count = 1;
  }

  assert(ad->m_count == 1);
  assert(ad->m_pos == 0);
  assert(PackedArray::checkInvariants(ad));
  return ad;
}

NEVER_INLINE
ArrayData* MixedArray::MakeReserveSlow(uint32_t capacity) {
  auto const cap = roundUpPackedCap(capacity);
  auto const requestSize = sizeof(ArrayData) + sizeof(TypedValue) * cap;
  auto const ad = static_cast<ArrayData*>(MM().objMallocLogged(requestSize));
  auto const capCode = packedCapToCode(cap);
  ad->m_sizeAndPos = 0;
  ad->m_kindAndCount = capCode; // kind=0, count=0
  assert(ad->isPacked());
  assert(packedCodeToCap(ad->m_packedCapCode) == cap);
  assert(ad->m_size == 0);
  return ad;
}

ArrayData* MixedArray::MakeReserveVArray(uint32_t capacity) {
  ArrayData* ad;
  if (LIKELY(capacity <= kPackedCapCodeThreshold)) {
    auto const kSmallSize = MixedArray::SmallSize;
    auto const cap = std::max(capacity, kSmallSize);
    ad = static_cast<ArrayData*>(
      MM().objMallocLogged(sizeof(ArrayData) + sizeof(TypedValue) * cap)
    );
    assert(cap == packedCodeToCap(cap));
    ad->m_sizeAndPos = 0; // size=0, pos=0
    ad->m_kindAndCount = uint64_t{kVPackedKind} << 24 | cap |
                         uint64_t{1} << 32; // count=1
    assert(ad->isPacked());
    assert(packedCodeToCap(ad->m_packedCapCode) == cap);
    assert(ad->m_size == 0);
  } else {
    ad = MakeReserveVArraySlow(capacity);
  }

  assert(ad->m_count == 1);
  assert(ad->m_pos == 0);
  assert(PackedArray::checkInvariants(ad));
  return ad;
}

NEVER_INLINE
ArrayData* MixedArray::MakeReserveVArraySlow(uint32_t capacity) {
  // Avoid code duplication.
  auto ad = MakeReserveSlow(capacity); // pos,count,size,kind=0
  ad->m_kind = kVPackedKind;
  ad->m_count = 1;
  return ad;
}

NEVER_INLINE
void PackedArray::Release(ArrayData* ad) {
  assert(checkInvariants(ad));
  assert(ad->isRefCounted());

  auto const size = ad->m_size;
  auto const data = packedData(ad);
  auto const stop = data + size;
  for (auto ptr = data; ptr != stop; ++ptr) {
    tvRefcountedDecRef(*ptr);
  }
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(ad);
  }
  MM().objFreeLogged(ad, heapSize(ad));
}

const TypedValue* PackedArray::NvGetInt(const ArrayData* ad, int64_t ki) {
  auto const data = packedData(ad);
  return LIKELY(size_t(ki) < ad->m_size) ? &data[ki] : nullptr;
}

const TypedValue*
PackedArray::NvGetIntConverted(const ArrayData* ad, int64_t ki) {
  assert(ad->isVPackedArray());
  PackedArray::warnUsage(Reason::kNumericString);
  return NvGetInt(ad, ki);
}

const TypedValue*
PackedArray::NvGetStr(const ArrayData* ad, const StringData* s) {
  if (ad->m_kind == ArrayData::kVPackedKind) {
    PackedArray::warnUsage(PackedArray::Reason::kGetStr);
  }
  return nullptr;
}

void PackedArray::NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
  assert(checkInvariants(ad));
  assert(pos != ad->m_size);
  out->m_data.num = pos;
  out->m_type = KindOfInt64;
}

size_t PackedArray::Vsize(const ArrayData*) { not_reached(); }

const Variant& PackedArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  assert(checkInvariants(ad));
  assert(pos != ad->m_size);
  return tvAsCVarRef(&packedData(ad)[pos]);
}

bool PackedArray::ExistsInt(const ArrayData* ad, int64_t k) {
  assert(checkInvariants(ad));
  return size_t(k) < ad->m_size;
}

bool PackedArray::ExistsStr(const ArrayData* ad, const StringData* s) {
  if (ad->m_kind == ArrayData::kVPackedKind) {
    PackedArray::warnUsage(PackedArray::Reason::kGetStr);
  }
  return false;
}

ArrayData* PackedArray::LvalInt(ArrayData* adIn,
                                int64_t k,
                                Variant*& ret,
                                bool copy) {
  assert(checkInvariants(adIn));

  if (LIKELY(size_t(k) < adIn->m_size)) {
    auto const ad = copy ? Copy(adIn) : adIn;
    ret = &tvAsVariant(&packedData(ad)[k]);
    return ad;
  }

  // We can stay packed if the index is m_size, and the operation does
  // the same thing as LvalNew.
  if (size_t(k) == adIn->m_size) return LvalNew(adIn, ret, copy);

  if (adIn->m_kind == ArrayData::kVPackedKind) {
    PackedArray::warnUsage(PackedArray::Reason::kOutOfOrderIntKey);
  }

  // Promote-to-mixed path, we know the key is new and should be using
  // findForNewInsert but aren't yet TODO(#2606310).
  auto const mixed = copy ? ToMixedCopy(adIn) : ToMixed(adIn);
  return mixed->addLvalImpl(k, ret);
}

ArrayData* PackedArray::LvalStr(ArrayData* adIn,
                                StringData* key,
                                Variant*& ret,
                                bool copy) {
  if (adIn->m_kind == ArrayData::kVPackedKind) {
    PackedArray::warnUsage(PackedArray::Reason::kSetStr);
  }

  // We have to promote.  We know the key doesn't exist, but aren't
  // making use of that fact yet.  TODO(#2606310).
  auto const mixed = copy ? ToMixedCopy(adIn) : ToMixed(adIn);
  return mixed->addLvalImpl(key, ret);
}

ArrayData* PackedArray::LvalNew(ArrayData* adIn, Variant*& ret, bool copy) {
  assert(checkInvariants(adIn));
  auto const ad = copy ? CopyAndResizeIfNeeded(adIn)
                       : ResizeIfNeeded(adIn);
  auto& tv = packedData(ad)[ad->m_size++];
  tv.m_type = KindOfNull;
  ret = &tvAsVariant(&tv);
  return ad;
}

ArrayData* PackedArray::LvalNewRef(ArrayData* adIn, Variant*& ret, bool copy) {
  assert(checkInvariants(adIn));
  auto const ad = copy ? CopyAndResizeIfNeeded(adIn)
                       : ResizeIfNeeded(adIn);
  if (ad->m_kind == ArrayData::kVPackedKind) {
    PackedArray::downgradeAndWarn(ad, PackedArray::Reason::kTakeByRef);
  }
  auto& tv = packedData(ad)[ad->m_size++];
  tv.m_type = KindOfNull;
  ret = &tvAsVariant(&tv);
  return ad;
}

ArrayData*
PackedArray::SetInt(ArrayData* adIn, int64_t k, Cell v, bool copy) {
  assert(checkInvariants(adIn));

  // Right now SetInt is used for the AddInt entry point also. This
  // first branch is the only thing we'd be able to omit if we were
  // doing AddInt.
  if (size_t(k) < adIn->m_size) {
    auto const ad = copy ? Copy(adIn) : adIn;
    auto& dst = *tvToCell(&packedData(ad)[k]);
    cellSet(v, dst);
    // TODO(#3888164): we should restructure things so we don't have to
    // check KindOfUninit here.
    if (UNLIKELY(dst.m_type == KindOfUninit)) {
      dst.m_type = KindOfNull;
    }
    return ad;
  }

  // Setting the int at the size of the array can keep it in packed
  // mode---it's the same as an append.
  if (size_t(k) == adIn->m_size) return Append(adIn, tvAsCVarRef(&v), copy);

  if (adIn->m_kind == ArrayData::kVPackedKind) {
    PackedArray::warnUsage(PackedArray::Reason::kOutOfOrderIntKey);
  }

  // On the promote-to-mixed path, we can use addVal since we know the
  // key can't exist.
  auto const mixed = copy ? ToMixedCopy(adIn) : ToMixed(adIn);
  return mixed->addVal(k, v);
}

ArrayData*
PackedArray::SetIntConverted(ArrayData* adIn, int64_t k, Cell v, bool copy) {
  assert(adIn->isVPackedArray());
  PackedArray::warnUsage(Reason::kNumericString);
  return PackedArray::SetInt(adIn, k, v, copy);
}

ArrayData* PackedArray::SetStr(ArrayData* adIn,
                               StringData* k,
                               Cell v,
                               bool copy) {
  // We must convert to mixed, but can call addVal since the key must
  // not exist.
  if (adIn->m_kind == ArrayData::kVPackedKind) {
    PackedArray::warnUsage(PackedArray::Reason::kSetStr);
  }
  auto const mixed = copy ? ToMixedCopy(adIn) : ToMixed(adIn);
  return mixed->addVal(k, v);
}

ArrayData* PackedArray::SetRefInt(ArrayData* adIn,
                                  int64_t k,
                                  Variant& v,
                                  bool copy) {
  if (adIn->m_kind == ArrayData::kVPackedKind) {
    PackedArray::downgradeAndWarn(adIn, PackedArray::Reason::kSetRef);
  }

  assert(checkInvariants(adIn));

  if (size_t(k) == adIn->m_size) return AppendRef(adIn, v, copy);
  if (size_t(k) < adIn->m_size) {
    auto const ad = copy ? Copy(adIn) : adIn;
    tvBind(v.asRef(), &packedData(ad)[k]);
    return ad;
  }

  // todo t2606310: key can't exist.  use add/findForNewInsert
  auto const mixed = copy ? ToMixedCopy(adIn) : ToMixed(adIn);
  mixed->updateRef(k, v);
  return mixed;
}

ArrayData* PackedArray::SetRefStr(ArrayData* adIn,
                                  StringData* k,
                                  Variant& v,
                                  bool copy) {
  if (adIn->m_kind == ArrayData::kVPackedKind) {
    PackedArray::warnUsage(PackedArray::Reason::kSetRef);
  }

  auto const mixed = copy ? ToMixedCopy(adIn) : ToMixed(adIn);
  // todo t2606310: key can't exist.  use add/findForNewInsert
  return mixed->updateRef(k, v);
}

ArrayData* PackedArray::RemoveInt(ArrayData* adIn, int64_t k, bool copy) {
  assert(checkInvariants(adIn));
  if (adIn->m_kind == ArrayData::kVPackedKind) {
    PackedArray::warnUsage(PackedArray::Reason::kRemoveInt);
  }
  if (size_t(k) < adIn->m_size) {
    // Escalate to mixed for correctness; unset preserves m_nextKI.
    //
    // TODO(#2606310): if we're removing the /last/ element, we
    // probably could stay packed, but this needs to be verified.
    auto const mixed = copy ? ToMixedCopy(adIn) : ToMixed(adIn);
    auto pos = mixed->findForRemove(k, false);
    if (validPos(pos)) mixed->erase(pos);
    return mixed;
  }
  // Key doesn't exist---we're still packed.
  return copy ? Copy(adIn) : adIn;
}

ArrayData*
PackedArray::RemoveStr(ArrayData* adIn, const StringData*, bool) {
  assert(checkInvariants(adIn));
  if (adIn->m_kind == ArrayData::kVPackedKind) {
    PackedArray::warnUsage(PackedArray::Reason::kRemoveStr);
  }
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
  if (ad->isVPackedArray()) {
    PackedArray::downgradeAndWarn(ad, Reason::kForeachByRef);
  }
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

ArrayData* PackedArray::Append(ArrayData* adIn, const Variant& v, bool copy) {
  assert(checkInvariants(adIn));
  auto const ad = copy ? CopyAndResizeIfNeeded(adIn)
                       : ResizeIfNeeded(adIn);
  auto& dst = packedData(ad)[ad->m_size++];
  cellDup(*v.asCell(), dst);
  // TODO(#3888164): restructure this so we don't need KindOfUninit checks.
  if (dst.m_type == KindOfUninit) dst.m_type = KindOfNull;
  return ad;
}

ArrayData* PackedArray::AppendRef(ArrayData* adIn,
                                  Variant& v,
                                  bool copy) {
  if (adIn->m_kind == ArrayData::kVPackedKind) {
    PackedArray::downgradeAndWarn(adIn, PackedArray::Reason::kSetRef);
  }

  assert(checkInvariants(adIn));
  auto const ad = copy ? CopyAndResizeIfNeeded(adIn)
                       : ResizeIfNeeded(adIn);
  auto& dst = packedData(ad)[ad->m_size++];
  dst.m_data.pref = v.asRef()->m_data.pref;
  dst.m_type = KindOfRef;
  dst.m_data.pref->incRefCount();
  return ad;
}

ArrayData* PackedArray::AppendWithRef(ArrayData* adIn,
                                      const Variant& v,
                                      bool copy) {
  if (adIn->m_kind == ArrayData::kVPackedKind && v.isReferenced()) {
    PackedArray::downgradeAndWarn(adIn, PackedArray::Reason::kSetRef);
  }

  assert(checkInvariants(adIn));
  auto const ad = copy ? CopyAndResizeIfNeeded(adIn)
                       : ResizeIfNeeded(adIn);
  auto& dst = packedData(ad)[ad->m_size++];
  dst.m_type = KindOfNull;
  tvAsVariant(&dst).setWithRef(v);
  return ad;
}

ArrayData* PackedArray::PlusEq(ArrayData* adIn, const ArrayData* elems) {
  assert(checkInvariants(adIn));
  auto const neededSize = adIn->size() + elems->size();
  if (adIn->m_kind == ArrayData::kVPackedKind) {
    PackedArray::warnUsage(PackedArray::Reason::kPlusNotSupported);
  }
  auto const mixed = ToMixedCopyReserve(adIn, neededSize);
  try {
    auto const ret = MixedArray::PlusEq(mixed, elems);
    assert(ret == mixed);
    assert(!mixed->hasMultipleRefs());
    return ret;
  } catch (...) {
    MixedArray::Release(mixed);
    throw;
  }
}

ArrayData* PackedArray::Merge(ArrayData* adIn, const ArrayData* elems) {
  assert(checkInvariants(adIn));
  if (adIn->m_kind == ArrayData::kVPackedKind) {
    PackedArray::warnUsage(PackedArray::Reason::kMergeNotSupported);
  }
  auto const neededSize = adIn->m_size + elems->size();
  auto const ret = ToMixedCopyReserve(adIn, neededSize);
  return MixedArray::ArrayMergeGeneric(ret, elems);
}

static void adjustMArrayIter(ArrayData* ad, ssize_t pos) {
  assert(ad->isPacked());
  for_each_strong_iterator([&] (MIterTable::Ent& miEnt) {
    if (miEnt.array != ad) return;
    auto const iter = miEnt.iter;
    if (iter->getResetFlag()) return;
    if (iter->m_pos == pos) {
      if (pos <= 0) {
        iter->m_pos = ad->getSize();
        iter->setResetFlag(true);
      } else {
        iter->m_pos = pos - 1;
      }
    }
  });
}

ArrayData* PackedArray::Pop(ArrayData* adIn, Variant& value) {
  assert(checkInvariants(adIn));

  auto const ad = adIn->hasMultipleRefs() ? Copy(adIn) : adIn;

  if (UNLIKELY(ad->m_size == 0)) {
    assert(ad->m_pos == 0);
    value = uninit_null();
    return ad;
  }

  auto const oldSize = ad->m_size;
  auto& tv = packedData(ad)[oldSize - 1];
  value = tvAsCVarRef(&tv);
  if (UNLIKELY(strong_iterators_exist())) {
    adjustMArrayIter(ad, oldSize - 1);
  }
  auto const oldType = tv.m_type;
  auto const oldDatum = tv.m_data.num;
  ad->m_size = oldSize - 1;
  ad->m_pos = 0;
  tvRefcountedDecRefHelper(oldType, oldDatum);
  return ad;
}

ArrayData* PackedArray::Dequeue(ArrayData* adIn, Variant& value) {
  assert(checkInvariants(adIn));

  auto const ad = adIn->hasMultipleRefs() ? Copy(adIn) : adIn;
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

ArrayData* PackedArray::Prepend(ArrayData* adIn,
                                const Variant& v,
                                bool copy) {
  assert(checkInvariants(adIn));

  auto const ad = adIn->hasMultipleRefs() ? CopyAndResizeIfNeeded(adIn)
                                          : ResizeIfNeeded(adIn);
  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is added to the beginning of the array.
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(ad);
  }

  auto const size = ad->m_size;
  auto const data = packedData(ad);
  std::memmove(data + 1, data, sizeof *data * size);
  // TODO(#3888164): constructValHelper is making KindOfUninit checks.
  tvAsUninitializedVariant(&data[0]).constructValHelper(v);
  ad->m_size = size + 1;
  ad->m_pos = 0;
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

ArrayData* PackedArray::EscalateForSort(ArrayData* ad) {
  // Note: ToMixedCopy also grows so we have !isFull.  We could use
  // ToMixedCopyReserve?
  if (ad->m_kind == ArrayData::kVPackedKind) {
    PackedArray::downgradeAndWarn(ad, PackedArray::Reason::kSortNotSupported);
  }
  assert(checkInvariants(ad));
  return ToMixedCopy(ad);
}

void PackedArray::Ksort(ArrayData*, int, bool) {
  not_reached();
}

void PackedArray::Sort(ArrayData*, int, bool) {
  not_reached();
}

void PackedArray::Asort(ArrayData*, int, bool) {
  not_reached();
}

bool PackedArray::Uksort(ArrayData*, const Variant&) {
  not_reached();
}

bool PackedArray::Usort(ArrayData*, const Variant&) {
  not_reached();
}

bool PackedArray::Uasort(ArrayData*, const Variant&) {
  not_reached();
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

//////////////////////////////////////////////////////////////////////

}
