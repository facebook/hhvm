/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>

#ifdef HAVE_NUMA
#include <sys/prctl.h>
#include <numa.h>
#endif

#include <folly/Bits.h>
#include <folly/Format.h>

#include "hphp/util/logger.h"
#include "hphp/util/async-func.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Default dirty page purging threshold.  This setting is especially relevant to
// arena 0, which all the service requests use.  Arena 1, the low_malloc arena,
// also uses this setting, but that arena is not tuning-sensitive.
#define LG_DIRTY_MULT_DEFAULT 5
// Dirty page purging thresholds for the per NUMA node arenas used by request
// threads.  These arenas tend to have proportionally large memory usage
// fluctuations because requests clean up nearly all allocated memory at request
// end.  Depending on number of request threads, current load, etc., this can
// easily result in excessive dirty page purging.  Therefore, apply loose
// constraints on unused dirty page accumulation under normal operation, but
// momentarily toggle the threshold when a thread idles so that the accumulated
// dirty pages aren't excessive compared to the likely memory usage needs of the
// remaining active threads.
#define LG_DIRTY_MULT_REQUEST_ACTIVE -1
#define LG_DIRTY_MULT_REQUEST_IDLE 3

#ifdef USE_JEMALLOC
static void numa_purge_arena();
#endif

void flush_thread_caches() {
#ifdef USE_JEMALLOC
  if (mallctl) {
    mallctlCall("thread.tcache.flush", true);
    numa_purge_arena();
  }
#endif
#ifdef USE_TCMALLOC
  if (MallocExtensionInstance) {
    MallocExtensionInstance()->MarkThreadIdle();
  }
#endif
}

bool purge_all(std::string* errStr) {
#ifdef USE_JEMALLOC
  if (mallctl) {
    assert(mallctlnametomib && mallctlbymib);
    // Purge all dirty unused pages.
    int err = mallctlWrite<uint64_t>("epoch", 1, true);
    if (err) {
      if (errStr) {
        std::ostringstream estr;
        estr << "Error " << err << " in mallctl(\"epoch\", ...)" << std::endl;
        *errStr = estr.str();
      }
      return false;
    }

    unsigned narenas;
    err = mallctlRead("arenas.narenas", &narenas, true);
    if (err) {
      if (errStr) {
        std::ostringstream estr;
        estr << "Error " << err << " in mallctl(\"arenas.narenas\", ...)"
             << std::endl;
        *errStr = estr.str();
      }
      return false;
    }

    size_t mib[3];
    size_t miblen = 3;
    err = mallctlnametomib("arena.0.purge", mib, &miblen);
    if (err) {
      if (errStr) {
        std::ostringstream estr;
        estr << "Error " << err
             << " in mallctlnametomib(\"arenas.narenas\", ...)" << std::endl;
        *errStr = estr.str();
      }
      return false;
    }
    mib[1] = narenas;

    err = mallctlbymib(mib, miblen, nullptr, nullptr, nullptr, 0);
    if (err) {
      if (errStr) {
        std::ostringstream estr;
        estr << "Error " << err << " in mallctlbymib([\"arena." << narenas
             << ".purge\"], ...)" << std::endl;
        *errStr = estr.str();
      }
      return false;
    }
  }
#endif
#ifdef USE_TCMALLOC
  if (MallocExtensionInstance) {
    MallocExtensionInstance()->ReleaseFreeMemory();
  }
#endif
  return true;
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
  struct rlimit rlim;

#ifndef __APPLE__
  if (pthread_attr_getstack(attr, &stackaddr, &stacksize) != 0) {
    always_assert(false);
  }
#else
  // We must use the following (undocumented) APIs because pthread_attr_getstack
  // returns incorrect values on OSX.
  pthread_t self = pthread_self();
  stackaddr = pthread_get_stackaddr_np(self);
  stacksize = pthread_get_stacksize_np(self);

  // On OSX 10.9, we are lied to about the main thread's stack size.
  // Set it to the minimum stack size, which is set earlier by
  // execute_program_impl.
  const size_t stackSizeMinimum = AsyncFuncImpl::kStackSizeMinimum;
  if (pthread_main_np() == 1) {
    if (s_stackSize < stackSizeMinimum) {
      char osRelease[256];
      size_t osReleaseSize = sizeof(osRelease);
      if (sysctlbyname("kern.osrelease", osRelease, &osReleaseSize,
                       nullptr, 0) == 0) {
        if (atoi(osRelease) >= 13) {
          stacksize = stackSizeMinimum;
        }
      }
    }
  }

  // stackaddr is not base, but top of the stack. Yes, really.
  stackaddr = ((char*) stackaddr) - stacksize;
#endif

  // Get the guard page's size, because the stack address returned
  // above starts at the guard page, so the thread's stack limit is
  // stackaddr + guardsize.
  if (pthread_attr_getguardsize(attr, &guardsize) != 0) {
    guardsize = 0;
  }

  assert(stackaddr != nullptr);
  assert(stacksize >= PTHREAD_STACK_MIN);
  s_stackLimit = uintptr_t(stackaddr) + guardsize;
  s_stackSize = stacksize - guardsize;

  // The main thread's native stack may be larger than desired if
  // set_stack_size() failed.  Make sure that even if the native stack is
  // extremely large (in which case anonymous mmap() could map some of the
  // "stack space"), we can differentiate between the part of the native stack
  // that could conceivably be used in practice and all anonymous mmap() memory.
  if (getrlimit(RLIMIT_STACK, &rlim) == 0 && rlim.rlim_cur == RLIM_INFINITY &&
      s_stackSize > AsyncFuncImpl::kStackSizeMinimum) {
    s_stackLimit += s_stackSize - AsyncFuncImpl::kStackSizeMinimum;
    s_stackSize = AsyncFuncImpl::kStackSizeMinimum;
  }
}

void flush_thread_stack() {
  uintptr_t top = get_stack_top() & ~(s_pageSize - 1);
  // s_stackLimit is already aligned
  assert(top >= s_stackLimit);
  size_t len = top - s_stackLimit;
  assert((len & (s_pageSize - 1)) == 0);
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

extern "C" {
HHVM_ATTRIBUTE_WEAK extern void numa_init(void);
}
static void initNuma() {

  // When linked dynamically numa_init() is called before JEMallocInitializer()
  // numa_init is not exported by libnuma.so so it will be NULL
  // however when linked statically numa_init() is not guaranteed to be called
  // before JEMallocInitializer(),so call it here.
  if(&numa_init) {
    numa_init();
  }
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

  numa_node_mask = folly::nextPowTwo(numa_num_nodes) - 1;
}

static bool purge_decay_hard() {
  const char *purge;
  if (mallctlRead("opt.purge", &purge, true) == 0) {
    return (strcmp(purge, "decay") == 0);
  }

  // If "opt.purge" is absent, it's either because jemalloc is too old
  // (pre-4.1.0), or because ratio-based purging is no longer present
  // (likely post-4.x).
  ssize_t decay_time;
  return (mallctlRead("opt.decay_time", &decay_time, true) == 0);
}

static bool purge_decay() {
  static bool initialized = false;
  static bool decay;

  if (!initialized) {
    decay = purge_decay_hard();
    initialized = true;
  }
  return decay;
}

static void set_lg_dirty_mult(unsigned arena, ssize_t lg_dirty_mult) {
  assert(!purge_decay());
  constexpr size_t max_miblen = 3;
  size_t miblen = max_miblen;
  size_t mib[max_miblen];
  if (mallctlnametomib("arena.0.lg_dirty_mult", mib, &miblen) == 0) {
    mib[1] = arena;
    int err = mallctlbymib(mib, miblen, nullptr, nullptr, &lg_dirty_mult,
                           sizeof(lg_dirty_mult));
    if (err != 0) {
      Logger::Warning("mallctl arena.%u.lg_dirty_mult failed with error %d",
                      arena, err);
    }
  }
}

static void numa_purge_arena() {
  // Only purge if the thread's assigned arena is one of those created for use
  // by request threads.
  if (!threads_bind_local) return;
  // Only purge if ratio-based purging is active.
  if (purge_decay()) return;
  unsigned arena;
  mallctlRead("thread.arena", &arena);
  if (arena >= base_arena && arena < base_arena + numa_num_nodes) {
    // Threads may race through the following calls, but the last call made by
    // any idling thread will correctly restore lg_dirty_mult.
    set_lg_dirty_mult(arena, LG_DIRTY_MULT_REQUEST_IDLE);
    set_lg_dirty_mult(arena, LG_DIRTY_MULT_REQUEST_ACTIVE);
  }
}

void enable_numa(bool local) {
  if (!numa_node_mask) return;

  // TODO: Turning off local doesn't really work,
  // see #2941881
  if (local) {
    threads_bind_local = true;

    unsigned arenas;
    if (mallctlRead("arenas.narenas", &arenas, true) != 0) {
      return;
    }

    base_arena = arenas;
    for (int i = 0; i < numa_num_nodes; i++) {
      int arena;
      if (mallctlRead("arenas.extend", &arena, true) != 0) {
        return;
      }
      if (arena != arenas) {
        return;
      }
      arenas++;
      if (!purge_decay()) {
        // Tune dirty page purging for new arena.
        set_lg_dirty_mult(arena, LG_DIRTY_MULT_REQUEST_ACTIVE);
      }
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
    mallctlWrite("thread.arena", arena);
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "hhvm.node.%d", node);
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
static void numa_purge_arena() {}
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

    // Enable backtracing through PHP frames (t9814472).
    setenv("UNW_RBP_ALWAYS_VALID", "1", false);

    initNuma();

    // Create a special arena to be used for allocating objects in low memory.
    if (mallctlRead("arenas.extend", &low_arena, true) != 0) {
      // Error; bail out.
      return;
    }
    if (mallctlWrite(folly::format("arena.{}.dss", low_arena).str().c_str(),
                     "primary", true) != 0) {
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
#define MAX_CONSTRUCTOR_PRIORITY __attribute__((__init_priority__(101)))
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
  void* ptr = mallocx(size, low_mallocx_flags());
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

int mallctlCall(const char* cmd, bool errOk) {
  // Use <unsigned> rather than <void> to avoid sizeof(void).
  return mallctlHelper<unsigned>(cmd, nullptr, nullptr, errOk);
}

int jemalloc_pprof_enable() {
  return mallctlWrite("prof.active", true, true);
}

int jemalloc_pprof_disable() {
  return mallctlWrite("prof.active", false, true);
}

int jemalloc_pprof_dump(const std::string& prefix, bool force) {
  if (!force) {
    bool active = false;
    // Check if profiling has been enabled before trying to dump.
    int err = mallctlRead("opt.prof", &active, true);
    if (err || !active) {
      return 0; // nothing to do
    }
  }

  if (prefix != "") {
    const char *s = prefix.c_str();
    return mallctlWrite("prof.dump", s, true);
  } else {
    return mallctlCall("prof.dump", true);
  }
}

///////////////////////////////////////////////////////////////////////////////
}

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

extern "C" {
  const char* malloc_conf = "narenas:1,lg_tcache_max:16,"
    "lg_dirty_mult:" STRINGIFY(LG_DIRTY_MULT_DEFAULT)
#ifdef ENABLE_HHPROF
    ",prof:true,prof_active:false,prof_thread_active_init:false"
#endif
    ;
}
