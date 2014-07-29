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

#include "hphp/runtime/base/mixed-array.h"

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

#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/array-iterator-defs.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

ArrayData* MixedArray::MakeReserveMixed(uint32_t capacity) {
  auto const cmret = computeCapAndMask(capacity);
  auto const cap   = cmret.first;
  auto const mask  = cmret.second;
  auto const ad    = smartAllocArray(cap, mask);

  ad->m_kindAndSize = kMixedKind << 24; // zero's size
  ad->m_posAndCount = uint64_t{1} << 32; // zero's pos
  ad->m_capAndUsed  = cap;
  ad->m_tableMask   = mask;
  ad->m_nextKI      = 0;

  auto const data = mixedData(ad);
  auto const hash = reinterpret_cast<int32_t*>(data + cap);
  wordfill(hash, Empty, mask + 1);

  assert(ad->m_kind == kMixedKind);
  assert(ad->m_size == 0);
  assert(ad->m_pos == 0);
  assert(ad->m_count == 1);
  assert(ad->m_cap == cap);
  assert(ad->m_used == 0);
  assert(ad->m_nextKI == 0);
  assert(ad->m_tableMask == mask);
  assert(ad->checkInvariants());
  return ad;
}

ArrayData* MixedArray::MakeReserveLike(const ArrayData* other,
                                       uint32_t capacity) {
  capacity = (capacity ? capacity : other->size());

  if (other->m_kind == kPackedKind) {
    return MixedArray::MakeReserve(capacity);
  } else {
    return MixedArray::MakeReserveMixed(capacity);
  }
}

ArrayData* MixedArray::MakeReserveIntMap(uint32_t capacity) {
  auto const cmret = computeCapAndMask(capacity);
  auto const cap   = cmret.first;
  auto const mask  = cmret.second;
  auto const ad    = smartAllocArray(cap, mask);

  ad->m_kindAndSize = kIntMapKind << 24; // zero's size
  ad->m_posAndCount = uint64_t{1} << 32; // zero's pos
  ad->m_capAndUsed  = cap;
  ad->m_tableMask   = mask;
  ad->m_nextKI      = 0;

  auto const data = reinterpret_cast<Elm*>(ad + 1);
  auto const hash = reinterpret_cast<int32_t*>(data + cap);
  wordfill(hash, Empty, mask + 1);

  assert(ad->m_kind == kIntMapKind);
  assert(ad->m_size == 0);
  assert(ad->m_pos == 0);
  assert(ad->m_count == 1);
  assert(ad->m_cap == cap);
  assert(ad->m_used == 0);
  assert(ad->m_nextKI == 0);
  assert(ad->m_tableMask == mask);
  assert(ad->checkInvariants());
  return ad;
}

ArrayData* MixedArray::MakeReserveStrMap(uint32_t capacity) {
  auto const cmret = computeCapAndMask(capacity);
  auto const cap   = cmret.first;
  auto const mask  = cmret.second;
  auto const ad    = smartAllocArray(cap, mask);

  ad->m_kindAndSize = kStrMapKind << 24; // zero's size
  ad->m_posAndCount = uint64_t{1} << 32; // zero's pos
  ad->m_capAndUsed  = cap;
  ad->m_tableMask   = mask;
  ad->m_nextKI      = 0;

  auto const data = reinterpret_cast<Elm*>(ad + 1);
  auto const hash = reinterpret_cast<int32_t*>(data + cap);
  wordfill(hash, Empty, mask + 1);

  assert(ad->m_kind == kStrMapKind);
  assert(ad->m_size == 0);
  assert(ad->m_pos == 0);
  assert(ad->m_count == 1);
  assert(ad->m_cap == cap);
  assert(ad->m_used == 0);
  assert(ad->m_nextKI == 0);
  assert(ad->m_tableMask == mask);
  assert(ad->checkInvariants());
  return ad;
}

ArrayData* MixedArray::MakePacked(uint32_t size, const TypedValue* values) {
  assert(size > 0);
  ArrayData* ad;
  if (LIKELY(size <= kPackedCapCodeThreshold)) {
    auto const cap = size;
    ad = static_cast<ArrayData*>(
      MM().objMallocLogged(sizeof(ArrayData) + sizeof(TypedValue) * cap)
    );
    assert(cap == packedCodeToCap(cap));
    ad->m_kindAndSize = uint64_t{size} << 32 | cap;  // sets kPackedKind
    assert(ad->m_kind == kPackedKind);
    assert(ad->m_size == size);
    assert(packedCodeToCap(ad->m_packedCapCode) == cap);
  } else {
    ad = MakePackedHelper(size, values);
  }

  ad->m_posAndCount = uint64_t{1} << 32; // zero's pos

  // Append values by moving -- Caller assumes we update refcount.
  // Values are in reverse order since they come from the stack, which
  // grows down.
  auto ptr = reinterpret_cast<TypedValue*>(ad + 1);
  for (auto i = uint32_t{0}; i < size; ++i) {
    auto const& src = values[size - i - 1];
    ptr->m_type = src.m_type;
    ptr->m_data = src.m_data;
    ++ptr;
  }

  assert(ad->m_pos == 0);
  assert(ad->m_count == 1);
  assert(PackedArray::checkInvariants(ad));
  return ad;
}

NEVER_INLINE
ArrayData*
MixedArray::MakePackedHelper(uint32_t size, const TypedValue* values) {
  auto const cap = roundUpPackedCap(size);
  auto const ad = static_cast<ArrayData*>(
    MM().objMallocLogged(sizeof(ArrayData) + sizeof(TypedValue) * cap)
  );
  auto const capCode = packedCapToCode(cap);
  ad->m_kindAndSize = uint64_t{size} << 32 | capCode;  // sets kPackedKind
  assert(ad->m_kind == kPackedKind);
  assert(ad->m_size == size);
  assert(packedCodeToCap(ad->m_packedCapCode) == cap);
  return ad;
}

MixedArray* MixedArray::MakeStruct(uint32_t size, StringData** keys,
                                 const TypedValue* values) {
  assert(size > 0);

  auto const cmret = computeCapAndMask(size);
  auto const cap   = cmret.first;
  auto const mask  = cmret.second;
  auto const ad    = smartAllocArray(cap, mask);

  auto const shiftedSize = uint64_t{size} << 32;
  ad->m_kindAndSize      = shiftedSize | kMixedKind << 24;
  ad->m_posAndCount      = uint64_t{1} << 32; // zero's pos
  ad->m_capAndUsed       = shiftedSize | cap;
  ad->m_tableMask        = mask;
  ad->m_nextKI           = 0;

  auto const data = mixedData(ad);
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

// for internal use by nonSmartCopy() and copyMixed()
template<class CopyKeyValue>
ALWAYS_INLINE
MixedArray* MixedArray::CopyMixed(const MixedArray& other,
                                  AllocMode mode,
                                  CopyKeyValue copyKeyValue) {
  assert(other.isMixed());

  auto const cap  = other.m_cap;
  auto const mask = other.m_tableMask;
  auto const ad = mode == AllocMode::Smart
    ? smartAllocArray(cap, mask)
    : mallocArray(cap, mask);
  auto const otherKind = other.m_kind;

  ad->m_kindAndSize     = uint64_t{other.m_size} << 32 | otherKind << 24;
  ad->m_posAndCount     = static_cast<uint32_t>(other.m_pos); // zero's count
  ad->m_capAndUsed      = uint64_t{other.m_used} << 32 | cap;
  ad->m_tableMask       = mask;
  ad->m_nextKI          = other.m_nextKI;

  auto const data      = mixedData(ad);
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

  // We need to assert this up here before we possibly call compact (which
  // will cause m_used to change)
  assert(ad->m_used == other.m_used);

  // If the element density dropped below 50% due to indirect elements
  // being converted into tombstones, we should do a compaction
  if (ad->m_size < ad->m_used / 2) {
    ad->compact(false);
  }

  assert(ad->m_kind == other.m_kind);
  assert(ad->m_size == other.m_size);
  assert(ad->m_pos == other.m_pos);
  assert(ad->m_count == 0);
  assert(ad->m_cap == cap);
  assert(ad->m_tableMask == mask);
  assert(ad->checkInvariants());
  return ad;
}

NEVER_INLINE ArrayData* MixedArray::NonSmartCopy(const ArrayData* in) {
  auto a = asMixed(in);
  assert(a->checkInvariants());
  return CopyMixed(
    *a,
    AllocMode::NonSmart,
    [&] (const Elm& from, Elm& to, const ArrayData* container) {
      to.skey = from.skey;
      to.data.hash() = from.data.hash();
      if (to.hasStrKey()) to.skey->incRefCount();
      tvDupFlattenVars(&from.data, &to.data, container);
      assert(to.hash() == from.hash()); // ensure not clobbered.
    }
  );
}

NEVER_INLINE MixedArray* MixedArray::copyMixed() const {
  assert(checkInvariants());
  return CopyMixed(*this, AllocMode::Smart,
      [&](const Elm& from, Elm& to, const ArrayData* container) {
        to.skey = from.skey;
        to.data.hash() = from.data.hash();
        if (to.hasStrKey()) to.skey->incRefCount();
        tvDupFlattenVars(&from.data, &to.data, container);
        assert(to.hash() == from.hash()); // ensure not clobbered.
      });
}

ALWAYS_INLINE
MixedArray* MixedArray::copyMixedAndResizeIfNeeded() const {
  if (LIKELY(!isFull())) return copyMixed();
  return copyMixedAndResizeIfNeededSlow();
}

NEVER_INLINE
MixedArray* MixedArray::copyMixedAndResizeIfNeededSlow() const {
  assert(isFull());

  // Note: this path will have to handle splitting strong iterators
  // later when we combine copyMixed & Grow into one operation.  For
  // now I'm just making use of copyMixed to do it for me before
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
      return init_null();
    case KindOfStaticString:
      return source.getStringData();

    case KindOfString: {
      auto const st = lookupStaticString(source.getStringData());
      if (st != nullptr) return st;
      return StringData::MakeUncounted(source.getStringData()->slice());
    }

    case KindOfArray: {
      auto const ad = source.getArrayData();
      return ad == staticEmptyArray() ? ad :
             ad->isPacked() ? MixedArray::MakeUncountedPacked(ad) :
             MixedArray::MakeUncounted(ad);
    }

    default:
      assert(false); // type not allowed
  }
  return init_null();
}

}

ArrayData* MixedArray::MakeUncounted(ArrayData* array) {
  auto a = asMixed(array);
  auto mixed = CopyMixed(
    *a,
    AllocMode::NonSmart,
    [&] (const Elm& fr, Elm& to, const ArrayData* container) {
      to.data.hash() = fr.data.hash();
      if (to.hasStrKey()) {
        auto const st = lookupStaticString(fr.skey);
        to.skey = (st != nullptr) ? st
                                  : StringData::MakeUncounted(fr.skey->slice());
      } else {
        to.skey = fr.skey;
      }
      tvCopy(
        *CreateVarForUncountedArray(tvAsCVarRef(&fr.data)).asTypedValue(),
        to.data);
      assert(to.hash() == fr.hash()); // ensure not clobbered.
    }
  );
  mixed->setUncounted();
  return mixed;
}

ArrayData* MixedArray::MakeUncountedPacked(ArrayData* array) {
  assert(PackedArray::checkInvariants(array));

  ArrayData* ad;
  auto const size = array->m_size;
  if (LIKELY(size <= kPackedCapCodeThreshold)) {
    // We don't need to copy the full capacity, since the array won't
    // change once it's uncounted.
    auto const cap = size;
    ad = static_cast<ArrayData*>(
      std::malloc(sizeof(ArrayData) + cap * sizeof(TypedValue))
    );
    assert(cap == packedCodeToCap(cap));
    ad->m_kindAndSize = uint64_t{size} << 32 | cap; // zero kind
    assert(ad->m_kind == ArrayData::kPackedKind);
    assert(packedCodeToCap(ad->m_packedCapCode) == cap);
    assert(ad->m_size == size);
  } else {
    ad = MakeUncountedPackedHelper(array);
  }

  ad->m_posAndCount = static_cast<uint64_t>(UncountedValue) << 32 |
                        static_cast<uint32_t>(array->m_pos);
  auto const srcData = packedData(array);
  auto const stop    = srcData + size;
  auto targetData    = reinterpret_cast<TypedValue*>(ad + 1);
  for (auto ptr = srcData; ptr != stop; ++ptr, ++targetData) {
    tvCopy(*CreateVarForUncountedArray(tvAsCVarRef(ptr)).asTypedValue(),
           *targetData);
  }
  assert(ad->m_pos == array->m_pos);
  assert(ad->m_count == UncountedValue);
  assert(ad->isUncounted());
  assert(PackedArray::checkInvariants(ad));
  return ad;
}

NEVER_INLINE
ArrayData* MixedArray::MakeUncountedPackedHelper(ArrayData* array) {
  auto const cap = roundUpPackedCap(array->m_size);
  auto const ad = static_cast<ArrayData*>(
    std::malloc(sizeof(ArrayData) + cap * sizeof(TypedValue))
  );
  auto const capCode = packedCapToCode(cap);
  auto const size = array->m_size;
  ad->m_kindAndSize = uint64_t{size} << 32 | capCode; // zero kind
  assert(ad->m_kind == ArrayData::kPackedKind);
  assert(packedCodeToCap(ad->m_packedCapCode) == cap);
  assert(ad->m_size == size);
  return ad;
}

//=============================================================================
// Destruction

NEVER_INLINE
void MixedArray::Release(ArrayData* in) {
  assert(in->isRefCounted());
  auto const ad = asMixed(in);

  if (!ad->isZombie()) {
    auto const data = ad->data();
    auto const stop = data + ad->m_used;

    for (auto ptr = data; ptr != stop; ++ptr) {
      if (isTombstone(ptr->data.m_type)) continue;
      if (ptr->hasStrKey()) decRefStr(ptr->skey);
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

static void release_unk_tv(TypedValue& tv) {
  if (tv.m_type == KindOfString) {
    assert(!tv.m_data.pstr->isRefCounted());
    if (!tv.m_data.pstr->isStatic()) {
      tv.m_data.pstr->destructStatic();
    }
    return;
  }

  if (tv.m_type == KindOfArray) {
    assert(!tv.m_data.parr->isRefCounted());
    if (!tv.m_data.parr->isStatic()) {
      if (tv.m_data.parr->isPacked()) {
        MixedArray::ReleaseUncountedPacked(tv.m_data.parr);
      } else {
        MixedArray::ReleaseUncounted(tv.m_data.parr);
      }
    }
    return;
  }

  assert(!IS_REFCOUNTED_TYPE(tv.m_type));
}

NEVER_INLINE
void MixedArray::ReleaseUncounted(ArrayData* in) {
  auto const ad = asMixed(in);
  assert(ad->m_count == UncountedValue);

  if (!ad->isZombie()) {
    auto const data = ad->data();
    auto const stop = data + ad->m_used;

    for (auto ptr = data; ptr != stop; ++ptr) {
      if (isTombstone(ptr->data.m_type)) continue;
      if (ptr->hasStrKey()) {
        assert(!ptr->skey->isRefCounted());
        if (!ptr->skey->isStatic()) {
          ptr->skey->destructStatic();
        }
      }

      release_unk_tv(ptr->data);
    }

    // We better not have strong iterators associated with uncounted
    // arrays.
    if (debug && UNLIKELY(strong_iterators_exist())) {
      for_each_strong_iterator([&] (const MIterTable::Ent& miEnt) {
        assert(miEnt.array != ad);
      });
    }
  }

  std::free(ad);
}

void MixedArray::ReleaseUncountedPacked(ArrayData* ad) {
  assert(PackedArray::checkInvariants(ad));
  assert(ad->m_count == UncountedValue);

  auto const data = packedData(ad);
  auto const stop = data + ad->m_size;
  for (auto ptr = data; ptr != stop; ++ptr) {
    release_unk_tv(*ptr);
  }

  // We better not have strong iterators associated with uncounted
  // arrays.
  if (debug && UNLIKELY(strong_iterators_exist())) {
    for_each_strong_iterator([&] (const MIterTable::Ent& miEnt) {
      assert(miEnt.array != ad);
    });
  }

  std::free(ad);
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
 *
 * kPackedKind:
 *   m_size == m_used
 *   m_nextKI = uninitialized
 *   Elm.skey uninitialized
 *   Elm.hash uninitialized
 *   no KindOfInvalid tombstones
 */
bool MixedArray::checkInvariants() const {
  static_assert(ssize_t(Empty) == ssize_t(-1), "");
  static_assert(sizeof(Elm) == 24, "");
  static_assert(sizeof(ArrayData) == 2 * sizeof(uint64_t), "");
  static_assert(
    sizeof(MixedArray) == sizeof(ArrayData) + 3 * sizeof(uint64_t),
    "Performance is sensitive to sizeof(MixedArray)."
    " Make sure you changed it with good reason and then update this assert."
  );

  // All arrays:
  assert(m_tableMask > 0 && ((m_tableMask+1) & m_tableMask) == 0);
  assert(m_tableMask == folly::nextPowTwo<uint64_t>(m_cap) - 1);
  assert(m_cap == computeMaxElms(m_tableMask));

  if (isZombie()) return true;

  // Non-zombie:
  assert(m_size <= m_used);
  assert(m_used <= m_cap);
  if (m_pos != m_used) {
    assert(size_t(m_pos) < m_used);
    assert(!isTombstone(data()[m_pos].data.m_type));
  }

  return true;
}

//=============================================================================
// Iteration.

inline ssize_t MixedArray::prevElm(Elm* elms, ssize_t ei) const {
  assert(ei < ssize_t(m_used));
  while (ei > 0) {
    --ei;
    if (!isTombstone(elms[ei].data.m_type)) {
      return ei;
    }
  }
  return m_used;
}

ssize_t MixedArray::IterBegin(const ArrayData* ad) {
  auto a = asMixed(ad);
  return a->nextElm(a->data(), -1);
}

ssize_t MixedArray::IterLast(const ArrayData* ad) {
  auto a = asMixed(ad);
  auto* elms = a->data();
  ssize_t ei = a->m_used;
  while (ei > 0) {
    --ei;
    if (!isTombstone(elms[ei].data.m_type)) {
      return ei;
    }
  }
  return a->m_used;
}

ssize_t MixedArray::IterEnd(const ArrayData* ad) {
  auto a = asMixed(ad);
  return a->m_used;
}

ssize_t MixedArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  auto a = asMixed(ad);
  ++pos;
  if (pos >= a->m_used) return a->m_used;
  if (!isTombstone(a->data()[pos].data.m_type)) {
    return pos;
  }
  return a->iter_advance_helper(pos);
}

// caller has already incremented pos but encountered a tombstone
ssize_t MixedArray::iter_advance_helper(ssize_t next_pos) const {
  Elm* elms = data();
  for (auto limit = m_used; size_t(next_pos) < limit; ++next_pos) {
    if (!isTombstone(elms[next_pos].data.m_type)) {
      return next_pos;
    }
  }
  return m_used;
}

ssize_t MixedArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  auto a = asMixed(ad);
  return a->prevElm(a->data(), pos);
}

size_t MixedArray::Vsize(const ArrayData*) { not_reached(); }

const Variant& MixedArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  auto a = asMixed(ad);
  assert(a->checkInvariants());
  assert(pos != a->m_used);
  auto& e = a->data()[pos];
  assert(!isTombstone(e.data.m_type));
  return tvAsCVarRef(&e.data);
}

bool MixedArray::IsVectorData(const ArrayData* ad) {
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

static bool hitStringKey(const MixedArray::Elm& e, const StringData* s,
                         int32_t hash) {
  // hitStringKey() should only be called on an Elm that is referenced by a
  // hash table entry. MixedArray guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!MixedArray::isTombstone(e.data.m_type));
  return hash == e.hash() && (s == e.skey || s->same(e.skey));
}

static bool hitIntKey(const MixedArray::Elm& e, int64_t ki) {
  // hitIntKey() should only be called on an Elm that is referenced by a
  // hash table entry. MixedArray guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!MixedArray::isTombstone(e.data.m_type));
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
ssize_t MixedArray::findImpl(size_t h0, Hit hit) const {
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

ssize_t MixedArray::find(int64_t ki) const {
  return findImpl(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

ssize_t MixedArray::find(const StringData* s, strhash_t prehash) const {
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
int32_t* MixedArray::findForInsertImpl(size_t h0, Hit hit) const {
  // tableMask, probeIndex, and pos are explicitly 64-bit, because performance
  // regressed when they were 32-bit types via auto.  Test carefully.
  size_t tableMask = m_tableMask;
  auto* elms = data();
  auto* hashtable = hashTab();
  for (size_t probeIndex = h0, i = 1;; ++i) {
    auto ei = &hashtable[probeIndex & tableMask];
    ssize_t pos = *ei;
    if (validPos(pos)) {
      if (hit(elms[pos])) {
        return ei;
      }
    } else if (pos == Empty) {
      return ei;
    }
    probeIndex += i;
    assert(i <= tableMask && probeIndex == h0 + (i + i*i) / 2);
  }
}

int32_t* MixedArray::findForInsert(int64_t ki) const {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  return findForInsertImpl(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

int32_t*
MixedArray::findForInsert(const StringData* s, strhash_t prehash) const {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  auto h = prehash | STRHASH_MSB;
  return findForInsertImpl(prehash, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

MixedArray::InsertPos MixedArray::insert(int64_t k) {
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

MixedArray::InsertPos MixedArray::insert(StringData* k) {
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
ssize_t MixedArray::findForRemoveImpl(size_t h0, Hit hit, Remove remove) const {
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
ssize_t MixedArray::findForRemove(int64_t ki, bool updateNext) {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  return findForRemoveImpl(ki,
      [&] (const Elm& e) {
        return hitIntKey(e, ki);
      },
      [this, ki, updateNext] (Elm& e) {
        assert(ki == e.ikey);
        // Conform to PHP5 behavior
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
MixedArray::findForRemove(const StringData* s, strhash_t prehash) {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  auto h = prehash | STRHASH_MSB;
  return findForRemoveImpl(prehash,
      [&] (const Elm& e) {
        return hitStringKey(e, s, h);
      },
      [] (Elm& e) {
        decRefStr(e.skey);
        e.setIntKey(0);
      }
    );
}

bool MixedArray::ExistsInt(const ArrayData* ad, int64_t k) {
  return ExistsIntImpl<kMixedKind>(ad, k);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
bool MixedArray::ExistsIntImpl(const ArrayData* ad, int64_t k) {
  if (aKind == kStrMapKind) {
    MixedArray::warnUsage(Reason::kExistsInt, aKind);
  }
  auto a = asMixed(ad);
  return validPos(a->find(k));
}

template bool
MixedArray::ExistsIntImpl<ArrayData::kStrMapKind>(const ArrayData*, int64_t);

bool MixedArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  return MixedArray::ExistsStrImpl<kMixedKind>(ad, k);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
bool MixedArray::ExistsStrImpl(const ArrayData* ad, const StringData* k) {
  if (aKind != kMixedKind) {
    MixedArray::warnUsage(Reason::kExistsStr, aKind);
  }
  auto a = asMixed(ad);
  return validPos(a->find(k, k->hash()));
}

template bool
MixedArray::ExistsStrImpl<ArrayData::kIntMapKind>(const ArrayData* ad,
                                                  const StringData* k);

//=============================================================================
// Append/insert/update.

ALWAYS_INLINE
MixedArray* MixedArray::initVal(TypedValue& tv, Cell v) {
  cellDup(v, tv);
  // TODO(#3888164): we should restructure things so we don't have to
  // check KindOfUninit here.
  if (UNLIKELY(tv.m_type == KindOfUninit)) {
    tv.m_type = KindOfNull;
  }
  return this;
}

ALWAYS_INLINE
ArrayData* MixedArray::zInitVal(TypedValue& tv, RefData* v) {
  tv.m_type = KindOfRef;
  tv.m_data.pref = v;
  return this;
}

ALWAYS_INLINE
MixedArray* MixedArray::initRef(TypedValue& tv, Variant& v) {
  tvAsUninitializedVariant(&tv).constructRefHelper(v);
  return this;
}

ALWAYS_INLINE
MixedArray* MixedArray::getLval(TypedValue& tv, Variant*& ret) {
  ret = &tvAsVariant(&tv);
  return this;
}

ALWAYS_INLINE
MixedArray* MixedArray::initLval(TypedValue& tv, Variant*& ret) {
  tvWriteNull(&tv);
  ret = &tvAsVariant(&tv);
  return this;
}

ALWAYS_INLINE
MixedArray* MixedArray::initWithRef(TypedValue& tv, const Variant& v) {
  tvWriteNull(&tv);
  tvAsVariant(&tv).setWithRef(v);
  return this;
}

ALWAYS_INLINE
MixedArray* MixedArray::setVal(TypedValue& tv, Cell src) {
  auto const dst = tvToCell(&tv);
  cellSet(src, *dst);
  // TODO(#3888164): we should restructure things so we don't have to
  // check KindOfUninit here.
  if (UNLIKELY(src.m_type == KindOfUninit)) {
    dst->m_type = KindOfNull;
  }
  return this;
}

ALWAYS_INLINE
ArrayData* MixedArray::zSetVal(TypedValue& tv, RefData* v) {
  // Dec ref the old value
  tvRefcountedDecRef(tv);
  // Store the RefData but do not increment the refcount
  tv.m_type = KindOfRef;
  tv.m_data.pref = v;
  return this;
}

/*
 * This is a streamlined copy of Variant.constructValHelper()
 * with no incref+decref because we're moving v to this array.
 */
ALWAYS_INLINE
MixedArray* MixedArray::moveVal(TypedValue& tv, TypedValue v) {
  tv.m_type = typeInitNull(v.m_type);
  tv.m_data.num = v.m_data.num;
  return this;
}

ALWAYS_INLINE MixedArray* MixedArray::resizeIfNeeded() {
  if (UNLIKELY(isFull())) return resize();
  return this;
}

NEVER_INLINE MixedArray* MixedArray::resize() {
  uint32_t maxElms = computeMaxElms(m_tableMask);
  assert(m_used <= maxElms);
  // At a minimum, compaction is required.  If the load factor would be >0.5
  // even after compaction, grow instead, in order to avoid the possibility
  // of repeated compaction if the load factor were to hover at nearly 0.75.
  if (m_size > maxElms / 2) {
    assert(m_tableMask <= 0x7fffffffU);
    return Grow(this, maxElms * 2, m_tableMask * 2 + 1);
  }
  compact(false);
  return this;
}

void MixedArray::downgradeAndWarn(ArrayData* ad, const Reason r) {
  assert(ad->isIntMapArray() || ad->isStrMapArray());
  MixedArray::warnUsage(r, ad->m_kind);
  ad->m_kind = kMixedKind;
}

void MixedArray::warnUsage(const Reason r, const ArrayKind kind) {
  assert(kind == kIntMapKind || kind == kStrMapKind);
  if (!RuntimeOption::EvalHackArrayWarnFrequency) {
    return;
  }
  static __thread uint32_t numWarnings = 0;
  numWarnings++;
  if (numWarnings % RuntimeOption::EvalHackArrayWarnFrequency != 0) {
    return;
  }
  auto arrayName = kind == kIntMapKind ? "miarray" : "msarray";
  switch (r) {
  case Reason::kForeachByRef:
    raise_warning("Foreach by reference over a %s, converting to array",
                  arrayName);
    break;
  case Reason::kPrepend:
    raise_warning("Using array_unshift on a %s, converting to array",
                  arrayName);
    break;
  case Reason::kPop:
    raise_warning("Using array_pop on a %s, converting to array", arrayName);
    break;
  case Reason::kTakeByRef:
    raise_warning("Taking an element by reference from a %s, "
                  "converting to array", arrayName);
    break;
  case Reason::kSetRef:
    raise_warning("Adding a reference to a %s, converting to array", arrayName);
    break;
  case Reason::kAppendRef:
    raise_warning("Appending a reference to a %s, converting to array",
                  arrayName);
    break;
  case Reason::kAppend:
    raise_warning("Appending to a %s, converting to array", arrayName);
    break;
  case Reason::kNvGetInt:
    raise_warning("Reading int key from a msarray, converting to array");
    break;
  case Reason::kNvGetStr:
    raise_warning("Reading string key from a miarray, converting to array");
    break;
  case Reason::kExistsInt:
    raise_warning("Reading int key from a msarray, converting to array");
    break;
  case Reason::kExistsStr:
    raise_warning("Reading string key from a miarray, converting to array");
    break;
  case Reason::kSetInt:
    raise_warning("Adding an int key to a msarray, converting to array");
    break;
  case Reason::kSetStr:
    raise_warning("Adding a string key to a miarray, converting to array");
    break;
  case Reason::kRemoveInt:
    raise_warning("Removing an int key from a msarray, converting to array");
    break;
  case Reason::kRemoveStr:
    raise_warning("Removing a string key from a miarray, converting to array");
    break;
  case Reason::kDequeue:
    raise_warning("Using array_shift on a %s, converting to array", arrayName);
    break;
  case Reason::kSort:
    raise_warning("Using sort on a %s, converting to array", arrayName);
    break;
  case Reason::kUsort:
    raise_warning("Using usort on a %s, converting to array", arrayName);
    break;
  case Reason::kNumericString:
    raise_warning("An integer-like string key used with a miarray");
    break;
  case Reason::kRenumber:
    raise_warning("Trying to renumber keys on a %s, converting to array",
                  arrayName);
    break;
  }
}

void NEVER_INLINE
MixedArray::InsertCheckUnbalanced(MixedArray* ad,
                                  int32_t* table,
                                  uint32_t mask,
                                  Elm* iter,
                                  Elm* stop) {
  for (uint32_t i = 0; iter != stop; ++iter, ++i) {
    auto& e = *iter;
    if (isTombstone(e.data.m_type)) continue;
    *ad->findForNewInsertCheckUnbalanced(table, mask,
                                         e.hasIntKey() ? e.ikey : e.hash())
      = i;
  }
}

MixedArray*
MixedArray::Grow(MixedArray* old, uint32_t newCap, uint32_t newMask) {
  assert(!old->isPacked());
  assert(old->m_size > 0);
  assert(newCap >= old->m_size);
  assert(newMask > 0 && ((newMask+1) & newMask) == 0);
  assert(newMask == folly::nextPowTwo<uint64_t>(newCap) - 1);
  assert(newCap == computeMaxElms(newMask));

  DEBUG_ONLY auto oldPos = old->m_pos;

  auto const mask       = newMask;
  auto const cap        = newCap;
  auto const ad         = smartAllocArray(cap, mask);
  auto const oldKind    = old->m_kind;

  auto const oldSize        = old->m_size;
  auto const oldPosUnsigned = static_cast<uint32_t>(old->m_pos);
  auto const oldUsed        = old->m_used;

  ad->m_kindAndSize     = uint64_t{oldSize} << 32 | oldKind << 24;
  ad->m_posAndCount     = oldPosUnsigned; // zero's count
  ad->m_capAndUsed      = uint64_t{oldUsed} << 32 | cap;
  ad->m_tableMask       = mask;
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
  if (UNLIKELY(oldUsed >= 2000)) {
    InsertCheckUnbalanced(ad, table, mask, iter, stop);
  } else {
    for (uint32_t i = 0; iter != stop; ++iter, ++i) {
      auto& e = *iter;
      if (isTombstone(e.data.m_type)) continue;
      *ad->findForNewInsert(table, mask, e.hasIntKey() ? e.ikey : e.hash()) = i;
    }
  }

  old->setZombie();

  assert(old->isZombie());
  assert(ad->m_kind == oldKind);
  assert(ad->m_size == oldSize);
  assert(ad->m_count == 0);
  assert(ad->m_pos == oldPos);
  assert(ad->m_used == oldUsed);
  assert(ad->m_tableMask == mask);
  assert(ad->checkInvariants());
  return ad;
}

namespace {
struct ElmKey {
  ElmKey() {}
  ElmKey(int32_t hash, StringData* key)
    : hash(hash)
    , skey(key)
  {}
  int32_t hash;
  union {
    StringData* skey;
    int64_t ikey;
  };
};
}

void MixedArray::compact(bool renumber /* = false */) {
  assert(!isPacked());
  ElmKey mPos;

  bool updatePosAfterCompact;
  bool hasStrongIters;
  TinyVector<ElmKey,3> siKeys;

  // Prep work before beginning the compaction process
  if (LIKELY(!renumber)) {
    if ((updatePosAfterCompact = (m_pos != 0 && m_pos != m_used))) {
      // Cache key for element associated with m_pos in order to
      // update m_pos after the compaction has been performed.
      // We only need to do this if m_pos is nonzero and is not
      // the canonical invalid position.
      assert(size_t(m_pos) < m_used);
      auto& e = data()[m_pos];
      mPos.hash = e.hasIntKey() ? 0 : e.hash();
      mPos.skey = e.skey;
    } else {
      if (m_pos == m_used) {
        // If m_pos is the canonical invalid position, we need to update
        // it to what the new canonical invalid position will be after
        // compaction
        m_pos = m_size;
      }
      mPos.hash = 0;
      mPos.skey = nullptr;
    }
    if (UNLIKELY((hasStrongIters = strong_iterators_exist()))) {
      for_each_strong_iterator([&] (const MIterTable::Ent& miEnt) {
        if (miEnt.array != this) return;
        if (miEnt.iter->getResetFlag()) return;
        auto const ei = miEnt.iter->m_pos;
        if (ei == m_used) return;
        auto& e = data()[ei];
        siKeys.push_back(ElmKey(e.hash(), e.skey));
      });
    }
  } else {
    // To conform to PHP5 behavior, when array's integer keys are renumbered
    // we invalidate all strong iterators and we reset the array's internal
    // cursor (even if the array is empty or has no integer keys).
    if (UNLIKELY(strong_iterators_exist())) {
      free_strong_iterators(this);
    }
    m_pos = 0;
    mPos.hash = 0;
    mPos.skey = nullptr;
    updatePosAfterCompact = false;
    hasStrongIters = false;
    // Set m_nextKI to 0 for now to prepare for renumbering integer keys
    m_nextKI = 0;
  }

  // Perform compaction
  auto elms = data();
  auto mask = m_tableMask;
  size_t tableSize = mask + 1;
  auto table = hashTab();
  initHash(table, tableSize);
  for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
    while (isTombstone(elms[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = elms[toPos];
    if (toPos != frPos) {
      toE = elms[frPos];
    }
    if (UNLIKELY(renumber && !toE.hasStrKey())) {
      toE.ikey = m_nextKI++;
    }
    auto ie = findForNewInsert(table, mask,
                               toE.hasIntKey() ? toE.ikey : toE.hash());
    *ie = toPos;
  }

  if (updatePosAfterCompact) {
    // Update m_pos, now that compaction is complete
    m_pos = mPos.hash ? ssize_t(find(mPos.skey, mPos.hash))
                      : ssize_t(find(mPos.ikey));
  }

  if (LIKELY(!hasStrongIters)) {
    // In the common case there aren't any strong iterators, so we
    // can just update m_used and return
    m_used = m_size;
    return;
  }

  // Update strong iterators now that compaction is complete. Note
  // that we wait to update m_used until after we've updated the
  // strong iterators because we need to consult what the _old_ value
  // of m_used before compaction was performed.
  int key = 0;
  for_each_strong_iterator(
    [&] (MIterTable::Ent& miEnt) {
      if (miEnt.array != this) return;
      auto const iter = miEnt.iter;
      if (iter->getResetFlag()) return;
      if (iter->m_pos == m_used) {
        // If this iterator was set to the _old_ canonical invalid position,
        // we need to update it to the _new_ canonical invalid position after
        // compaction.
        iter->m_pos = m_size;
        return;
      }
      auto& k = siKeys[key];
      key++;
      iter->m_pos = k.hash ? ssize_t(find(k.skey, k.hash))
                           : ssize_t(find(k.ikey));
    }
  );
  // Finally, update m_used and return
  m_used = m_size;
}

bool MixedArray::nextInsert(const Variant& data) {
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
  initVal(e.data, *data.asCell());
  return true;
}

ArrayData* MixedArray::nextInsertRef(Variant& data) {
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

ArrayData* MixedArray::nextInsertWithRef(const Variant& data) {
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

template <class K> ALWAYS_INLINE
ArrayData* MixedArray::update(K k, Cell data) {
  assert(!isPacked());
  assert(!isFull());
  auto p = insert(k);
  if (p.found) {
    return setVal(p.tv, data);
  }
  return initVal(p.tv, data);
}

template <class K> ALWAYS_INLINE
ArrayData* MixedArray::zSetImpl(K k, RefData* data) {
  auto p = insert(k);
  if (p.found) {
    return zSetVal(p.tv, data);
  }
  return zInitVal(p.tv, data);
}

ALWAYS_INLINE
ArrayData* MixedArray::zAppendImpl(RefData* data, int64_t* key_ptr) {
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
  *key_ptr = ki;
  return zInitVal(e.data, data);
}

ArrayData* MixedArray::LvalInt(ArrayData* ad, int64_t k, Variant*& ret,
                              bool copy) {
  return LvalIntImpl<kMixedKind>(ad, k, ret, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
ArrayData* MixedArray::LvalIntImpl(ArrayData* ad, int64_t k, Variant*& ret,
                                   bool copy) {
  auto a = asMixed(ad);
  if (copy) {
    a = a->copyMixedAndResizeIfNeeded();
  } else {
    a = a->resizeIfNeeded();
  }
  if (aKind == kStrMapKind) {
    MixedArray::downgradeAndWarn(a, Reason::kSetInt);
  }
  return a->addLvalImpl(k, ret);
}

template ArrayData*
MixedArray::LvalIntImpl<ArrayData::kStrMapKind>(ArrayData* ad, int64_t k,
                                                Variant*& ret, bool copy);

ArrayData* MixedArray::LvalStr(ArrayData* ad,
                              StringData* key,
                              Variant*& ret,
                              bool copy) {
  return LvalStrImpl<kMixedKind>(ad, key, ret, copy);
}


template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
ArrayData* MixedArray::LvalStrImpl(ArrayData* ad,
                                   StringData* key,
                                   Variant*& ret,
                                   bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kSetStr);
  }
  return a->addLvalImpl(key, ret);
}

template ArrayData*
MixedArray::LvalStrImpl<ArrayData::kIntMapKind>(ArrayData* ad, StringData* key,
                                                Variant*& ret, bool copy);

ArrayData* MixedArray::LvalNew(ArrayData* ad, Variant*& ret, bool copy) {
  return LvalNewImpl<kMixedKind>(ad, ret, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
ArrayData* MixedArray::LvalNewImpl(ArrayData* ad, Variant*& ret, bool copy) {
  auto a = asMixed(ad);
  if (UNLIKELY(a->m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    ret = &lvalBlackHole();
    return a;
  }

  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();

  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kAppend);
  }

  if (UNLIKELY(!a->nextInsert(uninit_null()))) {
    ret = &lvalBlackHole();
    return a;
  }

  ret = &tvAsVariant(&a->data()[a->m_used - 1].data);
  return a;
}

template ArrayData*
MixedArray::LvalNewImpl<ArrayData::kIntMapKind>(ArrayData* ad, Variant*& ret,
                                                bool copy);

template ArrayData*
MixedArray::LvalNewImpl<ArrayData::kStrMapKind>(ArrayData* ad, Variant*& ret,
                                                bool copy);

ArrayData* MixedArray::SetInt(ArrayData* ad, int64_t k, Cell v, bool copy) {
  return SetIntImpl<kMixedKind>(ad, k, v, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE ArrayData*
MixedArray::SetIntImpl(ArrayData* ad, int64_t k, Cell v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  if (aKind == kStrMapKind) {
    MixedArray::downgradeAndWarn(a, Reason::kSetInt);
  }
  return a->update(k, v);
}

template ArrayData*
MixedArray::SetIntImpl<ArrayData::kStrMapKind>(ArrayData*, int64_t, Cell, bool);

ArrayData* MixedArray::SetIntConverted(ArrayData* ad, int64_t k, Cell v,
                                       bool copy) {
  assert(ad->isIntMapArray());
  MixedArray::warnUsage(Reason::kNumericString, ad->m_kind);
  return MixedArray::SetInt(ad, k, v, copy);
}

ArrayData*
MixedArray::SetStr(ArrayData* ad, StringData* k, Cell v, bool copy) {
  return SetStrImpl<kMixedKind>(ad, k, v, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE ArrayData*
MixedArray::SetStrImpl(ArrayData* ad, StringData* k, Cell v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kSetStr);
  }
  return a->update(k, v);
}

template ArrayData*
MixedArray::SetStrImpl<ArrayData::kIntMapKind>(ArrayData* ad, StringData* k,
                                               Cell v, bool copy);

ArrayData*
MixedArray::SetRefInt(ArrayData* ad, int64_t k, Variant& v, bool copy) {
  return SetRefIntImpl<kMixedKind>(ad, k, v, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE ArrayData*
MixedArray::SetRefIntImpl(ArrayData* ad, int64_t k, Variant& v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kSetRef);
  }
  return a->updateRef(k, v);
}

template ArrayData*
MixedArray::SetRefIntImpl<ArrayData::kIntMapKind>(ArrayData* ad, int64_t k,
                                                  Variant& v, bool copy);

template ArrayData*
MixedArray::SetRefIntImpl<ArrayData::kStrMapKind>(ArrayData* ad, int64_t k,
                                                  Variant& v, bool copy);

ArrayData*
MixedArray::SetRefStr(ArrayData* ad, StringData* k, Variant& v, bool copy) {
  return SetRefStrImpl<kMixedKind>(ad, k, v, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE ArrayData*
MixedArray::SetRefStrImpl(ArrayData* ad, StringData* k, Variant& v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kSetRef);
  }
  return a->updateRef(k, v);
}

template ArrayData*
MixedArray::SetRefStrImpl<ArrayData::kIntMapKind>(ArrayData* ad, StringData* k,
                                                  Variant& v, bool copy);

template ArrayData*
MixedArray::SetRefStrImpl<ArrayData::kStrMapKind>(ArrayData* ad, StringData* k,
                                                  Variant& v, bool copy);

ArrayData*
MixedArray::AddInt(ArrayData* ad, int64_t k, Cell v, bool copy) {
  assert(!ad->exists(k));
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  return a->addVal(k, v);
}

ArrayData*
MixedArray::AddStr(ArrayData* ad, StringData* k, Cell v, bool copy) {
  return AddStrImpl<kMixedKind>(ad, k, v, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE ArrayData*
MixedArray::AddStrImpl(ArrayData* ad, StringData* k, Cell v, bool copy) {
  assert(!ad->exists(k));
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kSetStr);
  }
  return a->addVal(k, v);
}

ArrayData*
MixedArray::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  auto a = asMixed(ad);
  a = a->resizeIfNeeded();
  return a->zSetImpl(k, v);
}

ArrayData*
MixedArray::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  auto a = asMixed(ad);
  a = a->resizeIfNeeded();
  return a->zSetImpl(k, v);
}

ArrayData*
MixedArray::ZAppend(ArrayData* ad, RefData* v, int64_t* key_ptr) {
  auto a = asMixed(ad);
  a = a->resizeIfNeeded();
  return a->zAppendImpl(v, key_ptr);
}

//=============================================================================
// Delete.

NEVER_INLINE
void MixedArray::adjustMArrayIter(ssize_t pos) {
  assert(pos >= 0 && pos < m_used);
  ssize_t eIPrev = Tombstone;
  for_each_strong_iterator([&] (MIterTable::Ent& miEnt) {
    if (miEnt.array != this) return;
    auto const iter = miEnt.iter;
    if (iter->getResetFlag()) return;
    if (iter->m_pos == pos) {
      if (eIPrev == Tombstone) {
        // eIPrev will actually be used, so properly initialize it with the
        // previous element before pos (or an invalid position if pos was the
        // first element).
        eIPrev = prevElm(data(), pos);
      }

      if (eIPrev == m_used) {
        iter->setResetFlag(true);
      }
      iter->m_pos = eIPrev;
    }
  });
}

void MixedArray::eraseNoCompact(ssize_t pos) {
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
  // Mark the hash entry as "deleted".
  assert(m_used <= m_cap);

  // Finally, decref the old value
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

ArrayData* MixedArray::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  return RemoveIntImpl<kMixedKind>(ad, k, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE ArrayData*
MixedArray::RemoveIntImpl(ArrayData* ad, int64_t k, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  if (aKind == kStrMapKind) {
    MixedArray::warnUsage(Reason::kRemoveInt, aKind);
  }
  auto pos = a->findForRemove(k, false);
  if (validPos(pos)) a->erase(pos);
  return a;
}

template ArrayData*
MixedArray::RemoveIntImpl<ArrayData::kStrMapKind>(ArrayData* ad, int64_t k,
                                                  bool copy);

ArrayData*
MixedArray::RemoveStr(ArrayData* ad, const StringData* key, bool copy) {
  return RemoveStrImpl<kMixedKind>(ad, key, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE ArrayData*
MixedArray::RemoveStrImpl(ArrayData* ad, const StringData* key, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kRemoveStr);
  }
  auto pos = a->findForRemove(key, key->hash());
  if (validPos(pos)) a->erase(pos);
  return a;
}

template ArrayData*
MixedArray::RemoveStrImpl<ArrayData::kIntMapKind>(ArrayData* ad,
                                                  const StringData* key,
                                                  bool copy);

ArrayData* MixedArray::Copy(const ArrayData* ad) {
  return asMixed(ad)->copyMixed();
}

ArrayData* MixedArray::CopyWithStrongIterators(const ArrayData* ad) {
  auto a = asMixed(ad);
  auto copied = a->copyMixed();
  if (LIKELY(strong_iterators_exist())) {
    move_strong_iterators(copied, const_cast<MixedArray*>(a));
  }
  return copied;
}

//=============================================================================
// non-variant interface

const TypedValue* MixedArray::NvGetInt(const ArrayData* ad, int64_t ki) {
  return NvGetIntImpl<kMixedKind>(ad, ki);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
const TypedValue* MixedArray::NvGetIntImpl(const ArrayData* ad, int64_t ki) {
  if (aKind != kMixedKind) {
    MixedArray::warnUsage(Reason::kNvGetInt, aKind);
  }
  auto a = asMixed(ad);
  auto i = a->find(ki);
  return LIKELY(validPos(i)) ? &a->data()[i].data : nullptr;
}

template const TypedValue*
MixedArray::NvGetIntImpl<ArrayData::kStrMapKind>(const ArrayData* ad,
                                                 int64_t ki);

const TypedValue* MixedArray::NvGetIntConverted(const ArrayData* ad,
                                                int64_t ki) {
  MixedArray::warnUsage(MixedArray::Reason::kNumericString, kIntMapKind);
  return NvGetInt(ad, ki);
}

const TypedValue* MixedArray::NvGetStr(const ArrayData* ad,
                                       const StringData* k) {
  return NvGetStrImpl<kMixedKind>(ad, k);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
const TypedValue* MixedArray::NvGetStrImpl(const ArrayData* ad,
                                           const StringData* k) {
  if (aKind != kMixedKind) {
    MixedArray::warnUsage(Reason::kNvGetStr, aKind);
  }
  auto a = asMixed(ad);
  auto i = a->find(k, k->hash());
  if (LIKELY(validPos(i))) {
    return &a->data()[i].data;
  }
  return nullptr;
}

template const TypedValue*
MixedArray::NvGetStrImpl<ArrayData::kIntMapKind>(const ArrayData* ad,
                                                 const StringData* k);

void MixedArray::NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
  auto a = asMixed(ad);
  assert(pos != a->m_used);
  assert(!isTombstone(a->data()[pos].data.m_type));
  getElmKey(a->data()[pos], out);
}

ArrayData* MixedArray::Append(ArrayData* ad, const Variant& v, bool copy) {
  return AppendImpl<kMixedKind>(ad, v, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
ArrayData* MixedArray::AppendImpl(ArrayData* ad, const Variant& v, bool copy) {
  auto a = asMixed(ad);
  if (UNLIKELY(a->m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return a;
  }
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kAppend);
  }
  a->nextInsert(v);
  return a;
}

template ArrayData*
MixedArray::AppendImpl<ArrayData::kIntMapKind>(ArrayData* ad, const Variant& v,
                                               bool copy);
template ArrayData*
MixedArray::AppendImpl<ArrayData::kStrMapKind>(ArrayData* ad, const Variant& v,
                                               bool copy);

ArrayData* MixedArray::AppendRef(ArrayData* ad, Variant& v, bool copy) {
  return AppendRefImpl<kMixedKind>(ad, v, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
ArrayData* MixedArray::AppendRefImpl(ArrayData* ad, Variant& v, bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kAppendRef);
  }

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

template ArrayData*
MixedArray::AppendRefImpl<ArrayData::kIntMapKind>(ArrayData*, Variant&, bool);
template ArrayData*
MixedArray::AppendRefImpl<ArrayData::kStrMapKind>(ArrayData*, Variant&, bool);

ArrayData* MixedArray::AppendWithRef(ArrayData* ad, const Variant& v,
                                     bool copy) {
  return AppendWithRefImpl<kMixedKind>(ad, v, copy);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
ArrayData* MixedArray::AppendWithRefImpl(ArrayData* ad, const Variant& v,
                                         bool copy) {
  auto a = asMixed(ad);
  a = copy ? a->copyMixedAndResizeIfNeeded()
           : a->resizeIfNeeded();
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kAppendRef);
  }
  return a->nextInsertWithRef(v);
}

template ArrayData*
MixedArray::AppendWithRefImpl<ArrayData::kIntMapKind>(ArrayData*,
                                                      const Variant&, bool);
template ArrayData*
MixedArray::AppendWithRefImpl<ArrayData::kStrMapKind>(ArrayData*,
                                                      const Variant&, bool);

/*
 * Copy an array to a new array of mixed kind, with a particular
 * pre-reserved size.  The input array may be either packed or mixed.
 */
NEVER_INLINE
MixedArray* MixedArray::CopyReserve(const MixedArray* src,
                                    size_t expectedSize) {
  assert(!src->isPacked());
  auto const cmret = computeCapAndMask(expectedSize);
  auto const cap   = cmret.first;
  auto const mask  = cmret.second;
  auto const ad    = smartAllocArray(cap, mask);

  auto const oldSize        = src->m_size;
  auto const oldPosUnsigned = static_cast<uint32_t>(src->m_pos);
  auto const oldNextKI      = src->m_nextKI;
  auto const oldUsed        = src->m_used;
  auto const oldKind        = src->m_kind;

  ad->m_kindAndSize     = uint64_t{oldSize} << 32 | oldKind << 24;
  ad->m_posAndCount     = uint64_t{1} << 32 | oldPosUnsigned;
  ad->m_cap             = cap;
  ad->m_tableMask       = mask;
  ad->m_nextKI          = oldNextKI;

  auto const data  = ad->data();
  auto const table = reinterpret_cast<int32_t*>(data + cap);
  ad->initHash(table, mask + 1);

  auto dstElm = data;
  auto srcElm = src->data();
  auto const srcStop = src->data() + oldUsed;
  uint32_t i = 0;

  // We're not copying the tombstones over to the new array, so the
  // positions of the elements in the new array may be shifted. Cache
  // the key for element associated with src->m_pos so that we can
  // properly initialize ad->m_pos below.
  ElmKey mPos;
  if (src->m_pos != src->m_used) {
    assert(size_t(src->m_pos) < src->m_used);
    auto& e = srcElm[src->m_pos];
    mPos.hash = e.hasIntKey() ? 0 : e.hash();
    mPos.skey = e.skey;
  } else {
    // Silence compiler warnings.
    mPos.hash = 0;
    mPos.skey = nullptr;
  }

  // Copy the elements
  for (; srcElm != srcStop; ++srcElm) {
    if (isTombstone(srcElm->data.m_type)) continue;
    tvDupFlattenVars(&srcElm->data, &dstElm->data, src);
    auto const hasIntKey = srcElm->hasIntKey();
    auto const hash = hasIntKey ? srcElm->ikey : srcElm->hash();
    if (hasIntKey) {
      dstElm->setIntKey(srcElm->ikey);
    } else {
      dstElm->setStrKey(srcElm->skey, hash);
    }
    *ad->findForNewInsert(table, mask, hash) = i;
    ++dstElm;
    ++i;
  }

  // Now that we have finished copying the elements, update ad->m_pos
  if (src->m_pos != src->m_used) {
    ad->m_pos = mPos.hash
      ? ssize_t(ad->find(mPos.skey, mPos.hash))
      : ssize_t(ad->find(mPos.ikey));
  } else {
    // If src->m_pos is equal to src's canonical invalid position, then
    // set ad->m_pos to ad's canonical invalid position.
    ad->m_pos = ad->m_size;
  }

  // Set new used value (we've removed any tombstones).
  assert(i == dstElm - data);
  ad->m_used = i;

  assert(ad->m_kind == oldKind);
  assert(ad->m_size == oldSize);
  assert(ad->m_count == 1);
  assert(ad->m_cap == cap);
  assert(ad->m_used <= oldUsed);
  assert(ad->m_used == dstElm - data);
  assert(ad->m_tableMask == mask);
  assert(ad->m_nextKI == oldNextKI);
  assert(ad->checkInvariants());
  return ad;
}

NEVER_INLINE
ArrayData* MixedArray::ArrayPlusEqGeneric(ArrayData* ad,
                                         MixedArray* ret,
                                         const ArrayData* elems,
                                         size_t neededSize) {
  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    const Variant& value = it.secondRef();

    if (UNLIKELY(ret->isFull())) {
      assert(ret == ad);
      ret = CopyReserve(asMixed(ad), neededSize);
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

// Note: the logic relating to how to grow in this function is coupled
// to PackedArray::PlusEq.
ArrayData* MixedArray::PlusEq(ArrayData* ad, const ArrayData* elems) {
  auto const neededSize = ad->size() + elems->size();

  auto ret =
    ad->hasMultipleRefs() ? CopyReserve(asMixed(ad), neededSize) :
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
      auto const ei = ret->findForInsert(srcElem->skey, hash);
      if (validPos(*ei)) continue;
      auto& e = ret->allocElm(ei);
      e.setStrKey(srcElem->skey, hash);
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
ArrayData* MixedArray::ArrayMergeGeneric(MixedArray* ret,
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

ArrayData* MixedArray::Merge(ArrayData* ad, const ArrayData* elems) {
  auto const ret = CopyReserve(asMixed(ad), ad->size() + elems->size());

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
        ret->addLvalImpl(srcElem->skey, p);
        p->setWithRef(tvAsCVarRef(&srcElem->data));
      }
    }
    return ret;
  }

  if (UNLIKELY(!elems->isPacked())) {
    return ArrayMergeGeneric(ret, elems);
  }

  assert(PackedArray::checkInvariants(elems));
  auto src           = packedData(elems);
  auto const srcStop = src + elems->m_size;
  for (; src != srcStop; ++src) {
    ret->nextInsertWithRef(tvAsCVarRef(src));
  }
  return ret;

  // Note: currently caller is responsible for calling renumber after
  // this.  Should refactor so we handle it (we already know things
  // about the array).
}

ArrayData* MixedArray::Pop(ArrayData* ad, Variant& value) {
  return PopImpl<kMixedKind>(ad, value);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
ArrayData* MixedArray::PopImpl(ArrayData* ad, Variant& value) {
  auto a = asMixed(ad);
  if (a->hasMultipleRefs()) a = a->copyMixed();
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kPop);
  }
  auto elms = a->data();
  if (a->m_size) {
    ssize_t pos = IterLast(a);
    assert(pos >= 0 && pos < a->m_used);
    auto& e = elms[pos];
    assert(!isTombstone(e.data.m_type));
    value = tvAsCVarRef(&e.data);
    auto pos2 = e.hasStrKey() ? a->findForRemove(e.skey, e.hash()) :
                a->findForRemove(e.ikey, true);
    assert(pos2 == pos);
    a->erase(pos2);
  } else {
    value = uninit_null();
  }
  // To conform to PHP5 behavior, the pop operation resets the array's
  // internal iterator.
  a->m_pos = a->nextElm(elms, -1);
  return a;
}

template
ArrayData* MixedArray::PopImpl<ArrayData::kIntMapKind>(ArrayData*, Variant&);
template
ArrayData* MixedArray::PopImpl<ArrayData::kStrMapKind>(ArrayData*, Variant&);

ArrayData* MixedArray::Dequeue(ArrayData* adInput, Variant& value) {
  return DequeueImpl<kMixedKind>(adInput, value);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
ArrayData* MixedArray::DequeueImpl(ArrayData* adInput, Variant& value) {
  auto a = asMixed(adInput);
  if (a->hasMultipleRefs()) a = a->copyMixed();
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(a, Reason::kDequeue);
  }
  auto elms = a->data();
  if (a->m_size) {
    ssize_t pos = a->nextElm(elms, -1);
    assert(pos >= 0 && pos < a->m_used);
    auto& e = elms[pos];
    assert(!isTombstone(e.data.m_type));
    value = tvAsCVarRef(&e.data);
    auto pos2 = e.hasStrKey() ? a->findForRemove(e.skey, e.hash()) :
                a->findForRemove(e.ikey, false);
    assert(pos2 == pos);
    a->erase(pos2);
  } else {
    value = uninit_null();
  }
  // Even if the array is empty, for PHP5 conformity we need call
  // compact() because it has side-effects that are important
  a->compact(true);
  return a;
}

template ArrayData*
MixedArray::DequeueImpl<ArrayData::kIntMapKind>(ArrayData*, Variant&);
template ArrayData*
MixedArray::DequeueImpl<ArrayData::kStrMapKind>(ArrayData*, Variant&);

ArrayData* MixedArray::Prepend(ArrayData* adInput,
                              const Variant& v,
                              bool copy) {
  auto a = asMixed(adInput);
  if (a->hasMultipleRefs()) a = a->copyMixedAndResizeIfNeeded();
  if (UNLIKELY(a->m_kind != kMixedKind)) {
    MixedArray::downgradeAndWarn(a, Reason::kPrepend);
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
  a->initVal(e.data, *v.asCell());

  // Renumber.
  a->compact(true);
  return a;
}

void MixedArray::Renumber(ArrayData* ad) {
  return RenumberImpl<kMixedKind>(ad);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
void MixedArray::RenumberImpl(ArrayData* ad) {
  if (aKind != kMixedKind) {
    MixedArray::warnUsage(Reason::kRenumber, aKind);
  }
  asMixed(ad)->compact(true);
}

template void MixedArray::RenumberImpl<ArrayData::kIntMapKind>(ArrayData*);
template void MixedArray::RenumberImpl<ArrayData::kStrMapKind>(ArrayData*);

void MixedArray::OnSetEvalScalar(ArrayData* ad) {
  auto a = asMixed(ad);
  auto elms = a->data();
  for (uint32_t i = 0, limit = a->m_used; i < limit; ++i) {
    auto& e = elms[i];
    if (!isTombstone(e.data.m_type)) {
      auto key = e.skey;
      if (e.hasStrKey() && !key->isStatic()) {
        e.skey = makeStaticString(key);
        decRefStr(key);
      }
      tvAsVariant(&e.data).setEvalScalar();
    }
  }
}

bool MixedArray::AdvanceMArrayIter(ArrayData* ad, MArrayIter& fp) {
  return AdvanceMArrayIterImpl<kMixedKind>(ad, fp);
}

template <ArrayData::ArrayKind aKind>
ALWAYS_INLINE
bool MixedArray::AdvanceMArrayIterImpl(ArrayData* ad, MArrayIter& fp) {
  if (aKind != kMixedKind) {
    MixedArray::downgradeAndWarn(ad, Reason::kForeachByRef);
  }
  auto a = asMixed(ad);
  Elm* elms = a->data();
  if (fp.getResetFlag()) {
    fp.setResetFlag(false);
    fp.m_pos = a->nextElm(elms, -1);
  } else if (fp.m_pos == a->m_used) {
    return false;
  } else {
    fp.m_pos = a->nextElm(elms, fp.m_pos);
  }
  if (fp.m_pos == a->m_used) {
    return false;
  }
  // To conform to PHP5 behavior, we need to set the internal
  // cursor to point to the next element.
  a->m_pos = a->nextElm(elms, fp.m_pos);
  return true;
}

template bool
MixedArray::AdvanceMArrayIterImpl<ArrayData::kIntMapKind>(ArrayData* ad,
                                                          MArrayIter& fp);
template bool
MixedArray::AdvanceMArrayIterImpl<ArrayData::kStrMapKind>(ArrayData* ad,
                                                          MArrayIter& fp);

//////////////////////////////////////////////////////////////////////

}
