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
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-helpers.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/str-key-table.h"
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
std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyMarkedDictArray;
std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyMarkedDArray;

struct MixedArray::DictInitializer {
  DictInitializer() {
    auto const ad = reinterpret_cast<MixedArray*>(&s_theEmptyDictArray);
    ad->initHash(1);
    ad->m_size  = 0;
    ad->m_extra = 0;
    ad->m_scale_used = 1;
    ad->m_nextKI = 0;
    ad->initHeader(HeaderKind::Dict, StaticValue);
    assertx(ad->checkInvariants());
  }
};
MixedArray::DictInitializer MixedArray::s_dict_initializer;

struct MixedArray::DArrayInitializer {
  DArrayInitializer() {
    auto const ad = reinterpret_cast<MixedArray*>(&s_theEmptyDArray);
    ad->initHash(1);
    ad->m_size = 0;
    ad->m_extra = 0;
    ad->m_scale_used = 1;
    ad->m_nextKI = 0;
    ad->initHeader(HeaderKind::Mixed, StaticValue);
    assertx(RuntimeOption::EvalHackArrDVArrs || ad->checkInvariants());
  }
};
MixedArray::DArrayInitializer MixedArray::s_darr_initializer;

struct MixedArray::MarkedDictArrayInitializer {
  MarkedDictArrayInitializer() {
    auto const ad = reinterpret_cast<MixedArray*>(&s_theEmptyMarkedDictArray);
    ad->initHash(1);
    ad->m_size = 0;
    ad->m_extra = 0;
    ad->m_scale_used = 1;
    ad->m_nextKI = 0;
    ad->initHeader_16(HeaderKind::Dict, StaticValue, ArrayData::kLegacyArray);
    assertx(!RuntimeOption::EvalHackArrDVArrs || ad->checkInvariants());
  }
};
MixedArray::MarkedDictArrayInitializer MixedArray::s_marked_dict_initializer;

struct MixedArray::MarkedDArrayInitializer {
  MarkedDArrayInitializer() {
    auto const ad = reinterpret_cast<MixedArray*>(&s_theEmptyMarkedDArray);
    ad->initHash(1);
    ad->m_size = 0;
    ad->m_extra = 0;
    ad->m_scale_used = 1;
    ad->m_nextKI = 0;
    ad->initHeader_16(HeaderKind::Mixed, StaticValue, ArrayData::kLegacyArray);
    assertx(RuntimeOption::EvalHackArrDVArrs || ad->checkInvariants());
  }
};
MixedArray::MarkedDArrayInitializer MixedArray::s_marked_darr_initializer;

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
ArrayData* MixedArray::MakeReserveImpl(uint32_t size, HeaderKind hk) {
  assertx(hk == HeaderKind::Mixed || hk == HeaderKind::Dict);
  assertx(!RuntimeOption::EvalHackArrDVArrs || hk != HeaderKind::Mixed);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAlloc(scale);

  // Intialize the hash table first, because the header is already in L1 cache,
  // but the hash table may not be.  So let's issue the cache request ASAP.
  ad->initHash(scale);

  ad->initHeader(hk, OneReference);
  ad->m_size         = 0;
  ad->m_extra        = 0;
  ad->m_scale_used   = scale; // used=0
  ad->m_nextKI       = 0;

  assertx(ad->m_kind == hk);
  assertx(ad->m_size == 0);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == 0);
  assertx(ad->m_nextKI == 0);
  assertx(ad->m_scale == scale);
  assertx(ad->checkInvariants());
  return ad;
}

ArrayData* MixedArray::MakeReserveMixed(uint32_t size) {
  return MakeReserveDArray(size);
}

ArrayData* MixedArray::MakeReserveDArray(uint32_t size) {
  if (RuntimeOption::EvalHackArrDVArrs) {
    auto const ad = MakeReserveDict(size);
    ad->setLegacyArray(RO::EvalHackArrDVArrMark);
    return ad;
  }

  auto ad = MakeReserveImpl(size, HeaderKind::Mixed);
  assertx(ad->isMixedKind());
  assertx(ad->isDArray());
  return tagArrProv(ad);
}

ArrayData* MixedArray::MakeReserveDict(uint32_t size) {
  auto ad = MakeReserveImpl(size, HeaderKind::Dict);
  assertx(ad->isDictKind());
  return ad;
}

ALWAYS_INLINE
MixedArray* MixedArray::MakeStructImpl(uint32_t size,
                                       const StringData* const* keys,
                                       const TypedValue* values,
                                       HeaderKind hk) {
  assertx(size > 0);
  assertx(hk == HeaderKind::Mixed || hk == HeaderKind::Dict);
  assertx(!RuntimeOption::EvalHackArrDVArrs || hk != HeaderKind::Mixed);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAlloc(scale);
  auto const aux = MixedArrayKeys::packStaticStrsForAux();

  ad->initHeader_16(hk, OneReference, aux);
  ad->m_size             = size;
  ad->m_extra            = 0;
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
  assertx(ad->m_kind == hk);
  assertx(ad->m_scale == scale);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == size);
  assertx(ad->m_nextKI == 0);
  assertx(ad->checkInvariants());
  return ad;
}

MixedArray* MixedArray::MakeStructDict(uint32_t size,
                                       const StringData* const* keys,
                                       const TypedValue* values) {
  return MakeStructImpl(size, keys, values, HeaderKind::Dict);
}

MixedArray* MixedArray::MakeStructDArray(uint32_t size,
                                         const StringData* const* keys,
                                         const TypedValue* values) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto const ad = MakeStructImpl(size, keys, values, HeaderKind::Mixed);
  return asMixed(tagArrProv(ad));
}

ALWAYS_INLINE
MixedArray* MixedArray::AllocStructImpl(uint32_t size,
                                        const int32_t* hash,
                                        HeaderKind hk) {
  assertx(size > 0);
  assertx(hk == HeaderKind::Mixed || hk == HeaderKind::Dict);
  assertx(!RuntimeOption::EvalHackArrDVArrs || hk != HeaderKind::Mixed);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAlloc(scale);
  auto const aux = MixedArrayKeys::packStaticStrsForAux();

  ad->initHeader_16(hk, OneReference, aux);
  ad->m_size             = size;
  ad->m_extra            = 0;
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
  assertx(ad->m_kind == hk);
  assertx(ad->m_scale == scale);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == size);
  assertx(ad->m_nextKI == 0);
  assertx(ad->checkInvariants());
  return ad;
}

MixedArray* MixedArray::AllocStructDict(uint32_t size, const int32_t* hash) {
  return AllocStructImpl(size, hash, HeaderKind::Dict);
}

MixedArray* MixedArray::AllocStructDArray(uint32_t size, const int32_t* hash) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto const ad = AllocStructImpl(size, hash, HeaderKind::Mixed);
  return asMixed(tagArrProv(ad));
}

template<HeaderKind hdr>
MixedArray* MixedArray::MakeMixedImpl(uint32_t size, const TypedValue* kvs) {
  assertx(size > 0);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAlloc(scale);

  ad->initHash(scale);

  ad->initHeader(hdr, OneReference);
  ad->m_size             = size;
  ad->m_extra            = 0;
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
  assertx(ad->m_scale == scale);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == size);
  assertx(ad->m_nextKI == 0);
  assertx(ad->checkInvariants());
  return ad;
}

MixedArray* MixedArray::MakeDArray(uint32_t size, const TypedValue* kvs) {
  if (RuntimeOption::EvalHackArrDVArrs) {
    auto const ad = MakeDict(size, kvs);
    ad->setLegacyArray(RO::EvalHackArrDVArrMark);
    return ad;
  }

  auto const ad = MakeMixedImpl<HeaderKind::Mixed>(size, kvs);
  assertx(ad == nullptr || ad->kind() == kMixedKind);
  assertx(ad == nullptr || ad->isDArray());
  return ad ? asMixed(tagArrProv(ad)) : nullptr;
}

MixedArray* MixedArray::MakeDict(uint32_t size, const TypedValue* kvs) {
  auto const ad = MakeMixedImpl<HeaderKind::Dict>(size, kvs);
  assertx(ad == nullptr || ad->kind() == kDictKind);
  return ad;
}

MixedArray* MixedArray::MakeDArrayNatural(uint32_t size,
                                          const TypedValue* vals) {
  assertx(size > 0);

  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAlloc(scale);
  auto const aux = MixedArrayKeys::packIntsForAux();

  ad->initHash(scale);

  if (RuntimeOption::EvalHackArrDVArrs) {
    ad->initHeader_16(
        HeaderKind::Dict,
        OneReference,
        aux | (RO::EvalHackArrDVArrMark ? ArrayData::kLegacyArray : 0));
  } else {
    ad->initHeader_16(HeaderKind::Mixed, OneReference, aux);
  }
  ad->m_size             = size;
  ad->m_extra            = 0;
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
MixedArray* MixedArray::CopyMixed(const MixedArray& other, AllocMode mode) {
  if (mode == AllocMode::Static) {
    assertx(other.m_size == other.m_used);
    assertx(!other.keyTypes().mayIncludeCounted());
    assertx(!other.keyTypes().mayIncludeTombstone());
  }

  auto const shouldCreateStrKeyTable =
    mode == AllocMode::Static &&
    other.keyTypes().mustBeStaticStrs() &&
    other.size() < StrKeyTable::kStrKeyTableSize * 3 / 4;
  auto const sizeOfStrKeyTable =
    shouldCreateStrKeyTable ? sizeof(StrKeyTable) : 0;
  auto const scale = other.m_scale;
  auto const ad = mode == AllocMode::Request
    ? reqAlloc(scale)
    : staticAlloc(scale, sizeOfStrKeyTable);
  // Copy everything including tombstones. This is a requirement for remove() to
  // work correctly, which assumes the position is the same in the original and
  // in the copy of the array, in case copying is needed.
#ifdef USE_JEMALLOC
  // Copy elements and hashes separately, because the array may not be very
  // full.
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
  auto const aux = other.auxBits() | other.keyTypes().packForAux() |
                   (shouldCreateStrKeyTable ? kHasStrKeyTable : 0);
  ad->initHeader_16(other.m_kind, count, aux);

  // We want SlowCopy to be a tail call in the opt build, but we still want to
  // check assertions in debug builds, so we check them in this helper.
  auto const check = [&](MixedArray* res) {
    assertx(res->checkInvariants());
    assertx(res->m_kind == other.m_kind);
    assertx(res->isLegacyArray() == other.isLegacyArray());
    assertx(res->keyTypes() == other.keyTypes());
    assertx(res->m_size == other.m_size);
    assertx(res->m_extra == other.m_extra);
    assertx(res->m_used == other.m_used);
    assertx(res->m_scale == scale);
    return res;
  };

  MixedArrayElm* elm = ad->data();
  auto const end = elm + ad->m_used;
  if (shouldCreateStrKeyTable) {
    auto table = ad->mutableStrKeyTable();
    for (; elm < end; ++elm) {
      assertx(elm->hasStrKey() && elm->strKey()->isStatic());
      table->add(elm->strKey());
    }
  }

  CopyHash(ad->hashTab(), other.hashTab(), scale);
  if (mode == AllocMode::Static) return check(ad);

  // Bump up refcounts as needed.
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
  auto ret = CopyMixed(*a, AllocMode::Static);

  assertx(IMPLIES(RO::EvalArrayProvenance,
                  arrprov::getTag(in) == arrprov::getTag(ret)));
  return ret;
}

NEVER_INLINE MixedArray* MixedArray::copyMixed() const {
  assertx(checkInvariants());
  auto const out = CopyMixed(*this, AllocMode::Request);
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

ArrayData* MixedArray::MakeDictFromAPC(const APCArray* apc, bool isLegacy) {
  assertx(apc->isHashed());
  auto const apcSize = apc->size();
  DictInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.setValidKey(apc->getKey(i), apc->getValue(i)->toLocal());
  }
  auto const ad = init.create();
  ad->setLegacyArray(isLegacy);
  return ad;
}

ArrayData* MixedArray::MakeDArrayFromAPC(const APCArray* apc, bool isMarked) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  assertx(apc->isDArray());
  auto const apcSize = apc->size();
  DArrayInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) {
    init.setValidKey(apc->getKey(i), apc->getValue(i)->toLocal());
  }
  auto const ad = init.create();
  ad->setLegacyArray(isMarked);
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
  assertx(IMPLIES(isDArray(), isMixedKind()));
  assertx(IMPLIES(isNotDVArray(), isDictKind()));
  assertx(IMPLIES(isLegacyArray(), isHAMSafeDArray()));
  assertx(!RuntimeOption::EvalHackArrDVArrs || isNotDVArray());
  assertx(checkCount());
  assertx(m_scale >= 1 && (m_scale & (m_scale - 1)) == 0);
  assertx(MixedArray::HashSize(m_scale) ==
         folly::nextPowTwo<uint64_t>(capacity()));
  assertx(IMPLIES(!arrprov::arrayWantsTag(this),
                  m_extra == 0 &&
                  IMPLIES(RO::EvalArrayProvenance,
                          !arrprov::getTag(this).valid())));

  if (isZombie()) return true;

  // Non-zombie:
  assertx(m_size <= m_used);
  assertx(m_used <= capacity());
  assertx(IMPLIES(isStatic(), m_used == m_size));
  return true;
}

//=============================================================================
// Iteration.

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

bool MixedArray::ExistsInt(const ArrayData* ad, int64_t k) {
  return asMixed(ad)->findForExists(k, hash_int64(k));
}

bool MixedArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  return NvGetStr(ad, k).is_init();
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
  ad->initHeader_16(old->m_kind, OneReference, old->m_aux16);
  ad->m_size = old->m_size;
  ad->m_extra = old->m_extra;
  ad->m_scale_used = newScale | uint64_t{oldUsed} << 32;
  ad->m_aux16 &= ~(ArrayData::kHasStrKeyTable);

  copyElmsNextUnsafe(ad, old, oldUsed);

  // We want SlowGrow to be a tail call in the opt build, but we still want to
  // check assertions in debug builds, so we check them in this helper.
  auto const check = [&](MixedArray* res) {
    assertx(res->checkInvariants());
    assertx(res->hasExactlyOneRef());
    assertx(res->kind() == old->kind());
    assertx(res->isDArray() == old->isDArray());
    assertx(res->isLegacyArray() == old->isLegacyArray());
    assertx(res->keyTypes() == old->keyTypes());
    assertx(res->m_size == old->m_size);
    assertx(res->m_used == oldUsed);
    assertx(res->m_scale == newScale);
    assertx(!res->hasStrKeyTable());
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
  // Set m_nextKI to 0 for now to prepare for renumbering integer keys
  if (UNLIKELY(renumber)) m_nextKI = 0;

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

arr_lval MixedArray::LvalInt(ArrayData* adIn, int64_t key) {
  auto const pos = asMixed(adIn)->find(key, hash_int64(key));
  if (!validPos(pos)) {
    throwOOBArrayKeyException(key, adIn);
  }
  auto const ad = adIn->cowCheck() ? Copy(adIn) : adIn;
  auto const& elm = asMixed(ad)->data()[pos];
  assertx(elm.intKey() == key);
  return { ad, const_cast<TypedValue*>(elm.datatv()) };
}

arr_lval MixedArray::LvalStr(ArrayData* adIn, StringData* key) {
  auto const pos = asMixed(adIn)->find(key, key->hash());
  if (!validPos(pos)) {
    throwOOBArrayKeyException(key, adIn);
  }
  auto const ad = adIn->cowCheck() ? Copy(adIn) : adIn;
  auto const& elm = asMixed(ad)->data()[pos];
  assertx(elm.strKey()->same(key));
  return { ad, const_cast<TypedValue*>(elm.datatv()) };
}

tv_lval MixedArray::LvalInPlace(ArrayData* ad, const Variant& k) {
  auto arr = asMixed(ad);
  assertx(!arr->isFull());
  assertx(!arr->cowCheck());
  return k.isInteger() ? arr->addLvalImpl(k.asInt64Val())
                       : arr->addLvalImpl(k.asCStrRef().get());
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

void MixedArray::updateNextKI(int64_t removedKi, bool updateNext) {
  // Conform to PHP5 behavior
  // Hacky: don't removed the unsigned cast, else g++ can optimize away
  // the check for == 0x7fff..., since there is no signed int k
  // for which k-1 == 0x7fff...
  if (((uint64_t)removedKi == (uint64_t)m_nextKI-1) &&
      (removedKi >= 0) &&
      (removedKi == 0x7fffffffffffffffLL || updateNext)) {
    m_nextKI = removedKi;
  }
}

void MixedArray::eraseNoCompact(RemovePos pos) {
  assertx(pos.valid());
  hashTab()[pos.probeIdx] = Tombstone;

  Elm* elms = data();
  auto& e = elms[pos.elmIdx];
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
  auto const pos = a->findForRemove(k, hash_int64(k));
  if (!pos.valid()) return a;
  if (copy) a = a->copyMixed();
  a->updateNextKI(k, false);
  a->erase(pos);
  return a;
}

ArrayData* MixedArray::RemoveInt(ArrayData* ad, int64_t k) {
  return RemoveIntImpl(ad, k, ad->cowCheck());
}

ArrayData*
MixedArray::RemoveStrImpl(ArrayData* ad, const StringData* key, bool copy) {
  auto a = asMixed(ad);
  auto const pos = a->findForRemove(key, key->hash());
  if (!pos.valid()) return a;
  if (copy) a = a->copyMixed();
  a->erase(pos);
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
                   ~ArrayData::kHasStrKeyTable;

  ad->initHeader_16(src->m_kind, OneReference, aux);
  ad->m_size            = src->m_size;
  ad->m_extra           = src->m_extra;
  ad->m_scale           = scale; // don't set m_used yet
  ad->m_nextKI          = src->m_nextKI;

  auto const table = ad->initHash(scale);

  auto dstElm = ad->data();
  auto srcElm = src->data();
  auto const srcStop = src->data() + oldUsed;
  uint32_t i = 0;

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

  // Set new used value (we've removed any tombstones).
  assertx(i == dstElm - ad->data());
  ad->m_used = i;

  assertx(ad->kind() == src->kind());
  assertx(ad->isDArray() == src->isDArray());
  assertx(ad->m_size == src->m_size);
  assertx(ad->m_extra == src->m_extra);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used <= oldUsed);
  assertx(ad->m_used == dstElm - ad->data());
  assertx(ad->m_scale == scale);
  assertx(ad->m_nextKI == src->m_nextKI);
  assertx(ad->checkInvariants());
  assertx(!ad->hasStrKeyTable());
  return ad;
}

NEVER_INLINE
ArrayData* MixedArray::ArrayMergeGeneric(MixedArray* ret,
                                         const ArrayData* elems) {
  assertx(ret->isHAMSafeDArray());

  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    auto const value = it.secondVal();
    if (key.asTypedValue()->m_type == KindOfInt64) {
      ret->nextInsert(value);
    } else {
      StringData* sd = key.getStringData();
      auto const lval = ret->addLvalImpl(sd);
      assertx(value.m_type != KindOfUninit);
      tvSet(value, lval);
    }
  }
  return tagArrProv(ret);
}

ArrayData* MixedArray::Merge(ArrayData* ad, const ArrayData* elems) {
  assertx(asMixed(ad)->checkInvariants());
  auto const ret = CopyReserve(asMixed(ad), ad->size() + elems->size());
  assertx(ret->hasExactlyOneRef());
  auto const aux = asMixed(ad)->keyTypes().packForAux();
  auto const hk  = RO::EvalHackArrDVArrs ? HeaderKind::Dict : HeaderKind::Mixed;
  ret->initHeader_16(hk, OneReference,
      aux |
      (RO::EvalHackArrDVArrMark && RO::EvalHackArrDVArrs ?
       ArrayData::kLegacyArray : 0));
  if (elems->hasVanillaMixedLayout()) {
    auto const rhs = asMixed(elems);
    auto srcElem = rhs->data();
    auto const srcStop = rhs->data() + rhs->m_used;

    for (; srcElem != srcStop; ++srcElem) {
      if (isTombstone(srcElem->data.m_type)) continue;

      if (srcElem->hasIntKey()) {
        ret->nextInsert(srcElem->data);
      } else {
        auto const lval = ret->addLvalImpl(srcElem->skey);
        assertx(srcElem->data.m_type != KindOfUninit);
        tvSet(srcElem->data, lval);
      }
    }
    return tagArrProv(ret);
  }

  if (UNLIKELY(!elems->hasVanillaPackedLayout())) {
    return ArrayMergeGeneric(ret, elems);
  }

  PackedArray::IterateVNoInc(elems, [&](TypedValue tv) {
    ret->nextInsert(tv);
  });

  return tagArrProv(ret);

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
                              : a->findForRemove(e.ikey, e.hash());
    assertx(pos2.elmIdx == pos);
    if (!e.hasStrKey()) {
      a->updateNextKI(e.ikey, true);
    }
    a->erase(pos2);
  } else {
    value = uninit_null();
  }
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
                              : a->findForRemove(e.ikey, e.hash());
    if (!e.hasStrKey()) {
      a->updateNextKI(e.ikey, false);
    }
    assertx(pos2.elmIdx == pos);
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

ArrayData* MixedArray::ToDArrayImpl(const MixedArray* ad) {
  auto const size = ad->size();
  auto const elms = ad->data();
  auto const used = ad->m_used;

  auto out = asMixed(MakeReserveDArray(size));
  for (uint32_t i = 0; i < used; ++i) {
    auto const& e = elms[i];
    if (e.isTombstone()) continue;
    if (e.hasIntKey()) {
      out->update(e.ikey, e.data);
    } else {
      out->update(e.skey, e.data);
    }
  }

  assertx(out->isPHPArrayType());
  assertx(out->checkInvariants());
  assertx(out->hasExactlyOneRef());
  return out;
}

ArrayData* MixedArray::ToDArray(ArrayData* in, bool copy) {
  if (in->isDArray()) return in;
  if (RuntimeOption::EvalHackArrDVArrs) {
    if (RuntimeOption::EvalHackArrDVArrMark && !in->isLegacyArray()) {
      if (copy) in = CopyMixed(*asMixed(in), AllocMode::Request);
      in->setLegacyArray(true);
    }
    return in;
  }

  if (in->empty()) return ArrayData::CreateDArray();

  auto const a = asMixed(in);
  if (copy) return ToDArrayImpl(a);

  a->m_kind = HeaderKind::Mixed;
  if (RO::EvalArrayProvenance) arrprov::reassignTag(a);
  assertx(a->checkInvariants());
  return a;
}

ArrayData* MixedArray::ToDict(ArrayData* ad, bool copy) {
  assertx(asMixed(ad)->checkInvariants());
  if (ad->isDictKind()) return ad;

  if (ad->isLegacyArray() && RO::EvalHackArrCompatCastMarkedArrayNotices) {
    raise_hack_arr_compat_cast_marked_array_notice(ad);
  }
  auto a = asMixed(ad);
  if (a->empty() && a->m_nextKI == 0) return ArrayData::CreateDict();

  if (copy) a = CopyMixed(*a, AllocMode::Request);

  if (RO::EvalArrayProvenance) arrprov::clearTag(a);
  a->m_kind = HeaderKind::Dict;
  a->setLegacyArray(false);
  assertx(a->checkInvariants());
  return a;
}

ArrayData* MixedArray::Renumber(ArrayData* adIn) {
  auto const ad = adIn->cowCheck() ? Copy(adIn) : adIn;
  asMixed(ad)->compact(true);
  return tagArrProv(ad);
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
        if (!HPHP::same(elm1->skey, elm2->skey)) return false;
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
      auto const other_tv = [&] {
        if (elm->hasIntKey()) {
          return NvGetInt(ad2, elm->ikey);
        } else {
          assertx(elm->hasStrKey());
          return NvGetStr(ad2, elm->skey);
        }
      }();
      if (!other_tv.is_init() ||
          !tvEqual(tvAssertPlausible(elm->data),
                   tvAssertPlausible(other_tv))) {
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
