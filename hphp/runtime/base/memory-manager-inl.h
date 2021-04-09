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

#pragma once

#include <limits>
#include <utility>

#include "hphp/util/bitops.h"
#include "hphp/util/compilation-flags.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static_assert(
  kMaxSmallSize <= std::numeric_limits<uint32_t>::max(),
  "Size-specified small block alloc functions assume this"
);

///////////////////////////////////////////////////////////////////////////////

inline bool SparseHeap::empty() const {
  return m_bigs.empty();
}

///////////////////////////////////////////////////////////////////////////////

struct MemoryManager::MaskAlloc {
  explicit MaskAlloc(MemoryManager& mm)
    : m_mm(mm)
    , startAlloc(s_statsEnabled ? *mm.m_allocated : 0)
    , startDealloc(s_statsEnabled ? *mm.m_deallocated : 0)
  {
    // capture all mallocs prior to construction
    FTRACE(1, "MaskAlloc()\n");
    m_mm.refreshStats();
  }

  ~MaskAlloc() {
    FTRACE(1, "~MaskAlloc()\n");
    // exclude mallocs and frees since construction
    if (s_statsEnabled) {
      FTRACE(1, "old: reset alloc: {} reset dealloc: {}\n",
        m_mm.m_resetAllocated, m_mm.m_resetDeallocated);

      m_mm.m_resetAllocated += *m_mm.m_allocated - startAlloc;
      m_mm.m_resetDeallocated += *m_mm.m_deallocated - startDealloc;

      FTRACE(1, "new: reset alloc: {} prev dealloc: {}\n\n",
        m_mm.m_resetAllocated, m_mm.m_resetDeallocated);
    }
  }

  MaskAlloc(const MaskAlloc&) = delete;
  MaskAlloc& operator=(const MaskAlloc&) = delete;

private:
  MemoryManager& m_mm;
  const uint64_t startAlloc;
  const uint64_t startDealloc;
};

struct MemoryManager::CountMalloc {
  explicit CountMalloc(MemoryManager& mm, uint64_t& allocated)
    : m_mm(mm)
    , m_allocated(allocated)
    , startAlloc(s_statsEnabled ? *mm.m_allocated : 0)
  {}

  ~CountMalloc() {
    if (s_statsEnabled) {
      assertx(*m_mm.m_allocated >= startAlloc);
      m_allocated = *m_mm.m_allocated - startAlloc;
    } else {
      m_allocated = 0;
    }
  }

  CountMalloc(const CountMalloc&) = delete;
  CountMalloc& operator=(const CountMalloc&) = delete;

private:
  MemoryManager& m_mm;
  uint64_t& m_allocated;
  const uint64_t startAlloc;
};

inline void MemoryManager::takeCreditForFreeOnOtherThread(uint64_t size) {
  m_freedOnOtherThread += size;
}

struct MemoryManager::SuppressOOM {
  explicit SuppressOOM(MemoryManager& mm)
      : m_mm(mm)
      , m_savedCouldOOM(mm.m_couldOOM) {
    FTRACE(2, "SuppressOOM() [couldOOM was {}]\n", m_savedCouldOOM);
    m_mm.m_couldOOM = false;
  }

  ~SuppressOOM() {
    FTRACE(2, "~SuppressOOM() [couldOOM is {}]\n", m_savedCouldOOM);
    m_mm.m_couldOOM = m_savedCouldOOM;
  }

  SuppressOOM(const SuppressOOM&) = delete;
  SuppressOOM& operator=(const SuppressOOM&) = delete;

private:
  MemoryManager& m_mm;
  bool m_savedCouldOOM;
};

///////////////////////////////////////////////////////////////////////////////

inline int operator<<(HeaderKind k, int bits) {
  return int(k) << bits;
}

inline void* MemoryManager::FreeList::likelyPop() {
  auto ret = head;
  if (LIKELY(ret != nullptr)) {
    // head already prefetched, this load should be fast
    auto next = ret->next;
    __builtin_prefetch(next, 0, 2); // HINT_T1 on x64
    head = next;
  }
  FTRACE(4, "FreeList::likelyPop(): returning {}\n", ret);
  return ret;
}

inline void* MemoryManager::FreeList::unlikelyPop() {
  auto ret = head;
  if (UNLIKELY(ret != nullptr)) head = ret->next;
  FTRACE(4, "FreeList::unlikelyPop(): returning {}\n", ret);
  return ret;
}

inline FreeNode*
FreeNode::InitFrom(void* addr, uint32_t size, HeaderKind kind) {
  auto node = static_cast<FreeNode*>(addr);
  node->initHeader_32(kind, size);
  return node;
}

inline FreeNode*
FreeNode::UninitFrom(void* addr, FreeNode* next) {
  // The extra store to initialize a HeaderKind::Free here would be expensive.
  // Instead, initFree() initializes free headers just before iterating
  auto node = static_cast<FreeNode*>(addr);
  node->next = next;
  return node;
}

inline void MemoryManager::FreeList::push(void* val) {
  FTRACE(4, "FreeList::push({}), prev head = {}\n", val, head);
  head = FreeNode::UninitFrom(val, head);
}

///////////////////////////////////////////////////////////////////////////////

inline size_t MemoryManager::computeSize2Index(size_t size) {
  assertx(size > 1);
  assertx(size <= kMaxSizeClass);
  // We want to round size up to the nearest size class, and return the index
  // of that size class. The first 1 << kLgSizeClassesPerDoubling size classes
  // are denormal; their sizes are (class + 1) << kLgSmallSizeQuantum.
  // After that we have the normal size classes, whose size is
  // (1 << kLgSizeClassesPerDoubling + mantissa) << (exp + kLgSmallSizeQuantum)
  // where (mantissa - 1) is stored in the kLgSizeClassesPerDoubling low bits
  // of the class index, and (exp + 1) is stored in the bits above that; for
  // denormal sizes, the bits above the mantissa are stored as 0.
  // In the normal case, if we do the math naively, we need to calculate
  // the class index as
  // (exp + 1) << kLgSizeClassesPerDoubling + (mantissa - 1)
  // but conveniently, that's equivalent to
  // (exp << kLgSizeClassesPerDoubling) +
  //   (1 << kLgSizeClassesPerDoubling + mantissa - 1)
  // This lets us skip taking the leading 1 off of the mantissa, and skip
  // adding 1 to the exponent.
  size_t nBits = fls64(--size);
  if (UNLIKELY(nBits < kLgSizeClassesPerDoubling + kLgSmallSizeQuantum)) {
    // denormal sizes
    // UNLIKELY because these normally go through lookupSmallSize2Index
    return size >> kLgSmallSizeQuantum;
  }
  size_t exp = nBits - (kLgSizeClassesPerDoubling + kLgSmallSizeQuantum);
  size_t rawMantissa = size >> (nBits - kLgSizeClassesPerDoubling);
  size_t index = (exp << kLgSizeClassesPerDoubling) + rawMantissa;
  assertx(index < kNumSizeClasses);
  return index;
}

inline size_t MemoryManager::lookupSmallSize2Index(size_t size) {
  assertx(size > 0);
  assertx(size <= kMaxSmallSizeLookup);
  auto const index = kSmallSize2Index[(size-1) >> kLgSmallSizeQuantum];
  return index;
}

inline size_t MemoryManager::size2Index(size_t size) {
  assertx(size > 0);
  assertx(size <= kMaxSizeClass);
  if (LIKELY(size <= kMaxSmallSizeLookup)) {
    return lookupSmallSize2Index(size);
  }
  return computeSize2Index(size);
}

inline constexpr size_t MemoryManager::sizeIndex2Size(size_t index) {
  return kSizeIndex2Size[index];
}

inline size_t MemoryManager::sizeClass(size_t size) {
  assertx(size > 1);
  assertx(size <= kMaxSizeClass);
  // Round up to the nearest kLgSizeClassesPerDoubling + 1 significant bits,
  // or to the nearest kLgSmallSizeQuantum, whichever is greater.
  ssize_t nInsignificantBits = fls64(--size) - kLgSizeClassesPerDoubling;
  size_t roundTo = (nInsignificantBits < ssize_t(kLgSmallSizeQuantum))
    ? kLgSmallSizeQuantum : nInsignificantBits;
  size_t ret = ((size >> roundTo) + 1) << roundTo;
  assertx(ret >= kSmallSizeAlign);
  assertx(ret <= kMaxSizeClass);
  return ret;
}

inline void* MemoryManager::mallocSmallIndex(size_t index) {
  return mallocSmallIndexSize(index, sizeIndex2Size(index));
}

inline void* MemoryManager::mallocSmallIndexSize(size_t index, size_t bytes) {
  if (debug) requestEagerGC();

  m_stats.mm_udebt -= bytes;
  if (LIKELY(m_stats.mm_udebt <= std::numeric_limits<int64_t>::max())) {
    auto clamped = std::min(index, kNumSmallSizes);
    if (auto p = m_freelists[clamped].likelyPop()) {
      assertx((reinterpret_cast<uintptr_t>(p) & kSmallSizeAlignMask) == 0);
      FTRACE(3, "mallocSmallIndex: {} -> {}\n", bytes, p);
      return p;
    }
  }
  return mallocSmallSizeSlow(bytes, index);
}

inline void* MemoryManager::mallocSmallSize(size_t bytes) {
  assertx(bytes > 0);
  // mallocSmallIndex() converts the size index back to a size to track the
  // size class's actual size, rather than the requested size.
  return mallocSmallIndex(size2Index(bytes));
}

inline void MemoryManager::freeSmallIndex(void* ptr, size_t index) {
  assertx((reinterpret_cast<uintptr_t>(ptr) & kSmallSizeAlignMask) == 0);

  size_t bytes = sizeIndex2Size(index);
  FTRACE(3, "freeSmallIndex({}, {}), freelist {}\n", ptr, bytes, index);

  assertx(memset(ptr, kSmallFreeFill, bytes));
  auto clamped = std::min(index, kNumSmallSizes);
  if (LIKELY(m_freelists[clamped].head != nullptr)) {
    m_freelists[clamped].push(ptr);
    m_stats.mm_freed += bytes;
  } else {
    freeSmallIndexSlow(ptr, index, bytes);
  }
}

inline void MemoryManager::freeSmallSize(void* ptr, size_t bytes) {
  freeSmallIndex(ptr, size2Index(bytes));
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
void* MemoryManager::objMalloc(size_t size) {
  return mallocSmallSize(size);
}

ALWAYS_INLINE
void MemoryManager::objFree(void* vp, size_t size) {
  freeSmallSize(vp, size);
}

ALWAYS_INLINE
void* MemoryManager::objMallocIndex(size_t index) {
  return mallocSmallIndex(index);
}

ALWAYS_INLINE
void MemoryManager::objFreeIndex(void* ptr, size_t index) {
  freeSmallIndex(ptr, index);
}

///////////////////////////////////////////////////////////////////////////////

inline int64_t MemoryManager::getAllocated() const {
  if (use_jemalloc) {
    assertx(m_allocated);
    return *m_allocated;
  }
  return 0;
}

inline int64_t MemoryManager::getDeallocated() const {
  if (use_jemalloc) {
    assertx(m_deallocated);
    return *m_deallocated;
  } else {
    return 0;
  }
}

inline int64_t MemoryManager::currentUsage() const {
  return m_stats.mmUsage();
}

inline MemoryUsageStats MemoryManager::getStats() {
  refreshStats();
  return m_stats;
}

inline MemoryUsageStats MemoryManager::getStatsCopy() {
  auto copy = m_stats;
  refreshStatsImpl(copy);
  return copy;
}

inline const MemoryUsageStats& MemoryManager::getStatsRaw() const {
  return m_stats;
}

inline bool MemoryManager::startStatsInterval() {
  auto ret = !m_statsIntervalActive;
  // Fetch current stats without changing m_stats or triggering OOM.
  auto stats = getStatsCopy();
  // For the reasons stated below in refreshStatsImpl, usage can potentially be
  // negative. Make sure that doesn't occur here.
  m_stats.peakIntervalUsage = std::max<int64_t>(0, stats.usage());
  m_stats.peakIntervalCap = m_stats.capacity();
  assertx(m_stats.peakIntervalCap >= 0);
  m_statsIntervalActive = true;
  return ret;
}

inline bool MemoryManager::stopStatsInterval() {
  auto ret = m_statsIntervalActive;
  m_statsIntervalActive = false;
  m_stats.peakIntervalUsage = 0;
  m_stats.peakIntervalCap = 0;
  return ret;
}

inline int64_t MemoryManager::getMemoryLimit() const {
  return m_usageLimit;
}

inline bool MemoryManager::preAllocOOM(int64_t size) {
  if (m_couldOOM) {
    auto stats = getStatsCopy();
    if (stats.usage() + size > m_usageLimit) {
      refreshStatsHelperExceeded();
      return true;
    }
  }
  return false;
}

inline void MemoryManager::forceOOM() {
  if (m_couldOOM) {
    refreshStatsHelperExceeded();
  }
}

///////////////////////////////////////////////////////////////////////////////

inline bool MemoryManager::empty() const {
  return m_heap.empty();
}

inline bool MemoryManager::contains(const void* p) const {
  return m_heap.contains(p);
}

inline HeapObject* MemoryManager::find(const void* p) {
  initFree();
  return m_heap.find(p);
}

///////////////////////////////////////////////////////////////////////////////

inline bool MemoryManager::sweeping() {
  return tl_heap && tl_sweeping;
}

inline bool MemoryManager::exiting() {
  return tl_heap && tl_heap->m_exiting;
}

inline void MemoryManager::setExiting() {
  if (tl_heap) tl_heap->m_exiting = true;
}

inline StringDataNode& MemoryManager::getStringList() {
  return m_strings;
}

///////////////////////////////////////////////////////////////////////////////

namespace req {
template<class T, class... Args> T* make_raw(Args&&... args) {
  static_assert(alignof(T) <= sizeof(MallocNode) &&
                alignof(T) <= kSmallSizeAlign, "");
  auto constexpr size = sizeof(MallocNode) + sizeof(T);
  auto n = static_cast<MallocNode*>(tl_heap->objMalloc(size));
  n->initHeader_32_16(HeaderKind::Cpp, 0,
                      type_scan::getIndexForMalloc<T>());
  n->nbytes = size;
  try {
    return new (n + 1) T(std::forward<Args>(args)...);
  } catch (...) {
    tl_heap->objFree(n, size);
    throw;
  }
}

template<class T> void destroy_raw(T* t) {
  t->~T();
  auto n = reinterpret_cast<MallocNode*>(t) - 1;
  tl_heap->objFree(n, n->nbytes);
}
}

}
