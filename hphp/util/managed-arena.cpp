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

#ifdef USE_JEMALLOC_EXTENT_HOOKS

#include "hphp/util/hugetlb.h"
#include "hphp/util/numa.h"

#include <cinttypes>
#include <stdexcept>
#include <thread>

namespace HPHP {

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

//////////////////////////////////////////////////////////////////////

std::atomic_bool ManagedArena::s_lock;
ManagedArena* ManagedArena::s_arenas[MAX_HUGE_ARENA_COUNT];

ManagedArena::ManagedArena(void* base, size_t maxCap, int nextNode /* = -1 */)
  : m_base(static_cast<char*>(base))
  , m_maxCapacity(maxCap)
  , m_nextNode(nextNode)
{
  assert(reinterpret_cast<uintptr_t>(base) % size1g == 0);
  assert(maxCap % size1g == 0);

  // Create a special arena to manage this piece of memory.
  size_t sz = sizeof(m_arenaId);
  if (mallctl(JEMALLOC_NEW_ARENA_CMD, &m_arenaId, &sz, nullptr, 0)) {
    throw std::runtime_error{"error when creating new arena."};
  }
  if (m_arenaId >= MAX_HUGE_ARENA_COUNT) {
    throw std::runtime_error{"too many arenas, check MAX_HUGE_ARENA_COUNT"};
  }
  char command[32];
  std::snprintf(command, sizeof(command), "arena.%d.extent_hooks", m_arenaId);
  extent_hooks_t *hooks_ptr = &custom_extent_hooks;
  sz = sizeof(hooks_ptr);
  if (mallctl(command, nullptr, nullptr, &hooks_ptr, sz)) {
    throw std::runtime_error{"error in setting extent hooks"};
  }

  // Disable purging in this arena, as we won't be able to return the memory to
  // the system anyway.
  ssize_t decay_time = -1;
  std::snprintf(command, sizeof(command),
                "arena.%d.dirty_decay_ms", m_arenaId);
  if (mallctl(command, nullptr, nullptr, &decay_time, sizeof(decay_time))) {
    throw std::runtime_error{"error when turning off decaying"};
  }
  std::snprintf(command, sizeof(command),
                "arena.%d.muzzy_decay_ms", m_arenaId);
  if (mallctl(command, nullptr, nullptr, &decay_time, sizeof(decay_time))) {
    throw std::runtime_error{"error when turning off decaying"};
  }
  s_arenas[m_arenaId] = this;
}

bool ManagedArena::tryGrab1G(size_t newSize) {
  // Must hold `s_lock` when calling this.
  assertx(s_lock.load(std::memory_order_relaxed));

  // Before we do anything, check if someone already added a page (after we
  // realized the need for a new page but before we got `s_lock`).
  if (newSize <= m_currCapacity) return true;

  int targetNode = -1; // in `mmap_1g()`, passing -1 as node indicating no NUMA
  if (m_nextNode.load(std::memory_order_relaxed) >= 0) {
    // Non-negative `m_nextNode` indicates that we do care about NUMA.
    targetNode = next_numa_node(m_nextNode);
    assertx(targetNode >=0 && numa_node_allowed(targetNode));
  }

  char* newPageStart = m_base - m_currCapacity - size1g;
  assertx(reinterpret_cast<uintptr_t>(newPageStart) % size1g == 0);

  if (mmap_1g(newPageStart, targetNode)) {
    m_currCapacity += size1g;
    return true;
  }
  return false;
}

void* ManagedArena::extent_alloc(extent_hooks_t* /*extent_hooks*/, void* addr,
                                 size_t size, size_t alignment, bool* zero,
                                 bool* commit, unsigned arena_ind) {
  if (addr != nullptr) return nullptr;
  if (size > size1g) return nullptr;

  ManagedArena* arena = s_arenas[arena_ind];
  if (arena == nullptr) return nullptr;

  assert(folly::isPowTwo(alignment));
  auto const mask = alignment - 1;
  size_t startOffset;
  uint32_t failCount = 0;
  do {
    size_t oldSize = arena->m_size.load(std::memory_order_relaxed);
    startOffset = (oldSize + size + mask) & ~mask;
    size_t newSize = startOffset;

    if (newSize > arena->m_currCapacity) {
      if (arena->m_outOf1GPages) {
        // TODO (T11400255): we plan add some normal pages here if we run out of
        // huge pages.
        return nullptr;
      }

      if (arena->m_currCapacity >= arena->m_maxCapacity) {
        arena->m_outOf1GPages = true;
        continue;
      }
      HugePageInfo info = get_huge1g_info();
      if (info.nr_hugepages == num_1g_pages()) {
        arena->m_outOf1GPages = true;
        continue;
      }
      if (info.free_hugepages <= 0) {
        // We haven't got all huge pages we want, but someone else is holding
        // some of the pages, so we cannot get them now.  We can try again
        // later.
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

      if (arena->tryGrab1G(newSize)) continue;

      // This test works even if NUMA is not supported, or if there is only 1
      // NUMA node, in which cases we just return nullptr after the first try.
      if (++failCount >= numa_num_nodes) return nullptr;

      // We haven't tried on all NUMA nodes yet, so retry in the next iteration.
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

size_t ManagedArena::activeSize() const {
  // update jemalloc stats
  uint64_t epoch = 1;
  mallctl("epoch", nullptr, nullptr, &epoch, sizeof(epoch));
  size_t pactive = 0;
  size_t sz = sizeof(pactive);
  char buffer[32];
  std::snprintf(buffer, sizeof(buffer),
                "stats.arenas.%d.pactive", m_arenaId);
  if (mallctl(buffer, &pactive, &sz, nullptr, 0) != 0) {
    return size();
  }
  auto const activeSize = pactive * s_pageSize;
  assertx(activeSize <= size());
  return activeSize;
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
                    numa_node_mask,
                    arena->m_currCapacity,
                    arena->m_maxCapacity,
                    arena->activeSize());
      result += buffer;
    }
  }
  return result;
}

}
#endif // USE_JEMALLOC_EXTENT_HOOKS
