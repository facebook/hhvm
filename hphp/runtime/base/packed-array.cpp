/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "folly/Likely.h"

#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/runtime-error.h"

#include "hphp/runtime/base/hphp-array-defs.h"
#include "hphp/runtime/base/array-iterator-defs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
bool HphpArray::isFullPacked() const {
  assert(isPacked());
  assert(m_size <= m_cap);
  return m_size == m_cap;
}

ALWAYS_INLINE
TypedValue& HphpArray::allocNextElm(uint32_t i) {
  assert(isPacked() && i == m_size);
  assert(!isFullPacked());
  auto next = i + 1;
  if (m_pos == invalid_index) m_pos = i;
  m_used = m_size = next;
  return data()[i].data;
}

NEVER_INLINE
HphpArray* HphpArray::copyPacked() const {
  assert(checkInvariants());
  return CopyPacked(
    *this,
    AllocMode::Smart,
    [&] (const TypedValue* fr, TypedValue* to, const ArrayData* container) {
      tvDupFlattenVars(fr, to, container);
    }
  );
}

NEVER_INLINE
HphpArray* HphpArray::copyPackedAndResizeIfNeededSlow() const {
  assert(isFullPacked());
  // Note: this path will have to handle splitting strong iterators
  // later when we combine copyPacked & GrowPacked into one operation.
  // For now I'm just making use of copyPacked to do it for me before
  // GrowPacked happens.
  auto const copy = copyPacked();
  auto const ret  = GrowPacked(copy);
  assert(ret != copy);
  assert(copy->getCount() == 0);
  PackedArray::Release(copy);
  return ret;
}

ALWAYS_INLINE
HphpArray* HphpArray::copyPackedAndResizeIfNeeded() const {
  if (LIKELY(!isFullPacked())) return copyPacked();
  return copyPackedAndResizeIfNeededSlow();
}

NEVER_INLINE
HphpArray* HphpArray::packedToMixed() {
  assert(isPacked());

  auto const size      = m_size;
  auto const tableMask = m_tableMask;
  auto pdata           = data();
  auto hash            = reinterpret_cast<int32_t*>(pdata + m_cap);

  m_kind   = kMixedKind;
  m_hLoad  = size;
  m_nextKI = size;

  uint32_t i = 0;
  for (; i < size; ++i) {
    pdata->setIntKey(i);
    *hash = i;
    ++pdata;
    ++hash;
  }
  for (; i <= tableMask; ++i) {
    *hash = Empty;
    ++hash;
  }

  assert(checkInvariants());
  return this;
}

NEVER_INLINE
HphpArray* HphpArray::GrowPacked(HphpArray* old) {
  assert(old->isPacked());
  assert(old->m_cap == old->m_used);

  DEBUG_ONLY auto const oldSize = old->m_size;
  DEBUG_ONLY auto const oldPos  = old->m_pos;

  auto const oldCap     = old->m_cap;
  auto const oldMask    = old->m_tableMask;
  auto const cap        = oldCap * 2;
  auto const mask       = oldMask * 2 + 1;
  auto const ad         = smartAllocArray(cap, mask);

  auto const oldUsed        = old->m_used;
  auto const oldKindAndSize = old->m_kindAndSize;
  auto const oldPosUnsigned = uint64_t{static_cast<uint32_t>(old->m_pos)};

  ad->m_kindAndSize     = oldKindAndSize;
  ad->m_posAndCount     = oldPosUnsigned;
  ad->m_capAndUsed      = uint64_t{oldUsed} << 32 | cap;
  ad->m_tableMask       = mask;

  if (UNLIKELY(strong_iterators_exist())) {
    move_strong_iterators(ad, old);
  }

  // Steal the old array payload.
  old->m_used = -uint32_t{1};
  copyElms(ad->data(), old->data(), oldUsed);

  // TODO(#2926276): it would be good to refactor callers to expect
  // our refcount to start at 1.

  assert(old->isZombie());
  assert(ad->m_kind == kPackedKind);
  assert(ad->m_pos == oldPos);
  assert(ad->m_count == 0);
  assert(ad->m_used == oldUsed);
  assert(ad->m_cap == cap);
  assert(ad->m_size == oldSize);
  assert(ad->checkInvariants());
  return ad;
}

ALWAYS_INLINE
HphpArray* HphpArray::resizePackedIfNeeded() {
  if (UNLIKELY(isFullPacked())) return GrowPacked(this);
  return this;
}

//////////////////////////////////////////////////////////////////////

NEVER_INLINE
void PackedArray::Release(ArrayData* in) {
  assert(in->isRefCounted());
  auto const ad = asPacked(in);

  if (!ad->isZombie()) {
    auto const data = ad->data();
    auto const stop = data + ad->m_used;

    for (auto ptr = data; ptr != stop; ++ptr) {
      tvRefcountedDecRef(ptr->data);
    }

    if (UNLIKELY(strong_iterators_exist())) {
      free_strong_iterators(ad);
    }
  }

  auto const cap  = ad->m_cap;
  auto const mask = ad->m_tableMask;
  MM().objFreeLogged(ad, computeAllocBytes(cap, mask));
}

TypedValue* PackedArray::NvGetInt(const ArrayData* ad, int64_t ki) {
  auto a = asPacked(ad);
  return LIKELY(size_t(ki) < a->m_size) ? &a->data()[ki].data : nullptr;
}

TypedValue* PackedArray::NvGetStr(const ArrayData* ad, const StringData* k) {
  assert(asPacked(ad));
  return nullptr;
}

// nvGetKey does not touch out->_count, so can be used
// for inner or outer cells.
void PackedArray::NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
  DEBUG_ONLY auto a = asPacked(ad);
  assert(pos != ArrayData::invalid_index);
  assert(!HphpArray::isTombstone(a->data()[pos].data.m_type));
  out->m_data.num = pos;
  out->m_type = KindOfInt64;
}

bool PackedArray::ExistsInt(const ArrayData* ad, int64_t k) {
  auto a = asPacked(ad);
  return size_t(k) < a->m_size;
}

ArrayData* PackedArray::LvalInt(ArrayData* ad,
                                int64_t k,
                                Variant*& ret,
                                bool copy) {
  auto a = asPacked(ad);

  if (size_t(k) < a->m_size) {
    if (copy) a = a->copyPacked();
    ret = &tvAsVariant(&a->data()[k].data);
    return a;
  }

  if (copy) {
    a = a->copyPackedAndResizeIfNeeded();
  } else {
    a = a->resizePackedIfNeeded();
  }

  if (size_t(k) == a->m_size) {
    auto& tv = a->allocNextElm(k);
    tvWriteNull(&tv);
    ret = &tvAsVariant(&tv);
    return a;
  }

  // todo t2606310: we know key is new.  use add/findForNewInsert
  a = a->packedToMixed(); // in place
  return a->addLvalImpl(k, ret);
}

ArrayData* PackedArray::LvalStr(ArrayData* ad,
                                StringData* key,
                                Variant*& ret,
                                bool copy) {
  auto a = asPacked(ad);
  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();
  a = a->packedToMixed();
  return a->addLvalImpl(key, ret);
}

ArrayData* PackedArray::LvalNew(ArrayData* ad, Variant*& ret, bool copy) {
  auto a = asPacked(ad);
  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();
  auto& tv = a->allocNextElm(a->m_size);
  tvWriteNull(&tv);
  ret = &tvAsVariant(&tv);
  return a;
}

ArrayData*
PackedArray::SetInt(ArrayData* ad, int64_t k, const Variant& v, bool copy) {
  auto a = asPacked(ad);

  if (size_t(k) < a->m_size) {
    if (copy) a = a->copyPacked();
    cellSet(*v.asCell(), *tvToCell(&a->data()[k].data));
    // TODO(#3888164): we should restructure things so we don't have to
    // check KindOfUninit here.
    if (UNLIKELY(v.asTypedValue()->m_type == KindOfUninit)) {
      a->data()[k].data.m_type = KindOfNull;
    }
    return a;
  }

  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();

  if (size_t(k) == a->m_size) {
    auto& tv = a->allocNextElm(k);
    // TODO(#3888164): constructValHelper is making KindOfUninit checks.
    tvAsUninitializedVariant(&tv).constructValHelper(v);
    return a;
  }

  // Must escalate to mixed, but call addVal() since key doesn't
  // exist.
  a = a->packedToMixed();
  return a->addVal(k, v);
}

ArrayData* PackedArray::SetStr(ArrayData* ad,
                               StringData* k,
                               const Variant& v,
                               bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  // must convert to mixed, but call addVal() since key doesn't exist.
  a = a->resizePackedIfNeeded();
  a = a->packedToMixed();
  return a->addVal(k, v);
}

ArrayData*
PackedArray::SetRefInt(ArrayData* ad, int64_t k, const Variant& v, bool copy) {
  auto a = asPacked(ad);

  if (size_t(k) < a->m_size) {
    if (copy) a = a->copyPacked();
    tvBind(v.asRef(), &a->data()[k].data);
    return a;
  }

  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();

  if (size_t(k) == a->m_size) {
    auto& tv = a->allocNextElm(k);
    tv.m_data.pref = v.asRef()->m_data.pref;
    tv.m_type = KindOfRef;
    tv.m_data.pref->incRefCount();
    return a;
  }

  // todo t2606310: key can't exist.  use add/findForNewInsert
  a = a->packedToMixed();
  return a->updateRef(k, v);
}

ArrayData* PackedArray::SetRefStr(ArrayData* ad,
                                  StringData* k,
                                  const Variant& v,
                                  bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  // todo t2606310: key can't exist.  use add/findForNewInsert
  a = a->resizePackedIfNeeded();
  a = a->packedToMixed();
  return a->updateRef(k, v);
}

ArrayData* PackedArray::AddInt(ArrayData* ad,
                               int64_t k,
                               const Variant& v,
                               bool copy) {
  assert(!ad->exists(k));
  auto a = asPacked(ad);

  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();

  if (size_t(k) == a->m_size) {
    auto& tv = a->allocNextElm(k);
    // TODO(#3888164): constructValHelper is making KindOfUninit checks.
    tvAsUninitializedVariant(&tv).constructValHelper(v);
    return a;
  }

  a = a->packedToMixed();
  return a->addVal(k, v);
}

ArrayData* PackedArray::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  if (size_t(k) < a->m_size) {
    // escalate to mixed for correctness; unset preserves m_nextKI
    a = a->packedToMixed();
    auto pos = a->findForRemove(k, false);
    if (validPos(pos)) a->erase(pos);
  }
  return a; // key didn't exist, so we're still vector
}

ArrayData*
PackedArray::RemoveStr(ArrayData* ad, const StringData* key, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  return a;
}

ArrayData* PackedArray::Copy(const ArrayData* ad) {
  return asPacked(ad)->copyPacked();
}

ArrayData* PackedArray::Append(ArrayData* ad, const Variant& v, bool copy) {
  auto a = asPacked(ad);
  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();
  auto& tv = a->allocNextElm(a->m_size);
  // TODO(#3888164): constructValHelper is making KindOfUninit checks.
  tvAsUninitializedVariant(&tv).constructValHelper(v);
  return a;
}

ArrayData* PackedArray::AppendRef(ArrayData* ad, const Variant& v, bool copy) {
  auto a = asPacked(ad);
  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();
  auto& tv = a->allocNextElm(a->m_size);
  tv.m_data.pref = v.asRef()->m_data.pref;
  tv.m_type = KindOfRef;
  tv.m_data.pref->incRefCount();
  return a;
}

ArrayData* PackedArray::AppendWithRef(ArrayData* ad,
                                      const Variant& v,
                                      bool copy) {
  auto a = asPacked(ad);
  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();
  auto& tv = a->allocNextElm(a->m_size);
  tvWriteNull(&tv);
  tvAsVariant(&tv).setWithRef(v);
  return a;
}

ArrayData* PackedArray::Pop(ArrayData* ad, Variant& value) {
  auto a = asPacked(ad);
  if (a->hasMultipleRefs()) a = a->copyPacked();
  if (a->m_size > 0) {
    auto i = a->m_size - 1;
    auto& tv = a->data()[i].data;
    value = tvAsCVarRef(&tv);
    if (UNLIKELY(strong_iterators_exist())) {
      a->adjustMArrayIter(i);
    }
    auto oldType = tv.m_type;
    auto oldDatum = tv.m_data.num;
    a->m_size = a->m_used = i;
    a->m_pos = a->m_size > 0
      ? 0
      : ArrayData::invalid_index; // reset internal iterator
    tvRefcountedDecRefHelper(oldType, oldDatum);
    return a;
  }
  value = uninit_null();
  a->m_pos = ArrayData::invalid_index; // reset internal iterator
  return a;
}

ArrayData* PackedArray::Dequeue(ArrayData* adInput, Variant& value) {
  auto a = asPacked(adInput);
  if (a->hasMultipleRefs()) a = a->copyPacked();
  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is removed from the beginning of the array.
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(a);
  }
  auto elms = a->data();
  if (a->m_size > 0) {
    auto n = a->m_size - 1;
    auto& tv = elms[0].data;
    value = std::move(tvAsVariant(&tv)); // no incref+decref
    memmove(&elms[0], &elms[1], n * sizeof(elms[0]));
    a->m_size = a->m_used = n;
    a->m_pos = n > 0 ? 0 : ArrayData::invalid_index;
  } else {
    value = uninit_null();
    a->m_pos = ArrayData::invalid_index;
  }
  return a;
}

ArrayData* PackedArray::Prepend(ArrayData* adInput,
                                const Variant& v,
                                bool copy) {
  auto a = asPacked(adInput);
  if (a->hasMultipleRefs()) a = a->copyPackedAndResizeIfNeeded();
  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is added to the beginning of the array.
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(a);
  }
  size_t n = a->m_size;
  if (n > 0) {
    if (n == a->m_cap) a = HphpArray::GrowPacked(a);
    auto elms = a->data();
    memmove(&elms[1], &elms[0], n * sizeof(elms[0]));
  }
  a->m_size = a->m_used = n + 1;
  a->m_pos = 0;
  // TODO(#3888164): constructValHelper is making KindOfUninit checks.
  tvAsUninitializedVariant(&a->data()[0].data).constructValHelper(v);
  return a;
}

void PackedArray::OnSetEvalScalar(ArrayData* ad) {
  auto a = asPacked(ad);
  auto const elms = a->data();
  for (uint32_t i = 0, limit = a->m_size; i < limit; ++i) {
    tvAsVariant(&elms[i].data).setEvalScalar();
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * Cold path helper for AddNewElemC delegates to the ArrayData::append
 * virtual method.
 */
static NEVER_INLINE
ArrayData* genericAddNewElemC(ArrayData* a, TypedValue value) {
  ArrayData* r = a->append(tvAsCVarRef(&value), a->getCount() != 1);
  if (UNLIKELY(r != a)) {
    r->incRefCount();
    decRefArr(a);
  }
  tvRefcountedDecRef(value);
  return r;
}

/*
 * The pass-by-value and move semantics of this helper are slightly different
 * than other array helpers, but tuned for the opcode.  See doc comment in
 * hphp_array.h.
 */
ArrayData* HphpArray::AddNewElemC(ArrayData* ad, TypedValue value) {
  assert(value.m_type != KindOfRef);
  HphpArray* a;
  int64_t k;
  if (LIKELY(ad->isPacked()) &&
      ((a = asPacked(ad)), LIKELY(a->m_pos >= 0)) &&
      LIKELY(!a->hasMultipleRefs()) &&
      ((k = a->m_size), LIKELY(size_t(k) < a->m_cap))) {
    assert(a->checkInvariants());
    auto& tv = a->allocNextElm(k);
    // TODO(#3888164): this KindOfUninit check is almost certainly
    // unnecessary, but it was here so it hasn't been removed yet.
    tv.m_type = value.m_type == KindOfUninit ? KindOfNull : value.m_type;
    tv.m_data = value.m_data;
    return a;
  }
  return genericAddNewElemC(ad, value);
}

//////////////////////////////////////////////////////////////////////

}
