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

#include "hphp/runtime/base/hphp-array.h"

#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/empty-array.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/vm/member-operations.h"

#include "hphp/util/alloc.h"
#include "hphp/util/hash.h"
#include "hphp/util/lock.h"
#include "hphp/util/trace.h"

#include <algorithm>
#include <utility>

#include "hphp/runtime/base/hphp-array-defs.h"
#include "hphp/runtime/base/array-iterator-defs.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

namespace {

ALWAYS_INLINE
HphpArray* mallocArray(uint32_t cap, uint32_t mask) {
  auto const allocBytes = computeAllocBytes(cap, mask);
  return static_cast<HphpArray*>(std::malloc(allocBytes));
}

}

//////////////////////////////////////////////////////////////////////

NEVER_INLINE
HphpArray* HphpArray::MakeReserve(uint32_t capacity) {
  auto const cmret = computeCapAndMask(capacity);
  auto const cap   = cmret.first;
  auto const mask  = cmret.second;
  auto const ad    = smartAllocArray(cap, mask);

  ad->m_kindAndSize     = kPackedKind; // zero's size
  ad->m_posAndCount     = uint64_t{1} << 32 |
                           static_cast<uint32_t>(ArrayData::invalid_index);
  ad->m_capAndUsed      = cap;
  ad->m_tableMask       = mask;

  assert(ad->m_kind == kPackedKind);
  assert(ad->m_size == 0);
  assert(ad->m_pos == ArrayData::invalid_index);
  assert(ad->m_count == 1);
  assert(ad->m_cap == cap);
  assert(ad->m_used == 0);
  assert(ad->checkInvariants());
  return ad;
}

HphpArray* HphpArray::MakePacked(uint32_t size, const TypedValue* values) {
  assert(size > 0);

  auto const cmret = computeCapAndMask(size);
  auto const cap   = cmret.first;
  auto const mask  = cmret.second;
  auto const ad    = smartAllocArray(cap, mask);

  auto const shiftedSize = uint64_t{size} << 32;
  ad->m_kindAndSize      = shiftedSize | kPackedKind;
  ad->m_posAndCount      = uint64_t{1} << 32;
  ad->m_capAndUsed       = shiftedSize | cap;
  ad->m_tableMask        = mask;

  auto const data = reinterpret_cast<Elm*>(ad + 1);

  // Append values by moving -- Caller assumes we update refcount.
  // Values are in reverse order since they come from the stack, which
  // grows down.
  for (uint32_t i = 0; i < size; i++) {
    const auto& tv = values[size - i - 1];
    data[i].data.m_data = tv.m_data;
    data[i].data.m_type = tv.m_type;
  }

  assert(ad->m_kind == kPackedKind);
  assert(ad->m_size == size);
  assert(ad->m_pos == 0);
  assert(ad->m_count == 1);
  assert(ad->m_cap == cap);
  assert(ad->m_used == size);
  assert(ad->checkInvariants());
  return ad;
}

HphpArray* HphpArray::MakeStruct(uint32_t size, StringData** keys,
                                 const TypedValue* values) {
  assert(size > 0);

  auto const cmret = computeCapAndMask(size);
  auto const cap   = cmret.first;
  auto const mask  = cmret.second;
  auto const ad    = smartAllocArray(cap, mask);

  auto const shiftedSize = uint64_t{size} << 32;
  ad->m_kindAndSize      = shiftedSize | kMixedKind;
  ad->m_posAndCount      = uint64_t{1} << 32;
  ad->m_capAndUsed       = shiftedSize | cap;
  ad->m_tableMask        = mask;
  ad->m_nextKI           = 0;

  auto const data = reinterpret_cast<Elm*>(ad + 1);
  auto const hash = reinterpret_cast<int32_t*>(data + cap);
  ad->initHash(hash, mask + 1);

  // Append values by moving -- Caller assumes we update refcount.
  // Values are in reverse order since they come from the stack, which
  // grows down.
  for (uint32_t i = 0; i < size; i++) {
    assert(keys[i]->isStatic());
    auto k = keys[i];
    auto h = k->hash();
    data[i].setStaticKey(k, h);
    const auto& tv = values[size - i - 1];
    data[i].data.m_data = tv.m_data;
    data[i].data.m_type = tv.m_type;
    auto ei = ad->findForNewInsert(h);
    *ei = i;
  }
  ad->m_hLoad = size;

  assert(ad->m_kind == kMixedKind);
  assert(ad->m_size == size);
  assert(ad->m_pos == 0);
  assert(ad->m_count == 1);
  assert(ad->m_cap == cap);
  assert(ad->m_used == size);
  assert(ad->m_nextKI == 0);
  assert(ad->checkInvariants());
  return ad;
}

// for internal use by nonSmartCopy() and copyPacked()
template<class CopyElem>
ALWAYS_INLINE
HphpArray* HphpArray::CopyPacked(const HphpArray& other,
                                 AllocMode mode,
                                 CopyElem copyElem) {
  assert(other.isPacked());

  auto const cap  = other.m_cap;
  auto const mask = other.m_tableMask;
  auto const ad = mode == AllocMode::Smart
    ? smartAllocArray(cap, mask)
    : mallocArray(cap, mask);

  ad->m_kindAndSize     = uint64_t{other.m_size} << 32 | kPackedKind;
  ad->m_posAndCount     = static_cast<uint32_t>(other.m_pos);
  ad->m_capAndUsed      = uint64_t{other.m_used} << 32 | cap;
  ad->m_tableMask       = mask;

  auto const targetElms = reinterpret_cast<Elm*>(ad + 1);

  // Copy the elements and bump up refcounts as needed.
  auto const elms = other.data();
  for (uint32_t i = 0, limit = ad->m_used; i < limit; ++i) {
    copyElem(&elms[i].data, &targetElms[i].data, &other);
  }

  assert(ad->m_kind == kPackedKind);
  assert(ad->m_size == other.m_size);
  assert(ad->m_pos == other.m_pos);
  assert(ad->m_count == 0);
  assert(ad->m_used == other.m_used);
  assert(ad->m_cap == cap);
  assert(ad->m_tableMask == mask);
  assert(ad->checkInvariants());
  return ad;
}

// for internal use by nonSmartCopy() and copyPacked()
template<class CopyKeyValue>
ALWAYS_INLINE
HphpArray* HphpArray::CopyMixed(const HphpArray& other,
                                AllocMode mode,
                                CopyKeyValue copyKeyValue) {
  assert(other.m_kind == kMixedKind);

  auto const cap  = other.m_cap;
  auto const mask = other.m_tableMask;
  auto const ad = mode == AllocMode::Smart
    ? smartAllocArray(cap, mask)
    : mallocArray(cap, mask);

  ad->m_kindAndSize     = uint64_t{other.m_size} << 32 | kMixedKind;
  ad->m_posAndCount     = static_cast<uint32_t>(other.m_pos);
  ad->m_capAndUsed      = uint64_t{other.m_used} << 32 | cap;
  ad->m_maskAndLoad     = uint64_t{other.m_hLoad} << 32 | mask;
  ad->m_nextKI          = other.m_nextKI;

  auto const data      = reinterpret_cast<Elm*>(ad + 1);
  auto const hash      = reinterpret_cast<int32_t*>(data + cap);

  copyHash(hash, other.hashTab(), mask + 1);

  // Copy the elements and bump up refcounts as needed.
  auto const elms = other.data();
  for (uint32_t i = 0, limit = ad->m_used; i < limit; ++i) {
    auto const& e = elms[i];
    auto& te = data[i];
    if (!isTombstone(e.data.m_type)) {
      copyKeyValue(e, te, &other);
    } else {
      // Tombstone.
      te.data.m_type = KindOfInvalid;
    }
  }

  // If the element density dropped below 50% due to indirect elements
  // being converted into tombstones, we should do a compaction
  if (ad->m_size < ad->m_used / 2) {
    ad->compact(false);
  }

  assert(ad->m_kind == other.m_kind);
  assert(ad->m_size == other.m_size);
  assert(ad->m_pos == other.m_pos);
  assert(ad->m_count == 0);
  assert(ad->m_used == other.m_used);
  assert(ad->m_cap == cap);
  assert(ad->m_tableMask == mask);
  assert(ad->m_hLoad == other.m_hLoad);
  assert(ad->checkInvariants());
  return ad;
}

NEVER_INLINE ArrayData* HphpArray::NonSmartCopy(const ArrayData* in) {
  auto a = asHphpArray(in);
  assert(a->checkInvariants());
  return a->isPacked()
    ? CopyPacked(*a, AllocMode::NonSmart,
        [&](const TypedValue* fr, TypedValue* to, const ArrayData* container) {
          tvDupFlattenVars(fr, to, container);
        })
    : CopyMixed(*a, AllocMode::NonSmart,
        [&](const Elm& from, Elm& to, const ArrayData* container) {
          to.key = from.key;
          to.data.hash() = from.data.hash();
          if (to.hasStrKey()) to.key->incRefCount();
          tvDupFlattenVars(&from.data, &to.data, container);
          assert(to.hash() == from.hash()); // ensure not clobbered.
        });
}

NEVER_INLINE HphpArray* HphpArray::copyPacked() const {
  assert(checkInvariants());
  return CopyPacked(*this, AllocMode::Smart,
      [&](const TypedValue* fr, TypedValue* to, const ArrayData* container) {
        tvDupFlattenVars(fr, to, container);
      });
}

NEVER_INLINE HphpArray* HphpArray::copyMixed() const {
  assert(checkInvariants());
  return CopyMixed(*this, AllocMode::Smart,
      [&](const Elm& from, Elm& to, const ArrayData* container) {
        to.key = from.key;
        to.data.hash() = from.data.hash();
        if (to.hasStrKey()) to.key->incRefCount();
        tvDupFlattenVars(&from.data, &to.data, container);
        assert(to.hash() == from.hash()); // ensure not clobbered.
      });
}

ALWAYS_INLINE
HphpArray* HphpArray::copyPackedAndResizeIfNeeded() const {
  if (LIKELY(!isFullPacked())) return copyPacked();
  return copyPackedAndResizeIfNeededSlow();
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
  ReleasePacked(copy);
  return ret;
}

ALWAYS_INLINE
HphpArray* HphpArray::copyMixedAndResizeIfNeeded() const {
  if (LIKELY(!isFull())) return copyMixed();
  return copyMixedAndResizeIfNeededSlow();
}

NEVER_INLINE
HphpArray* HphpArray::copyMixedAndResizeIfNeededSlow() const {
  assert(isFull());

  // Note: this path will have to handle splitting strong iterators
  // later when we combine copyPacked & GrowPacked into one operation.
  // For now I'm just making use of copyPacked to do it for me before
  // GrowPacked happens.
  auto const copy = copyMixed();
  auto const ret = copy->resize();
  if (copy != ret) Release(copy);
  return ret;
}

namespace {

Variant CreateVarForUncountedArray(const Variant& source) {
  auto type = source.getType(); // this gets rid of the ref, if it was one
  switch (type) {
    case KindOfBoolean:
      return source.getBoolean();
    case KindOfInt64:
      return source.getInt64();
    case KindOfDouble:
      return source.getDouble();
    case KindOfUninit:
    case KindOfNull:
      return null_variant;
    case KindOfStaticString:
      return source.getStringData();

    case KindOfString: {
      auto const st = lookupStaticString(source.getStringData());
      if (st != nullptr) return st;
      return StringData::MakeUncounted(source.getStringData()->slice());
    }

    case KindOfArray: {
      auto const ad = source.getArrayData();
      return ad == HphpArray::GetStaticEmptyArray()
        ? ad
        : HphpArray::MakeUncounted(ad);
    }

    default:
      assert(false); // type not allowed
  }
  return null_variant;
}

}

HphpArray* HphpArray::MakeUncounted(ArrayData* array) {
  auto a = asHphpArray(array);
  assert(a->checkInvariants());
  if (array->isVectorData()) {
    auto packed = CopyPacked(*a, AllocMode::NonSmart,
        [&](const TypedValue* fr, TypedValue* to, const ArrayData* container) {
          tvCopy(*CreateVarForUncountedArray(tvAsCVarRef(fr)).asTypedValue(),
                 *to);
        });
    packed->setUncounted();
    return packed;
  }
  auto mixed = CopyMixed(*a, AllocMode::NonSmart,
      [&](const Elm& fr, Elm& to, const ArrayData* container) {
        to.data.hash() = fr.data.hash();
        if (to.hasStrKey()) {
          auto const st = lookupStaticString(fr.key);
          to.key = (st != nullptr) ? st :
                          StringData::MakeUncounted(fr.key->slice());
        } else {
          to.key = fr.key;
        }
        tvCopy(
            *CreateVarForUncountedArray(tvAsCVarRef(&fr.data)).asTypedValue(),
            to.data);
        assert(to.hash() == fr.hash()); // ensure not clobbered.
      });
  mixed->setUncounted();
  return mixed;
}

//=============================================================================
// Destruction

NEVER_INLINE
void HphpArray::ReleasePacked(ArrayData* in) {
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

NEVER_INLINE
void HphpArray::Release(ArrayData* in) {
  assert(in->isRefCounted());
  auto const ad = asMixed(in);

  if (!ad->isZombie()) {
    auto const data = ad->data();
    auto const stop = data + ad->m_used;

    for (auto ptr = data; ptr != stop; ++ptr) {
      if (isTombstone(ptr->data.m_type)) continue;
      if (ptr->hasStrKey()) decRefStr(ptr->key);
      tvRefcountedDecRef(&ptr->data);
    }

    if (UNLIKELY(strong_iterators_exist())) {
      free_strong_iterators(ad);
    }
  }

  auto const cap  = ad->m_cap;
  auto const mask = ad->m_tableMask;
  MM().objFreeLogged(ad, computeAllocBytes(cap, mask));
}

NEVER_INLINE
void HphpArray::ReleaseUncounted(ArrayData* in) {
  auto const ad = asHphpArray(in);
  assert(ad->m_count == UncountedValue);

  if (!ad->isZombie()) {
    auto const data = ad->data();
    auto const stop = data + ad->m_used;

    for (auto ptr = data; ptr != stop; ++ptr) {
      if (!ad->isPacked()) {
        if (isTombstone(ptr->data.m_type)) continue;
        if (ptr->hasStrKey()) {
          assert(!ptr->key->isRefCounted());
          if (!ptr->key->isStatic()) {
            ptr->key->destructStatic();
          }
        }
      }
      if (ptr->data.m_type == KindOfString) {
        assert(!ptr->data.m_data.pstr->isRefCounted());
        if (!ptr->data.m_data.pstr->isStatic()) {
          ptr->data.m_data.pstr->destructStatic();
        }
      } else if (ptr->data.m_type == KindOfArray) {
        assert(!ptr->data.m_data.parr->isRefCounted());
        if (!ptr->data.m_data.parr->isStatic()) {
          ReleaseUncounted(ptr->data.m_data.parr);
        }
      } else {
        assert(!IS_REFCOUNTED_TYPE(ptr->data.m_type));
      }
    }

    if (UNLIKELY(strong_iterators_exist())) {
      free_strong_iterators(ad);
    }
  }

  std::free(ad);
}

//=============================================================================
// State transitions

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
 *   m_tableMask is 2^k - 1 (required for quadratic probe)
 *   m_tableMask == folly::nextPowTwo(m_cap) - 1;
 *   m_cap == computeMaxElms(m_tableMask);
 *
 * Zombie state:
 *
 *   m_used == UINT32_MAX
 *   no MArrayIter's are pointing to this array
 *
 * Non-zombie:
 *
 *   m_size <= m_used; m_used <= m_cap
 *   last element cannot be a tombstone
 *   m_pos and all external iterators can't be on a tombstone
 *
 * kMixedKind:
 *   m_nextKI >= highest actual int key
 *   Elm.data.m_type maybe KindOfInvalid (tombstone)
 *   hash[] maybe Tombstone
 *   m_hLoad >= m_size, == number of non-Empty hash entries
 *
 * kPackedKind:
 *   m_size == m_used
 *   m_nextKI = uninitialized
 *   m_hLoad = uninitialized
 *   Elm.key uninitialized
 *   Elm.hash uninitialized
 *   no KindOfInvalid tombstones
 */
bool HphpArray::checkInvariants() const {
  static_assert(sizeof(Elm) == 24, "");
  static_assert(sizeof(ArrayData) == 2 * sizeof(uint64_t), "");
  static_assert(
    sizeof(HphpArray) == sizeof(ArrayData) + 3 * sizeof(uint64_t),
    "Performance is sensitive to sizeof(HphpArray)."
    " Make sure you changed it with good reason and then update this assert."
  );

  // All arrays:
  assert(m_tableMask > 0 && ((m_tableMask+1) & m_tableMask) == 0);
  assert(m_tableMask == folly::nextPowTwo(m_cap) - 1);
  assert(m_cap == computeMaxElms(m_tableMask));

  if (isZombie()) return true;

  // Non-zombie:
  assert(m_size <= m_used);
  assert(m_used <= m_cap);
  if (m_pos != invalid_index) {
    assert(size_t(m_pos) < m_used);
    assert(!isTombstone(data()[m_pos].data.m_type));
  }
  if (m_used > 0) {
    // can't have a tombstone at the end; m_used should have been trimmed.
    assert(!isTombstone(data()[m_used - 1].data.m_type));
  }

  // Type specific:
  switch (m_kind) {
  case kPackedKind:
    assert(m_size == m_used);
    break;
  case kMixedKind: {
    assert(m_hLoad >= m_size);
    size_t load = 0;
    return true;
    // The following loop is for debugging arrays only; it slows
    // things down too much for general use
    for (size_t i = 0; i <= m_tableMask; i++) {
      load += hashTab()[i] != Empty;
    }
    assert(m_hLoad == load);
    break;
  }
  default:
    assert(false);
    break;
  }

  return true;
}

//=============================================================================
// Iteration.

inline ssize_t HphpArray::prevElm(Elm* elms, ssize_t ei) const {
  assert(ei <= ssize_t(m_used));
  while (ei > 0) {
    --ei;
    if (!isTombstone(elms[ei].data.m_type)) {
      return ei;
    }
  }
  return invalid_index;
}

ssize_t HphpArray::IterBegin(const ArrayData* ad) {
  auto a = asHphpArray(ad);
  return a->nextElm(a->data(), invalid_index);
}

ssize_t HphpArray::IterEnd(const ArrayData* ad) {
  auto a = asHphpArray(ad);
  return a->prevElm(a->data(), a->m_used);
}

ssize_t HphpArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  auto a = asHphpArray(ad);
  // Since m_used is always less than 2^32 and invalid_index == -1,
  // we can save a check by doing an unsigned comparison instead
  // of a signed comparison.
  if (size_t(++pos) < a->m_used && !isTombstone(a->data()[pos].data.m_type)) {
    return pos;
  }
  return a->iter_advance_helper(pos);
  static_assert(invalid_index == -1, "");
}

// caller has already incremented pos but encountered a tombstone
ssize_t HphpArray::iter_advance_helper(ssize_t next_pos) const {
  Elm* elms = data();
  // Since m_used is always less than 2^32 and invalid_index == -1,
  // we can save a check by doing an unsigned comparison instead of
  // a signed comparison.
  for (auto limit = m_used; size_t(next_pos) < limit; ++next_pos) {
    if (!isTombstone(elms[next_pos].data.m_type)) {
      return next_pos;
    }
  }
  return invalid_index;
}

ssize_t HphpArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  if (pos == invalid_index) return invalid_index;
  auto a = asHphpArray(ad);
  return a->prevElm(a->data(), pos);
}

const Variant& HphpArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  auto a = asHphpArray(ad);
  assert(a->checkInvariants());
  assert(pos != invalid_index);
  auto& e = a->data()[pos];
  assert(!isTombstone(e.data.m_type));
  return tvAsCVarRef(&e.data);
}

bool HphpArray::IsVectorData(const ArrayData* ad) {
  auto a = asMixed(ad);
  if (a->m_size == 0) {
    // any 0-length array is "vector-like" for the sake of this
    // function, even if m_kind != kVector.
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

static bool hitStringKey(const HphpArray::Elm& e, const StringData* s,
                         int32_t hash) {
  // hitStringKey() should only be called on an Elm that is referenced by a
  // hash table entry. HphpArray guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!HphpArray::isTombstone(e.data.m_type));
  return hash == e.hash() && (s == e.key || s->same(e.key));
}

static bool hitIntKey(const HphpArray::Elm& e, int64_t ki) {
  // hitIntKey() should only be called on an Elm that is referenced by a
  // hash table entry. HphpArray guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!HphpArray::isTombstone(e.data.m_type));
  return e.ikey == ki && e.hasIntKey();
}

// Quadratic probe is:
//
//   h(k, i) = (k + c1*i + c2*(i^2)) % tableSize
//
// Use 1/2 for c1 and c2.  In combination with a table size that is a power of
// 2, this guarantees a probe sequence of length tableSize that probes all
// table elements exactly once.

template <class Hit> ALWAYS_INLINE
ssize_t HphpArray::findImpl(size_t h0, Hit hit) const {
  // tableMask, probeIndex, and pos are explicitly 64-bit, because performance
  // regressed when they were 32-bit types via auto.  Test carefully.
  size_t tableMask = m_tableMask;
  auto* elms = data();
  auto* hashtable = hashTab();
  for (size_t probeIndex = h0, i = 1;; ++i) {
    ssize_t pos = hashtable[probeIndex & tableMask];
    if ((validPos(pos) && hit(elms[pos])) || pos == Empty) {
      return pos;
    }
    probeIndex += i;
    assert(i <= tableMask && probeIndex == h0 + (i + i*i) / 2);
  }
}

ssize_t HphpArray::find(int64_t ki) const {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  return findImpl(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

ssize_t HphpArray::find(const StringData* s, strhash_t prehash) const {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  auto h = prehash | STRHASH_MSB;
  return findImpl(prehash, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

NEVER_INLINE
int32_t* warnUnbalanced(size_t n, int32_t* ei) {
  if (n > size_t(RuntimeOption::MaxArrayChain)) {
    raise_error("Array is too unbalanced (%lu)", n);
  }
  return ei;
}

template <class Hit> ALWAYS_INLINE
int32_t* HphpArray::findForInsertImpl(size_t h0, Hit hit) const {
  // tableMask, probeIndex, and pos are explicitly 64-bit, because performance
  // regressed when they were 32-bit types via auto.  Test carefully.
  assert(m_hLoad <= computeMaxElms(m_tableMask));
  size_t tableMask = m_tableMask;
  auto* elms = data();
  auto* hashtable = hashTab();
  int32_t* ret = nullptr;
  for (size_t probeIndex = h0, i = 1;; ++i) {
    auto ei = &hashtable[probeIndex & tableMask];
    ssize_t pos = *ei;
    if (validPos(pos)) {
      if (hit(elms[pos])) {
        return ei;
      }
    } else {
      if (!ret) ret = ei;
      if (pos == Empty) {
        return LIKELY(i <= 100) ? ret : warnUnbalanced(i, ret);
      }
    }
    probeIndex += i;
    assert(i <= tableMask && probeIndex == h0 + (i + i*i) / 2);
  }
}

int32_t* HphpArray::findForInsert(int64_t ki) const {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  return findForInsertImpl(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

int32_t*
HphpArray::findForInsert(const StringData* s, strhash_t prehash) const {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  auto h = prehash | STRHASH_MSB;
  return findForInsertImpl(prehash, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

HphpArray::InsertPos HphpArray::insert(int64_t k) {
  assert(!isFull());
  auto ei = findForInsert(k);
  if (validPos(*ei)) {
    return InsertPos(true, data()[*ei].data);
  }
  if (k >= m_nextKI && m_nextKI >= 0) m_nextKI = k + 1;
  auto& e = allocElm(ei);
  e.setIntKey(k);
  return InsertPos(false, e.data);
}

HphpArray::InsertPos HphpArray::insert(StringData* k) {
  assert(!isFull());
  auto const h = k->hash();
  auto ei = findForInsert(k, h);
  if (validPos(*ei)) {
    return InsertPos(true, data()[*ei].data);
  }
  auto& e = allocElm(ei);
  e.setStrKey(k, h);
  return InsertPos(false, e.data);
}

template <class Hit, class Remove> ALWAYS_INLINE
ssize_t HphpArray::findForRemoveImpl(size_t h0, Hit hit, Remove remove) const {
  assert(m_hLoad <= computeMaxElms(m_tableMask));
  size_t mask = m_tableMask;
  auto* elms = data();
  auto* hashtable = hashTab();
  for (size_t i = 1, probe = h0;; ++i) {
    auto* ei = &hashtable[probe & mask];
    ssize_t pos = *ei;
    if (validPos(pos)) {
      if (hit(elms[pos])) {
        remove(elms[pos]);
        *ei = Tombstone;
        return pos;
      }
    } else {
      if (pos == Empty) {
        // not found, terminate search
        return pos;
      }
    }
    probe += i;
    assert(probe == (h0 + (i + i*i) / 2));
    assert(i <= mask);
  }
}

NEVER_INLINE
ssize_t HphpArray::findForRemove(int64_t ki, bool updateNext) {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  return findForRemoveImpl(ki,
      [&] (const Elm& e) {
        return hitIntKey(e, ki);
      },
      [this, ki, updateNext] (Elm& e) {
        assert(ki == e.ikey);
        // Match PHP 5.3.1 semantics
        // Hacky: don't removed the unsigned cast, else g++ can optimize away
        // the check for == 0x7fff..., since there is no signed int k
        // for which k-1 == 0x7fff...
        if ((uint64_t)ki == (uint64_t)m_nextKI-1
              && (ki == 0x7fffffffffffffffLL || updateNext)) {
          --m_nextKI;
        }
      }
  );
}

ssize_t
HphpArray::findForRemove(const StringData* s, strhash_t prehash) {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  auto h = prehash | STRHASH_MSB;
  return findForRemoveImpl(prehash,
      [&] (const Elm& e) {
        return hitStringKey(e, s, h);
      },
      [] (Elm& e) {
        decRefStr(e.key);
        e.setIntKey(0);
      }
    );
}

bool HphpArray::ExistsIntPacked(const ArrayData* ad, int64_t k) {
  auto a = asPacked(ad);
  return size_t(k) < a->m_size;
}

bool HphpArray::ExistsInt(const ArrayData* ad, int64_t k) {
  auto a = asMixed(ad);
  return validPos(a->find(k));
}

bool HphpArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  auto a = asMixed(ad);
  return validPos(a->find(k, k->hash()));
}

//=============================================================================
// Append/insert/update.

ALWAYS_INLINE bool HphpArray::isFull() const {
  assert(!isPacked());
  assert(m_used <= m_cap);
  assert(m_hLoad <= m_cap);
  return m_used == m_cap || m_hLoad == m_cap;
}

ALWAYS_INLINE bool HphpArray::isFullPacked() const {
  assert(isPacked());
  assert(m_size <= m_cap);
  return m_size == m_cap;
}

ALWAYS_INLINE HphpArray::Elm& HphpArray::allocElm(int32_t* ei) {
  assert(!validPos(*ei) && !isFull());
  assert(m_size != 0 || m_used == 0);
  ++m_size;
  m_hLoad += (*ei == Empty);
  size_t i = m_used;
  (*ei) = i;
  m_used = i + 1;
  if (m_pos == invalid_index) m_pos = i;
  return data()[i];
}

ALWAYS_INLINE TypedValue& HphpArray::allocNextElm(uint32_t i) {
  assert(isPacked() && i == m_size);
  assert(!isFullPacked());
  auto next = i + 1;
  if (m_pos == invalid_index) m_pos = i;
  m_used = m_size = next;
  return data()[i].data;
}

ALWAYS_INLINE
HphpArray* HphpArray::initVal(TypedValue& tv, const Variant& v) {
  tvAsUninitializedVariant(&tv).constructValHelper(v);
  return this;
}

ALWAYS_INLINE
ArrayData* HphpArray::zInitVal(TypedValue& tv, RefData* v) {
  tv.m_type = KindOfRef;
  tv.m_data.pref = v;
  return this;
}

ALWAYS_INLINE
HphpArray* HphpArray::initRef(TypedValue& tv, const Variant& v) {
  tvAsUninitializedVariant(&tv).constructRefHelper(v);
  return this;
}

ALWAYS_INLINE
HphpArray* HphpArray::getLval(TypedValue& tv, Variant*& ret) {
  ret = &tvAsVariant(&tv);
  return this;
}

ALWAYS_INLINE
HphpArray* HphpArray::initLval(TypedValue& tv, Variant*& ret) {
  tvWriteNull(&tv);
  ret = &tvAsVariant(&tv);
  return this;
}

ALWAYS_INLINE
HphpArray* HphpArray::initWithRef(TypedValue& tv, const Variant& v) {
  tvWriteNull(&tv);
  tvAsVariant(&tv).setWithRef(v);
  return this;
}

ALWAYS_INLINE
HphpArray* HphpArray::setVal(TypedValue& tv, const Variant& v) {
  auto const src = v.asCell();
  auto const dst = tvToCell(&tv);
  cellSet(*src, *dst);
  // TODO(#3888164): we should restructure things so we don't have to
  // check KindOfUninit here.
  if (UNLIKELY(src->m_type == KindOfUninit)) {
    dst->m_type = KindOfNull;
  }
  return this;
}

ALWAYS_INLINE
ArrayData* HphpArray::zSetVal(TypedValue& tv, RefData* v) {
  // Dec ref the old value
  tvRefcountedDecRef(tv);
  // Store the RefData but do not increment the refcount
  tv.m_type = KindOfRef;
  tv.m_data.pref = v;
  return this;
}

ALWAYS_INLINE
HphpArray* HphpArray::setRef(TypedValue& tv, const Variant& v) {
  auto const ref = v.asRef();
  tvBind(ref, &tv);
  return this;
}

/*
 * This is a streamlined copy of Variant.constructValHelper()
 * with no incref+decref because we're moving v to this array.
 */
ALWAYS_INLINE
HphpArray* HphpArray::moveVal(TypedValue& tv, TypedValue v) {
  tv.m_type = typeInitNull(v.m_type);
  tv.m_data.num = v.m_data.num;
  return this;
}

ALWAYS_INLINE HphpArray* HphpArray::resizeIfNeeded() {
  if (UNLIKELY(isFull())) return resize();
  return this;
}

ALWAYS_INLINE HphpArray* HphpArray::resizePackedIfNeeded() {
  if (UNLIKELY(isFullPacked())) return GrowPacked(this);
  return this;
}

NEVER_INLINE HphpArray* HphpArray::resize() {
  uint32_t maxElms = computeMaxElms(m_tableMask);
  assert(m_used <= maxElms);
  assert(m_hLoad <= maxElms);
  // At a minimum, compaction is required.  If the load factor would be >0.5
  // even after compaction, grow instead, in order to avoid the possibility
  // of repeated compaction if the load factor were to hover at nearly 0.75.
  if (m_size > maxElms / 2) {
    return Grow(this);
  }
  compact(false);
  return this;
}

HphpArray* HphpArray::Grow(HphpArray* old) {
  assert(!old->isPacked());
  assert(old->m_tableMask <= 0x7fffffffU);

  DEBUG_ONLY auto oldPos = old->m_pos;

  auto const oldMask    = old->m_tableMask;
  auto const mask       = oldMask * 2 + 1;
  auto const cap        = computeMaxElms(mask);
  auto const ad         = smartAllocArray(cap, mask);

  auto const oldSize        = old->m_size;
  auto const oldPosUnsigned = static_cast<uint32_t>(old->m_pos);
  auto const oldUsed        = old->m_used;

  ad->m_kindAndSize     = uint64_t{oldSize} << 32 | kMixedKind;
  ad->m_posAndCount     = oldPosUnsigned;
  ad->m_capAndUsed      = uint64_t{oldUsed} << 32 | cap;
  ad->m_maskAndLoad     = uint64_t{oldSize} << 32 | mask;
  ad->m_nextKI          = old->m_nextKI;
  auto table            = reinterpret_cast<int32_t*>(ad->data() + cap);

  if (UNLIKELY(strong_iterators_exist())) {
    move_strong_iterators(ad, old);
  }

  // Copy the old element array, and initialize the hashtable to all empty.
  copyElms(ad->data(), old->data(), oldUsed);
  ad->initHash(table, mask + 1);

  auto iter = ad->data();
  auto const stop = iter + oldUsed;
  assert(mask == ad->m_tableMask);
  for (uint32_t i = 0; iter != stop; ++iter, ++i) {
    auto& e = *iter;
    if (isTombstone(e.data.m_type)) continue;
    *ad->findForNewInsert(table, mask, e.hasIntKey() ? e.ikey : e.hash()) = i;
  }

  old->m_used = -uint32_t{1};

  assert(old->isZombie());
  assert(ad->m_kind == kMixedKind);
  assert(ad->m_size == oldSize);
  assert(ad->m_count == 0);
  assert(ad->m_pos == oldPos);
  assert(ad->m_used == oldUsed);
  assert(ad->m_tableMask == mask);
  assert(ad->m_hLoad == oldSize);
  assert(ad->checkInvariants());
  return ad;
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

namespace {
struct ElmKey {
  ElmKey() {}
  ElmKey(int32_t hash, StringData* key)
    : hash(hash)
    , key(key)
  {}
  int32_t hash;
  union {
    StringData* key;
    int64_t ikey;
  };
};
}

void HphpArray::compact(bool renumber /* = false */) {
  assert(!isPacked());
  ElmKey mPos;
  if (m_pos != invalid_index) {
    // Cache key for element associated with m_pos in order to update m_pos
    // below.
    assert(size_t(m_pos) < m_used);
    auto& e = data()[m_pos];
    mPos.hash = e.hasIntKey() ? 0 : e.hash();
    mPos.key = e.key;
  } else {
    // Silence compiler warnings.
    mPos.hash = 0;
    mPos.key = nullptr;
  }

  TinyVector<ElmKey,3> siKeys;
  auto const checkingStrongIterators = strong_iterators_exist();
  if (UNLIKELY(checkingStrongIterators)) {
    for_each_strong_iterator([&] (const MIterTable::Ent& miEnt) {
      if (miEnt.array != this) return;
      auto const ei = miEnt.iter->m_pos;
      if (ei != invalid_index) {
        auto& e = data()[ei];
        siKeys.push_back(ElmKey(e.hash(), e.key));
      }
    });
  }

  if (renumber) {
    m_nextKI = 0;
  }
  auto elms = data();
  auto mask = m_tableMask;
  size_t tableSize = mask + 1;
  auto table = hashTab();
  initHash(table, tableSize);
  m_hLoad = 0;
  for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
    while (isTombstone(elms[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = elms[toPos];
    if (toPos != frPos) {
      toE = elms[frPos];
    }
    if (renumber && !toE.hasStrKey()) {
      toE.ikey = m_nextKI++;
    }
    auto ie = findForNewInsert(table, mask,
                               toE.hasIntKey() ? toE.ikey : toE.hash());
    *ie = toPos;
  }
  m_used = m_size;
  m_hLoad = m_size;
  if (m_pos != invalid_index) {
    // Update m_pos, now that compaction is complete.
    if (mPos.hash) {
      m_pos = ssize_t(find(mPos.key, mPos.hash));
    } else {
      m_pos = ssize_t(find(mPos.ikey));
    }
  }

  // Update strong iterators, now that compaction is complete.
  if (LIKELY(!checkingStrongIterators)) return;

  int key = 0;
  for_each_strong_iterator([&] (MIterTable::Ent& miEnt) {
    if (miEnt.array != this) return;
    auto const iter = miEnt.iter;
    if (iter->m_pos == invalid_index) return;
    auto& k = siKeys[key];
    key++;
    if (k.hash) { // string key
      iter->m_pos = ssize_t(find(k.key, k.hash));
    } else { // int key
      iter->m_pos = ssize_t(find(k.ikey));
    }
  });
}

bool HphpArray::nextInsert(const Variant& data) {
  assert(m_nextKI >= 0);
  assert(!isPacked());
  assert(!isFull());

  int64_t ki = m_nextKI;
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  auto ei = findForNewInsert(ki);
  assert(!validPos(*ei));
  // Allocate and initialize a new element.
  auto& e = allocElm(ei);
  e.setIntKey(ki);
  m_nextKI = ki + 1; // Update next free element.
  initVal(e.data, data);
  return true;
}

ArrayData* HphpArray::nextInsertRef(const Variant& data) {
  assert(!isPacked());
  assert(!isFull());
  assert(m_nextKI >= 0);

  int64_t ki = m_nextKI;
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  auto ei = findForNewInsert(ki);
  auto& e = allocElm(ei);
  e.setIntKey(ki);
  m_nextKI = ki + 1; // Update next free element.
  return initRef(e.data, data);
}

ArrayData* HphpArray::nextInsertWithRef(const Variant& data) {
  assert(!isFull());

  int64_t ki = m_nextKI;
  auto ei = findForNewInsert(ki);
  assert(!validPos(*ei));

  // Allocate a new element.
  auto& e = allocElm(ei);
  e.setIntKey(ki);
  m_nextKI = ki + 1; // Update next free element.
  return initWithRef(e.data, data);
}

template <class K>
ArrayData* HphpArray::addLvalImpl(K k, Variant*& ret) {
  assert(!isPacked());
  assert(!isFull());
  auto p = insert(k);
  if (!p.found) tvWriteNull(&p.tv);
  return getLval(p.tv, ret);
}

inline ArrayData* HphpArray::addVal(int64_t ki, const Variant& data) {
  assert(!exists(ki));
  assert(!isPacked());
  assert(!isFull());
  auto ei = findForNewInsert(ki);
  auto& e = allocElm(ei);
  e.setIntKey(ki);
  if (ki >= m_nextKI && m_nextKI >= 0) m_nextKI = ki + 1;
  return initVal(e.data, data);
}

inline ArrayData* HphpArray::addVal(StringData* key, const Variant& data) {
  assert(!exists(key));
  assert(!isPacked());
  assert(!isFull());
  strhash_t h = key->hash();
  auto ei = findForNewInsert(h);
  auto& e = allocElm(ei);
  e.setStrKey(key, h);
  return initVal(e.data, data);
}

template <class K> ALWAYS_INLINE
ArrayData* HphpArray::update(K k, const Variant& data) {
  assert(!isPacked());
  assert(!isFull());
  auto p = insert(k);
  if (p.found) {
    return setVal(p.tv, data);
  }
  return initVal(p.tv, data);
}

template <class K>
ArrayData* HphpArray::updateRef(K k, const Variant& data) {
  assert(!isPacked());
  assert(!isFull());
  auto p = insert(k);
  if (p.found) {
    return setRef(p.tv, data);
  }
  return initRef(p.tv, data);
}

template <class K> ALWAYS_INLINE
ArrayData* HphpArray::zSetImpl(K k, RefData* data) {
  auto p = insert(k);
  if (p.found) {
    return zSetVal(p.tv, data);
  }
  return zInitVal(p.tv, data);
}

ALWAYS_INLINE
ArrayData* HphpArray::zAppendImpl(RefData* data) {
  if (UNLIKELY(m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return this;
  }
  int64_t ki = m_nextKI;
  auto ei = findForNewInsert(ki);
  assert(!validPos(*ei));
  auto& e = allocElm(ei);
  e.setIntKey(ki);
  m_nextKI = ki + 1;
  return zInitVal(e.data, data);
}

ArrayData* HphpArray::LvalIntPacked(ArrayData* ad, int64_t k, Variant*& ret,
                                    bool copy) {
  auto a = asPacked(ad);

  if (size_t(k) < a->m_size) {
    if (copy) a = a->copyPacked();
    return a->getLval(a->data()[k].data, ret);
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

ArrayData* HphpArray::LvalInt(ArrayData* ad, int64_t k, Variant*& ret,
                              bool copy) {
  auto a = asMixed(ad);
  if (copy) {
    a = a->copyMixedAndResizeIfNeeded();
  } else {
    a = a->resizeIfNeeded();
  }
  return a->addLvalImpl(k, ret);
}

ArrayData*
HphpArray::LvalStrPacked(ArrayData* ad, StringData* key, Variant*& ret,
                         bool copy) {
  auto a = asPacked(ad);
  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();
  a = a->packedToMixed();
  return a->addLvalImpl(key, ret);
}

ArrayData* HphpArray::LvalStr(ArrayData* ad, StringData* key, Variant*& ret,
                              bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->addLvalImpl(key, ret);
}

ArrayData* HphpArray::LvalNewPacked(ArrayData* ad, Variant*& ret, bool copy) {
  auto a = asPacked(ad);
  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();
  auto& tv = a->allocNextElm(a->m_size);
  tvWriteUninit(&tv);  // TODO(#2942090)
  ret = &tvAsVariant(&tv);
  return a;
}

ArrayData* HphpArray::LvalNew(ArrayData* ad, Variant*& ret, bool copy) {
  auto a = asMixed(ad);
  if (UNLIKELY(a->m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    ret = &Variant::lvalBlackHole();
    return a;
  }

  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();

  if (UNLIKELY(!a->nextInsert(uninit_null()))) {
    ret = &Variant::lvalBlackHole();
    return a;
  }

  ret = &tvAsVariant(&a->data()[a->m_used - 1].data);
  return a;
}

ArrayData*
HphpArray::SetIntPacked(ArrayData* ad, int64_t k, const Variant& v, bool copy) {
  auto a = asPacked(ad);

  if (size_t(k) < a->m_size) {
    if (copy) a = a->copyPacked();
    return a->setVal(a->data()[k].data, v);
  }

  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();

  if (size_t(k) == a->m_size) {
    auto& tv = a->allocNextElm(k);
    return a->initVal(tv, v);
  }

  // Must escalate to mixed, but call addVal() since key doesn't
  // exist.
  a = a->packedToMixed();
  return a->addVal(k, v);
}

ArrayData* HphpArray::SetInt(ArrayData* ad, int64_t k, const Variant& v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->update(k, v);
}

ArrayData*
HphpArray::SetStrPacked(ArrayData* ad, StringData* k, const Variant& v, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  // must convert to mixed, but call addVal() since key doesn't exist.
  a = a->resizePackedIfNeeded();
  a = a->packedToMixed();
  return a->addVal(k, v);
}

ArrayData*
HphpArray::SetStr(ArrayData* ad, StringData* k, const Variant& v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->update(k, v);
}

ArrayData*
HphpArray::SetRefIntPacked(ArrayData* ad, int64_t k, const Variant& v, bool copy) {
  auto a = asPacked(ad);

  if (size_t(k) < a->m_size) {
    if (copy) a = a->copyPacked();
    return a->setRef(a->data()[k].data, v);
  }

  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();

  if (size_t(k) == a->m_size) {
    auto& tv = a->allocNextElm(k);
    return a->initRef(tv, v);
  }

  // todo t2606310: key can't exist.  use add/findForNewInsert
  a = a->packedToMixed();
  return a->updateRef(k, v);
}

ArrayData*
HphpArray::SetRefInt(ArrayData* ad, int64_t k, const Variant& v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->updateRef(k, v);
}

ArrayData*
HphpArray::SetRefStrPacked(ArrayData* ad, StringData* k, const Variant& v, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  // todo t2606310: key can't exist.  use add/findForNewInsert
  a = a->resizePackedIfNeeded();
  a = a->packedToMixed();
  return a->updateRef(k, v);
}

ArrayData*
HphpArray::SetRefStr(ArrayData* ad, StringData* k, const Variant& v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->updateRef(k, v);
}

ArrayData*
HphpArray::AddIntPacked(ArrayData* ad, int64_t k, const Variant& v, bool copy) {
  assert(!ad->exists(k));
  auto a = asPacked(ad);

  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();

  if (size_t(k) == a->m_size) {
    auto& tv = a->allocNextElm(k);
    return a->initVal(tv, v);
  }

  a = a->packedToMixed();
  return a->addVal(k, v);
}

ArrayData*
HphpArray::AddInt(ArrayData* ad, int64_t k, const Variant& v, bool copy) {
  assert(!ad->exists(k));
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->addVal(k, v);
}

ArrayData*
HphpArray::AddStr(ArrayData* ad, StringData* k, const Variant& v, bool copy) {
  assert(!ad->exists(k));
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->addVal(k, v);
}

ArrayData*
HphpArray::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  auto a = asHphpArray(ad);
  if (a->isPacked()) {
    a->packedToMixed();
  }
  a = a->resizeIfNeeded();
  return a->zSetImpl(k, v);
}

ArrayData*
HphpArray::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  auto a = asHphpArray(ad);
  if (a->isPacked()) {
    a->packedToMixed();
  }
  a = a->resizeIfNeeded();
  return a->zSetImpl(k, v);
}

ArrayData*
HphpArray::ZAppend(ArrayData* ad, RefData* v) {
  auto a = asHphpArray(ad);
  if (a->isPacked()) {
    a->packedToMixed();
  }
  a = a->resizeIfNeeded();
  return a->zAppendImpl(v);
}

//=============================================================================
// Delete.

NEVER_INLINE
void HphpArray::adjustMArrayIter(ssize_t pos) {
  ssize_t eIPrev = Tombstone;
  for_each_strong_iterator([&] (MIterTable::Ent& miEnt) {
    if (miEnt.array != this) return;
    auto const iter = miEnt.iter;
    if (iter->m_pos == pos) {
      if (eIPrev == Tombstone) {
        // eIPrev will actually be used, so properly initialize it with the
        // previous element before pos, or invalid_index if pos is the first
        // element.
        eIPrev = prevElm(data(), pos);
      }
      if (eIPrev == Empty) {
        iter->setResetFlag(true);
      }
      iter->m_pos = eIPrev;
    }
  });
}

void HphpArray::erase(ssize_t pos) {
  assert(validPos(pos));

  // move strong iterators to the previous element
  if (UNLIKELY(strong_iterators_exist())) adjustMArrayIter(pos);

  // If the internal pointer points to this element, advance it.
  Elm* elms = data();
  if (m_pos == pos) {
    m_pos = nextElm(elms, pos);
  }

  auto& e = elms[pos];
  // Mark the value as a tombstone.
  TypedValue* tv = &e.data;
  DataType oldType = tv->m_type;
  uint64_t oldDatum = tv->m_data.num;
  tv->m_type = KindOfInvalid;
  --m_size;
  // If this element was last, adjust m_used.
  if (size_t(pos + 1) == m_used) {
    do {
      --m_used;
    } while (m_used > 0 && isTombstone(elms[m_used - 1].data.m_type));
  }
  // Mark the hash entry as "deleted".
  assert(m_used <= m_cap);
  assert(m_hLoad <= m_cap);

  // Finally, decref the old value
  tvRefcountedDecRefHelper(oldType, oldDatum);

  if (m_size < m_used / 2) {
    // Compact in order to keep elms from being overly sparse.
    compact(false);
  }
}

ArrayData* HphpArray::RemoveIntPacked(ArrayData* ad, int64_t k, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  if (size_t(k) < a->m_size) {
    // escalate to mixed for correctness; unset preserves m_nextKI
    a->packedToMixed();
    auto pos = a->findForRemove(k, false);
    if (validPos(pos)) a->erase(pos);
  }
  return a; // key didn't exist, so we're still vector
}

ArrayData* HphpArray::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  auto pos = a->findForRemove(k, false);
  if (validPos(pos)) a->erase(pos);
  return a;
}

ArrayData*
HphpArray::RemoveStrPacked(ArrayData* ad, const StringData* key, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  return a;
}

ArrayData*
HphpArray::RemoveStr(ArrayData* ad, const StringData* key, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  auto pos = a->findForRemove(key, key->hash());
  if (validPos(pos)) a->erase(pos);
  return a;
}

ArrayData* HphpArray::CopyPacked(const ArrayData* ad) {
  return asPacked(ad)->copyPacked();
}

ArrayData* HphpArray::Copy(const ArrayData* ad) {
  return asMixed(ad)->copyMixed();
}

ArrayData* HphpArray::CopyWithStrongIterators(const ArrayData* ad) {
  auto a = asHphpArray(ad);
  auto copied = a->copyImpl();
  if (LIKELY(strong_iterators_exist())) {
    move_strong_iterators(copied, const_cast<HphpArray*>(a));
  }
  return copied;
}

//=============================================================================
// non-variant interface

TypedValue* HphpArray::NvGetIntPacked(const ArrayData* ad, int64_t ki) {
  auto a = asPacked(ad);
  return LIKELY(size_t(ki) < a->m_size) ? &a->data()[ki].data : nullptr;
}

TypedValue* HphpArray::NvGetInt(const ArrayData* ad, int64_t ki) {
  auto a = asMixed(ad);
  auto i = a->find(ki);
  return LIKELY(validPos(i)) ? &a->data()[i].data : nullptr;
}

TypedValue*
HphpArray::NvGetStrPacked(const ArrayData* ad, const StringData* k) {
  assert(asPacked(ad));
  return nullptr;
}

TypedValue* HphpArray::NvGetStr(const ArrayData* ad, const StringData* k) {
  auto a = asMixed(ad);
  auto i = a->find(k, k->hash());
  if (LIKELY(validPos(i))) {
    return &a->data()[i].data;
  }
  return nullptr;
}

// nvGetKey does not touch out->_count, so can be used
// for inner or outer cells.
void HphpArray::NvGetKeyPacked(const ArrayData* ad, TypedValue* out,
                               ssize_t pos) {
  DEBUG_ONLY auto a = asPacked(ad);
  assert(pos != invalid_index);
  assert(!isTombstone(a->data()[pos].data.m_type));
  out->m_data.num = pos;
  out->m_type = KindOfInt64;
}

void HphpArray::NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
  auto a = asMixed(ad);
  assert(pos != invalid_index);
  assert(!isTombstone(a->data()[pos].data.m_type));
  getElmKey(a->data()[pos], out);
}

ArrayData* HphpArray::AppendPacked(ArrayData* ad, const Variant& v, bool copy) {
  auto a = asPacked(ad);
  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();
  auto& tv = a->allocNextElm(a->m_size);
  return a->initVal(tv, v);
}

ArrayData* HphpArray::Append(ArrayData* ad, const Variant& v, bool copy) {
  auto a = asMixed(ad);
  if (UNLIKELY(a->m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return a;
  }
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  a->nextInsert(v);
  return a;
}

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
    return a->moveVal(tv, value);
  }
  return genericAddNewElemC(ad, value);
}

ArrayData* HphpArray::AppendRefPacked(ArrayData* ad, const Variant& v, bool copy) {
  auto a = asPacked(ad);
  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();
  auto& tv = a->allocNextElm(a->m_size);
  return a->initRef(tv, v);
}

ArrayData* HphpArray::AppendRef(ArrayData* ad, const Variant& v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();

  // Note: preserving behavior, but I think this can leak the copy if
  // the user error handler throws.
  if (UNLIKELY(a->m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return a;
  }

  // TODO: maybe inline manually (only caller).
  return a->nextInsertRef(v);
}

ArrayData *HphpArray::AppendWithRefPacked(ArrayData* ad, const Variant& v, bool copy) {
  auto a = asPacked(ad);
  a = copy ? a->copyPackedAndResizeIfNeeded()
           : a->resizePackedIfNeeded();
  auto& tv = a->allocNextElm(a->m_size);
  return a->initWithRef(tv, v);
}

ArrayData *HphpArray::AppendWithRef(ArrayData* ad, const Variant& v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->nextInsertWithRef(v);
}

/*
 * Copy an array to a new array of mixed kind, with a particular
 * pre-reserved size.  The input array may be either packed or mixed.
 */
NEVER_INLINE
HphpArray* HphpArray::CopyReserve(const HphpArray* src,
                                  size_t expectedSize) {
  auto const cmret = computeCapAndMask(expectedSize);
  auto const cap   = cmret.first;
  auto const mask  = cmret.second;
  auto const ad    = smartAllocArray(cap, mask);

  auto const oldPacked      = src->isPacked();
  auto const oldSize        = src->m_size;
  auto const oldPosUnsigned = static_cast<uint32_t>(src->m_pos);
  auto const oldNextKI      = oldPacked ? oldSize : src->m_nextKI;
  auto const oldUsed        = src->m_used;

  ad->m_kindAndSize     = uint64_t{oldSize} << 32 | kMixedKind;
  ad->m_posAndCount     = uint64_t{1} << 32 | oldPosUnsigned;
  ad->m_cap             = cap;
  ad->m_maskAndLoad     = uint64_t{oldSize} << 32 | mask;
  ad->m_nextKI          = oldNextKI;

  auto const data  = ad->data();
  auto const table = reinterpret_cast<int32_t*>(data + cap);
  ad->initHash(table, mask + 1);

  auto dstElm = data;
  auto srcElm = src->data();
  auto const srcStop = src->data() + oldUsed;
  uint32_t i = 0;
  if (oldPacked) {
    auto const mask = ad->m_tableMask;
    for (; srcElm != srcStop; ++srcElm, ++i) {
      tvDupFlattenVars(&srcElm->data, &dstElm->data, src);
      dstElm->setIntKey(i);
      *ad->findForNewInsert(table, mask, i) = i;
      ++dstElm;
    }
    assert(ad->m_pos == oldPosUnsigned);
  } else {
    // We're not copying the tombstones over to the new array, so the
    // positions of the elements in the new array may be shifted. Cache
    // the key for element associated with src->m_pos so that we can
    // properly initialize ad->m_pos below.
    ElmKey mPos;
    if (src->m_pos != invalid_index) {
      assert(size_t(src->m_pos) < src->m_used);
      auto& e = srcElm[src->m_pos];
      mPos.hash = e.hasIntKey() ? 0 : e.hash();
      mPos.key = e.key;
    } else {
      // Silence compiler warnings.
      mPos.hash = 0;
      mPos.key = nullptr;
    }
    // Copy the elements
    auto const mask = ad->m_tableMask;
    for (; srcElm != srcStop; ++srcElm) {
      if (isTombstone(srcElm->data.m_type)) continue;
      tvDupFlattenVars(&srcElm->data, &dstElm->data, src);
      auto const hasIntKey = srcElm->hasIntKey();
      auto const hash = hasIntKey ? srcElm->ikey : srcElm->hash();
      if (hasIntKey) {
        dstElm->setIntKey(srcElm->ikey);
      } else {
        srcElm->key->incRefCount();
        dstElm->setStrKey(srcElm->key, hash);
      }
      *ad->findForNewInsert(table, mask, hash) = i;
      ++dstElm;
      ++i;
    }
    // Now that we have finished copying the elements, update ad->m_pos
    if (src->m_pos != invalid_index) {
      ad->m_pos = mPos.hash
        ? ssize_t(ad->find(mPos.key, mPos.hash))
        : ssize_t(ad->find(mPos.ikey));
    }
  }

  // Set new used value (we've removed any tombstones).
  assert(i == dstElm - data);
  ad->m_used = i;

  assert(ad->m_kind == kMixedKind);
  assert(ad->m_size == oldSize);
  assert(ad->m_count == 1);
  assert(ad->m_cap == cap);
  assert(ad->m_used <= oldUsed);
  assert(ad->m_used == dstElm - data);
  assert(ad->m_hLoad == oldSize);
  assert(ad->m_tableMask == mask);
  assert(ad->m_nextKI == oldNextKI);
  assert(ad->checkInvariants());
  return ad;
}

NEVER_INLINE
ArrayData* HphpArray::ArrayPlusEqGeneric(ArrayData* ad,
                                         HphpArray* ret,
                                         const ArrayData* elems,
                                         size_t neededSize) {
  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    const Variant& value = it.secondRef();

    if (UNLIKELY(ret->isFull())) {
      assert(ret == ad);
      ret = CopyReserve(asHphpArray(ad), neededSize);
    }

    auto tv = key.asTypedValue();
    auto p = tv->m_type == KindOfInt64
      ? ret->insert(tv->m_data.num)
      : ret->insert(tv->m_data.pstr);
    if (!p.found) {
      ret->initWithRef(p.tv, value);
    }
  }

  return ret;
}

ArrayData* HphpArray::PlusEq(ArrayData* ad, const ArrayData* elems) {
  auto const neededSize = ad->size() + elems->size();

  auto ret =
    ad->hasMultipleRefs() ? CopyReserve(asHphpArray(ad), neededSize) :
    ad->isPacked()        ? asPacked(ad)->packedToMixed() :
    asMixed(ad);

  if (UNLIKELY(!elems->isMixed())) {
    return ArrayPlusEqGeneric(ad, ret, elems, neededSize);
  }

  auto const rhs = asMixed(elems);

  auto srcElem = rhs->data();
  auto const srcStop = rhs->data() + rhs->m_used;
  for (; srcElem != srcStop; ++srcElem) {
    if (isTombstone(srcElem->data.m_type)) continue;

    if (UNLIKELY(ret->isFull())) {
      assert(ret == ad);
      ret = CopyReserve(ret, neededSize);
    }

    auto const hash = srcElem->hash();
    if (srcElem->hasStrKey()) {
      auto const ei = ret->findForInsert(srcElem->key, hash);
      if (ret->validPos(*ei)) continue;
      auto& e = ret->allocElm(ei);
      e.setStrKey(srcElem->key, hash);
      ret->initWithRef(e.data, tvAsCVarRef(&srcElem->data));
      continue;
    }

    auto const ei = ret->findForInsert(srcElem->ikey);
    if (validPos(*ei)) continue;
    auto& e = ret->allocElm(ei);
    e.setIntKey(srcElem->ikey);
    ret->initWithRef(e.data, tvAsCVarRef(&srcElem->data));
  }

  return ret;
}

NEVER_INLINE
ArrayData* HphpArray::ArrayMergeGeneric(HphpArray* ret,
                                        const ArrayData* elems) {
  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    const Variant& value = it.secondRef();
    if (key.asTypedValue()->m_type == KindOfInt64) {
      ret->nextInsertWithRef(value);
    } else {
      Variant* p;
      StringData* sd = key.getStringData();
      ret->addLvalImpl(sd, p);
      p->setWithRef(value);
    }
  }
  return ret;
}

ArrayData* HphpArray::Merge(ArrayData* ad, const ArrayData* elems) {
  auto const ret = CopyReserve(asHphpArray(ad), ad->size() + elems->size());

  if (elems->isMixed()) {
    auto const rhs = asMixed(elems);
    auto srcElem = rhs->data();
    auto const srcStop = rhs->data() + rhs->m_used;
    for (; srcElem != srcStop; ++srcElem) {
      if (isTombstone(srcElem->data.m_type)) continue;

      if (srcElem->hasIntKey()) {
        ret->nextInsertWithRef(tvAsCVarRef(&srcElem->data));
      } else {
        Variant* p;
        ret->addLvalImpl(srcElem->key, p);
        p->setWithRef(tvAsCVarRef(&srcElem->data));
      }
    }
    return ret;
  }

  if (UNLIKELY(!elems->isPacked())) {
    return ArrayMergeGeneric(ret, elems);
  }

  auto const rhs = asPacked(elems);
  auto srcElem = rhs->data();
  auto const srcStop = rhs->data() + rhs->m_used;
  for (; srcElem != srcStop; ++srcElem) {
    assert(!isTombstone(srcElem->data.m_type));
    ret->nextInsertWithRef(tvAsCVarRef(&srcElem->data));
  }
  return ret;

  // Note: currently caller is responsible for calling renumber after
  // this.  Should refactor so we handle it (we already know things
  // about the array).
}

ArrayData* HphpArray::PopPacked(ArrayData* ad, Variant& value) {
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
    a->m_pos = a->m_size > 0 ? 0 : invalid_index; // reset internal iterator
    tvRefcountedDecRefHelper(oldType, oldDatum);
    return a;
  }
  value = uninit_null();
  a->m_pos = invalid_index; // reset internal iterator
  return a;
}

ArrayData* HphpArray::Pop(ArrayData* ad, Variant& value) {
  auto a = asMixed(ad);
  if (a->hasMultipleRefs()) a = a->copyMixed();
  auto elms = a->data();
  ssize_t pos = IterEnd(a);
  if (validPos(pos)) {
    auto& e = elms[pos];
    assert(!isTombstone(e.data.m_type));
    value = tvAsCVarRef(&e.data);
    auto pos2 = e.hasStrKey() ? a->findForRemove(e.key, e.hash()) :
                a->findForRemove(e.ikey, true);
    assert(pos2 == pos);
    a->erase(pos2);
  } else {
    value = uninit_null();
  }
  // To conform to PHP behavior, the pop operation resets the array's
  // internal iterator.
  a->m_pos = a->nextElm(elms, invalid_index);
  return a;
}

ArrayData* HphpArray::DequeuePacked(ArrayData* adInput, Variant& value) {
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
    a->m_pos = n > 0 ? 0 : invalid_index;
  } else {
    value = uninit_null();
    a->m_pos = invalid_index;
  }
  return a;
}

ArrayData* HphpArray::Dequeue(ArrayData* adInput, Variant& value) {
  auto a = asMixed(adInput);
  if (a->hasMultipleRefs()) a = a->copyMixed();
  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is removed from the beginning of the array.
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(a);
  }
  auto elms = a->data();
  ssize_t pos = a->nextElm(elms, invalid_index);
  if (validPos(pos)) {
    auto& e = elms[pos];
    value = tvAsCVarRef(&e.data);
    auto pos2 = e.hasStrKey() ? a->findForRemove(e.key, e.hash()) :
                a->findForRemove(e.ikey, false);
    assert(pos2 == pos);
    a->erase(pos2);
    a->compact(true);
  } else {
    value = uninit_null();
  }
  // To conform to PHP behavior, the dequeue operation resets the array's
  // internal iterator
  a->m_pos = a->nextElm(elms, invalid_index);
  return a;
}

ArrayData* HphpArray::PrependPacked(ArrayData* adInput,
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
    if (n == a->m_cap) a = GrowPacked(a);
    auto elms = a->data();
    memmove(&elms[1], &elms[0], n * sizeof(elms[0]));
  }
  a->m_size = a->m_used = n + 1;
  a->m_pos = 0;
  a->initVal(a->data()[0].data, v);
  return a;
}

ArrayData* HphpArray::Prepend(ArrayData* adInput,
                              const Variant& v,
                              bool copy) {
  auto a = asMixed(adInput);
  if (a->hasMultipleRefs()) a = a->copyMixedAndResizeIfNeeded();

  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is added to the beginning of the array.
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(a);
  }

  auto elms = a->data();
  if (a->m_used == 0 || !isTombstone(elms[0].data.m_type)) {
    // Make sure there is room to insert an element.
    a = a->resizeIfNeeded();
    // Recompute elms, in case resizeIfNeeded() had side effects.
    elms = a->data();
    // Move the existing elements to make element 0 available.
    memmove(&elms[1], &elms[0], a->m_used * sizeof(*elms));
    ++a->m_used;
  }

  // Prepend.
  ++a->m_size;
  auto& e = elms[0];
  e.setIntKey(0);
  a->initVal(e.data, v);

  // Renumber.
  a->compact(true);
  // To conform to PHP behavior, the prepend operation resets the array's
  // internal iterator
  a->m_pos = a->nextElm(elms, invalid_index);
  return a;
}

void HphpArray::RenumberPacked(ArrayData* ad) {
  assert(asPacked(ad)); // for the checkInvariants() call
  // renumber has no effect on Vector and doesn't move internal pos
}

void HphpArray::Renumber(ArrayData* ad) {
  asMixed(ad)->compact(true);
}

void HphpArray::OnSetEvalScalarPacked(ArrayData* ad) {
  auto a = asPacked(ad);
  Elm* elms = a->data();
  for (uint32_t i = 0, limit = a->m_size; i < limit; ++i) {
    tvAsVariant(&elms[i].data).setEvalScalar();
  }
}

void HphpArray::OnSetEvalScalar(ArrayData* ad) {
  auto a = asMixed(ad);
  auto elms = a->data();
  for (uint32_t i = 0, limit = a->m_used; i < limit; ++i) {
    auto& e = elms[i];
    if (!isTombstone(e.data.m_type)) {
      auto key = e.key;
      if (e.hasStrKey() && !key->isStatic()) {
        e.key = makeStaticString(key);
        decRefStr(key);
      }
      tvAsVariant(&e.data).setEvalScalar();
    }
  }
}

bool HphpArray::ValidMArrayIter(const ArrayData* ad, const MArrayIter& fp) {
  assert(fp.getContainer() == asHphpArray(ad));
  if (fp.getResetFlag()) return false;
  return fp.m_pos != invalid_index;
}

bool HphpArray::AdvanceMArrayIter(ArrayData* ad, MArrayIter& fp) {
  auto a = asHphpArray(ad);
  Elm* elms = a->data();
  if (fp.getResetFlag()) {
    fp.setResetFlag(false);
    fp.m_pos = invalid_index;
  } else if (fp.m_pos == invalid_index) {
    return false;
  }
  fp.m_pos = a->nextElm(elms, fp.m_pos);
  if (fp.m_pos == invalid_index) {
    return false;
  }
  // To conform to PHP behavior, we need to set the internal
  // cursor to point to the next element.
  a->m_pos = a->nextElm(elms, fp.m_pos);
  return true;
}

//////////////////////////////////////////////////////////////////////

}
