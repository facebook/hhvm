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

#include <folly/portability/SysMman.h>

#if USE_JEMALLOC_EXTENT_HOOKS

namespace HPHP { namespace alloc {

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

extent_hooks_t BumpExtentAllocator::s_hooks {
  BumpExtentAllocator::extent_alloc,
  nullptr,                              // dalloc
  nullptr,                              // destroy
  extent_commit,
  nullptr,                              // decommit
  extent_purge,                         // purge_lazy
  nullptr,                              // purge_forced
  extent_split,
  extent_merge
};

BumpExtentAllocator::BumpExtentAllocator(uintptr_t highAddr, size_t maxCap,
                                         LockPolicy p, BumpMapper* mapper)
  : BumpAllocState(highAddr, maxCap, p)
  , m_mapper(mapper) {
  if (highAddr == 0 || maxCap == 0) return;

  auto constexpr mask = (1u << 21) - 1;
  if ((m_base | maxCap) & mask) {
    throw std::invalid_argument{"address and capacity not multiple of 2M"};
  }
}

void* BumpExtentAllocator::
extent_alloc(extent_hooks_t* extent_hooks, void* addr,
             size_t size, size_t alignment, bool* zero,
             bool* commit, unsigned arena_ind) {
  assert(extent_hooks == &BumpExtentAllocator::s_hooks);
  if (addr != nullptr) return nullptr;
  assert(folly::isPowTwo(alignment));
  const uintptr_t mask = ~(alignment - 1);

  BumpExtentAllocator* extAlloc = GetByArenaId<BumpExtentAllocator>(arena_ind);
  do {
    size_t oldSize = extAlloc->m_size.load(std::memory_order_relaxed);
    uintptr_t ret = (extAlloc->m_base + oldSize + alignment - 1) & mask;
    size_t newSize = ret + size - extAlloc->m_base;
    if (newSize <= extAlloc->m_currCapacity) {
      // Looks like existing capacity is enough.
      if (extAlloc->m_size.compare_exchange_weak(oldSize, newSize)) {
        *zero = true;
        *commit = true;
        return reinterpret_cast<void*>(ret);
      }
    } else {
      if (newSize > extAlloc->m_maxCapacity) return nullptr;
      if (extAlloc->m_lockPolicy != LockPolicy::Blocking) {
        if (!extAlloc->m_mutex.try_lock()) {
          return nullptr;
        }
      } else {
        extAlloc->m_mutex.lock();
      }
      bool succ = extAlloc->m_mapper &&
        extAlloc->m_mapper->addMapping(*extAlloc, newSize);
      extAlloc->m_mutex.unlock();
      if (!succ) return nullptr;
    }
  } while (true);
  not_reached();
}

}}

#endif
