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
#ifndef incl_HPHP_UTIL_ARENA_H_
#define incl_HPHP_UTIL_ARENA_H_

#include <vector>
#include <cstdlib>

#include "hphp/util/tiny-vector.h"
#include "hphp/util/pointer-list.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/**
 * Arena/ArenaImpl is an allocator that frees all memory when the
 * arena instance is destroyed.  No destructors of allocated objects
 * will be called!  It is a bump-pointer allocator.
 *
 * At various points in the lifetime of the arena, you can introduce a
 * new `frame' by calling beginFrame.  This is essentially a marker of
 * the current allocator state, which you can pop back to by calling
 * endFrame.
 *
 * Allocations smaller than kMinBytes bytes are rounded up to
 * kMinBytes, and all allocations are kMinBytes-aligned.  This mirrors
 * the way stack alignment works in gcc, which should be good enough.
 *
 * Allocations larger than kChunkBytes are acquired directly from
 * malloc, and don't (currently) get freed with frames.
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
   * Return the amount of memory this arena has handed out via alloc().
   */
  size_t size() const;

  /*
   * Return the amount of memory the arena has allocated, but not yet
   * handed out via alloc().  This can be used to estimate memory
   * usage ignoring arena overhead.
   *
   * Note that this is only an estimate, because we will include
   * fragmentation on the ends of slabs or due to alignment.
   */
  size_t slackEstimate() const { return kChunkBytes - m_frame.offset; }

  /*
   * Framed arena allocation.
   *
   * Nesting allocations between beginFrame() and endFrame() will
   * release memory in a stack-like fashion.  Calling endFrame() more
   * times than beginFrame() will break things.
   *
   * Chunks allocated larger than kChunkBytes are not freed until the
   * entire arena is destroyed.
   *
   * Memory is not released back to malloc until the entire arena is
   * destroyed.
   */
  void beginFrame();
  void endFrame();

 private:
  // copying Arenas will end badly.
  ArenaImpl(const ArenaImpl&);
  ArenaImpl& operator=(const ArenaImpl&);

 private:
  struct Frame {
    Frame*   prev;
    uint32_t index;
    uint32_t offset;
  };

 private:
  void* allocSlow(size_t nbytes);
  void createSlab();

 private:
  char* m_current;
  Frame m_frame;
  TinyVector<char*> m_ptrs; // inlines 1 pointer, may not be optimal
  PointerList<char> m_externalPtrs;
#ifdef DEBUG
  size_t m_externalAllocSize;
#endif
};

//////////////////////////////////////////////////////////////////////

template<size_t kChunkBytes>
inline void* ArenaImpl<kChunkBytes>::alloc(size_t nbytes) {
  nbytes = (nbytes + (kMinBytes - 1)) & ~(kMinBytes - 1); // round up
  size_t newOff = m_frame.offset + nbytes;
  if (newOff <= kChunkBytes) {
    char* ptr = m_current + m_frame.offset;
    m_frame.offset = newOff;
    return ptr;
  }
  return allocSlow(nbytes);
}

template<size_t kChunkBytes>
inline void ArenaImpl<kChunkBytes>::beginFrame() {
  Frame curFrame = m_frame; // don't include the Frame allocation
  Frame* oldFrame = static_cast<Frame*>(alloc(sizeof(Frame)));
  *oldFrame = curFrame;
  m_frame.prev = oldFrame;
}

template<size_t kChunkBytes>
inline void ArenaImpl<kChunkBytes>::endFrame() {
  assert(m_frame.prev);
  m_frame = *m_frame.prev;
  m_current = m_ptrs[m_frame.index];
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
