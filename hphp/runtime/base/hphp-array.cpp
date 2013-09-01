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

#define INLINE_VARIANT_HELPER 1

#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/shared-map.h"
#include "hphp/util/hash.h"
#include "hphp/util/lock.h"
#include "hphp/util/alloc.h"
#include "hphp/util/trace.h"
#include "hphp/util/util.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/base/stats.h"

// inline methods of HphpArray
#include "hphp/runtime/base/hphp-array-defs.h"

namespace HPHP {

static_assert(
  sizeof(HphpArray) == 152,
  "Performance is sensitive to sizeof(HphpArray)."
  " Make sure you changed it with good reason and then update this assert.");

TRACE_SET_MOD(runtime);
///////////////////////////////////////////////////////////////////////////////

/*
 * Allocation of HphpArray buffers works like this: the smallest buffer
 * size is allocated inline in HphpArray.  Larger buffer sizes are smart
 * allocated or malloc-allocated depending on whether the array itself
 * was smart-allocated or not.  (nonSmartCopy() is used to create static
 * arrays).  HphpArray::m_allocMode tracks the state as it progresses:
 *
 *   kInline -> kSmart, or
 *           -> kMalloc
 *
 * Hashtables never shrink, so the allocMode Never goes backwards.
 * If an array is pre-sized, we might skip directly to kSmart or kMalloc.
 * If an array is created via nonSmartCopy(), we skip kSmart.
 * Since kMalloc is only used for static arrays, and static arrays are
 * never swept, we don't need any sweep method.
 *
 * For kInline, we use space in HphpArray defined as InlineSlots, which
 * has enough room for slots and the hashtable.  The next few larger array
 * sizes use the inline space for just the hashtable, with slots allocated
 * separately.  Even larger tables allocate the hashtable and slots
 * contiguously.
 */
void *HphpArray::SmaAllocatorInitSetup = SmartAllocatorInitSetup<HphpArray>();

//=============================================================================
// Static members.

HphpArray HphpArray::s_theEmptyArray(StaticEmptyArray);

//=============================================================================
// Helpers.

static inline uint32_t computeMaskFromNumElms(const uint32_t n) {
  assert(n <= 0x7fffffffU);
  auto lgSize = HphpArray::MinLgTableSize;
  auto maxElms = HphpArray::SmallSize;
  assert(lgSize >= 2);
  while (maxElms < n) {
    ++lgSize;
    maxElms <<= 1;
  }
  assert(lgSize <= 32);
  // return 2^lgSize - 1
  return ((size_t(1U)) << lgSize) - 1;
  static_assert(HphpArray::MinLgTableSize >= 2,
                "lower limit for 0.75 load factor");
}

//=============================================================================
// Construction/destruction.

HphpArray::HphpArray(uint capacity)
    : ArrayData(kPackedKind, AllocationMode::smart, 0)
    , m_used(0) {
  assert(m_size == 0);
  const auto mask = computeMaskFromNumElms(capacity);
  m_tableMask = mask;
  allocData(computeMaxElms(mask), computeTableSize(mask));
  assert(checkInvariants());
}

HphpArray::HphpArray(uint size, const TypedValue* values)
    : ArrayData(kPackedKind, AllocationMode::smart, size)
    , m_used(size) {
  const auto mask = computeMaskFromNumElms(size);
  m_tableMask = mask;
  allocData(computeMaxElms(mask), computeTableSize(mask));
  // append values by moving -- Caller assumes we update refcount.  Values
  // are in reverse order since they come from the stack, which grows down.
  Elm* data = m_data;
  for (uint32_t i = 0; i < size; i++) {
    const auto& tv = values[size - i - 1];
    data[i].data.m_data = tv.m_data;
    data[i].data.m_type = tv.m_type;
  }
  assert(size == 0 || m_pos == 0);
  assert(checkInvariants());
}

HphpArray::HphpArray(EmptyMode)
    : ArrayData(kPackedKind, AllocationMode::smart, 0)
    , m_used(0)
    , m_tableMask(SmallHashSize - 1) {
  allocData(SmallSize, SmallHashSize);
  setStatic();
  assert(checkInvariants());
}

// for internal use by nonSmartCopy() and copyPacked()
inline ALWAYS_INLINE
HphpArray::HphpArray(const HphpArray& other, AllocationMode mode, ClonePacked)
    : ArrayData(other.m_kind, mode, other.m_size)
    , m_used(other.m_used)
    , m_tableMask(other.m_tableMask) {
  assert(other.isPacked());
  m_pos = other.m_pos;
  allocData(other.m_cap, computeTableSize(m_tableMask));
  // Copy the elements and bump up refcounts as needed.
  Elm* elms = other.m_data;
  Elm* targetElms = m_data;
  for (uint32_t i = 0, limit = m_used; i < limit; ++i) {
    tvDupFlattenVars(&elms[i].data, &targetElms[i].data, &other);
  }
  assert(checkInvariants());
}

// For internal use by nonSmartCopy() and copyMixed()
inline ALWAYS_INLINE
HphpArray::HphpArray(const HphpArray& other, AllocationMode mode, CloneMixed)
    : ArrayData(other.m_kind, mode, other.m_size)
    , m_used(other.m_used)
    , m_tableMask(other.m_tableMask)
    , m_hLoad(other.m_hLoad)
    , m_nextKI(other.m_nextKI) {
  assert(!other.isPacked());
  m_pos = other.m_pos;
  auto maxElms = other.m_cap;
  auto tableSize = computeTableSize(m_tableMask);
  m_hash = allocData(maxElms, tableSize);
  // Copy the hash.
  memcpy(m_hash, other.m_hash, tableSize * sizeof(ElmInd));
  // Copy the elements and bump up refcounts as needed.
  auto elms = other.m_data;
  auto targetElms = m_data;
  for (uint32_t i = 0, limit = m_used; i < limit; ++i) {
    const auto e = &elms[i];
    auto te = &targetElms[i];
    if (!isTombstone(e->data.m_type)) {
      te->key = e->key;
      te->data.hash() = e->data.hash();
      if (te->hasStrKey()) te->key->incRefCount();
      tvDupFlattenVars(&e->data, &te->data, &other);
      assert(te->hash() == e->hash()); // ensure not clobbered.
    } else {
      // Tombstone.
      te->data.m_type = KindOfInvalid;
    }
  }
  // If the element density dropped below 50% due to indirect elements
  // being converted into tombstones, we should do a compaction
  if (m_size < m_used / 2) {
    compact(false);
  }
  assert(checkInvariants());
}

inline void HphpArray::destroyPacked() {
  auto const elms = m_data;
  for (uint32_t i = 0, n = m_used; i < n; ++i) {
    tvRefcountedDecRef(&elms[i].data);
  }
  if (elms != m_inline_data.slots) modeFree(elms);
}

inline void HphpArray::destroy() {
  auto const elms = m_data;
  for (uint32_t i = 0, n = m_used; i < n; ++i) {
    auto& e = elms[i];
    if (isTombstone(e.data.m_type)) continue;
    if (e.hasStrKey()) decRefStr(e.key);
    tvRefcountedDecRef(&e.data);
  }
  if (elms != m_inline_data.slots) modeFree(elms);
}

HphpArray::~HphpArray() {
  assert(checkInvariants());
  if (isPacked()) destroyPacked();
  else destroy();
}

HOT_FUNC_VM
void HphpArray::ReleasePacked(ArrayData* ad) {
  auto a = asPacked(ad);
  a->destroyPacked();
  a->ArrayData::destroy();
  HphpArray::AllocatorType::getNoCheck()->dealloc(a);
}

HOT_FUNC_VM
void HphpArray::Release(ArrayData* ad) {
  auto a = asMixed(ad);
  a->destroy();
  a->ArrayData::destroy();
  HphpArray::AllocatorType::getNoCheck()->dealloc(a);
}

NEVER_INLINE HOT_FUNC_VM
HphpArray* HphpArray::packedToMixed() {
  assert(isPacked());
  if (m_data == m_inline_data.slots) {
    m_hash = m_inline_data.hash;
  } else {
    auto dataSize = m_cap * sizeof(*m_data);
    auto hashSize = computeTableSize(m_tableMask) * sizeof(*m_hash);
    m_hash = hashSize <= sizeof(m_inline_hash) ? m_inline_hash :
             (ElmInd*)(uintptr_t(m_data) + dataSize);
  }
  m_kind = kMixedKind;
  uint32_t i = 0;
  auto size = m_size;
  for (; i < size; ++i) {
    m_data[i].setIntKey(i);
    m_hash[i] = i;
  }
  for (; i <= m_tableMask; ++i) {
    m_hash[i] = ElmIndEmpty;
  }
  m_hLoad = size;
  m_nextKI = size;
  assert(checkInvariants());
  return this;
}

// Invariants:
//
// m_size <= m_used; m_used <= m_cap
// last element cannot be a tombstone
// m_pos and all external iterators can't be on a tombstone
// m_tableMask is 2^k - 1 (required for quadratic probe)
// m_tableMask == nextPower2(m_cap) - 1;
// m_cap == computeMaxElms(m_tableMask);
//
// kMixedKind:
//   m_nextKI >= highest actual int key
//   Elm.data.m_type maybe KindOfInvalid (tombstone)
//   hash[] maybe ElmIndTombstone
//   m_hLoad >= m_size, == number of non-ElmIndEmpty hash entries
//
// kPackedKind:
//   m_size == m_used
//   m_nextKI = uninitialized
//   m_hLoad = uninitialized
//   m_hash = uninitialized
//   Elm.key uninitialized
//   Elm.hash uninitialized
//   no KindOfInvalid tombstones
//
bool HphpArray::checkInvariants() const {
  static_assert(sizeof(Elm) == 24, "");

  assert(m_size <= m_used);
  assert(m_used <= m_cap);
  assert(m_tableMask > 0 && ((m_tableMask+1) & m_tableMask) == 0);
  assert(m_tableMask == Util::nextPower2(m_cap) - 1);
  assert(m_cap == computeMaxElms(m_tableMask));
  if (m_pos != invalid_index) {
    assert(size_t(m_pos) < m_used);
    assert(!isTombstone(m_data[m_pos].data.m_type));
  }
  if (m_used > 0) {
    // can't have a tombstone at the end; m_used should have been trimmed.
    assert(!isTombstone(m_data[m_used - 1].data.m_type));
  }
  switch (m_kind) {
  case kPackedKind:
    assert(m_size == m_used);
    break;
  case kMixedKind: {
    assert(m_hash);
    assert(m_hLoad >= m_size);
    size_t load = 0;
    return true;
    // The following loop is for debugging arrays only; it slows
    // things down too much for general use
    for (size_t i = 0; i <= m_tableMask; i++) {
      load += m_hash[i] != ElmIndEmpty;
    }
    assert(m_hLoad == load);
    break;
  }
  default:
    assert(false);
    break;
  }
  if (this == &s_theEmptyArray) {
    assert(m_size == 0);
    assert(m_used == 0);
    assert(isPacked());
    assert(m_pos == invalid_index);
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
  return (ssize_t)ElmIndEmpty;
}

ssize_t HphpArray::IterBegin(const ArrayData* ad) {
  auto a = asHphpArray(ad);
  return a->nextElm(a->m_data, ElmIndEmpty);
}

ssize_t HphpArray::IterEnd(const ArrayData* ad) {
  auto a = asHphpArray(ad);
  return a->prevElm(a->m_data, a->m_used);
}

ssize_t HphpArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  auto a = asHphpArray(ad);
  // Since m_used is always less than 2^32 and invalid_index == -1,
  // we can save a check by doing an unsigned comparison instead
  // of a signed comparison.
  if (size_t(++pos) < a->m_used && !isTombstone(a->m_data[pos].data.m_type)) {
    return pos;
  }
  return a->iter_advance_helper(pos);
  static_assert(invalid_index == -1, "");
}

// caller has already incremented pos but encountered a tombstone
ssize_t HphpArray::iter_advance_helper(ssize_t next_pos) const {
  Elm* elms = m_data;
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
  return a->prevElm(a->m_data, pos);
}

CVarRef HphpArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  auto a = asHphpArray(ad);
  assert(a->checkInvariants());
  assert(pos != ArrayData::invalid_index);
  Elm* e = &a->m_data[pos];
  assert(!isTombstone(e->data.m_type));
  return tvAsCVarRef(&e->data);
}

bool HphpArray::IsVectorDataPacked(const ArrayData*) {
  return true;
}

bool HphpArray::IsVectorData(const ArrayData* ad) {
  auto a = asMixed(ad);
  if (a->m_size == 0) {
    // any 0-length array is "vector-like" for the sake of this
    // function, even if m_kind != kVector.
    return true;
  }
  auto const elms = a->m_data;
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

#define STRING_HASH(x)   (int32_t(x) | 0x80000000)

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

template <class Hit> inline ALWAYS_INLINE
HphpArray::ElmInd HphpArray::findBody(size_t h0, Hit hit) const {
  // tableMask, probeIndex, and pos are explicitly 64-bit, because performance
  // regressed when they were 32-bit types via auto.  Test carefully.
  size_t tableMask = m_tableMask;
  size_t probeIndex = h0 & tableMask;
  auto* elms = m_data;
  auto* hashtable = m_hash;
  ssize_t pos = hashtable[probeIndex];
  if ((validElmInd(pos) && hit(elms[pos])) || pos == ElmIndEmpty) {
    return pos;
  }
  for (size_t i = 1;; ++i) {
    assert(i <= tableMask);
    probeIndex = (probeIndex + i) & tableMask;
    assert(probeIndex == ((h0 + (i + i*i) / 2) & tableMask));
    pos = hashtable[probeIndex];
    if ((validElmInd(pos) && hit(elms[pos])) || pos == ElmIndEmpty) {
      return pos;
    }
  }
}

NEVER_INLINE
ssize_t HphpArray::find(int64_t ki) const {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  if (size_t(ki) < m_used) {
    // Try to get at it without dirtying a data cache line.
    auto& e = m_data[ki];
    if (!isTombstone(e.data.m_type) && hitIntKey(e, ki)) {
      Stats::inc(Stats::HA_FindIntFast);
      // Our results had better match the other path
      assert(ki == findBody(ki, [ki] (const Elm& e) {
        return hitIntKey(e, ki);
      }));
      return ki;
    }
  }
  Stats::inc(Stats::HA_FindIntSlow);
  return findBody(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

NEVER_INLINE
ssize_t HphpArray::find(const StringData* s, strhash_t prehash) const {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  int32_t h = STRING_HASH(prehash);
  return findBody(prehash, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

NEVER_INLINE
HphpArray::ElmInd* warnUnbalanced(size_t n, HphpArray::ElmInd* ei) {
  raise_error("Array is too unbalanced (%lu)", n);
  return ei;
}

template <class Hit> inline ALWAYS_INLINE
HphpArray::ElmInd* HphpArray::findForInsertBody(size_t h0, Hit hit) const {
  // tableMask, probeIndex, and pos are explicitly 64-bit, because performance
  // regressed when they were 32-bit types via auto.  Test carefully.
  assert(m_hLoad <= computeMaxElms(m_tableMask));
  size_t tableMask = m_tableMask;
  auto* elms = m_data;
  auto* hashtable = m_hash;
  ElmInd* ret = nullptr;
  size_t probeIndex = h0 & tableMask;
  auto* ei = &hashtable[probeIndex];
  ssize_t pos = *ei;
  if ((validElmInd(pos) && hit(elms[pos])) || pos == ElmIndEmpty) {
    return ei;
  }
  if (!validElmInd(pos)) ret = ei;
  for (size_t i = 1;; ++i) {
    assert(i <= tableMask);
    probeIndex = (probeIndex + i) & tableMask;
    assert(probeIndex == ((h0 + (i + i*i) / 2) & tableMask));
    ei = &hashtable[probeIndex];
    pos = *ei;
    if (validElmInd(pos)) {
      if (hit(elms[pos])) {
        return ei;
      }
    } else {
      if (!ret) ret = ei;
      if (pos == ElmIndEmpty) {
        return LIKELY(i <= 100) ||
          LIKELY(i <= size_t(RuntimeOption::MaxArrayChain)) ?
          ret : warnUnbalanced(i, ret);
      }
    }
  }
}

NEVER_INLINE
HphpArray::ElmInd* HphpArray::findForInsert(int64_t ki) const {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  return findForInsertBody(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

NEVER_INLINE
HphpArray::ElmInd* HphpArray::findForInsert(const StringData* s,
                                            strhash_t prehash) const {
  // all vector methods should work w/out touching the hashtable
  assert(!isPacked());
  int32_t h = STRING_HASH(prehash);
  return findForInsertBody(prehash, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

NEVER_INLINE HphpArray::ElmInd*
HphpArray::findForNewInsertLoop(size_t tableMask, size_t h0) const {
  /* Quadratic probe. */
  size_t probeIndex = h0 & tableMask;
  for (size_t i = 1;; ++i) {
    assert(i <= tableMask);
    probeIndex = (probeIndex + i) & tableMask;
    assert(((h0 + ((i + i * i) >> 1)) & tableMask) == probeIndex);
    ElmInd* ei = &m_hash[probeIndex];
    ssize_t pos = ssize_t(*ei);
    if (!validElmInd(pos)) {
      return ei;
    }
  }
}

bool HphpArray::ExistsIntPacked(const ArrayData* ad, int64_t k) {
  auto a = asPacked(ad);
  return size_t(k) < a->m_size;
}

bool HphpArray::ExistsInt(const ArrayData* ad, int64_t k) {
  auto a = asMixed(ad);
  return a->find(k) != ElmIndEmpty;
}

bool HphpArray::ExistsStrPacked(const ArrayData* ad, const StringData* k) {
  assert(asPacked(ad));
  return false;
}

bool HphpArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  auto a = asMixed(ad);
  return a->find(k, k->hash()) != ElmIndEmpty;
}

//=============================================================================
// Append/insert/update.

inline ALWAYS_INLINE bool HphpArray::isFull() const {
  assert(!isPacked());
  assert(m_used <= m_cap);
  assert(m_hLoad <= m_cap);
  return m_used == m_cap || m_hLoad == m_cap;
}

inline ALWAYS_INLINE HphpArray::Elm* HphpArray::allocElm(ElmInd* ei) {
  assert(!validElmInd(*ei) && !isFull());
  assert(m_size != 0 || m_used == 0);
  ++m_size;
  m_hLoad += (*ei == ElmIndEmpty);
  size_t i = m_used;
  (*ei) = i;
  m_used = i + 1;
  if (m_pos == invalid_index) m_pos = i;
  return &m_data[i];
}

inline ALWAYS_INLINE TypedValue& HphpArray::allocNextElm(uint32_t i) {
  assert(isPacked() && i == m_size);
  if (i == m_cap) growPacked();
  auto next = i + 1;
  if (m_pos == invalid_index) m_pos = i;
  m_used = m_size = next;
  return m_data[i].data;
}

inline ALWAYS_INLINE
HphpArray::Elm* HphpArray::newElm(ElmInd* ei, size_t h0) {
  if (isFull()) return newElmGrow(h0);
  return allocElm(ei);
}

NEVER_INLINE
HphpArray::Elm* HphpArray::newElmGrow(size_t h0) {
  resize();
  return allocElm(findForNewInsert(h0));
}

inline ALWAYS_INLINE
HphpArray* HphpArray::initVal(TypedValue& tv, CVarRef v) {
  tvAsUninitializedVariant(&tv).constructValHelper(v);
  return this;
}

inline ALWAYS_INLINE
HphpArray* HphpArray::initRef(TypedValue& tv, CVarRef v) {
  tvAsUninitializedVariant(&tv).constructRefHelper(v);
  return this;
}

inline ALWAYS_INLINE
HphpArray* HphpArray::getLval(TypedValue& tv, Variant*& ret) {
  ret = &tvAsVariant(&tv);
  return this;
}

inline ALWAYS_INLINE
HphpArray* HphpArray::initLval(TypedValue& tv, Variant*& ret) {
  tvWriteNull(&tv);
  ret = &tvAsVariant(&tv);
  return this;
}

inline ALWAYS_INLINE
HphpArray* HphpArray::initWithRef(TypedValue& tv, CVarRef v) {
  tvWriteNull(&tv);
  tvAsVariant(&tv).setWithRef(v);
  return this;
}

inline ALWAYS_INLINE
HphpArray* HphpArray::setVal(TypedValue& tv, CVarRef v) {
  tvAsVariant(&tv).assignValHelper(v);
  return this;
}

inline ALWAYS_INLINE
HphpArray* HphpArray::setRef(TypedValue& tv, CVarRef v) {
  tvAsVariant(&tv).assignRefHelper(v);
  return this;
}

/*
 * This is a streamlined copy of Variant.constructValHelper()
 * with no incref+decref because we're moving v to this array.
 */
inline ALWAYS_INLINE
HphpArray* HphpArray::moveVal(TypedValue& tv, TypedValue v) {
  tv.m_type = typeInitNull(v.m_type);
  tv.m_data.num = v.m_data.num;
  return this;
}

HphpArray::ElmInd* HphpArray::allocData(size_t maxElms, size_t tableSize) {
  m_cap = maxElms;
  if (maxElms <= SmallSize) {
    m_data = m_inline_data.slots;
    return m_inline_data.hash;
  }
  size_t hashSize = tableSize * sizeof(ElmInd);
  size_t dataSize = maxElms * sizeof(Elm);
  size_t allocSize = hashSize <= sizeof(m_inline_hash) ? dataSize :
                     dataSize + hashSize;
  m_data = (Elm*) modeAlloc(allocSize);
  return hashSize <= sizeof(m_inline_hash) ? m_inline_hash :
         (ElmInd*)(uintptr_t(m_data) + dataSize);
}

HphpArray::ElmInd* HphpArray::reallocData(size_t maxElms, size_t tableSize) {
  assert(m_data && m_cap > 0 && maxElms > SmallSize);
  size_t hashSize = tableSize * sizeof(ElmInd);
  size_t dataSize = maxElms * sizeof(Elm);
  size_t allocSize = hashSize <= sizeof(m_inline_hash) ? dataSize :
                     dataSize + hashSize;
  size_t oldDataSize = m_cap * sizeof(Elm); // slots only.
  if (m_data == m_inline_data.slots) {
    m_data = (Elm*) modeAlloc(allocSize);
    memcpy(m_data, m_inline_data.slots, oldDataSize);
  } else {
    m_data = (Elm*) modeRealloc(m_data, allocSize);
  }
  m_cap = maxElms;
  return hashSize <= sizeof(m_inline_hash) ? m_inline_hash :
         (ElmInd*)(uintptr_t(m_data) + dataSize);
}

inline ALWAYS_INLINE void HphpArray::resizeIfNeeded() {
  if (isFull()) resize();
}

NEVER_INLINE void HphpArray::resize() {
  uint32_t maxElms = computeMaxElms(m_tableMask);
  assert(m_used <= maxElms);
  assert(m_hLoad <= maxElms);
  // At a minimum, compaction is required.  If the load factor would be >0.5
  // even after compaction, grow instead, in order to avoid the possibility
  // of repeated compaction if the load factor were to hover at nearly 0.75.
  if (m_size > maxElms / 2) {
    grow();
  } else {
    compact(false);
  }
}

void HphpArray::grow() {
  assert(!isPacked());
  assert(m_tableMask <= 0x7fffffffU);
  m_tableMask = 2 * m_tableMask + 1;
  auto tableSize = computeTableSize(m_tableMask);
  auto maxElms = computeMaxElms(m_tableMask);
  m_hash = reallocData(maxElms, tableSize);
  // All the elements have been copied and their offsets from the base are
  // still the same, so we just need to build the new hash table.
  initHash(tableSize);
  Elm* elms = m_data;
  for (uint32_t i = 0, limit = m_used; i < limit; ++i) {
    auto& e = elms[i];
    if (isTombstone(e.data.m_type)) continue;
    auto* ei = findForNewInsert(e.hasIntKey() ? e.ikey : e.hash());
    *ei = i;
  }
  m_hLoad = m_size;
}

NEVER_INLINE
void HphpArray::growPacked() {
  assert(isPacked());
  auto maxElms = m_cap * 2;
  auto mask = m_tableMask * 2 + 1;
  m_tableMask = mask;
  reallocData(maxElms, computeTableSize(mask));
}

void HphpArray::compact(bool renumber /* = false */) {
  struct ElmKey {
    ElmKey() {}
    ElmKey(int32_t hash, StringData* key) {
      this->hash = hash;
      this->key = key;
    }
    int32_t hash;
    union {
      StringData* key;
      int64_t ikey;
    };
  };

  assert(!isPacked());
  ElmKey mPos;
  if (m_pos != ArrayData::invalid_index) {
    // Cache key for element associated with m_pos in order to update m_pos
    // below.
    assert(size_t(m_pos) < m_used);
    Elm* e = &(m_data[(ElmInd)m_pos]);
    mPos.hash = e->hasIntKey() ? 0 : e->hash();
    mPos.key = e->key;
  } else {
    // Silence compiler warnings.
    mPos.hash = 0;
    mPos.key = nullptr;
  }
  TinyVector<ElmKey, 3> siKeys;
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    ElmInd ei = r.front()->m_pos;
    if (ei != ElmIndEmpty) {
      Elm* e = &m_data[ei];
      siKeys.push_back(ElmKey(e->hash(), e->key));
    }
  }
  if (renumber) {
    m_nextKI = 0;
  }
  Elm* elms = m_data;
  size_t tableSize = computeTableSize(m_tableMask);
  initHash(tableSize);
  for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
    while (isTombstone(elms[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    Elm& toE = elms[toPos];
    if (toPos != frPos) {
      toE = elms[frPos];
    }
    if (renumber && !toE.hasStrKey()) {
      toE.ikey = m_nextKI++;
    }
    ElmInd* ie = findForNewInsert(toE.hasIntKey() ? toE.ikey : toE.hash());
    *ie = toPos;
  }
  m_used = m_size;
  m_hLoad = m_size;
  if (m_pos != ArrayData::invalid_index) {
    // Update m_pos, now that compaction is complete.
    if (mPos.hash) {
      m_pos = ssize_t(find(mPos.key, mPos.hash));
    } else {
      m_pos = ssize_t(find(mPos.ikey));
    }
  }
  // Update strong iterators, now that compaction is complete.
  int key = 0;
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    FullPos* fp = r.front();
    if (fp->m_pos != ArrayData::invalid_index) {
      ElmKey &k = siKeys[key];
      key++;
      if (k.hash) { // string key
        fp->m_pos = ssize_t(find(k.key, k.hash));
      } else { // int key
        fp->m_pos = ssize_t(find(k.ikey));
      }
    }
  }
}

bool HphpArray::nextInsert(CVarRef data) {
  if (UNLIKELY(m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return false;
  }
  resizeIfNeeded();
  int64_t ki = m_nextKI;
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  ElmInd* ei = findForNewInsert(ki);
  assert(!validElmInd(*ei));
  // Allocate and initialize a new element.
  auto* e = allocElm(ei);
  e->setIntKey(ki);
  m_nextKI = ki + 1; // Update next free element.
  initVal(e->data, data);
  return true;
}

ArrayData* HphpArray::nextInsertRef(CVarRef data) {
  if (UNLIKELY(m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return this;
  }
  resizeIfNeeded();
  int64_t ki = m_nextKI;
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  ElmInd* ei = findForNewInsert(ki);
  auto e = allocElm(ei);
  e->setIntKey(ki);
  m_nextKI = ki + 1; // Update next free element.
  return initRef(e->data, data);
}

ArrayData* HphpArray::nextInsertWithRef(CVarRef data) {
  resizeIfNeeded();
  int64_t ki = m_nextKI;
  ElmInd* ei = findForInsert(ki);
  assert(!validElmInd(*ei));

  // Allocate a new element.
  Elm* e = allocElm(ei);
  e->setIntKey(ki);
  m_nextKI = ki + 1; // Update next free element.
  return initWithRef(e->data, data);
}

ArrayData* HphpArray::addLvalImpl(int64_t ki, Variant*& ret) {
  assert(!isPacked());
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    return getLval(m_data[*ei].data, ret);
  }
  Elm* e = newElm(ei, ki);
  e->setIntKey(ki);
  if (ki >= m_nextKI && m_nextKI >= 0) m_nextKI = ki + 1;
  return initLval(e->data, ret);
}

ArrayData* HphpArray::addLvalImpl(StringData* key, strhash_t h,
                                  Variant*& ret) {
  assert(key && !isPacked());
  ElmInd* ei = findForInsert(key, h);
  if (validElmInd(*ei)) {
    return getLval(m_data[*ei].data, ret);
  }
  Elm* e = newElm(ei, h);
  e->setStrKey(key, h);
  return initLval(e->data, ret);
}

inline ArrayData* HphpArray::addVal(int64_t ki, CVarRef data) {
  assert(!isPacked());
  resizeIfNeeded();
  ElmInd* ei = findForNewInsert(ki);
  Elm* e = allocElm(ei);
  e->setIntKey(ki);
  if (ki >= m_nextKI && m_nextKI >= 0) m_nextKI = ki + 1;
  return initVal(e->data, data);
}

inline ArrayData* HphpArray::addVal(StringData* key, CVarRef data) {
  assert(!exists(key) && !isPacked());
  resizeIfNeeded();
  strhash_t h = key->hash();
  ElmInd* ei = findForNewInsert(h);
  Elm *e = allocElm(ei);
  e->setStrKey(key, h);
  return initVal(e->data, data);
}

inline ArrayData* HphpArray::addValWithRef(int64_t ki, CVarRef data) {
  resizeIfNeeded();
  ElmInd* ei = findForInsert(ki);
  if (!validElmInd(*ei)) {
    Elm* e = allocElm(ei);
    e->setIntKey(ki);
    if (ki >= m_nextKI) m_nextKI = ki + 1;
    initWithRef(e->data, data);
  }
  return this;
}

inline ArrayData* HphpArray::addValWithRef(StringData* key, CVarRef data) {
  resizeIfNeeded();
  strhash_t h = key->hash();
  ElmInd* ei = findForInsert(key, h);
  if (!validElmInd(*ei)) {
    Elm* e = allocElm(ei);
    e->setStrKey(key, h);
    initWithRef(e->data, data);
  }
  return this;
}

inline INLINE_SINGLE_CALLER
ArrayData* HphpArray::update(int64_t ki, CVarRef data) {
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    return setVal(m_data[*ei].data, data);
  }
  auto e = newElm(ei, ki);
  e->setIntKey(ki);
  if (ki >= m_nextKI && m_nextKI >= 0) m_nextKI = ki + 1;
  return initVal(e->data, data);
}

inline INLINE_SINGLE_CALLER
ArrayData* HphpArray::update(StringData* key, CVarRef data) {
  strhash_t h = key->hash();
  ElmInd* ei = findForInsert(key, h);
  if (validElmInd(*ei)) {
    return setVal(m_data[*ei].data, data);
  }
  auto e = newElm(ei, h);
  e->setStrKey(key, h);
  return initVal(e->data, data);
}

ArrayData* HphpArray::updateRef(int64_t ki, CVarRef data) {
  assert(!isPacked());
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    return setRef(m_data[*ei].data, data);
  }
  auto e = newElm(ei, ki);
  e->setIntKey(ki);
  if (ki >= m_nextKI && m_nextKI >= 0) m_nextKI = ki + 1;
  return initRef(e->data, data);
}

ArrayData* HphpArray::updateRef(StringData* key, CVarRef data) {
  assert(!isPacked());
  strhash_t h = key->hash();
  ElmInd* ei = findForInsert(key, h);
  if (validElmInd(*ei)) {
    return setRef(m_data[*ei].data, data);
  }
  auto e = newElm(ei, h);
  e->setStrKey(key, h);
  return initRef(e->data, data);
}

// return true if Elm contains a Reference that won't be flattened
// by a copy, or an object.
static inline bool isContainer(const TypedValue& tv) {
  auto& v = tvAsCVarRef(&tv);
  return v.isReferenced() || v.isObject();
}

ArrayData* HphpArray::LvalIntPacked(ArrayData* ad, int64_t k, Variant*& ret,
                                    bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  if (size_t(k) < a->m_size) {
    return a->getLval(a->m_data[k].data, ret);
  }
  if (size_t(k) == a->m_size) {
    auto& tv = a->allocNextElm(k);
    return a->initLval(tv, ret);
  }
  // todo t2606310: we know key is new.  use add/findForNewInsert
  return a->packedToMixed()->addLvalImpl(k, ret);
}

ArrayData* HphpArray::LvalInt(ArrayData* ad, int64_t k, Variant*& ret,
                              bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  return a->addLvalImpl(k, ret);
}

ArrayData*
HphpArray::LvalStrPacked(ArrayData* ad, StringData* key, Variant*& ret,
                         bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  return a->packedToMixed()->addLvalImpl(key, key->hash(), ret);
}

ArrayData* HphpArray::LvalStr(ArrayData* ad, StringData* key, Variant*& ret,
                              bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  return a->addLvalImpl(key, key->hash(), ret);
}

ArrayData* HphpArray::LvalNewPacked(ArrayData* ad, Variant*& ret, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  auto& tv = a->allocNextElm(a->m_size);
  tvWriteUninit(&tv);
  ret = &tvAsVariant(&tv);
  return a;
}

ArrayData* HphpArray::LvalNew(ArrayData* ad, Variant*& ret, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  if (UNLIKELY(!a->nextInsert(uninit_null()))) {
    ret = &Variant::lvalBlackHole();
    return a;
  }
  ret = &tvAsVariant(&a->m_data[a->m_used - 1].data);
  return a;
}

ArrayData*
HphpArray::SetIntPacked(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  if (size_t(k) < a->m_size) {
    return a->setVal(a->m_data[k].data, v);
  }
  if (size_t(k) == a->m_size) {
    auto& tv = a->allocNextElm(k);
    return a->initVal(tv, v);
  }
  // must escalate, but call addVal() since key doesn't exist.
  return a->packedToMixed()->addVal(k, v);
}

ArrayData* HphpArray::SetInt(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  return a->update(k, v);
}

ArrayData*
HphpArray::SetStrPacked(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  // must escalate, but call addVal() since key doesn't exist.
  return a->packedToMixed()->addVal(k, v);
}

ArrayData*
HphpArray::SetStr(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  return a->update(k, v);
}

ArrayData*
HphpArray::SetRefIntPacked(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  if (size_t(k) < a->m_size) {
    return a->setRef(a->m_data[k].data, v);
  }
  if (size_t(k) == a->m_size) {
    auto& tv = a->allocNextElm(k);
    return a->initRef(tv, v);
  }
  // todo t2606310: key can't exist.  use add/findForNewInsert
  return a->packedToMixed()->updateRef(k, v);
}

ArrayData*
HphpArray::SetRefInt(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  return a->updateRef(k, v);
}

ArrayData*
HphpArray::SetRefStrPacked(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  // todo t2606310: key can't exist.  use add/findForNewInsert
  return a->packedToMixed()->updateRef(k, v);
}

ArrayData*
HphpArray::SetRefStr(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  return a->updateRef(k, v);
}

ArrayData*
HphpArray::AddIntPacked(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  assert(!ad->exists(k));
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  if (size_t(k) == a->m_size) {
    auto& tv = a->allocNextElm(k);
    return a->initVal(tv, v);
  }
  return a->packedToMixed()->addVal(k, v);
}

ArrayData*
HphpArray::AddInt(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  assert(!ad->exists(k));
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  return a->addVal(k, v);
}

ArrayData*
HphpArray::AddStr(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  assert(!ad->exists(k));
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  return a->addVal(k, v);
}

//=============================================================================
// Delete.

NEVER_INLINE
void HphpArray::adjustFullPos(ElmInd pos) {
  ElmInd eIPrev = ElmIndTombstone;
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    FullPos* fp = r.front();
    if (fp->m_pos == ssize_t(pos)) {
      if (eIPrev == ElmIndTombstone) {
        // eIPrev will actually be used, so properly initialize it with the
        // previous element before pos, or ElmIndEmpty if pos is the first
        // element.
        eIPrev = prevElm(m_data, pos);
      }
      if (eIPrev == ElmIndEmpty) {
        fp->setResetFlag(true);
      }
      fp->m_pos = ssize_t(eIPrev);
    }
  }
}

ArrayData* HphpArray::erase(ElmInd* ei, bool updateNext) {
  ElmInd pos = *ei;
  if (!validElmInd(pos)) {
    return this;
  }

  // move strong iterators to the previous element
  if (strongIterators()) adjustFullPos(pos);

  // If the internal pointer points to this element, advance it.
  Elm* elms = m_data;
  if (m_pos == ssize_t(pos)) {
    ElmInd eINext = nextElm(elms, pos);
    m_pos = ssize_t(eINext);
  }

  Elm* e = &elms[pos];
  // Mark the value as a tombstone.
  TypedValue* tv = &e->data;
  DataType oldType = tv->m_type;
  uint64_t oldDatum = tv->m_data.num;
  tv->m_type = KindOfInvalid;
  // Free the key if necessary, and clear the h and key fields in order to
  // increase the chances that subsequent searches will quickly/safely fail
  // when encountering tombstones, even though checking for KindOfInvalid is
  // the last validation step during search.
  if (e->hasStrKey()) {
    decRefStr(e->key);
    e->setIntKey(0);
  } else {
    // Match PHP 5.3.1 semantics
    // Hacky: don't removed the unsigned cast, else g++ can optimize away
    // the check for == 0x7fff..., since there is no signed int k
    // for which k-1 == 0x7fff...
    if ((uint64_t)e->ikey == (uint64_t)m_nextKI-1
          && (e->ikey == 0x7fffffffffffffffLL || updateNext)) {
      --m_nextKI;
    }
  }
  --m_size;
  // If this element was last, adjust m_used.
  if (size_t(pos + 1) == m_used) {
    do {
      --m_used;
    } while (m_used > 0 && isTombstone(elms[m_used - 1].data.m_type));
  }
  // Mark the hash entry as "deleted".
  *ei = ElmIndTombstone;
  assert(m_used <= m_cap);
  assert(m_hLoad <= m_cap);

  // Finally, decref the old value
  tvRefcountedDecRefHelper(oldType, oldDatum);

  if (m_size < m_used / 2) {
    // Compact in order to keep elms from being overly sparse.
    compact(false);
  }
  return this;
}

ArrayData* HphpArray::RemoveIntPacked(ArrayData* ad, int64_t k, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  // todo t2606310: what is probability of (k == size-1)
  if (size_t(k) < a->m_size) {
    a->packedToMixed();
    return a->erase(a->findForInsert(k), false);
  }
  return a; // key didn't exist, so we're still vector
}

ArrayData* HphpArray::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  return a->erase(a->findForInsert(k), false);
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
  return a->erase(a->findForInsert(key, key->hash()), false);
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
  moveStrongIterators(copied, const_cast<HphpArray*>(a));
  return copied;
}

//=============================================================================
// non-variant interface

TypedValue* HphpArray::NvGetIntPacked(const ArrayData* ad, int64_t ki) {
  auto a = asPacked(ad);
  return LIKELY(size_t(ki) < a->m_size) ? &a->m_data[ki].data : nullptr;
}

TypedValue* HphpArray::NvGetInt(const ArrayData* ad, int64_t ki) {
  auto a = asMixed(ad);
  auto i = a->find(ki);
  return LIKELY(i != ElmIndEmpty) ? &a->m_data[i].data : nullptr;
}

TypedValue*
HphpArray::NvGetStrPacked(const ArrayData* ad, const StringData* k) {
  assert(asPacked(ad));
  return nullptr;
}

TypedValue* HphpArray::NvGetStr(const ArrayData* ad, const StringData* k) {
  auto a = asMixed(ad);
  auto i = a->find(k, k->hash());
  if (LIKELY(i != ElmIndEmpty)) {
    return &a->m_data[i].data;
  }
  return nullptr;
}

// nvGetKey does not touch out->_count, so can be used
// for inner or outer cells.
void HphpArray::NvGetKeyPacked(const ArrayData* ad, TypedValue* out,
                               ssize_t pos) {
  DEBUG_ONLY auto a = asPacked(ad);
  assert(pos != ArrayData::invalid_index);
  assert(!isTombstone(a->m_data[pos].data.m_type));
  out->m_data.num = pos;
  out->m_type = KindOfInt64;
}

void HphpArray::NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
  auto a = asMixed(ad);
  assert(pos != ArrayData::invalid_index);
  assert(!isTombstone(a->m_data[pos].data.m_type));
  getElmKey(a->m_data[pos], out);
}

/*
 * Insert a new element with index k in to the array,
 * doing nothing and returning false if the element
 * already exists.
 */
bool HphpArray::nvInsert(StringData *k, TypedValue *data) {
  assert(checkInvariants());
  if (isPacked()) {
    packedToMixed();
    // todo t2606310: we know key doesn't exist.
  }
  strhash_t h = k->hash();
  ElmInd* ei = findForInsert(k, h);
  if (validElmInd(*ei)) {
    return false;
  }
  auto e = newElm(ei, h);
  e->setStrKey(k, h);
  initVal(e->data, tvAsVariant(data));
  return true;
}

HphpArray* HphpArray::nextInsertPacked(CVarRef v) {
  assert(isPacked());
  auto& tv = allocNextElm(m_size);
  return initVal(tv, v);
}

ArrayData* HphpArray::AppendPacked(ArrayData* ad, CVarRef v, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  return a->nextInsertPacked(v);
}

ArrayData* HphpArray::Append(ArrayData* ad, CVarRef v, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
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
      LIKELY(a->getCount() <= 1) &&
      ((k = a->m_size), LIKELY(size_t(k) < a->m_cap))) {
    assert(a->checkInvariants());
    auto& tv = a->allocNextElm(k);
    return a->moveVal(tv, value);
  }
  return genericAddNewElemC(ad, value);
}

ArrayData* HphpArray::AppendRefPacked(ArrayData* ad, CVarRef v, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  auto &tv = a->allocNextElm(a->m_size);
  return a->initRef(tv, v);
}

ArrayData* HphpArray::AppendRef(ArrayData* ad, CVarRef v, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  return a->nextInsertRef(v);
}

ArrayData *HphpArray::AppendWithRefPacked(ArrayData* ad, CVarRef v, bool copy) {
  auto a = asPacked(ad);
  if (copy) a = a->copyPacked();
  auto& tv = a->allocNextElm(a->m_size);
  return a->initWithRef(tv, v);
}

ArrayData *HphpArray::AppendWithRef(ArrayData* ad, CVarRef v, bool copy) {
  auto a = asMixed(ad);
  if (copy) a = a->copyMixed();
  return a->nextInsertWithRef(v);
}

ArrayData* HphpArray::Plus(ArrayData* ad, const ArrayData* elems, bool copy) {
  auto a = asHphpArray(ad);
  if (copy) a = a->copyImpl();
  if (a->isPacked()) {
    // todo t2606310: is there a fast path if elems is also a vector?
    a->packedToMixed();
  }
  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    CVarRef value = it.secondRef();
    if (key.asTypedValue()->m_type == KindOfInt64) {
      a->addValWithRef(key.toInt64(), value);
    } else {
      a->addValWithRef(key.getStringData(), value);
    }
  }
  return a;
}

ArrayData* HphpArray::Merge(ArrayData* ad, const ArrayData* elems, bool copy) {
  auto a = asHphpArray(ad);
  if (copy) a = a->copyImpl();
  if (a->isPacked()) {
    // todo t2606310: is there a fast path if elems is also a vector?
    a->packedToMixed();
  }
  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    CVarRef value = it.secondRef();
    if (key.asTypedValue()->m_type == KindOfInt64) {
      a->nextInsertWithRef(value);
    } else {
      Variant *p;
      StringData *sd = key.getStringData();
      a->addLvalImpl(sd, sd->hash(), p);
      p->setWithRef(value);
    }
  }
  return a;
}

ArrayData* HphpArray::PopPacked(ArrayData* ad, Variant& value) {
  auto a = asPacked(ad);
  if (a->getCount() > 1) a = a->copyPacked();
  if (a->m_size > 0) {
    auto i = a->m_size - 1;
    auto& tv = a->m_data[i].data;
    value = tvAsCVarRef(&tv);
    if (a->strongIterators()) a->adjustFullPos(i);
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
  if (a->getCount() > 1) a = a->copyMixed();
  Elm* elms = a->m_data;
  ElmInd pos = IterEnd(a);
  if (validElmInd(pos)) {
    Elm* e = &elms[pos];
    assert(!isTombstone(e->data.m_type));
    value = tvAsCVarRef(&e->data);
    ElmInd* ei = e->hasStrKey()
        ? a->findForInsert(e->key, e->hash())
        : a->findForInsert(e->ikey);
    a->erase(ei, true);
  } else {
    value = uninit_null();
  }
  // To conform to PHP behavior, the pop operation resets the array's
  // internal iterator.
  a->m_pos = a->nextElm(elms, ElmIndEmpty);
  return a;
}

ArrayData* HphpArray::DequeuePacked(ArrayData* ad, Variant& value) {
  auto a = asPacked(ad);
  if (a->getCount() > 1) a = a->copyPacked();
  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is removed from the beginning of the array.
  a->freeStrongIterators();
  auto elms = a->m_data;
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

ArrayData* HphpArray::Dequeue(ArrayData* ad, Variant& value) {
  auto a = asMixed(ad);
  if (a->getCount() > 1) a = a->copyMixed();
  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is removed from the beginning of the array.
  a->freeStrongIterators();
  Elm* elms = a->m_data;
  ElmInd pos = a->nextElm(elms, ElmIndEmpty);
  if (validElmInd(pos)) {
    Elm* e = &elms[pos];
    value = tvAsCVarRef(&e->data);
    ElmInd* ei = e->hasStrKey() ?  a->findForInsert(e->key, e->hash()) :
                 a->findForInsert(e->ikey);
    a->erase(ei, false);
    a->compact(true);
  } else {
    value = uninit_null();
  }
  // To conform to PHP behavior, the dequeue operation resets the array's
  // internal iterator
  a->m_pos = a->nextElm(elms, ElmIndEmpty);
  return a;
}

ArrayData* HphpArray::PrependPacked(ArrayData* ad, CVarRef v, bool copy) {
  auto a = asPacked(ad);
  if (a->getCount() > 1) a = a->copyPacked();
  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is added to the beginning of the array.
  a->freeStrongIterators();
  size_t n = a->m_size;
  if (n > 0) {
    if (n == a->m_cap) a->growPacked();
    auto elms = a->m_data;
    memmove(&elms[1], &elms[0], n * sizeof(elms[0]));
  }
  a->m_size = a->m_used = n + 1;
  a->m_pos = 0;
  a->initVal(a->m_data[0].data, v);
  return a;
}

ArrayData* HphpArray::Prepend(ArrayData* ad, CVarRef v, bool copy) {
  auto a = asMixed(ad);
  if (a->getCount() > 1) a = a->copyMixed();
  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is added to the beginning of the array.
  a->freeStrongIterators();

  Elm* elms = a->m_data;
  if (a->m_used == 0 || !isTombstone(elms[0].data.m_type)) {
    // Make sure there is room to insert an element.
    a->resizeIfNeeded();
    // Reload elms, in case resizeIfNeeded() had side effects.
    elms = a->m_data;
    // Move the existing elements to make element 0 available.
    memmove(&elms[1], &elms[0], a->m_used * sizeof(Elm));
    ++a->m_used;
  }

  // Prepend.
  ++a->m_size;
  Elm* e = &elms[0];
  e->setIntKey(0);
  a->initVal(e->data, v);

  // Renumber.
  a->compact(true);
  // To conform to PHP behavior, the prepend operation resets the array's
  // internal iterator
  a->m_pos = a->nextElm(elms, ElmIndEmpty);
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
  Elm* elms = a->m_data;
  for (uint32_t i = 0, limit = a->m_size; i < limit; ++i) {
    tvAsVariant(&elms[i].data).setEvalScalar();
  }
}

void HphpArray::OnSetEvalScalar(ArrayData* ad) {
  auto a = asMixed(ad);
  Elm* elms = a->m_data;
  for (uint32_t i = 0, limit = a->m_used; i < limit; ++i) {
    Elm* e = &elms[i];
    if (!isTombstone(e->data.m_type)) {
      StringData *key = e->key;
      if (e->hasStrKey() && !key->isStatic()) {
        e->key = StringData::GetStaticString(key);
        decRefStr(key);
      }
      tvAsVariant(&e->data).setEvalScalar();
    }
  }
}

bool HphpArray::ValidFullPos(const ArrayData* ad, const FullPos &fp) {
  assert(fp.getContainer() == asHphpArray(ad));
  if (fp.getResetFlag()) return false;
  return (fp.m_pos != ssize_t(ElmIndEmpty));
}

bool HphpArray::AdvanceFullPos(ArrayData* ad, FullPos& fp) {
  auto a = asHphpArray(ad);
  Elm* elms = a->m_data;
  if (fp.getResetFlag()) {
    fp.setResetFlag(false);
    fp.m_pos = ElmIndEmpty;
  } else if (fp.m_pos == ssize_t(ElmIndEmpty)) {
    return false;
  }
  fp.m_pos = a->nextElm(elms, fp.m_pos);
  if (fp.m_pos == ssize_t(ElmIndEmpty)) {
    return false;
  }
  // To conform to PHP behavior, we need to set the internal
  // cursor to point to the next element.
  a->m_pos = a->nextElm(elms, fp.m_pos);
  return true;
}

//=============================================================================

NEVER_INLINE ArrayData* HphpArray::NonSmartCopy(const ArrayData* ad) {
  auto a = asHphpArray(ad);
  return a->isPacked() ?
    new HphpArray(*a, AllocationMode::nonSmart, ClonePacked()) :
    new HphpArray(*a, AllocationMode::nonSmart, CloneMixed());
}

NEVER_INLINE HphpArray* HphpArray::copyPacked() const {
  assert(checkInvariants());
  return new (HphpArray::AllocatorType::getNoCheck()->alloc(sizeof(*this)))
         HphpArray(*this, AllocationMode::smart, ClonePacked());
}

NEVER_INLINE HphpArray* HphpArray::copyMixed() const {
  assert(checkInvariants());
  return new (HphpArray::AllocatorType::getNoCheck()->alloc(sizeof(*this)))
         HphpArray(*this, AllocationMode::smart, CloneMixed());
}

//=============================================================================

HOT_FUNC_VM
HphpArray* ArrayData::MakeTuple(uint size, const TypedValue* data) {
  auto a = NEW(HphpArray)(size, data);
  a->setRefCount(1);
  TRACE(2, "MakeTuple: size %d\n", size);
  return a;
}

HOT_FUNC_VM
HphpArray* ArrayData::MakeReserve(uint capacity) {
  auto a = NEW(HphpArray)(capacity);
  a->setRefCount(1);
  TRACE(2, "MakeReserve: capacity %d\n", capacity);
  return a;
}

///////////////////////////////////////////////////////////////////////////////
}
