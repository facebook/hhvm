/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifdef USE_JEMALLOC_CHUNK_HOOKS
#include <stdexcept>

namespace HPHP {

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
                           size_t length, unsigned arena_id) {
  return true;
}

static bool chunk_purge(void* chunk, size_t size, size_t offset,
                        size_t length, unsigned arena_id) {
  return true;
}

static bool chunk_split(void* chunk, size_t size, size_t sizea, size_t sizeb,
                        bool comitted, unsigned arena_id) {
  return false;
}

static bool chunk_merge(void* chunka, size_t sizea, void* chunkb, size_t sizeb,
                        bool committed, unsigned arena_ind) {
  return false;
}

//////////////////////////////////////////////////////////////////////

ManagedArena** ManagedArena::s_allocs;
size_t ManagedArena::s_allocs_cap;

ManagedArena::ManagedArena(void* base, size_t cap)
  : m_base(static_cast<char*>(base)), m_capacity(cap) {
  // Create a special arena to manage this piece of memory.
  size_t sz = sizeof(m_arenaId);
  if (mallctl("arenas.extend", &m_arenaId, &sz, nullptr, 0) != 0) {
    throw std::runtime_error{"error in arenas.extend"};
  }
  chunk_hooks_t hooks {
    ManagedArena::chunk_alloc,
    chunk_dalloc,
    chunk_commit,
    chunk_decommit,
    chunk_purge,
    chunk_split,
    chunk_merge
  };
  auto const& command = folly::sformat("arena.{}.chunk_hooks", m_arenaId);
  sz = sizeof(hooks);
  if (mallctl(command.c_str(), nullptr, nullptr, &hooks, sz) != 0) {
    throw std::runtime_error("error in setting chunk hooks");
  }
  // Not thread-safe.  Only create ManagedArenas before other threads are
  // running.
  if (s_allocs == nullptr) {
    s_allocs_cap = m_arenaId + 16;
    s_allocs = static_cast<ManagedArena**>(
      calloc(sizeof(ManagedArena*), s_allocs_cap));
  } else if (m_arenaId >= s_allocs_cap) {
    auto const oldCap = s_allocs_cap;
    s_allocs_cap = m_arenaId + 16;
    s_allocs = static_cast<ManagedArena**>(realloc(s_allocs, s_allocs_cap));
    memset(s_allocs + oldCap, 0,
           sizeof(ManagedArena*) * (s_allocs_cap - oldCap));
  }
  s_allocs[m_arenaId] = this;
}

void* ManagedArena::chunk_alloc(void* chunk, size_t size, size_t alignment,
                                bool* zero, bool* commit, unsigned arena_ind) {
  if (chunk != nullptr) return nullptr;

  ManagedArena* allocator = s_allocs[arena_ind];
  if (allocator == nullptr) return nullptr;
  // Just in case size is too big (negative)
  if (size > allocator->m_capacity) return nullptr;

  assert(folly::isPowTwo(alignment));
  auto const mask = alignment - 1;
  size_t startOffset;
  do {
    auto currSize = allocator->m_size.load(std::memory_order_relaxed);
    startOffset = (currSize + mask) & ~mask;
    auto newFrontier = startOffset + size;
    if (newFrontier > allocator->m_capacity) {
      return nullptr;
    }
    if (allocator->m_size.compare_exchange_weak(currSize, newFrontier)) {
      break;
    }
  } while (true);
  *zero = true;
  *commit = true;
  return const_cast<char*>(allocator->m_base) + startOffset;
}

}
#endif // USE_JEMALLOC_CHUNK_HOOKS
