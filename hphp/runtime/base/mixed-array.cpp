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
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-helpers.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/empty-array.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/tv-val.h"
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

#include <folly/CPortability.h>
#include <folly/portability/Constexpr.h>

#include <algorithm>
#include <utility>

#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

static_assert(MixedArray::computeAllocBytes(MixedArray::SmallScale) ==
              kEmptyMixedArraySize, "");

std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyDictArray;
std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyDArray;

struct MixedArray::Initializer {
  Initializer() {
    auto const ad = reinterpret_cast<MixedArray*>(&s_theEmptyDictArray);
    ad->initHash(1);
    ad->m_sizeAndPos = 0;
    ad->m_scale_used = 1;
    ad->m_nextKI = 0;
    ad->initHeader(HeaderKind::Dict, StaticValue);
    assertx(ad->checkInvariants());
  }
};
MixedArray::Initializer MixedArray::s_initializer;

struct MixedArray::DArrayInitializer {
  DArrayInitializer() {
    auto const ad = reinterpret_cast<MixedArray*>(&s_theEmptyDArray);
    ad->initHash(1);
    ad->m_sizeAndPos = 0;
    ad->m_scale_used = 1;
    ad->m_nextKI = 0;
    ad->initHeader_16(HeaderKind::Mixed, StaticValue, ArrayData::kDArray);
    assertx(RuntimeOption::EvalHackArrDVArrs || ad->checkInvariants());
  }
};
MixedArray::DArrayInitializer MixedArray::s_darr_initializer;

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
ArrayData* MixedArray::MakeReserveImpl(uint32_t size,
                                       HeaderKind hk,
                                       ArrayData::DVArray dvArray) {
  assertx(hk == HeaderKind::Mixed || hk == HeaderKind::Dict);
  assertx(dvArray == ArrayData::kNotDVArray || dvArray == ArrayData::kDArray);
  assertx(hk != HeaderKind::Dict || dvArray == ArrayData::kNotDVArray);
  assertx(!RuntimeOption::EvalHackArrDVArrs ||
         dvArray == ArrayData::kNotDVArray);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAlloc(scale);

  // Intialize the hash table first, because the header is already in L1 cache,
  // but the hash table may not be.  So let's issue the cache request ASAP.
  ad->initHash(scale);

  ad->m_sizeAndPos   = 0; // size=0, pos=0
  ad->initHeader_16(hk, OneReference, dvArray);
  ad->m_scale_used   = scale; // used=0
  ad->m_nextKI       = 0;

  assertx(ad->m_kind == hk);
  assertx(ad->dvArray() == dvArray);
  assertx(ad->m_size == 0);
  assertx(ad->m_pos == 0);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == 0);
  assertx(ad->m_nextKI == 0);
  assertx(ad->m_scale == scale);
  assertx(ad->checkInvariants());
  return ad;
}

ArrayData* MixedArray::MakeReserveMixed(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::Mixed, ArrayData::kNotDVArray);
  assertx(ad->isMixedKind());
  assertx(ad->isNotDVArray());
  return ad;
}

ArrayData* MixedArray::MakeReserveDArray(uint32_t size) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto ad = MakeReserveImpl(size, HeaderKind::Mixed, ArrayData::kDArray);
  assertx(ad->isMixedKind());
  assertx(ad->isDArray());
  return tagArrProv(ad);
}

ArrayData* MixedArray::MakeReserveDict(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::Dict, ArrayData::kNotDVArray);
  assertx(ad->isDictKind());
  return tagArrProv(ad);
}

ArrayData* MixedArray::MakeReserveLike(const ArrayData* other,
                                       uint32_t capacity) {
  capacity = (capacity ? capacity : other->size());

  if (other->hasVanillaPackedLayout()) {
    return PackedArray::MakeReserve(capacity);
  } else {
    return MixedArray::MakeReserveMixed(capacity);
  }
}

ALWAYS_INLINE
MixedArray* MixedArray::MakeStructImpl(uint32_t size,
                                       const StringData* const* keys,
                                       const TypedValue* values,
                                       HeaderKind hk,
                                       ArrayData::DVArray dvArray) {
  assertx(size > 0);
  assertx(hk == HeaderKind::Mixed || hk == HeaderKind::Dict);
  assertx(dvArray == ArrayData::kNotDVArray ||
          dvArray == ArrayData::kDArray);
  assertx(hk != HeaderKind::Dict || dvArray == ArrayData::kNotDVArray);
  assertx(!RuntimeOption::EvalHackArrDVArrs ||
          dvArray == ArrayData::kNotDVArray);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAlloc(scale);

  auto const aux = MixedArrayKeys::packStaticStrsForAux() |
                   static_cast<uint16_t>(dvArray);

  ad->m_sizeAndPos       = size; // pos=0
  ad->initHeader_16(hk, OneReference, aux);
  ad->m_scale_used       = scale | uint64_t{size} << 32; // used=size
  ad->m_nextKI           = 0;

  auto const table = ad->initHash(scale);
  auto const mask = ad->mask();
  auto const data = ad->data();

  // Append values by moving -- Caller assumes we update refcount.
  // Values are in reverse order since they come from the stack, which
  // grows down.
  for (uint32_t i = 0; i < size; i++) {
    assertx(keys[i]->isStatic());
    auto k = keys[i];
    auto h = k->hash();
    data[i].setStaticKey(const_cast<StringData*>(k), h);
    const auto& tv = values[size - i - 1];
    data[i].data.m_data = tv.m_data;
    data[i].data.m_type = tv.m_type;
    auto ei = ad->findForNewInsert(table, mask, h);
    *ei = i;
  }

  assertx(ad->m_size == size);
  assertx(ad->dvArray() == dvArray);
  assertx(ad->m_pos == 0);
  assertx(ad->m_kind == hk);
  assertx(ad->m_scale == scale);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == size);
  assertx(ad->m_nextKI == 0);
  assertx(ad->checkInvariants());
  return ad;
}

MixedArray* MixedArray::MakeStruct(uint32_t size,
                                   const StringData* const* keys,
                                   const TypedValue* values) {
  return MakeStructImpl(size, keys, values,
                        HeaderKind::Mixed,
                        ArrayData::kNotDVArray);
}

MixedArray* MixedArray::MakeStructDict(uint32_t size,
                                       const StringData* const* keys,
                                       const TypedValue* values) {
  auto const ad = MakeStructImpl(size, keys, values,
                                 HeaderKind::Dict,
                                 ArrayData::kNotDVArray);
  return asMixed(tagArrProv(ad));
}

MixedArray* MixedArray::MakeStructDArray(uint32_t size,
                                         const StringData* const* keys,
                                         const TypedValue* values) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto const ad = MakeStructImpl(size, keys, values,
                                 HeaderKind::Mixed,
                                 ArrayData::kDArray);
  return asMixed(tagArrProv(ad));
}

ALWAYS_INLINE
MixedArray* MixedArray::AllocStructImpl(uint32_t size,
                                        const int32_t* hash,
                                        HeaderKind hk,
                                        ArrayData::DVArray dvArray) {
  assertx(size > 0);
  assertx(hk == HeaderKind::Mixed || hk == HeaderKind::Dict);
  assertx(dvArray == ArrayData::kNotDVArray ||
          dvArray == ArrayData::kDArray);
  assertx(hk != HeaderKind::Dict || dvArray == ArrayData::kNotDVArray);
  assertx(!RuntimeOption::EvalHackArrDVArrs ||
          dvArray == ArrayData::kNotDVArray);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAlloc(scale);

  auto const aux = MixedArrayKeys::packStaticStrsForAux() |
                   static_cast<uint16_t>(dvArray);

  ad->m_sizeAndPos       = size; // pos=0
  ad->initHeader_16(hk, OneReference, aux);
  ad->m_scale_used       = scale | uint64_t{size} << 32; // used=size
  ad->m_nextKI           = 0;

  CopyHash(ad->hashTab(), hash, scale);

  // If we don't trash the elements here, `ad` may fail checkInvariants
  // because some of its element's type bytes happen to be tombstones.
  //
  // Trashing the elements isn't likely to hide real failures, because if we
  // fail to initialize them later, any lookups will return implausible TVs.
  if (debug) memset(ad->data(), kMixedElmFill, sizeof(MixedArrayElm) * size);

  assertx(ad->m_size == size);
  assertx(ad->dvArray() == dvArray);
  assertx(ad->m_pos == 0);
  assertx(ad->m_kind == hk);
  assertx(ad->m_scale == scale);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == size);
  assertx(ad->m_nextKI == 0);
  assertx(ad->checkInvariants());
  return ad;
}

MixedArray* MixedArray::AllocStruct(uint32_t size, const int32_t* hash) {
  return AllocStructImpl(size, hash, HeaderKind::Mixed, ArrayData::kNotDVArray);
}

MixedArray* MixedArray::AllocStructDict(uint32_t size, const int32_t* hash) {
  auto const ad = AllocStructImpl(size, hash, HeaderKind::Dict,
                                  ArrayData::kNotDVArray);
  return asMixed(tagArrProv(ad));
}

MixedArray* MixedArray::AllocStructDArray(uint32_t size, const int32_t* hash) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto const ad = AllocStructImpl(size, hash, HeaderKind::Mixed,
                                  ArrayData::kDArray);
  return asMixed(tagArrProv(ad));
}

template<HeaderKind hdr, ArrayData::DVArray dv>
MixedArray* MixedArray::MakeMixedImpl(uint32_t size, const TypedValue* kvs) {
  assertx(size > 0);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAlloc(scale);

  ad->initHash(scale);

  ad->m_sizeAndPos       = size; // pos=0
  ad->initHeader_16(hdr, OneReference, dv);
  ad->m_scale_used       = scale | uint64_t{size} << 32; // used=size
  ad->m_nextKI           = 0;

  // Append values by moving -- no refcounts are updated.
  auto const data = ad->data();
  for (uint32_t i = 0; i < size; i++) {
    auto& kTv = kvs[i * 2];
    if (isStringType(kTv.m_type)) {
      auto k = kTv.m_data.pstr;
      auto h = k->hash();
      auto ei = ad->findForInsertUpdate(k, h);
      if (isValidPos(ei)) {
        // it's the caller's responsibility to free kvs
        ad->setZombie();
        Release(ad);
        return nullptr;
      }
      data[i].setStrKeyNoIncRef(k, h);
      ad->mutableKeyTypes()->recordStr(k);
      *ei = i;
    } else {
      assertx(kTv.m_type == KindOfInt64);
      auto k = kTv.m_data.num;
      auto h = hash_int64(k);
      auto ei = ad->findForInsertUpdate(k, h);
      if (isValidPos(ei)) {
        // it's the caller's responsibility to free kvs
        ad->setZombie();
        Release(ad);
        return nullptr;
      }
      data[i].setIntKey(k, h);
      ad->mutableKeyTypes()->recordInt();
      *ei = i;
    }
    const auto& tv = kvs[(i * 2) + 1];
    data[i].data.m_data = tv.m_data;
    data[i].data.m_type = tv.m_type;
  }

  assertx(ad->m_size == size);
  assertx(ad->m_pos == 0);
  assertx(ad->m_scale == scale);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == size);
  assertx(ad->m_nextKI == 0);
  assertx(ad->checkInvariants());
  return ad;
}

MixedArray* MixedArray::MakeMixed(uint32_t size, const TypedValue* kvs) {
  auto const ad =
    MakeMixedImpl<HeaderKind::Mixed, ArrayData::kNotDVArray>(size, kvs);
  assertx(ad == nullptr || ad->kind() == kMixedKind);
  assertx(ad == nullptr || ad->isNotDVArray());
  return ad;
}

MixedArray* MixedArray::MakeDArray(uint32_t size, const TypedValue* kvs) {
  if (RuntimeOption::EvalHackArrDVArrs) return MakeDict(size, kvs);

  auto const ad =
    MakeMixedImpl<HeaderKind::Mixed, ArrayData::kDArray>(size, kvs);
  assertx(ad == nullptr || ad->kind() == kMixedKind);
  assertx(ad == nullptr || ad->isDArray());
  return ad ? asMixed(tagArrProv(ad)) : nullptr;
}

MixedArray* MixedArray::MakeDict(uint32_t size, const TypedValue* kvs) {
  auto const ad =
    MakeMixedImpl<HeaderKind::Dict, ArrayData::kNotDVArray>(size, kvs);
  assertx(ad == nullptr || ad->kind() == kDictKind);
  return ad ? asMixed(tagArrProv(ad)) : nullptr;
}

MixedArray* MixedArray::MakeDArrayNatural(uint32_t size,
                                          const TypedValue* vals) {
  assertx(size > 0);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAlloc(scale);

  ad->initHash(scale);

  ad->m_sizeAndPos       = size; // pos=0
  if (RuntimeOption::EvalHackArrDVArrs) {
    auto const aux = MixedArrayKeys::packIntsForAux() |
                     static_cast<uint16_t>(ArrayData::kNotDVArray);
    ad->initHeader_16(HeaderKind::Dict, OneReference, aux);
  } else {
    auto const aux = MixedArrayKeys::packIntsForAux() |
                     static_cast<uint16_t>(ArrayData::kDArray);
    ad->initHeader_16(HeaderKind::Mixed, OneReference, aux);
  }
  ad->m_scale_used       = scale | uint64_t{size} << 32; // used=size
  ad->m_nextKI           = size;

  // Append values by moving -- no refcounts are updated.
  auto const data = ad->data();
  for (uint32_t i = 0; i < size; i++) {
    auto h = hash_int64(i);
    auto ei = ad->findForInsertUpdate(i, h);
    assertx(!isValidPos(ei));
    data[i].setIntKey(i, h);
    *ei = i;

    const auto& tv = vals[i];
    data[i].data.m_data = tv.m_data;
    data[i].data.m_type = tv.m_type;
  }

  assertx(ad->m_size == size);
  assertx(ad->m_pos == 0);
  assertx(ad->m_scale == scale);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == size);
  assertx(ad->m_nextKI == size);
  assertx(ad->checkInvariants());
  return asMixed(tagArrProv(ad));
}

NEVER_INLINE
MixedArray* MixedArray::SlowCopy(MixedArray* ad, const ArrayData& old,
                                 MixedArrayElm* elm, MixedArrayElm* end) {
  assertx(ad->isRefCounted());

  for (; elm < end; ++elm) {
    if (elm->hasStrKey()) elm->skey->incRefCount();
    // When we convert an element to a tombstone, we set its value type to
    // kInvalidDataType, which is not a refcounted type.
    tvIncRefGenUnsafe(elm->data);
  }
  return ad;
}

// for internal use by copyStatic() and copyMixed()
ALWAYS_INLINE
MixedArray* MixedArray::CopyMixed(const MixedArray& other,
                                  AllocMode mode,
                                  HeaderKind dest_hk,
                                  ArrayData::DVArray dvArray) {
  assertx(dest_hk == HeaderKind::Mixed || dest_hk == HeaderKind::Dict);
  assertx(dvArray == ArrayData::kNotDVArray || dvArray == ArrayData::kDArray);
  assertx(dest_hk != HeaderKind::Dict || dvArray == ArrayData::kNotDVArray);
  if (mode == AllocMode::Static) {
    assertx(other.m_size == other.m_used);
    assertx(!other.keyTypes().mayIncludeCounted());
    assertx(!other.keyTypes().mayIncludeTombstone());
  }

  auto const scale = other.m_scale;
  auto const ad = mode == AllocMode::Request
    ? reqAlloc(scale)
    : staticAlloc(scale, arrprov::tagSize(&other));
#ifdef USE_JEMALLOC
  // Copy everything including tombstones.  We want to copy the elements and
  // the hash separately, because the array may not be very full.
  assertx(reinterpret_cast<uintptr_t>(ad) % 16 == 0);
  assertx(reinterpret_cast<uintptr_t>(&other) % 16 == 0);
  // Adding 24 bytes so that we can copy in 32-byte groups. This might
  // overwrite the hash table, but won't overrun the allocated space as long as
  // `malloc' returns multiple of 16 bytes.
  bcopy32_inline(ad, &other,
                 sizeof(MixedArray) + sizeof(Elm) * other.m_used + 24);
#else
  memcpy(ad, &other, sizeof(MixedArray) + sizeof(Elm) * other.m_used);
#endif
  auto const count = mode == AllocMode::Request ? OneReference : StaticValue;
  auto const aux =
    other.keyTypes().packForAux() |
    (dvArray | (other.isLegacyArray() ? ArrayData::kLegacyArray : 0));
  ad->initHeader_16(dest_hk, count, aux);

  // We want SlowCopy to be a tail call in the opt build, but we still want to
  // check assertions in debug builds, so we check them in this helper.
  auto const check = [&](MixedArray* res) {
    assertx(res->checkInvariants());
    assertx(res->m_kind == dest_hk);
    assertx(res->dvArray() == dvArray);
    assertx(res->isLegacyArray() == other.isLegacyArray());
    assertx(res->keyTypes() == other.keyTypes());
    assertx(res->m_size == other.m_size);
    assertx(res->m_pos == other.m_pos);
    assertx(res->m_used == other.m_used);
    assertx(res->m_scale == scale);
    return res;
  };

  CopyHash(ad->hashTab(), other.hashTab(), scale);
  if (mode == AllocMode::Static) return check(ad);

  // Bump up refcounts as needed.
  MixedArrayElm* elm = ad->data();
  auto const end = elm + ad->m_used;
  if (other.keyTypes().mayIncludeCounted()) {
    return check(SlowCopy(ad, other, elm, end));
  }
  for (; elm < end; ++elm) {
    assertx(IMPLIES(elm->hasStrKey(), elm->strKey()->isStatic()));
    // When we convert an element to a tombstone, we set its key type to int
    // and value type to kInvalidDataType, neither of which are refcounted.
    tvIncRefGenUnsafe(elm->data);
  }
  return check(ad);
}

NEVER_INLINE ArrayData* MixedArray::CopyStatic(const ArrayData* in) {
  auto a = asMixed(in);
  assertx(a->checkInvariants());
  auto ret = CopyMixed(*a, AllocMode::Static, in->m_kind, in->dvArray());

  if (RuntimeOption::EvalArrayProvenance) {
    assertx(!ret->hasProvenanceData());
    auto const tag = arrprov::getTag(in);
    if (tag.valid()) {
      arrprov::setTag(ret, tag);
    }
  }
  assertx(!arrprov::arrayWantsTag(ret) ||
          !!arrprov::getTag(ret) == !!arrprov::getTag(in));
  return ret;
}

NEVER_INLINE MixedArray* MixedArray::copyMixed() const {
  assertx(checkInvariants());
  auto const out = CopyMixed(*this, AllocMode::Request, m_kind, dvArray());
  return asMixed(tagArrProv(out, this));
}

//////////////////////////////////////////////////////////////////////

ArrayData* MixedArray::MakeUncounted(ArrayData* array,
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
  auto a = asMixed(array);
  assertx(!a->empty());
  auto const extra = uncountedAllocExtra(array, withApcTypedValue);
  auto const ad = uncountedAlloc(a->scale(), extra);
  auto const used = a->m_used;
  // Do a raw copy first, without worrying about counted types or refcount
  // manipulation.  To copy in 32-byte chunks, we add 24 bytes to the length.
  // This might overwrite the hash table, but won't go beyond the space
  // allocated for the MixedArray, assuming `malloc()' always allocates
  // multiple of 16 bytes and extra is also a multiple of 16.
  assertx((extra & 0xf) == 0);
  bcopy32_inline(ad, a, sizeof(MixedArray) + sizeof(Elm) * used + 24);
  ad->m_count = UncountedValue; // after bcopy, update count
  if (withApcTypedValue) {
    ad->m_aux16 |= kHasApcTv;
  } else {
    ad->m_aux16 &= ~kHasApcTv;
  }
  ad->m_aux16 &= ~kHasProvenanceData;
  assertx(ad->keyTypes() == a->keyTypes());
  CopyHash(ad->hashTab(), a->hashTab(), a->scale());
  ad->mutableKeyTypes()->makeStatic();

  // Need to make sure keys and values are all uncounted.
  auto dstElem = ad->data();
  for (uint32_t i = 0; i < used; ++i) {
    auto& te = dstElem[i];
    auto const type = te.data.m_type;
    if (UNLIKELY(isTombstone(type))) continue;
    if (te.hasStrKey() && !te.skey->isStatic()) {
      if (te.skey->isUncounted() && te.skey->uncountedIncRef()) {
        ad->mutableKeyTypes()->recordNonStaticStr();
      } else {
        te.skey = [&] {
          if (auto const st = lookupStaticString(te.skey)) return st;
          ad->mutableKeyTypes()->recordNonStaticStr();
          HeapObject** seenStr = nullptr;
          if (seen && te.skey->hasMultipleRefs()) {
            seenStr = &(*seen)[te.skey];
            if (auto const st = static_cast<StringData*>(*seenStr)) {
              if (st->uncountedIncRef()) return st;
            }
          }
          auto const st = StringData::MakeUncounted(te.skey->slice());
          if (seenStr) *seenStr = st;
          return st;
        }();
      }
    }
    ConvertTvToUncounted(&te.data, seen);
  }
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().addAPCUncountedBlock();
  }
  if (updateSeen) (*seen)[array] = ad;
  assertx(ad->checkInvariants());
  return tagArrProv(ad, array);
}

ArrayData* MixedArray::MakeDictFromAPC(const APCArray* apc) {
  assertx(apc->isDict());
  auto const apcSize = apc->size();
  DictInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.setValidKey(apc->getKey(i), apc->getValue(i)->toLocal());
  }
  auto const ad = init.create();
  return tagArrProv(ad, apc);
}

ArrayData* MixedArray::MakeDArrayFromAPC(const APCArray* apc) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  assertx(apc->isDArray());
  auto const apcSize = apc->size();
  DArrayInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.setValidKey(apc->getKey(i), apc->getValue(i)->toLocal());
  }
  auto const ad = init.create();
  return tagArrProv(ad, apc);
}

//=============================================================================
// Destruction

NEVER_INLINE
void MixedArray::SlowRelease(MixedArray* ad) {
  assertx(ad->isRefCounted());
  assertx(ad->hasExactlyOneRef());
  assertx(!ad->isZombie());

  MixedArrayElm* elm = ad->data();
  for (auto const end = elm + ad->m_used; elm < end; ++elm) {
    if (elm->hasStrKey()) {
      decRefStr(elm->skey);
      // Keep GC from asserting on freed string keys in debug mode.
      if (debug) elm->skey = nullptr;
    }
    // When we convert an element to a tombstone, we set its key type to int
    // and value type to kInvalidDataType, neither of which are refcounted.
    tvDecRefGen(&elm->data);
  }
  tl_heap->objFree(ad, ad->heapSize());
}

NEVER_INLINE
void MixedArray::Release(ArrayData* in) {
  in->fixCountForRelease();
  assertx(in->isRefCounted());
  assertx(in->hasExactlyOneRef());

  if (RuntimeOption::EvalArrayProvenance) {
    arrprov::clearTag(in);
  }
  auto const ad = asMixed(in);

  if (!ad->isZombie()) {
    assertx(ad->checkInvariants());
    // keyTypes checks are too slow to go in MixedArray::checkInvariants.
    assertx(ad->keyTypes().checkInvariants(ad));
    if (ad->keyTypes().mayIncludeCounted()) return SlowRelease(ad);

    MixedArrayElm* elm = ad->data();
    for (auto const end = elm + ad->m_used; elm < end; ++elm) {
      // Keep the GC from asserting on freed string keys in debug mode.
      assertx(IMPLIES(elm->hasStrKey(), elm->strKey()->isStatic()));
      if (debug && elm->hasStrKey()) elm->skey = nullptr;
      // When we convert an element to a tombstone, we set its key type to int
      // and value type to kInvalidDataType, neither of which are refcounted.
      tvDecRefGen(&elm->data);
    }
  }
  tl_heap->objFree(ad, ad->heapSize());
  AARCH64_WALKABLE_FRAME();
}

NEVER_INLINE
void MixedArray::ReleaseUncounted(ArrayData* in) {
  auto const ad = asMixed(in);
  if (!ad->uncountedDecRef()) return;

  if (RuntimeOption::EvalArrayProvenance) {
    arrprov::clearTag(in);
  }
  if (!ad->isZombie()) {
    auto const data = ad->data();
    auto const stop = data + ad->m_used;

    for (auto ptr = data; ptr != stop; ++ptr) {
      if (isTombstone(ptr->data.m_type)) continue;
      if (ptr->hasStrKey()) {
        assertx(!ptr->skey->isRefCounted());
        if (ptr->skey->isUncounted()) {
          StringData::ReleaseUncounted(ptr->skey);
        }
      }
      ReleaseUncountedTv(&ptr->data);
    }
  }
  auto const extra = uncountedAllocExtra(ad, ad->hasApcTv());
  uncounted_sized_free(reinterpret_cast<char*>(ad) - extra,
                       computeAllocBytes(ad->scale()) + extra);
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
 *
 * Non-zombie:
 *
 *   m_size <= m_used; m_used <= capacity()
 *   last element cannot be a tombstone
 *   m_pos and all external iterators can't be on a tombstone
 *   m_nextKI >= highest actual int key
 *   Elm.data.m_type maybe kInvalidDataType (tombstone)
 *   hash[] maybe Tombstone
 *   static arrays cannot contain tombstones
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
  assertx(hasVanillaMixedLayout());
  assertx(!isMixedKind() || !isVArray());
  assertx(!isDictKind() || isNotDVArray());
  assertx(!RuntimeOption::EvalHackArrDVArrs || isNotDVArray());
  assertx(checkCount());
  assertx(m_scale >= 1 && (m_scale & (m_scale - 1)) == 0);
  assertx(MixedArray::HashSize(m_scale) ==
         folly::nextPowTwo<uint64_t>(capacity()));
  assertx(arrprov::arrayWantsTag(this) || !hasProvenanceData());

  if (isZombie()) return true;

  // Non-zombie:
  assertx(m_size <= m_used);
  assertx(m_used <= capacity());
  assertx(IMPLIES(isStatic(), m_used == m_size));
  if (m_pos != m_used) {
    assertx(size_t(m_pos) < m_used);
    assertx(!isTombstone(data()[m_pos].data.m_type));
  }
  return true;
}

//=============================================================================
// Iteration.

size_t MixedArray::Vsize(const ArrayData*) { not_reached(); }

TypedValue MixedArray::GetPosVal(const ArrayData* ad, ssize_t pos) {
  auto a = asMixed(ad);
  assertx(a->checkInvariants());
  assertx(pos != a->m_used);
  auto const& e = a->data()[pos];
  assertx(!e.isTombstone());
  return e.data;
}

bool MixedArray::IsVectorData(const ArrayData* ad) {
  auto a = asMixed(ad);
  if (a->m_size == 0) {
    // any 0-length array is "vector-like" for the sake of this function.
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
  assertx(!isFull());
  auto h = hash_int64(k);
  auto ei = findForInsertUpdate(k, h);
  if (isValidPos(ei)) {
    return InsertPos(true, data()[(int32_t)*ei].data);
  }
  if (k >= m_nextKI && m_nextKI >= 0) m_nextKI = static_cast<uint64_t>(k) + 1;
  auto e = allocElm(ei);
  e->setIntKey(k, h);
  mutableKeyTypes()->recordInt();
  return InsertPos(false, e->data);
}

MixedArray::InsertPos MixedArray::insert(StringData* k) {
  assertx(!isFull());
  auto const h = k->hash();
  auto ei = findForInsertUpdate(k, h);
  if (isValidPos(ei)) {
    return InsertPos(true, data()[(int32_t)*ei].data);
  }
  auto e = allocElm(ei);
  e->setStrKey(k, h);
  mutableKeyTypes()->recordStr(k);
  return InsertPos(false, e->data);
}

NEVER_INLINE
int32_t MixedArray::findForRemove(int64_t ki, inthash_t h, bool updateNext) {
  // all vector methods should work w/out touching the hashtable
  return findForRemove(ki, h,
      [this, ki, updateNext] (Elm& e) {
        assertx(ki == e.ikey);
        // Conform to PHP5 behavior
        // Hacky: don't removed the unsigned cast, else g++ can optimize away
        // the check for == 0x7fff..., since there is no signed int k
        // for which k-1 == 0x7fff...
        if (((uint64_t)ki == (uint64_t)m_nextKI-1) &&
            (ki >= 0) &&
            (ki == 0x7fffffffffffffffLL || updateNext)) {
          m_nextKI = ki;
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
  return asMixed(ad)->findForExists(k, hash_int64(k));
}

bool MixedArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  return NvGetStr(ad, k).is_set();
}

//=============================================================================
// Append/insert/update.

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
MixedArray* MixedArray::SlowGrow(MixedArray* ad, const ArrayData& old,
                                 MixedArrayElm* elm, MixedArrayElm* end) {
  for (; elm < end; ++elm) {
    if (elm->hasStrKey()) elm->skey->incRefCount();
    // When we convert an element to a tombstone, we set its value type to
    // kInvalidDataType, which is not a refcounted type.
    tvIncRefGenUnsafe(elm->data);
  }

  auto const table = ad->initHash(ad->m_scale);
  auto const mask = MixedArray::Mask(ad->m_scale);

  elm = ad->data();
  if (UNLIKELY(ad->m_used >= 2000)) {
    return InsertCheckUnbalanced(ad, table, mask, elm, end);
  }
  for (uint32_t i = 0; elm != end; ++elm, ++i) {
    if (elm->isTombstone()) continue;
    *ad->findForNewInsert(table, mask, elm->probe()) = i;
  }
  return ad;
}

NEVER_INLINE
MixedArray* MixedArray::Grow(MixedArray* old, uint32_t newScale, bool copy) {
  assertx(old->m_size > 0);
  assertx(MixedArray::Capacity(newScale) >= old->m_size);
  assertx(newScale >= 1 && (newScale & (newScale - 1)) == 0);

  auto ad            = reqAlloc(newScale);
  auto const oldUsed = old->m_used;
  ad->m_sizeAndPos   = old->m_sizeAndPos;
  ad->initHeader_16(old->m_kind, OneReference, old->m_aux16);
  ad->m_scale_used   = newScale | uint64_t{oldUsed} << 32;
  ad->m_aux16 &= ~ArrayData::kHasProvenanceData;

  copyElmsNextUnsafe(ad, old, oldUsed);

  // We want SlowGrow to be a tail call in the opt build, but we still want to
  // check assertions in debug builds, so we check them in this helper.
  auto const check = [&](MixedArray* res) {
    assertx(res->checkInvariants());
    assertx(res->hasExactlyOneRef());
    assertx(res->kind() == old->kind());
    assertx(res->dvArray() == old->dvArray());
    assertx(res->isLegacyArray() == old->isLegacyArray());
    assertx(res->keyTypes() == old->keyTypes());
    assertx(res->m_size == old->m_size);
    assertx(res->m_pos == old->m_pos);
    assertx(res->m_used == oldUsed);
    assertx(res->m_scale == newScale);
    return asMixed(tagArrProv(res, old));
  };

  if (copy) {
    MixedArrayElm* elm = ad->data();
    auto const end = elm + oldUsed;
    if (ad->keyTypes().mayIncludeCounted()) {
      return check(SlowGrow(ad, *old, elm, end));
    }
    for (; elm < end; ++elm) {
      // When we convert an element to a tombstone, we set its key type to int
      // and value type to kInvalidDataType, neither of which are refcounted.
      tvIncRefGenUnsafe(elm->data);
    }
  } else {
    old->setZombie();
  }

  auto const table = ad->initHash(newScale);

  auto iter = ad->data();
  auto const stop = iter + oldUsed;
  assertx(newScale == ad->m_scale);
  auto mask = MixedArray::Mask(newScale);

  if (UNLIKELY(oldUsed >= 2000)) {
    return check(InsertCheckUnbalanced(ad, table, mask, iter, stop));
  }
  for (uint32_t i = 0; iter != stop; ++iter, ++i) {
    auto& e = *iter;
    if (e.isTombstone()) continue;
    *ad->findForNewInsert(table, mask, e.probe()) = i;
  }
  return check(ad);
}

ALWAYS_INLINE
MixedArray* MixedArray::prepareForInsert(bool copy) {
  assertx(checkInvariants());
  if (isFull()) return Grow(this, m_scale * 2, copy);
  if (copy) return copyMixed();
  return this;
}

void MixedArray::compact(bool renumber /* = false */) {
  bool updatePosAfterCompact = false;
  ElmKey mPos;

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
      assertx(size_t(m_pos) < m_used);
      auto& e = data()[m_pos];
      mPos.hash = e.hash();
      mPos.skey = e.skey;
    }
  } else {
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
      assertx(frPos + 1 < m_used);
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
    assertx(m_pos >= 0 && m_pos < m_size);
  }

  m_used = m_size;
  // Even if renumber is true, we'll leave string keys in the array untouched,
  // so the only keyTypes update we can do here is to unset the tombstone bit.
  mutableKeyTypes()->makeCompact();
  assertx(checkInvariants());
}

void MixedArray::nextInsert(TypedValue v) {
  assertx(m_nextKI >= 0);
  assertx(!isFull());

  int64_t ki = m_nextKI;
  auto h = hash_int64(ki);
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  auto ei = findForNewInsert(h);
  assertx(!isValidPos(ei));
  // Allocate and initialize a new element.
  auto e = allocElm(ei);
  e->setIntKey(ki, h);
  mutableKeyTypes()->recordInt();
  m_nextKI = static_cast<uint64_t>(ki) + 1; // Update next free element.
  tvDup(v, e->data);
}

template <class K, bool move> ALWAYS_INLINE
ArrayData* MixedArray::update(K k, TypedValue data) {
  assertx(!isFull());
  auto p = insert(k);
  if (p.found) {
    setElem(p.tv, data, move);
    return this;
  }
  initElem(p.tv, data, move);
  return this;
}

arr_lval MixedArray::LvalInt(ArrayData* ad, int64_t k, bool copy) {
  return asMixed(ad)->prepareForInsert(copy)->addLvalImpl<true>(k);
}

arr_lval MixedArray::LvalStr(ArrayData* ad, StringData* key, bool copy) {
  return asMixed(ad)->prepareForInsert(copy)->addLvalImpl<true>(key);
}

tv_lval MixedArray::LvalInPlace(ArrayData* ad, const Variant& k) {
  auto arr = asMixed(ad);
  assertx(!arr->isFull());
  assertx(!arr->cowCheck());
  return k.isInteger() ? arr->addLvalImpl<false>(k.asInt64Val())
                       : arr->addLvalImpl<false>(k.asCStrRef().get());
}

arr_lval MixedArray::LvalSilentInt(ArrayData* ad, int64_t k) {
  auto a = asMixed(ad);
  auto const pos = a->find(k, hash_int64(k));
  if (UNLIKELY(!validPos(pos))) return arr_lval { a, nullptr };
  if (a->cowCheck()) a = a->copyMixed();
  return arr_lval { a, &a->data()[pos].data };
}

arr_lval MixedArray::LvalSilentStr(ArrayData* ad, StringData* k) {
  auto a = asMixed(ad);
  auto const pos = a->find(k, k->hash());
  if (UNLIKELY(!validPos(pos))) return arr_lval { a, nullptr };
  if (a->cowCheck()) a = a->copyMixed();
  return arr_lval { a, &a->data()[pos].data };
}

ArrayData* MixedArray::SetInt(ArrayData* ad, int64_t k, TypedValue v) {
  assertx(ad->cowCheck() || ad->notCyclic(v));
  return asMixed(ad)->prepareForInsert(ad->cowCheck())->update(k, v);
}

ArrayData* MixedArray::SetIntMove(ArrayData* ad, int64_t k, TypedValue v) {
  assertx(ad->cowCheck() || ad->notCyclic(v));
  auto const preped = asMixed(ad)->prepareForInsert(ad->cowCheck());
  auto const result = preped->update<int64_t, true>(k, v);
  if (ad != result && ad->decReleaseCheck()) MixedArray::Release(ad);
  return result;
}

ArrayData* MixedArray::SetIntInPlace(ArrayData* ad, int64_t k, TypedValue v) {
  assertx(!ad->cowCheck());
  assertx(ad->notCyclic(v));
  return asMixed(ad)->prepareForInsert(false/*copy*/)->update(k, v);
}

ArrayData* MixedArray::SetStr(ArrayData* ad, StringData* k, TypedValue v) {
  assertx(ad->cowCheck() || ad->notCyclic(v));
  return asMixed(ad)->prepareForInsert(ad->cowCheck())->update(k, v);
}

ArrayData* MixedArray::SetStrMove(ArrayData* ad, StringData* k, TypedValue v) {
  assertx(ad->cowCheck() || ad->notCyclic(v));
  auto const preped = asMixed(ad)->prepareForInsert(ad->cowCheck());
  auto const result = preped->update<StringData*, true>(k, v);
  if (ad != result && ad->decReleaseCheck()) MixedArray::Release(ad);
  return result;
}

ArrayData* MixedArray::SetStrInPlace(ArrayData* ad, StringData* k, TypedValue v) {
  assertx(!ad->cowCheck());
  assertx(ad->notCyclic(v));
  return asMixed(ad)->prepareForInsert(false/*copy*/)->update(k, v);
}

ArrayData* MixedArray::AddInt(ArrayData* ad, int64_t k, TypedValue v, bool copy) {
  assertx(!ad->exists(k));
  return asMixed(ad)->prepareForInsert(copy)->addVal(k, v);
}

ArrayData* MixedArray::AddStr(ArrayData* ad, StringData* k, TypedValue v, bool copy) {
  assertx(!ad->exists(k));
  return asMixed(ad)->prepareForInsert(copy)->addVal(k, v);
}

//=============================================================================
// Delete.

void MixedArray::eraseNoCompact(ssize_t pos) {
  assertx(validPos(pos));

  // If the internal pointer points to this element, advance it.
  Elm* elms = data();
  if (m_pos == pos) {
    m_pos = nextElm(elms, pos);
  }

  auto& e = elms[pos];
  auto const oldTV = e.data;
  if (e.hasStrKey()) {
    decRefStr(e.skey);
  }
  e.setTombstone();
  mutableKeyTypes()->recordTombstone();

  --m_size;
  // Mark the hash entry as "deleted".
  assertx(m_used <= capacity());

  // Finally, decref the old value
  tvDecRefGen(oldTV);
}

ArrayData* MixedArray::RemoveIntImpl(ArrayData* ad, int64_t k, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  auto pos = a->findForRemove(k, hash_int64(k), false/*updateNext*/);
  if (validPos(pos)) a->erase(pos);
  return a;
}

ArrayData* MixedArray::RemoveInt(ArrayData* ad, int64_t k) {
  return RemoveIntImpl(ad, k, ad->cowCheck());
}

ArrayData*
MixedArray::RemoveStrImpl(ArrayData* ad, const StringData* key, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  auto pos = a->findForRemove(key, key->hash());
  if (validPos(pos)) a->erase(pos);
  return a;
}

ArrayData* MixedArray::RemoveStr(ArrayData* ad, const StringData* key) {
  return RemoveStrImpl(ad, key, ad->cowCheck());
}

ArrayData* MixedArray::Copy(const ArrayData* ad) {
  return asMixed(ad)->copyMixed();
}

ArrayData* MixedArray::AppendImpl(ArrayData* ad, TypedValue v, bool copy) {
  assertx(copy || ad->notCyclic(v));
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

ArrayData* MixedArray::Append(ArrayData* ad, TypedValue v) {
  return AppendImpl(ad, v, ad->cowCheck());
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

  auto const aux = MixedArrayKeys::compactPacked(src->m_aux16) &
                   ~ArrayData::kHasProvenanceData;

  ad->m_sizeAndPos      = src->m_sizeAndPos;
  ad->initHeader_16(src->m_kind, OneReference, aux);
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
    assertx(size_t(src->m_pos) < src->m_used);
    auto& e = srcElm[src->m_pos];
    mPos.hash = e.probe();
    mPos.skey = e.skey;
  }

  // Copy the elements
  auto mask = MixedArray::Mask(scale);
  for (; srcElm != srcStop; ++srcElm) {
    if (srcElm->isTombstone()) continue;
    tvDup(srcElm->data, dstElm->data);
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
    assertx(ad->m_pos >=0 && ad->m_pos < ad->m_size);
  } else {
    // If src->m_pos is equal to src's canonical invalid position, then
    // set ad->m_pos to ad's canonical invalid position.
    if (src->m_pos != 0)
      ad->m_pos = ad->m_size;
  }

  // Set new used value (we've removed any tombstones).
  assertx(i == dstElm - ad->data());
  ad->m_used = i;

  assertx(ad->kind() == src->kind());
  assertx(ad->dvArray() == src->dvArray());
  assertx(ad->m_size == src->m_size);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used <= oldUsed);
  assertx(ad->m_used == dstElm - ad->data());
  assertx(ad->m_scale == scale);
  assertx(ad->m_nextKI == src->m_nextKI);
  assertx(ad->checkInvariants());
  return ad;
}

NEVER_INLINE
ArrayData* MixedArray::ArrayPlusEqGeneric(ArrayData* ad,
                                          MixedArray* ret,
                                          const ArrayData* elems,
                                          size_t neededSize) {
  assertx(ad->isPHPArrayType());
  assertx(elems->isPHPArrayType());
  assertx(ret->isMixedKind());

  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    auto const value = it.secondVal();

    if (UNLIKELY(ret->isFull())) {
      assertx(ret == ad);
      ret = CopyReserve(asMixed(ad), neededSize);
    }

    auto tv = key.asTypedValue();
    auto p = tv->m_type == KindOfInt64
      ? ret->insert(tv->m_data.num)
      : ret->insert(tv->m_data.pstr);
    if (!p.found) {
      tvDup(value, p.tv);
    }
  }

  return ret;
}

// Note: the logic relating to how to grow in this function is coupled
// to PackedArray::PlusEq.
ArrayData* MixedArray::PlusEq(ArrayData* ad, const ArrayData* elems) {
  assertx(asMixed(ad)->checkInvariants());

  if (!ad->isPHPArrayType()) throwInvalidAdditionException(ad);
  if (!elems->isPHPArrayType()) throwInvalidAdditionException(elems);

  auto const neededSize = ad->size() + elems->size();

  auto ret =
    ad->cowCheck() ? CopyReserve(asMixed(ad), neededSize) :
    asMixed(ad);

  if (UNLIKELY(!elems->hasVanillaMixedLayout())) {
    return ArrayPlusEqGeneric(ad, ret, elems, neededSize);
  }

  auto const rhs = asMixed(elems);

  auto srcElem = rhs->data();
  auto const srcStop = rhs->data() + rhs->m_used;
  for (; srcElem != srcStop; ++srcElem) {
    if (srcElem->isTombstone()) continue;

    if (UNLIKELY(ret->isFull())) {
      assertx(ret == ad);
      ret = CopyReserve(ret, neededSize);
    }

    auto const hash = srcElem->hash();
    if (srcElem->hasStrKey()) {
      auto const ei = ret->findForInsertUpdate(srcElem->skey, hash);
      if (isValidPos(ei)) continue;
      auto e = ret->allocElm(ei);
      e->setStrKey(srcElem->skey, hash);
      ret->mutableKeyTypes()->recordStr(srcElem->skey);
      tvDup(srcElem->data, e->data);
    } else {
      auto const ei = ret->findForInsertUpdate(srcElem->ikey, hash);
      if (isValidPos(ei)) continue;
      auto e = ret->allocElm(ei);
      e->setIntKey(srcElem->ikey, hash);
      ret->mutableKeyTypes()->recordInt();
      tvDup(srcElem->data, e->data);
    }
  }

  return ret;
}

NEVER_INLINE
ArrayData* MixedArray::ArrayMergeGeneric(MixedArray* ret,
                                         const ArrayData* elems) {
  assertx(ret->isMixedKind());
  assertx(ret->isNotDVArray());

  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    auto const value = it.secondVal();
    if (key.asTypedValue()->m_type == KindOfInt64) {
      ret->nextInsert(value);
    } else {
      StringData* sd = key.getStringData();
      auto const lval = ret->addLvalImpl<false>(sd);
      assertx(value.m_type != KindOfUninit);
      tvSet(value, lval);
    }
  }
  return ret;
}

ArrayData* MixedArray::Merge(ArrayData* ad, const ArrayData* elems) {
  assertx(asMixed(ad)->checkInvariants());
  auto const ret = CopyReserve(asMixed(ad), ad->size() + elems->size());
  assertx(ret->hasExactlyOneRef());
  // Output is always a non-darray
  auto const aux = asMixed(ad)->keyTypes().packForAux();
  ret->initHeader_16(HeaderKind::Mixed, OneReference, aux);

  if (elems->hasVanillaMixedLayout()) {
    auto const rhs = asMixed(elems);
    auto srcElem = rhs->data();
    auto const srcStop = rhs->data() + rhs->m_used;

    for (; srcElem != srcStop; ++srcElem) {
      if (isTombstone(srcElem->data.m_type)) continue;

      if (srcElem->hasIntKey()) {
        ret->nextInsert(srcElem->data);
      } else {
        auto const lval = ret->addLvalImpl<false>(srcElem->skey);
        assertx(srcElem->data.m_type != KindOfUninit);
        tvSet(srcElem->data, lval);
      }
    }
    return ret;
  }

  if (UNLIKELY(!elems->hasVanillaPackedLayout())) {
    return ArrayMergeGeneric(ret, elems);
  }

  assertx(PackedArray::checkInvariants(elems));
  auto src           = packedData(elems);
  auto const srcStop = src + elems->m_size;
  for (; src != srcStop; ++src) {
    ret->nextInsert(*src);
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
    assertx(pos >= 0 && pos < a->m_used);
    auto& e = elms[pos];
    assertx(!isTombstone(e.data.m_type));
    value = tvAsCVarRef(&e.data);
    auto pos2 = e.hasStrKey() ? a->findForRemove(e.skey, e.hash())
                              : a->findForRemove(e.ikey, e.hash(), true);
    assertx(pos2 == pos);
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
    assertx(pos >= 0 && pos < a->m_used);
    auto& e = elms[pos];
    assertx(!isTombstone(e.data.m_type));
    value = tvAsCVarRef(&e.data);
    auto pos2 = e.hasStrKey() ? a->findForRemove(e.skey, e.hash())
                              : a->findForRemove(e.ikey, e.hash(), false);
    assertx(pos2 == pos);
    a->erase(pos2);
  } else {
    value = uninit_null();
  }
  // Even if the array is empty, for PHP5 conformity we need call
  // compact() because it has side-effects that are important
  a->compact(true);
  return a;
}

ArrayData* MixedArray::Prepend(ArrayData* adInput, TypedValue v) {
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
  a->mutableKeyTypes()->recordInt();
  tvDup(v, e.data);

  // Renumber.
  a->compact(true);
  return a;
}

ArrayData* MixedArray::ToPHPArray(ArrayData* in, bool copy) {
  auto adIn = asMixed(in);
  assertx(adIn->isPHPArrayKind());
  if (adIn->isNotDVArray()) return adIn;
  assertx(adIn->isDArray());
  if (adIn->getSize() == 0) return ArrayData::Create();
  auto ad = copy ? adIn->copyMixed() : adIn;
  ad->setDVArray(ArrayData::kNotDVArray);
  if (RO::EvalArrayProvenance) arrprov::clearTag(ad);
  assertx(ad->checkInvariants());
  return ad;
}

bool MixedArray::hasIntishKeys() const {
  auto const elms = data();
  for (uint32_t i = 0, limit = m_used; i < limit; ++i) {
    auto const& e = elms[i];
    if (e.isTombstone()) continue;
    if (e.hasStrKey()) {
      int64_t ignore;
      if (e.skey->isStrictlyInteger(ignore)) {
        return true;
      }
    }
  }
  return false;
}

/*
  * Copy this from adIn, intish casting all the intish string keys in
  * accordance with the value of the intishCast template parameter
  */
template <IntishCast IC>
ALWAYS_INLINE
ArrayData* MixedArray::copyWithIntishCast(MixedArray* adIn,
                                          bool asDArray /* = false */) {
  auto size = adIn->size();
  auto const elms = adIn->data();
  auto out =
    asMixed(asDArray ? MakeReserveDArray(size) : MakeReserveMixed(size));
  for (uint32_t i = 0, limit = adIn->m_used; i < limit; ++i) {
    auto const& e = elms[i];
    if (e.isTombstone()) continue;
    if (e.hasIntKey()) {
      out->update(e.ikey, e.data);
    } else {
      if (auto const intish = tryIntishCast<IC>(e.skey)) {
        out->update(*intish, e.data);
      } else {
        out->update(e.skey, e.data);
      }
    }
  }

  assertx(out->isMixedKind());
  assertx(out->checkInvariants());
  assertx(out->hasExactlyOneRef());
  return out;
}

ArrayData* MixedArray::ToPHPArrayIntishCast(ArrayData* in, bool copy) {
  // the input array should already be a PHP-array so we just need to
  // clear DV array bits and cast any intish strings that may appear
  auto adIn = asMixed(in);
  assertx(adIn->isPHPArrayKind());
  if (adIn->size() == 0) return ArrayData::Create();

  if (copy || adIn->hasIntishKeys()) {
    return copyWithIntishCast<IntishCast::Cast>(adIn);
  } else {
    // we don't need to CoW and there were no intish keys, so we can just update
    // dv-arrayness in place and get on with our day
    adIn->setDVArray(ArrayData::kNotDVArray);
    return adIn;
  }
}

template <IntishCast IC>
ALWAYS_INLINE
ArrayData* MixedArray::FromDictImpl(ArrayData* adIn,
                                    bool copy,
                                    bool toDArray) {
  assertx(adIn->isDictKind());
  auto a = asMixed(adIn);

  auto const size = a->size();

  if (!size) return toDArray ? ArrayData::CreateDArray() : ArrayData::Create();

  // If we don't necessarily have to make a copy, first scan the dict looking
  // for any int-like string keys. If we don't find any, we can transform the
  // dict in place.
  if (!copy && !a->hasIntishKeys()) {
    // No int-like string keys, so transform in place.
    a->m_kind = HeaderKind::Mixed;
    if (toDArray) a->setDVArray(ArrayData::kDArray);
    a->setLegacyArray(false);
    if (RO::EvalArrayProvenance) arrprov::reassignTag(a);
    assertx(a->checkInvariants());
    return a;
  } else {
    // Either we need to make a copy anyways, or we don't, but there are
    // int-like string keys. In either case, create the array from scratch,
    // inserting each element one-by-one, doing key conversion as necessary.
    return copyWithIntishCast<IC>(a, toDArray);
  }
}

ArrayData* MixedArray::ToPHPArrayDict(ArrayData* adIn, bool copy) {
  auto out = FromDictImpl<IntishCast::None>(adIn, copy, false);
  assertx(out->isNotDVArray());
  assertx(!out->isLegacyArray());
  return out;
}

ArrayData* MixedArray::ToPHPArrayIntishCastDict(ArrayData* adIn, bool copy) {
  auto out = FromDictImpl<IntishCast::Cast>(adIn, copy, false);
  assertx(out->isNotDVArray());
  assertx(!out->isLegacyArray());
  return out;
}

ArrayData* MixedArray::ToDArray(ArrayData* in, bool copy) {
  auto a = asMixed(in);
  assertx(a->isMixedKind());
  if (RuntimeOption::EvalHackArrDVArrs) return ToDict(in, copy);
  if (a->isDArray()) return a;
  if (a->getSize() == 0) return ArrayData::CreateDArray();
  auto out = copy ? a->copyMixed() : a;
  out->setDVArray(ArrayData::kDArray);
  out->setLegacyArray(false);
  assertx(out->checkInvariants());
  if (RO::EvalArrayProvenance) arrprov::reassignTag(out);
  return out;
}

ArrayData* MixedArray::ToDArrayDict(ArrayData* in, bool copy) {
  if (RuntimeOption::EvalHackArrDVArrs) return in;
  auto out = FromDictImpl<IntishCast::None>(in, copy, true);
  assertx(out->isDArray());
  assertx(!out->isLegacyArray());
  return out;
}

MixedArray* MixedArray::ToDictInPlace(ArrayData* ad) {
  auto a = asMixed(ad);
  assertx(a->isMixedKind());
  assertx(!a->cowCheck());
  a->m_kind = HeaderKind::Dict;
  a->setDVArray(ArrayData::kNotDVArray);
  if (RO::EvalArrayProvenance) arrprov::reassignTag(a);
  return a;
}

ArrayData* MixedArray::ToDict(ArrayData* ad, bool copy) {
  auto a = asMixed(ad);
  assertx(a->isMixedKind());

  if (a->empty() && a->m_nextKI == 0) return ArrayData::CreateDict();

  if (copy) {
    auto const out = CopyMixed(
      *a, AllocMode::Request,
      HeaderKind::Dict, ArrayData::kNotDVArray
    );
    return tagArrProv(out);
  } else {
    return tagArrProv(ToDictInPlace(a));
  }
}

ArrayData* MixedArray::ToDictDict(ArrayData* ad, bool) {
  assertx(asMixed(ad)->checkInvariants());
  assertx(ad->isDictKind());
  return ad;
}

void MixedArray::Renumber(ArrayData* ad) {
  asMixed(ad)->compact(true);
}

void MixedArray::OnSetEvalScalar(ArrayData* ad) {
  auto a = asMixed(ad);
  if (UNLIKELY(a->m_size < a->m_used)) {
    a->compact(/*renumber=*/false);
  }
  a->mutableKeyTypes()->makeStatic();
  auto elm = a->data();
  for (auto const end = elm + a->m_used; elm < end; ++elm) {
    assertx(!elm->isTombstone());
    auto key = elm->skey;
    if (elm->hasStrKey() && !key->isStatic()) {
      elm->skey = makeStaticString(key);
      decRefStr(key);
    }
    tvAsVariant(&elm->data).setEvalScalar();
  }
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
bool MixedArray::DictEqualHelper(const ArrayData* ad1, const ArrayData* ad2,
                                 bool strict) {
  assertx(asMixed(ad1)->checkInvariants());
  assertx(asMixed(ad2)->checkInvariants());
  assertx(ad1->isDictKind());
  assertx(ad2->isDictKind());

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
      if (!tvSame(*tvAssertPlausible(&elm1->data), *tvAssertPlausible(&elm2->data))) {
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
          return NvGetIntDict(ad2, elm->ikey);
        } else {
          assertx(elm->hasStrKey());
          return NvGetStrDict(ad2, elm->skey);
        }
      }();
      if (!other_rval ||
          !tvEqual(tvAssertPlausible(elm->data),
                     tvAssertPlausible(other_rval.tv()))) {
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

}
