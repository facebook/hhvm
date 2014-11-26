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

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/memory-profile.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stack-logger.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/server/http-server.h"

#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"

#include <folly/ScopeGuard.h>

#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/vm/globals-array.h"

namespace HPHP {

TRACE_SET_MOD(smartalloc);

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
  m_strings.next = m_strings.prev = &m_strings;
  m_bypassSlabAlloc = RuntimeOption::DisableSmartAllocator;
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

void MemoryManager::refreshStatsHelperExceeded() {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  info->m_reqInjectionData.setMemExceededFlag();
  m_couldOOM = false;
  if (RuntimeOption::LogNativeStackOnOOM) {
    log_native_stack("Exceeded memory limit");
  }
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
  if (debug) checkHeap();
  m_sweeping = true;
  SCOPE_EXIT { m_sweeping = false; };
  DEBUG_ONLY auto sweepable = Sweepable::SweepAll();
  DEBUG_ONLY auto native = m_natives.size();
  Native::sweepNativeData(m_natives);
  TRACE(1, "sweep: sweepable %u native %lu\n", sweepable, native);
}

void MemoryManager::resetAllocator() {
  // decref apc strings and arrays referenced by this request
  DEBUG_ONLY auto napcs = m_apc_arrays.size();
  while (!m_apc_arrays.empty()) {
    auto a = m_apc_arrays.back();
    m_apc_arrays.pop_back();
    a->sweep();
  }
  DEBUG_ONLY auto nstrings = StringData::sweepAll();

  // free the heap
  m_heap.reset();

  // zero out freelists
  for (auto& i : m_freelists) i.head = nullptr;
  m_front = m_limit = 0;
  if (m_trackingInstances) {
    m_instances = std::unordered_set<void*>();
  }

  resetStatsImpl(true);
  resetCouldOOM();
  TRACE(1, "reset: apc-arrays %lu strings %u\n", napcs, nstrings);
}

void MemoryManager::flush() {
  always_assert(empty());
  m_heap.flush();
  m_apc_arrays = std::vector<APCLocalArray*>();
  m_natives = std::vector<NativeNode*>();
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
  if (LIKELY(nbytes_padded) <= kMaxSmartSize) {
    auto const ptr = static_cast<SmallNode*>(smartMallocSize(nbytes_padded));
    ptr->padbytes = nbytes_padded;
    ptr->kind = HeaderKind::Small;
    return ptr + 1;
  }
  return smartMallocBig(nbytes);
}

union MallocNode {
  BigNode big;
  SmallNode small;
};
static_assert(sizeof(SmallNode) == sizeof(BigNode), "");

inline void MemoryManager::smartFree(void* ptr) {
  assert(ptr != 0);
  auto const n = static_cast<MallocNode*>(ptr) - 1;
  auto const padbytes = n->small.padbytes;
  if (LIKELY(padbytes <= kMaxSmartSize)) {
    return smartFreeSize(&n->small, n->small.padbytes);
  }
  m_heap.freeBig(ptr);
}

inline void* MemoryManager::smartRealloc(void* ptr, size_t nbytes) {
  FTRACE(3, "smartRealloc: {} to {}\n", ptr, nbytes);
  assert(nbytes > 0);
  auto const n = static_cast<MallocNode*>(ptr) - 1;
  if (LIKELY(n->small.padbytes <= kMaxSmartSize)) {
    void* newmem = smart_malloc(nbytes);
    auto const copySize = std::min(
      n->small.padbytes - sizeof(SmallNode),
      nbytes
    );
    newmem = memcpy(newmem, ptr, copySize);
    smart_free(ptr);
    return newmem;
  }
  // Ok, it's a big allocation.
  auto block = m_heap.resizeBig(ptr, nbytes);
  refreshStats();
  return block.ptr;
}

namespace {
const char* header_names[] = {
  "Packed", "Mixed", "StrMap", "IntMap", "VPacked", "Empty", "Shared",
  "Globals", "Proxy", "String", "Object", "Resource", "Ref", "Native",
  "Sweepable", "Small", "Big", "Free", "Hole", "Debug"
};
static_assert(sizeof(header_names)/sizeof(*header_names) == NumHeaderKinds, "");

// union of all the possible header types, and some utilities
struct Header {
  struct DummySweepable: Sweepable, ObjectData { void sweep() {} };
  size_t size() const;
  bool check() const;
  union {
    struct {
      uint64_t q;
      uint8_t b[3];
      HeaderKind kind_;
    };
    StringData str_;
    ArrayData arr_;
    MixedArray mixed_;
    ObjectData obj_;
    ResourceData res_;
    RefData ref_;
    SmallNode small_;
    BigNode big_;
    FreeNode free_;
    NativeNode native_;
    DebugHeader debug_;
    DummySweepable sweepable_;
  };
};

bool Header::check() const {
  return unsigned(kind_) <= NumHeaderKinds;
}

size_t Header::size() const {
  auto resourceSize = [](const ResourceData* r) {
    // explicitly virtual-call ResourceData::heapSize() through a pointer
    assert(r->heapSize());
    return r->heapSize();
  };
  assert(check());
  switch (kind_) {
    case HeaderKind::Packed: case HeaderKind::VPacked:
      return PackedArray::heapSize(&arr_);
    case HeaderKind::Mixed: case HeaderKind::StrMap: case HeaderKind::IntMap:
      return mixed_.heapSize();
    case HeaderKind::Empty:
      return sizeof(ArrayData);
    case HeaderKind::Shared:
      return sizeof(APCLocalArray);
    case HeaderKind::Globals:
      return sizeof(GlobalsArray);
    case HeaderKind::Proxy:
      return sizeof(ProxyArray);
    case HeaderKind::String:
      return str_.heapSize();
    case HeaderKind::Object:
      // [ObjectData][subclass][props]
      return obj_.heapSize();
    case HeaderKind::Resource:
      // [ResourceData][subclass]
      return resourceSize(&res_);
    case HeaderKind::Ref:
      return sizeof(RefData);
    case HeaderKind::Small:
      return small_.padbytes;
    case HeaderKind::Big:
      return big_.nbytes;
    case HeaderKind::Free:
      return free_.size;
    case HeaderKind::Native:
      // [NativeNode][NativeData][ObjectData][props] is one allocation.
      return native_.obj_offset + Native::obj(&native_)->heapSize();
    case HeaderKind::Sweepable:
      // [Sweepable][ObjectData][subclass][props] or
      // [Sweepable][ResourceData][subclass]
      return sizeof(Sweepable) + sweepable_.heapSize();
    case HeaderKind::Hole:
      return free_.size;
    case HeaderKind::Debug:
      assert(debug_.allocatedMagic == DebugHeader::kAllocatedMagic);
      return sizeof(DebugHeader);
  }
  return 0;
}

// Iterator for slab scanning; only knows how to parse each object's
// size and move to the next object.
struct Headiter {
  explicit Headiter(void* p) : raw_((char*)p) {}
  Headiter& operator=(void* p) {
    raw_ = (char*)p;
    return *this;
  }
  Headiter& operator++() {
    assert(h_->size() > 0);
    raw_ += MemoryManager::smartSizeClass(h_->size());
    return *this;
  }
  Header& operator*() { return *h_; }
  Header* operator->() { return h_; }
  bool operator<(Headiter i) const { return raw_ < i.raw_; }
  bool operator==(Headiter i) const { return raw_ == i.raw_; }
private:
  union {
    Header* h_;
    char* raw_;
  };
};
}

// Iterator over all the slabs and bigs
struct BigHeap::iterator {
  enum State { Slabs, Bigs };
  using slab_iter = std::vector<MemBlock>::iterator;
  using big_iter  = std::vector<BigNode*>::iterator;
  explicit iterator(slab_iter slab, BigHeap& heap)
    : m_state{Slabs}
    , m_slab{slab}
    , m_header{slab->ptr} // should never be at end.
    , m_heap(heap)
  {}
  explicit iterator(big_iter big, BigHeap& heap)
    : m_state{Bigs}
    , m_big{big}
    , m_header{big != heap.m_bigs.end() ? *big : nullptr}
    , m_heap(heap)
  {}
  bool operator==(const iterator& it) const {
    if (m_state != it.m_state) return false;
    switch (m_state) {
      case Slabs: return m_slab == it.m_slab && m_header == it.m_header;
      case Bigs: return m_big == it.m_big;
    };
    not_reached();
  }
  bool operator!=(const iterator& it) const {
    return !(*this == it);
  }
  iterator& operator++() {
    switch (m_state) {
      case Slabs: {
        auto end = Headiter{static_cast<char*>(m_slab->ptr) + m_slab->size};
        if (++m_header < end) {
          return *this;
        }
        if (++m_slab != m_heap.m_slabs.end()) {
          m_header = m_slab->ptr;
          return *this;
        }
        m_state = Bigs;
        m_big = m_heap.m_bigs.begin();
        m_header = m_big != m_heap.m_bigs.end() ? *m_big : nullptr;
        return *this;
      }
      case Bigs:
        ++m_big;
        m_header = m_big != m_heap.m_bigs.end() ? *m_big : nullptr;
        return *this;
    }
    not_reached();
  }
  Header& operator*() { return *m_header; }
  Header* operator->() { return &*m_header; }
 private:
  State m_state;
  union {
    slab_iter m_slab;
    big_iter m_big;
  };
  Headiter m_header;
  BigHeap& m_heap;
};

// initialize a Hole header in the unused memory between m_front and m_limit
void MemoryManager::initHole() {
  if ((char*)m_front < (char*)m_limit) {
    auto hdr = static_cast<FreeNode*>(m_front);
    hdr->kind = HeaderKind::Hole;
    hdr->size = (char*)m_limit - (char*)m_front;
  }
}

BigHeap::iterator MemoryManager::begin() {
  initHole();
  return m_heap.begin();
}

BigHeap::iterator MemoryManager::end() {
  return m_heap.end();
}

// test iterating objects in slabs
void MemoryManager::checkHeap() {
  size_t bytes=0;
  std::vector<Header*> hdrs;
  std::unordered_set<FreeNode*> free_blocks;
  size_t counts[NumHeaderKinds];
  for (unsigned i=0; i < NumHeaderKinds; i++) counts[i] = 0;
  for (auto h = begin(), lim = end(); h != lim; ++h) {
    hdrs.push_back(&*h);
    TRACE(2, "checkHeap: hdr %p\n", hdrs[hdrs.size()-1]);
    bytes += h->size();
    counts[(int)h->kind_]++;
    if (h->kind_ == HeaderKind::Debug) {
      // the next block's parsed size should agree with DebugHeader
      auto h2 = h; ++h2;
      if (h2 != lim) {
        assert(h2->kind_ != HeaderKind::Debug);
        assert(h->debug_.returnedCap ==
               MemoryManager::smartSizeClass(h2->size()));
      }
    } else if (h->kind_ == HeaderKind::Free) {
      free_blocks.insert(&h->free_);
    }
  }
  // make sure everything in a free list was scanned exactly once.
  for (size_t i = 0; i < kNumSmartSizes; i++) {
    for (auto n = m_freelists[i].head; n; n = n->next) {
      assert(free_blocks.find(n) != free_blocks.end());
      free_blocks.erase(n);
    }
  }
  assert(free_blocks.empty());
  TRACE(1, "checkHeap: %lu objects %lu bytes\n", hdrs.size(), bytes);
  TRACE(1, "checkHeap-types: ");
  for (unsigned i = 0; i < NumHeaderKinds; ++i) {
    TRACE(1, "%s %lu%s", header_names[i], counts[i],
          (i + 1 < NumHeaderKinds ? " " : "\n"));
  }
}

/*
 * Get a new slab, then allocate nbytes from it and install it in our
 * slab list.  Return the newly allocated nbytes-sized block.
 */
NEVER_INLINE void* MemoryManager::newSlab(size_t nbytes) {
  if (UNLIKELY(m_stats.usage > m_stats.maxBytes)) {
    refreshStats();
  }
  initHole(); // enable parsing the leftover space in the old slab
  if (debug) checkHeap();
  auto slab = m_heap.allocSlab(kSlabSize);
  assert((uintptr_t(slab.ptr) & kSmartSizeAlignMask) == 0);
  m_stats.borrow(slab.size);
  m_stats.alloc += slab.size;
  if (m_stats.alloc > m_stats.peakAlloc) {
    m_stats.peakAlloc = m_stats.alloc;
  }
  m_front = (void*)(uintptr_t(slab.ptr) + nbytes);
  m_limit = (void*)(uintptr_t(slab.ptr) + slab.size);
  FTRACE(3, "newSlab: adding slab at {} to limit {}\n", slab.ptr, m_limit);
  return slab.ptr;
}

/*
 * Allocate `bytes' from the current slab, aligned to kSmartSizeAlign.
 */
void* MemoryManager::slabAlloc(uint32_t bytes, unsigned index) {
  size_t nbytes = debugAddExtra(smartSizeClass(bytes));

  assert(nbytes <= kSlabSize);
  assert((nbytes & kSmartSizeAlignMask) == 0);
  assert((uintptr_t(m_front) & kSmartSizeAlignMask) == 0);

  if (UNLIKELY(m_bypassSlabAlloc)) {
    // Stats correction; smartMallocSizeBig() pulls stats from jemalloc.
    m_stats.usage -= bytes;
    // smartMallocSizeBig already wraps its allocation in a debug header, but
    // the caller will try to do it again, so we need to adjust this pointer
    // before returning it.
    return ((char*)smartMallocSizeBig<false>(nbytes).ptr) - kDebugExtraSize;
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
    auto usable = debugRemoveExtra(nbytes);
    auto ptr = debugPostAllocate(p, usable, usable);
    debugPreFree(ptr, usable, usable);
    m_freelists[index].push(ptr, usable);
  }
  return ptr;
}

inline void MemoryManager::updateBigStats() {
  // If we are using jemalloc, it is keeping track of allocations outside of
  // the slabs and the usage so we should force this after an allocation that
  // was too large for one of the existing slabs. When we're not using jemalloc
  // this check won't do anything so avoid the extra overhead.
  if (use_jemalloc || UNLIKELY(m_stats.usage > m_stats.maxBytes)) {
    refreshStats();
  }
}

NEVER_INLINE
void* MemoryManager::smartMallocBig(size_t nbytes) {
  assert(nbytes > 0);
  auto block = m_heap.allocBig(nbytes);
  updateBigStats();
  return block.ptr;
}

template NEVER_INLINE
MemBlock MemoryManager::smartMallocSizeBig<true>(size_t);
template NEVER_INLINE
MemBlock MemoryManager::smartMallocSizeBig<false>(size_t);

template<bool callerSavesActualSize> NEVER_INLINE
MemBlock MemoryManager::smartMallocSizeBig(size_t bytes) {
  auto block = m_heap.allocBig(debugAddExtra(bytes));
  auto szOut = debugRemoveExtra(block.size);
#ifdef USE_JEMALLOC
  // NB: We don't report the SweepNode size in the stats.
  auto const delta = callerSavesActualSize ? szOut : bytes;
  m_stats.usage += int64_t(delta);
  // Adjust jemalloc otherwise we'll double count the direct allocation.
  m_stats.borrow(delta);
#else
  m_stats.usage += bytes;
#endif
  updateBigStats();
  auto ptrOut = debugPostAllocate(block.ptr, bytes, szOut);
  FTRACE(3, "smartMallocSizeBig: {} ({} requested, {} usable)\n",
         ptrOut, bytes, szOut);
  return {ptrOut, szOut};
}

NEVER_INLINE
void* MemoryManager::smartCallocBig(size_t totalbytes) {
  assert(totalbytes > 0);
  auto block = m_heap.callocBig(totalbytes);
  updateBigStats();
  return block.ptr;
}

// smart_malloc api entry points, with support for malloc/free corner cases.

void* smart_malloc(size_t nbytes) {
  auto& mm = MM();
  auto const size = std::max(nbytes, size_t(1));
  return mm.smartMalloc(size);
}

void* smart_calloc(size_t count, size_t nbytes) {
  auto& mm = MM();
  auto const totalBytes = std::max<size_t>(count * nbytes, 1);
  if (totalBytes <= kMaxSmartSize) {
    return memset(smart_malloc(totalBytes), 0, totalBytes);
  }
  return mm.smartCallocBig(totalBytes);
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
  if (ptr) MM().smartFree(ptr);
}

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

//////////////////////////////////////////////////////////////////////

#ifdef DEBUG

void* MemoryManager::debugPostAllocate(void* p,
                                       size_t bytes,
                                       size_t returnedCap) {
  auto const header = static_cast<DebugHeader*>(p);
  header->allocatedMagic = DebugHeader::kAllocatedMagic;
  header->kind = HeaderKind::Debug;
  header->requestedSize = bytes;
  header->returnedCap = returnedCap;
  return (void*)(uintptr_t(header) + kDebugExtraSize);
}

void* MemoryManager::debugPreFree(void* p,
                                  size_t bytes,
                                  size_t userSpecifiedBytes) {
  auto const header = reinterpret_cast<DebugHeader*>(uintptr_t(p) -
                                                     kDebugExtraSize);
  assert(checkPreFree(header, bytes, userSpecifiedBytes));
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
    // must be between the requested size and the actual capacity.
    assert(userSpecifiedBytes >= p->requestedSize &&
           userSpecifiedBytes <= p->returnedCap);
  }
  if (!m_bypassSlabAlloc && bytes != 0 && bytes <= kMaxSmartSize) {
    assert(m_heap.contains(p));
  }

  return true;
}

void MemoryManager::logAllocation(void* p, size_t bytes) {
  MemoryProfile::logAllocation(p, bytes);
}

void MemoryManager::logDeallocation(void* p) {
  MemoryProfile::logDeallocation(p);
}

void MemoryManager::resetCouldOOM(bool state) {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  info->m_reqInjectionData.clearMemExceededFlag();
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
  bool active = true;
  size_t boolsz = sizeof(bool);

  // Reset jemalloc stats.
  if (mallctl("prof.reset", nullptr, nullptr, nullptr, 0)) {
    return;
  }

  // Enable jemalloc thread-local heap dumps.
  if (mallctl("prof.active",
              &profctx.prof_active, &boolsz,
              &active, sizeof(bool))) {
    profctx = ReqProfContext{};
    return;
  }
  if (mallctl("thread.prof.active",
              &profctx.thread_prof_active, &boolsz,
              &active, sizeof(bool))) {
    mallctl("prof.active", nullptr, nullptr,
            &profctx.prof_active, sizeof(bool));
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

  mallctl("thread.prof.active", nullptr, nullptr,
          &profctx.thread_prof_active, sizeof(bool));
  mallctl("prof.active", nullptr, nullptr,
          &profctx.prof_active, sizeof(bool));
#endif

  MM().m_bypassSlabAlloc = RuntimeOption::DisableSmartAllocator;
  profctx = ReqProfContext{};
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

void BigHeap::reset() {
  TRACE(1, "BigHeap-reset: slabs %lu bigs %lu\n", m_slabs.size(),
        m_bigs.size());
  for (auto slab : m_slabs) {
    free(slab.ptr);
  }
  m_slabs.clear();
  for (auto n : m_bigs) {
    free(n);
  }
  m_bigs.clear();
}

void BigHeap::flush() {
  assert(empty());
  m_slabs = std::vector<MemBlock>{};
  m_bigs = std::vector<BigNode*>{};
}

MemBlock BigHeap::allocSlab(size_t size) {
  void* slab = safe_malloc(size);
  m_slabs.push_back({slab, size});
  return {slab, size};
}

MemBlock BigHeap::allocBig(size_t bytes) {
#ifdef USE_JEMALLOC
  auto n = static_cast<BigNode*>(mallocx(bytes + sizeof(BigNode), 0));
  auto cap = sallocx(n, 0);
#else
  auto cap = bytes + sizeof(BigNode);
  auto n = static_cast<BigNode*>(safe_malloc(cap));
#endif
  enlist(n, cap);
  return {n + 1, cap - sizeof(BigNode)};
}

MemBlock BigHeap::callocBig(size_t nbytes) {
  auto cap = nbytes + sizeof(BigNode);
  auto const n = static_cast<BigNode*>(safe_calloc(cap, 1));
  enlist(n, cap);
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

void BigHeap::enlist(BigNode* n, size_t size) {
  n->nbytes = size;
  n->kind = HeaderKind::Big;
  n->index = m_bigs.size();
  m_bigs.push_back(n);
}

NEVER_INLINE
void BigHeap::freeBig(void* ptr) {
  auto n = static_cast<BigNode*>(ptr) - 1;
  auto i = n->index;
  auto last = m_bigs.back();
  last->index = i;
  m_bigs[i] = last;
  m_bigs.pop_back();
  free(n);
}

MemBlock BigHeap::resizeBig(void* ptr, size_t newsize) {
  // Since we don't know how big it is (i.e. how much data we should memcpy),
  // we have no choice but to ask malloc to realloc for us.
  auto const n = static_cast<BigNode*>(ptr) - 1;
  auto const newNode = static_cast<BigNode*>(
    safe_realloc(n, newsize + sizeof(BigNode))
  );
  if (newNode != n) {
    m_bigs[newNode->index] = newNode;
  }
  return {newNode + 1, newsize};
}

BigHeap::iterator BigHeap::begin() {
  if (!m_slabs.empty()) return iterator{m_slabs.begin(), *this};
  return iterator{m_bigs.begin(), *this};
}

BigHeap::iterator BigHeap::end() {
  return iterator{m_bigs.end(), *this};
}

}
