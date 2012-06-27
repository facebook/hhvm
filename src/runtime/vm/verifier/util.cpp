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

#include <runtime/vm/verifier/util.h>
#include <util/alloc.h>

#include <vector>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

using HPHP::Util::safe_malloc;

using namespace HPHP;
using namespace VM;
using namespace Verifier;

Arena::Arena() : m_next(0), m_limit(0) {
  static_assert((kChunkBytes & (kMinBytes - 1)) == 0,
                "kChunkBytes must be multiple of kMinBytes");
}

Arena::~Arena() {
  for (StdRange<std::vector<char*> > r(m_ptrs); !r.empty();) {
    free(r.popFront());
  }
}

void* Arena::alloc_slow(size_t nbytes) {
  // Large allocations go directly to malloc without discarding our chunk.
  if (nbytes >= kChunkBytes) return fill(nbytes);
  char *ptr = fill(kChunkBytes);
  m_limit = ptr + malloc_usable_size(ptr);
  m_next = ptr + nbytes;
  return ptr;
}

/**
 * malloc nbytes bytes of data and remember the pointer.  We only call this
 * for blocks >= kChunkSize, and therefore assume any self-respecting 
 * malloc implementation will return aligned memory for such a block.
 */
char* Arena::fill(size_t nbytes) {
  char* ptr = (char*)safe_malloc(nbytes);
  ASSERT((intptr_t(ptr) & (kMinBytes - 1)) == 0); // we need aligned blocks.
  m_ptrs.push_back(ptr);
  return ptr;
}

Bits::Bits(Arena& arena, int bit_cap) {
  int word_cap = (bit_cap + BPW - 1) / BPW;
  m_words = new (arena) uintptr_t[word_cap];
  memset(m_words, 0, word_cap * sizeof(*m_words));
}
