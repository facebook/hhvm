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
#include <mutex>

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

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
#include "hphp/util/slab-manager.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void flush_thread_caches() {
#ifdef USE_JEMALLOC
  mallctlCall<true>("thread.tcache.flush");
#if USE_JEMALLOC_EXTENT_HOOKS
  arenas_thread_flush();
#endif
#endif
}

bool purge_all(std::string* errStr) {
#ifdef USE_JEMALLOC
  assert(mallctlnametomib && mallctlbymib);
  unsigned allArenas = 0;
#ifndef MALLCTL_ARENAS_ALL
  if (mallctlRead<unsigned, true>("arenas.narenas", &allArenas)) {
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
#endif
  return true;
}

__thread int32_t s_numaNode;

__thread uintptr_t s_stackLimit;
__thread size_t s_stackSize;
const size_t s_pageSize =  sysconf(_SC_PAGESIZE);

__thread MemBlock s_tlSpace;
__thread MemBlock s_hugeRange;

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
  uintptr_t top = get_stack_top() & (s_pageSize - 1);
  auto const hugeBase = reinterpret_cast<uintptr_t>(s_hugeRange.ptr);
  if (top > hugeBase) top = hugeBase;
  if (top <= s_stackLimit) return;
  size_t len = top - s_stackLimit;
  if (madvise((void*)s_stackLimit, len, MADV_DONTNEED) != 0 &&
      errno != EAGAIN) {
    fprintf(stderr, "%s failed to madvise with error %d\n", __func__, errno);
  }
}

#if !defined USE_JEMALLOC || !defined HAVE_NUMA
void enable_numa() {}
void set_numa_binding(int node) {}
void* mallocx_on_node(size_t size, int node, size_t align) {
  void* ret = nullptr;
  posix_memalign(&ret, align, size);
  return ret;
}
#endif

#ifdef USE_JEMALLOC
unsigned low_arena = 0;
unsigned lower_arena = 0;
unsigned low_cold_arena = 0;
unsigned high_arena = 0;
unsigned high_cold_arena = 0;
__thread unsigned local_arena = 0;

int low_arena_flags = 0;
int lower_arena_flags = 0;
int low_cold_arena_flags = 0;
int high_cold_arena_flags = 0;
__thread int high_arena_flags = 0;
__thread int local_arena_flags = 0;

// jemalloc frequently used mibs
size_t g_pactive_mib[4];                // "stats.arenas.<i>.pactive"
size_t g_epoch_mib[1];                  // "epoch"

void mallctl_epoch() {
  uint64_t epoch = 1;
  mallctlbymib(g_epoch_mib, 1, nullptr, nullptr, &epoch, sizeof(epoch));
}

size_t mallctl_pactive(unsigned arenaId) {
  size_t mib[4] =
    {g_pactive_mib[0], g_pactive_mib[1], arenaId, g_pactive_mib[3]};
  size_t pactive = 0;
  size_t sz = sizeof(pactive);
  if (mallctlbymib(mib, 4, &pactive, &sz, nullptr, 0)) return 0;
  return pactive;
}

#if USE_JEMALLOC_EXTENT_HOOKS
// Keep track of the size of recently freed memory that might be in the high1g
// arena when it is disabled, so that we know when to reenable it.
std::atomic_uint g_highArenaRecentlyFreed;

alloc::Bump2MMapper* low_2m_mapper = nullptr;
alloc::Bump2MMapper* high_2m_mapper = nullptr;

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

// Explicit per-thread tcache arenas needing it.
// In jemalloc/include/jemalloc/jemalloc_macros.h.in, we have
// #define MALLOCX_TCACHE_NONE MALLOCX_TCACHE(-1)
__thread int high_arena_tcache = -1;
__thread int local_arena_tcache = -1;
#endif

static unsigned base_arena;

#ifdef HAVE_NUMA

void enable_numa() {
  if (numa_available() < 0) return;
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
  if (node < 0) return;                 // thread not created from JobQueue
  s_numaNode = node;
  unsigned arena = base_arena + node;
  mallctlWrite("thread.arena", arena);

  if (use_numa) {
    numa_sched_setaffinity(0, node_to_cpu_mask[node]);
    numa_set_interleave_mask(numa_no_nodes_ptr);
    bitmask* nodes = numa_allocate_nodemask();
    numa_bitmask_setbit(nodes, node);
    numa_set_membind(nodes);
    numa_bitmask_free(nodes);
  }
}

void* mallocx_on_node(size_t size, int node, size_t align) {
  assert((align & (align - 1)) == 0);
  int flags = MALLOCX_ALIGN(align);
  if (node < 0) return mallocx(size, flags);
  int arena = base_arena + node;
#ifdef MALLOCX_TCACHE_NONE
  flags |= MALLOCX_ARENA(arena) | MALLOCX_TCACHE_NONE;
#else
  flags |= MALLOCX_ARENA(arena);
#endif
  return mallocx(size, flags);
}

#endif // HAVE_NUMA

#if USE_JEMALLOC_EXTENT_HOOKS
using namespace alloc;
static NEVER_INLINE
RangeMapper* getMapperChain(RangeState& range, unsigned n1GPages,
                            bool use2MPages, unsigned n2MPages,
                            bool useNormalPages,
                            int numaMask, short nextNode) {
  RangeMapper* head = nullptr;
  RangeMapper** ptail = &head;
  if (n1GPages) {
    RangeMapper::append(ptail,
                        new Bump1GMapper(range, n1GPages, numaMask, nextNode));
  }
  if (use2MPages) {
    RangeMapper::append(ptail, new Bump2MMapper(range, n2MPages, numaMask));
  }
  if (useNormalPages) {
    RangeMapper::append(ptail, new BumpNormalMapper(range, 0, numaMask));
  }
  assertx(head);
  return head;
}

void setup_low_arena(unsigned n1GPages) {
  assert(reinterpret_cast<uintptr_t>(sbrk(0)) <= kLowArenaMinAddr);
  // Initialize mappers for the VeryLow and Low address ranges.
  auto& veryLowRange = getRange(AddrRangeClass::VeryLow);
  auto& lowRange = getRange(AddrRangeClass::Low);
  auto veryLowMapper =
    getMapperChain(veryLowRange,
                   (n1GPages != 0) ? 1 : 0,
                   true, 0,                 // 2M
                   true,                    // 4K
                   numa_node_set, 0);
  auto lowMapper =
    getMapperChain(lowRange,
                   (n1GPages > 1) ? (n1GPages - 1) : 0,
                   true, 0,             // 2M
                   true,                // 4K
                   numa_node_set, 1);
  veryLowRange.setLowMapper(veryLowMapper);
  lowRange.setLowMapper(lowMapper);
  if (n1GPages == 0) {
    low_2m_mapper = dynamic_cast<Bump2MMapper*>(veryLowMapper);
  } else if (n1GPages == 1) {
    low_2m_mapper = dynamic_cast<Bump2MMapper*>(lowMapper);
  } else {
    low_2m_mapper = dynamic_cast<Bump2MMapper*>(lowMapper->next());
  }

  auto veryLowColdMapper =
    new BumpNormalMapper<Direction::HighToLow>(veryLowRange, 0, numa_node_set);
  auto lowColdMapper =
    new BumpNormalMapper<Direction::HighToLow>(lowRange, 0, numa_node_set);
  veryLowRange.setHighMapper(veryLowColdMapper);
  lowRange.setHighMapper(lowColdMapper);

  auto ma = LowArena::CreateAt(&g_lowArena);
  ma->appendMapper(lowMapper);
  ma->appendMapper(veryLowMapper);
  low_arena = ma->id();
  low_arena_flags = MALLOCX_ARENA(low_arena) | MALLOCX_TCACHE_NONE;

  ma = LowArena::CreateAt(&g_lowerArena);
  ma->appendMapper(veryLowMapper);
  ma->appendMapper(lowMapper);
  lower_arena = ma->id();
  lower_arena_flags = MALLOCX_ARENA(lower_arena) | MALLOCX_TCACHE_NONE;

  ma = LowArena::CreateAt(&g_lowColdArena);
  ma->appendMapper(lowColdMapper);
  ma->appendMapper(veryLowColdMapper);
  low_cold_arena = ma->id();
  low_cold_arena_flags = MALLOCX_ARENA(low_cold_arena) | MALLOCX_TCACHE_NONE;
}

void setup_high_arena(unsigned n1GPages) {
  auto& range = getRange(AddrRangeClass::Uncounted);
  auto mapper = getMapperChain(range, n1GPages,
                               true, 0, // 2M pages can be added later
                               true,    // use normal pages
                               numa_node_set,
                               num_numa_nodes() / 2 + 1);
  range.setLowMapper(mapper);
  if (n1GPages == 0) {
    high_2m_mapper = dynamic_cast<Bump2MMapper*>(mapper);
  } else {
    high_2m_mapper = dynamic_cast<Bump2MMapper*>(mapper->next());
  }
  auto coldMapper =
    new BumpNormalMapper<Direction::HighToLow>(range, 0, numa_node_set);
  range.setHighMapper(coldMapper);

  auto arena = HighArena::CreateAt(&g_highArena);
  arena->appendMapper(range.getLowMapper());
  high_arena = arena->id();

  auto coldArena = HighArena::CreateAt(&g_coldArena);
  coldArena->appendMapper(range.getHighMapper());
  high_cold_arena = coldArena->id();
  high_cold_arena_flags = MALLOCX_ARENA(high_cold_arena) | MALLOCX_TCACHE_NONE;
}

void setup_arena0(PageSpec s) {
  size_t size = size1g * s.n1GPages + size2m * s.n2MPages;
  if (size == 0) return;
  // Give arena 0 some huge pages, starting at 2TB.
  auto ret = mmap(reinterpret_cast<void*>(kArena0Base),
                  size + size1g, PROT_NONE,
                  MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE,
                  -1, 0);
  auto base = reinterpret_cast<uintptr_t>(ret);
  if (auto r = base % size1g) {         // align to 1G boundary
    base = base + size1g - r;
  }
  assertx(base % size1g == 0);

  auto a0 = PreMappedArena::AttachTo(low_malloc(sizeof(PreMappedArena)), 0,
                                     base, base + size, Reserved{});
  auto mapper = getMapperChain(*a0, s.n1GPages,
                               s.n2MPages, s.n2MPages,
                               false,
                               numa_node_set, 0);
  a0->setLowMapper(mapper);
}

// Set up extra arenas for use in non-VM threads, when we have short bursts of
// worker threads running, e.g., during deserialization of profile data.
static std::vector<std::pair<std::vector<DefaultArena*>,
                             std::atomic_uint*>> s_extra_arenas;
static unsigned s_extra_arena_per_node;
bool setup_extra_arenas(unsigned count) {
  if (count == 0) return false;
  // This may be called when we have many other threads running.  So hold the
  // lock while making changes.
  static std::mutex lock;
  std::lock_guard<std::mutex> g(lock);
  // only the first call allocate the arenas.
  if (!s_extra_arenas.empty()) {
    return count <= s_extra_arenas.size() * s_extra_arenas[0].first.size();
  }
  // `count` needs to be a multiple of `num_numa_nodes()`, if it isn't, we round
  // it up to make it easy to balance across nodes.
  auto const nNodes = std::max(1u, num_numa_nodes());
  s_extra_arena_per_node = (count + nNodes - 1) / nNodes;
  assert(s_extra_arena_per_node >= 1);
  s_extra_arenas.resize(nNodes);
  for (unsigned n = 0; n < nNodes; ++n) {
    s_extra_arenas[n].first.resize(s_extra_arena_per_node);
    auto constexpr kArenaSize =
      (sizeof(DefaultArena) + alignof(DefaultArena) - 1)
      / alignof(DefaultArena) * alignof(DefaultArena);
    auto const allocSize = kArenaSize * s_extra_arena_per_node
      + sizeof(std::atomic_uint);
    void* addr = mallocx_on_node(allocSize, n, alignof(DefaultArena));
    memset(addr, 0, allocSize);
    for (unsigned i = 0; i < s_extra_arena_per_node; ++i) {
      s_extra_arenas[n].first[i] = DefaultArena::CreateAt(addr);
      addr = (char*)addr + kArenaSize;
    }
    s_extra_arenas[n].second = static_cast<std::atomic_uint*>(addr);
  }
  return true;
}

DefaultArena* next_extra_arena(int node) {
  if (s_extra_arena_per_node == 0) return nullptr;
  if (node >= s_extra_arenas.size()) return nullptr;
  if (node < 0) node = 0;
  auto const n = static_cast<unsigned>(node);
  auto counter = s_extra_arenas[n].second;
  auto const next = counter->fetch_add(1, std::memory_order_relaxed);
  return s_extra_arenas[n].first[next % s_extra_arena_per_node];
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
#if !JEMALLOC_METADATA_1G_PAGES
  return;
#endif
  assert(!jemallocMetadataCanUseHuge.load());
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
  int err = mallctlRead<extent_hooks_t*, true>("arena.0.extent_hooks",
                                               &orig_hooks);
  if (err) return;

  orig_alloc = orig_hooks->alloc;
  huge_page_metadata_hooks = *orig_hooks;
  huge_page_metadata_hooks.alloc = &huge_page_extent_alloc;

  err = mallctlWrite<extent_hooks_t*, true>("arena.0.extent_hooks",
                                            &huge_page_metadata_hooks);
  if (err) return;

  jemallocMetadataCanUseHuge.store(true);
}

void arenas_thread_init() {
  if (high_arena_tcache == -1) {
    mallctlRead<int, true>("tcache.create", &high_arena_tcache);
    high_arena_flags =
      MALLOCX_ARENA(high_arena) | MALLOCX_TCACHE(high_arena_tcache);
  }
  if (local_arena_tcache == -1) {
    local_arena = get_local_arena(s_numaNode);
    if (local_arena) {
      mallctlRead<int, true>("tcache.create", &local_arena_tcache);
      local_arena_flags =
        MALLOCX_ARENA(local_arena) | MALLOCX_TCACHE(local_arena_tcache);
    }
  }
}

void arenas_thread_flush() {
  // It is OK if flushing fails
  if (high_arena_tcache != -1) {
    mallctlWrite<int, true>("tcache.flush", high_arena_tcache);
  }
  if (local_arena_tcache != -1) {
    mallctlWrite<int, true>("tcache.flush", local_arena_tcache);
  }
}

void arenas_thread_exit() {
  if (high_arena_tcache != -1) {
    mallctlWrite<int, true>("tcache.destroy", high_arena_tcache);
    high_arena_tcache = -1;
    // Ideally we shouldn't read high_arena_flags any more, but just in case.
    high_arena_flags = MALLOCX_ARENA(high_arena) | MALLOCX_TCACHE_NONE;
  }
  if (local_arena_tcache != -1) {
    mallctlWrite<int, true>("tcache.destroy", local_arena_tcache);
    local_arena_tcache = -1;
    // Ideally we shouldn't read high_arena_flags any more, but just in case.
    local_arena_flags = MALLOCX_ARENA(local_arena) | MALLOCX_TCACHE_NONE;
  }
}

#endif // USE_JEMALLOC_EXTENT_HOOKS

std::vector<unsigned> s_req_heap_arenas; // keyed by numa node id
std::vector<SlabManager*> s_slab_managers;
void setup_local_arenas(PageSpec spec, unsigned slabs) {
  s_req_heap_arenas.reserve(num_numa_nodes());
  s_slab_managers.reserve(num_numa_nodes());
  slabs /= num_numa_nodes();

  mallctlRead<unsigned>("arenas.narenas", &base_arena); // throw upon failure
  // The default one per node.
  for (int i = 0; i < num_numa_nodes(); i++) {
    unsigned arena = 0;
    mallctlRead<unsigned>(JEMALLOC_NEW_ARENA_CMD, &arena);
    always_assert(arena == base_arena + i);
    if (slabs) {
      auto mem = low_malloc(sizeof(SlabManager));
      s_slab_managers.push_back(new (mem) SlabManager);
    } else {
      s_slab_managers.push_back(nullptr);
    }
  }

#if USE_JEMALLOC_EXTENT_HOOKS
  spec.n1GPages = std::min(spec.n1GPages, get_huge1g_info().nr_hugepages);
  spec.n1GPages /= num_numa_nodes();
  spec.n2MPages = std::min(spec.n2MPages, get_huge2m_info().nr_hugepages);
  spec.n2MPages /= num_numa_nodes();
  const size_t reserveSize =
    spec.n1GPages * size1g + spec.n2MPages * size2m;
  if (reserveSize == 0) return;

  s_req_heap_arenas.resize(num_numa_nodes(), 0);
  for (unsigned i = 0; i < num_numa_nodes(); ++i) {
    static_assert(kLocalArenaMinAddr % size1g == 0, "");
    auto const desiredBase = kLocalArenaMinAddr + i * kLocalArenaSizeLimit;
    // Try to get the desired address range, but don't use MAP_FIXED.
    auto ret = mmap(reinterpret_cast<void*>(desiredBase),
                    reserveSize + size1g, PROT_NONE,
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE,
                    -1, 0);
    if (ret == MAP_FAILED) {
      throw std::runtime_error{"mmap() failed to reserve address range"};
    }
    auto base = reinterpret_cast<uintptr_t>(ret);
    if (base % size1g) {                // adjust to start at 1GB boundary
      auto const newBase = (base + size1g - 1) & ~(size1g - 1);
      munmap(reinterpret_cast<void*>(base), newBase - base);
      base = newBase;
    }
    assert(base % size1g == 0);
    auto arena = PreMappedArena::CreateAt(low_malloc(sizeof(PreMappedArena)),
                                          base, base + reserveSize, Reserved{});
    auto mapper = getMapperChain(*arena,
                                 spec.n1GPages,
                                 (bool)spec.n2MPages,
                                 spec.n2MPages,
                                 false,       // don't use normal pages
                                 1u << i,
                                 i);
    // Allocate some slabs first, which are not given to the arena, but managed
    // separately by the slab manager.
    auto const totalSlabSize = std::min(slabs * kSlabSize, reserveSize);
    if (totalSlabSize) {
      auto slabRange = mapper->alloc(totalSlabSize, kSlabAlign);
      if (slabRange) {
        s_slab_managers[i]->addRange<true>(slabRange, totalSlabSize);
      }
    }
    if (totalSlabSize == reserveSize) continue;
    arena->setLowMapper(mapper);
    s_req_heap_arenas[i] = arena->id();
  }
#endif
}

unsigned get_local_arena(uint32_t node) {
  if (node >= s_req_heap_arenas.size()) return 0;
  return s_req_heap_arenas[node];
}

SlabManager* get_local_slab_manager(uint32_t node) {
  if (node >= s_slab_managers.size()) return nullptr;
  return s_slab_managers[node];
}

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
    // allocations by pooling memory internally.
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
    // control.  If the mallctl fails, it will always_assert in mallctlHelper.
    if (mallctlRead<unsigned, true>(JEMALLOC_NEW_ARENA_CMD, &low_arena)) {
      return;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "arena.%u.dss", low_arena);
    if (mallctlWrite<const char*, true>(buf, "primary") != 0) {
      // Error; bail out.
      return;
    }
    low_arena_flags = MALLOCX_ARENA(low_arena);
#ifdef MALLOCX_TCACHE_NONE
    low_arena_flags |= MALLOCX_TCACHE_NONE;
    // Earlier versions of jemalloc do not have MALLOCX_TCACHE_NONE, but will
    // still bypass tcache when arena is specified.
#endif
    lower_arena = low_arena;
    lower_arena_flags = low_arena_flags;
    low_cold_arena = low_arena;
    low_cold_arena_flags = low_arena_flags;

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
    // Make sure high/low arenas are available to the current thread.
    arenas_thread_init();
#endif
    // Initialize global mibs
    size_t miblen = 1;
    mallctlnametomib("epoch", g_epoch_mib, &miblen);
    miblen = 4;
    mallctlnametomib("stats.arenas.0.pactive", g_pactive_mib, &miblen);
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

void low_2m_pages(uint32_t pages) {
#if USE_JEMALLOC_EXTENT_HOOKS
  if (low_2m_mapper) {
    low_2m_mapper->setMaxPages(pages);
  }
#endif
}

void high_2m_pages(uint32_t pages) {
#if USE_JEMALLOC_EXTENT_HOOKS
  if (high_2m_mapper) {
    high_2m_mapper->setMaxPages(pages);
  }
#endif
}

int jemalloc_pprof_enable() {
  return mallctlWrite<bool, true>("prof.active", true);
}

int jemalloc_pprof_disable() {
  return mallctlWrite<bool, true>("prof.active", false);
}

int jemalloc_pprof_dump(const std::string& prefix, bool force) {
  if (!force) {
    bool enabled = false;
    bool active = false;
    // Check if profiling is active before trying to dump.
    int err = mallctlRead<bool, true>("opt.prof", &enabled) ||
      (enabled && mallctlRead<bool, true>("prof.active", &active));
    if (err || !active) {
      return 0; // nothing to do
    }
  }

  if (prefix != "") {
    const char *s = prefix.c_str();
    return mallctlWrite<const char*, true>("prof.dump", s);
  } else {
    return mallctlCall<true>("prof.dump");
  }
}

///////////////////////////////////////////////////////////////////////////////
}

extern "C" {
  const char* malloc_conf = "narenas:1,lg_tcache_max:16"
#if (JEMALLOC_VERSION_MAJOR == 5 && JEMALLOC_VERSION_MINOR >= 1) || \
    (JEMALLOC_VERSION_MAJOR > 5) // requires jemalloc >= 5.1
    ",metadata_thp:disabled"
#endif
#ifdef ENABLE_HHPROF
    ",prof:true,prof_active:false,prof_thread_active_init:false"
#endif
    ;
}
