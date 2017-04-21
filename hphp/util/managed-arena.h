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

#ifndef incl_HPHP_UTIL_MANAGED_ARENA_H_
#define incl_HPHP_UTIL_MANAGED_ARENA_H_

#include "hphp/util/alloc.h"

#ifdef USE_JEMALLOC_CHUNK_HOOKS

#include <string>

namespace HPHP {

/*
 * jemalloc arena backed by 1G huge pages.
 *
 * The huge pages are added on demand, until the maximum capacity is reached.
 * Virtual address for newly added pages grow downward, and the "maximum
 * address" is fixed for the arena.  For example, in the low-memory huge arena,
 * the maximum address is 4G - 1.  After the first page is added, the arena
 * contains address range [3G, 4G).  If another page is later added, the range
 * is [2G, 4G).  We make it grow downward so that if the second huge page isn't
 * available, the address space can still be allocated using brk().
 */
struct ManagedArena {
 public:
  ManagedArena(void* base, size_t maxCap,
               int nextNode = -1, int nodeMask = -1);

  inline void* malloc(size_t size) {
    if (m_base == nullptr || m_maxCapacity == 0) return nullptr;
    return mallocx(size, MALLOCX_ARENA(m_arenaId) | MALLOCX_TCACHE_NONE);
  }

  inline void free(void* ptr) {
    if (ptr) dallocx(ptr, MALLOCX_ARENA(m_arenaId) | MALLOCX_TCACHE_NONE);
  }

  inline unsigned id() const {
    return m_arenaId;
  }

  inline size_t size() const {
    return m_size;
  }

  // Report usage.
  static std::string reportStats();

 private:
  static void* chunk_alloc(void* chunk, size_t size, size_t alignment,
                           bool* zero, bool* commit, unsigned arena_ind);

 private:
  char* const m_base{nullptr};
  size_t m_maxCapacity{0};
  size_t m_currCapacity{0};             // Change protected by s_lock
  std::atomic_size_t m_size{0};
  int m_nextNode{-1};
  int const m_nodeMask{-1};
  unsigned m_arenaId{static_cast<unsigned>(-1)};

  // Hold this lock while adding new pages to any arena.  This is not a member
  // to each arena, because we don't want multiple threads to grab huge pages
  // simultaneously.
  static std::atomic_bool s_lock;

  // `malloc_conf` has "narenas:1", so we won't have many arenas.  For efficient
  // lookup from arena ind to ManagedArena, we use an array here.
#ifndef MAX_HUGE_ARENA_COUNT
#define MAX_HUGE_ARENA_COUNT 8
#endif
  static ManagedArena* s_arenas[MAX_HUGE_ARENA_COUNT];
};

}
#endif // USE_JEMALLOC_CHUNK_HOOKS
#endif
