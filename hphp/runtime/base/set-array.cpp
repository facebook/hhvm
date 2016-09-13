/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-iterator-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/apc-array.h"

#include "hphp/util/alloc.h"
#include "hphp/util/hash.h"
#include "hphp/util/trace.h"

#include <folly/portability/Constexpr.h>

namespace HPHP {

TRACE_SET_MOD(runtime);

std::aligned_storage<
  SetArray::ComputeAllocBytes(SetArray::SmallScale),
  folly::constexpr_max(alignof(SetArray), size_t(16))
>::type s_theEmptySetArray;

struct SetArray::Initializer {
  Initializer() {
    auto const ad = reinterpret_cast<SetArray*>(&s_theEmptySetArray);
    auto const hash = SetHashTab(ad, SetArray::SmallScale);
    InitHash(hash, SetArray::SmallScale);
    ad->m_sizeAndPos = 0;
    ad->m_scale_used = SetArray::SmallScale;
    ad->m_hdr.init(HeaderKind::Keyset, StaticValue);
  }
};
SetArray::Initializer SetArray::s_initializer;

//////////////////////////////////////////////////////////////////////

namespace {

inline uint32_t keysetComputeScaleFromSize(uint32_t n) {
  assert(n <= 0x7fffffffU);
  auto scale = SetArray::SmallScale;
  while (SetArray::Capacity(scale) < n) scale *= 2;
  return scale;
}

SetArray* keysetReqAllocSet(uint32_t scale) {
  auto const allocBytes = SetArray::ComputeAllocBytes(scale);
  return static_cast<SetArray*>(MM().objMalloc(allocBytes));
}

SetArray* keysetStaticAllocSet(uint32_t scale) {
  auto const allocBytes = SetArray::ComputeAllocBytes(scale);
  return static_cast<SetArray*>(std::malloc(allocBytes));
}

} // namespace

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

void SetArray::InitHash(uint32_t* table, uint32_t scale) {
  // wordfill(table, Empty, HashSize(scale));
  // wordfill does not work here because we use uint32_t...
  memset(table, -1, HashSize(scale) * sizeof(uint32_t));
}

void SetArray::CopyHash(uint32_t* dest, uint32_t* src, uint32_t scale) {
  uint64_t bytes = HashSize(scale) * sizeof(uint32_t);
  assert(bytes % 16 == 0);
  memcpy16_inline(dest, src, bytes);
}

bool SetArray::ClearElms(Elm* elms, uint32_t count) {
  static_assert(static_cast<uint32_t>(Elm::kEmpty) == 0,
                "ClearElms zeroes elements.");
  memset(elms, 0, count * sizeof(Elm));
  return true;
}

bool SetArray::isFull() const {
  assert(m_used <= capacity());
  return m_used == capacity();
}

//////////////////////////////////////////////////////////////////////

ArrayData* SetArray::MakeReserveSet(uint32_t size) {
  auto const scale = keysetComputeScaleFromSize(size);
  auto const ad    = keysetReqAllocSet(scale);

  auto const hash = SetHashTab(ad, scale);
  assert(ClearElms(SetData(ad), Capacity(scale)));
  InitHash(hash, scale);

  ad->m_hdr.init(HeaderKind::Keyset, 1);
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
  auto a = asSet(array);
  assertx(!a->empty());
  auto const scale = a->scale();
  auto const used = a->m_used;
  char* mem = static_cast<char*>(std::malloc(extra + ComputeAllocBytes(scale)));
  auto const ad = reinterpret_cast<SetArray*>(mem + extra);

  assert((extra % 16) == 0);
  assert(reinterpret_cast<uintptr_t>(ad) % 16 == 0);
  assert(reinterpret_cast<uintptr_t>(a) % 16 == 0);
  memcpy16_inline(ad, a, sizeof(SetArray) + sizeof(Elm) * used);
  assert(ClearElms(SetData(ad) + used, Capacity(scale) - used));
  CopyHash(SetHashTab(ad, scale), a->hashTab(), scale);
  ad->m_hdr.count = UncountedValue;

  // Make sure all strings are uncounted.
  auto const elms = a->data();
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    assert(!elm.isEmpty());
    StringData*& skey = elm.tv.m_data.pstr;
    if (elm.hasStrKey() && !skey->isStatic()) {
      elm.tv.m_type = KindOfPersistentString;
      if (auto const st = lookupStaticString(skey)) {
        skey = st;
      } else {
        skey = StringData::MakeUncounted(skey->slice());
      }
    }
  }
  return ad;
}

SetArray* SetArray::CopySet(const SetArray& other, AllocMode mode) {
  auto const scale = other.m_scale;
  auto const used = other.m_used;
  auto const ad = mode == AllocMode::Request
    ? keysetReqAllocSet(scale)
    : keysetStaticAllocSet(scale);

  assert(reinterpret_cast<uintptr_t>(ad) % 16 == 0);
  assert(reinterpret_cast<uintptr_t>(&other) % 16 == 0);
  memcpy16_inline(ad, &other, sizeof(SetArray) + sizeof(Elm) * used);
  assert(ClearElms(SetData(ad) + used, Capacity(scale) - used));
  CopyHash(SetHashTab(ad, scale), other.hashTab(), scale);
  RefCount count = mode == AllocMode::Request ? 1 : StaticValue;
  ad->m_hdr.init(HeaderKind::Keyset, count);

  // Bump refcounts.
  auto const elms = other.data();
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    assert(!elm.isEmpty());
    tvRefcountedIncRef(&elm.tv);
  }

  assert(ad->m_hdr.kind == HeaderKind::Keyset);
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
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    assert(!elm.isEmpty());
    auto const loc = ad->findForNewInsert(elm.hash());
    auto const newElm = ad->allocElm(loc);
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
  auto a = asSet(ad);
  a = copy ? a->copyAndResizeIfNeeded() : a->resizeIfNeeded();
  a->insert(i);
  return a;
}

ArrayData* SetArray::AddToSet(ArrayData* ad, StringData* s, bool copy) {
  auto a = asSet(ad);
  a = copy ? a->copyAndResizeIfNeeded() : a->resizeIfNeeded();
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
      tvRefcountedDecRef(&elm.tv);
    }

    /*
     * We better not have strong iterators associated with keysets.
     */
    if (debug && UNLIKELY(strong_iterators_exist())) {
      for_each_strong_iterator([&] (const MIterTable::Ent& miEnt) {
        assert(miEnt.array != ad);
      });
    }
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

    /*
     * We better not have strong iterators associated with keysets.
     */
    if (debug && UNLIKELY(strong_iterators_exist())) {
      for_each_strong_iterator([&] (const MIterTable::Ent& miEnt) {
        assert(miEnt.array != ad);
      });
    }
  }
  std::free(reinterpret_cast<char*>(ad) - extra);
}

//////////////////////////////////////////////////////////////////////

template <SetArray::FindType type, class Hit>
typename std::conditional<
  type == SetArray::FindType::Lookup,
  ssize_t,
  uint32_t*
>::type SetArray::findImpl(hash_t h0, Hit hit) const {
  static_assert(
    static_cast<int>(FindType::Lookup) == 0 &&
    static_cast<int>(FindType::Insert) == 1 &&
    static_cast<int>(FindType::Remove) == 2,
    "Update the tuple accessing code below."
  );
  auto const mask = this->mask();
  auto const hash = hashTab();
  auto const elms = data();

  for (uint32_t probeIndex = h0, i = 1;; ++i) {
    uint32_t idx = probeIndex & mask;
    auto const ei = hash[idx];

    if (ei == Empty) {
      return std::get<static_cast<int>(type)>(
        std::make_tuple(ssize_t(-1), &hash[idx], nullptr)
      );
    }

    if (LIKELY(ei != Tombstone)) {
      assert(0 <= ei && ei < m_used);
      auto& elm = elms[ei];
      assert(!elm.isInvalid());
      if (hit(elm)) {
        return std::get<static_cast<int>(type)>(
          std::make_tuple(ssize_t(ei), nullptr, &hash[idx])
        );
      }
    }

    probeIndex += i;
    assertx(i <= mask);
    assertx(probeIndex == static_cast<uint32_t>(h0) + (i * (i + 1)) / 2);
  }
}

static bool hitIntKey(const SetArray::Elm& e, int64_t ki) {
  assert(!e.isInvalid());
  return e.hasIntKey() && e.intKey() == ki;
}

static bool hitStrKey(const SetArray::Elm& e, const StringData* s,
                      strhash_t h) {
  assert(!e.isInvalid());
  /*
   * We do not have to check e.hasStrKey() because it is
   * implicitely done by the check on the hash.
   */
  return e.hash() == h && (s == e.strKey() || s->same(e.strKey()));
}

ssize_t SetArray::find(int64_t ki, inthash_t h) const {
  return findImpl<FindType::Lookup>(h, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

ssize_t SetArray::find(const StringData* ks, strhash_t h) const {
  return findImpl<FindType::Lookup>(h, [ks, h] (const Elm& e) {
    return hitStrKey(e, ks, h);
  });
}

ssize_t SetArray::findElm(const Elm& e) const {
  assert(!e.isInvalid());
  return e.hasIntKey()
    ? find(e.intKey(), e.hash())
    : find(e.strKey(), e.hash());
}

template<SetArray::FindType t>
uint32_t* SetArray::findHash(int64_t ki, inthash_t h) const {
  return findImpl<t>(h, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

template<SetArray::FindType t>
uint32_t* SetArray::findHash(const StringData* ks, strhash_t h) const {
  return findImpl<t>(h, [ks, h] (const Elm& e) {
    return hitStrKey(e, ks, h);
  });
}

uint32_t* SetArray::findForNewInsert(hash_t h) const {
  return findImpl<FindType::Insert>(h, [](const Elm&) {
    return false;
  });
}

SetArray::Elm* SetArray::allocElm(uint32_t* loc) {
  assert(m_used < capacity());
  ++m_size;
  return &data()[*loc = m_used++];
}

void SetArray::insert(int64_t k, inthash_t h) {
  assert(!isFull());
  if (auto const loc = findHash<FindType::Insert>(k, h)) {
    auto const elm = allocElm(loc);
    elm->setIntKey(k, h);
  }
}
void SetArray::insert(int64_t k) { return insert(k, hashint(k)); }

void SetArray::insert(StringData* k, strhash_t h) {
  assert(!isFull());
  if (auto const loc = findHash<FindType::Insert>(k, h)) {
    auto const elm = allocElm(loc);
    elm->setStrKey(k, h);
  }
}
void SetArray::insert(StringData* k) { return insert(k, k->hash()); }

void SetArray::erase(uint32_t* loc) {
  auto const pos = *loc;
  *loc = Tombstone;
  assert(pos < m_used);
  auto const elms = data();

  if (m_pos == pos) {
    m_pos = nextElm(elms, pos);
  }

  auto& elm = elms[pos];
  assert(!elm.isInvalid());
  tvRefcountedDecRef(&elm.tv);
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

ssize_t SetArray::getIterBegin() const {
  return nextElm(data(), -1);
}

ssize_t SetArray::getIterLast() const {
  auto elms = data();
  ssize_t ei = m_used;
  while (--ei >= 0) {
    if (!elms[ei].isTombstone()) {
      return ei;
    }
  }
  return m_used;
}

void SetArray::getElm(ssize_t ei, TypedValue* out) const {
  assert(0 <= ei && ei < m_used);
  auto& elm = data()[ei];
  assert(!elm.isInvalid());
  tvDup(elm.tv, *out);
}

ssize_t SetArray::nextElm(Elm* elms, ssize_t ei) const {
  assert(-1 <= ei && ei < m_used);
  while (++ei < m_used) {
    assert(!elms[ei].isEmpty());
    if (LIKELY(!elms[ei].isTombstone())) return ei;
  }
  assert(ei == m_used);
  return m_used;
}

ssize_t SetArray::prevElm(Elm* elms, ssize_t ei) const {
  assert(0 <= ei && ei < m_used);
  while (--ei >= 0) {
    assert(!elms[ei].isEmpty());
    if (LIKELY(!elms[ei].isTombstone())) return ei;
  }
  assert(ei == -1);
  return m_used;
}

//////////////////////////////////////////////////////////////////////

SetArray* SetArray::grow(uint32_t newScale) {
  assert(m_size > 0);
  assert(Capacity(newScale) >= m_size);
  assert(newScale >= SmallScale && (newScale & (newScale - 1)) == 0);

  auto ad            = keysetReqAllocSet(newScale);
  auto const oldData = data();
  auto const oldUsed = m_used;
  ad->m_sizeAndPos   = m_sizeAndPos;
  ad->m_scale_used   = newScale | (uint64_t{oldUsed} << 32);
  ad->m_hdr.init(m_hdr, 1);
  assert(reinterpret_cast<uintptr_t>(SetData(ad)) % 16 == 0);
  assert(reinterpret_cast<uintptr_t>(data()) % 16 == 0);
  memcpy16_inline(SetData(ad), data(), sizeof(Elm) * oldUsed);
  assert(ClearElms(SetData(ad) + oldUsed, Capacity(newScale) - oldUsed));
  InitHash(SetHashTab(ad, newScale), newScale);
  setZombie();

  for (uint32_t i = 0; i < oldUsed; ++i) {
    auto& elm = oldData[i];
    if (UNLIKELY(elm.isTombstone())) continue;
    assert(!elm.isEmpty());
    *ad->findForNewInsert(elm.hash()) = i;
  }

  assert(isZombie());
  assert(ad->hasExactlyOneRef());
  assert(ad->kind() == kind());
  assert(ad->m_size == m_size);
  assert(ad->m_scale == newScale);
  assert(ad->m_used = oldUsed);
  assert(ad->checkInvariants());
  return ad;
}

void SetArray::compact() {
  auto const elms = data();
  Elm posElm;
  if (m_pos < m_used) {
    posElm = elms[m_pos];
  }

  InitHash(hashTab(), m_scale);
  uint32_t j = 0;
  auto const used = m_used;
  for (uint32_t i = 0; i < used; ++i) {
    auto& elm = elms[i];
    if (elm.isTombstone()) continue;
    assert(!elm.isEmpty());
    if (j != i) elms[j] = elms[i];
    *findForNewInsert(elm.hash()) = j;
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

SetArray* SetArray::resize() {
  assert(m_size <= capacity());

  /*
   * If after compaction the new load exceeds 0.5 we
   * grow the array.  This threshold avoids repeated
   * compactions if the load were to hover near 0.75.
   */
  if (m_size > capacity() / 2) {
    /*
     * FIXME: I stole the assert from mixed-array.cpp
     * but don't really understand it.
     */
    assert(mask() <= 0x7fffffffU);
    return grow(m_scale * 2);
  } else {
    compact();
    return this;
  }
}

SetArray* SetArray::resizeIfNeeded() {
  if (UNLIKELY(isFull())) return resize();
  return this;
}

SetArray* SetArray::copyAndResizeIfNeeded() const {
  auto const ad = copySet();
  if (LIKELY(!ad->isFull())) return ad;

  /* FIXME: This is kind of lame. */
  auto const ret = ad->resize();
  if (ad != ret) Release(ad);
  return ret;
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

const TypedValue* SetArray::NvGetInt(const ArrayData* ad, int64_t ki) {
  auto a = asSet(ad);
  auto i = a->find(ki, hashint(ki));
  if (LIKELY(i >= 0)) {
    return a->tvOfPos(i);
  } else {
    assert(i == -1);
    return nullptr;
  }
}

const TypedValue* SetArray::NvGetStr(const ArrayData* ad,
                                     const StringData* ks) {
  auto a = asSet(ad);
  auto i = a->find(ks, ks->hash());
  if (LIKELY(i >= 0)) {
    return a->tvOfPos(i);
  } else {
    assert(i == -1);
    return nullptr;
  }
}

const TypedValue* SetArray::NvTryGetInt(const ArrayData* ad, int64_t ki) {
  auto const tv = SetArray::NvGetInt(ad, ki);
  if (UNLIKELY(!tv)) throwOOBArrayKeyException(ki, ad);
  return tv;
}

const TypedValue* SetArray::NvTryGetStr(const ArrayData* ad,
                                        const StringData* ks) {
  auto const tv = SetArray::NvGetStr(ad, ks);
  if (UNLIKELY(!tv)) throwOOBArrayKeyException(ks, ad);
  return tv;
}

void SetArray::NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
  auto a = asSet(ad);
  assert(0 <= pos  && pos < a->m_used);
  assert(!a->data()[pos].isInvalid());
  a->getElm(pos, out);
}

size_t SetArray::Vsize(const ArrayData*) { not_reached(); }

const Variant& SetArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  auto a = asSet(ad);
  assert(0 <= pos && pos < a->m_used);
  return tvAsCVarRef(a->tvOfPos(pos));
}

bool SetArray::IsVectorData(const ArrayData*) {
  return false;
}

bool SetArray::ExistsInt(const ArrayData* ad, int64_t k) {
  auto a = asSet(ad);
  return a->find(k, hashint(k)) != -1;
}

bool SetArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  auto a = asSet(ad);
  return a->find(k, k->hash()) != -1;
}

ArrayData* SetArray::LvalInt(ArrayData*, int64_t, Variant*&, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (lval int)"
  );
}

ArrayData* SetArray::LvalIntRef(ArrayData* ad, int64_t, Variant*&, bool) {
  throwRefInvalidArrayValueException(ad);
}

ArrayData* SetArray::LvalStr(ArrayData*, StringData*, Variant*&, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (lval string)"
  );
}

ArrayData* SetArray::LvalStrRef(ArrayData* ad, StringData*, Variant*&, bool) {
  throwRefInvalidArrayValueException(ad);
}

ArrayData* SetArray::LvalNew(ArrayData*, Variant*&, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (lval new)"
  );
}

ArrayData* SetArray::LvalNewRef(ArrayData* ad, Variant*&, bool) {
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

ArrayData* SetArray::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  auto a = asSet(ad);
  if (copy) a = a->copySet();
  auto const h = hashint(k);
  if (auto const loc = a->findHash<FindType::Remove>(k, h)) {
    a->erase(loc);
  }
  return a;
}

ArrayData* SetArray::RemoveStr(ArrayData* ad, const StringData* k, bool copy) {
  auto a = asSet(ad);
  if (copy) a = a->copySet();
  auto const h = k->hash();
  if (auto const loc = a->findHash<FindType::Remove>(k, h)) {
    a->erase(loc);
  }
  return a;
}

ssize_t SetArray::IterBegin(const ArrayData* ad) {
  auto a = asSet(ad);
  return a->getIterBegin();
}

ssize_t SetArray::IterLast(const ArrayData* ad) {
  auto a = asSet(ad);
  return a->getIterLast();
}

ssize_t SetArray::IterEnd(const ArrayData* ad) {
  auto a = asSet(ad);
  return a->getIterEnd();
}

ssize_t SetArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  auto a = asSet(ad);
  if (pos == a->getIterEnd()) return pos;
  return a->nextElm(pos);
}

ssize_t SetArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  auto a = asSet(ad);
  if (pos == a->getIterEnd()) return pos;
  return a->prevElm(a->data(), pos);
}

bool SetArray::AdvanceMArrayIter(ArrayData*, MArrayIter&) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (strong iteration)"
  );
}

void SetArray::SortThrow(ArrayData*, int, bool) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (sort)"
  );
}

bool SetArray::USortThrow(ArrayData*, const Variant&) {
  SystemLib::throwInvalidOperationExceptionObject(
    "Invalid keyset operation (usort)"
  );
}

ArrayData* SetArray::Copy(const ArrayData* ad) {
  auto a = asSet(ad);
  return a->copySet();
}

ArrayData* SetArray::CopyWithStrongIterators(const ArrayData* ad) {
  auto a = asSet(ad);
  return a->copySet();
}

ArrayData* SetArray::CopyStatic(const ArrayData* ad) {
  auto a = asSet(ad);
  return CopySet(*a, AllocMode::Static);
}

ArrayData* SetArray::Append(ArrayData* ad, Cell v, bool copy) {
  auto a = asSet(ad);
  a = copy ? a->copyAndResizeIfNeeded() : a->resizeIfNeeded();
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

ArrayData* SetArray::AppendWithRef(ArrayData* ad, const Variant& v, bool copy) {
  if (v.isReferenced()) throwRefInvalidArrayValueException(ad);
  auto const cell = LIKELY(v.getType() != KindOfUninit)
    ? *v.asCell()
    : make_tv<KindOfNull>();
  return Append(ad, cell, copy);
}

ArrayData* SetArray::AppendRef(ArrayData* ad, Variant&, bool) {
  throwRefInvalidArrayValueException(ad);
}

ArrayData* SetArray::PlusEq(ArrayData* ad, const ArrayData* others) {
  for (ArrayIter it(others); !it.end(); it.next()) {
    Variant key = it.first();
    auto tv = key.asTypedValue();
    if (!isIntType(tv->m_type) && !isStringType(tv->m_type)) {
      throwInvalidArrayKeyException(tv, ad);
    }
  }
  auto a = asSet(ad);
  auto const neededSize = a->size() + others->size();
  if (a->cowCheck()) {
    a = CopyReserve(a, neededSize);
  }
  for (ArrayIter it(others); !it.end(); it.next()) {
    if (UNLIKELY(a->isFull())) {
      assert(a == ad);
      a = CopyReserve(a, neededSize);
    }
    Variant key = it.first();
    auto tv = key.asTypedValue();
    if (isIntType(tv->m_type)) {
      a->insert(tv->m_data.num);
    } else {
      a->insert(tv->m_data.pstr);
    }
  }
  return a;
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
    a->getElm(pos, value.asTypedValue());
    auto const pelm = &a->data()[pos];
    auto loc = a->findImpl<FindType::Remove>(pelm->hash(),
      [pelm] (const Elm& e) { return &e == pelm; }
    );
    assert(loc);
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
    a->getElm(pos, value.asTypedValue());
    auto const pelm = &a->data()[pos];
    auto loc = a->findImpl<FindType::Remove>(pelm->hash(),
      [pelm] (const Elm& e) { return &e == pelm; }
    );
    assert(loc);
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

ArrayData* SetArray::Prepend(ArrayData* ad, Cell v, bool copy) {
  auto a = asSet(ad);
  Elm e;
  assert(ClearElms(&e, 1));
  if (isIntType(v.m_type)) {
    e.setIntKey(v.m_data.num, hashint(v.m_data.num));
  } else if (isStringType(v.m_type)) {
    e.setStrKey(v.m_data.pstr, v.m_data.pstr->hash());
  } else {
    throwInvalidArrayKeyException(&v, ad);
  }

  auto elms = a->data();
  if (!elms[0].isTombstone()) {
    a = a->resizeIfNeeded();
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
    StringData*& skey = elm.tv.m_data.pstr;
    if (elm.hasStrKey() && !skey->isStatic()) {
      if (auto const st = lookupStaticString(skey)) {
        skey = st;
      } else {
        skey = StringData::MakeUncounted(skey->slice());
      }
    }
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
      init.add(elm.intKey(), tvAsCVarRef(&elm.tv), /* keyConverted */ true);
    } else {
      init.add(elm.strKey(), tvAsCVarRef(&elm.tv), /* keyConverted */ true);
    }
  }

  return init.create();
}

ArrayData* SetArray::ToKeyset(ArrayData* ad, bool copy) {
  assertx(asSet(ad)->checkInvariants());
  return ad;
}

bool SetArray::Equal(const ArrayData* ad1, const ArrayData* ad2) {
  assert(asSet(ad1)->checkInvariants());
  assert(asSet(ad2)->checkInvariants());

  if (ad1 == ad2) return true;
  if (ad1->size() != ad2->size()) return false;

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

  return true;
}

bool SetArray::NotEqual(const ArrayData* ad1, const ArrayData* ad2) {
  return !Equal(ad1, ad2);
}

} // namespace HPHP
