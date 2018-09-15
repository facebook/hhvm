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
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/safe-cast.h"
#include "hphp/util/trace.h"

namespace HPHP {

TRACE_SET_MOD(mm);

void SparseHeap::threadInit() {
  m_slabManager = SlabManager::get(s_numaNode);
}

void SparseHeap::reset() {
  TRACE(1, "heap-id %lu SparseHeap-reset: pooled_slabs %lu bigs %lu\n",
        tl_heap_id, m_pooled_slabs.size(), m_bigs.countBlocks());
#if !FOLLY_SANITIZE
  // trash fill is redundant with ASAN
  if (RuntimeOption::EvalTrashFillOnRequestExit) {
    m_bigs.iterate([&](HeapObject* h, size_t size) {
      memset(h, kSmallFreeFill, size);
    });
  }
#endif
  auto const do_free = [](void* ptr, size_t size) {
#ifdef USE_JEMALLOC
#if JEMALLOC_VERSION_MAJOR >= 4
    sdallocx(ptr, size, 0);
#else
    dallocx(ptr, 0);
#endif
#else
    free(ptr);
#endif
  };
  TaggedSlabList pooledSlabs;
  void* pooledSlabTail = nullptr;
  for (auto& slab : m_pooled_slabs) {
    if (!pooledSlabTail) pooledSlabTail = slab.ptr;
    pooledSlabs.push_front<true>(slab.ptr, slab.version);
    m_bigs.erase(slab.ptr);
  }
  if (pooledSlabTail) {
    m_slabManager->merge(pooledSlabs.head(), pooledSlabTail);
  }
  m_pooled_slabs.clear();
  m_hugeBytes = 0;
  m_bigs.iterate([&](HeapObject* h, size_t size) {
    do_free(h, size);
  });
  m_bigs.clear();
  m_slab_range = {nullptr, 0};
}

void SparseHeap::flush() {
  assertx(empty());
  m_pooled_slabs = std::vector<SlabInfo>{};
  m_bigs.clear();
  m_hugeBytes = 0;
}

HeapObject* SparseHeap::allocSlab(MemoryUsageStats& stats) {
  auto finish = [&](void* p) {
    // expand m_slab_range to include this new slab
    if (!m_slab_range.size) {
      m_slab_range = {p, kSlabSize};
    } else {
      auto min = std::min(m_slab_range.ptr, p);
      auto max = std::max((char*)p + kSlabSize,
                          (char*)m_slab_range.ptr + m_slab_range.size);
      m_slab_range = {min, size_t((char*)max - (char*)min)};
    }
    return static_cast<HeapObject*>(p);
  };

  if (m_slabManager && m_hugeBytes < RuntimeOption::RequestHugeMaxBytes) {
    if (auto slab = m_slabManager->tryAlloc()) {
      stats.mmap_volume += kSlabSize;
      stats.mmap_cap += kSlabSize;
      stats.peakCap = std::max(stats.peakCap, stats.capacity());
      m_pooled_slabs.emplace_back(slab.ptr(), kSlabSize, slab.tag());
      m_bigs.insert((HeapObject*)slab.ptr(), kSlabSize);
      m_hugeBytes += kSlabSize;
      return finish(slab.ptr());
    }
  }
#ifdef USE_JEMALLOC
  void* slab = mallocx(kSlabSize, MALLOCX_ALIGN(kSlabAlign));
  auto usable = sallocx(slab, 0);
#else
  auto slab = safe_aligned_alloc(kSlabAlign, kSlabSize);
  auto usable = kSlabSize;
#endif
  m_bigs.insert((HeapObject*)slab, kSlabSize);
  stats.malloc_cap += usable;
  stats.peakCap = std::max(stats.peakCap, stats.capacity());
  return finish(slab);
}

void* SparseHeap::allocBig(size_t bytes, bool zero, MemoryUsageStats& stats) {
#ifdef USE_JEMALLOC
  int flags = zero ? MALLOCX_ZERO : 0;
  auto n = static_cast<HeapObject*>(mallocx(bytes, flags));
  auto cap = sallocx(n, 0);
#else
  auto n = static_cast<MallocNode*>(
    zero ? safe_calloc(1, bytes) : safe_malloc(bytes)
  );
  auto cap = malloc_usable_size(n);
  if (cap % kSmallSizeAlign != 0) {
    // cap==bytes is legal, but RadixMap requires at least kSmallSizeAlign
    // for tracking purposes. In this mode we just call free(), so it's safe
    // to slightly overestimate the block size
    cap += kSmallSizeAlign - cap % kSmallSizeAlign;
  }
#endif
  m_bigs.insert(n, cap);
  stats.mm_udebt -= cap;
  stats.malloc_cap += cap;
  return n;
}

bool SparseHeap::contains(const void* ptr) const {
  return m_bigs.find(ptr).ptr != nullptr;
}

void SparseHeap::freeBig(void* ptr, MemoryUsageStats& stats) {
  // Since we account for these direct allocations in our usage and adjust for
  // them on allocation, we also need to adjust for them negatively on free.
  auto cap = m_bigs.erase(ptr);
  stats.mm_freed += cap;
  stats.malloc_cap -= cap;
#ifdef USE_JEMALLOC
  assertx(nallocx(cap, 0) == sallocx(ptr, 0));
#if JEMALLOC_VERSION_MAJOR >= 4
  sdallocx(ptr, cap, 0);
#else
  dallocx(ptr, 0);
#endif
#else
  free(ptr);
#endif
}

void* SparseHeap::resizeBig(void* ptr, size_t new_size,
                            MemoryUsageStats& stats) {
  auto old = static_cast<HeapObject*>(ptr);
  auto old_cap = m_bigs.get(old);
#ifdef USE_JEMALLOC
  auto const newNode = static_cast<HeapObject*>(
    rallocx(ptr, new_size, 0)
  );
  auto new_cap = sallocx(newNode, 0);
#else
  auto const newNode = static_cast<HeapObject*>(
    safe_realloc(ptr, new_size)
  );
  auto new_cap = malloc_usable_size(newNode);
  if (new_cap % kSmallSizeAlign != 0) {
    // adjust to satisfy RadixMap (see justification in allocBig())
    new_cap += kSmallSizeAlign - new_cap % kSmallSizeAlign;
  }
#endif
  if (newNode != old || new_cap != old_cap) {
    m_bigs.erase(old);
    m_bigs.insert(newNode, new_cap);
  }
  stats.mm_udebt -= new_cap - old_cap;
  stats.malloc_cap += new_cap - old_cap;
  return newNode;
}

}
