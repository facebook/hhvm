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

#include "hphp/runtime/base/memory-manager.h"

// Get SIZE_MAX definition.  Do this before including any other files, to make
// sure that this is the first place that stdint.h is included.
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#define __STDC_LIMIT_MACROS

#include <algorithm>
#include <cstdint>

#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/memory-profile.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/util/alloc.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"
#include "folly/ScopeGuard.h"

namespace HPHP {

TRACE_SET_MOD(smartalloc);

//////////////////////////////////////////////////////////////////////

const uint32_t SLAB_SIZE = 2 << 20;

//////////////////////////////////////////////////////////////////////

#ifdef USE_JEMALLOC
bool MemoryManager::s_statsEnabled = false;
size_t MemoryManager::s_cactiveLimitCeiling = 0;

static size_t threadAllocatedpMib[2];
static size_t threadDeallocatedpMib[2];
static size_t statsCactiveMib[2];
static pthread_once_t threadStatsOnce = PTHREAD_ONCE_INIT;
static void threadStatsInit() {
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

static inline void threadStats(uint64_t*& allocated, uint64_t*& deallocated,
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

  size_t headRoom = RuntimeOption::ServerMemoryHeadRoom;
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
  resetStats();
  m_stats.maxBytes = std::numeric_limits<int64_t>::max();
  // make the circular-lists empty.
  m_sweep.next = m_sweep.prev = &m_sweep;
  m_strings.next = m_strings.prev = &m_strings;
}

void MemoryManager::resetStats() {
  m_stats.usage = 0;
  m_stats.alloc = 0;
  m_stats.peakUsage = 0;
  m_stats.peakAlloc = 0;
  m_stats.totalAlloc = 0;
#ifdef USE_JEMALLOC
  if (s_statsEnabled) {
    m_stats.jemallocDebt = 0;
    m_prevAllocated = int64_t(*m_allocated);
    m_delta = m_prevAllocated - int64_t(*m_deallocated);
  }
#endif
}

NEVER_INLINE
void MemoryManager::refreshStatsHelper() {
  refreshStats();
}

void MemoryManager::refreshStatsHelperExceeded() {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  info->m_reqInjectionData.setMemExceededFlag();
}

#ifdef USE_JEMALLOC
void MemoryManager::refreshStatsHelperStop() {
  HttpServer::Server->stop();
  // Increase the limit to the maximum possible value, so that this method
  // won't be called again.
  m_cactiveLimit = std::numeric_limits<size_t>::max();
}
#endif

void MemoryManager::sweepAll() {
  m_sweeping = true;
  SCOPE_EXIT { m_sweeping = false; };
  Sweepable::SweepAll();
}

void MemoryManager::rollback() {
  StringData::sweepAll();

  // free smart-malloc slabs
  for (auto slab : m_slabs) {
    free(slab);
  }
  m_slabs.clear();

  // free large allocation blocks
  for (SweepNode *n = m_sweep.next, *next; n != &m_sweep; n = next) {
    next = n->next;
    free(n);
  }
  m_sweep.next = m_sweep.prev = &m_sweep;

  // zero out freelists
  for (auto& i : m_sizeUntrackedFree) i.clear();
  for (auto& i : m_sizeTrackedFree)   i.clear();
  m_front = m_limit = 0;
}

void MemoryManager::checkMemory() {
  printf("----- MemoryManager for Thread %ld -----\n", (long)pthread_self());

  refreshStats();
  printf("Current Usage: %" PRId64 " bytes\t", m_stats.usage);
  printf("Current Alloc: %" PRId64 " bytes\n", m_stats.alloc);
  printf("Peak Usage: %" PRId64 " bytes\t", m_stats.peakUsage);
  printf("Peak Alloc: %" PRId64 " bytes\n", m_stats.peakAlloc);

  printf("Slabs: %lu KiB\n", m_slabs.size() * SLAB_SIZE / 1024);
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
  nbytes += sizeof(SmallNode);
  if (UNLIKELY(nbytes > kMaxSmartSize)) {
    return smartMallocBig(nbytes);
  }

  nbytes = smartSizeClass(nbytes);
  m_stats.usage += nbytes;

  auto const idx = (nbytes - 1) >> kLgSizeQuantum;
  assert(idx < kNumSizes && idx >= 0);
  void* vp = m_sizeTrackedFree[idx].maybePop();
  if (UNLIKELY(vp == nullptr)) {
    return smartMallocSlab(nbytes);
  }
  FTRACE(1, "smartMalloc: {} -> {}\n", nbytes, vp);

  return vp;
}

inline void MemoryManager::smartFree(void* ptr) {
  assert(ptr != 0);
  auto const n = static_cast<SweepNode*>(ptr) - 1;
  auto const padbytes = n->padbytes;
  if (LIKELY(padbytes <= kMaxSmartSize)) {
    assert(memset(ptr, kSmartFreeFill, padbytes - sizeof(SmallNode)));
    auto const idx = (padbytes - 1) >> kLgSizeQuantum;
    assert(idx < kNumSizes && idx >= 0);
    FTRACE(1, "smartFree: {}\n", ptr);
    m_sizeTrackedFree[idx].push(ptr);
    m_stats.usage -= padbytes;
    return;
  }
  smartFreeBig(n);
}

// quick-and-dirty realloc implementation.  We could do better if the block
// is malloc'd, by deferring to the underlying realloc.
inline void* MemoryManager::smartRealloc(void* ptr, size_t nbytes) {
  FTRACE(1, "smartRealloc: {} to {}\n", ptr, nbytes);

  assert(ptr != 0 && nbytes > 0);
  SweepNode* n = ((SweepNode*)ptr) - 1;
  size_t old_padbytes = n->padbytes;
  if (LIKELY(old_padbytes <= kMaxSmartSize)) {
    void* newmem = smartMalloc(nbytes);
    memcpy(newmem, ptr, std::min(old_padbytes - sizeof(SmallNode), nbytes));
    smartFree(ptr);
    return newmem;
  }
  SweepNode* next = n->next;
  SweepNode* prev = n->prev;
  SweepNode* n2 = (SweepNode*) realloc(n, nbytes + sizeof(SweepNode));

  // ensure that we have not exceeded the per request memory limit (#2529805)
  refreshStatsHelper();
  if (n2 != n) {
    // block moved; must re-link to sweeplist
    next->prev = prev->next = n2;
  }
  return n2 + 1;
}

/*
 * Get a new slab, then allocate nbytes from it and install it in our
 * slab list.  Return the newly allocated nbytes-sized block.
 */
NEVER_INLINE char* MemoryManager::newSlab(size_t nbytes) {
  if (UNLIKELY(m_stats.usage > m_stats.maxBytes)) {
    refreshStatsHelper();
  }
  char* slab = (char*) Util::safe_malloc(SLAB_SIZE);
  assert(uintptr_t(slab) % 16 == 0);
  JEMALLOC_STATS_ADJUST(&m_stats, SLAB_SIZE);
  m_stats.alloc += SLAB_SIZE;
  if (m_stats.alloc > m_stats.peakAlloc) {
    m_stats.peakAlloc = m_stats.alloc;
  }
  m_slabs.push_back(slab);
  m_front = slab + nbytes;
  m_limit = slab + SLAB_SIZE;
  FTRACE(1, "newSlab: adding slab at {} to limit {}\n",
         static_cast<void*>(slab),
         static_cast<void*>(m_limit));
  return slab;
}

// allocate nbytes from the current slab, aligned to 16-bytes
void* MemoryManager::slabAlloc(size_t nbytes) {
  const size_t kAlignMask = 15;
  assert((nbytes & 7) == 0);
  char* ptr = (char*)(uintptr_t(m_front + kAlignMask) & ~kAlignMask);
  if (ptr + nbytes <= m_limit) {
    m_front = ptr + nbytes;
    return ptr;
  }
  return newSlab(nbytes);
}

NEVER_INLINE
void* MemoryManager::smartMallocSlab(size_t padbytes) {
  SmallNode* n = (SmallNode*) slabAlloc(padbytes);
  n->padbytes = padbytes;
  FTRACE(1, "smartMallocSlab: {} -> {}\n", padbytes,
         static_cast<void*>(n + 1));
  return n + 1;
}

inline void* MemoryManager::smartEnlist(SweepNode* n) {
  if (UNLIKELY(m_stats.usage > m_stats.maxBytes)) {
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
    Util::safe_malloc(nbytes + sizeof(SweepNode) - sizeof(SmallNode))
  );
  return smartEnlist(n);
}

#ifdef USE_JEMALLOC
NEVER_INLINE
void* MemoryManager::smartMallocSizeBigHelper(void*& ptr,
                                              size_t& szOut,
                                              size_t bytes) {
  m_stats.usage += bytes;
  allocm(&ptr, &szOut, debugAddExtra(bytes + sizeof(SweepNode)), 0);
  szOut = debugRemoveExtra(szOut - sizeof(SweepNode));
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
    Util::safe_calloc(totalbytes + sizeof(SweepNode), 1)
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

HOT_FUNC
void* smart_malloc(size_t nbytes) {
  return MM().smartMalloc(std::max(nbytes, size_t(1)));
}

HOT_FUNC
void* smart_calloc(size_t count, size_t nbytes) {
  auto const totalBytes = std::max<size_t>(count * nbytes, 1);
  if (totalBytes <= MemoryManager::kMaxSmartSize) {
    return memset(MM().smartMalloc(totalBytes), 0, totalBytes);
  }
  return MM().smartCallocBig(totalBytes);
}

HOT_FUNC
void* smart_realloc(void* ptr, size_t nbytes) {
  if (!ptr) return MM().smartMalloc(std::max(nbytes, size_t(1)));
  if (!nbytes) return ptr ? MM().smartFree(ptr), (void*)0 : (void*)0;
  return MM().smartRealloc(ptr, nbytes);
}

HOT_FUNC
void smart_free(void* ptr) {
  if (ptr) MM().smartFree(ptr);
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
  return header + 1;
}

void* MemoryManager::debugPreFree(void* p,
                                  size_t bytes,
                                  size_t userSpecifiedBytes) {
  auto const header = reinterpret_cast<DebugHeader*>(p) - 1;
  assert(checkPreFree(header, bytes, userSpecifiedBytes));
  header->allocatedMagic = 0; // will get a freelist pointer shortly
  header->requestedSize = DebugHeader::kFreedMagic;
  memset(header + 1, kSmartFreeFill, bytes);
  return header;
}

#endif

bool MemoryManager::checkPreFree(DebugHeader* p,
                                 size_t bytes,
                                 size_t userSpecifiedBytes) {
  assert(debug);

  assert(p->allocatedMagic == DebugHeader::kAllocatedMagic);

  if (userSpecifiedBytes != 0) {
    // For size-specified frees, the size they report when freeing
    // must be either what they asked for, or what we returned as the
    // actual capacity;
    assert(userSpecifiedBytes == p->requestedSize ||
           userSpecifiedBytes == p->returnedCap);
  }

  if (p->requestedSize <= MemoryManager::kMaxSmartSize) {
    auto const ptrInt = reinterpret_cast<uintptr_t>(p);
    DEBUG_ONLY auto it = std::find_if(
      begin(m_slabs), end(m_slabs),
      [&] (char* base) {
        auto const baseInt = reinterpret_cast<uintptr_t>(base);
        return ptrInt >= baseInt && ptrInt < baseInt + SLAB_SIZE;
      }
    );
    assert(it != end(m_slabs));
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

}
