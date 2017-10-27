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
#include "hphp/util/trace.h"

namespace HPHP {

TRACE_SET_MOD(mm);

SparseHeap::~SparseHeap() {
  reset();
}

void SparseHeap::reset() {
  TRACE(1, "heap-id %lu SparseHeap-reset: slabs %lu bigs %lu\n",
        tl_heap_id, m_slabs.size(), m_bigs.size());
  auto const do_free = [&](void* ptr, size_t size) {
    if (RuntimeOption::EvalTrashFillOnRequestExit) {
      memset(ptr, kSmallFreeFill, size);
    }
#ifdef USE_JEMALLOC
    dallocx(ptr, 0);
#else
    free(ptr);
#endif
  };
  for (auto slab : m_slabs) do_free(slab.ptr, slab.size);
  m_slabs.clear();
  for (auto n : m_bigs) do_free(n, n->nbytes);
  m_bigs.clear();
}

void SparseHeap::flush() {
  assert(empty());
  m_slabs = std::vector<MemBlock>{};
  m_bigs = std::vector<MallocNode*>{};
}

MemBlock SparseHeap::allocSlab(size_t size, MemoryUsageStats& stats) {
#ifdef USE_JEMALLOC
  void* slab = mallocx(size, 0);
  auto usable = sallocx(slab, 0);
#else
  void* slab = safe_malloc(size);
  auto usable = size;
#endif
  m_slabs.push_back({slab, size});
  stats.malloc_cap += usable;
  stats.peakCap = std::max(stats.peakCap, stats.capacity());
  return {slab, usable};
}

void SparseHeap::enlist(MallocNode* n, HeaderKind kind,
                        size_t size, type_scan::Index tyindex) {
  n->initHeader_32_16(kind, m_bigs.size(), tyindex);
  n->nbytes = size;
  m_bigs.push_back(n);
}

MemBlock SparseHeap::allocBig(size_t bytes, HeaderKind kind,
                              type_scan::Index tyindex,
                              MemoryUsageStats& stats) {
#ifdef USE_JEMALLOC
  auto n = static_cast<MallocNode*>(mallocx(bytes + sizeof(MallocNode), 0));
  auto cap = sallocx(n, 0);
#else
  auto cap = bytes + sizeof(MallocNode);
  auto n = static_cast<MallocNode*>(safe_malloc(cap));
#endif
  enlist(n, kind, cap, tyindex);
  stats.mmUsage += cap;
  stats.malloc_cap += cap;
  return {n + 1, cap - sizeof(MallocNode)};
}

MemBlock SparseHeap::callocBig(size_t nbytes, HeaderKind kind,
                               type_scan::Index tyindex,
                               MemoryUsageStats& stats) {
#ifdef USE_JEMALLOC
  auto n = static_cast<MallocNode*>(
      mallocx(nbytes + sizeof(MallocNode), MALLOCX_ZERO)
  );
  auto cap = sallocx(n, 0);
#else
  auto cap = nbytes + sizeof(MallocNode);
  auto const n = static_cast<MallocNode*>(safe_calloc(cap, 1));
#endif
  enlist(n, kind, cap, tyindex);
  stats.mmUsage += cap;
  stats.malloc_cap += cap;
  return {n + 1, cap - sizeof(MallocNode)};
}

bool SparseHeap::contains(void* ptr) const {
  auto const ptrInt = reinterpret_cast<uintptr_t>(ptr);
  auto it = std::find_if(std::begin(m_slabs), std::end(m_slabs),
    [&] (MemBlock slab) {
      auto const baseInt = reinterpret_cast<uintptr_t>(slab.ptr);
      return ptrInt >= baseInt && ptrInt < baseInt + slab.size;
    }
  );
  return it != std::end(m_slabs);
}

void SparseHeap::freeBig(void* ptr) {
  auto n = static_cast<MallocNode*>(ptr) - 1;
  auto i = n->index();
  auto last = m_bigs.back();
  last->index() = i;
  m_bigs[i] = last;
  m_bigs.pop_back();
#ifdef USE_JEMALLOC
  dallocx(n, 0);
#else
  free(n);
#endif
}

MemBlock SparseHeap::resizeBig(void* ptr, size_t newsize,
                               MemoryUsageStats& stats) {
  // Since we don't know how big it is (i.e. how much data we should memcpy),
  // we have no choice but to ask malloc to realloc for us.
  auto const n = static_cast<MallocNode*>(ptr) - 1;
  auto old_size = n->nbytes;
#ifdef USE_JEMALLOC
  auto const newNode = static_cast<MallocNode*>(
    rallocx(n, newsize + sizeof(MallocNode), 0)
  );
  newNode->nbytes = sallocx(newNode, 0);
#else
  auto const newNode = static_cast<MallocNode*>(
    safe_realloc(n, newsize + sizeof(MallocNode))
  );
  newNode->nbytes = newsize + sizeof(MallocNode);
#endif
  if (newNode != n) {
    m_bigs[newNode->index()] = newNode;
  }
  stats.mmUsage += newsize - old_size;
  stats.malloc_cap += newsize - old_size;
  return {newNode + 1, newNode->nbytes - sizeof(MallocNode)};
}

void SparseHeap::sort() {
  std::sort(std::begin(m_slabs), std::end(m_slabs),
    [] (const MemBlock& l, const MemBlock& r) {
      assertx(static_cast<char*>(l.ptr) + l.size <= r.ptr ||
              static_cast<char*>(r.ptr) + r.size <= l.ptr);
      return l.ptr < r.ptr;
    }
  );
  std::sort(std::begin(m_bigs), std::end(m_bigs));
  for (size_t i = 0, n = m_bigs.size(); i < n; ++i) {
    m_bigs[i]->index() = i;
  }
}

/*
 * To find `p', we sort the slabs, bisect them, then iterate the slab
 * containing `p'.  If there is no such slab, we bisect the bigs to try to find
 * a big containing `p'.
 *
 * If that fails, we return nullptr.
 */
HeapObject* SparseHeap::find(const void* p) {
  sort();
  auto const slab = std::lower_bound(
    std::begin(m_slabs), std::end(m_slabs), p,
    [] (const MemBlock& slab, const void* p) {
      return static_cast<const char*>(slab.ptr) + slab.size <= p;
    }
  );

  if (slab != std::end(m_slabs) && slab->ptr <= p) {
    // std::lower_bound() finds the first slab that is not less than `p'.  By
    // our comparison predicate, a slab is less than `p' iff its entire range
    // is below `p', so if the returned slab's start address is <= `p', then
    // the slab must contain `p'.  Within the slab, we just do a linear search.
    auto const slab_end = static_cast<char*>(slab->ptr) + slab->size;
    auto h = reinterpret_cast<char*>(slab->ptr);
    while (h < slab_end) {
      auto const hdr = reinterpret_cast<HeapObject*>(h);
      auto const size = allocSize(hdr);
      if (p < h + size) return hdr;
      h += size;
    }
    // We know `p' is in the slab, so it must belong to one of the headers.
    always_assert(false);
  }

  auto const big = std::lower_bound(
    std::begin(m_bigs), std::end(m_bigs), p,
    [] (const MallocNode* big, const void* p) {
      return reinterpret_cast<const char*>(big) + big->nbytes <= p;
    }
  );

  if (big != std::end(m_bigs) && *big <= p) {
    auto const hdr = reinterpret_cast<HeapObject*>(*big);
    if (hdr->kind() != HeaderKind::BigObj) {
      // `p' is part of the MallocNode.
      return hdr;
    }
    auto const sub = reinterpret_cast<HeapObject*>(*big + 1);
    return p >= sub ? sub : hdr;
  }
  return nullptr;
}

}
