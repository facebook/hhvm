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
#pragma once

#include <vector>
#include <cstdlib>
#include <cstring>

#include "hphp/util/pointer-list.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/**
 * Arena/ArenaImpl is an allocator that frees all its memory when the arena
 * instance is destroyed.  It is a bump-pointer allocator.
 *
 * There are two main methods to allocate memory from the arena:
 *   1) alloc() allocates raw memory for which no destructor is invoked when the
 *      arena is destroyed.
 *   2) allocD<C>() allocates memory for an object of class C, whose destructor
 *      (or a custom callback) is invoked when the arena is destroyed.
 *
 * Allocations smaller than kMinBytes bytes are rounded up to kMinBytes, and
 * all allocations are kMinBytes-aligned.
 *
 * Allocations larger than kChunkBytes are acquired directly from
 * malloc.
 *
 * The Arena typedef is for convenience when you want a default
 * configuration.  Use ArenaImpl if you want something specific.
 */
template<size_t kChunkBytes> struct ArenaImpl;
typedef ArenaImpl<4096> Arena;

//////////////////////////////////////////////////////////////////////

template<size_t kChunkBytes>
struct ArenaImpl {
 public:
  ArenaImpl();
  ~ArenaImpl();

  /*
   * Allocate a raw chunk of memory with `nbytes' bytes.  No destructors will be
   * called when the arena is destroyed.
   */
  void* alloc(size_t nbytes);

  /*
   * Allocate a chunk of memory large enough to hold an object of class C, and
   * register a destructor to be invoked when the arena is destroyed.  A custom
   * `dtor' to be invoked can be provided, otherwise C's destructor is invoked.
   */
  template<class C>
  C* allocD();

  template<class C, class D>
  C* allocD(D dtor);

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
  size_t slackEstimate() const { return kChunkBytes - m_offset; }

 private:
  struct Destroyer {
    void* obj;
    void (*dtor)(void*);
    Destroyer* next;
  };

  // copying Arenas will end badly.
  ArenaImpl(const ArenaImpl&);
  ArenaImpl& operator=(const ArenaImpl&);

  static const size_t kMinBytes = 8;

  void* allocSlow(size_t nbytes);
  void createSlab();

  char* m_current;
  uint32_t m_offset;
  std::vector<char*> m_ptrs;
  PointerList<char> m_externalPtrs;
  Destroyer* m_dtors;
  bool m_bypassSlabAlloc;
#ifndef NDEBUG
  size_t m_externalAllocSize;
#endif
};

//////////////////////////////////////////////////////////////////////

template<size_t kChunkBytes>
inline void* ArenaImpl<kChunkBytes>::alloc(size_t nbytes) {
  nbytes = (nbytes + (kMinBytes - 1)) & ~(kMinBytes - 1); // round up
  size_t newOff = m_offset + nbytes;
  if (newOff <= kChunkBytes) {
    char* ptr = m_current + m_offset;
    m_offset = newOff;
    return ptr;
  }
  return allocSlow(nbytes);
}

template<size_t kChunkBytes>
template<class C, class D>
inline C* ArenaImpl<kChunkBytes>::allocD(D dtor) {
  auto ptr = (C*)alloc(sizeof(C));
  auto dtorPtr = (Destroyer*)alloc(sizeof(Destroyer));
  dtorPtr->obj = ptr;
  dtorPtr->dtor = dtor;
  dtorPtr->next = m_dtors;
  m_dtors = dtorPtr;
  return ptr;
}

template<size_t kChunkBytes>
template<class C>
inline C* ArenaImpl<kChunkBytes>::allocD() {
  return allocD([](void* p) { static_cast<C*>(p)->~C(); });
}

void SetArenaSlabAllocBypass(bool f);

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
