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

#include "hphp/util/extent-hooks.h"

#include "hphp/util/assertions.h"
#include "hphp/util/managed-arena.h"
#include <folly/portability/SysMman.h>

#if USE_JEMALLOC_EXTENT_HOOKS

namespace HPHP { namespace alloc {

extent_hooks_t* g_defaultHooks{nullptr};

// Trivial jemalloc extent hooks.  If a hook always returns true (indicating
// failure), setting it to NULL can be more efficient.

static bool
extent_commit(extent_hooks_t* /*extent_hooks*/, void* /*addr*/, size_t /*size*/,
              size_t /*offset*/, size_t /*length*/, unsigned /*arena_ind*/) {
  return false;
}

static bool
extent_purge(extent_hooks_t* /*extent_hooks*/, void* addr, size_t size,
             size_t offset, size_t length, unsigned /*arena_ind*/) {
  // This function should return false upon success, which is the case when
  // madvise returns 0.
  return madvise((char*)addr + offset, length, MADV_DONTNEED);
}

static bool extent_split(extent_hooks_t* /*extent_hooks*/, void* /*addr*/,
                         size_t /*size*/, size_t /*sizea*/, size_t /*sizeb*/,
                         bool /*comitted*/, unsigned /*arena_ind*/) {
  return false;
}

static bool extent_merge(extent_hooks_t* /*extent_hooks*/, void* /*addra*/,
                         size_t /*sizea*/, void* /*addrb*/, size_t /*sizeb*/,
                         bool /*committed*/, unsigned /*arena_ind*/) {
  return false;
}

extent_hooks_t MultiRangeExtentAllocator::s_hooks {
  MultiRangeExtentAllocator::extent_alloc,
  nullptr,                              // dalloc
  nullptr,                              // destroy
  extent_commit,
  nullptr,                              // decommit
  extent_purge,                         // purge_lazy
  nullptr,                              // purge_forced
  extent_split,
  extent_merge
};

void MultiRangeExtentAllocator::appendMapper(RangeMapper* m) {
  for (auto& p : m_mappers) {
    if (p == nullptr) {
      p = m;
      return;
    }
  }
  throw std::runtime_error{"too many mappers (check kMaxMapperCount)"};
}

size_t MultiRangeExtentAllocator::maxCapacity() const {
  size_t result = 0;
  for (auto& p : m_mappers) {
    if (p == nullptr) {
      break;
    }
    result += p->getRangeState().capacity();
  }
  return result;
}


void* MultiRangeExtentAllocator::
extent_alloc(extent_hooks_t* extent_hooks, void* addr,
             size_t size, size_t alignment, bool* zero,
             bool* commit, unsigned arena_ind) {
  assertx(extent_hooks == &MultiRangeExtentAllocator::s_hooks);
  if (addr != nullptr) {
    assertx(false);
    return nullptr;
  }
  assert(folly::isPowTwo(alignment));
  constexpr size_t size2m = (2u << 20);
  if (auto rem = size % size2m) {
    size += size2m - rem;               // round up to align at 2M
  }
  assertx(alignment <= size2m);

  auto extAlloc = GetByArenaId<MultiRangeExtentAllocator>(arena_ind);
  for (auto rangeMapper : extAlloc->m_mappers) {
    if (!rangeMapper) return nullptr;
    // RangeMapper::addMappingImpl() holds the lock on RangeState when adding
    // new mappings, so no additional locking is needed here.
    if (auto addr = rangeMapper->alloc(size)) {
      extAlloc->m_allocatedSize.fetch_add(size, std::memory_order_relaxed);
      return addr;
    }
  }
  not_reached();
  return nullptr;
}

extent_hooks_t RangeFallbackExtentAllocator::s_hooks {
  RangeFallbackExtentAllocator::extent_alloc,
  RangeFallbackExtentAllocator::extent_dalloc,
  RangeFallbackExtentAllocator::extent_destroy,
  RangeFallbackExtentAllocator::extent_commit,
  RangeFallbackExtentAllocator::extent_decommit,
  RangeFallbackExtentAllocator::extent_purge,
  nullptr,                              // purge_forced
  extent_split,                         // always split
  extent_merge                          // always merge
};


void* RangeFallbackExtentAllocator::
extent_alloc(extent_hooks_t* extent_hooks, void* addr,
             size_t size, size_t alignment, bool* zero,
             bool* commit, unsigned arena_ind) {
  assertx(extent_hooks == &RangeFallbackExtentAllocator::s_hooks);
  assertx(arena_ind != 0);
  constexpr size_t kAlign = 1u << 16;
  if (addr != nullptr || alignment > kAlign || (size % kAlign)) {
    // Let the default hook handle weird cases.
    return g_defaultHooks->alloc(extent_hooks, addr, size, alignment,
                                 zero, commit, arena_ind);
  }
  auto extAlloc = GetByArenaId<RangeFallbackExtentAllocator>(arena_ind);
  if (auto addr = extAlloc->getLowMapper()->alloc(size)) return addr;
  return g_defaultHooks->alloc(extent_hooks, addr, size, alignment,
                               zero, commit, arena_ind);
}

bool RangeFallbackExtentAllocator::
extent_dalloc(extent_hooks_t* extent_hooks, void* addr, size_t size,
              bool committed, unsigned arena_ind) {
  auto extAlloc = GetByArenaId<RangeFallbackExtentAllocator>(arena_ind);
  if (extAlloc->inRange(addr)) return true;
  return g_defaultHooks->dalloc(extent_hooks, addr, size, committed, arena_ind);
}

void RangeFallbackExtentAllocator::
extent_destroy(extent_hooks_t* extent_hooks, void* addr, size_t size,
               bool committed, unsigned arena_ind) {
  auto extAlloc = GetByArenaId<RangeFallbackExtentAllocator>(arena_ind);
  if (extAlloc->inRange(addr)) return;
  return g_defaultHooks->destroy(extent_hooks, addr, size,
                                 committed, arena_ind);
}

bool RangeFallbackExtentAllocator::
extent_commit(extent_hooks_t* extent_hooks, void* addr, size_t size,
              size_t offset, size_t length, unsigned arena_ind) {
  auto extAlloc = GetByArenaId<RangeFallbackExtentAllocator>(arena_ind);
  if (extAlloc->inRange(addr)) return false;
  return g_defaultHooks->commit(extent_hooks, addr, size,
                                offset, length, arena_ind);
}

bool RangeFallbackExtentAllocator::
extent_decommit(extent_hooks_t* extent_hooks, void* addr, size_t size,
                size_t offset, size_t length, unsigned arena_ind) {
  auto extAlloc = GetByArenaId<RangeFallbackExtentAllocator>(arena_ind);
  if (extAlloc->inRange(addr)) return true; // never decommit the fixed range
  return g_defaultHooks->decommit(extent_hooks, addr, size, offset,
                                  length, arena_ind);
}

bool RangeFallbackExtentAllocator::
extent_purge(extent_hooks_t* extent_hooks, void* addr, size_t size,
             size_t offset, size_t length, unsigned arena_ind) {
  auto extAlloc = GetByArenaId<RangeFallbackExtentAllocator>(arena_ind);
  if (extAlloc->inRange(addr)) return true; // never purge
  if (g_defaultHooks->purge_lazy == nullptr) return true;
  return g_defaultHooks->purge_lazy(extent_hooks, addr, size, offset,
                                    length, arena_ind);
}

}}

#endif
