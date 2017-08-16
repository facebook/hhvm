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
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-iterator-defs.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/member-val.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"

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
    ad->m_sizeAndPos = 0;
    ad->m_scale_used = SetArray::SmallScale;
    ad->initHeader(HeaderKind::Keyset, StaticValue);
  }
};
SetArray::Initializer SetArray::s_initializer;

//////////////////////////////////////////////////////////////////////

SetArray* SetArray::asSet(ArrayData* ad) {
  assert(ad->isKeyset());
  auto a = static_cast<SetArray*>(ad);
  assert(a->checkInvariants());
  return a;
}

const SetArray* SetArray::asSet(const ArrayData* ad) {
  assert(ad->isKeyset());
  auto a = static_cast<const SetArray*>(ad);
  assert(a->checkInvariants());
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

  assert(ClearElms(Data(ad), Capacity(scale)));

  ad->initHash(scale);
  ad->initHeader(HeaderKind::Keyset, 1);
  ad->m_sizeAndPos   = 0;                   // size = 0, pos = 0
  ad->m_scale_used   = scale;               // scale = scale, used = 0

  assert(ad->kind() == kKeysetKind);
  assert(!ad->isZombie());
  assert(ad->m_size == 0);
  assert(ad->m_pos == 0);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_scale == scale);
  assert(ad->m_used == 0);
  assert(ad->checkInvariants());
  return ad;
}

ArrayData* SetArray::MakeSet(uint32_t size, const TypedValue* values) {
  for (uint32_t i = 0; i < size; ++i) {
    auto& tv = values[i];
    if (!isIntType(tv.m_type) && !isStringType(tv.m_type)) {
      throwInvalidArrayKeyException(&tv, staticEmptyKeysetArray());
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
    } else {
      ad->insert(tv.m_data.pstr);
      decRefStr(tv.m_data.pstr); // FIXME
    }
  }
  return ad;
}

ArrayData* SetArray::MakeUncounted(ArrayData* array, size_t extra) {
  auto src = asSet(array);
  assertx(!src->empty());
  auto const scale = src->scale();
  auto const used = src->m_used;
  char* mem =
    static_cast<char*>(malloc_huge(extra + computeAllocBytes(scale)));
  auto const dest = reinterpret_cast<SetArray*>(mem + extra);

  assert((extra % 16) == 0);
  assert(reinterpret_cast<uintptr_t>(dest) % 16 == 0);
  assert(reinterpret_cast<uintptr_t>(src) % 16 == 0);
  memcpy16_inline(dest, src, sizeof(SetArray) + sizeof(Elm) * used);
  assert(ClearElms(Data(dest) + used, Capacity(scale) - used));
  CopyHash(HashTab(dest, scale), src->hashTab(), scale);
  dest->m_count = UncountedValue;

  // Make sure all strings are uncounted.
  auto const elms = dest->data();
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    assert(!elm.isEmpty());
    StringData*& skey = elm.tv.m_data.pstr;
    if (elm.hasStrKey()) {
      elm.tv.m_type = KindOfPersistentString;
      if (!skey->isStatic()) {
        if (auto const st = lookupStaticString(skey)) {
          skey = st;
        } else {
          skey = StringData::MakeUncounted(skey->slice());
        }
      }
    }
  }
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().addAPCUncountedBlock();
  }

  return dest;
}

SetArray* SetArray::CopySet(const SetArray& other, AllocMode mode) {
  auto const scale = other.m_scale;
  auto const used = other.m_used;
  auto const ad = mode == AllocMode::Request
    ? reqAlloc(scale)
    : staticAlloc(scale);

  assert(reinterpret_cast<uintptr_t>(ad) % 16 == 0);
  assert(reinterpret_cast<uintptr_t>(&other) % 16 == 0);
  memcpy16_inline(ad, &other, sizeof(SetArray) + sizeof(Elm) * used);
  assert(ClearElms(Data(ad) + used, Capacity(scale) - used));
  CopyHash(HashTab(ad, scale), other.hashTab(), scale);
  RefCount count = mode == AllocMode::Request ? 1 : StaticValue;
  ad->initHeader(HeaderKind::Keyset, count);

  // Bump refcounts.
  auto const elms = other.data();
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    assert(!elm.isEmpty());
    tvIncRefGen(&elm.tv);
  }

  assert(ad->m_kind == HeaderKind::Keyset);
  assert(ad->m_size == other.m_size);
  assert(ad->m_pos == other.m_pos);
  assert(mode == AllocMode::Request ?
         ad->hasExactlyOneRef() : ad->isStatic());
  assert(ad->m_scale == scale);
  assert(ad->m_used == used);
  assert(ad->checkInvariants());
  return ad;
}

SetArray* SetArray::CopyReserve(const SetArray* src, size_t expectedSize) {
  assert(expectedSize >= src->size());
  auto const ad = asSet(MakeReserveSet(expectedSize));
  auto const used = src->m_used;
  auto const elms = src->data();
  auto const table = ad->hashTab();
  auto const mask = ad->mask();
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    assert(!elm.isEmpty());
    auto const loc = ad->findForNewInsert(table, mask, elm.hash());
    auto newElm = ad->allocElm(loc);
    if (elm.hasIntKey()) {
      newElm->setIntKey(elm.intKey(), elm.hash());
    } else {
      newElm->setStrKey(elm.strKey(), elm.hash());
    }
  }
  if (src->m_pos == used) {
    ad->m_pos = ad->m_used;
  } else {
    ad->m_pos = ad->findElm(elms[src->m_pos]);
  }
  assert(ad->kind() == ArrayKind::kKeysetKind);
  assert(ad->m_size == src->m_size);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_used == src->m_size);
  assert(ad->checkInvariants());
  return ad;
}

ArrayData* SetArray::MakeSetFromAPC(const APCArray* apc) {
  assert(apc->isKeyset());
  auto const apcSize = apc->size();
  KeysetInit init{apcSize};
  for (uint32_t i = 0; i < apcSize; ++i) init.add(apc->getValue(i)->toLocal());
  return init.create();
}

ArrayData* SetArray::AddToSet(ArrayData* ad, int64_t i, bool copy) {
  auto a = asSet(ad)->prepareForInsert(copy);
  a->insert(i);
  return a;
}

ArrayData* SetArray::AddToSet(ArrayData* ad, StringData* s, bool copy) {
  auto a = asSet(ad)->prepareForInsert(copy);
  a->insert(s);
  return a;
}

//////////////////////////////////////////////////////////////////////

void SetArray::Release(ArrayData* in) {
  assert(in->isRefCounted());
  assert(in->hasExactlyOneRef());
  auto const ad = asSet(in);

  if (!ad->isZombie()) {
    auto const elms = ad->data();
    auto const used = ad->m_used;
    for (uint32_t i = 0; i < used; ++i) {
      auto& elm = elms[i];
      if (UNLIKELY(elm.isTombstone())) continue;
      assert(!elm.isEmpty());
      tvDecRefGen(&elm.tv);
    }

    // We better not have strong iterators associated with keysets.
    assert(!has_strong_iterator(ad));
  }
  MM().objFree(ad, ad->heapSize());
}

void SetArray::ReleaseUncounted(ArrayData* in, size_t extra) {
  assert(in->isUncounted());
  auto const ad = asSet(in);

  if (!ad->isZombie()) {
    auto const elms = ad->data();
    auto const used = ad->m_used;
    for (uint32_t i = 0; i < used; ++i) {
      auto& elm = elms[i];
      if (UNLIKELY(elm.isTombstone())) continue;
      assert(!elm.isEmpty());
      if (elm.hasStrKey()) {
        auto const skey = elm.strKey();
        assert(!skey->isRefCounted());
        if (skey->isUncounted()) {
          skey->destructUncounted();
        }
      }
    }

    // We better not have strong iterators associated with keysets.
    assert(!has_strong_iterator(ad));
  }
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().removeAPCUncountedBlock();
  }
  free_huge(reinterpret_cast<char*>(ad) - extra);
}

//////////////////////////////////////////////////////////////////////

ssize_t SetArray::findElm(const Elm& e) const {
  assert(!e.isInvalid());
  return e.hasIntKey()
    ? find(e.intKey(), e.hash())
    : find(e.strKey(), e.hash());
}

void SetArray::insert(int64_t k, inthash_t h) {
  assert(!isFull());
  auto const loc = findForInsert(k, h);
  if (isValidIns(loc)) {
    auto elm = allocElm(loc);
    elm->setIntKey(k, h);
  }
}
void SetArray::insert(int64_t k) { return insert(k, hash_int64(k)); }

void SetArray::insert(StringData* k, strhash_t h) {
  assert(!isFull());
  auto const loc = findForInsert(k, h);
  if (isValidIns(loc)) {
    auto elm = allocElm(loc);
    elm->setStrKey(k, h);
  }
}
void SetArray::insert(StringData* k) { return insert(k, k->hash()); }

void SetArray::erase(int32_t pos) {
  assert(pos < m_used);
  assert(0 <= pos);
  auto const elms = data();

  if (m_pos == pos) {
    m_pos = nextElm(elms, pos);
  }

  auto& elm = elms[pos];
  assert(!elm.isInvalid());
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

Cell SetArray::getElm(ssize_t ei) const {
  assert(0 <= ei && ei < m_used);
  return data()[ei].getKey();
}

//////////////////////////////////////////////////////////////////////

NEVER_INLINE
SetArray* SetArray::grow(bool copy) {
  assert(m_size > 0);
  auto const oldUsed = m_used;
  auto const newScale = m_scale * 2;
  assert(Capacity(newScale) >= m_size);
  assert(newScale >= SmallScale && (newScale & (newScale - 1)) == 0);

  auto ad            = reqAlloc(newScale);
  ad->m_sizeAndPos   = m_sizeAndPos;
  ad->m_scale_used   = newScale | (uint64_t{oldUsed} << 32);
  ad->initHeader(*this, 1);

  assert(reinterpret_cast<uintptr_t>(Data(ad)) % 16 == 0);
  assert(reinterpret_cast<uintptr_t>(data()) % 16 == 0);
  memcpy16_inline(Data(ad), data(), sizeof(Elm) * oldUsed);
  assert(ClearElms(Data(ad) + oldUsed, Capacity(newScale) - oldUsed));

  auto const table = ad->initHash(newScale);
  auto const mask = ad->mask();
  auto iter = data();
  auto const stop = iter + oldUsed;
  if (copy) {
    // we need to bump refcounts and insert into the new hash
    for (uint32_t i = 0; iter != stop; ++iter, ++i) {
      auto& e = *iter;
      if (UNLIKELY(e.isTombstone())) continue;
      tvIncRefGen(&e.tv);
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

  assert(ad->hasExactlyOneRef());
  assert(ad->kind() == kind());
  assert(ad->m_size == m_size);
  assert(ad->m_pos == m_pos);
  assert(ad->m_scale == newScale);
  assert(ad->m_used == oldUsed);
  assert(ad->checkInvariants());
  return ad;
}

ALWAYS_INLINE
SetArray* SetArray::prepareForInsert(bool copy) {
  assert(checkInvariants());
  if (isFull()) return grow(copy);
  if (copy) return copySet();
  return this;
}

void SetArray::compact() {
  auto const elms = data();
  Elm posElm;
  if (m_pos < m_used) {
    posElm = elms[m_pos];
  }

  auto const table = initHash(m_scale);
  auto const mask = this->mask();
  uint32_t j = 0;
  auto const used = m_used;
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (elm.isTombstone()) continue;
    assert(!elm.isEmpty());
    if (j != i) elms[j] = elms[i];
    *findForNewInsert(table, mask, elm.hash()) = j;
    ++j;
  }
  assert(ClearElms(elms + j, m_used - j));

  if (m_pos == m_used) {
    m_pos = j;
  } else {
    assert(m_pos < m_used);
    m_pos = findElm(posElm);
  }
  m_used = j;

  assert(m_size == m_used);
  assert(checkInvariants());
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
 *   no MArrayIter's are pointing to this array
 *
 * Non-zombie:
 *
 *   m_size <= m_used
 *   m_used <= capacity()
 *   m_pos and all external iterators can't be on a tombstone
 *     or an empty element
 *
 * kKeysetKind:
 *   0 <= m_pos <= m_used
 */
bool SetArray::checkInvariants() const {
  static_assert(sizeof(SetArray) % 16 == 0, "Some memcpy16 can fail.");
  static_assert(sizeof(Elm) <= 16, "Don't loose the precious memory gainz!");

  // All arrays:
  assert(checkCount());
  assert(m_scale >= 1 && (m_scale & (m_scale - 1)) == 0);
  assert(HashSize(m_scale) == folly::nextPowTwo<uint64_t>(capacity()));

  if (isZombie()) return true;

  // Non-zombie:
  assert(m_size <= m_used);
  assert(m_used <= capacity());
  assert(0 <= m_pos && m_pos <= m_used);
  assert(m_pos == m_used || !data()[m_pos].isInvalid());

#if 0
  /*
   * This is very expensive, but nice for debugging.
   */
  uint32_t nonempty = 0;
  auto const elms = data();
  for (uint32_t i = 0; i < capacity(); ++i) {
    auto& elm = elms[i];
    if (i >= m_used) {
      assert(elm.isEmpty());
    } else {
      assert(!elm.isEmpty());
      if (!elm.isTombstone()) nonempty++;
    }
  }
  assert(nonempty == m_size);

  uint32_t nused = 0;
  nonempty = 0;
  auto const hash = hashTab();
  nused = 0;
  for (uint32_t i = 0; i < HashSize(m_scale); ++i) {
    if (hash[i] != Empty) {
      if (hash[i] != Tombstone) {
        assert(hash[i] < m_used);
        nonempty++;
      }
      nused++;
    }
  }
  assert(nused == m_used);
  assert(nonempty == m_size);
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

member_rval::ptr_u SetArray::NvTryGetInt(const ArrayData* ad, int64_t ki) {
  auto const tv = SetArray::NvGetInt(ad, ki);
  if (UNLIKELY(!tv)) throwOOBArrayKeyException(ki, ad);
  return tv;
}

member_rval::ptr_u SetArray::NvTryGetStr(const ArrayData* ad,
                                        const StringData* ks) {
  auto const ptr = SetArray::NvGetStr(ad, ks);
  if (UNLIKELY(!ptr)) throwOOBArrayKeyException(ks, ad);
  return ptr;
}

size_t SetArray::Vsize(const ArrayData*) { not_reached(); }

member_rval::ptr_u SetArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  auto a = asSet(ad);
  assert(0 <= pos && pos < a->m_used);
  return a->tvOfPos(pos);
}

bool SetArray::IsVectorData(const ArrayData*) {
  return false;
}

bool SetArray::ExistsInt(const ArrayData* ad, int64_t k) {
  auto a = asSet(ad);
  return a->find(k, hash_int64(k)) != -1;
}

bool SetArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  auto a = asSet(ad);
  return a->find(k, k->hash()) != -1;
}

member_lval SetArray::LvalInt(ArrayData*, int64_t, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (lval int)"
  );
}

member_lval SetArray::LvalIntRef(ArrayData* ad, int64_t, bool) {
  throwRefInvalidArrayValueException(ad);
}

member_lval SetArray::LvalStr(ArrayData*, StringData*, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (lval string)"
  );
}

member_lval SetArray::LvalStrRef(ArrayData* ad, StringData*, bool) {
  throwRefInvalidArrayValueException(ad);
}

member_lval SetArray::LvalNew(ArrayData*, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (lval new)"
  );
}

member_lval SetArray::LvalNewRef(ArrayData* ad, bool) {
  throwRefInvalidArrayValueException(ad);
}

ArrayData* SetArray::SetRefInt(ArrayData* ad, int64_t, Variant&, bool) {
  throwRefInvalidArrayValueException(ad);
}

ArrayData* SetArray::SetRefStr(ArrayData* ad, StringData*, Variant&, bool) {
  throwRefInvalidArrayValueException(ad);
}

ArrayData* SetArray::SetInt(ArrayData*, int64_t, Cell, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (set int)"
  );
}

ArrayData* SetArray::SetStr(ArrayData*, StringData*, Cell, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (set string)"
  );
}

ArrayData* SetArray::SetWithRefInt(ArrayData*, int64_t, TypedValue, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (set with ref int)"
  );
}

ArrayData* SetArray::SetWithRefStr(ArrayData*, StringData*, TypedValue, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (set with ref string)"
  );
}

ArrayData* SetArray::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  auto a = asSet(ad);
  if (copy) a = a->copySet();
  auto const h = hash_int64(k);
  auto const loc = a->findForRemove(k, h);
  if (validPos(loc)) {
    a->erase(loc);
  }
  return a;
}

ArrayData* SetArray::RemoveStr(ArrayData* ad, const StringData* k, bool copy) {
  auto a = asSet(ad);
  if (copy) a = a->copySet();
  auto const h = k->hash();
  auto const loc = a->findForRemove(k, h);
  if (validPos(loc)) {
    a->erase(loc);
  }
  return a;
}

bool SetArray::AdvanceMArrayIter(ArrayData*, MArrayIter&) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (strong iteration)"
  );
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

ArrayData* SetArray::Append(ArrayData* ad, Cell v, bool copy) {
  auto a = asSet(ad)->prepareForInsert(copy);
  if (isIntType(v.m_type)) {
    a->insert(v.m_data.num);
    return a;
  } else if (isStringType(v.m_type)) {
    a->insert(v.m_data.pstr);
    return a;
  } else {
    throwInvalidArrayKeyException(&v, ad);
  }
}

ArrayData* SetArray::AppendWithRef(ArrayData* ad, TypedValue v, bool copy) {
  if (tvIsReferenced(v)) throwRefInvalidArrayValueException(ad);
  return Append(ad, tvToInitCell(v), copy);
}

ArrayData* SetArray::AppendRef(ArrayData* ad, Variant&, bool) {
  throwRefInvalidArrayValueException(ad);
}

ArrayData* SetArray::PlusEq(ArrayData* ad, const ArrayData*) {
  assertx(asSet(ad)->checkInvariants());
  throwInvalidAdditionException(ad);
}

ArrayData* SetArray::Merge(ArrayData*, const ArrayData*) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (merge)"
  );
}

ArrayData* SetArray::Pop(ArrayData* ad, Variant& value) {
  auto a = asSet(ad);
  if (a->cowCheck()) a = a->copySet();
  if (a->m_size) {
    ssize_t pos = a->getIterLast();
    cellCopy(a->getElm(pos), *value.asTypedValue());
    auto const pelm = &a->data()[pos];
    auto loc = a->findForRemove(pelm->hash(),
      [pelm] (const Elm& e) { return &e == pelm; }
    );
    assert(loc != Empty);
    a->erase(loc);
  } else {
    value = uninit_null();
  }
  /*
   * To conform to PHP5 behavior, the pop operation resets the array's
   * internal iterator.
   */
  a->m_pos = a->getIterBegin();
  return a;
}

ArrayData* SetArray::Dequeue(ArrayData* ad, Variant& value) {
  auto a = asSet(ad);
  if (a->cowCheck()) a = a->copySet();
  if (a->m_size) {
    ssize_t pos = a->getIterBegin();
    cellCopy(a->getElm(pos), *value.asTypedValue());
    auto const pelm = &a->data()[pos];
    auto loc = a->findForRemove(pelm->hash(),
      [pelm] (const Elm& e) { return &e == pelm; }
    );
    assert(loc != Empty);
    a->erase(loc);
  } else {
    value = uninit_null();
  }
  /*
   * To conform to PHP5 behavior, the shift operation resets the array's
   * internal iterator.
   */
  a->m_pos = a->getIterBegin();
  return a;
}

ArrayData* SetArray::Prepend(ArrayData* ad, Cell v, bool /*copy*/) {
  auto a = asSet(ad);
  Elm e;
  assert(ClearElms(&e, 1));
  if (isIntType(v.m_type)) {
    e.setIntKey(v.m_data.num, hash_int64(v.m_data.num));
  } else if (isStringType(v.m_type)) {
    e.setStrKey(v.m_data.pstr, v.m_data.pstr->hash());
  } else {
    throwInvalidArrayKeyException(&v, ad);
  }

  auto elms = a->data();
  if (!elms[0].isTombstone()) {
    a = a->prepareForInsert(false);
    elms = a->data();
    memmove(&elms[1], &elms[0], a->m_used * sizeof(Elm));
    ++a->m_used;
    ++a->m_pos;
  }

  ++a->m_size;
  elms[0] = e;
  assert(!elms[0].isInvalid());
  a->compact(); // Rebuild the hash table.
  return a;
}

void SetArray::Renumber(ArrayData*) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (renumbering)"
  );
}

void SetArray::OnSetEvalScalar(ArrayData* ad) {
  auto a = asSet(ad);
  auto const used = a->m_used;
  auto const elms = a->data();
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    assert(!elm.isEmpty());
    tvAsVariant(&elm.tv).setEvalScalar();
  }
}

ArrayData* SetArray::Escalate(const ArrayData* ad) {
  return const_cast<ArrayData*>(ad);
}

ArrayData* SetArray::ToPHPArray(ArrayData* ad, bool) {
  auto a = asSet(ad);
  auto size = a->size();

  if (!size) return staticEmptyArray();

  ArrayInit init{size, ArrayInit::Map{}};

  auto const elms = a->data();
  auto const used = a->m_used;
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    if (elm.hasIntKey()) {
      init.set(elm.intKey(), tvAsCVarRef(&elm.tv));
    } else {
      auto const key = elm.strKey();
      int64_t n;
      if (key->isStrictlyInteger(n)) {
        init.set(n, make_tv<KindOfInt64>(n));
      } else {
        init.set(key, tvAsCVarRef(&elm.tv));
      }
    }
  }

  return init.create();
}

ArrayData* SetArray::ToKeyset(ArrayData* ad, bool /*copy*/) {
  assertx(asSet(ad)->checkInvariants());
  return ad;
}

ALWAYS_INLINE
bool SetArray::EqualHelper(const ArrayData* ad1, const ArrayData* ad2,
                           bool strict) {
  assert(asSet(ad1)->checkInvariants());
  assert(asSet(ad2)->checkInvariants());

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
        if (!same(elm1->strKey(), elm2->strKey())) return false;
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
        if (!NvGetInt(ad2, elm.intKey())) return false;
      } else {
        if (!NvGetStr(ad2, elm.strKey())) return false;
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
