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
#include <malloc.h>
#include "util/alloc.h"
#include "util/assert.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

template<size_t kChunkBytes>
ArenaImpl<kChunkBytes>::ArenaImpl() : m_next(0), m_limit(0) {
  static_assert((kChunkBytes & (kMinBytes - 1)) == 0,
                "kChunkBytes must be multiple of kMinBytes");
}

template<size_t kChunkBytes>
ArenaImpl<kChunkBytes>::~ArenaImpl() {
  for (size_t i = 0; i < m_ptrs.size(); ++i) {
    free(m_ptrs[i]);
  }
}

template<size_t kChunkBytes>
void* ArenaImpl<kChunkBytes>::alloc_slow(size_t nbytes) {
  // Large allocations go directly to malloc without discarding our chunk.
  if (nbytes >= kChunkBytes) return fill(nbytes);
  char *ptr = fill(kChunkBytes);
  m_limit = ptr + malloc_usable_size(ptr);
  m_next = ptr + nbytes;
  return ptr;
}

/**
 * malloc nbytes bytes of data and remember the pointer.  We only call this
 * for blocks >= kChunkBytes, and therefore assume any self-respecting 
 * malloc implementation will return aligned memory for such a block.
 */
template<size_t kChunkBytes>
char* ArenaImpl<kChunkBytes>::fill(size_t nbytes) {
  char* ptr = (char*)Util::safe_malloc(nbytes);
  ASSERT((intptr_t(ptr) & (kMinBytes - 1)) == 0); // we need aligned blocks.
  m_ptrs.push_back(ptr);
  return ptr;
}

//////////////////////////////////////////////////////////////////////

template class ArenaImpl<4096>;
template class ArenaImpl<32 * 1024>;

}

