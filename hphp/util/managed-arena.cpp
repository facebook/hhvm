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

#include "hphp/util/managed-arena.h"

#ifdef USE_JEMALLOC_CUSTOM_HOOKS

#include "hphp/util/hugetlb.h"
#include "hphp/util/numa.h"

#include <cinttypes>
#include <stdexcept>
#include <thread>

namespace HPHP {

#ifdef USE_JEMALLOC_CHUNK_HOOKS
// jemalloc chunk hooks
static bool chunk_dalloc(void* chunk, size_t size,
                         bool committed, unsigned arena_ind) {
  return true;
}

static bool chunk_commit(void* chunk, size_t size, size_t offset,
                         size_t length, unsigned arena_ind) {
  return false;
}

static bool chunk_decommit(void* chunk, size_t size, size_t offset,
                           size_t length, unsigned arena_ind) {
  return true;
}

static bool chunk_purge(void* chunk, size_t size, size_t offset,
                        size_t length, unsigned arena_ind) {
  return true;
}

static bool chunk_split(void* chunk, size_t size, size_t sizea, size_t sizeb,
                        bool comitted, unsigned arena_ind) {
  return false;
}

static bool chunk_merge(void* chunka, size_t sizea, void* chunkb, size_t sizeb,
                        bool committed, unsigned arena_ind) {
  return false;
}
#elif defined USE_JEMALLOC_EXTENT_HOOKS
// jemalloc extent hooks
static bool
extent_dalloc(extent_hooks_t* /*extent_hooks*/, void* /*addr*/, size_t /*size*/,
              bool /*committed*/, unsigned /*arena_ind*/) {
  return true;
}

static void
extent_destroy(extent_hooks_t* /*extent_hooks*/, void* /*addr*/,
               size_t /*size*/, bool /*committed*/, unsigned /*arena_ind*/) {
  return;
}

static bool
extent_commit(extent_hooks_t* /*extent_hooks*/, void* /*addr*/, size_t /*size*/,
              size_t /*offset*/, size_t /*length*/, unsigned /*arena_ind*/) {
  return false;
}

static bool extent_decommit(extent_hooks_t* /*extent_hooks*/, void* /*addr*/,
                            size_t /*size*/, size_t /*offset*/,
                            size_t /*length*/, unsigned /*arena_ind*/) {
  return true;
}

static bool
extent_purge(extent_hooks_t* /*extent_hooks*/, void* /*addr*/, size_t /*size*/,
             size_t /*offset*/, size_t /*length*/, unsigned /*arena_ind*/) {
  return true;
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

static extent_hooks_t custom_extent_hooks {
  ManagedArena::extent_alloc,
  extent_dalloc,
  extent_destroy,
  extent_commit,
  extent_decommit,
  extent_purge, // purge_lazy
  extent_purge, // purge_forced
  extent_split,
  extent_merge
};
#else
# error "Missing jemalloc custom hook definitions."
#endif

//////////////////////////////////////////////////////////////////////

std::atomic_bool ManagedArena::s_lock;
ManagedArena* ManagedArena::s_arenas[MAX_HUGE_ARENA_COUNT];

ManagedArena::ManagedArena(void* base, size_t maxCap,
                           int nextNode /* = -1 */,
                           int nodeMask /* = -1 */)
  : m_base(static_cast<char*>(base))
  , m_maxCapacity(maxCap)
  , m_nextNode(nextNode)
  , m_nodeMask(nodeMask)
{
  assert(reinterpret_cast<uintptr_t>(base) % size1g == 0);
  assert(maxCap % size1g == 0);

  // Create a special arena to manage this piece of memory.
  size_t sz = sizeof(m_arenaId);
  if (mallctl(JEMALLOC_NEW_ARENA_CMD, &m_arenaId, &sz, nullptr, 0) != 0) {
    throw std::runtime_error{"error when creating new arena."};
  }
  if (m_arenaId >= MAX_HUGE_ARENA_COUNT) {
    always_assert(false);               // testing
    throw std::runtime_error{"too many arenas, check MAX_HUGE_ARENA_COUNT"};
  }
#ifdef USE_JEMALLOC_CHUNK_HOOKS
  chunk_hooks_t hooks {
    ManagedArena::chunk_alloc,
    chunk_dalloc,
    chunk_commit,
    chunk_decommit,
    chunk_purge,
    chunk_split,
    chunk_merge
  };
  char command[32];
  std::snprintf(command, sizeof(command), "arena.%d.chunk_hooks", m_arenaId);
  sz = sizeof(hooks);
  if (mallctl(command, nullptr, nullptr, &hooks, sz) != 0) {
    throw std::runtime_error("error in setting chunk hooks");
  }
#else
  char command[32];
  std::snprintf(command, sizeof(command), "arena.%d.extent_hooks", m_arenaId);
  extent_hooks_t *hooks_ptr = &custom_extent_hooks;
  sz = sizeof(hooks_ptr);
  if (mallctl(command, nullptr, nullptr, &hooks_ptr, sz) != 0) {
    throw std::runtime_error("error in setting extent hooks");
  }
#endif
  s_arenas[m_arenaId] = this;
}

#ifdef USE_JEMALLOC_CHUNK_HOOKS
void* ManagedArena::chunk_alloc(void* addr, size_t size,
                                size_t alignment, bool* zero, bool* commit,
                                unsigned arena_ind) {
#else
void* ManagedArena::extent_alloc(extent_hooks_t* /*extent_hooks*/, void* addr,
                                 size_t size, size_t alignment, bool* zero,
                                 bool* commit, unsigned arena_ind) {
#endif
  if (addr != nullptr) return nullptr;
  if (size > size1g) return nullptr;

  ManagedArena* arena = s_arenas[arena_ind];
  if (arena == nullptr) return nullptr;

  // Just in case size is too big (negative)
  if (size > arena->m_maxCapacity) return nullptr;

  assert(folly::isPowTwo(alignment));
  auto const mask = alignment - 1;
  size_t startOffset;
  int failCount = 0;
  do {
    size_t oldSize = arena->m_size.load(std::memory_order_relaxed);
    startOffset = (oldSize + size + mask) & ~mask;
    size_t newSize = startOffset;
    if (newSize > arena->m_maxCapacity) return nullptr;
    if (newSize > arena->m_currCapacity) {
      // Check if any huge page is available.
      HugePageInfo info = get_huge1g_info();
      if (info.nr_hugepages == num_1g_pages()) {
        // We've got all possible pages, don't try to grab more in future.
        arena->m_maxCapacity = arena->m_currCapacity;
        return nullptr;
      }
      if (info.free_hugepages <= 0) {
        // We haven't got all possible pages reserved, but someone else is
        // holding some of the pages, so we cannot get them now.  We can still
        // try again later.
        return nullptr;
      }

      bool expected = false;
      if (!s_lock.compare_exchange_weak(expected, true)) {
        // Someone else has the lock.  Don't wait and risk massive thread
        // contention here.  Just fail and caller will fall back to other
        // arenas.
        return nullptr;
      }
      // We have the lock, remember to unlock.
      SCOPE_EXIT { s_lock.store(false, std::memory_order_relaxed); };

      // Before we do anything, check if someone already added a page (after we
      // realized a new page is needed but before we tried to grab the lock).
      if (newSize <= arena->m_currCapacity) {
        continue;
      }

      // OK. It is our duty to add a new page.
      char* newPageStart = arena->m_base - arena->m_currCapacity - size1g;
      assert(reinterpret_cast<uintptr_t>(newPageStart) % size1g == 0);

      if (arena->m_nextNode < 0) {
        if (mmap_1g(newPageStart)) {
          arena->m_currCapacity += size1g;
        } else {
          // I thought a page was available, hmmm..  Maybe some other process
          // got the page?
          return nullptr;
        }
      } else {
#ifdef HAVE_NUMA
        if (arena->m_nodeMask == 0) return nullptr;
        auto targetNode = arena->m_nextNode & numa_node_mask;
        while ((arena->m_nodeMask & (1 << targetNode)) == 0) {
          targetNode = (targetNode + 1) & numa_node_mask;
        }
        arena->m_nextNode = (targetNode + 1) & numa_node_mask;
        assert(arena->m_nodeMask & (1 << targetNode));
        if (mmap_1g(newPageStart, targetNode)) {
          arena->m_currCapacity += size1g;
        } else if (++failCount == numa_num_nodes) {
          // We tried on all the nodes, but couldn't get a page, even
          // though a page appeared available.
          return nullptr;
        }
#else
        // Shouldn't specify next node if NUMA is unavailable.
        return nullptr;
#endif
      }
      // We have successfully added a page to the arena, or have failed for a
      // specific node, move to the next node and retry.
      continue;
    }
    if (arena->m_size.compare_exchange_weak(oldSize, newSize)) {
      break;
    }
  } while (true);
  *zero = true;
  *commit = true;
  return arena->m_base - startOffset;
}

std::string ManagedArena::reportStats() {
  std::string result;
  for (unsigned i = 1; i < MAX_HUGE_ARENA_COUNT; ++i) {
    if (auto arena = s_arenas[i]) {
      assert(arena->m_arenaId == i);
      char buffer[128];
      std::snprintf(buffer, sizeof(buffer),
                    "Arena %d on NUMA mask %d: capacity %zd, "
                    "max_capacity %zd, used %zd\n",
                    i,
                    arena->m_nodeMask,
                    arena->m_currCapacity,
                    arena->m_maxCapacity,
                    arena->m_size.load(std::memory_order_relaxed));
      result += buffer;
    }
  }
  return result;
}

}
#endif // USE_JEMALLOC_CUSTOM_HOOKS
