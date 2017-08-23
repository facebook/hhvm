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
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/server/http-server.h"

#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/ptr-map.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include <folly/Random.h>
#include <folly/ScopeGuard.h>
#include <folly/portability/SysMman.h>
#include <folly/portability/Unistd.h>

#include "hphp/runtime/base/memory-manager-defs.h"

namespace HPHP {

const unsigned kInvalidSweepIndex = 0xffffffff;
__thread bool tl_sweeping;

TRACE_SET_MOD(mm);

//////////////////////////////////////////////////////////////////////

std::atomic<MemoryManager::ReqProfContext*>
  MemoryManager::s_trigger{nullptr};

bool MemoryManager::s_statsEnabled = false;

static std::atomic<size_t> s_heap_id; // global counter of heap instances
static thread_local size_t t_heap_id; // thread's current heap instance id

#ifdef USE_JEMALLOC
static size_t threadAllocatedpMib[2];
static size_t threadDeallocatedpMib[2];
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
  MemoryManager::s_statsEnabled = true;
}

inline
void MemoryManager::threadStats(uint64_t*& allocated, uint64_t*& deallocated) {
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
}
#endif

static void* MemoryManagerInit() {
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
  threadStats(m_allocated, m_deallocated);
#endif
  FTRACE(1, "heap-id {} new MM pid {}\n", t_heap_id, getpid());
  resetAllStats();
  setMemoryLimit(std::numeric_limits<int64_t>::max());
  resetGC(); // so each thread has unique req_num at startup
  // make the circular-lists empty.
  m_strings.next = m_strings.prev = &m_strings;
  m_bypassSlabAlloc = RuntimeOption::DisableSmallAllocator;
  m_req_start_micros = HPHP::Timer::GetThreadCPUTimeNanos() / 1000;
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL, "zend.enable_gc",
      &m_gc_enabled);
}

MemoryManager::~MemoryManager() {
  FTRACE(1, "heap-id {} ~MM\n", t_heap_id);
  // TODO(T20916887): Enable this for one-bit refcounting.
  if (debug && !one_bit_refcount) {
    // Check that every object in the heap is free.
    forEachHeapObject([&](HeapObject* h, size_t) {
        assert_flog(h->kind() == HeaderKind::Free,
                    "{} still live in ~MemoryManager()",
                    header_names[size_t(h->kind())]);
    });
  }
  // ~BigHeap releases its slabs/bigs.
}

void MemoryManager::resetRuntimeOptions() {
  if (debug) checkHeap("resetRuntimeOptions");

  MemoryManager::TlsWrapper::destroy(); // ~MemoryManager()
  MemoryManager::TlsWrapper::getCheck(); // new MemoryManager()
}

void MemoryManager::traceStats(const char* event) {
  FTRACE(1, "heap-id {} {} ", t_heap_id, event);
  if (use_jemalloc) {
    FTRACE(1, "mm-usage {} extUsage {} ",
           m_stats.mmUsage, m_stats.extUsage);
    FTRACE(1, "capacity {} peak usage {} peak capacity {} ",
           m_stats.capacity, m_stats.peakUsage, m_stats.peakCap);
    FTRACE(1, "total {} reset alloc-dealloc {} cur alloc-dealloc {}\n",
           m_stats.totalAlloc, m_resetAllocated - m_resetDeallocated,
           *m_allocated - *m_deallocated);
  } else {
    FTRACE(1, "usage: {} capacity: {} peak usage: {} peak capacity: {}\n",
           m_stats.usage(), m_stats.capacity,
           m_stats.peakUsage, m_stats.peakCap);
  }
}

// Reset all memory stats counters, both internal and external; intended to
// be used between requests when the whole heap is being reset.
void MemoryManager::resetAllStats() {
  traceStats("resetAllStats pre");
  m_statsIntervalActive = false;
  m_stats.mmUsage = 0;
  m_stats.extUsage = 0;
  m_stats.capacity = 0;
  m_stats.heapAllocVolume = 0;
  m_stats.peakUsage = 0;
  m_stats.peakCap = 0;
  m_stats.totalAlloc = 0;
  m_stats.peakIntervalUsage = 0;
  m_stats.peakIntervalCap = 0;
  m_enableStatsSync = false;
  if (Trace::enabled) t_heap_id = ++s_heap_id;
  if (s_statsEnabled) {
    m_resetDeallocated = *m_deallocated;
    m_resetAllocated = *m_allocated;
  }
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
  assert(m_enableStatsSync ||
         (m_stats.extUsage == 0 && m_stats.totalAlloc == 0));
  m_enableStatsSync = s_statsEnabled; // false if !use_jemalloc
  if (s_statsEnabled) {
    m_resetDeallocated = *m_deallocated;
#ifndef USE_CONTIGUOUS_HEAP
    m_resetAllocated = *m_allocated - m_stats.capacity;
    // By subtracting capcity here, the next call to refreshStatsImpl()
    // will correctly include m_stats.capacity in extUsage and totalAlloc.
#else
    // Contiguous heap does not use jemalloc,
    // so we don't need to subtract capacity to avoid double-counting.
    m_resetAllocated = *m_allocated;
#endif
  }
  traceStats("resetExternalStats post");
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

/*
 * Refresh stats to reflect directly malloc()ed memory, and determine
 * whether the request memory limit has been exceeded.
 *
 * The stats parameter allows the updates to be applied to either
 * m_stats as in refreshStats() or to a separate MemoryUsageStats
 * struct as in getStatsCopy().
 *
 * The template variable live controls whether or not MemoryManager
 * member variables are updated and whether or not to call helper
 * methods in response to memory anomalies.
 */
void MemoryManager::refreshStatsImpl(MemoryUsageStats& stats) {
  // Incrementally incorporate the difference between the previous and current
  // deltas into the memory usage statistic.  For reference, the total
  // malloced memory usage could be calculated as such, if delta0 were
  // recorded in resetAllStats():
  //
  //   int64 musage = delta - delta0;
  //
  // Note however, the slab allocator adds to m_stats.capacity
  // when it calls malloc(), so that this function can avoid
  // double-counting the malloced memory. Thus musage in the example
  // code may well substantially exceed m_stats.usage.
  if (m_enableStatsSync) {
    // We can't currently handle wrapping so make sure this isn't happening.
    assert(*m_allocated <= uint64_t(std::numeric_limits<int64_t>::max()));
    assert(*m_deallocated <= uint64_t(std::numeric_limits<int64_t>::max()));
    const int64_t curAllocated = *m_allocated;
    const int64_t curDeallocated = *m_deallocated;

    // Since these deltas potentially include memory allocated from another
    // thread but deallocated on this one, it is possible for these numbers to
    // go negative.
    auto curUsage = curAllocated - curDeallocated;
    auto resetUsage = m_resetAllocated - m_resetDeallocated;

    FTRACE(1, "heap-id {} Before stats sync: ", t_heap_id);
    FTRACE(1, "reset alloc-dealloc {} cur alloc-dealloc: {} alloc-change: {} ",
      resetUsage, curUsage, curAllocated - m_resetAllocated);
    FTRACE(1, "dealloc-change: {} ", curDeallocated - m_resetDeallocated);
    FTRACE(1, "mm usage {} extUsage {} totalAlloc {} capacity {}\n",
      stats.mmUsage, stats.extUsage, stats.totalAlloc, stats.capacity);

    // External usage (allocated-deallocated) since the last resetStats().
    stats.extUsage = curUsage - resetUsage;

    // Calculate the allocation volume since the last reset.
    // We need to do the calculation instead of just setting it to curAllocated
    // because of the MaskAlloc capability, which updates m_resetAllocated.

    // stats.heapAllocVolume is only used for contiguous heap, it would always
    // be 0 in default BigHeap.
    stats.totalAlloc = curAllocated - m_resetAllocated + stats.heapAllocVolume;
    FTRACE(1, "heap-id {} after sync extUsage {} totalAlloc: {}\n",
      t_heap_id, stats.extUsage, stats.totalAlloc);
  }
  assert(m_usageLimit > 0);
  auto usage = stats.usage();
  stats.peakUsage = std::max(stats.peakUsage, usage);
  if (m_statsIntervalActive) {
    stats.peakIntervalUsage = std::max(stats.peakIntervalUsage, usage);
    stats.peakIntervalCap = std::max(stats.peakIntervalCap, stats.capacity);
  }
}

/*
 * Refresh our internally stored m_stats, then check for OOM and the
 * memThresholdCallback
 */
void MemoryManager::refreshStats() {
  refreshStatsImpl(m_stats);
  auto usage = m_stats.usage();
  if (usage > m_usageLimit && m_couldOOM) {
    refreshStatsHelperExceeded();
  }
  if (usage > m_memThresholdCallbackPeakUsage) {
    m_memThresholdCallbackPeakUsage = SIZE_MAX;
    setSurpriseFlag(MemThresholdFlag);
  }
}

void MemoryManager::sweep() {
  assert(!sweeping());
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
  FTRACE(1, "heap-id {} sweep: sweepable {} native {} apc array {}\n",
         t_heap_id,
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
  assert(m_natives.empty() && m_sweepables.empty() && tl_sweeping);
  // decref apc strings referenced by this request
  DEBUG_ONLY auto nstrings = StringData::sweepAll();
  FTRACE(1, "heap-id {} resetAllocator: strings {}\n", t_heap_id, nstrings);

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

const std::array<char*,NumHeaderKinds> header_names = {{
  "PackedArray", "MixedArray", "EmptyArray", "ApcArray",
  "GlobalsArray", "ProxyArray", "DictArray", "VecArray", "KeysetArray",
  "String", "Resource", "Ref",
  "Object", "WaitHandle", "AsyncFuncWH", "AwaitAllWH", "Closure",
  "Vector", "Map", "Set", "Pair", "ImmVector", "ImmMap", "ImmSet",
  "AsyncFuncFrame", "NativeData", "ClosureHdr",
  "SmallMalloc", "BigMalloc", "BigObj",
  "Free", "Hole", "Slab"
}};

// initialize a Hole header in the unused memory between m_front and m_limit
void MemoryManager::initHole(void* ptr, uint32_t size) {
  FreeNode::InitFrom(ptr, size, HeaderKind::Hole);
}

void MemoryManager::initHole() {
  if ((char*)m_front < (char*)m_limit) {
    initHole(m_front, (char*)m_limit - (char*)m_front);
  }
}

void MemoryManager::initFree() {
  initHole();
  m_heap.sort();
  reinitFree();
}

void MemoryManager::reinitFree() {
  for (auto i = 0; i < kNumSmallSizes; i++) {
    auto size = sizeIndex2Size(i);
    auto n = m_freelists[i].head;
    for (; n && n->kind() != HeaderKind::Free; n = n->next) {
      n->initHeader_32(HeaderKind::Free, size);
    }
    if (debug) {
      // ensure the freelist tail is already initialized.
      for (; n; n = n->next) {
        assert(n->kind() == HeaderKind::Free && n->size() == size);
      }
    }
  }
}

MemoryManager::FreelistArray MemoryManager::beginQuarantine() {
  FreelistArray list;
  for (auto i = 0; i < kNumSmallSizes; ++i) {
    list[i].head = m_freelists[i].head;
    m_freelists[i].head = nullptr;
  }
  return list;
}

// turn free blocks into holes, restore original freelists
void MemoryManager::endQuarantine(FreelistArray&& list) {
  for (auto i = 0; i < kNumSmallSizes; i++) {
    auto size = sizeIndex2Size(i);
    while (auto n = m_freelists[i].maybePop()) {
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
  PtrMap<HeapObject*> free_blocks, apc_arrays, apc_strings;
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
      case HeaderKind::Apc:
        if (static_cast<APCLocalArray*>(h)->m_sweep_index !=
            kInvalidSweepIndex) {
          apc_arrays.insert(h, alloc_size);
        }
        break;
      case HeaderKind::String:
        if (static_cast<StringData*>(h)->isProxy()) {
          apc_strings.insert(h, alloc_size);
        }
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
      case HeaderKind::Slab:
        assert(false && "forEachHeapObject skips these kinds");
        break;
    }
  });

  // check the free lists
  free_blocks.prepare();
  size_t num_free_blocks = 0;
  for (auto i = 0; i < kNumSmallSizes; i++) {
    for (auto n = m_freelists[i].head; n; n = n->next) {
      assert(free_blocks.isStart(n));
      ++num_free_blocks;
    }
  }
  assert(num_free_blocks == free_blocks.size());

  // check the apc array list
  assert(apc_arrays.size() == m_apc_arrays.size());
  apc_arrays.prepare();
  for (UNUSED auto a : m_apc_arrays) {
    assert(apc_arrays.isStart(a));
  }

  // check the apc string list
  size_t num_apc_strings = 0;
  apc_strings.prepare();
  for (StringDataNode *next, *n = m_strings.next; n != &m_strings; n = next) {
    next = n->next;
    UNUSED auto const s = StringData::node2str(n);
    assert(s->isProxy());
    assert(apc_strings.isStart(s));
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
    uint32_t fragUsable = sizeIndex2Size(fragInd);
    auto frag = FreeNode::InitFrom((char*)rem + remBytes - fragUsable,
                                   fragUsable, HeaderKind::Hole);
    FTRACE(4, "MemoryManager::storeTail({}, {}): rem={}, remBytes={}, "
              "frag={}, fragBytes={}, fragUsable={}, fragInd={}\n", tail,
              (void*)uintptr_t(tailBytes), rem, (void*)uintptr_t(remBytes),
              frag, (void*)uintptr_t(fragBytes), (void*)uintptr_t(fragUsable),
              fragInd);
    m_freelists[fragInd].push(frag);
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
  assert(splitUsable == sizeIndex2Size(splitInd));
  for (uint32_t i = nSplit; i--;) {
    auto split = FreeNode::InitFrom((char*)tail + i * splitUsable,
                                    splitUsable, HeaderKind::Hole);
    FTRACE(4, "MemoryManager::splitTail(tail={}, tailBytes={}, tailPast={}): "
              "split={}, splitUsable={}, splitInd={}\n", tail,
              (void*)uintptr_t(tailBytes), (void*)(uintptr_t(tail) + tailBytes),
              split, splitUsable, splitInd);
    m_freelists[splitInd].push(split);
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
  refreshStats();
  requestGC();
  storeTail(m_front, (char*)m_limit - (char*)m_front);
  auto mem = m_heap.allocSlab(kSlabSize, m_stats);
  assert((uintptr_t(mem.ptr) & kSmallSizeAlignMask) == 0);
  auto slab = static_cast<Slab*>(mem.ptr);
  auto slab_start = slab->init();
  m_front = (void*)(slab_start + nbytes); // allocate requested object
  // we can't use any space after slab->end() even if the allocator allows
  // (indiciated by mem.size), because of the fixed-sized crossing map.
  m_limit = slab->end();
  FTRACE(3, "newSlab: adding slab at {} to limit {}\n", slab_start, m_limit);
  return slab_start;
}

/*
 * Allocate `bytes' from the current slab, aligned to kSmallSizeAlign.
 */
inline void* MemoryManager::slabAlloc(uint32_t nbytes, size_t index) {
  FTRACE(3, "slabAlloc({}, {}): m_front={}, m_limit={}\n", nbytes, index,
            m_front, m_limit);
  assert(nbytes == sizeIndex2Size(index));
  assert(nbytes <= kSlabSize);
  assert((uintptr_t(m_front) & kSmallSizeAlignMask) == 0);

  if (UNLIKELY(m_bypassSlabAlloc)) {
    // Stats correction; mallocBigSize() pulls stats from jemalloc.
    m_stats.mmUsage -= nbytes;
    return mallocBigSize<Unzeroed>(nbytes);
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
  FTRACE(4, "slabAlloc({}, {}) --> ptr={}, m_front={}, m_limit={}\n", nbytes,
            index, ptr, m_front, m_limit);
  return ptr;
}

void* MemoryManager::mallocSmallSizeSlow(size_t nbytes, size_t index) {
  assert(nbytes == sizeIndex2Size(index));
  unsigned nContig = kNContigTab[index];
  size_t contigMin = nContig * nbytes;
  unsigned contigInd = smallSize2Index(contigMin);
  for (unsigned i = contigInd; i < kNumSmallSizes; ++i) {
    FTRACE(4, "MemoryManager::mallocSmallSizeSlow({}, {}): contigMin={}, "
              "contigInd={}, try i={}\n", nbytes, index, contigMin,
              contigInd, i);
    void* p = m_freelists[i].maybePop();
    if (p != nullptr) {
      FTRACE(4, "MemoryManager::mallocSmallSizeSlow({}, {}): "
                "contigMin={}, contigInd={}, use i={}, size={}, p={}\n",
                nbytes, index, contigMin, contigInd, i, sizeIndex2Size(i),
                p);
      // Split tail into preallocations and store them back into freelists.
      size_t availBytes = sizeIndex2Size(i);
      size_t tailBytes = availBytes - nbytes;
      if (tailBytes > 0) {
        void* tail = (void*)(uintptr_t(p) + nbytes);
        splitTail(tail, tailBytes, nContig - 1, nbytes, index);
      }
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

template<MemoryManager::MBS Mode> NEVER_INLINE
void* MemoryManager::mallocBigSize(size_t bytes, HeaderKind kind,
                                   type_scan::Index ty) {
  if (debug) MM().requestEagerGC();
  auto block = Mode == Zeroed ? m_heap.callocBig(bytes, kind, ty, m_stats) :
               m_heap.allocBig(bytes, kind, ty, m_stats);
  updateBigStats();
  FTRACE(3, "mallocBigSize: {} ({} requested, {} usable)\n",
         block.ptr, bytes, block.size);
  return block.ptr;
}

template NEVER_INLINE
void* MemoryManager::mallocBigSize<MemoryManager::Unzeroed>(
    size_t, HeaderKind, type_scan::Index
);
template NEVER_INLINE
void* MemoryManager::mallocBigSize<MemoryManager::Zeroed>(
    size_t, HeaderKind, type_scan::Index
);

void* MemoryManager::resizeBig(MallocNode* n, size_t nbytes) {
  assert(n->kind() == HeaderKind::BigMalloc);
  auto block = m_heap.resizeBig(n + 1, nbytes, m_stats);
  updateBigStats();
  return block.ptr;
}

NEVER_INLINE
void MemoryManager::freeBigSize(void* vp) {
  // Since we account for these direct allocations in our usage and adjust for
  // them on allocation, we also need to adjust for them negatively on free.
  auto bytes = static_cast<MallocNode*>(vp)[-1].nbytes;
  m_stats.mmUsage -= bytes;
  m_stats.capacity -= bytes;
  FTRACE(3, "freeBigSize: {} ({} bytes)\n", vp, bytes);
  m_heap.freeBig(vp);
}

// req::malloc api entry points, with support for malloc/free corner cases.
namespace req {

using MBS = MemoryManager::MBS;

template<MBS Mode>
static void* allocate(size_t nbytes, type_scan::Index ty) {
  nbytes = std::max(nbytes, size_t(1));
  auto const npadded = nbytes + sizeof(MallocNode);
  if (LIKELY(npadded <= kMaxSmallSize)) {
    auto const ptr = static_cast<MallocNode*>(MM().mallocSmallSize(npadded));
    ptr->nbytes = npadded;
    ptr->initHeader_32_16(HeaderKind::SmallMalloc, 0, ty);
    return Mode == MBS::Zeroed ? memset(ptr + 1, 0, nbytes) : ptr + 1;
  }
  return MM().mallocBigSize<Mode>(nbytes, HeaderKind::BigMalloc, ty);
}

void* malloc(size_t nbytes, type_scan::Index tyindex) {
  assert(type_scan::isKnownType(tyindex));
  return allocate<MBS::Unzeroed>(nbytes, tyindex);
}

void* calloc(size_t count, size_t nbytes, type_scan::Index tyindex) {
  assert(type_scan::isKnownType(tyindex));
  return allocate<MBS::Zeroed>(count * nbytes, tyindex);
}

void* malloc_untyped(size_t nbytes) {
  return MM().mallocBigSize<MBS::Unzeroed>(
      std::max(nbytes, 1ul),
      HeaderKind::BigMalloc,
      type_scan::kIndexUnknown
  );
}

void* calloc_untyped(size_t count, size_t bytes) {
  return MM().mallocBigSize<MBS::Zeroed>(
      std::max(count * bytes, 1ul),
      HeaderKind::BigMalloc,
      type_scan::kIndexUnknown
  );
}

void* realloc(void* ptr, size_t nbytes, type_scan::Index tyindex) {
  assert(type_scan::isKnownType(tyindex));
  if (!ptr) {
    return allocate<MBS::Unzeroed>(nbytes, tyindex);
  }
  if (!nbytes) {
    req::free(ptr);
    return nullptr;
  }
  FTRACE(3, "MemoryManager::realloc: {} to {} [type_index: {}]\n",
         ptr, nbytes, tyindex);
  auto const n = static_cast<MallocNode*>(ptr) - 1;
  assert(n->typeIndex() == tyindex);
  if (LIKELY(n->kind() == HeaderKind::SmallMalloc) ||
      UNLIKELY(nbytes + sizeof(MallocNode) <= kMaxSmallSize)) {
    // either the old or new block will be small; force a copy.
    auto newmem = allocate<MBS::Unzeroed>(nbytes, tyindex);
    auto copy_size = std::min(n->nbytes - sizeof(MallocNode), nbytes);
    newmem = memcpy(newmem, ptr, copy_size);
    req::free(ptr);
    return newmem;
  }
  // it's a big allocation.
  return MM().resizeBig(n, nbytes);
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
  assert(n->kind() == HeaderKind::BigMalloc);
  assert(n->typeIndex() == type_scan::kIndexUnknown);
  return MM().resizeBig(n, nbytes);
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
    return MM().freeSmallSize(n, n->nbytes);
  }
  assert(n->kind() == HeaderKind::BigMalloc);
  MM().freeBigSize(ptr);
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
  MM().m_req_start_micros = HPHP::Timer::GetThreadCPUTimeNanos() / 1000;

  // If the trigger has already been claimed, do nothing.
  auto trigger = s_trigger.exchange(nullptr);
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

bool MemoryManager::isGCEnabled() {
  return m_gc_enabled;
}

void MemoryManager::setGCEnabled(bool isGCEnabled) {
  m_gc_enabled = isGCEnabled;
}

///////////////////////////////////////////////////////////////////////////////

void BigHeap::reset() {
  TRACE(1, "heap-id %lu BigHeap-reset: slabs %lu bigs %lu\n",
        t_heap_id, m_slabs.size(), m_bigs.size());
  auto const do_free = [&](void* ptr, size_t size) {
    if (RuntimeOption::EvalTrashFillOnRequestExit) {
      memset(ptr, size, kSmallFreeFill);
    }
#ifdef USE_JEMALLOC
    dallocx(ptr, 0);
#else
    free(ptr);
#endif
  };
  for (auto slab : m_slabs) do_free(slab.ptr, slab.size);
  m_slabs.clear();
  for (auto n : m_bigs) do_free(n, n->nbytes);
  m_bigs.clear();
}

void BigHeap::flush() {
  assert(empty());
  m_slabs = std::vector<MemBlock>{};
  m_bigs = std::vector<MallocNode*>{};
}

MemBlock BigHeap::allocSlab(size_t size, MemoryUsageStats& stats) {
#ifdef USE_JEMALLOC
  void* slab = mallocx(size, 0);
  auto usable = sallocx(slab, 0);
#else
  void* slab = safe_malloc(size);
  auto usable = size;
#endif
  m_slabs.push_back({slab, size});
  stats.capacity += usable;
  stats.peakCap = std::max(stats.peakCap, stats.capacity);
  return {slab, usable};
}

void BigHeap::enlist(MallocNode* n, HeaderKind kind,
                     size_t size, type_scan::Index tyindex) {
  n->initHeader_32_16(kind, m_bigs.size(), tyindex);
  n->nbytes = size;
  m_bigs.push_back(n);
}

MemBlock BigHeap::allocBig(size_t bytes, HeaderKind kind,
                           type_scan::Index tyindex, MemoryUsageStats& stats) {
#ifdef USE_JEMALLOC
  auto n = static_cast<MallocNode*>(mallocx(bytes + sizeof(MallocNode), 0));
  auto cap = sallocx(n, 0);
#else
  auto cap = bytes + sizeof(MallocNode);
  auto n = static_cast<MallocNode*>(safe_malloc(cap));
#endif
  enlist(n, kind, cap, tyindex);
  stats.mmUsage += cap;
  stats.capacity += cap;
  return {n + 1, cap - sizeof(MallocNode)};
}

MemBlock BigHeap::callocBig(size_t nbytes, HeaderKind kind,
                            type_scan::Index tyindex, MemoryUsageStats& stats) {
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
  stats.mmUsage += cap;
  stats.capacity += cap;
  return {n + 1, cap - sizeof(MallocNode)};
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

MemBlock BigHeap::resizeBig(void* ptr, size_t newsize,
                            MemoryUsageStats& stats) {
  // Since we don't know how big it is (i.e. how much data we should memcpy),
  // we have no choice but to ask malloc to realloc for us.
  auto const n = static_cast<MallocNode*>(ptr) - 1;
  auto old_size = n->nbytes;
#ifdef USE_JEMALLOC
  auto const newNode = static_cast<MallocNode*>(
    rallocx(n, newsize + sizeof(MallocNode), 0)
  );
  newNode->nbytes = sallocx(newNode, 0);
#else
  auto const newNode = static_cast<MallocNode*>(
    safe_realloc(n, newsize + sizeof(MallocNode))
  );
  newNode->nbytes = newsize + sizeof(MallocNode);
#endif
  if (newNode != n) {
    m_bigs[newNode->index()] = newNode;
  }
  stats.mmUsage += newsize - old_size;
  stats.capacity += newsize - old_size;
  return {newNode + 1, newNode->nbytes - sizeof(MallocNode)};
}

void BigHeap::sort() {
  std::sort(std::begin(m_slabs), std::end(m_slabs),
    [] (const MemBlock& l, const MemBlock& r) {
      assertx(static_cast<char*>(l.ptr) + l.size <= r.ptr ||
              static_cast<char*>(r.ptr) + r.size <= l.ptr);
      return l.ptr < r.ptr;
    }
  );
  std::sort(std::begin(m_bigs), std::end(m_bigs));
  for (size_t i = 0, n = m_bigs.size(); i < n; ++i) {
    m_bigs[i]->index() = i;
  }
}

/*
 * To find `p', we sort the slabs, bisect them, then iterate the slab
 * containing `p'.  If there is no such slab, we bisect the bigs to try to find
 * a big containing `p'.
 *
 * If that fails, we return nullptr.
 */
HeapObject* BigHeap::find(const void* p) {
  sort();
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
      auto const hdr = reinterpret_cast<HeapObject*>(h);
      auto const size = allocSize(hdr);
      if (p < h + size) return hdr;
      h += size;
    }
    // We know `p' is in the slab, so it must belong to one of the headers.
    always_assert(false);
  }

  auto const big = std::lower_bound(
    std::begin(m_bigs), std::end(m_bigs), p,
    [] (const MallocNode* big, const void* p) {
      return reinterpret_cast<const char*>(big) + big->nbytes <= p;
    }
  );

  if (big != std::end(m_bigs) && *big <= p) {
    auto const hdr = reinterpret_cast<HeapObject*>(*big);
    if (hdr->kind() != HeaderKind::BigObj) {
      // `p' is part of the MallocNode.
      return hdr;
    }
    auto const sub = reinterpret_cast<HeapObject*>(*big + 1);
    return p >= sub ? sub : hdr;
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
