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

#include "alloc.h"

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

///////////////////////////////////////////////////////////////////////////////
}}
