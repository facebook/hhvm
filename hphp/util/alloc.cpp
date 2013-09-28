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
#include "hphp/util/alloc.h"

#include <atomic>

#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include "hphp/util/util.h"
#include "hphp/util/logger.h"

#include "folly/Format.h"

namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

void flush_thread_caches() {
#ifdef USE_JEMALLOC
  if (mallctl) {
    unsigned arena;
    size_t usz = sizeof(unsigned);
    if (mallctl("tcache.flush", nullptr, nullptr, nullptr, 0)
        || mallctl("thread.arena", &arena, &usz, nullptr, 0)
        || mallctl("arenas.purge", nullptr, nullptr, &arena, usz)) {
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
const size_t s_pageSize =  sysconf(_SC_PAGESIZE);


static NEVER_INLINE uintptr_t get_stack_top() {
  char marker;
  uintptr_t rsp;

  rsp = uintptr_t(&marker);
  return rsp;
}

void init_stack_limits(pthread_attr_t* attr) {
  size_t stacksize, guardsize;
  void *stackaddr;

  if (pthread_attr_getstack(attr, &stackaddr, &stacksize) != 0) {
    always_assert(false);
  }

  // Get the guard page's size, because the stack address returned
  // above starts at the guard page, so the thread's stack limit is
  // stackaddr + guardsize.
  if (pthread_attr_getguardsize(attr, &guardsize) != 0)
    guardsize = 0;

  assert(stackaddr != nullptr);
  assert(stacksize >= PTHREAD_STACK_MIN);
  Util::s_stackLimit = uintptr_t(stackaddr) + guardsize;
  Util::s_stackSize = stacksize - guardsize;
}

void flush_thread_stack() {
  uintptr_t top = get_stack_top() & ~(Util::s_pageSize - 1);
  // s_stackLimit is already aligned
  assert(top >= s_stackLimit);
  size_t len = top - s_stackLimit;
  assert((len & (Util::s_pageSize - 1)) == 0);
  if (madvise((void*)s_stackLimit, len, MADV_DONTNEED) != 0 &&
      errno != EAGAIN) {
    fprintf(stderr, "%s failed to madvise with error %d\n", __func__, errno);
    abort();
  }
}

#ifdef USE_JEMALLOC
unsigned low_arena = 0;
std::atomic<int> low_huge_pages(0);
std::atomic<void*> highest_lowmall_addr;
static const unsigned kLgHugeGranularity = 21;
static const unsigned kHugePageSize = 1 << kLgHugeGranularity;
static const unsigned kHugePageMask = (1 << kLgHugeGranularity) - 1;

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
    size_t sz = sizeof(low_arena);
    if (mallctl("arenas.extend", &low_arena, &sz, nullptr, 0) != 0) {
      // Error; bail out.
      return;
    }
    const char *dss = "primary";
    if (mallctl(folly::format("arena.{}.dss", low_arena).str().c_str(),
                nullptr, nullptr,
                (void *)&dss, sizeof(const char *)) != 0) {
      // Error; bail out.
      return;
    }

    // We normally maintain the invariant that the region surrounding the
    // current brk is mapped huge, but we don't know yet whether huge pages
    // are enabled for low memory. Round up to the start of a huge page,
    // and set the high water mark to one below.
    unsigned leftInPage = kHugePageSize - (uintptr_t(sbrk(0)) & kHugePageMask);
    (void) sbrk(leftInPage);
    assert((uintptr_t(sbrk(0)) & kHugePageMask) == 0);
    highest_lowmall_addr = (char*)sbrk(0) - 1;
  }
};

#if defined(__GNUC__) && !defined(__APPLE__)
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

static void low_malloc_hugify(void* ptr) {
  // In practice, the things we low_malloc are both long-lived and likely
  // to be randomly accessed. This makes them good candidates for mapping
  // with huge pages. Track a high water mark, and incrementally map each
  // huge page we low_malloc with a huge mapping.
  int remaining = low_huge_pages.load();
  if (!remaining) return;
  for (void* oldValue = highest_lowmall_addr.load(); ptr > oldValue; ) {
    if (highest_lowmall_addr.compare_exchange_weak(oldValue, ptr)) {
      uintptr_t prevRegion = uintptr_t(oldValue) >> kLgHugeGranularity;
      uintptr_t newRegion = uintptr_t(ptr) >> kLgHugeGranularity;
      if (prevRegion != newRegion) {
        // Whoever updates highest_ever is responsible for hinting all the
        // intervening regions. prevRegion is already huge, so bump the
        // region we're hugening by 1.
        int pages = newRegion - prevRegion;
        do {
          if (pages > remaining) pages = remaining;

          if (low_huge_pages.compare_exchange_weak(remaining,
                                                   remaining - pages)) {
            hintHuge((void*)((prevRegion + 1) << kLgHugeGranularity),
                     pages << kLgHugeGranularity);
            break;
          }
        } while (remaining);
      }
      break;
    }
    // Try again.
  }
}

void* low_malloc_impl(size_t size) {
  void* ptr = nullptr;
  allocm(&ptr, nullptr, size, ALLOCM_ARENA(low_arena));
  low_malloc_hugify((char*)ptr + size - 1);
  return ptr;
}

void low_malloc_skip_huge(void* start, void* end) {
  if (low_huge_pages.load()) {
    low_malloc_hugify((char*)start - 1);
    for (void* oldValue = highest_lowmall_addr.load(); end > oldValue; ) {
      if (highest_lowmall_addr.compare_exchange_weak(oldValue, end)) break;
    }
  }
}

#else

void low_malloc_skip_huge(void* start, void* end) {}

#endif // USE_JEMALLOC

///////////////////////////////////////////////////////////////////////////////
}}

extern "C" {
  const char* malloc_conf = "narenas:1,lg_tcache_max:16";
}
