/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <stdlib.h>
#include <errno.h>

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#include <folly/portability/SysMman.h>
#include <folly/portability/SysResource.h>

#include "hphp/util/bump-mapper.h"
#include "hphp/util/extent-hooks.h"
#include "hphp/util/hugetlb.h"
#include "hphp/util/kernel-version.h"
#include "hphp/util/managed-arena.h"
#include "hphp/util/numa.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void flush_thread_caches() {
#ifdef USE_JEMALLOC
  if (mallctl) {
    mallctlCall("thread.tcache.flush", true);
  }
#ifdef USE_JEMALLOC_EXTENT_HOOKS
  high_arena_tcache_flush();
#endif
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
    unsigned allArenas = 0;
#ifndef MALLCTL_ARENAS_ALL
    if (mallctlRead("arenas.narenas", &allArenas, true)) {
      if (errStr) {
        *errStr = "arenas.narena";
      }
      return false;
    }
#else
    allArenas = MALLCTL_ARENAS_ALL;
#endif

    size_t mib[3];
    size_t miblen = 3;
    if (mallctlnametomib("arena.0.purge", mib, &miblen)) {
      if (errStr) {
        *errStr = "mallctlnametomib(arena.0.purge)";
      }
      return false;
    }

    mib[1] = allArenas;
    if (mallctlbymib(mib, miblen, nullptr, nullptr, nullptr, 0)) {
      if (errStr) {
        *errStr = "mallctlbymib(arena.all.purge)";
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
unsigned s_hugeStackSizeKb;

static NEVER_INLINE uintptr_t get_stack_top() {
  using ActRec = char;
  DECLARE_FRAME_POINTER(fp);
  return uintptr_t(fp) - s_pageSize;
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

  // On OSX 10.9, we are lied to about the main thread's stack size.  Set it to
  // the minimum stack size, which is set earlier by execute_program_impl.
  if (pthread_main_np() == 1) {
    if (s_stackSize < kStackSizeMinimum) {
      char osRelease[256];
      size_t osReleaseSize = sizeof(osRelease);
      if (sysctlbyname("kern.osrelease", osRelease, &osReleaseSize,
                       nullptr, 0) == 0) {
        if (atoi(osRelease) >= 13) {
          stacksize = kStackSizeMinimum;
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
      s_stackSize > kStackSizeMinimum) {
    s_stackLimit += s_stackSize - kStackSizeMinimum;
    s_stackSize = kStackSizeMinimum;
  }
}

void flush_thread_stack() {
  uintptr_t top = get_stack_top() & ~(s_pageSize - 1);
  if (s_firstSlab.ptr) {
    uintptr_t boundary =               // between hugetlb pages and normal pages
       reinterpret_cast<uintptr_t>(s_firstSlab.ptr) - s_hugeStackSizeKb * 1024;
    assert(boundary % size2m == 0);
    if (boundary < top) top = boundary;
  }
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

__thread int32_t s_numaNode;
__thread MemBlock s_firstSlab;

#if !defined USE_JEMALLOC || !defined HAVE_NUMA
void enable_numa(bool local) {}
void set_numa_binding(int node) {}
void* mallocx_on_node(size_t size, int node, size_t align) {
  void* ret = nullptr;
  posix_memalign(&ret, align, size);
  return ret;
}
#endif

#ifdef USE_JEMALLOC

#ifdef USE_JEMALLOC_EXTENT_HOOKS
unsigned low_arena = 0;
unsigned high_arena = 0;

// Keep track of the size of recently freed memory that might be in the high1g
// arena when it is disabled, so that we know when to reenable it.
std::atomic_uint g_highArenaRecentlyFreed;

alloc::Bump2MMapper* low_2m_mapper = nullptr;

// Customized hooks to use 1g pages for jemalloc metadata.
static extent_hooks_t huge_page_metadata_hooks;
static extent_alloc_t* orig_alloc = nullptr;

static bool enableArenaMetadata1GPage = false;
static bool enableNumaArenaMetadata1GPage = false;
// jemalloc metadata is allocated through the internal base allocator, which
// expands memory with an increasingly larger sequence.  The default reserved
// space (216MB)is a sum of the sequence, from 2MB to 40MB.
static size_t a0MetadataReservedSize = 0;
static std::atomic<bool> jemallocMetadataCanUseHuge(false);
static void* a0ReservedBase = nullptr;
static std::atomic<size_t> a0ReservedLeft(0);

// Explicit per-thread tcache for huge arenas.  -1 means no tcache.
// In jemalloc/include/jemalloc/jemalloc_macros.h.in, we have
// #define MALLOCX_TCACHE_NONE MALLOCX_TCACHE(-1)
__thread int high_arena_tcache = -1;
static_assert(MALLOCX_TCACHE(-1) == MALLOCX_TCACHE_NONE,
              "Are you using jemalloc 5.x?");
#else
// legacy implementation of low arena using brk
unsigned dss_arena = 0;
#endif

#ifdef HAVE_NUMA
static uint32_t base_arena;
static bool threads_bind_local = false;

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
      int arena, ret;

#ifdef USE_JEMALLOC_EXTENT_HOOKS
      if (jemallocMetadataCanUseHuge.load() && enableNumaArenaMetadata1GPage) {
        size_t size = sizeof(unsigned);
        extent_hooks_t *hooks = &huge_page_metadata_hooks;
        ret = mallctl(JEMALLOC_NEW_ARENA_CMD, &arena, &size, &hooks,
                      sizeof(hooks));
      } else
#endif
      {
        ret = mallctlRead(JEMALLOC_NEW_ARENA_CMD, &arena, true);
      }

      if (ret != 0) {
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
    assert(node_to_cpu_mask.size() == i);
    node_to_cpu_mask.push_back(cpus_for_node);
  }
  numa_bitmask_free(enabled);

  use_numa = true;
}

void set_numa_binding(int node) {
  if (!use_numa) return;

  s_numaNode = node;
  numa_sched_setaffinity(0, node_to_cpu_mask[node]);
  if (threads_bind_local) {
    numa_set_interleave_mask(numa_no_nodes_ptr);
    bitmask* nodes = numa_allocate_nodemask();
    numa_bitmask_setbit(nodes, node);
    numa_set_membind(nodes);
    numa_bitmask_free(nodes);

    int arena = base_arena + node;
    mallctlWrite("thread.arena", arena);
  }
}

void* mallocx_on_node(size_t size, int node, size_t align) {
  assertx((align & (align - 1)) == 0);
  int flags = MALLOCX_ALIGN(align);
  if (node < 0 || !use_numa) return mallocx(size, flags);
  int arena = base_arena + node;
#ifdef MALLOCX_TCACHE_NONE
  flags |= MALLOCX_ARENA(arena) | MALLOCX_TCACHE_NONE;
#else
  flags |= MALLOCX_ARENA(arena);
#endif
  return mallocx(size, flags);
}

#endif // HAVE_NUMA

#ifdef USE_JEMALLOC_EXTENT_HOOKS
static void set_arena_retain_grow_limit(unsigned id) {
  size_t mib[3];
  size_t miblen = sizeof(mib) / sizeof(size_t);

  if (mallctlnametomib("arena.0.retain_grow_limit", mib, &miblen) == 0) {
    // Limit grow_retained to reduce fragmentation on 1g pages.
    size_t grow_retained_limit = size2m;
    mib[1] = id;
    mallctlbymib(mib, miblen, nullptr, nullptr, &grow_retained_limit,
                 sizeof(grow_retained_limit));
  }
}

using namespace alloc;
static BumpMapper* getHugeMapperWithFallback(unsigned n1GPages,
                                             bool use2MFallback,
                                             short nextNode) {
#ifdef HAVE_NUMA
  const int max_node = numa_max_node();
#else
  constexpr int max_node = 0;
#endif
  BumpMapper* mapper = nullptr;
  BumpMapper* fallback = nullptr;
  if (max_node < 1) {
    // We either don't have libnuma, or run on a single-socket CPU.  In either
    // case, no need to worry about NUMA.
    fallback = new Bump4KMapper;
    if (n1GPages) {
      mapper = new Bump1GMapper(n1GPages);
    } else if (use2MFallback) {
      mapper = new Bump2MMapper;
    }
#ifdef HAVE_NUMA
  } else {
    fallback = new Bump4KMapper(numa_node_set);
    if (n1GPages) {
      mapper = new Bump1GMapper(n1GPages, numa_node_set, nextNode);
    } else {
      mapper = new Bump2MMapper(numa_node_set);
    }
#endif
  }
  if (mapper) {
    mapper->append(fallback);
  } else {
    mapper = fallback;
  }
  return mapper;
}

void setup_low_arena(unsigned n1GPages) {
  auto mapper = getHugeMapperWithFallback(n1GPages, true, 0);
  if (n1GPages == 0) {
    // If we are using 2M pages, save this so we can change how many 2M huge
    // pages to use.
    low_2m_mapper = dynamic_cast<Bump2MMapper*>(mapper);
  }
  auto ma = LowArena::CreateAt(&g_lowArena, kLowArenaMaxAddr,
                                   kLowArenaMaxCap, false, mapper);
  set_arena_retain_grow_limit(ma->id());
  low_arena = ma->id();
}

void setup_high_arena(unsigned n1GPages) {
  // If we use 1G huge pages on NUMA servers, start grabbing 1G huge pages from
  // a node different from the one for low arena.
  auto mapper = getHugeMapperWithFallback(n1GPages, false,
                                          num_numa_nodes() / 2 + 1);
  auto ma = HighArena::CreateAt(&g_highArena,
                                kHighArenaMaxAddr, kHighArenaMaxCap,
                                false, mapper);
  set_arena_retain_grow_limit(ma->id());
  high_arena = ma->id();
}

void* huge_page_extent_alloc(extent_hooks_t* extent_hooks, void* addr,
                             size_t size, size_t alignment, bool* zero,
                             bool* commit, unsigned arena_ind) {
  // This is used for arena 0's extent_alloc.  No malloc / free allowed within
  // this function since reentrancy is not supported for a0's extent hooks.

  // Note that, only metadata will use 2M alignment (size will be multiple of 2M
  // as well). Aligned allocation doesn't require alignment by default, because
  // of the way virtual memory is expanded with opt.retain (which is the
  // default).  The current extent hook API has no other way to tell if the
  // allocation is for metadata.  The next major jemalloc release will include
  // this information in the API.
  if (!jemallocMetadataCanUseHuge.load() || alignment != size2m) {
    goto default_alloc;
  }

  assert(a0ReservedBase != nullptr && (size & (size2m - 1)) == 0);
  if (arena_ind == 0) {
    size_t oldValue;
    while (size <= (oldValue = a0ReservedLeft.load())) {
      // Try placing a0 metadata on 1G huge pages.
      if (a0ReservedLeft.compare_exchange_weak(oldValue, oldValue - size)) {
        assert((oldValue & (size2m - 1)) == 0);
        return
          reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(a0ReservedBase) +
                                   (a0MetadataReservedSize - oldValue));
      }
    }
  } else if (auto ma = alloc::highArena()) {
    // For non arena 0: malloc / free allowed in this branch.
    void* ret = ma->extent_alloc(extent_hooks, addr, size, alignment, zero,
                                 commit, high_arena);
    if (ret != nullptr) return ret;
  }
default_alloc:
  return orig_alloc(extent_hooks, addr, size, alignment, zero,
                    commit, arena_ind);
}

/*
 * Customize arena 0's extent hook to use 1g pages for metadata.
 */
void setup_jemalloc_metadata_extent_hook(bool enable, bool enable_numa_arena,
                                         size_t reserved) {
  assert(!jemallocMetadataCanUseHuge.load());
#ifndef USE_JEMALLOC_METADATA_1G_PAGES
  return;
#endif
  enableArenaMetadata1GPage = enable;
  enableNumaArenaMetadata1GPage = enable_numa_arena;
  a0MetadataReservedSize = reserved;

  auto ma = alloc::highArena();
  if (!ma) return;
  bool retain_enabled = false;
  mallctlRead("opt.retain", &retain_enabled);
  if (!enableArenaMetadata1GPage || !retain_enabled) return;

  bool zero = true, commit = true;
  void* ret = ma->extent_alloc(nullptr, nullptr, a0MetadataReservedSize, size2m,
                               &zero, &commit, high_arena);
  if (!ret) return;

  a0ReservedBase = ret;
  a0ReservedLeft.store(a0MetadataReservedSize);

  extent_hooks_t* orig_hooks;
  int err = mallctlRead("arena.0.extent_hooks", &orig_hooks);
  if (err) return;

  orig_alloc = orig_hooks->alloc;
  huge_page_metadata_hooks = *orig_hooks;
  huge_page_metadata_hooks.alloc = &huge_page_extent_alloc;

  err = mallctlWrite("arena.0.extent_hooks", &huge_page_metadata_hooks);
  if (err) return;

  jemallocMetadataCanUseHuge.store(true);
}

#endif // USE_JEMALLOC_EXTENT_HOOKS
#endif // USE_JEMALLOC

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
#ifdef USE_JEMALLOC
#if !USE_JEMALLOC_EXTENT_HOOKS
    // Create the legacy low arena that uses brk() instead of mmap().  When
    // using newer versions of jemalloc, we use extent hooks to get more
    // control.
    if (mallctlRead(JEMALLOC_NEW_ARENA_CMD, &dss_arena, true) != 0) {
      // Error; bail out.
      return;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "arena.%u.dss", dss_arena);
    if (mallctlWrite(buf, "primary", true) != 0) {
      // Error; bail out.
      return;
    }

    // We normally maintain the invariant that the region surrounding the
    // current brk is mapped huge, but we don't know yet whether huge pages
    // are enabled for low memory. Round up to the start of a huge page,
    // and set the high water mark to one below.
    constexpr size_t kHugePageSize = size2m;
    constexpr size_t kHugePageMask = kHugePageSize - 1;
    unsigned leftInPage = kHugePageSize - (uintptr_t(sbrk(0)) & kHugePageMask);
    (void) sbrk(leftInPage);
    assert((uintptr_t(sbrk(0)) & kHugePageMask) == 0);

#else // USE_JEMALLOC_EXTENT_HOOKS
    unsigned low_1g_pages = 0;
    if (char* buffer = getenv("HHVM_LOW_1G_PAGE")) {
      if (!sscanf(buffer, "%u", &low_1g_pages)) {
        fprintf(stderr,
                "Bad environment variable HHVM_LOW_1G_PAGE: %s\n", buffer);
        abort();
      }
    }
    unsigned high_1g_pages = 0;
    if (char* buffer = getenv("HHVM_HIGH_1G_PAGE")) {
      if (!sscanf(buffer, "%u", &high_1g_pages)) {
        fprintf(stderr,
                "Bad environment variable HHVM_HIGH_1G_PAGE: %s\n", buffer);
        abort();
      }
    }

    HugePageInfo info = get_huge1g_info();
    unsigned remaining = static_cast<unsigned>(info.nr_hugepages);
    if (remaining == 0) {
      low_1g_pages = high_1g_pages = 0;
    } else if (low_1g_pages > 0 || high_1g_pages > 0) {
      KernelVersion version;
      if (version.m_major < 3 ||
          (version.m_major == 3 && version.m_minor < 9)) {
        // Older kernels need an explicit hugetlbfs mount point.
        find_hugetlbfs_path() || auto_mount_hugetlbfs();
      }
    }

    // Do some allocation between low and high 1G arenas.  We use at most 2 1G
    // pages for the low 1G arena; usually 1 is good enough.
    auto const origLow1G = low_1g_pages;
    auto const origHigh1G = high_1g_pages;
    if (low_1g_pages > 0) {
      if (low_1g_pages > 2) {
        low_1g_pages = 2;
      }
      if (low_1g_pages + high_1g_pages > remaining) {
        low_1g_pages = 1;
      }
      assert(remaining >= low_1g_pages);
      remaining -= low_1g_pages;
    }
    if (origLow1G) {
      fprintf(stderr,
              "using %u (specified %u) 1G huge pages for low arena\n",
              low_1g_pages, origLow1G);
    }
    setup_low_arena(low_1g_pages);

    if (high_1g_pages > remaining) {
      high_1g_pages = remaining;
    }
    if (origHigh1G) {
      fprintf(stderr,
              "using %u (specified %u) 1G huge pages for high arena\n",
              high_1g_pages, origHigh1G);
    }
    setup_high_arena(high_1g_pages);
#endif
#endif
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

#ifdef USE_JEMALLOC
void* low_malloc_impl(size_t size) {
  if (size == 0) return nullptr;
  void* ptr = mallocx(size, low_mallocx_flags());
#ifndef USE_LOWPTR
  if (!ptr) {
    if (size < size2m) {
#ifdef USE_JEMALLOC_EXTENT_HOOKS
      low_arena = 0;
#else
      dss_arena = 0;
#endif
    }
    return malloc(size);
  }
#endif
  return ptr;
}

#endif // USE_JEMALLOC

void low_malloc_huge_pages(int pages) {
#ifdef USE_JEMALLOC_EXTENT_HOOKS
  if (pages <= 0) return;
  if (low_2m_mapper) {
    low_2m_mapper->setMaxPages(pages);
  }
#endif
}

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
    bool enabled = false;
    bool active = false;
    // Check if profiling is active before trying to dump.
    int err = mallctlRead("opt.prof", &enabled, true) ||
      (enabled && mallctlRead("prof.active", &active, true));
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

#ifdef USE_JEMALLOC_EXTENT_HOOKS

void* malloc_huge_impl(size_t size) {
  if (size == 0) return nullptr;
  return mallocx(size, mallocx_huge_flags());
}

void high_arena_tcache_create() {
  assert(high_arena_tcache == -1);
  mallctlRead("tcache.create", &high_arena_tcache, true);
}

void high_arena_tcache_flush() {
  // It is OK if flushing fails
  if (MALLOCX_TCACHE(high_arena_tcache) != MALLOCX_TCACHE_NONE) {
    mallctlWrite("tcache.flush", high_arena_tcache, true);
  }
}

void high_arena_tcache_destroy() {
  if (MALLOCX_TCACHE(high_arena_tcache) != MALLOCX_TCACHE_NONE) {
    mallctlWrite("tcache.destroy", high_arena_tcache, true);
    high_arena_tcache = -1;
  }
}

#endif

///////////////////////////////////////////////////////////////////////////////
}

extern "C" {
  const char* malloc_conf = "narenas:1,lg_tcache_max:16"
#if (JEMALLOC_VERSION_MAJOR >= 5) && defined(FACEBOOK)
// FB-only as this feature does not exist in any jemalloc release as of
// 2017-11-02 (latest: 5.0.1)
    ",metadata_thp:disabled"
#endif
#ifdef ENABLE_HHPROF
    ",prof:true,prof_active:false,prof_thread_active_init:false"
#endif
    ;
}
