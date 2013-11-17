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
#include "hphp/util/arena.h"
#include "hphp/util/util.h"
#include "hphp/util/assertions.h"
#include "hphp/util/malloc-size-class.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

template<size_t kChunkBytes>
ArenaImpl<kChunkBytes>::ArenaImpl() {
  static_assert(kChunkBytes <= 4294967295U,
                "Arena slab size may not be larger than UINT32_MAX");
  static_assert(is_malloc_size_class<kChunkBytes>::value,
                "ArenaImpl instantiated with size that was not a "
                "malloc size class");
  static_assert((kChunkBytes & (kMinBytes - 1)) == 0,
                "kChunkBytes must be multiple of kMinBytes");

  memset(&m_frame, 0, sizeof m_frame);
  m_current = static_cast<char*>(malloc(kChunkBytes));
  m_ptrs.push_back(m_current);
#ifdef DEBUG
  m_externalAllocSize = 0;
#endif
}

template<size_t kChunkBytes>
ArenaImpl<kChunkBytes>::~ArenaImpl() {
  for (size_t i = 0, sz = m_ptrs.size(); i < sz; ++i) {
    free(m_ptrs[i]);
  }
  for (size_t i = 0, sz = m_externalPtrs.size(); i < sz; ++i) {
    free(m_externalPtrs.get(i));
  }
}

template<size_t kChunkBytes>
size_t ArenaImpl<kChunkBytes>::size() const {
  size_t ret = m_ptrs.size() * kChunkBytes - slackEstimate();
#ifdef DEBUG
  ret += m_externalAllocSize;
#endif
  return ret;
}

template<size_t kChunkBytes>
void* ArenaImpl<kChunkBytes>::allocSlow(size_t nbytes) {
  // Large allocations go directly to malloc without discarding our
  // current chunk.
  if (UNLIKELY(nbytes >= kChunkBytes)) {
#if defined(VALGRIND) || !defined(USE_JEMALLOC)
    // We want all our pointers to be kMinBytes - 1 byte aligned.
    // Without jemalloc we have to do that by hand.
    auto extra = kMinBytes - 1;
#else
    auto extra = 0;
#endif

    char* ptr = static_cast<char*>(malloc(nbytes + extra));
#ifdef DEBUG
    m_externalAllocSize += nbytes + extra;
#endif
    m_externalPtrs.push(ptr); // save ptr before aligning it
    // align up to (extra + 1) bytes
    ptr = (char*)((uintptr_t(ptr) + extra) & ~extra);
    assert((intptr_t(ptr) & (kMinBytes - 1)) == 0);
    return ptr;
  }
  createSlab();
  return alloc(nbytes);
}

template<size_t kChunkBytes>
void ArenaImpl<kChunkBytes>::createSlab() {
  ++m_frame.index;
  m_frame.offset = 0;
  if (m_frame.index < m_ptrs.size()) {
    m_current = m_ptrs[m_frame.index];
  } else {
    m_current = static_cast<char*>(malloc(kChunkBytes));
    m_ptrs.push_back(m_current);
  }
  assert((intptr_t(m_current) & (kMinBytes - 1)) == 0);
}

//////////////////////////////////////////////////////////////////////

template class ArenaImpl<4096>;
template class ArenaImpl<32 * 1024>;

//////////////////////////////////////////////////////////////////////

}

