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

#ifndef incl_HPHP_UTIL_MANAGED_ARENA_H_
#define incl_HPHP_UTIL_MANAGED_ARENA_H_

#include "hphp/util/alloc.h"

#ifdef USE_JEMALLOC_CHUNK_HOOKS
#include <atomic>

namespace HPHP {
// jemalloc managed arena, backed by an alllocated memory region.
struct ManagedArena {
  // s_allocs[arena_idx] is a pointer to the ManagedArena.
  static ManagedArena** s_allocs;
  static size_t s_allocs_cap;

 public:
  // Constructor takes a piece of allocated memory.
  explicit ManagedArena(void* base, size_t cap);

  inline bool valid() const {
    return m_base != nullptr;
  }

  inline void* malloc(size_t size) {
    if (!valid()) return nullptr;
    return mallocx(size, MALLOCX_ARENA(m_arenaId));
  }

  inline void free(void* ptr) {
    if (ptr) dallocx(ptr, MALLOCX_ARENA(m_arenaId));
  }

  inline unsigned getArenaId() const {
    return m_arenaId;
  }

 private:
  static void* chunk_alloc(void* chunk, size_t size, size_t alignment,
                           bool* zero, bool* commit, unsigned arena_ind);

 private:
  char* const m_base{nullptr};
  const size_t m_capacity{0};
  std::atomic_size_t m_size{0};
  unsigned m_arenaId{static_cast<unsigned>(-1)};
};

}
#endif // USE_JEMALLOC_CHUNK_HOOKS
#endif
