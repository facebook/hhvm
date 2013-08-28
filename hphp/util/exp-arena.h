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
#ifndef incl_HPHP_UTIL_EXP_ARENA_H_
#define incl_HPHP_UTIL_EXP_ARENA_H_

#include <boost/noncopyable.hpp>
#include <stdint.h>
#include "hphp/util/alloc.h"
#include "hphp/util/util.h"
#include "hphp/util/assertions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Simple arena allocator with exponentially increasing slab size, up
 * to a limit, where it switches to constant slab size.
 *
 * We also want sizeof(ExpArena) be reasonably small, so the slabs are
 * linked together.
 *
 * For an Arena allocator that uses linear growth, see arena.h.
 *
 * Use the ExpArena typedef for the default configuration.
 */
template<size_t MaxSlabSize> struct ExpArenaImpl;
typedef ExpArenaImpl<1 << 20> ExpArena;

//////////////////////////////////////////////////////////////////////

template<size_t MaxSlabSize = (1 << 20)>
struct ExpArenaImpl : boost::noncopyable {
  static_assert(MaxSlabSize <= 4294967295U,
                "MaxSlabSize may not be larger than UINT32_MAX");

  explicit ExpArenaImpl(size_t reserve)
    : m_current(0)
  {
    createSlab(reserve);
  }

  ~ExpArenaImpl() {
    clear();
  }

  /*
   * Allocate `sz' bytes, where `sz' must be smaller than the initial
   * reserve size or you'll get extremely pathological behavior.
   * (Preferably reserve much larger than the sizes of allocations
   * expected.)
   */
  void* alloc(size_t sz) {
    void* p = m_current + m_curFrontier;
    m_curFrontier += sz;
    if (LIKELY(m_curFrontier <= m_curSize)) {
      return p;
    }

    createSlab(size_t(m_curSize) * 3 / 2);
    return alloc(sz);
  }

  /*
   * Deallocate all used memory.  Invalidates all pointers returned
   * from alloc().
   */
  void clear() {
    void* vpCurrent = m_current;
    Slab* p = static_cast<Slab*>(vpCurrent);
    while (p) {
      Slab* prev = p->prev;
      free(p);
      p = prev;
    }
    m_current = 0;
  }

private:
  struct Slab {
    Slab* prev;
    uintptr_t empty;  // For 16-byte alignment
    unsigned char buf[];
  };
  static_assert(sizeof(Slab) == 16, "");

  void createSlab(size_t desiredSz) {
    desiredSz += offsetof(Slab, buf);
    size_t sz = std::min(desiredSz, MaxSlabSize);

    void* newVp = malloc(sz);
    void* curVp = m_current;
    static_cast<Slab*>(newVp)->prev = static_cast<Slab*>(curVp);
    m_current = static_cast<unsigned char*>(newVp);
    m_curSize = malloc_usable_size(m_current);
    m_curFrontier = offsetof(Slab, buf);
  }

private:
  unsigned char* m_current;
  uint32_t m_curSize; // includes space for the prev block pointer
  uint32_t m_curFrontier;
};

//////////////////////////////////////////////////////////////////////

}

#endif
