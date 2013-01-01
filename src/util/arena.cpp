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
#include "util/arena.h"
#include "util/util.h"
#include "util/assertions.h"
#include "util/malloc_size_class.h"

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
void* ArenaImpl<kChunkBytes>::allocSlow(size_t nbytes) {
  // Large allocations go directly to malloc without discarding our
  // current chunk.
  if (UNLIKELY(nbytes >= kChunkBytes)) {
    char* ptr = static_cast<char*>(malloc(nbytes));
    m_externalPtrs.push(ptr);
    ASSERT((intptr_t(ptr) & (kMinBytes - 1)) == 0);
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
  ASSERT((intptr_t(m_current) & (kMinBytes - 1)) == 0);
}

//////////////////////////////////////////////////////////////////////

template class ArenaImpl<4096>;
template class ArenaImpl<32 * 1024>;

//////////////////////////////////////////////////////////////////////

}

