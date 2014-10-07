/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/memory-profile.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"
#include "folly/ScopeGuard.h"
#ifdef FACEBOOK
#include "folly/experimental/symbolizer/StackTrace.h"
#include "folly/experimental/symbolizer/Symbolizer.h"
#endif

namespace HPHP {

TRACE_SET_MOD(smartalloc);

//////////////////////////////////////////////////////////////////////

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
  size_t footprint = Process::GetCodeFootprint(Process::GetProcessId());
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
  // object, overlapping SmartHeader.data, and we also clobber _count
  // as a free-object flag when the object is deallocated.  This
  // assert just makes sure they don't overflow.
  assert(FAST_REFCOUNT_OFFSET + sizeof(int) <=
    MemoryManager::smartSizeClass(1));
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

MemoryManager::MemoryManager()
    : m_front(nullptr)
    , m_limit(nullptr)
    , m_sweeping(false) {
#ifdef USE_JEMALLOC
  threadStats(m_allocated, m_deallocated, m_cactive, m_cactiveLimit);
#endif
  resetStatsImpl(true);
  m_stats.maxBytes = std::numeric_limits<int64_t>::max();
  // make the circular-lists empty.
  m_sweep.next = m_sweep.prev = &m_sweep;
  m_strings.next = m_strings.prev = &m_strings;
}

void MemoryManager::resetStatsImpl(bool isInternalCall) {
#ifdef USE_JEMALLOC
  FTRACE(1, "resetStatsImpl({}) pre:\n", isInternalCall);
  FTRACE(1, "usage: {}\nalloc: {}\npeak usage: {}\npeak alloc: {}\n",
    m_stats.usage, m_stats.alloc, m_stats.peakUsage, m_stats.peakAlloc);
  FTRACE(1, "total alloc: {}\nje alloc: {}\nje dealloc: {}\n",
    m_stats.totalAlloc, m_prevAllocated, m_prevDeallocated);
  FTRACE(1, "je debt: {}\n\n", m_stats.jemallocDebt);
#else
  FTRACE(1, "resetStatsImpl({}) pre:\n"
    "usage: {}\nalloc: {}\npeak usage: {}\npeak alloc: {}\n\n",
    isInternalCall,
    m_stats.usage, m_stats.alloc, m_stats.peakUsage, m_stats.peakAlloc);
#endif
  if (isInternalCall) {
    m_statsIntervalActive = false;
    m_stats.usage = 0;
    m_stats.alloc = 0;
    m_stats.peakUsage = 0;
    m_stats.peakAlloc = 0;
    m_stats.totalAlloc = 0;
    m_stats.peakIntervalUsage = 0;
    m_stats.peakIntervalAlloc = 0;
#ifdef USE_JEMALLOC
    m_enableStatsSync = false;
  } else if (!m_enableStatsSync) {
#else
  } else {
#endif
    // This is only set by the jemalloc stats sync which we don't enable until
    // after this has been called.
    assert(m_stats.totalAlloc == 0);
#ifdef USE_JEMALLOC
    assert(m_stats.jemallocDebt >= m_stats.alloc);
#endif

    // The effect of this call is simply to ignore anything we've done *outside*
    // the smart allocator after we initialized to avoid attributing shared
    // structure initialization that happens during init_thread_locals() to this
    // session.

    // We don't want to clear the other values because we do already have some
    // sized smart allocator usage and live slabs and wiping now will result in
    // negative values when we try to reconcile our accounting with jemalloc.
#ifdef USE_JEMALLOC
    // Anything that was definitively allocated by the smart allocator should
    // be counted in this number even if we're otherwise zeroing out the count
    // for each thread.
    m_stats.totalAlloc = s_statsEnabled ? m_stats.jemallocDebt : 0;

    m_enableStatsSync = s_statsEnabled;
#else
    m_stats.totalAlloc = 0;
#endif
  }
#ifdef USE_JEMALLOC
  if (s_statsEnabled) {
    m_stats.jemallocDebt = 0;
    m_prevDeallocated = *m_deallocated;
    m_prevAllocated = *m_allocated;
  }
#endif
#ifdef USE_JEMALLOC
  FTRACE(1, "resetStatsImpl({}) post:\n", isInternalCall);
  FTRACE(1, "usage: {}\nalloc: {}\npeak usage: {}\npeak alloc: {}\n",
    m_stats.usage, m_stats.alloc, m_stats.peakUsage, m_stats.peakAlloc);
  FTRACE(1, "total alloc: {}\nje alloc: {}\nje dealloc: {}\n",
    m_stats.totalAlloc, m_prevAllocated, m_prevDeallocated);
  FTRACE(1, "je debt: {}\n\n", m_stats.jemallocDebt);
#else
  FTRACE(1, "resetStatsImpl({}) post:\n"
    "usage: {}\nalloc: {}\npeak usage: {}\npeak alloc: {}\n\n",
    isInternalCall,
    m_stats.usage, m_stats.alloc, m_stats.peakUsage, m_stats.peakAlloc);
#endif
}

NEVER_INLINE
void MemoryManager::refreshStatsHelper() {
  refreshStats();
}

void MemoryManager::refreshStatsHelperExceeded() {
#ifdef FACEBOOK
  if (RuntimeOption::LogNativeStackOnOOM) {
    using namespace folly::symbolizer;
    constexpr size_t kMaxFrames = 128;

    uintptr_t addresses[kMaxFrames];
    auto nframes = getStackTrace(addresses, kMaxFrames);
    std::vector<SymbolizedFrame> frames(nframes);
    Symbolizer symbolizer;
    symbolizer.symbolize(addresses, frames.data(), nframes);
    StringSymbolizePrinter printer;
    printer.println(addresses, frames.data(), nframes);
    Logger::Error("Exceeded memory limit\n\nC++ stack:\n%s",
                  printer.str().c_str());
  }
#endif
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  info->m_reqInjectionData.setMemExceededFlag();
  m_couldOOM = false;
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
  // Note however, that SmartAllocator adds to m_stats.jemallocDebt
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

    // Since these deltas potentially include memory allocated from another
    // thread but deallocated on this one, it is possible for these nubmers to
    // go negative.
    int64_t jeDeltaAllocated =
      int64_t(jeAllocated) - int64_t(jeDeallocated);
    int64_t mmDeltaAllocated =
      int64_t(m_prevAllocated) - int64_t(m_prevDeallocated);

    // This is the delta between the current and the previous jemalloc reading.
    int64_t jeMMDeltaAllocated =
      int64_t(jeAllocated) - int64_t(m_prevAllocated);

    FTRACE(1, "Before stats sync:\n");
    FTRACE(1, "je alloc:\ncurrent: {}\nprevious: {}\ndelta with MM: {}\n",
      jeAllocated, m_prevAllocated, jeAllocated - m_prevAllocated);
    FTRACE(1, "je dealloc:\ncurrent: {}\nprevious: {}\ndelta with MM: {}\n",
      jeDeallocated, m_prevDeallocated, jeDeallocated - m_prevDeallocated);
    FTRACE(1, "je delta:\ncurrent: {}\nprevious: {}\n",
      jeDeltaAllocated, mmDeltaAllocated);
    FTRACE(1, "usage: {}\ntotal (je) alloc: {}\nje debt: {}\n",
      stats.usage, stats.totalAlloc, stats.jemallocDebt);

    // Subtract the old jemalloc adjustment (delta0) and add the current one
    // (delta) to arrive at the new combined usage number.
    stats.usage += jeDeltaAllocated - mmDeltaAllocated;
    // Remove the "debt" accrued from allocating the slabs so we don't double
    // count the slab-based allocations.
    stats.usage -= stats.jemallocDebt;

    stats.jemallocDebt = 0;
    // We need to do the calculation instead of just setting it to jeAllocated
    // because of the MaskAlloc capability.
    stats.totalAlloc += jeMMDeltaAllocated;
    if (live) {
      m_prevAllocated = jeAllocated;
      m_prevDeallocated = jeDeallocated;
    }

    FTRACE(1, "After stats sync:\n");
    FTRACE(1, "usage: {}\ntotal (je) alloc: {}\n\n",
      stats.usage, stats.totalAlloc);
  }
#endif
  if (stats.usage > stats.peakUsage) {
    // NOTE: the peak memory usage monotonically increases, so there cannot
    // be a second OOM exception in one request.
    assert(stats.maxBytes > 0);
    if (live && m_couldOOM && stats.usage > stats.maxBytes) {
      refreshStatsHelperExceeded();
    }
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
    stats.peakUsage = stats.usage;
  }
  if (live && m_statsIntervalActive) {
    if (stats.usage > stats.peakIntervalUsage) {
      stats.peakIntervalUsage = stats.usage;
    }
    if (stats.alloc > stats.peakIntervalAlloc) {
      stats.peakIntervalAlloc = stats.alloc;
    }
  }
}

template void MemoryManager::refreshStatsImpl<true>(MemoryUsageStats& stats);
template void MemoryManager::refreshStatsImpl<false>(MemoryUsageStats& stats);

void MemoryManager::sweep() {
  assert(!sweeping());
  m_sweeping = true;
  SCOPE_EXIT { m_sweeping = false; };
  Sweepable::SweepAll();
  Native::sweepNativeData();
}

void MemoryManager::resetAllocator() {
  StringData::sweepAll();

  // free smart-malloc slabs
  for (auto slab : m_slabs) {
    free(slab);
  }
  m_slabs.clear();
  resetStatsImpl(true);

  // free large allocation blocks
  for (SweepNode *n = m_sweep.next, *next; n != &m_sweep; n = next) {
    next = n->next;
    free(n);
  }
  m_sweep.next = m_sweep.prev = &m_sweep;

  // zero out freelists
  for (auto& i : m_freelists) i.head = nullptr;
  m_front = m_limit = 0;
  m_instances.clear();

  resetCouldOOM();
}

void MemoryManager::iterate(iterate_callback callback, void* user_data) {
  // Iterate smart alloc slabs
  for (auto slab : m_slabs) {
    callback(slab, kSlabSize, false, user_data);
  }

  // Iterate large alloc slabs (Size N/A for now)
  for (SweepNode *n = m_sweep.next, *next; n != &m_sweep; n = next) {
    next = n->next;
    size_t size = 16;
#ifdef USE_JEMALLOC
    size = malloc_usable_size(n) - sizeof(SweepNode);
#endif
    callback(n + 1, size, true, user_data);
  }
}

/*
 * smart_malloc & friends implementation notes
 *
 * There are three kinds of smart mallocation:
 *
 *  a) Large allocations.  (size >= kMaxSmartSize)
 *
 *     In this case we behave as a wrapper around the normal libc
 *     malloc/free.  We insert a SweepNode header at the front of the
 *     allocation in order to find these at sweep time (end of
 *     request) so we can give them back to libc.
 *
 *  b) Size-tracked small allocations.
 *
 *     This is used for the generic case, for callers who can't tell
 *     us the size of the allocation at free time.
 *
 *     In this situation, we put a SmallNode header at the front of
 *     the block that tells us the size for when we need to free it
 *     later.  We differentiate this from a SweepNode (for a big
 *     allocation) by assuming that no SweepNode::prev will point to
 *     an address in the first kMaxSmartSize bytes of virtual address
 *     space.
 *
 *  c) Size-untracked small allocation
 *
 *     Many callers have an easy time telling you how big the object
 *     was when they need to free it.  In this case we can avoid the
 *     SmallNode, which saves us some memory and also let's us give
 *     out 16-byte aligned pointers easily.
 *
 *     We know when we have one of these because it has to be freed
 *     through a different entry point.  (E.g. MM().smartFreeSize or
 *     MM().smartFreeSizeBig.)
 *
 * When small blocks are freed (case b and c), they're placed the
 * appropriate size-segregated freelist.  Large blocks are immediately
 * passed back to libc via free.
 *
 * There are currently two kinds of freelist entries: entries where
 * there is already a valid SmallNode on the list (case b), and
 * entries where there isn't (case c).  The reason for this is that
 * that way, when allocating for case b, you don't need to store the
 * SmallNode size again.  Much of the heap is going through case b at
 * the time of this writing, so it is a measurable regression to try
 * to just combine the free lists, but presumably we can move more to
 * case c and combine the lists eventually.
 */

inline void* MemoryManager::smartMalloc(size_t nbytes) {
  auto const nbytes_padded = nbytes + sizeof(SmallNode);
  if (UNLIKELY(nbytes_padded > kMaxSmartSize)) {
    return smartMallocBig(nbytes);
  }

  auto const ptr = static_cast<SmallNode*>(smartMallocSize(nbytes_padded));
  ptr->padbytes = nbytes_padded;
  return ptr + 1;
}

inline void MemoryManager::smartFree(void* ptr) {
  assert(ptr != 0);
  auto const n = static_cast<SweepNode*>(ptr) - 1;
  auto const padbytes = n->padbytes;
  if (LIKELY(padbytes <= kMaxSmartSize)) {
    return smartFreeSize(static_cast<SmallNode*>(ptr) - 1, n->padbytes);
  }
  smartFreeBig(n);
}

inline void* MemoryManager::smartRealloc(void* inputPtr, size_t nbytes) {
  FTRACE(3, "smartRealloc: {} to {}\n", inputPtr, nbytes);
  assert(nbytes > 0);

  void* ptr = debug ? static_cast<DebugHeader*>((void*)(uintptr_t(inputPtr) -
                                                kDebugExtraSize)) : inputPtr;

  auto const n = static_cast<SweepNode*>(ptr) - 1;
  if (LIKELY(n->padbytes <= kMaxSmartSize)) {
    void* newmem = smart_malloc(nbytes);
    auto const copySize = std::min(
      n->padbytes - sizeof(SmallNode) - kDebugExtraSize,
      nbytes
    );
    newmem = memcpy(newmem, inputPtr, copySize);
    smart_free(inputPtr);
    return newmem;
  }

  // Ok, it's a big allocation.  Since we don't know how big it is
  // (i.e. how much data we should memcpy), we have no choice but to
  // ask malloc to realloc for us.
  auto const oldNext = n->next;
  auto const oldPrev = n->prev;

  auto const newNode = static_cast<SweepNode*>(
    safe_realloc(n, debugAddExtra(nbytes + sizeof(SweepNode)))
  );

  refreshStatsHelper();
  if (newNode != n) {
    oldNext->prev = oldPrev->next = newNode;
  }
  return debugPostAllocate(newNode + 1, 0, 0);
}

/*
 * Get a new slab, then allocate nbytes from it and install it in our
 * slab list.  Return the newly allocated nbytes-sized block.
 */
NEVER_INLINE void* MemoryManager::newSlab(size_t nbytes) {
  if (UNLIKELY(m_stats.usage > m_stats.maxBytes)) {
    refreshStatsHelper();
  }
  void* slab = safe_malloc(kSlabSize);
  assert((uintptr_t(slab) & kSmartSizeAlignMask) == 0);
  JEMALLOC_STATS_ADJUST(&m_stats, kSlabSize);
  m_stats.alloc += kSlabSize;
  if (m_stats.alloc > m_stats.peakAlloc) {
    m_stats.peakAlloc = m_stats.alloc;
  }
  m_slabs.push_back(slab);
  m_front = (void*)(uintptr_t(slab) + nbytes);
  m_limit = (void*)(uintptr_t(slab) + kSlabSize);
  FTRACE(3, "newSlab: adding slab at {} to limit {}\n", slab, m_limit);
  return slab;
}

// allocate nbytes from the current slab, aligned to kSmartSizeAlign
void* MemoryManager::slabAlloc(size_t nbytes, unsigned index) {
  assert(nbytes <= kSlabSize);
  assert((nbytes & kSmartSizeAlignMask) == 0);
  assert((uintptr_t(m_front) & kSmartSizeAlignMask) == 0);
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
  unsigned nPrealloc;
  if (nbytes * kSmartPreallocCountLimit <= kSmartPreallocBytesLimit) {
    nPrealloc = kSmartPreallocCountLimit;
  } else {
    nPrealloc = kSmartPreallocBytesLimit / nbytes;
  }
  {
    void* front = (void*)(uintptr_t(m_front) + nPrealloc*nbytes);
    if (uintptr_t(front) > uintptr_t(m_limit)) {
      nPrealloc = ((uintptr_t)m_limit - uintptr_t(m_front)) / nbytes;
      front = (void*)(uintptr_t(m_front) + nPrealloc*nbytes);
    }
    m_front = front;
  }
  for (void* p = (void*)(uintptr_t(m_front) - nbytes); p != ptr;
       p = (void*)(uintptr_t(p) - nbytes)) {
    m_freelists[index].push(
        debugPreFree(debugPostAllocate(p, debugRemoveExtra(nbytes),
                                       debugRemoveExtra(nbytes)),
                     debugRemoveExtra(nbytes), debugRemoveExtra(nbytes)));
  }
  return ptr;
}

inline void* MemoryManager::smartEnlist(SweepNode* n) {
  // If we are using jemalloc, it is keeping track of allocations outside of
  // the slabs and the usage so we should force this after an allocation that
  // was too large for one of the existing slabs. When we're not using jemalloc
  // this check won't do anything so avoid the extra overhead.
  if (use_jemalloc || UNLIKELY(m_stats.usage > m_stats.maxBytes)) {
    refreshStatsHelper();
  }
  // link after m_sweep
  SweepNode* next = m_sweep.next;
  n->next = next;
  n->prev = &m_sweep;
  next->prev = m_sweep.next = n;
  assert(n->padbytes > kMaxSmartSize);
  return n + 1;
}

NEVER_INLINE
void* MemoryManager::smartMallocBig(size_t nbytes) {
  assert(nbytes > 0);
  auto const n = static_cast<SweepNode*>(
    safe_malloc(nbytes + sizeof(SweepNode))
  );
  return smartEnlist(n);
}

#ifdef USE_JEMALLOC
template
NEVER_INLINE
void* MemoryManager::smartMallocSizeBigHelper<true>(
    void*&, size_t&, size_t);
template
NEVER_INLINE
void* MemoryManager::smartMallocSizeBigHelper<false>(
    void*&, size_t&, size_t);

template<bool callerSavesActualSize>
NEVER_INLINE
void* MemoryManager::smartMallocSizeBigHelper(void*& ptr,
                                              size_t& szOut,
                                              size_t bytes) {
  ptr = mallocx(debugAddExtra(bytes + sizeof(SweepNode)), 0);
  szOut = debugRemoveExtra(sallocx(ptr, 0) - sizeof(SweepNode));

  // NB: We don't report the SweepNode size in the stats.
  auto const delta = callerSavesActualSize ? szOut : bytes;
  m_stats.usage += int64_t(delta);
  // Adjust jemalloc otherwise we'll double count the direct allocation.
  JEMALLOC_STATS_ADJUST(&m_stats, delta);

  return debugPostAllocate(
    smartEnlist(static_cast<SweepNode*>(ptr)),
    bytes,
    szOut
  );
}
#endif

NEVER_INLINE
void* MemoryManager::smartCallocBig(size_t totalbytes) {
  assert(totalbytes > 0);
  auto const n = static_cast<SweepNode*>(
    safe_calloc(totalbytes + sizeof(SweepNode), 1)
  );
  return smartEnlist(n);
}

NEVER_INLINE
void MemoryManager::smartFreeBig(SweepNode* n) {
  SweepNode* next = n->next;
  SweepNode* prev = n->prev;
  next->prev = prev;
  prev->next = next;
  free(n);
}

// smart_malloc api entry points, with support for malloc/free corner cases.

void* smart_malloc(size_t nbytes) {
  auto& mm = MM();
  auto const size = mm.debugAddExtra(std::max(nbytes, size_t(1)));
  return mm.debugPostAllocate(mm.smartMalloc(size), 0, 0);
}

void* smart_calloc(size_t count, size_t nbytes) {
  auto& mm = MM();
  auto const totalBytes = std::max<size_t>(count * nbytes, 1);
  if (totalBytes <= kMaxSmartSize) {
    return memset(smart_malloc(totalBytes), 0, totalBytes);
  }
  auto const withExtra = mm.debugAddExtra(totalBytes);
  return mm.debugPostAllocate(
    mm.smartCallocBig(withExtra), 0, 0
  );
}

void* smart_realloc(void* ptr, size_t nbytes) {
  auto& mm = MM();
  if (!ptr) return smart_malloc(nbytes);
  if (!nbytes) {
    smart_free(ptr);
    return nullptr;
  }
  return mm.smartRealloc(ptr, nbytes);
}

void smart_free(void* ptr) {
  if (!ptr) return;
  auto& mm = MM();
  mm.smartFree(mm.debugPreFree(ptr, 0, 0));
}

//////////////////////////////////////////////////////////////////////

#ifdef DEBUG

void* MemoryManager::debugPostAllocate(void* p,
                                       size_t bytes,
                                       size_t returnedCap) {
  auto const header = static_cast<DebugHeader*>(p);
  header->allocatedMagic = DebugHeader::kAllocatedMagic;
  header->requestedSize = bytes;
  header->returnedCap = returnedCap;
  header->padding = 0;
  return (void*)(uintptr_t(header) + kDebugExtraSize);
}

void* MemoryManager::debugPreFree(void* p,
                                  size_t bytes,
                                  size_t userSpecifiedBytes) {
  auto const header = reinterpret_cast<DebugHeader*>(uintptr_t(p) -
                                                     kDebugExtraSize);
  assert(checkPreFree(header, bytes, userSpecifiedBytes));
  header->allocatedMagic = 0; // will get a freelist pointer shortly
  header->requestedSize = DebugHeader::kFreedMagic;
  memset(p, kSmartFreeFill, bytes);
  return header;
}

#endif

bool MemoryManager::checkPreFree(DebugHeader* p,
                                 size_t bytes,
                                 size_t userSpecifiedBytes) const {
  assert(debug);

  assert(p->allocatedMagic == DebugHeader::kAllocatedMagic);

  if (userSpecifiedBytes != 0) {
    // For size-specified frees, the size they report when freeing
    // must be either what they asked for, or what we returned as the
    // actual capacity;
    assert(userSpecifiedBytes == p->requestedSize ||
           userSpecifiedBytes == p->returnedCap);
  }
  if (bytes != 0 && bytes <= kMaxSmartSize) {
    auto const ptrInt = reinterpret_cast<uintptr_t>(p);
    DEBUG_ONLY auto it = std::find_if(
      std::begin(m_slabs), std::end(m_slabs),
      [&] (void* base) {
        auto const baseInt = reinterpret_cast<uintptr_t>(base);
        return ptrInt >= baseInt && ptrInt < baseInt + kSlabSize;
      }
    );
    assert(it != std::end(m_slabs));
  }

  return true;
}

void MemoryManager::logAllocation(void* p, size_t bytes) {
  MemoryProfile::logAllocation(p, bytes);
}

void MemoryManager::logDeallocation(void* p) {
  MemoryProfile::logDeallocation(p);
}

void MemoryManager::resetCouldOOM() {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  info->m_reqInjectionData.clearMemExceededFlag();
  m_couldOOM = true;
}

///////////////////////////////////////////////////////////////////////////////

NEVER_INLINE
void* MemoryManager::trackSlow(void* p) {
  m_instances.insert(p);
  return p;
}

NEVER_INLINE
void* MemoryManager::untrackSlow(void* p) {
  m_instances.erase(p);
  return p;
}

///////////////////////////////////////////////////////////////////////////////

}
