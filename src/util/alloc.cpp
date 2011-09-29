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

#include <sys/mman.h>
#include <sys/user.h>
#include <stdlib.h>
#include <errno.h>
#include "alloc.h"
#include "util.h"
#include "logger.h"

namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

void* safe_malloc(size_t size) {
  void *ptr = std::malloc(size);

  if (ptr == NULL) {
    throw OutOfMemoryException(size);
  }
  return ptr;
}

void* safe_realloc(void *var, size_t size) {
  void *ptr = NULL;

  ptr = std::realloc(var, size);
  if (ptr == NULL && size != 0) {
    throw OutOfMemoryException(size);
  }

  return ptr;
}

void safe_free(void *ptr) {
  if (!ptr) {
    free(ptr);
  }
}

void flush_thread_caches() {
#ifndef NO_JEMALLOC
  if (mallctl) {
    unsigned arena;
    size_t usz = sizeof(unsigned);
    if (mallctl("tcache.flush", NULL, NULL, NULL, 0)
        || mallctl("thread.arena", &arena, &usz, NULL, 0)
        || mallctl("arenas.purge", NULL, NULL, &arena, usz)) {
      // Error; do nothing.
    }
  }
#endif
#ifndef NO_TCMALLOC
  if (MallocExtensionInstance) {
    MallocExtensionInstance()->MarkThreadIdle();
  }
#endif
}

__thread uintptr_t s_stackLimit;
__thread size_t s_stackSize;

static NEVER_INLINE uintptr_t get_stack_top() {
  char marker;
  uintptr_t rsp;

  rsp = uintptr_t(&marker);
  return rsp;
}

void flush_thread_stack() {
  uintptr_t top = get_stack_top() & ~(PAGE_SIZE - 1);
  // s_stackLimit is already aligned
  ASSERT(top >= s_stackLimit);
  size_t len = top - s_stackLimit;
  ASSERT((len & (PAGE_SIZE - 1)) == 0);
  if (madvise((void*)s_stackLimit, len, MADV_DONTNEED) != 0 &&
      errno != EAGAIN) {
    fprintf(stderr, "%s failed to madvise with error %d\n", __func__, errno);
    abort();
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
