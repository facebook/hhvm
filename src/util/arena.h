/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#ifndef incl_UTIL_ARENA_H_
#define incl_UTIL_ARENA_H_

#include <vector>
#include <cstdlib>

#include "util/tiny_vector.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/**
 * Arena/ArenaImpl is an allocator that frees all memory when the
 * arena instance is destroyed.  No destructors of allocated objects
 * will be called!  It is a bump-pointer allocator.
 *
 * If we're out of memory, allocation functions throw an exception.
 * Blocks smaller than kMinBytes bytes are rounded up to kMinBytes,
 * and all blocks are kMinBytes-aligned.  This mirrors the way stack
 * alignment works in gcc, which should be good enough.
 *
 * The Arena typedef is for convenience when you want a default
 * configuration.  Use ArenaImpl if you want something specific.
 */
template<size_t kChunkBytes> class ArenaImpl;
typedef ArenaImpl<4096> Arena;

//////////////////////////////////////////////////////////////////////

template<size_t kChunkBytes>
class ArenaImpl {
  static const size_t kMinBytes = 16;
 public:
  ArenaImpl();
  ~ArenaImpl();

  void* alloc(size_t nbytes);

  /*
   * Return the amount of memory the arena has allocated, but not yet
   * handed out via alloc().  This can be used to estimate memory
   * usage ignoring arena overhead.
   *
   * Note that this is only an estimate, because we will include
   * fragmentation on the ends of slabs or due to alignment.
   */
  size_t slackEstimate() const { return size_t(m_limit - m_next); }

 private:
  // copying Arenas will end badly.
  ArenaImpl(const ArenaImpl&);
  ArenaImpl& operator=(const ArenaImpl&);
 private:
  void* alloc_slow(size_t nbytes);
  char* fill(size_t nbytes);
 private:
  char* m_next;
  char* m_limit;
  TinyVector<char*> m_ptrs;
};

//////////////////////////////////////////////////////////////////////

template<size_t kChunkBytes>
inline void* ArenaImpl<kChunkBytes>::alloc(size_t nbytes) {
  nbytes = (nbytes + (kMinBytes - 1)) & ~(kMinBytes - 1); // round up
  char* ptr = m_next;
  char* next = ptr + nbytes;
  if (next <= m_limit) {
    m_next = next;
    return ptr;
  }
  return alloc_slow(nbytes);
}

//////////////////////////////////////////////////////////////////////

} // HPHP

// These global-operator-new declarations cannot be in a namespace,
// but since they take Arena arguments we won't overload anything else.

template<size_t kChunkBytes>
inline void* operator new(size_t nbytes,
                          HPHP::ArenaImpl<kChunkBytes>& a) {
  return a.alloc(nbytes);
}

template<size_t kChunkBytes>
inline void* operator new[](size_t nbytes,
                            HPHP::ArenaImpl<kChunkBytes>& a) {
  return a.alloc(nbytes);
}

//////////////////////////////////////////////////////////////////////

#endif
