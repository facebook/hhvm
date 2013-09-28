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

#ifndef incl_HPHP_MEMORY_MANAGER_INL_H
#define incl_HPHP_MEMORY_MANAGER_INL_H

#include <limits>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

static_assert(
  MemoryManager::kMaxSmartSize <= std::numeric_limits<uint32_t>::max(),
  "Size-specified smart malloc functions assume this"
);

//////////////////////////////////////////////////////////////////////

template<class SizeT> ALWAYS_INLINE
SizeT MemoryManager::debugAddExtra(SizeT sz) {
  if (!debug) return sz;
  return sz + sizeof(DebugHeader);
}

template<class SizeT> ALWAYS_INLINE
SizeT MemoryManager::debugRemoveExtra(SizeT sz) {
  if (!debug) return sz;
  return sz - sizeof(DebugHeader);
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

inline uint32_t MemoryManager::smartSizeClass(uint32_t reqBytes) {
  assert(reqBytes <= kMaxSmartSize);
  auto const ret = (reqBytes + kSmartSizeMask) & ~kSmartSizeMask;
  assert(ret <= kMaxSmartSize);
  return ret;
}

inline void* MemoryManager::smartMallocSize(uint32_t bytes) {
  assert(bytes > 0);
  assert(bytes <= kMaxSmartSize);

  // Note: unlike smart_malloc, we don't track internal fragmentation
  // in the usage stats when we're going through smartMallocSize.
  m_stats.usage += bytes;

  unsigned i = (bytes - 1) >> kLgSizeQuantum;
  assert(i < kNumSizes);
  void* p = m_sizeUntrackedFree[i].maybePop();
  if (UNLIKELY(p == nullptr)) {
    p = slabAlloc(debugAddExtra(MemoryManager::smartSizeClass(bytes)));
  }
  assert(reinterpret_cast<uintptr_t>(p) % 16 == 0);

  FTRACE(1, "smartMallocSize: {} -> {}\n", bytes, p);
  return debugPostAllocate(p, bytes, bytes);
}

inline void MemoryManager::smartFreeSize(void* ptr, uint32_t bytes) {
  assert(bytes > 0);
  assert(bytes <= kMaxSmartSize);
  assert(reinterpret_cast<uintptr_t>(ptr) % 16 == 0);

  unsigned i = (bytes - 1) >> kLgSizeQuantum;
  assert(i < kNumSizes);
  m_sizeUntrackedFree[i].push(debugPreFree(ptr, bytes, bytes));
  m_stats.usage -= bytes;

  FTRACE(1, "smartFreeSize: {} ({} bytes)\n", ptr, bytes);
}

ALWAYS_INLINE
std::pair<void*,size_t> MemoryManager::smartMallocSizeBig(size_t bytes) {
#ifdef USE_JEMALLOC
  void* ptr;
  size_t sz;
  auto const retptr = smartMallocSizeBigHelper(ptr, sz, bytes);
  FTRACE(1, "smartMallocBig: {} ({} requested, {} usable)\n",
         retptr, bytes, sz);
  return std::make_pair(retptr, sz);
#else
  m_stats.usage += bytes;
  // TODO(#2831116): we only add sizeof(SmallNode) so smartMallocBig
  // can subtract it.
  auto const ret = smartMallocBig(debugAddExtra(bytes + sizeof(SmallNode)));
  FTRACE(1, "smartMallocBig: {} ({} bytes)\n", ret, bytes);
  return std::make_pair(debugPostAllocate(ret, bytes, bytes), bytes);
#endif
}

ALWAYS_INLINE
void MemoryManager::smartFreeSizeBig(void* vp, size_t bytes) {
  m_stats.usage -= bytes;
  FTRACE(1, "smartFreeBig: {} ({} bytes)\n", vp, bytes);
  return smartFreeBig(static_cast<SweepNode*>(debugPreFree(vp, bytes, 0)) - 1);
}

//////////////////////////////////////////////////////////////////////

}

#endif
