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

#ifndef incl_HPHP_MEMORY_MANAGER_INL_H
#define incl_HPHP_MEMORY_MANAGER_INL_H

#include <limits>
#include <utility>

#include "hphp/util/compilation-flags.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

static_assert(
  kMaxSmartSize <= std::numeric_limits<uint32_t>::max(),
  "Size-specified smart malloc functions assume this"
);

//////////////////////////////////////////////////////////////////////

inline MemoryManager& MM() { return *MemoryManager::TlsWrapper::getNoCheck(); }

template<class T, class... Args> T* smart_new(Args&&... args) {
  auto const mem = smart_malloc(sizeof(T));
  try {
    return new (mem) T(std::forward<Args>(args)...);
  } catch (...) {
    smart_free(mem);
    throw;
  }
}

template<class T> void smart_delete(T* t) {
  t->~T();
  smart_free(t);
}

template<class T> T* smart_new_array(size_t count) {
  T* ret = static_cast<T*>(smart_malloc(count * sizeof(T)));
  size_t i = 0;
  try {
    for (; i < count; ++i) {
      new (&ret[i]) T();
    }
  } catch (...) {
    size_t j = i;
    while (j-- > 0) {
      ret[j].~T();
    }
    smart_free(ret);
    throw;
  }
  return ret;
}

template<class T>
void smart_delete_array(T* t, size_t count) {
  size_t i = count;
  while (i-- > 0) {
    t[i].~T();
  }
  smart_free(t);
}

//////////////////////////////////////////////////////////////////////

struct MemoryManager::MaskAlloc {
  explicit MaskAlloc(MemoryManager& mm) : m_mm(mm) {
    // capture all mallocs prior to construction
    FTRACE(1, "MaskAlloc()\n");
    m_mm.refreshStats();
  }
  ~MaskAlloc() {
    FTRACE(1, "~MaskAlloc()\n");
#ifdef USE_JEMALLOC
    // exclude mallocs and frees since construction
    if (s_statsEnabled) {
      FTRACE(1, "old: prev alloc: {}\nprev dealloc: {}\n",
        m_mm.m_prevAllocated, m_mm.m_prevDeallocated);

      m_mm.m_prevAllocated = *m_mm.m_allocated;
      m_mm.m_prevDeallocated = *m_mm.m_deallocated;

      FTRACE(1, "new: prev alloc: {}\nprev dealloc: {}\n\n",
        m_mm.m_prevAllocated, m_mm.m_prevDeallocated);
    }
#endif
  }

  MaskAlloc(const MaskAlloc&) = delete;
  MaskAlloc& operator=(const MaskAlloc&) = delete;

private:
  MemoryManager& m_mm;
};

struct MemoryManager::SuppressOOM {
  explicit SuppressOOM(MemoryManager& mm)
      : m_mm(mm)
      , m_savedCouldOOM(mm.m_couldOOM) {
    FTRACE(1, "SuppressOOM() [couldOOM was {}]\n", m_savedCouldOOM);
    m_mm.m_couldOOM = false;
  }

  ~SuppressOOM() {
    FTRACE(1, "~SuppressOOM() [couldOOM is {}]\n", m_savedCouldOOM);
    m_mm.m_couldOOM = m_savedCouldOOM;
  }

  SuppressOOM(const SuppressOOM&) = delete;
  SuppressOOM& operator=(const SuppressOOM&) = delete;

private:
  MemoryManager& m_mm;
  bool m_savedCouldOOM;
};

//////////////////////////////////////////////////////////////////////

inline int operator<<(HeaderKind k, int bits) {
  return int(k) << bits;
}

inline void* MemoryManager::FreeList::maybePop() {
  auto ret = head;
  if (LIKELY(ret != nullptr)) head = ret->next;
  FTRACE(4, "FreeList::maybePop(): returning {}\n", ret);
  return ret;
}

inline void MemoryManager::FreeList::push(void* val, size_t size) {
  FTRACE(4, "FreeList::push({}, {}), prev head = {}\n", val, size, head);
  auto constexpr kMaxFreeSize = std::numeric_limits<uint32_t>::max();
  static_assert(kMaxSmartSize <= kMaxFreeSize, "");
  assert(size > 0 && size <= kMaxFreeSize);
  auto const node = static_cast<FreeNode*>(val);
  node->next = head;
  // The extra store to initialize a free header here is expensive.
  // Instead, initFree() initializes all free headers just before iterating
  head = node;
}

//////////////////////////////////////////////////////////////////////

template<class SizeT> ALWAYS_INLINE
SizeT MemoryManager::debugAddExtra(SizeT sz) {
  if (!debug) return sz;
  return sz + kDebugExtraSize;
}

template<class SizeT> ALWAYS_INLINE
SizeT MemoryManager::debugRemoveExtra(SizeT sz) {
  if (!debug) return sz;
  return sz - kDebugExtraSize;
}

#ifndef DEBUG

ALWAYS_INLINE void* MemoryManager::debugPostAllocate(void* p, size_t, size_t) {
  return p;
}

ALWAYS_INLINE void* MemoryManager::debugPreFree(void* p, size_t, size_t) {
  return p;
}

#endif

//////////////////////////////////////////////////////////////////////

inline uint32_t MemoryManager::bsr(uint32_t x) {
#if defined(__i386__) || defined(__x86_64__)
  uint32_t ret;
  __asm__ ("bsr %1, %0"
           : "=r"(ret) // Outputs.
           : "r"(x)    // Inputs.
           );
  return ret;
#else
  // Equivalent (but incompletely strength-reduced by gcc):
  return 31 - __builtin_clz(x);
#endif
}

inline uint8_t MemoryManager::smartSize2IndexCompute(uint32_t size) {
  uint32_t x = bsr((size<<1)-1);
  uint32_t shift = (x < kLgSizeClassesPerDoubling + kLgSmartSizeQuantum)
                   ? 0 : x - (kLgSizeClassesPerDoubling + kLgSmartSizeQuantum);
  uint32_t grp = shift << kLgSizeClassesPerDoubling;

  int32_t lgReduced = x - kLgSizeClassesPerDoubling
                      - 1; // Counteract left shift of bsr() argument.
  uint32_t lgDelta = (lgReduced < int32_t(kLgSmartSizeQuantum))
                     ? kLgSmartSizeQuantum : lgReduced;
  uint32_t deltaInverseMask = -1 << lgDelta;
  constexpr uint32_t kModMask = (1u << kLgSizeClassesPerDoubling) - 1;
  uint32_t mod = ((((size-1) & deltaInverseMask) >> lgDelta)) & kModMask;

  auto const index = grp + mod;
  assert(index < kNumSmartSizes);
  return index;
}

inline uint8_t MemoryManager::smartSize2IndexLookup(uint32_t size) {
  uint8_t index = kSmartSize2Index[(size-1) >> kLgSmartSizeQuantum];
  assert(index == smartSize2IndexCompute(size));
  return index;
}

inline uint8_t MemoryManager::smartSize2Index(uint32_t size) {
  assert(size > 0);
  assert(size <= kMaxSmartSize + kDebugExtraSize);
  if (LIKELY(size <= kMaxSmartSizeLookup)) {
    return smartSize2IndexLookup(size);
  }
  return smartSize2IndexCompute(size);
}

inline uint32_t MemoryManager::smartSizeClass(uint32_t reqBytes) {
  uint32_t x = bsr((reqBytes<<1)-1);
  int32_t lgReduced = x - kLgSizeClassesPerDoubling
                      - 1; // Counteract left shift of bsr() argument.
  uint32_t lgDelta = (lgReduced < int32_t(kLgSmartSizeQuantum))
                      ? kLgSmartSizeQuantum : lgReduced;
  uint32_t delta = 1u << lgDelta;
  uint32_t deltaMask = delta - 1;
  auto const ret = (reqBytes + deltaMask) & ~deltaMask;
  assert(ret <= kMaxSmartSize + kDebugExtraSize);
  if (kDebugExtraSize != 0) {
    return std::min(ret, uint32_t(kMaxSmartSize));
  }
  return ret;
}

inline bool MemoryManager::sweeping() {
  return !TlsWrapper::isNull() && MM().m_sweeping;
}

inline void* MemoryManager::smartMallocSize(uint32_t bytes) {
  assert(bytes > 0);
  assert(bytes <= kMaxSmartSize + kDebugExtraSize);

  // Note: unlike smart_malloc, we don't track internal fragmentation
  // in the usage stats when we're going through smartMallocSize.
  m_stats.usage += bytes;

  unsigned i = smartSize2Index(bytes);
  void* p = m_freelists[i].maybePop();
  if (UNLIKELY(p == nullptr)) {
    p = slabAlloc(bytes, i);
    p = ((char*)p + kDebugExtraSize);
  }
  p = ((char*)p - kDebugExtraSize);
  assert((reinterpret_cast<uintptr_t>(p) & kSmartSizeAlignMask) == 0);
  FTRACE(3, "smartMallocSize: {} -> {}\n", bytes, p);
  return debugPostAllocate(p, bytes, debug ? smartSizeClass(bytes) : bytes);
}

inline void MemoryManager::smartFreeSize(void* ptr, uint32_t bytes) {
  assert(bytes > 0);
  assert(bytes <= kMaxSmartSize + kDebugExtraSize);
  assert((reinterpret_cast<uintptr_t>(ptr) & kSmartSizeAlignMask) == 0);

  if (UNLIKELY(m_bypassSlabAlloc)) {
    return smartFreeSizeBig(ptr, bytes);
  }
  unsigned i = smartSize2Index(bytes);
  FTRACE(3, "smartFreeSize({}, {}), freelist {}\n", ptr, bytes, i);
  debugPreFree(ptr, bytes, bytes);
  m_freelists[i].push(ptr, bytes);
  m_stats.usage -= bytes;

  FTRACE(3, "smartFreeSize: {} ({} bytes)\n", ptr, bytes);
}

ALWAYS_INLINE
void MemoryManager::smartFreeSizeBig(void* vp, size_t bytes) {
  m_stats.usage -= bytes;
  // Since we account for these direct allocations in our usage and adjust for
  // them on allocation, we also need to adjust for them negatively on free.
  m_stats.borrow(-bytes);
  FTRACE(3, "smartFreeSizeBig: {} ({} bytes)\n", vp, bytes);
  m_heap.freeBig(debugPreFree(vp, bytes, 0));
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
void* MemoryManager::objMalloc(size_t size) {
  if (LIKELY(size <= kMaxSmartSize)) return smartMallocSize(size);
  return smartMallocSizeBig<false>(size).ptr;
}

ALWAYS_INLINE
void MemoryManager::objFree(void* vp, size_t size) {
  if (LIKELY(size <= kMaxSmartSize)) return smartFreeSize(vp, size);
  smartFreeSizeBig(vp, size);
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
void* MemoryManager::smartMallocSizeLogged(uint32_t size) {
  auto const retptr = smartMallocSize(size);
  if (memory_profiling) { logAllocation(retptr, size); }
  return retptr;
}

ALWAYS_INLINE
void MemoryManager::smartFreeSizeLogged(void* p, uint32_t size) {
  if (memory_profiling) { logDeallocation(p); }
  smartFreeSize(p, size);
}

template<bool callerSavesActualSize> ALWAYS_INLINE
MemBlock MemoryManager::smartMallocSizeBigLogged(size_t size) {
  auto const block = smartMallocSizeBig<callerSavesActualSize>(size);
  if (memory_profiling) { logAllocation(block.ptr, size); }
  return block;
}

ALWAYS_INLINE
void MemoryManager::smartFreeSizeBigLogged(void* vp, size_t size) {
  if (memory_profiling) { logDeallocation(vp); }
  smartFreeSizeBig(vp, size);
}

ALWAYS_INLINE
void* MemoryManager::objMallocLogged(size_t size) {
  auto const retptr = objMalloc(size);
  if (memory_profiling) { logAllocation(retptr, size); }
  return retptr;
}

ALWAYS_INLINE
void MemoryManager::objFreeLogged(void* vp, size_t size) {
  if (memory_profiling) { logDeallocation(vp); }
  objFree(vp, size);
}

//////////////////////////////////////////////////////////////////////

inline int64_t MemoryManager::getAllocated() const {
#ifdef USE_JEMALLOC
  assert(m_allocated);
  return *m_allocated;
#else
  return 0;
#endif
}

inline int64_t MemoryManager::getDeallocated() const {
#ifdef USE_JEMALLOC
  assert(m_deallocated);
  return *m_deallocated;
#else
  return 0;
#endif
}

inline MemoryUsageStats& MemoryManager::getStatsNoRefresh() { return m_stats; }
inline MemoryUsageStats& MemoryManager::getStats() {
  refreshStats();
  return m_stats;
}

inline MemoryUsageStats MemoryManager::getStatsCopy() {
  MemoryUsageStats ret;
  ret = m_stats;
  refreshStatsImpl<false>(ret);
  return ret;
}

inline void MemoryManager::refreshStats() { refreshStatsImpl<true>(m_stats); }

inline bool MemoryManager::startStatsInterval() {
  auto ret = !m_statsIntervalActive;
  refreshStats();
  // For the reasons stated below in refreshStatsImpl, usage can potentially be
  // negative. Make sure that doesn't occur here.
  m_stats.peakIntervalUsage = std::max<int64_t>(0, m_stats.usage);
  m_stats.peakIntervalAlloc = m_stats.alloc;
  assert(m_stats.peakIntervalAlloc >= 0);
  m_statsIntervalActive = true;
  return ret;
}

inline bool MemoryManager::stopStatsInterval() {
  auto ret = m_statsIntervalActive;
  m_statsIntervalActive = false;
  m_stats.peakIntervalUsage = 0;
  m_stats.peakIntervalAlloc = 0;
  return ret;
}

inline bool MemoryManager::preAllocOOM(int64_t size) {
  if (m_couldOOM && m_stats.usage + size > m_stats.maxBytes) {
    refreshStatsHelperExceeded();
    return true;
  }
  return false;
}

inline void MemoryManager::forceOOM() {
  if (m_couldOOM) {
    refreshStatsHelperExceeded();
  }
}

inline void MemoryManager::resetExternalStats() { resetStatsImpl(false); }

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
void MemoryManager::setObjectTracking(bool val) {
  m_trackingInstances = val;
}

ALWAYS_INLINE
bool MemoryManager::getObjectTracking() {
  return m_trackingInstances;
}

}

#endif
