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
#include "hphp/runtime/base/memory-manager.h"

#include <algorithm>
#include <cstdint>
#include <limits>

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stack-logger.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/server/http-server.h"

#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/ptr-map.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include <folly/CPortability.h>
#include <folly/Random.h>
#include <folly/ScopeGuard.h>
#include <folly/portability/SysMman.h>
#include <folly/portability/Unistd.h>

#include "hphp/runtime/base/memory-manager-defs.h"

namespace HPHP {

__thread bool tl_sweeping;
THREAD_LOCAL_FLAT(MemoryManager, tl_heap);
__thread size_t tl_heap_id; // thread's current heap instance id

TRACE_SET_MOD(mm);

//////////////////////////////////////////////////////////////////////

std::atomic<MemoryManager::ReqProfContext*>
  MemoryManager::s_trigger{nullptr};

std::atomic<ssize_t> MemoryManager::s_req_heap_usage;

static std::atomic<size_t> s_heap_id; // global counter of heap instances

#ifdef USE_JEMALLOC
static size_t threadAllocatedpMib[2];
static size_t threadDeallocatedpMib[2];
#endif

#ifndef USE_JEMALLOC
__thread uint64_t MemoryManager::g_threadAllocated{0};
__thread uint64_t MemoryManager::g_threadDeallocated{0};
#endif

static bool threadStatsInit() {
#if USE_JEMALLOC
  if (!mallctlnametomib) return false;
  size_t miblen = sizeof(threadAllocatedpMib) / sizeof(size_t);
  if (mallctlnametomib("thread.allocatedp", threadAllocatedpMib, &miblen)) {
    return false;
  }
  miblen = sizeof(threadDeallocatedpMib) / sizeof(size_t);
  if (mallctlnametomib("thread.deallocatedp", threadDeallocatedpMib, &miblen)) {
    return false;
  }
#endif
  return true;
}

inline
void MemoryManager::threadStats(uint64_t*& allocated, uint64_t*& deallocated) {
  static auto const inited = threadStatsInit();
  if (!inited) return;

#ifdef USE_JEMALLOC
  size_t len = sizeof(allocated);
  if (mallctlbymib(threadAllocatedpMib,
                   sizeof(threadAllocatedpMib) / sizeof(size_t),
                   &allocated, &len, nullptr, 0)) {
    always_assert(false);
  }

  len = sizeof(deallocated);
  if (mallctlbymib(threadDeallocatedpMib,
                   sizeof(threadDeallocatedpMib) / sizeof(size_t),
                   &deallocated, &len, nullptr, 0)) {
    always_assert(false);
  }
#else
  allocated = &MemoryManager::g_threadAllocated;
  deallocated = &MemoryManager::g_threadDeallocated;
#endif
}

MemoryManager::MemoryManager() {
  threadStats(m_allocated, m_deallocated);
  rl_gcdata.getCheck();
  resetAllStats();
  setMemoryLimit(std::numeric_limits<int64_t>::max());
  resetGC(); // so each thread has unique req_num at startup
  m_bypassSlabAlloc = RuntimeOption::DisableSmallAllocator;
  m_req_start_micros = HPHP::Timer::GetThreadCPUTimeNanos() / 1000;
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request, "zend.enable_gc",
                   &m_gc_enabled);
}

MemoryManager::~MemoryManager() {
  FTRACE(1, "heap-id {} ~MM\n", tl_heap_id);
  // TODO(T20916887): Enable this for one-bit refcounting.
  if (debug) {
    // Check that every object in the heap is free.
    forEachHeapObject([&](HeapObject* h, size_t) {
        assert_flog(h->kind() == HeaderKind::Free,
                    "{} still live in ~MemoryManager()",
                    header_names[size_t(h->kind())]);
    });
  }
  // ~SparseHeap releases its slabs/bigs.
}

void MemoryManager::traceStats(const char* event) {
  FTRACE(1, "heap-id {} {} ", tl_heap_id, event);
  if (use_jemalloc) {
    FTRACE(1, "mm-usage {} extUsage {} ",
           m_stats.mmUsage(), m_stats.extUsage);
    FTRACE(1, "capacity {} peak usage {} peak capacity {} ",
           m_stats.capacity(), m_stats.peakUsage, m_stats.peakCap);
    FTRACE(1, "total {} reset alloc-dealloc {} cur alloc-dealloc {}\n",
           m_stats.totalAlloc, m_resetAllocated - m_resetDeallocated,
           *m_allocated - *m_deallocated);
    FTRACE(1, "freed on another thread {}\n", m_freedOnOtherThread);
  } else {
    FTRACE(1, "usage: {} capacity: {} peak usage: {} peak capacity: {}\n",
           m_stats.usage(), m_stats.capacity(),
           m_stats.peakUsage, m_stats.peakCap);
  }
}

// Reset all memory stats counters, both internal and external; intended to
// be used between requests when the whole heap is being reset.
void MemoryManager::resetAllStats() {
  traceStats("resetAllStats pre");
  m_statsIntervalActive = false;
  m_stats.mm_udebt = 0;
  m_stats.mm_uallocated = 0;
  m_stats.mm_freed = 0;
  m_stats.extUsage = 0;
  m_stats.malloc_cap = 0;
  m_stats.mmap_cap = 0;
  m_stats.mmap_volume = 0;
  m_stats.peakUsage = 0;
  m_stats.peakCap = 0;
  m_stats.totalAlloc = 0;
  m_stats.peakIntervalUsage = 0;
  m_stats.peakIntervalCap = 0;
  m_enableStatsSync = false;

  // Reset this memory managers portion of the bytes used across all memory
  // managers.
  s_req_heap_usage.fetch_sub(m_lastUsage, std::memory_order_relaxed);
  m_lastUsage = 0;

  if (Trace::enabled) tl_heap_id = ++s_heap_id;
  m_resetDeallocated = *m_deallocated;
  m_resetAllocated = *m_allocated;
  m_freedOnOtherThread = 0;
  traceStats("resetAllStats post");
}

// Reset external allocation counters, but preserve MemoryManager counters.
// The effect of this call is simply to ignore anything we've done *outside*
// the MemoryManager allocator after we initialized, to avoid attributing
// shared structure initialization that happens during hphp_thread_init()
// to this session. Intended to be used once per request, early in the request
// lifetime before PHP execution begins.
void MemoryManager::resetExternalStats() {
  traceStats("resetExternalStats pre");
  // extUsage and totalAlloc are only set by refreshStatsImpl, which we don't
  // enable until after this has been called.
  assertx(m_enableStatsSync ||
         (m_stats.extUsage == 0 && m_stats.totalAlloc == 0));
  m_enableStatsSync = true;
  m_resetDeallocated = *m_deallocated;
  // By subtracting malloc_cap here, the next call to refreshStatsImpl()
  // will correctly include m_stats.malloc_cap in extUsage and totalAlloc.
  m_resetAllocated = *m_allocated - m_stats.malloc_cap;
  // takeCreditForFreeOnOtherThread should not have been used yet
  assertx(m_freedOnOtherThread == 0);
  traceStats("resetExternalStats post");
}

void MemoryManager::refreshStatsHelperExceeded() {
  setSurpriseFlag(MemExceededFlag);
  RID().setRequestOOMFlag();
  m_couldOOM = false;
  if (Cfg::ErrorHandling::LogNativeStackOnOOM) {
    log_native_stack("Exceeded memory limit");
  }
  if (m_profctx.flag) {
    // Dump out heap profile at peak usage.
    jemalloc_pprof_dump("", false);
  }
}

void MemoryManager::setMemThresholdCallback(size_t threshold) {
  m_memThresholdCallbackPeakUsage = threshold;
}

/*
 * Refresh stats to reflect directly malloc()ed memory, and determine
 * whether the request memory limit has been exceeded.
 *
 * The stats parameter allows the updates to be applied to either
 * m_stats as in refreshStats() or to a separate MemoryUsageStats
 * struct as in getStatsCopy().
 */
void MemoryManager::refreshStatsImpl(MemoryUsageStats& stats) {
  // Incrementally incorporate the difference between the previous and current
  // deltas into the memory usage statistic.  For reference, the total
  // malloced memory usage could be calculated as such, if delta0 were
  // recorded in resetAllStats():
  //
  //   int64 musage = delta - delta0;
  //
  // Note however, the slab allocator adds to m_stats.malloc_cap
  // when it calls malloc(), so that this function can avoid
  // double-counting the malloced memory. Thus musage in the example
  // code may well substantially exceed m_stats.usage.
  if (m_enableStatsSync) {
    // We can't currently handle wrapping so make sure this isn't happening.
    assertx(*m_allocated <= uint64_t(std::numeric_limits<int64_t>::max()));
    assertx(*m_deallocated <= uint64_t(std::numeric_limits<int64_t>::max()));
    const int64_t curAllocated = *m_allocated;
    const int64_t curDeallocated = *m_deallocated;

    // Since these deltas potentially include memory allocated from another
    // thread but deallocated on this one, it is possible for these numbers to
    // go negative.
    auto curUsage = curAllocated - curDeallocated - m_freedOnOtherThread;
    auto resetUsage = m_resetAllocated - m_resetDeallocated;

    FTRACE(1, "heap-id {} Before stats sync: ", tl_heap_id);
    FTRACE(1, "reset alloc-dealloc {} cur alloc-dealloc: {} alloc-change: {} ",
      resetUsage, curUsage, curAllocated - m_resetAllocated);
    FTRACE(1, "dealloc-change: {} ",
      curDeallocated - m_resetDeallocated + m_freedOnOtherThread);
    FTRACE(1, "mm usage {} extUsage {} totalAlloc {} capacity {}\n",
      stats.mmUsage(), stats.extUsage, stats.totalAlloc, stats.capacity());

    // External usage (allocated-deallocated) since the last resetStats().
    stats.extUsage = curUsage - resetUsage;

    // Calculate the allocation volume since the last reset.
    // We need to do the calculation instead of just setting it to curAllocated
    // because of the MaskAlloc capability, which updates m_resetAllocated.

    // stats.mmap_volume is only used for mmap'd heap space; any malloc'd
    // space is included in curAllocated.
    stats.totalAlloc = curAllocated - m_resetAllocated + stats.mmap_volume;
    FTRACE(1, "heap-id {} after sync extUsage {} totalAlloc: {}\n",
      tl_heap_id, stats.extUsage, stats.totalAlloc);
  }
  assertx(m_usageLimit > 0);
  auto usage = stats.usage();
  stats.peakUsage = std::max(stats.peakUsage, usage);
  if (m_statsIntervalActive) {
    stats.peakIntervalUsage = std::max(stats.peakIntervalUsage, usage);
    stats.peakIntervalCap = std::max(stats.peakIntervalCap, stats.capacity());
  }
}

/*
 * Refresh our internally stored m_stats, then check for OOM and the
 * memThresholdCallback
 */
void MemoryManager::refreshStats() {
  refreshStatsImpl(m_stats);
  auto usage = m_stats.usage();
  s_req_heap_usage.fetch_add(usage - m_lastUsage, std::memory_order_relaxed);
  m_lastUsage = usage;
  if (usage > m_usageLimit && m_couldOOM) {
    refreshStatsHelperExceeded();
  } else if (usage >= 0 && usage > m_memThresholdCallbackPeakUsage) {
    m_memThresholdCallbackPeakUsage = SIZE_MAX;
    setSurpriseFlag(MemThresholdFlag);
  }
}

void MemoryManager::recordStats(StructuredLogEntry& entry) {
  auto const stats = getStatsCopy();
  entry.ints["mem-peak-usage"] =  stats.peakUsage;
  entry.ints["mem-peak-capacity"] = stats.peakCap;
  entry.ints["mem-total-alloc"] = stats.totalAlloc;
}

/*
 * Calculate how many bytes of allocation should happen before the next
 * time the fast path is interrupted.
 */
void MemoryManager::updateMMDebt() {
  const int64_t delta_sample = m_nextSample - m_stats.mmAllocated();
  const int64_t delta_gc = m_nextGC - m_stats.mmUsage();
  auto const delta = static_cast<uint64_t>(std::min(delta_sample, delta_gc));
  auto const new_debt = delta > std::numeric_limits<int64_t>::max() ? 0 : delta;
  m_stats.mm_uallocated += new_debt - m_stats.mm_udebt;
  m_stats.mm_udebt = new_debt;
}

void MemoryManager::sweep() {
  assertx(!sweeping());
  tl_sweeping = true;
  DEBUG_ONLY size_t num_sweepables = 0, num_natives = 0;

  // iterate until both sweep lists are empty. Entries can be added or
  // removed from either list during sweeping.
  do {
    while (!m_sweepables.empty()) {
      num_sweepables++;
      auto obj = m_sweepables.next();
      obj->unregister();
      obj->sweep();
    }
    while (!m_natives.empty()) {
      num_natives++;
      assertx(m_natives.back()->sweep_index == m_natives.size() - 1);
      auto node = m_natives.back();
      m_natives.pop_back();
      auto obj = Native::obj(node);
      auto ndi = obj->getVMClass()->getNativeDataInfo();
      ndi->sweep(obj);
      // trash the native data but leave the header and object parsable
      assertx(memset(node+1, kSmallFreeFill, node->obj_offset - sizeof(*node)));
    }
  } while (!m_sweepables.empty());

  if (debug) checkHeap("after MM::sweep");
}

void MemoryManager::resetAllocator() {
  assertx(m_natives.empty() && m_sweepables.empty() && tl_sweeping);

  // free the heap
  m_heap.reset();

  // zero out freelists
  for (auto& list : m_freelists) list.head = nullptr;
  m_front = m_limit = 0;
  tl_sweeping = false;
  m_exiting = false;
  resetAllStats();
  setGCEnabled(RuntimeOption::EvalEnableGC);
  resetGC();
  if (debug) resetEagerGC();
}

void MemoryManager::flush() {
  always_assert(empty());
  m_heap.flush();
  m_natives = std::vector<NativeNode*>();
  m_root_handles = std::vector<req::root_handle*>{};
}

/*
 * req::malloc & friends implementation notes
 *
 * There are three kinds of allocations:
 *
 *  a) Big allocations.  (size >= kMaxSmallSize)
 *
 *     In this case we behave as a wrapper around the normal libc
 *     malloc/free.  We insert a MallocNode header at the front of the
 *     allocation in order to find these at sweep time (end of
 *     request) so we can give them back to libc.
 *
 *  b) Size-tracked small allocations.
 *
 *     This is used for the generic case, for callers who can't tell
 *     us the size of the allocation at free time.
 *
 *     In this situation, we put a MallocNode header at the front of
 *     the block that tells us the size for when we need to free it
 *     later.  We differentiate this from a MallocNode using the size
 *     field in either structure (they overlap at the same address).
 *
 *  c) Size-untracked small allocation
 *
 *     Many callers have an easy time telling you how big the object
 *     was when they need to free it.  In this case we can avoid the
 *     MallocNode, which saves us some memory and also let's us give
 *     out 16-byte aligned pointers easily.
 *
 *     We know when we have one of these because it has to be freed
 *     through a different entry point.  (E.g. tl_heap->freeSmallSize() or
 *     tl_heap->freeBigSize().)
 *
 * When small blocks are freed (case b and c), they're placed in the
 * appropriate size-segregated freelist.  Large blocks are immediately
 * passed back to libc via free.
 *
 * There are currently two kinds of freelist entries: entries where
 * there is already a valid MallocNode on the list (case b), and
 * entries where there isn't (case c).  The reason for this is that
 * that way, when allocating for case b, you don't need to store the
 * MallocNode size again.  Much of the heap is going through case b at
 * the time of this writing, so it is a measurable regression to try
 * to just combine the free lists, but presumably we can move more to
 * case c and combine the lists eventually.
 */

constexpr const std::array<char*,NumHeaderKinds> header_names = {{
  "Vec", "BespokeVec", "Dict", "BespokeDict", "Keyset", "BespokeKeyset",
  "String", "Resource", "ClsMeth", "RClsMeth", "RFunc",
  "Object", "NativeObject", "WaitHandle", "AsyncFuncWH", "AwaitAllWH",
  "ConcurrentWH", "Closure", "Vector", "Map", "Set", "Pair", "ImmVector",
  "ImmMap", "ImmSet", "AsyncFuncFrame", "NativeData", "ClosureHdr", "MemoData",
  "Cpp", "SmallMalloc", "BigMalloc",
  "Free", "Hole", "Slab"
}};

// you already get a compiler error for having too many header_names--
// this should catch cases where there are too few header_names
static_assert(header_names[NumHeaderKinds - 1] != nullptr,
              "If you add a header kind, add an entry for it to header_names");

// initialize a Hole header in the unused memory between m_front and m_limit
void MemoryManager::initHole(void* ptr, uint32_t size) {
  FreeNode::InitFrom(ptr, size, HeaderKind::Hole);
}

void MemoryManager::initFree() {
  if ((char*)m_front < (char*)m_limit) {
    initHole(m_front, (char*)m_limit - (char*)m_front);
    Slab::fromPtr(m_front)->setCachedStart(m_front);
  }
  reinitFree();
}

void MemoryManager::reinitFree() {
  size_t i = 0;
  for (size_t e = m_bypassSlabAlloc ? 0 : kNumSmallSizes; i < e; i++) {
    auto size = sizeIndex2Size(i);
    auto n = m_freelists[i].head;

    for (; n; n = n->next) {
      auto const slab = Slab::fromPtr(n);
      if (n->kind() == HeaderKind::Free && slab->isCachedStart(n)) break;
      n->initHeader_32(HeaderKind::Free, size);
      slab->setCachedStart(n);
    }
    if (debug) {
      // ensure the freelist tail is already initialized.
      for (; n; n = n->next) {
        assertx(n->kind() == HeaderKind::Free && n->size() == size);
      }
    }
  }

  for (size_t e = m_freelists.size(); i < e; i++) {
    auto size = sizeIndex2Size(i);
    auto n = m_freelists[i].head;
    for (; n && n->kind() != HeaderKind::Free; n = n->next) {
      n->initHeader_32(HeaderKind::Free, size);
    }
    if (debug) {
      // ensure the freelist tail is already initialized.
      for (; n; n = n->next) {
        assertx(n->kind() == HeaderKind::Free && n->size() == size);
      }
    }
  }
  m_lastInitFreeAllocated = m_stats.mmAllocated();
  m_lastInitFreeFreed = m_stats.mm_freed;
}

MemoryManager::FreelistArray MemoryManager::beginQuarantine() {
  FreelistArray list;
  for (size_t i = 0, n = list.size(); i < n; ++i) {
    list[i].head = m_freelists[i].head;
    m_freelists[i].head = nullptr;
  }
  return list;
}

// turn free blocks into holes, restore original freelists
void MemoryManager::endQuarantine(FreelistArray&& list) {
  for (size_t i = 0, n = list.size(); i < n; i++) {
    auto size = sizeIndex2Size(i);
    while (auto n = m_freelists[i].likelyPop()) {
      memset(n, 0x8a, size);
      initHole(n, size);
    }
    m_freelists[i].head = list[i].head;
    list[i].head = nullptr;
  }
}

// test iterating objects in slabs
void MemoryManager::checkHeap(const char* phase) {
  size_t bytes=0;
  std::vector<HeapObject*> hdrs;
  PtrMap<HeapObject*> free_blocks;
  size_t counts[NumHeaderKinds];
  for (unsigned i=0; i < NumHeaderKinds; i++) counts[i] = 0;
  forEachHeapObject([&](HeapObject* h, size_t alloc_size) {
    hdrs.push_back(h);
    bytes += alloc_size;
    auto kind = h->kind();
    counts[(int)kind]++;
    switch (kind) {
      case HeaderKind::Free:
        free_blocks.insert(h, alloc_size);
        break;
      case HeaderKind::Vec:
      case HeaderKind::Dict:
      case HeaderKind::Keyset:
      case HeaderKind::Object:
      case HeaderKind::String:
      case HeaderKind::NativeObject:
      case HeaderKind::WaitHandle:
      case HeaderKind::AsyncFuncWH:
      case HeaderKind::AwaitAllWH:
      case HeaderKind::ConcurrentWH:
      case HeaderKind::Closure:
      case HeaderKind::Vector:
      case HeaderKind::Map:
      case HeaderKind::Set:
      case HeaderKind::Pair:
      case HeaderKind::ImmVector:
      case HeaderKind::ImmMap:
      case HeaderKind::ImmSet:
      case HeaderKind::Resource:
      case HeaderKind::ClsMeth:
      case HeaderKind::RClsMeth:
      case HeaderKind::AsyncFuncFrame:
      case HeaderKind::NativeData:
      case HeaderKind::ClosureHdr:
      case HeaderKind::MemoData:
      case HeaderKind::Cpp:
      case HeaderKind::SmallMalloc:
      case HeaderKind::BigMalloc:
      case HeaderKind::BespokeVec:
      case HeaderKind::BespokeDict:
      case HeaderKind::BespokeKeyset:
      case HeaderKind::RFunc:
        break;
      case HeaderKind::Hole:
      case HeaderKind::Slab:
        assertx(false && "forEachHeapObject skips these kinds");
        break;
    }
  });

  // check the free lists
  free_blocks.prepare();
  size_t num_free_blocks = 0;
  for (auto& list : m_freelists) {
    for (auto n = list.head; n; n = n->next) {
      ++num_free_blocks;
    }
  }
  assertx(num_free_blocks == free_blocks.size());

  // heap check is done. If we are not exiting, check pointers using HeapGraph
  if (Trace::moduleEnabled(Trace::heapreport)) {
    auto g = makeHeapGraph(true /* include free blocks */);
    if (!exiting()) checkPointers(g, phase);
    if (Trace::moduleEnabled(Trace::heapreport, 2)) {
      printHeapReport(g, phase);
    }
  }
}

// Filling the start bits one word at a time requires writing the mask for
// the appropriate size class, and left-shifting the mask each time to insert
// any necessary zeros not included in the previous word, for size classes
// that don't pack perfectly into 64 bits.  Suppose:
//
//   d = size / kSmallSizeAlign = number of bits for the given size class
//
// In other words, when d%64 != 0, we need to shift the mask slightly after
// each store, until the shift amount wraps. For example, using 8-bit words
// for brevity, the sequence of stores would be:
//
// 11111111 11111111 11111111 11111111          size=16 d=1 shift 0,0,0,0
// 10101010 10101010 10101010 10101010          size=32 d=2 shift 0,0,0,0
// 10010010 01001001 00100100 10010010          size=48 d=3 shift 0,1,2,0
// 10001000 10001000 10001000 10001000          size=64 d=4 shift 0,0,0,0
// 10000100 00100001 00001000 01000010 00010000 size=80 d=5 shift 0,2,4,1,3,0
// 10000010 00001000 00100000 10000010          size=96 d=6 shift 0,4,2,0
// 10000001 00000010 00000100 00001000 00010000 00100000 01000000 10000001
//                                          size=112 d=7 shift 0,6,5,4,3,2,1,0
// 10000000 10000000                        size=128 d=8 shift 0,0

// build a bitmask-init table for size classes that fit at least one
// object per 64*kSmallSizeAlign bytes; this means they fit at least
// one start bit per 64 bits, supporting fast nContig initialization.

// masks_[i] = bitmask to store each time
std::array<uint64_t,Slab::kNumMasks> Slab::masks_;

// shifts_[i] = how much to shift masks_[i] after each store
std::array<uint8_t,Slab::kNumMasks> Slab::shifts_;

struct Slab::InitMasks {
  InitMasks() {
    static_assert(kSizeIndex2Size[kNumMasks - 1] <= 64 * kSmallSizeAlign, "");
    for (size_t i = 0; i < kNumMasks; i++) {
      auto const d = kSizeIndex2Size[i] / kSmallSizeAlign;
      for (size_t j = 0; j < 64; j += d) {
        masks_[i] |= 1ull << j;
      }
      shifts_[i] = d - 64 % d; // # of high-order zeros not in mask
    }
  }
};

namespace {

Slab::InitMasks s_init_masks;

using FreelistArray = MemoryManager::FreelistArray;

alignas(64) constexpr size_t kContigTab[] = {
#define SIZE_CLASS(index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig) \
  ncontig * kSizeIndex2Size[index],
  SIZE_CLASSES
#undef SIZE_CLASS
};

alignas(64) const uint8_t kContigIndexTab[] = {
#define SIZE_CLASS(index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig) \
  (uint8_t)std::max(size_t(index+1),\
                    MemoryManager::size2Index(kContigTab[index])),
  SIZE_CLASSES
#undef SIZE_CLASS
};

/*
 * Store slab tail bytes (if any) in freelists.
 */
inline
void storeTail(FreelistArray& freelists, void* tail, size_t tailBytes) {
  void* rem = tail;
  for (auto remBytes = tailBytes; remBytes > 0;) {
    auto fragBytes = remBytes;
    assertx(fragBytes >= kSmallSizeAlign);
    assertx((fragBytes & kSmallSizeAlignMask) == 0);
    auto fragInd = MemoryManager::size2Index(fragBytes + 1) - 1;
    auto fragUsable = MemoryManager::sizeIndex2Size(fragInd);
    auto frag = (char*)rem + remBytes - fragUsable;
    FTRACE(4, "storeTail({}, {}): rem={}, remBytes={}, "
              "frag={}, fragBytes={}, fragUsable={}, fragInd={}\n", tail,
              (void*)uintptr_t(tailBytes), rem, (void*)uintptr_t(remBytes),
              frag, (void*)uintptr_t(fragBytes), (void*)uintptr_t(fragUsable),
              fragInd);
    freelists[fragInd].push(frag);
    remBytes -= fragUsable;
  }
}

/*
 * Create split_bytes worth of contiguous regions, each of size splitUsable,
 * and store them in the appropriate freelist.
 */
inline
void splitTail(FreelistArray& freelists, void* tail, size_t tailBytes,
               size_t split_bytes, size_t splitUsable, size_t index) {
  assertx(tailBytes >= kSmallSizeAlign);
  assertx((tailBytes & kSmallSizeAlignMask) == 0);
  assertx((splitUsable & kSmallSizeAlignMask) == 0);
  assertx(split_bytes <= tailBytes);

  // initialize the free objects, and push them onto the freelist.
  auto head = freelists[index].head;
  auto rem = (char*)tail + split_bytes;
  for (auto next = rem - splitUsable; next >= tail; next -= splitUsable) {
    auto split = next;
    FTRACE(4, "MemoryManager::splitTail(tail={}, tailBytes={}, tailPast={}): "
              "split={}, splitUsable={}\n", tail,
              (void*)uintptr_t(tailBytes), (void*)(uintptr_t(tail) + tailBytes),
              split, splitUsable);
    head = FreeNode::UninitFrom(split, head);
  }
  freelists[index].head = head;

  auto remBytes = tailBytes - split_bytes;
  assertx(uintptr_t(rem) + remBytes == uintptr_t(tail) + tailBytes);
  storeTail(freelists, rem, remBytes);
}
}

void MemoryManager::freeOveralloc(void* base, size_t bytes) {
  if (!bytes) return;
  m_stats.mm_udebt += bytes;
  storeTail(m_freelists, base, bytes);
}

/*
 * Get a new slab, then allocate nbytes from it and install it in our
 * slab list.  Return the newly allocated nbytes-sized block.
 */
NEVER_INLINE void* MemoryManager::newSlab(size_t nbytes) {
  refreshStats();
  if (m_front < m_limit) {
    storeTail(m_freelists, m_front, (char*)m_limit - (char*)m_front);
  }
  auto mem = m_heap.allocSlab(m_stats);
  always_assert(reinterpret_cast<uintptr_t>(mem) % kSlabAlign == 0);
  auto slab = static_cast<Slab*>(mem);
  auto slab_start = slab->init();
  m_front = slab_start + nbytes; // allocate requested object
  // we can't use any space after slab->end() even if the allocator allows
  // (indiciated by mem.size), because of the fixed-sized crossing map.
  m_limit = slab->end();
  assertx(m_front <= m_limit);
  FTRACE(3, "newSlab: adding slab at {} to limit {}\n", slab_start, m_limit);
  slab->setCachedStart(slab_start);
  return slab_start;
}

/*
 * Allocate `bytes' from the current slab, aligned to kSmallSizeAlign.
 */
inline void* MemoryManager::slabAlloc(size_t nbytes, size_t index) {
  FTRACE(3, "slabAlloc({}, {}): m_front={}, m_limit={}\n", nbytes, index,
            m_front, m_limit);
  assertx(nbytes == sizeIndex2Size(index));
  assertx((uintptr_t(m_front) & kSmallSizeAlignMask) == 0);

  auto ptr = m_front;
  auto next = (void*)(uintptr_t(ptr) + nbytes);
  if (uintptr_t(next) <= uintptr_t(m_limit)) {
    m_front = next;
  } else {
    if (UNLIKELY(index >= kNumSmallSizes) || UNLIKELY(m_bypassSlabAlloc)) {
      // Stats correction; mallocBigSize() updates m_stats. Add to mm_udebt
      // rather than mm_freed because we're adjusting for double-counting, not
      // actually freeing anything.
      m_stats.mm_udebt += nbytes;
      return mallocBigSize(nbytes);
    }
    ptr = newSlab(nbytes); // sets start bit at ptr
  }
  // Preallocate more of the same in order to amortize entry into this method.
  auto split_bytes = kContigTab[index] - nbytes;
  auto avail = uintptr_t(m_limit) - uintptr_t(m_front);
  if (UNLIKELY(split_bytes > avail)) {
    split_bytes = avail - avail % nbytes; // Expensive division.
  }
  if (split_bytes > 0) {
    auto tail = m_front;
    m_front = (void*)(uintptr_t(tail) + split_bytes);
    splitTail(m_freelists, tail, split_bytes, split_bytes, nbytes, index);
  }
  FTRACE(4, "slabAlloc({}, {}) --> ptr={}, m_front={}, m_limit={}\n", nbytes,
            index, ptr, m_front, m_limit);
  return ptr;
}

NEVER_INLINE
void* MemoryManager::mallocSmallSizeSlow(size_t nbytes, size_t index) {
  assertx(nbytes == sizeIndex2Size(index));

  if (UNLIKELY(m_stats.mm_udebt > std::numeric_limits<int64_t>::max())) {
    checkSampling(nbytes);
    // Must be here to check gc; might still have free objects.
    checkGC();
    updateMMDebt();
    auto clamped = std::min(index, kNumSmallSizes);
    if (auto p = m_freelists[clamped].unlikelyPop()) {
      FTRACE(3, "mallocSmallSizeSlow: check gc {} -> {}\n", nbytes, p);
      return p;
    }
  }

  size_t contigInd = kContigIndexTab[index];
  for (auto i = contigInd; i < kNumSmallSizes; ++i) {
    FTRACE(4, "MemoryManager::mallocSmallSizeSlow({}, {}): contigMin={}, "
              "contigInd={}, try i={}\n", nbytes, index, kContigTab[index],
              contigInd, i);
    if (auto p = m_freelists[i].unlikelyPop()) {
      assertx(i > index); // because freelist[index] was empty
      FTRACE(4, "MemoryManager::mallocSmallSizeSlow({}, {}): "
                "contigMin={}, contigInd={}, use i={}, size={}, p={}\n",
                nbytes, index, kContigTab[index], contigInd, i,
                sizeIndex2Size(i), p);
      // Split tail into preallocations and store them back into freelists.
      splitTail(m_freelists, (char*)p + nbytes, sizeIndex2Size(i) - nbytes,
                kContigTab[index] - nbytes, nbytes, index);
      return p;
    }
  }

  // No available free list items; carve new space from the current slab.
  return slabAlloc(nbytes, index);
}

inline void MemoryManager::updateBigStats() {
  // If we are using jemalloc, it is keeping track of allocations outside of
  // the slabs and the usage so we should force this after an allocation that
  // was too large for one of the existing slabs. When we're not using jemalloc
  // this check won't do anything so avoid the extra overhead.
  if (debug) requestEagerGC();
  refreshStats();
}

NEVER_INLINE
void MemoryManager::freeSmallIndexSlow(void* ptr, size_t index, size_t bytes) {
  if (UNLIKELY(index >= kNumSmallSizes) || UNLIKELY(m_bypassSlabAlloc)) {
    return freeBigSize(ptr);
  }
  // copy of FreeList::push() fast path when head == nullptr
  m_freelists[index].head = FreeNode::UninitFrom(ptr, nullptr);
  m_stats.mm_freed += bytes;
}

NEVER_INLINE
void* MemoryManager::mallocBigSize(size_t bytes, bool zero) {
  if (debug) tl_heap->requestEagerGC();
  auto ptr = m_heap.allocBig(bytes, zero, m_stats);
  updateBigStats();
  checkSampling(bytes);
  checkGC();
  FTRACE(3, "mallocBigSize: {} ({} requested)\n", ptr, bytes);
  return ptr;
}

MallocNode* MemoryManager::reallocBig(MallocNode* n, size_t nbytes) {
  assertx(n->kind() == HeaderKind::BigMalloc);
  auto n2 = static_cast<MallocNode*>(
    m_heap.resizeBig(n, nbytes, m_stats)
  );
  n2->nbytes = nbytes;
  updateBigStats();
  return n2;
}

NEVER_INLINE
void MemoryManager::freeBigSize(void* vp) {
  FTRACE(3, "freeBigSize: {}\n", vp);
  m_heap.freeBig(vp, m_stats);
}

// req::malloc api entry points, with support for malloc/free corner cases.
namespace req {

static void* allocate(size_t nbytes, bool zero, type_scan::Index ty) {
  nbytes = std::max(nbytes, size_t(1));
  auto const npadded = nbytes + sizeof(MallocNode);
  if (LIKELY(npadded <= kMaxSmallSize)) {
    auto const ptr = static_cast<MallocNode*>(
        tl_heap->mallocSmallSize(npadded)
    );
    ptr->initHeader_32_16(HeaderKind::SmallMalloc, 0, ty);
    ptr->nbytes = npadded;
    return zero ? memset(ptr + 1, 0, nbytes) : ptr + 1;
  }
  auto const ptr = static_cast<MallocNode*>(
    tl_heap->mallocBigSize(npadded, zero)
  );
  ptr->initHeader_32_16(HeaderKind::BigMalloc, 0, ty);
  ptr->nbytes = npadded;
  return ptr + 1;
}

void* malloc(size_t nbytes, type_scan::Index tyindex) {
  assertx(type_scan::isKnownType(tyindex));
  return allocate(nbytes, false, tyindex);
}

void* calloc(size_t count, size_t nbytes, type_scan::Index tyindex) {
  assertx(type_scan::isKnownType(tyindex));
  return allocate(count * nbytes, true, tyindex);
}

void* malloc_untyped(size_t nbytes) {
  auto n = static_cast<MallocNode*>(
    tl_heap->mallocBigSize(std::max(nbytes + sizeof(MallocNode), 1ul), false)
  );
  n->initHeader_32_16(HeaderKind::BigMalloc, 0, type_scan::kIndexUnknown);
  n->nbytes = nbytes + sizeof(MallocNode);
  return n + 1;
}

void* calloc_untyped(size_t count, size_t bytes) {
  auto nbytes = count * bytes + sizeof(MallocNode);
  auto n = static_cast<MallocNode*>(
    tl_heap->mallocBigSize(nbytes, true)
  );
  n->initHeader_32_16(HeaderKind::BigMalloc, 0, type_scan::kIndexUnknown);
  n->nbytes = nbytes;
  return n + 1;
}

void* realloc(void* ptr, size_t nbytes, type_scan::Index tyindex) {
  assertx(type_scan::isKnownType(tyindex));
  if (!ptr) {
    return allocate(nbytes, false, tyindex);
  }
  if (!nbytes) {
    req::free(ptr);
    return nullptr;
  }
  FTRACE(3, "MemoryManager::realloc: {} to {} [type_index: {}]\n",
         ptr, nbytes, tyindex);
  auto const n = static_cast<MallocNode*>(ptr) - 1;
  assertx(n->typeIndex() == tyindex);
  auto new_size = nbytes + sizeof(MallocNode);
  if (LIKELY(n->kind() == HeaderKind::SmallMalloc) ||
      UNLIKELY(new_size <= kMaxSmallSize)) {
    // either the old or new block will be small; force a copy.
    auto newmem = allocate(nbytes, false, tyindex);
    auto copy_size = std::min(n->nbytes - sizeof(MallocNode), nbytes);
    newmem = memcpy(newmem, ptr, copy_size);
    req::free(ptr);
    return newmem;
  }
  // it's a big allocation.
  auto n2 = tl_heap->reallocBig(n, new_size);
  return n2 + 1;
}

void* realloc_untyped(void* ptr, size_t nbytes) {
  // first handle corner cases that degenerate to malloc() or free()
  if (!ptr) {
    return req::malloc_untyped(nbytes);
  }
  if (!nbytes) {
    req::free(ptr);
    return nullptr;
  }
  FTRACE(3, "MemoryManager::realloc: {} to {} [type_index: {}]\n",
         ptr, nbytes, type_scan::kIndexUnknown);
  auto const n = static_cast<MallocNode*>(ptr) - 1;
  assertx(n->kind() == HeaderKind::BigMalloc);
  assertx(n->typeIndex() == type_scan::kIndexUnknown);
  auto n2 = tl_heap->reallocBig(n, nbytes + sizeof(MallocNode));
  return n2 + 1;
}

char* strndup(const char* str, size_t len) {
  size_t n = std::min(len, strlen(str));
  char* ret = reinterpret_cast<char*>(
    req::malloc(n + 1, type_scan::kIndexUnknownNoPtrs)
  );
  memcpy(ret, str, n);
  ret[n] = 0;
  return ret;
}

void free(void* ptr) {
  if (!ptr) return;
  auto const n = static_cast<MallocNode*>(ptr) - 1;
  if (LIKELY(n->kind() == HeaderKind::SmallMalloc)) {
    return tl_heap->freeSmallSize(n, n->nbytes);
  }
  if (n->kind() == HeaderKind::Cpp) {
    return tl_heap->objFree(n, n->nbytes);
  }
  assertx(n->kind() == HeaderKind::BigMalloc);
  tl_heap->freeBigSize(n);
}

} // namespace req

//////////////////////////////////////////////////////////////////////

void MemoryManager::addNativeObject(NativeNode* node) {
  if (debug) for (DEBUG_ONLY auto n : m_natives) assertx(n != node);
  node->sweep_index = m_natives.size();
  m_natives.push_back(node);
}

void MemoryManager::removeNativeObject(NativeNode* node) {
  assertx(node->sweep_index < m_natives.size());
  assertx(m_natives[node->sweep_index] == node);
  auto index = node->sweep_index;
  auto last = m_natives.back();
  m_natives[index] = last;
  m_natives.pop_back();
  last->sweep_index = index;
}

void MemoryManager::addSweepable(Sweepable* obj) {
  obj->enlist(&m_sweepables);
}

// defined here because memory-manager.h includes sweepable.h
Sweepable::Sweepable() {
  tl_heap->addSweepable(this);
}

//////////////////////////////////////////////////////////////////////

void MemoryManager::resetCouldOOM(bool state) {
  clearSurpriseFlag(MemExceededFlag);
  RID().clearRequestOOMFlag();
  m_couldOOM = state;
}


///////////////////////////////////////////////////////////////////////////////
// Request profiling.

bool MemoryManager::triggerProfiling(const std::string& filename) {
  auto trigger = new ReqProfContext();
  trigger->flag = true;
  trigger->filename = filename;

  ReqProfContext* expected = nullptr;

  if (!s_trigger.compare_exchange_strong(expected, trigger)) {
    delete trigger;
    return false;
  }
  return true;
}

void MemoryManager::requestInit() {
  tl_heap->m_req_start_micros = HPHP::Timer::GetThreadCPUTimeNanos() / 1000;

  if (RuntimeOption::EvalHeapAllocSampleRequests > 0 &&
      RuntimeOption::EvalHeapAllocSampleBytes > 0) {
    if (!folly::Random::rand32(RuntimeOption::EvalHeapAllocSampleRequests)) {
      tl_heap->m_nextSample =
        folly::Random::rand32(RuntimeOption::EvalHeapAllocSampleBytes);
    }
  }

  // If the trigger has already been claimed, do nothing.
  auto trigger = s_trigger.exchange(nullptr);
  if (trigger == nullptr) return;

  always_assert(tl_heap->empty());

  // Initialize the request-local context from the trigger.
  auto& profctx = tl_heap->m_profctx;
  assertx(!profctx.flag);

  always_assert(tl_heap->empty());
  tl_heap->m_bypassSlabAlloc = true;
  profctx = *trigger;
  delete trigger;

#ifdef USE_JEMALLOC
  // Reset jemalloc stats.
  if (mallctlCall<true>("prof.reset") != 0) {
    return;
  }

  // Enable jemalloc thread-local heap dumps.
  if (mallctlReadWrite<bool, true>("prof.active", &profctx.prof_active, true)
      != 0) {
    profctx = ReqProfContext{};
    return;
  }
  if (mallctlReadWrite<bool, true>("thread.prof.active",
                                   &profctx.thread_prof_active,
                                   true) != 0) {
    mallctlWrite("prof.active", profctx.prof_active);
    profctx = ReqProfContext{};
    return;
  }
#endif
}

void MemoryManager::requestShutdown() {
  if (tl_heap->m_nextSample != kNoNextSample) {
    reset_alloc_sampling();
    tl_heap->m_nextSample = kNoNextSample;
  }

  auto& profctx = tl_heap->m_profctx;
  if (!profctx.flag) return;

#ifdef USE_JEMALLOC
  jemalloc_pprof_dump(profctx.filename, true);

  mallctlWrite("thread.prof.active", profctx.thread_prof_active);
  mallctlWrite("prof.active", profctx.prof_active);
#endif

  always_assert(tl_heap->empty());
  tl_heap->m_bypassSlabAlloc = RuntimeOption::DisableSmallAllocator;
  tl_heap->m_memThresholdCallbackPeakUsage = SIZE_MAX;
  profctx = ReqProfContext{};
}

/* static */ void MemoryManager::setupProfiling() {
  always_assert(tl_heap->empty());
  tl_heap->m_bypassSlabAlloc = true;
}

/* static */ void MemoryManager::teardownProfiling() {
  always_assert(tl_heap->empty());
  tl_heap->m_bypassSlabAlloc = RuntimeOption::DisableSmallAllocator;
}

bool MemoryManager::isGCEnabled() {
  return m_gc_enabled;
}

void MemoryManager::setGCEnabled(bool isGCEnabled) {
  m_gc_enabled = isGCEnabled;
  updateNextGc();
}

void MemoryManager::debugFreeFill(void* ptr, size_t bytes) {
  if (bytes >= sizeof(FreeNode) + 16u) {
    auto node = static_cast<FreeNode*>(ptr);
    auto p = reinterpret_cast<uint64_t*>(node + 1);
    // try to remember the class of the object when applicable.
    auto const obj = innerObj(static_cast<HeapObject*>(ptr));
    if (obj) {
      *p = reinterpret_cast<uint64_t>(obj->getVMClass());
    } else {
      *p = *(reinterpret_cast<uint64_t*>(ptr) + 1);
    }
    *(p + 1) = *reinterpret_cast<uint64_t*>(ptr);
    memset(ptr, kSmallFreeFill, sizeof(FreeNode));
    memset(p + 2, kSmallFreeFill, bytes - sizeof(FreeNode) - 16);
  } else {
    memset(ptr, kSmallFreeFill, bytes);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
