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

#include "hphp/runtime/base/set-array.h"

#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/tv-uncounted.h"

#include "hphp/util/alloc.h"
#include "hphp/util/hash.h"
#include "hphp/util/trace.h"

#include <folly/portability/Constexpr.h>

namespace HPHP {

TRACE_SET_MOD(runtime);

static_assert(SetArray::computeAllocBytes(SetArray::SmallScale) ==
              kEmptySetArraySize, "");

std::aligned_storage<kEmptySetArraySize, 16>::type s_theEmptySetArray;

struct SetArray::Initializer {
  Initializer() {
    auto const ad = reinterpret_cast<SetArray*>(&s_theEmptySetArray);
    ad->initHash(SetArray::SmallScale);
    ad->m_size = 0;
    ad->m_layout_index = kVanillaLayoutIndex;
    ad->m_scale_used = SetArray::SmallScale;
    ad->initHeader(HeaderKind::Keyset, StaticValue);
    assertx(ad->checkInvariants());
  }
};
SetArray::Initializer SetArray::s_initializer;

//////////////////////////////////////////////////////////////////////

SetArray* SetArray::asSet(ArrayData* ad) {
  assertx(ad->isVanillaKeyset());
  auto a = static_cast<SetArray*>(ad);
  assertx(a->checkInvariants());
  return a;
}

const SetArray* SetArray::asSet(const ArrayData* ad) {
  assertx(ad->isVanillaKeyset());
  auto a = static_cast<const SetArray*>(ad);
  assertx(a->checkInvariants());
  return a;
}

bool SetArray::ClearElms(Elm* elms, uint32_t count) {
  static_assert(static_cast<uint32_t>(Elm::kEmpty) == 0,
                "ClearElms zeroes elements.");
  memset(elms, 0, count * sizeof(Elm));
  return true;
}

//////////////////////////////////////////////////////////////////////

ArrayData* SetArray::MakeReserveSet(uint32_t size) {
  auto const scale = computeScaleFromSize(size);
  auto const ad    = reqAlloc(scale);

  assertx(ClearElms(Data(ad), Capacity(scale)));

  ad->initHash(scale);
  ad->initHeader(HeaderKind::Keyset, OneReference);
  ad->m_size          = 0;
  ad->m_layout_index  = kVanillaLayoutIndex;
  ad->m_scale_used    = scale;               // scale = scale, used = 0

  assertx(ad->kind() == kKeysetKind);
  assertx(!ad->isZombie());
  assertx(ad->m_size == 0);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_scale == scale);
  assertx(ad->m_used == 0);
  assertx(ad->checkInvariants());
  return ad;
}

ArrayData* SetArray::MakeSet(uint32_t size, const TypedValue* values) {
  for (uint32_t i = 0; i < size; ++i) {
    auto& tv = values[i];
    if (!isIntType(tv.m_type) &&
        !isStringType(tv.m_type) &&
        !isClassType(tv.m_type) &&
        !isLazyClassType(tv.m_type))  {
      throwInvalidArrayKeyException(&tv, ArrayData::CreateKeyset());
    }
  }
  auto ad = asSet(MakeReserveSet(size));
  for (uint32_t i = 0; i < size; ++i) {
    auto& tv = values[size - i - 1];
    /*
     * We have to use insert here because it is possible
     * that the passed values contain duplicates.
     */
    if (isIntType(tv.m_type)) {
      ad->insert(tv.m_data.num);
    } else if (isStringType(tv.m_type)) {
      ad->insert<false>(tv.m_data.pstr);
      decRefStr(tv.m_data.pstr); // FIXME
    } else if (isClassType(tv.m_type)) {
      auto const keyStr =
        const_cast<StringData*>(classToStringHelper(tv.m_data.pclass));
      ad->insert<false>(keyStr);
    } else {
      assertx(isLazyClassType(tv.m_type));
      auto const keyStr =
        const_cast<StringData*>(lazyClassToStringHelper(tv.m_data.plazyclass));
      ad->insert<false>(keyStr);
    }
  }
  return ad;
}

ArrayData* SetArray::MakeUncounted(
    ArrayData* array, const MakeUncountedEnv& env, bool hasApcTv) {
  auto src = asSet(array);
  assertx(!src->empty());
  assertx(src->isRefCounted());

  auto const scale = src->scale();
  auto const used = src->m_used;
  auto const extra = hasApcTv ? sizeof(APCTypedValue) : 0;
  auto const dest = uncountedAlloc(scale, extra);

  assertx(reinterpret_cast<uintptr_t>(dest) % 16 == 0);
  assertx(reinterpret_cast<uintptr_t>(src) % 16 == 0);
  memcpy16_inline(dest, src, sizeof(SetArray) + sizeof(Elm) * used);
  assertx(ClearElms(Data(dest) + used, Capacity(scale) - used));
  CopyHash(HashTab(dest, scale), src->hashTab(), scale);
  dest->initHeader_16(HeaderKind::Keyset, UncountedValue,
                      hasApcTv ? ArrayData::kHasApcTv : 0);

  // Make sure all strings are uncounted.
  auto const elms = dest->data();
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    assertx(!elm.isEmpty());
    if (elm.hasStrKey()) {
      elm.tv.m_type = KindOfPersistentString;
      auto& skey = elm.tv.m_data.pstr;
      skey = MakeUncountedString(skey, env);
    }
  }

  assertx(dest->checkInvariants());
  return dest;
}

SetArray* SetArray::CopySet(const SetArray& other, AllocMode mode) {
  auto const scale = other.m_scale;
  auto const used = other.m_used;
  auto const ad = mode == AllocMode::Request
    ? reqAlloc(scale)
    : staticAlloc(scale);

  assertx(reinterpret_cast<uintptr_t>(ad) % 16 == 0);
  assertx(reinterpret_cast<uintptr_t>(&other) % 16 == 0);
  memcpy16_inline(ad, &other, sizeof(SetArray) + sizeof(Elm) * used);
  assertx(ClearElms(Data(ad) + used, Capacity(scale) - used));
  CopyHash(HashTab(ad, scale), other.hashTab(), scale);
  auto const count = mode == AllocMode::Request ? OneReference : StaticValue;
  ad->initHeader(HeaderKind::Keyset, count);

  // Bump refcounts.
  auto const elms = other.data();
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    assertx(!elm.isEmpty());
    tvIncRefGen(elm.tv);
  }

  assertx(ad->m_kind == HeaderKind::Keyset);
  assertx(ad->m_size == other.m_size);
  assertx(mode == AllocMode::Request ?
          ad->hasExactlyOneRef() : ad->isStatic());
  assertx(ad->m_scale == scale);
  assertx(ad->m_used == used);
  assertx(ad->checkInvariants());
  return ad;
}

ArrayData* SetArray::MakeSetFromAPC(const APCArray* apc) {
  assertx(apc->isKeyset());
  auto const apcSize = apc->size();
  KeysetInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) init.add(apc->getValue(i)->toLocal());
  return init.create();
}

ArrayData* SetArray::AddToSet(ArrayData* ad, int64_t i) {
  auto a = asSet(ad)->prepareForInsert(ad->cowCheck());
  a->insert(i);
  return a;
}

ArrayData* SetArray::AddToSetInPlace(ArrayData* ad, int64_t i) {
  auto a = asSet(ad)->prepareForInsert(false);
  a->insert(i);
  return a;
}

ArrayData* SetArray::AddToSet(ArrayData* ad, StringData* s) {
  auto a = asSet(ad)->prepareForInsert(ad->cowCheck());
  a->insert<false>(s);
  return a;
}

ArrayData* SetArray::AddToSetInPlace(ArrayData* ad, StringData* s) {
  auto a = asSet(ad)->prepareForInsert(false);
  a->insert<false>(s);
  return a;
}

//////////////////////////////////////////////////////////////////////

void SetArray::Release(ArrayData* in) {
  in->fixCountForRelease();
  assertx(in->isRefCounted());
  assertx(in->hasExactlyOneRef());
  auto const ad = asSet(in);

  if (!ad->isZombie()) {
    auto const elms = ad->data();
    auto const used = ad->m_used;
    for (uint32_t i = 0; i < used; ++i) {
      auto& elm = elms[i];
      // It is OK to decref a TypedValue with kInvalidDataType.  It will appear
      // uncounted.
      assertx(!elm.isEmpty());
      tvDecRefGen(&elm.tv);
    }
  }
  tl_heap->objFree(ad, ad->heapSize());
  AARCH64_WALKABLE_FRAME();
}

void SetArray::ReleaseUncounted(ArrayData* in) {
  assertx(!in->uncountedCowCheck());
  auto const ad = asSet(in);

  if (!ad->isZombie()) {
    auto const elms = ad->data();
    auto const used = ad->m_used;
    for (uint32_t i = 0; i < used; ++i) {
      auto& elm = elms[i];
      if (UNLIKELY(elm.isTombstone())) continue;
      assertx(!elm.isEmpty());
      if (elm.hasStrKey()) DecRefUncountedString(elm.strKey());
    }
  }

  auto const extra = uncountedAllocExtra(ad, ad->hasApcTv());
  FreeUncounted(reinterpret_cast<char*>(ad) - extra,
                computeAllocBytes(ad->scale()) + extra);
}

//////////////////////////////////////////////////////////////////////

void SetArray::insert(int64_t k, inthash_t h) {
  assertx(!isFull());
  auto const loc = findForInsert(k, h);
  if (isValidIns(loc)) {
    auto elm = allocElm(loc);
    elm->setIntKey(k, h);
  }
}
void SetArray::insert(int64_t k) { return insert(k, hash_int64(k)); }

template <bool Move>
void SetArray::insert(StringData* k, strhash_t h) {
  assertx(!isFull());
  auto const loc = findForInsert(k, h);
  if (isValidIns(loc)) {
    auto elm = allocElm(loc);
    elm->setStrKey<Move>(k, h);
  }
}
template <bool Move>
void SetArray::insert(StringData* k) { return insert<Move>(k, k->hash()); }

void SetArray::erase(RemovePos pos) {
  assertx(pos.valid() && pos.elmIdx < m_used);
  hashTab()[pos.probeIdx] = Tombstone;

  auto const elms = data();
  auto& elm = elms[pos.elmIdx];
  assertx(!elm.isInvalid());
  tvDecRefGen(&elm.tv);
  elm.setTombstone();
  --m_size;

  if (m_size <= m_used / 2) {
    /*
     * Compact the array when the load become <=.5.
     */
    compact();
  }
}

//////////////////////////////////////////////////////////////////////

TypedValue SetArray::getElm(ssize_t ei) const {
  assertx(0 <= ei && ei < m_used);
  return data()[ei].getKey();
}

//////////////////////////////////////////////////////////////////////

NEVER_INLINE
SetArray* SetArray::grow(bool copy) {
  assertx(m_size > 0);
  auto const oldUsed = m_used;
  auto const newScale = m_scale * 2;
  assertx(Capacity(newScale) >= m_size);
  assertx(newScale >= SmallScale && (newScale & (newScale - 1)) == 0);

  auto ad              = reqAlloc(newScale);
  ad->m_size           = m_size;
  ad->m_layout_index   = kVanillaLayoutIndex;
  ad->m_scale_used     = newScale | (uint64_t{oldUsed} << 32);
  ad->initHeader(HeaderKind::Keyset, OneReference);

  assertx(reinterpret_cast<uintptr_t>(Data(ad)) % 16 == 0);
  assertx(reinterpret_cast<uintptr_t>(data()) % 16 == 0);
  memcpy16_inline(Data(ad), data(), sizeof(Elm) * oldUsed);
  assertx(ClearElms(Data(ad) + oldUsed, Capacity(newScale) - oldUsed));

  auto const table = ad->initHash(newScale);
  auto const mask = ad->mask();
  auto iter = data();
  auto const stop = iter + oldUsed;
  if (copy) {
    // we need to bump refcounts and insert into the new hash
    for (uint32_t i = 0; iter != stop; ++iter, ++i) {
      auto& e = *iter;
      if (UNLIKELY(e.isTombstone())) continue;
      tvIncRefGen(e.tv);
      *ad->findForNewInsert(table, mask, e.hash()) = i;
    }
  } else {
    // we can make this a zombie (no need to adjust refcounts), but we still
    // need to insert into the new hash
    for (uint32_t i = 0; iter != stop; ++iter, ++i) {
      auto& e = *iter;
      if (UNLIKELY(e.isTombstone())) continue;
      *ad->findForNewInsert(table, mask, e.hash()) = i;
    }
    setZombie();
  }

  assertx(ad->hasExactlyOneRef());
  assertx(ad->kind() == kind());
  assertx(ad->m_size == m_size);
  assertx(ad->m_scale == newScale);
  assertx(ad->m_used == oldUsed);
  assertx(ad->checkInvariants());
  return ad;
}

ALWAYS_INLINE
SetArray* SetArray::prepareForInsert(bool copy) {
  assertx(checkInvariants());
  if (isFull()) return grow(copy);
  if (copy) return copySet();
  return this;
}

void SetArray::compact() {
  auto const elms = data();
  auto const table = initHash(m_scale);
  auto const mask = this->mask();
  uint32_t j = 0;
  auto const used = m_used;
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (elm.isTombstone()) continue;
    assertx(!elm.isEmpty());
    if (j != i) elms[j] = elms[i];
    *findForNewInsert(table, mask, elm.hash()) = j;
    ++j;
  }
  assertx(ClearElms(elms + j, m_used - j));

  m_used = j;
  assertx(m_size == m_used);
  assertx(checkInvariants());
}

//////////////////////////////////////////////////////////////////////

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
 *   m_used = UINT_MAX
 *
 * Non-zombie:
 *
 *   m_size <= m_used
 *   m_used <= capacity()
 */
bool SetArray::checkInvariants() const {
  static_assert(sizeof(SetArray) % 16 == 0, "Some memcpy16 can fail.");
  static_assert(sizeof(Elm) <= 16, "Don't loose the precious memory gainz!");

  // All arrays:
  assertx(checkCount());
  assertx(m_scale >= 1 && (m_scale & (m_scale - 1)) == 0);
  assertx(HashSize(m_scale) == folly::nextPowTwo<uint64_t>(capacity()));
  assertx(m_layout_index == kVanillaLayoutIndex);

  if (isZombie()) return true;

  // Non-zombie:
  assertx(m_size <= m_used);
  assertx(m_used <= capacity());

#if 0
  /*
   * This is very expensive, but nice for debugging.
   */
  uint32_t nonempty = 0;
  auto const elms = data();
  for (uint32_t i = 0; i < capacity(); ++i) {
    auto& elm = elms[i];
    if (i >= m_used) {
      assertx(elm.isEmpty());
    } else {
      assertx(!elm.isEmpty());
      if (!elm.isTombstone()) nonempty++;
    }
  }
  assertx(nonempty == m_size);

  uint32_t nused = 0;
  nonempty = 0;
  auto const hash = hashTab();
  nused = 0;
  for (uint32_t i = 0; i < HashSize(m_scale); ++i) {
    if (hash[i] != Empty) {
      if (hash[i] != Tombstone) {
        assertx(hash[i] < m_used);
        nonempty++;
      }
      nused++;
    }
  }
  assertx(nused == m_used);
  assertx(nonempty == m_size);
#endif

  return true;
}

//////////////////////////////////////////////////////////////////////

const TypedValue* SetArray::tvOfPos(uint32_t pos) const {
  assertx(validPos(ssize_t(pos)));
  if (size_t(pos) >= m_used) return nullptr;
  auto& elm = data()[pos];
  return !elm.isTombstone() ? &elm.tv : nullptr;
}

TypedValue SetArray::GetPosVal(const ArrayData* ad, ssize_t pos) {
  auto a = asSet(ad);
  assertx(0 <= pos && pos < a->m_used);
  return *a->tvOfPos(pos);
}

bool SetArray::IsVectorData(const ArrayData* ad) {
  auto a = asSet(ad);
  if (a->m_size == 0) {
    // any 0-length array is "vector-like" for the sake of this function.
    return true;
  }
  auto const elms = a->data();
  int64_t i = 0;
  for (uint32_t pos = 0, limit = a->m_used; pos < limit; ++pos) {
    auto const& elm = elms[pos];
    if (elm.isTombstone()) {
      continue;
    }
    if (elm.hasStrKey() || elm.intKey() != i) {
      return false;
    }
    ++i;
  }
  return true;
}

bool SetArray::ExistsInt(const ArrayData* ad, int64_t k) {
  return asSet(ad)->findForExists(k, hash_int64(k));
}

bool SetArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  return NvGetStr(ad, k).is_init();
}

arr_lval SetArray::LvalInt(ArrayData*, int64_t) {
  throwInvalidKeysetOperation();
}

arr_lval SetArray::LvalStr(ArrayData*, StringData*) {
  throwInvalidKeysetOperation();
}

ArrayData* SetArray::SetIntMove(ArrayData*, int64_t, TypedValue) {
  throwInvalidKeysetOperation();
}

ArrayData* SetArray::SetStrMove(ArrayData*, StringData*, TypedValue) {
  throwInvalidKeysetOperation();
}

template<class K> ArrayData*
SetArray::RemoveImpl(ArrayData* ad, K k, bool copy, SetArrayElm::hash_t h) {
  auto a = asSet(ad);
  auto const pos = a->findForRemove(k, h);
  if (!pos.valid()) return a;
  if (copy) a = a->copySet();
  a->erase(pos);
  return a;
}

ArrayData* SetArray::RemoveInt(ArrayData* ad, int64_t k) {
  return RemoveImpl(ad, k, ad->cowCheck(), hash_int64(k));
}

ArrayData* SetArray::RemoveStr(ArrayData* ad, const StringData* k) {
  return RemoveImpl(ad, k, ad->cowCheck(), k->hash());
}

void SetArray::Sort(ArrayData*, int, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (sort)"
  );
}

bool SetArray::Usort(ArrayData*, const Variant&) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (usort)"
  );
}

ArrayData* SetArray::Copy(const ArrayData* ad) {
  auto a = asSet(ad);
  return a->copySet();
}

ArrayData* SetArray::CopyStatic(const ArrayData* ad) {
  auto a = asSet(ad);
  return CopySet(*a, AllocMode::Static);
}

ArrayData* SetArray::AppendImpl(ArrayData* ad, TypedValue v, bool copy) {
  auto const res = [&] {
    if (isIntType(v.m_type)) {
      auto a = asSet(ad)->prepareForInsert(copy);
      a->insert(v.m_data.num);
      return a;
    } else if (isStringType(v.m_type)) {
      auto a = asSet(ad)->prepareForInsert(copy);
      a->insert<true>(v.m_data.pstr);
      return a;
    } else {
      throwInvalidArrayKeyException(&v, ad);
    }
  }();
  if (res != ad && ad->decReleaseCheck()) SetArray::Release(ad);
  return res;
}

ArrayData* SetArray::AppendMove(ArrayData* ad, TypedValue v) {
  return AppendImpl(ad, tvClassToString(v), ad->cowCheck());
}

ArrayData* SetArray::Pop(ArrayData* ad, Variant& value) {
  auto a = asSet(ad);
  if (a->cowCheck()) a = a->copySet();
  if (a->m_size) {
    ssize_t pos = a->getIterLast();
    tvDup(a->getElm(pos), *value.asTypedValue());
    auto const pelm = &a->data()[pos];
    auto const loc = a->findForRemove(pelm->hash(),
      [pelm] (const Elm& e) { return &e == pelm; }
    );
    assertx(loc.valid());
    a->erase(loc);
  } else {
    value = uninit_null();
  }
  return a;
}

void SetArray::OnSetEvalScalar(ArrayData* ad) {
  auto a = asSet(ad);
  auto const used = a->m_used;
  auto const elms = a->data();
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    assertx(!elm.isEmpty());
    tvAsVariant(&elm.tv).setEvalScalar();
  }
}

ALWAYS_INLINE
bool SetArray::EqualHelper(const ArrayData* ad1, const ArrayData* ad2,
                           bool strict) {
  assertx(asSet(ad1)->checkInvariants());
  assertx(asSet(ad2)->checkInvariants());

  if (ad1 == ad2) return true;
  if (ad1->size() != ad2->size()) return false;

  if (strict) {
    auto const a1 = asSet(ad1);
    auto const a2 = asSet(ad2);
    auto elm1 = a1->data();
    auto elm2 = a2->data();
    auto i1 = a1->m_used;
    auto i2 = a2->m_used;
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
        if (elm1->intKey() != elm2->intKey()) return false;
      } else {
        assertx(elm1->hasStrKey());
        if (!elm2->hasStrKey()) return false;
        if (!HPHP::same(elm1->strKey(), elm2->strKey())) return false;
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
    auto const a1 = asSet(ad1);
    auto const used = a1->m_used;
    auto const elms = a1->data();
    for (uint32_t i = 0; i < used; ++i) {
      auto& elm = elms[i];
      if (UNLIKELY(elm.isTombstone())) continue;
      if (elm.hasIntKey()) {
        if (!ExistsInt(ad2, elm.intKey())) return false;
      } else {
        if (!ExistsStr(ad2, elm.strKey())) return false;
      }
    }
  }

  return true;
}

bool SetArray::Equal(const ArrayData* ad1, const ArrayData* ad2) {
  return EqualHelper(ad1, ad2, false);
}

bool SetArray::NotEqual(const ArrayData* ad1, const ArrayData* ad2) {
  return !EqualHelper(ad1, ad2, false);
}

bool SetArray::Same(const ArrayData* ad1, const ArrayData* ad2) {
  return EqualHelper(ad1, ad2, true);
}

bool SetArray::NotSame(const ArrayData* ad1, const ArrayData* ad2) {
  return !EqualHelper(ad1, ad2, true);
}

} // namespace HPHP
