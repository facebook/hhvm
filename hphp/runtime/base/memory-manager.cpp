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
#include "hphp/runtime/base/memory-manager.h"

#include <algorithm>
#include <cstdint>
#include <limits>

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stack-logger.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/server/http-server.h"

#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"

#include <folly/Random.h>
#include <folly/ScopeGuard.h>
#include <folly/portability/SysMman.h>
#include <folly/portability/Unistd.h>

#include "hphp/runtime/base/memory-manager-defs.h"

namespace HPHP {

const unsigned kInvalidSweepIndex = 0xffffffff;

TRACE_SET_MOD(mm);

//////////////////////////////////////////////////////////////////////

std::atomic<MemoryManager::ReqProfContext*>
  MemoryManager::s_trigger{nullptr};

#ifdef USE_JEMALLOC
bool MemoryManager::s_statsEnabled = false;
size_t MemoryManager::s_cactiveLimitCeiling = 0;

static size_t threadAllocatedpMib[2];
static size_t threadDeallocatedpMib[2];
static size_t statsCactiveMib[2];
static pthread_once_t threadStatsOnce = PTHREAD_ONCE_INIT;

void MemoryManager::threadStatsInit() {
  if (!mallctlnametomib) return;
  size_t miblen = sizeof(threadAllocatedpMib) / sizeof(size_t);
  if (mallctlnametomib("thread.allocatedp", threadAllocatedpMib, &miblen)) {
    return;
  }
  miblen = sizeof(threadDeallocatedpMib) / sizeof(size_t);
  if (mallctlnametomib("thread.deallocatedp", threadDeallocatedpMib, &miblen)) {
    return;
  }
  miblen = sizeof(statsCactiveMib) / sizeof(size_t);
  if (mallctlnametomib("stats.cactive", statsCactiveMib, &miblen)) {
    return;
  }
  MemoryManager::s_statsEnabled = true;

  // In threadStats() we wish to solve for cactiveLimit in:
  //
  //   footprint + cactiveLimit + headRoom == MemTotal
  //
  // However, headRoom comes from RuntimeOption::ServerMemoryHeadRoom, which
  // isn't initialized until after the code here runs.  Therefore, compute
  // s_cactiveLimitCeiling here in order to amortize the cost of introspecting
  // footprint and MemTotal.
  //
  //   cactiveLimit == (MemTotal - footprint) - headRoom
  //
  //   cactiveLimit == s_cactiveLimitCeiling - headRoom
  // where
  //   s_cactiveLimitCeiling == MemTotal - footprint
  size_t footprint = Process::GetCodeFootprint(getpid());
  size_t MemTotal  = 0;
#ifndef __APPLE__
  size_t pageSize = size_t(sysconf(_SC_PAGESIZE));
  MemTotal = size_t(sysconf(_SC_PHYS_PAGES)) * pageSize;
#else
  int mib[2] = { CTL_HW, HW_MEMSIZE };
  u_int namelen = sizeof(mib) / sizeof(mib[0]);
  size_t len = sizeof(MemTotal);
  sysctl(mib, namelen, &MemTotal, &len, nullptr, 0);
#endif
  if (MemTotal > footprint) {
    MemoryManager::s_cactiveLimitCeiling = MemTotal - footprint;
  }
}

inline
void MemoryManager::threadStats(uint64_t*& allocated, uint64_t*& deallocated,
                                size_t*& cactive, size_t& cactiveLimit) {
  pthread_once(&threadStatsOnce, threadStatsInit);
  if (!MemoryManager::s_statsEnabled) return;

  size_t len = sizeof(allocated);
  if (mallctlbymib(threadAllocatedpMib,
                   sizeof(threadAllocatedpMib) / sizeof(size_t),
                   &allocated, &len, nullptr, 0)) {
    not_reached();
  }

  len = sizeof(deallocated);
  if (mallctlbymib(threadDeallocatedpMib,
                   sizeof(threadDeallocatedpMib) / sizeof(size_t),
                   &deallocated, &len, nullptr, 0)) {
    not_reached();
  }

  len = sizeof(cactive);
  if (mallctlbymib(statsCactiveMib,
                   sizeof(statsCactiveMib) / sizeof(size_t),
                   &cactive, &len, nullptr, 0)) {
    not_reached();
  }

  int64_t headRoom = RuntimeOption::ServerMemoryHeadRoom;
  // Compute cactiveLimit based on s_cactiveLimitCeiling, as computed in
  // threadStatsInit().
  if (headRoom != 0 && headRoom < MemoryManager::s_cactiveLimitCeiling) {
    cactiveLimit = MemoryManager::s_cactiveLimitCeiling - headRoom;
  } else {
    cactiveLimit = std::numeric_limits<size_t>::max();
  }
}
#endif

static void* MemoryManagerInit() {
  // We store the free list pointers right at the start of each
  // object (overlapping whatever it's first word holds), and we also clobber
  // _count as a free-object flag when the object is deallocated. This
  // assert just makes sure they don't overflow.
  assert(FAST_REFCOUNT_OFFSET + sizeof(int) <=
    MemoryManager::smallSizeClass(1));
  MemoryManager::TlsWrapper tls;
  return (void*)tls.getNoCheck;
}

void* MemoryManager::TlsInitSetup = MemoryManagerInit();

void MemoryManager::Create(void* storage) {
  new (storage) MemoryManager();
}

void MemoryManager::Delete(MemoryManager* mm) {
  mm->~MemoryManager();
}

void MemoryManager::OnThreadExit(MemoryManager* mm) {
  mm->~MemoryManager();
}

MemoryManager::MemoryManager() {
#ifdef USE_JEMALLOC
  threadStats(m_allocated, m_deallocated, m_cactive, m_cactiveLimit);
#endif
  resetStatsImpl(true);
  setMemoryLimit(std::numeric_limits<int64_t>::max());
  resetGC(); // so each thread has unique req_num at startup
  // make the circular-lists empty.
  m_strings.next = m_strings.prev = &m_strings;
  m_bypassSlabAlloc = RuntimeOption::DisableSmallAllocator;
}

MemoryManager::~MemoryManager() {
  dropRootMaps();
  if (debug) {
    // Check that every allocation in heap has been freed before destruction.
    forEachHeader([&](Header* h) {
      assert(h->kind() == HeaderKind::Free);
    });
  }
  // ~BigHeap releases its slabs/bigs.
}

void MemoryManager::dropRootMaps() {
  m_objectRoots = nullptr;
  m_resourceRoots = nullptr;
  for (auto r : m_root_handles) r->invalidate();
  m_root_handles.clear();
}

void MemoryManager::deleteRootMaps() {
  if (m_objectRoots) {
    req::destroy_raw(m_objectRoots);
    m_objectRoots = nullptr;
  }
  if (m_resourceRoots) {
    req::destroy_raw(m_resourceRoots);
    m_resourceRoots = nullptr;
  }
  for (auto r : m_root_handles) r->invalidate();
  m_root_handles.clear();
}

void MemoryManager::resetRuntimeOptions() {
  if (debug) {
    deleteRootMaps();
    checkHeap("resetRuntimeOptions");
    // check that every allocation in heap has been freed before reset
    iterate([&](Header* h) {
      assert(h->kind() == HeaderKind::Free);
    });
  }
  MemoryManager::TlsWrapper::destroy(); // ~MemoryManager()
  MemoryManager::TlsWrapper::getCheck(); // new MemeoryManager()
}

void MemoryManager::resetStatsImpl(bool isInternalCall) {
#ifdef USE_JEMALLOC
  FTRACE(1, "resetStatsImpl({}) pre:\n", isInternalCall);
  FTRACE(1, "usage: {}\ncapacity: {}\npeak usage: {}\npeak alloc: {}\n",
    m_stats.usage(), m_stats.capacity, m_stats.peakUsage,
    m_stats.peakCap);
  FTRACE(1, "total alloc: {}\nje alloc: {}\nje dealloc: {}\n",
    m_stats.totalAlloc, m_prevAllocated, m_prevDeallocated);
  FTRACE(1, "je debt: {}\n\n", m_stats.mallocDebt);
#else
  FTRACE(1, "resetStatsImpl({}) pre:\n"
    "usage: {}\ncapacity: {}\npeak usage: {}\npeak capacity: {}\n\n",
    isInternalCall, m_stats.usage(), m_stats.capacity, m_stats.peakUsage,
    m_stats.peakCap);
#endif
  if (isInternalCall) {
    m_statsIntervalActive = false;
    m_stats.mmUsage = 0;
    m_stats.auxUsage = 0;
    m_stats.capacity = 0;
    m_stats.peakUsage = 0;
    m_stats.peakCap = 0;
    m_stats.totalAlloc = 0;
    m_stats.peakIntervalUsage = 0;
    m_stats.peakIntervalCap = 0;
#ifdef USE_JEMALLOC
    m_enableStatsSync = false;
  } else if (!m_enableStatsSync) {
#else
  } else {
#endif
    // This is only set by the jemalloc stats sync which we don't enable until
    // after this has been called.
    assert(m_stats.totalAlloc == 0);

    // The effect of this call is simply to ignore anything we've done *outside*
    // the MemoryManager allocator after we initialized to avoid attributing
    // shared structure initialization that happens during hphp_thread_init()
    // to this session.

    // We don't want to clear the other values because we do already have some
    // small-sized allocator usage and live slabs and wiping now will result in
    // negative values when we try to reconcile our accounting with jemalloc.
#ifdef USE_JEMALLOC
    // Anything that was definitively allocated by the MemoryManager allocator
    // should be counted in this number even if we're otherwise zeroing out
    // the count for each thread.
    m_stats.totalAlloc = s_statsEnabled ? m_stats.mallocDebt : 0;

    m_enableStatsSync = s_statsEnabled;
#else
    m_stats.totalAlloc = 0;
#endif
  }
#ifdef USE_JEMALLOC
  if (s_statsEnabled) {
    m_stats.mallocDebt = 0;
    m_prevDeallocated = *m_deallocated;
    m_prevAllocated = *m_allocated;
  }
#endif
#ifdef USE_JEMALLOC
  FTRACE(1, "resetStatsImpl({}) post:\n", isInternalCall);
  FTRACE(1, "usage: {}\ncapacity: {}\npeak usage: {}\npeak capacity: {}\n",
    m_stats.usage(), m_stats.capacity, m_stats.peakUsage,
    m_stats.peakCap);
  FTRACE(1, "total alloc: {}\nje alloc: {}\nje dealloc: {}\n",
    m_stats.totalAlloc, m_prevAllocated, m_prevDeallocated);
  FTRACE(1, "je debt: {}\n\n", m_stats.mallocDebt);
#else
  FTRACE(1, "resetStatsImpl({}) post:\n"
    "usage: {}\ncapacity: {}\npeak usage: {}\npeak capacity: {}\n\n",
    isInternalCall, m_stats.usage(), m_stats.capacity,
    m_stats.peakUsage, m_stats.peakCap);
#endif
}

void MemoryManager::refreshStatsHelperExceeded() {
  setSurpriseFlag(MemExceededFlag);
  m_couldOOM = false;
  if (RuntimeOption::LogNativeStackOnOOM) {
    log_native_stack("Exceeded memory limit");
  }
}

void MemoryManager::setMemThresholdCallback(size_t threshold) {
  m_memThresholdCallbackPeakUsage = threshold;
}

#ifdef USE_JEMALLOC
void MemoryManager::refreshStatsHelperStop() {
  HttpServer::Server->stop();
  // Increase the limit to the maximum possible value, so that this method
  // won't be called again.
  m_cactiveLimit = std::numeric_limits<size_t>::max();
}
#endif

/*
 * Refresh stats to reflect directly malloc()ed memory, and determine
 * whether the request memory limit has been exceeded.
 *
 * The stats parameter allows the updates to be applied to either
 * m_stats as in refreshStats() or to a separate MemoryUsageStats
 * struct as in getStatsSafe().
 *
 * The template variable live controls whether or not MemoryManager
 * member variables are updated and whether or not to call helper
 * methods in response to memory anomalies.
 */
template<bool live>
void MemoryManager::refreshStatsImpl(MemoryUsageStats& stats) {
#ifdef USE_JEMALLOC
  // Incrementally incorporate the difference between the previous and current
  // deltas into the memory usage statistic.  For reference, the total
  // malloced memory usage could be calculated as such, if delta0 were
  // recorded in resetStatsImpl():
  //
  //   int64 musage = delta - delta0;
  //
  // Note however, the slab allocator adds to m_stats.mallocDebt
  // when it calls malloc(), so that this function can avoid
  // double-counting the malloced memory. Thus musage in the example
  // code may well substantially exceed m_stats.usage.
  if (m_enableStatsSync) {
    uint64_t jeDeallocated = *m_deallocated;
    uint64_t jeAllocated = *m_allocated;

    // We can't currently handle wrapping so make sure this isn't happening.
    assert(jeAllocated >= 0 &&
           jeAllocated <= std::numeric_limits<int64_t>::max());
    assert(jeDeallocated >= 0 &&
           jeDeallocated <= std::numeric_limits<int64_t>::max());

    // This is the delta between the current and the previous jemalloc reading.
    int64_t jeMMDeltaAllocated =
      int64_t(jeAllocated) - int64_t(m_prevAllocated);

    FTRACE(1, "Before stats sync:\n");
    FTRACE(1, "je alloc:\ncurrent: {}\nprevious: {}\ndelta with MM: {}\n",
      jeAllocated, m_prevAllocated, jeAllocated - m_prevAllocated);
    FTRACE(1, "je dealloc:\ncurrent: {}\nprevious: {}\ndelta with MM: {}\n",
      jeDeallocated, m_prevDeallocated, jeDeallocated - m_prevDeallocated);
    FTRACE(1, "usage: {}\ntotal (je) alloc: {}\nje debt: {}\n",
      stats.usage(), stats.totalAlloc, stats.mallocDebt);

    // Since these deltas potentially include memory allocated from another
    // thread but deallocated on this one, it is possible for these numbers to
    // go negative.
    int64_t jeDeltaAllocated =
      int64_t(jeAllocated) - int64_t(jeDeallocated);
    int64_t mmDeltaAllocated =
      int64_t(m_prevAllocated) - int64_t(m_prevDeallocated);
    FTRACE(1, "je delta:\ncurrent: {}\nprevious: {}\n",
        jeDeltaAllocated, mmDeltaAllocated);

    // Subtract the old jemalloc adjustment (delta0) and add the current one
    // (delta) to arrive at the new combined usage number.
    stats.auxUsage += jeDeltaAllocated - mmDeltaAllocated;
    // Remove the "debt" accrued from allocating the slabs so we don't double
    // count the slab-based allocations.
    stats.auxUsage -= stats.mallocDebt;

    stats.mallocDebt = 0;
    // We need to do the calculation instead of just setting it to jeAllocated
    // because of the MaskAlloc capability.
    stats.totalAlloc += jeMMDeltaAllocated;
    if (live) {
      m_prevAllocated = jeAllocated;
      m_prevDeallocated = jeDeallocated;
    }

    FTRACE(1, "After stats sync:\n");
    FTRACE(1, "usage: {}\ntotal (je) alloc: {}\n\n",
      stats.usage(), stats.totalAlloc);
  }
#endif
  assert(stats.limit > 0);
  if (live && stats.usage() > stats.limit && m_couldOOM) {
    refreshStatsHelperExceeded();
  }
  if (stats.usage() > stats.peakUsage) {
    // Check whether the process's active memory limit has been exceeded, and
    // if so, stop the server.
    //
    // Only check whether the total memory limit was exceeded if this request
    // is at a new high water mark.  This check could be performed regardless
    // of this request's current memory usage (because other request threads
    // could be to blame for the increased memory usage), but doing so would
    // measurably increase computation for little benefit.
#ifdef USE_JEMALLOC
    // (*m_cactive) consistency is achieved via atomic operations.  The fact
    // that we do not use an atomic operation here means that we could get a
    // stale read, but in practice that poses no problems for how we are
    // using the value.
    if (live && s_statsEnabled && *m_cactive > m_cactiveLimit) {
      refreshStatsHelperStop();
    }
#endif
    if (live &&
        stats.usage() > m_memThresholdCallbackPeakUsage &&
        stats.peakUsage <= m_memThresholdCallbackPeakUsage) {
      setSurpriseFlag(MemThresholdFlag);
    }

    stats.peakUsage = stats.usage();
  }
  if (live && m_statsIntervalActive) {
    stats.peakIntervalUsage = std::max(stats.peakIntervalUsage, stats.usage());
    stats.peakIntervalCap = std::max(stats.peakIntervalCap, stats.capacity);
  }
}

template void MemoryManager::refreshStatsImpl<true>(MemoryUsageStats& stats);
template void MemoryManager::refreshStatsImpl<false>(MemoryUsageStats& stats);

void MemoryManager::sweep() {
  assert(!sweeping());
  m_sweeping = true;
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
      assert(m_natives.back()->sweep_index == m_natives.size() - 1);
      auto node = m_natives.back();
      m_natives.pop_back();
      auto obj = Native::obj(node);
      auto ndi = obj->getVMClass()->getNativeDataInfo();
      ndi->sweep(obj);
      // trash the native data but leave the header and object parsable
      assert(memset(node+1, kSmallFreeFill, node->obj_offset - sizeof(*node)));
    }
  } while (!m_sweepables.empty());

  DEBUG_ONLY auto napcs = m_apc_arrays.size();
  FTRACE(1, "sweep: sweepable {} native {} apc array {}\n",
         num_sweepables,
         num_natives,
         napcs);

  // decref apc arrays referenced by this request.  This must happen here
  // (instead of in resetAllocator), because the sweep routine may use
  // g_context.
  while (!m_apc_arrays.empty()) {
    auto a = m_apc_arrays.back();
    m_apc_arrays.pop_back();
    a->sweep();
    if (debug) a->m_sweep_index = kInvalidSweepIndex;
  }

  if (debug) checkHeap("after MM::sweep");
}

void MemoryManager::resetAllocator() {
  assert(m_natives.empty() && m_sweepables.empty() && m_sweeping);
  // decref apc strings referenced by this request
  DEBUG_ONLY auto nstrings = StringData::sweepAll();

  // cleanup root maps
  dropRootMaps();

  // free the heap
  m_heap.reset();

  // zero out freelists
  for (auto& i : m_freelists) i.head = nullptr;
  m_front = m_limit = 0;
  m_sweeping = false;
  m_exiting = false;
  resetStatsImpl(true);
  resetGC();
  FTRACE(1, "reset: strings {}\n", nstrings);
  if (debug) resetEagerGC();
}

void MemoryManager::flush() {
  always_assert(empty());
  m_heap.flush();
  m_apc_arrays = std::vector<APCLocalArray*>();
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
 *     through a different entry point.  (E.g. MM().freeSmallSize() or
 *     MM().freeBigSize().)
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

const std::array<char*,NumHeaderKinds> header_names = {
  "PackedArray", "MixedArray", "EmptyArray", "ApcArray",
  "GlobalsArray", "ProxyArray", "DictArray", "VecArray", "KeysetArray",
  "String", "Resource", "Ref",
  "Object", "WaitHandle", "AsyncFuncWH", "AwaitAllWH", "Closure",
  "Vector", "Map", "Set", "Pair", "ImmVector", "ImmMap", "ImmSet",
  "AsyncFuncFrame", "NativeData", "ClosureHdr",
  "SmallMalloc", "BigMalloc", "BigObj",
  "Free", "Hole"
};

// initialize a Hole header in the unused memory between m_front and m_limit
void MemoryManager::initHole(void* ptr, uint32_t size) {
  auto hdr = static_cast<FreeNode*>(ptr);
  hdr->hdr.kind = HeaderKind::Hole;
  hdr->size() = size;
}

void MemoryManager::initHole() {
  if ((char*)m_front < (char*)m_limit) {
    initHole(m_front, (char*)m_limit - (char*)m_front);
  }
}

// initialize the FreeNode header on all freelist entries.
void MemoryManager::initFree() {
  initHole();
  for (auto i = 0; i < kNumSmallSizes; i++) {
    auto size = smallIndex2Size(i);
    auto n = m_freelists[i].head;
    for (; n && n->hdr.kind != HeaderKind::Free; n = n->next) {
      n->hdr.init(HeaderKind::Free, size);
    }
    if (debug) {
      // ensure the freelist tail is already initialized.
      for (; n; n = n->next) {
        assert(n->hdr.kind == HeaderKind::Free && n->size() == size);
      }
    }
  }
}

void MemoryManager::beginQuarantine() {
  std::swap(m_freelists, m_quarantine);
}

// turn free blocks into holes, restore original freelists
void MemoryManager::endQuarantine() {
  for (auto i = 0; i < kNumSmallSizes; i++) {
    auto size = smallIndex2Size(i);
    while (auto n = m_freelists[i].maybePop()) {
      memset(n, 0x8a, size);
      static_cast<FreeNode*>(n)->hdr.init(HeaderKind::Hole, size);
    }
  }
  std::swap(m_freelists, m_quarantine);
}

// test iterating objects in slabs
void MemoryManager::checkHeap(const char* phase) {
  size_t bytes=0;
  std::vector<Header*> hdrs;
  PtrMap free_blocks, apc_arrays, apc_strings;
  size_t counts[NumHeaderKinds];
  for (unsigned i=0; i < NumHeaderKinds; i++) counts[i] = 0;
  forEachHeader([&](Header* h) {
    hdrs.push_back(&*h);
    bytes += h->size();
    counts[(int)h->kind()]++;
    switch (h->kind()) {
      case HeaderKind::Free:
        free_blocks.insert(h);
        break;
      case HeaderKind::Apc:
        if (h->apc_.m_sweep_index != kInvalidSweepIndex) {
          apc_arrays.insert(h);
        }
        break;
      case HeaderKind::String:
        if (h->str_.isProxy()) apc_strings.insert(h);
        break;
      case HeaderKind::Packed:
      case HeaderKind::Mixed:
      case HeaderKind::Dict:
      case HeaderKind::Empty:
      case HeaderKind::VecArray:
      case HeaderKind::Keyset:
      case HeaderKind::Globals:
      case HeaderKind::Proxy:
      case HeaderKind::Object:
      case HeaderKind::WaitHandle:
      case HeaderKind::AsyncFuncWH:
      case HeaderKind::AwaitAllWH:
      case HeaderKind::Closure:
      case HeaderKind::Vector:
      case HeaderKind::Map:
      case HeaderKind::Set:
      case HeaderKind::Pair:
      case HeaderKind::ImmVector:
      case HeaderKind::ImmMap:
      case HeaderKind::ImmSet:
      case HeaderKind::Resource:
      case HeaderKind::Ref:
      case HeaderKind::AsyncFuncFrame:
      case HeaderKind::NativeData:
      case HeaderKind::ClosureHdr:
      case HeaderKind::SmallMalloc:
      case HeaderKind::BigMalloc:
        break;
      case HeaderKind::BigObj:
      case HeaderKind::Hole:
        assert(false && "forEachHeader skips these kinds");
        break;
    }
  });

  // check the free lists
  free_blocks.prepare();
  size_t num_free_blocks = 0;
  for (auto i = 0; i < kNumSmallSizes; i++) {
    for (auto n = m_freelists[i].head; n; n = n->next) {
      assert(free_blocks.isHeader(n));
      ++num_free_blocks;
    }
  }
  assert(num_free_blocks == free_blocks.size());

  // check the apc array list
  assert(apc_arrays.size() == m_apc_arrays.size());
  apc_arrays.prepare();
  for (UNUSED auto a : m_apc_arrays) {
    assert(apc_arrays.isHeader(a));
  }

  // check the apc string list
  size_t num_apc_strings = 0;
  apc_strings.prepare();
  for (StringDataNode *next, *n = m_strings.next; n != &m_strings; n = next) {
    next = n->next;
    UNUSED auto const s = StringData::node2str(n);
    assert(s->isProxy());
    assert(apc_strings.isHeader(s));
    ++num_apc_strings;
  }
  assert(num_apc_strings == apc_strings.size());

  // heap check is done. If we are not exiting, check pointers using HeapGraph
  if (Trace::moduleEnabled(Trace::heapreport)) {
    auto g = makeHeapGraph(true /* include free blocks */);
    if (!exiting()) checkPointers(g, phase);
    if (Trace::moduleEnabled(Trace::heapreport, 2)) {
      printHeapReport(g, phase);
    }
  }
}

/*
 * Store slab tail bytes (if any) in freelists.
 */
inline void MemoryManager::storeTail(void* tail, uint32_t tailBytes) {
  void* rem = tail;
  for (uint32_t remBytes = tailBytes; remBytes > 0;) {
    uint32_t fragBytes = remBytes;
    assert(fragBytes >= kSmallSizeAlign);
    assert((fragBytes & kSmallSizeAlignMask) == 0);
    unsigned fragInd = smallSize2Index(fragBytes + 1) - 1;
    uint32_t fragUsable = smallIndex2Size(fragInd);
    auto frag = FreeNode::InitFrom((char*)rem + remBytes - fragUsable,
                                   fragUsable, HeaderKind::Hole);
    FTRACE(4, "MemoryManager::storeTail({}, {}): rem={}, remBytes={}, "
              "frag={}, fragBytes={}, fragUsable={}, fragInd={}\n", tail,
              (void*)uintptr_t(tailBytes), rem, (void*)uintptr_t(remBytes),
              frag, (void*)uintptr_t(fragBytes), (void*)uintptr_t(fragUsable),
              fragInd);
    m_freelists[fragInd].push(frag, fragUsable);
    remBytes -= fragUsable;
  }
}

/*
 * Create nSplit contiguous regions and store them in the appropriate freelist.
 */
inline void MemoryManager::splitTail(void* tail, uint32_t tailBytes,
                                     unsigned nSplit, uint32_t splitUsable,
                                     unsigned splitInd) {
  assert(tailBytes >= kSmallSizeAlign);
  assert((tailBytes & kSmallSizeAlignMask) == 0);
  assert((splitUsable & kSmallSizeAlignMask) == 0);
  assert(nSplit * splitUsable <= tailBytes);
  assert(splitUsable == smallIndex2Size(splitInd));
  for (uint32_t i = nSplit; i--;) {
    auto split = FreeNode::InitFrom((char*)tail + i * splitUsable,
                                    splitUsable, HeaderKind::Hole);
    FTRACE(4, "MemoryManager::splitTail(tail={}, tailBytes={}, tailPast={}): "
              "split={}, splitUsable={}, splitInd={}\n", tail,
              (void*)uintptr_t(tailBytes), (void*)(uintptr_t(tail) + tailBytes),
              split, splitUsable, splitInd);
    m_freelists[splitInd].push(split, splitUsable);
  }
  void* rem = (void*)(uintptr_t(tail) + nSplit * splitUsable);
  assert(tailBytes >= nSplit * splitUsable);
  uint32_t remBytes = tailBytes - nSplit * splitUsable;
  assert(uintptr_t(rem) + remBytes == uintptr_t(tail) + tailBytes);
  storeTail(rem, remBytes);
}

/*
 * Get a new slab, then allocate nbytes from it and install it in our
 * slab list.  Return the newly allocated nbytes-sized block.
 */
NEVER_INLINE void* MemoryManager::newSlab(uint32_t nbytes) {
  if (UNLIKELY(m_stats.usage() > m_stats.limit)) {
    refreshStats();
  }
  requestGC();
  storeTail(m_front, (char*)m_limit - (char*)m_front);
  auto slab = m_heap.allocSlab(kSlabSize);
  assert((uintptr_t(slab.ptr) & kSmallSizeAlignMask) == 0);
  m_stats.mallocDebt += slab.size;
  m_stats.capacity += slab.size;
  m_stats.peakCap = std::max(m_stats.peakCap, m_stats.capacity);
  m_front = (void*)(uintptr_t(slab.ptr) + nbytes);
  m_limit = (void*)(uintptr_t(slab.ptr) + slab.size);
  FTRACE(3, "newSlab: adding slab at {} to limit {}\n", slab.ptr, m_limit);
  return slab.ptr;
}

/*
 * Allocate `bytes' from the current slab, aligned to kSmallSizeAlign.
 */
inline void* MemoryManager::slabAlloc(uint32_t bytes, unsigned index) {
  FTRACE(3, "slabAlloc({}, {}): m_front={}, m_limit={}\n", bytes, index,
            m_front, m_limit);
  uint32_t nbytes = smallIndex2Size(index);

  assert(bytes <= nbytes);
  assert(nbytes <= kSlabSize);
  assert((nbytes & kSmallSizeAlignMask) == 0);
  assert((uintptr_t(m_front) & kSmallSizeAlignMask) == 0);

  if (UNLIKELY(m_bypassSlabAlloc)) {
    // Stats correction; mallocBigSize() pulls stats from jemalloc.
    m_stats.mmUsage -= bytes;
    return mallocBigSize<FreeRequested>(nbytes).ptr;
  }

  void* ptr = m_front;
  {
    void* next = (void*)(uintptr_t(ptr) + nbytes);
    if (uintptr_t(next) <= uintptr_t(m_limit)) {
      m_front = next;
    } else {
      ptr = newSlab(nbytes);
    }
  }
  // Preallocate more of the same in order to amortize entry into this method.
  unsigned nSplit = kNContigTab[index] - 1;
  uintptr_t avail = uintptr_t(m_limit) - uintptr_t(m_front);
  if (UNLIKELY(nSplit * nbytes > avail)) {
    nSplit = avail / nbytes; // Expensive division.
  }
  if (nSplit > 0) {
    void* tail = m_front;
    uint32_t tailBytes = nSplit * nbytes;
    m_front = (void*)(uintptr_t(m_front) + tailBytes);
    splitTail(tail, tailBytes, nSplit, nbytes, index);
  }
  FTRACE(4, "slabAlloc({}, {}) --> ptr={}, m_front={}, m_limit={}\n", bytes,
            index, ptr, m_front, m_limit);
  return ptr;
}

void* MemoryManager::mallocSmallSizeSlow(uint32_t bytes, unsigned index) {
  size_t nbytes = smallIndex2Size(index);
  unsigned nContig = kNContigTab[index];
  size_t contigMin = nContig * nbytes;
  unsigned contigInd = smallSize2Index(contigMin);
  for (unsigned i = contigInd; i < kNumSmallSizes; ++i) {
    FTRACE(4, "MemoryManager::mallocSmallSizeSlow({}-->{}, {}): contigMin={}, "
              "contigInd={}, try i={}\n", bytes, nbytes, index, contigMin,
              contigInd, i);
    void* p = m_freelists[i].maybePop();
    if (p != nullptr) {
      FTRACE(4, "MemoryManager::mallocSmallSizeSlow({}-->{}, {}): "
                "contigMin={}, contigInd={}, use i={}, size={}, p={}\n", bytes,
                nbytes, index, contigMin, contigInd, i, smallIndex2Size(i),
                p);
      // Split tail into preallocations and store them back into freelists.
      uint32_t availBytes = smallIndex2Size(i);
      uint32_t tailBytes = availBytes - nbytes;
      if (tailBytes > 0) {
        void* tail = (void*)(uintptr_t(p) + nbytes);
        splitTail(tail, tailBytes, nContig - 1, nbytes, index);
      }
      return p;
    }
  }

  // No available free list items; carve new space from the current slab.
  return slabAlloc(bytes, index);
}

inline void MemoryManager::updateBigStats() {
  // If we are using jemalloc, it is keeping track of allocations outside of
  // the slabs and the usage so we should force this after an allocation that
  // was too large for one of the existing slabs. When we're not using jemalloc
  // this check won't do anything so avoid the extra overhead.
  if (debug) requestEagerGC();
  if (use_jemalloc || UNLIKELY(m_stats.usage() > m_stats.limit)) {
    refreshStats();
  }
}

template<MemoryManager::MBS Mode> NEVER_INLINE
MemBlock MemoryManager::mallocBigSize(size_t bytes, HeaderKind kind,
                                      type_scan::Index ty) {
  if (debug) MM().requestEagerGC();
  auto block = Mode == ZeroFreeActual ? m_heap.callocBig(bytes, kind, ty) :
               m_heap.allocBig(bytes, kind, ty);
  // NB: We don't report the SweepNode size in the stats.
  auto const delta = Mode == FreeRequested ? bytes : block.size;
  m_stats.mmUsage += delta;
  // Adjust jemalloc otherwise we'll double count the direct allocation.
  m_stats.mallocDebt += delta;
  m_stats.capacity += block.size + sizeof(MallocNode);
  updateBigStats();
  FTRACE(3, "mallocBigSize: {} ({} requested, {} usable)\n",
         block.ptr, bytes, block.size);
  return block;
}

template NEVER_INLINE
MemBlock MemoryManager::mallocBigSize<MemoryManager::FreeRequested>(
    size_t, HeaderKind, type_scan::Index
);
template NEVER_INLINE
MemBlock MemoryManager::mallocBigSize<MemoryManager::FreeActual>(
    size_t, HeaderKind, type_scan::Index
);
template NEVER_INLINE
MemBlock MemoryManager::mallocBigSize<MemoryManager::ZeroFreeActual>(
    size_t, HeaderKind, type_scan::Index
);

MemBlock MemoryManager::resizeBig(MallocNode* n, size_t nbytes) {
  auto old_size = n->nbytes - sizeof(MallocNode);
  auto block = m_heap.resizeBig(n + 1, nbytes);
  m_stats.mmUsage += block.size - old_size;
  m_stats.mallocDebt += block.size - old_size;
  m_stats.capacity += block.size - old_size;
  updateBigStats();
  return block;
}

NEVER_INLINE
void MemoryManager::freeBigSize(void* vp, size_t bytes) {
  // Since we account for these direct allocations in our usage and adjust for
  // them on allocation, we also need to adjust for them negatively on free.
  m_stats.mmUsage -= bytes;
  m_stats.mallocDebt -= bytes;
  auto actual = static_cast<MallocNode*>(vp)[-1].nbytes;
  assert(bytes <= actual);
  m_stats.capacity -= actual;
  FTRACE(3, "freeBigSize: {} ({} bytes)\n", vp, bytes);
  m_heap.freeBig(vp);
}

// req::malloc api entry points, with support for malloc/free corner cases.
namespace req {

template<bool zero>
static void* allocate(size_t nbytes, type_scan::Index ty) {
  nbytes = std::max(nbytes, size_t(1));
  auto const npadded = nbytes + sizeof(MallocNode);
  if (LIKELY(npadded <= kMaxSmallSize)) {
    auto const ptr = static_cast<MallocNode*>(MM().mallocSmallSize(npadded));
    ptr->nbytes = npadded;
    ptr->hdr.init(ty, HeaderKind::SmallMalloc, 0);
    return zero ? memset(ptr + 1, 0, nbytes) : ptr + 1;
  }
  auto constexpr mode = zero ? MemoryManager::ZeroFreeActual :
                        MemoryManager::FreeActual;
  auto block = MM().mallocBigSize<mode>(nbytes, HeaderKind::BigMalloc, ty);
  return block.ptr;
}

void* malloc(size_t nbytes, type_scan::Index tyindex) {
  return allocate<false>(nbytes, tyindex);
}

void* calloc(size_t count, size_t nbytes, type_scan::Index tyindex) {
  return allocate<true>(count * nbytes, tyindex);
}

void* realloc(void* ptr, size_t nbytes, type_scan::Index tyindex) {
  // first handle corner cases that degenerate to malloc() or free()
  if (!ptr) {
    return req::malloc(nbytes, tyindex);
  }
  if (!nbytes) {
    req::free(ptr);
    return nullptr;
  }
  FTRACE(3, "MemoryManager::realloc: {} to {} [type_index: {}]\n",
         ptr, nbytes, tyindex);
  auto const n = static_cast<MallocNode*>(ptr) - 1;
  if (LIKELY(n->nbytes <= kMaxSmallSize)) {
    // old block was small, cannot resize.
    auto newmem = req::malloc(nbytes, tyindex);
    auto copy_size = std::min(n->nbytes - sizeof(MallocNode), nbytes);
    newmem = memcpy(newmem, ptr, copy_size);
    MM().freeSmallSize(n, n->nbytes);
    return newmem;
  }
  // Ok, it's a big allocation.
  auto block = MM().resizeBig(n, nbytes);
  return block.ptr;
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
  if (LIKELY(n->nbytes <= kMaxSmallSize)) {
    return MM().freeSmallSize(n, n->nbytes);
  }
  MM().freeBigSize(ptr, n->nbytes - sizeof(MallocNode));
}

} // namespace req

//////////////////////////////////////////////////////////////////////

void MemoryManager::addNativeObject(NativeNode* node) {
  if (debug) for (DEBUG_ONLY auto n : m_natives) assert(n != node);
  node->sweep_index = m_natives.size();
  m_natives.push_back(node);
}

void MemoryManager::removeNativeObject(NativeNode* node) {
  assert(node->sweep_index < m_natives.size());
  assert(m_natives[node->sweep_index] == node);
  auto index = node->sweep_index;
  auto last = m_natives.back();
  m_natives[index] = last;
  m_natives.pop_back();
  last->sweep_index = index;
}

void MemoryManager::addApcArray(APCLocalArray* a) {
  a->m_sweep_index = m_apc_arrays.size();
  m_apc_arrays.push_back(a);
}

void MemoryManager::removeApcArray(APCLocalArray* a) {
  assert(a->m_sweep_index < m_apc_arrays.size());
  assert(m_apc_arrays[a->m_sweep_index] == a);
  auto index = a->m_sweep_index;
  auto last = m_apc_arrays.back();
  m_apc_arrays[index] = last;
  m_apc_arrays.pop_back();
  last->m_sweep_index = index;
}

void MemoryManager::addSweepable(Sweepable* obj) {
  obj->enlist(&m_sweepables);
}

// defined here because memory-manager.h includes sweepable.h
Sweepable::Sweepable() {
  MM().addSweepable(this);
}

//////////////////////////////////////////////////////////////////////

void MemoryManager::resetCouldOOM(bool state) {
  clearSurpriseFlag(MemExceededFlag);
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
  auto trigger = s_trigger.exchange(nullptr);

  // If the trigger has already been claimed, do nothing.
  if (trigger == nullptr) return;

  always_assert(MM().empty());

  // Initialize the request-local context from the trigger.
  auto& profctx = MM().m_profctx;
  assert(!profctx.flag);

  MM().m_bypassSlabAlloc = true;
  profctx = *trigger;
  delete trigger;

#ifdef USE_JEMALLOC
  // Reset jemalloc stats.
  if (mallctlCall("prof.reset", true) != 0) {
    return;
  }

  // Enable jemalloc thread-local heap dumps.
  if (mallctlReadWrite("prof.active", &profctx.prof_active, true, true)
      != 0) {
    profctx = ReqProfContext{};
    return;
  }
  if (mallctlReadWrite("thread.prof.active", &profctx.thread_prof_active,
                       true, true) != 0) {
    mallctlWrite("prof.active", profctx.prof_active);
    profctx = ReqProfContext{};
    return;
  }
#endif
}

void MemoryManager::requestShutdown() {
  auto& profctx = MM().m_profctx;

  if (!profctx.flag) return;

#ifdef USE_JEMALLOC
  jemalloc_pprof_dump(profctx.filename, true);

  mallctlWrite("thread.prof.active", profctx.thread_prof_active);
  mallctlWrite("prof.active", profctx.prof_active);
#endif

  MM().m_bypassSlabAlloc = RuntimeOption::DisableSmallAllocator;
  MM().m_memThresholdCallbackPeakUsage = SIZE_MAX;
  profctx = ReqProfContext{};
}

/* static */ void MemoryManager::setupProfiling() {
  always_assert(MM().empty());
  MM().m_bypassSlabAlloc = true;
}

/* static */ void MemoryManager::teardownProfiling() {
  MM().m_bypassSlabAlloc = RuntimeOption::DisableSmallAllocator;
}

///////////////////////////////////////////////////////////////////////////////

void BigHeap::reset() {
  TRACE(1, "BigHeap-reset: slabs %lu bigs %lu\n", m_slabs.size(),
        m_bigs.size());
#ifdef USE_JEMALLOC
  auto do_free = [&](void* ptr) { dallocx(ptr, 0); };
#else
  auto do_free = [&](void* ptr) { free(ptr); };
#endif
  for (auto slab : m_slabs) do_free(slab.ptr);
  m_slabs.clear();
  for (auto n : m_bigs) do_free(n);
  m_bigs.clear();
}

void BigHeap::flush() {
  assert(empty());
  m_slabs = std::vector<MemBlock>{};
  m_bigs = std::vector<MallocNode*>{};
}

MemBlock BigHeap::allocSlab(size_t size) {
#ifdef USE_JEMALLOC
  void* slab = mallocx(size, 0);
#else
  void* slab = safe_malloc(size);
#endif
  m_slabs.push_back({slab, size});
  return {slab, size};
}

void BigHeap::enlist(MallocNode* n, HeaderKind kind,
                     size_t size, type_scan::Index tyindex) {
  n->nbytes = size;
  n->hdr.kind = kind;
  n->index() = m_bigs.size();
  n->typeIndex() = tyindex;
  m_bigs.push_back(n);
}

MemBlock BigHeap::allocBig(size_t bytes, HeaderKind kind,
                           type_scan::Index tyindex) {
#ifdef USE_JEMALLOC
  auto n = static_cast<MallocNode*>(mallocx(bytes + sizeof(MallocNode), 0));
  auto cap = sallocx(n, 0);
#else
  auto cap = bytes + sizeof(MallocNode);
  auto n = static_cast<MallocNode*>(safe_malloc(cap));
#endif
  enlist(n, kind, cap, tyindex);
  return {n + 1, cap - sizeof(MallocNode)};
}

MemBlock BigHeap::callocBig(size_t nbytes, HeaderKind kind,
                            type_scan::Index tyindex) {
#ifdef USE_JEMALLOC
  auto n = static_cast<MallocNode*>(
      mallocx(nbytes + sizeof(MallocNode), MALLOCX_ZERO)
  );
  auto cap = sallocx(n, 0);
#else
  auto cap = nbytes + sizeof(MallocNode);
  auto const n = static_cast<MallocNode*>(safe_calloc(cap, 1));
#endif
  enlist(n, kind, cap, tyindex);
  return {n + 1, nbytes};
}

bool BigHeap::contains(void* ptr) const {
  auto const ptrInt = reinterpret_cast<uintptr_t>(ptr);
  auto it = std::find_if(std::begin(m_slabs), std::end(m_slabs),
    [&] (MemBlock slab) {
      auto const baseInt = reinterpret_cast<uintptr_t>(slab.ptr);
      return ptrInt >= baseInt && ptrInt < baseInt + slab.size;
    }
  );
  return it != std::end(m_slabs);
}

void BigHeap::freeBig(void* ptr) {
  auto n = static_cast<MallocNode*>(ptr) - 1;
  auto i = n->index();
  auto last = m_bigs.back();
  last->index() = i;
  m_bigs[i] = last;
  m_bigs.pop_back();
#ifdef USE_JEMALLOC
  dallocx(n, 0);
#else
  free(n);
#endif
}

MemBlock BigHeap::resizeBig(void* ptr, size_t newsize) {
  // Since we don't know how big it is (i.e. how much data we should memcpy),
  // we have no choice but to ask malloc to realloc for us.
  auto const n = static_cast<MallocNode*>(ptr) - 1;
#ifdef USE_JEMALLOC
  auto const newNode = static_cast<MallocNode*>(
    rallocx(n, newsize + sizeof(MallocNode), 0)
  );
  n->nbytes = sallocx(newNode, 0);
#else
  auto const newNode = static_cast<MallocNode*>(
    safe_realloc(n, newsize + sizeof(MallocNode))
  );
  n->nbytes = newsize + sizeof(MallocNode);
#endif
  if (newNode != n) {
    m_bigs[newNode->index()] = newNode;
  }
  return {newNode + 1, newsize};
}

/*
 * To find `p', we sort the slabs, bisect them, then iterate the slab
 * containing `p'.  If there is no such slab, we bisect the bigs to try to find
 * a big containing `p'.
 *
 * If that fails, we return nullptr.
 */
Header* BigHeap::find(const void* p) {
  std::sort(std::begin(m_slabs), std::end(m_slabs),
    [] (const MemBlock& l, const MemBlock& r) {
      assertx(static_cast<char*>(l.ptr) + l.size <= r.ptr ||
              static_cast<char*>(r.ptr) + r.size <= l.ptr);
      return l.ptr < r.ptr;
    }
  );

  auto const slab = std::lower_bound(
    std::begin(m_slabs), std::end(m_slabs), p,
    [] (const MemBlock& slab, const void* p) {
      return static_cast<const char*>(slab.ptr) + slab.size <= p;
    }
  );

  if (slab != std::end(m_slabs) && slab->ptr <= p) {
    // std::lower_bound() finds the first slab that is not less than `p'.  By
    // our comparison predicate, a slab is less than `p' iff its entire range
    // is below `p', so if the returned slab's start address is <= `p', then
    // the slab must contain `p'.  Within the slab, we just do a linear search.
    auto const slab_end = static_cast<char*>(slab->ptr) + slab->size;
    auto h = reinterpret_cast<char*>(slab->ptr);
    while (h < slab_end) {
      auto const hdr = reinterpret_cast<Header*>(h);
      auto const size = hdr->allocSize();
      if (p < h + size) return hdr;
      h += size;
    }
    // We know `p' is in the slab, so it must belong to one of the headers.
    always_assert(false);
  }

  std::sort(std::begin(m_bigs), std::end(m_bigs));

  auto const big = std::lower_bound(
    std::begin(m_bigs), std::end(m_bigs), p,
    [] (const MallocNode* big, const void* p) {
      return reinterpret_cast<const char*>(big) + big->nbytes <= p;
    }
  );

  if (big != std::end(m_bigs) && *big <= p) {
    auto const hdr = reinterpret_cast<Header*>(*big);
    if (hdr->kind() != HeaderKind::BigObj) {
      // `p' is part of the MallocNode.
      return hdr;
    } else {
      auto const sub = reinterpret_cast<Header*>(*big + 1);
      auto const start = reinterpret_cast<const char*>(sub);
      return start <= p && p < start + sub->size()
        ? sub   // `p' is part of the allocated object.
        : hdr;  // `p' is part of the MallocNode.
    }
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
