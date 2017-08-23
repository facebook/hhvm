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

#ifndef incl_HPHP_MEMORY_MANAGER_INL_H
#define incl_HPHP_MEMORY_MANAGER_INL_H

#include <limits>
#include <utility>

#include "hphp/util/compilation-flags.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static_assert(
  kMaxSmallSize <= std::numeric_limits<uint32_t>::max(),
  "Size-specified small block alloc functions assume this"
);

inline MemoryManager& MM() {
  return *MemoryManager::TlsWrapper::getNoCheck();
}

///////////////////////////////////////////////////////////////////////////////

inline BigHeap::~BigHeap() {
  reset();
}

inline bool BigHeap::empty() const {
  return m_slabs.empty() && m_bigs.empty();
}

inline bool ContiguousBigHeap::empty() const {
  return m_base == m_front;
}

inline size_t ContiguousBigHeap::chunk_index(char* p) const {
  assert(p >= m_base);
  return (p - m_base) / ChunkSize;
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

inline void* MemoryManager::FreeList::maybePop() {
  auto ret = head;
  if (LIKELY(ret != nullptr)) head = ret->next;
  FTRACE(4, "FreeList::maybePop(): returning {}\n", ret);
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

inline size_t MemoryManager::estimateCap(size_t requested) {
  return requested <= kMaxSmallSize ? smallSizeClass(requested)
                                    : requested;
}

inline size_t MemoryManager::bsrq(size_t x) {
#if defined(__x86_64__)
  size_t ret;
  __asm__ ("bsrq %1, %0"
           : "=r"(ret) // Outputs.
           : "r"(x)    // Inputs.
           );
  return ret;
#elif defined(__powerpc64__)
  size_t ret;
  __asm__ ("cntlzd %0, %1"
           : "=r"(ret) // Outputs.
           : "r"(x)    // Inputs.
           );
  return 63 - ret;
#elif defined(__aarch64__)
  size_t ret;
  __asm__ ("clz %x0, %x1"
           : "=r"(ret) // Outputs.
           : "r"(x)    // Inputs.
           );
  return 63 - ret;
#else
  // Equivalent (but incompletely strength-reduced by gcc):
  return 63 - __builtin_clzl(x);
#endif
}

inline size_t MemoryManager::computeSize2Index(size_t size) {
  assert(size > 1);
  assert(size <= kMaxSizeClass);
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
  size_t nBits = bsrq(--size);
  if (UNLIKELY(nBits < kLgSizeClassesPerDoubling + kLgSmallSizeQuantum)) {
    // denormal sizes
    // UNLIKELY because these normally go through lookupSmallSize2Index
    return size >> kLgSmallSizeQuantum;
  }
  size_t exp = nBits - (kLgSizeClassesPerDoubling + kLgSmallSizeQuantum);
  size_t rawMantissa = size >> (nBits - kLgSizeClassesPerDoubling);
  size_t index = (exp << kLgSizeClassesPerDoubling) + rawMantissa;
  assert(index < kNumSizeClasses);
  return index;
}

inline size_t MemoryManager::lookupSmallSize2Index(size_t size) {
  assert(size > 0);
  assert(size <= kMaxSmallSizeLookup);
  auto const index = kSmallSize2Index[(size-1) >> kLgSmallSizeQuantum];
  return index;
}

inline size_t MemoryManager::smallSize2Index(size_t size) {
  assert(size > 0);
  assert(size <= kMaxSmallSize);
  if (LIKELY(size <= kMaxSmallSizeLookup)) {
    return lookupSmallSize2Index(size);
  }
  return computeSize2Index(size);
}

inline size_t MemoryManager::size2Index(size_t size) {
  assert(size > 0);
  assert(size <= kMaxSizeClass);
  if (LIKELY(size <= kMaxSmallSizeLookup)) {
    return lookupSmallSize2Index(size);
  }
  return computeSize2Index(size);
}

inline size_t MemoryManager::sizeIndex2Size(size_t index) {
  return kSizeIndex2Size[index];
}

inline size_t MemoryManager::smallSizeClass(size_t size) {
  assert(size > 1);
  assert(size <= kMaxSmallSize);
  // Round up to the nearest kLgSizeClassesPerDoubling + 1 significant bits,
  // or to the nearest kLgSmallSizeQuantum, whichever is greater.
  ssize_t nInsignificantBits = bsrq(--size) - kLgSizeClassesPerDoubling;
  size_t roundTo = (nInsignificantBits < ssize_t(kLgSmallSizeQuantum))
    ? kLgSmallSizeQuantum : nInsignificantBits;
  size_t ret = ((size >> roundTo) + 1) << roundTo;
  assert(ret >= kSmallSizeAlign);
  assert(ret <= kMaxSmallSize);
  return ret;
}

inline void* MemoryManager::mallocSmallIndex(size_t index) {
  assert(index < kNumSmallSizes);
  if (debug) requestEagerGC();

  auto bytes = sizeIndex2Size(index);
  m_stats.mmUsage += bytes;

  void *p = m_freelists[index].maybePop();
  if (UNLIKELY(p == nullptr)) {
    p = mallocSmallSizeSlow(bytes, index);
  }
  assert((reinterpret_cast<uintptr_t>(p) & kSmallSizeAlignMask) == 0);
  FTRACE(3, "mallocSmallIndex: {} -> {}\n", bytes, p);
  return p;
}

inline void* MemoryManager::mallocSmallSize(size_t bytes) {
  assert(bytes > 0);
  assert(bytes <= kMaxSmallSize);
  return mallocSmallIndex(smallSize2Index(bytes));
}

inline void MemoryManager::freeSmallIndex(void* ptr, size_t index) {
  assert(index < kNumSmallSizes);
  assert((reinterpret_cast<uintptr_t>(ptr) & kSmallSizeAlignMask) == 0);

  if (UNLIKELY(m_bypassSlabAlloc)) {
    return freeBigSize(ptr);
  }

  size_t bytes = sizeIndex2Size(index);
  FTRACE(3, "freeSmallIndex({}, {}), freelist {}\n", ptr, bytes, index);

  m_freelists[index].push(ptr);
  m_stats.mmUsage -= bytes;
}

inline void MemoryManager::freeSmallSize(void* ptr, size_t bytes) {
  freeSmallIndex(ptr, smallSize2Index(bytes));
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
void* MemoryManager::objMalloc(size_t size) {
  if (LIKELY(size <= kMaxSmallSize)) return mallocSmallSize(size);
  return mallocBigSize<Unzeroed>(size);
}

ALWAYS_INLINE
void MemoryManager::objFree(void* vp, size_t size) {
  if (LIKELY(size <= kMaxSmallSize)) return freeSmallSize(vp, size);
  freeBigSize(vp);
}

ALWAYS_INLINE
void* MemoryManager::objMallocIndex(size_t index) {
  if (LIKELY(index < kNumSmallSizes)) return mallocSmallIndex(index);
  return mallocBigSize<Unzeroed>(sizeIndex2Size(index));
}

ALWAYS_INLINE
void MemoryManager::objFreeIndex(void* ptr, size_t index) {
  if (LIKELY(index < kNumSmallSizes)) return freeSmallIndex(ptr, index);
  return freeBigSize(ptr);
}

///////////////////////////////////////////////////////////////////////////////

inline int64_t MemoryManager::getAllocated() const {
  if (use_jemalloc) {
    assert(m_allocated);
    return *m_allocated;
  }
  return 0;
}

inline int64_t MemoryManager::getDeallocated() const {
  if (use_jemalloc) {
    assert(m_deallocated);
    return *m_deallocated;
  } else {
    return 0;
  }
}

inline int64_t MemoryManager::currentUsage() const {
  return m_stats.mmUsage;
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

inline bool MemoryManager::startStatsInterval() {
  auto ret = !m_statsIntervalActive;
  // Fetch current stats without changing m_stats or triggering OOM.
  auto stats = getStatsCopy();
  // For the reasons stated below in refreshStatsImpl, usage can potentially be
  // negative. Make sure that doesn't occur here.
  m_stats.peakIntervalUsage = std::max<int64_t>(0, stats.usage());
  m_stats.peakIntervalCap = m_stats.capacity;
  assert(m_stats.peakIntervalCap >= 0);
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

inline bool MemoryManager::contains(void* p) const {
  return m_heap.contains(p);
}

inline bool MemoryManager::checkContains(DEBUG_ONLY void* p) const {
  // Be conservative if the small-block allocator is disabled.
  assert(RuntimeOption::DisableSmallAllocator || m_bypassSlabAlloc ||
         contains(p));
  return true;
}

inline HeapObject* MemoryManager::find(const void* p) {
  initFree();
  return m_heap.find(p);
}

///////////////////////////////////////////////////////////////////////////////

inline bool MemoryManager::sweeping() {
  return !TlsWrapper::isNull() && tl_sweeping;
}

inline bool MemoryManager::exiting() {
  return !TlsWrapper::isNull() && MM().m_exiting;
}

inline void MemoryManager::setExiting() {
  if (!TlsWrapper::isNull()) MM().m_exiting = true;
}

inline StringDataNode& MemoryManager::getStringList() {
  return m_strings;
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
