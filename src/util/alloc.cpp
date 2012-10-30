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
#include <sys/mman.h>
#include <sys/user.h>
#include <stdlib.h>
#include <errno.h>
#include "util.h"
#include "logger.h"

namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

void flush_thread_caches() {
#ifdef USE_JEMALLOC
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
#ifdef USE_TCMALLOC
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

#ifdef USE_JEMALLOC
unsigned low_arena = 0;
struct JEMallocInitializer {
  JEMallocInitializer() {
    // The following comes from malloc_extension.cc in google-perftools
#ifdef __GLIBC__
    // GNU libc++ versions 3.3 and 3.4 obey the environment variables
    // GLIBCPP_FORCE_NEW and GLIBCXX_FORCE_NEW respectively.  Setting
    // one of these variables forces the STL default allocator to call
    // new() or delete() for each allocation or deletion.  Otherwise
    // the STL allocator tries to avoid the high cost of doing
    // allocations by pooling memory internally.  However, tcmalloc
    // does allocations really fast, especially for the types of small
    // items one sees in STL, so it's better off just using us.
    // TODO: control whether we do this via an environment variable?
    setenv("GLIBCPP_FORCE_NEW", "1", false /* no overwrite*/);
    setenv("GLIBCXX_FORCE_NEW", "1", false /* no overwrite*/);

    // Now we need to make the setenv 'stick', which it may not do since
    // the env is flakey before main() is called.  But luckily stl only
    // looks at this env var the first time it tries to do an alloc, and
    // caches what it finds.  So we just cause an stl alloc here.
    std::string dummy("I need to be allocated");
    dummy += "!";         // so the definition of dummy isn't optimized out
#endif  /* __GLIBC__ */
    // Create a special arena to be used for allocating objects in low memory.
    int err;
    size_t sz = sizeof(low_arena);
    if ((err = mallctl("arenas.extend", &low_arena, &sz, NULL, 0)) != 0) {
      // Error; bail out.
      return;
    }
    size_t mib[3];
    size_t miblen = sizeof(mib) / sizeof(size_t);
    const char *dss = "primary";
    if ((err = mallctlnametomib("arena.0.dss", mib, &miblen)) != 0) {
      // Error; bail out.
      return;
    }
    mib[1] = low_arena;
    if ((err = mallctlbymib(mib, miblen, NULL, NULL, (void *)&dss,
        sizeof(const char *))) != 0) {
      // Error; bail out.
      return;
    }
  }
};

#ifdef __GNUC__
// Construct this object before any others.
// 101 is the highest priority allowed by the init_priority attribute.
// http://gcc.gnu.org/onlinedocs/gcc-4.0.4/gcc/C_002b_002b-Attributes.html
#define MAX_CONSTRUCTOR_PRIORITY __attribute__((init_priority(101)))
#else
// init_priority is a gcc extension, so we can't use it on other compilers.
// However, since constructor ordering is only known to be an issue with
// GNU libc++ we're probably OK on other compilers so let the situation pass
// silently instead of issuing a warning.
#define MAX_CONSTRUCTOR_PRIORITY
#endif

static JEMallocInitializer initJEMalloc MAX_CONSTRUCTOR_PRIORITY;
#endif

///////////////////////////////////////////////////////////////////////////////
}}

extern "C" {
  const char* malloc_conf = "narenas:1,lg_tcache_max:16";
}
