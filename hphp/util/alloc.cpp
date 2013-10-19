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

#ifdef HAVE_NUMA
#include <sys/prctl.h>
#include <numa.h>
#endif

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

int32_t __thread s_numaNode;

#if !defined USE_JEMALLOC || !defined HAVE_NUMA
void enable_numa(bool local) {}
int next_numa_node() { return 0; }
void set_numa_binding(int node) {}
int num_numa_nodes() { return 1; }
void numa_interleave(void* start, size_t size) {}
void numa_local(void* start, size_t size) {}
void numa_bind_to(void* start, size_t size, int node) {}
#endif

#ifdef USE_JEMALLOC
unsigned low_arena = 0;
std::atomic<int> low_huge_pages(0);
std::atomic<void*> highest_lowmall_addr;
static const unsigned kLgHugeGranularity = 21;
static const unsigned kHugePageSize = 1 << kLgHugeGranularity;
static const unsigned kHugePageMask = (1 << kLgHugeGranularity) - 1;

#ifdef HAVE_NUMA
static uint32_t numa_node_set;
static uint32_t numa_num_nodes;
static uint32_t numa_node_mask;
static uint32_t base_arena;
static std::atomic<uint32_t> numa_cur_node;
static std::vector<bitmask*> *node_to_cpu_mask;
static bool use_numa = false;
static bool threads_bind_local = false;

extern "C" void numa_init();

static void initNuma() {
  // numa_init is called automatically, but is probably called after
  // JEMallocInitializer(). its idempotent, so call it here.
  numa_init();
  if (numa_available() < 0) return;

  // set interleave for early code. we'll then force interleave
  // for a few regions, and switch to local for the threads
  numa_set_interleave_mask(numa_all_nodes_ptr);

  int max_node = numa_max_node();
  if (!max_node || max_node >= 32) return;

  bool ret = true;
  bitmask* run_nodes = numa_get_run_node_mask();
  bitmask* mem_nodes = numa_get_mems_allowed();
  for (int i = 0; i <= max_node; i++) {
    if (!numa_bitmask_isbitset(run_nodes, i) ||
        !numa_bitmask_isbitset(mem_nodes, i)) {
      // Only deal with the case of a contiguous set
      // of nodes where we can run/allocate memory
      // on each node.
      ret = false;
      break;
    }
    numa_node_set |= (uint32_t)1 << i;
    numa_num_nodes++;
  }
  numa_bitmask_free(run_nodes);
  numa_bitmask_free(mem_nodes);

  if (!ret || numa_num_nodes <= 1) return;

  numa_node_mask = Util::roundUpToPowerOfTwo(numa_num_nodes) - 1;
}

void enable_numa(bool local) {
  if (!numa_node_mask) return;

  // TODO: Turning off local doesn't really work,
  // see #2941881
  if (local) {
    threads_bind_local = true;

    unsigned arenas;
    size_t sz_arenas = sizeof(arenas);
    if (mallctl("arenas.narenas", &arenas, &sz_arenas, nullptr, 0) != 0) {
      return;
    }

    base_arena = arenas;
    for (int i = 0; i < numa_num_nodes; i++) {
      int arena;
      size_t sz = sizeof(arena);
      if (mallctl("arenas.extend", &arena, &sz, nullptr, 0) != 0) {
        return;
      }
      if (arena != arenas) {
        return;
      }
      arenas++;
    }
  }

  /*
   * libnuma is only partially aware of taskset. If on entry,
   * you have completely disabled a node via taskset, the node
   * will not be available, and calling numa_run_on_node will
   * not work for that node. But if only some of the cpu's on a
   * node were disabled, then calling numa_run_on_node will enable
   * them all. To prevent this, compute the actual masks up front
   */
  bitmask* enabled = numa_allocate_cpumask();
  if (numa_sched_getaffinity(0, enabled) < 0) {
    return;
  }
  node_to_cpu_mask = new std::vector<bitmask*>;
  int num_cpus = numa_num_configured_cpus();
  int max_node = numa_max_node();
  for (int i = 0; i <= max_node; i++) {
    bitmask* cpus_for_node = numa_allocate_cpumask();
    numa_node_to_cpus(i, cpus_for_node);
    for (int j = 0; j < num_cpus; j++) {
      if (!numa_bitmask_isbitset(enabled, j)) {
        numa_bitmask_clearbit(cpus_for_node, j);
      }
    }
    assert(node_to_cpu_mask->size() == i);
    node_to_cpu_mask->push_back(cpus_for_node);
  }
  numa_bitmask_free(enabled);

  use_numa = true;
}

int next_numa_node() {
  if (!use_numa) return 0;
  int node;
  do {
    node = numa_cur_node.fetch_add(1, std::memory_order_relaxed);
    node &= numa_node_mask;
  } while (!((numa_node_set >> node) & 1));
  return node;
}

void set_numa_binding(int node) {
  if (!use_numa) return;

  s_numaNode = node;
  numa_sched_setaffinity(0, (*node_to_cpu_mask)[node]);
  if (threads_bind_local) {
    numa_set_interleave_mask(numa_no_nodes_ptr);
    bitmask* nodes = numa_allocate_nodemask();
    numa_bitmask_setbit(nodes, node);
    numa_set_membind(nodes);
    numa_bitmask_free(nodes);

    int arena = base_arena + node;
    int DEBUG_ONLY e = mallctl("thread.arena", nullptr, nullptr,
                               &arena, sizeof(arena));
    assert(!e);
  }

  char buf[32];
  sprintf(buf, "hhvm.node.%d", node);
  prctl(PR_SET_NAME, buf);
}

int num_numa_nodes() {
  if (!use_numa) return 1;
  return numa_num_nodes;
}

void numa_interleave(void* start, size_t size) {
  if (!use_numa) return;
  numa_interleave_memory(start, size, numa_all_nodes_ptr);
}

void numa_local(void* start, size_t size) {
  if (!use_numa) return;
  numa_setlocal_memory(start, size);
}

void numa_bind_to(void* start, size_t size, int node) {
  if (!use_numa) return;
  numa_tonode_memory(start, size, node);
}

#else
static void initNuma() {}
#endif

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

    initNuma();

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
