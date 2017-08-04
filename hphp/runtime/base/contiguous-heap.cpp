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
#include "hphp/runtime/base/memory-manager-defs.h"

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
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include <folly/Random.h>
#include <folly/ScopeGuard.h>
#include <folly/portability/SysMman.h>
#include <folly/portability/Unistd.h>


////////////////////////////////////////////////////////////////////////////////
//   The contiguous request heap.  Memory address space in the heap is        //
//   contiguous, so that HHVM has complete control of a request's memory      //
//   space.                                                                   //
//                                                                            //
//   The Purger struct, which is a thread_local variable, is used for memory  //
//   purge (use madvise(DONTNEED)) when we want to clean the memory space of  //
//   ContiguousBigHeap.                                                       //
////////////////////////////////////////////////////////////////////////////////
namespace HPHP {

TRACE_SET_MOD(mm);

///////////////////////////////////////////////////////////////////////////////
namespace {
void* alloc_heap(size_t size) {
  void* mem = mmap(nullptr, size, PROT_READ|PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) {
    std::string buffer = folly::sformat("Failed to mmap a region of size {}\n",
                                        size);
    log_native_stack(buffer.c_str());
  }
  always_assert(mem != MAP_FAILED);
  // TODO(T20460380): Investigate what kind of madvise is better in
  // contiguous heap, MADV_DONTNEED or MADV_FREE
  madvise(mem, size, MADV_DONTNEED); // decommit all
  return mem;
}

void free_heap(void* ptr, size_t size) {
  munmap(ptr, size);
}

/*
 * Purger implements decay-based purging of unused dirty pages using a similar
 * algorithm to jemalloc. we maintain a number of buckets, representing time
 * slices over the decay time window. Each bucket tracks the max heap size
 * observed in that time slice.
 *
 * Each time purge() is called, we apply the decay function to the heap size for
 * each slice, to arrive at the number of unused dirty pages we would like to
 * retain, then purge the rest by calling madvise(MADV_DONTNEED).
 *
 * There are a few tuning parameters in Purger:
 * kNumBuckets: 16,     the number of time slices over the decay time window
 * kThreshold:  RuntimeOption::ContiguousHeapThreshold (128MB by default),
 *                      the dirty pages threshold we could retain
 * kWindow:     RuntimeOption::ContiguousHeapWindowSize (5s by default),
 *                      the size of decay time window
 */

struct Purger {
  static const int64_t kNumBuckets = 16;
 /*
  *  Purge the memory from base to front, according to the decay curve
  */
  void purge(char* base, char* front) {
    assert(uintptr_t(base) % kPageSize == 0);
    assert(uintptr_t(front) % kPageSize == 0);
    size_t size = front - base;
    dirty_ = std::max(size, dirty_);
    // If dirty_ exceeds kThreshold, purge all the exceeded memory.
    if (dirty_ > kThreshold) {
      madvise(base + kThreshold, dirty_ - kThreshold, MADV_DONTNEED);
      dirty_ = kThreshold;
      return;
    }
    auto age = Timer::GetCurrentTimeMicros() - kTime0;
    // Update current bucket (reset to size if it expired)
    auto index = (age / kBucketSlice) % kNumBuckets;
    buckets_[index].max = age - buckets_[index].time > kBucketSlice ? size :
                     std::max(buckets_[index].max, size);
    buckets_[index].time = age;
    // Max of decayed max heap sizes, which means the upper bound that
    // how much memory can still live in the heap.
    size_t max = 0;
    for (auto& b : buckets_) {
      max = std::max(max, decay(b.max, age - b.time));
    }

    max = (max + kPageSize - 1) & ~(kPageSize - 1); // round up

    // If the dirty memory exceeds max, purge the exceeded part,
    // otherwise do nothing.
    if (dirty_ > max) {
      if (debug) {
        VLOG(1) << "purge " << id_ << ": t " << age/kBucketSlice
                << " size " << mb(size) << " max " << mb(max)
                << " dirty " << mb(dirty_) << " madvise " << mb(dirty_ - max)
                << std::endl;
      }
      madvise(base + max, dirty_ - max, MADV_DONTNEED);
      dirty_ = max;
    } else {
      if (debug) {
        VLOG(1) << "purge " << id_ << ": t " << age/kBucketSlice
                << " size " << mb(size)<< " max "<< mb(max)
                << " dirty " << mb(dirty_) << std::endl;
      }
    }
  }

 /*
  *  Purge all the dirty pages starting from base
  */
  void flush(char* base) {
    assert(uintptr_t(base) % kPageSize == 0);
    if (debug) {
      auto age = Timer::GetCurrentTimeMicros() - kTime0;
      VLOG(1) << "flush " << id_ << ": t " << age/kBucketSlice
              << " dirty " << mb(dirty_) << std::endl;
    }
    madvise(base, dirty_, MADV_DONTNEED);
    dirty_ = 0;
  }

private:
  struct Bucket {
    int64_t time; // most recent timestamp for this bucket
    size_t max;   // max heap size in this bucket time slice
  };


  /*
  *   Smoothstep of sigmoidal curves (https://en.wikipedia.org/wiki/Smoothstep)
  *   that grow from 0 to 1 in 0 <= x <= 1.
  *   smoothstep(x) = -2x^3  + 3x^2
  */
  static double smoothstep(double x) {
    return x * x * (3 - 2 * x);
  }

  /*
  *   Use decay() to determine how much memory we could have at the specific
  *   time slot. And use a lookup table to reduce float calculation.
  */
  size_t decay(size_t n, int64_t age) {
    return age <= 0 ? n :
           age >= kWindow ? 0 :
           n - n * smoothstep(double(age) / kWindow);
  }

  // Convert nbytes from Byte to MegaByte
  static size_t mb(size_t nbytes) {
    return nbytes >> 20;
  }

  static std::atomic<int> kNextid;
  static const int64_t kTime0;        // the process start time
  static const int64_t kPageSize;     // current page size

  // Time window size for purge size calculation. The amount of memory that
  // is allowed to be alive will be set to the maximum memory amount of the
  // latest time window according to the decay curve.
  int64_t const kWindow{
      RuntimeOption::EvalContiguousHeapWindowSize
  };
  // the dirty_ memory threshold
  size_t const kThreshold{RuntimeOption::EvalContiguousHeapThreshold};
  int64_t const kBucketSlice{kWindow / kNumBuckets};  // time window for bucket
  int const id_{kNextid++};                  // Purger id
  size_t dirty_{0};                          // the amount of dirty memory
  std::array<Bucket,kNumBuckets> buckets_;   // buckets array for time window
};

std::atomic<int> Purger::kNextid{0};
const int64_t Purger::kTime0{Timer::GetCurrentTimeMicros()};
const int64_t Purger::kPageSize{sysconf(_SC_PAGESIZE)};

thread_local Purger purger;
}

ContiguousBigHeap::ContiguousBigHeap()
  : m_base((char*)alloc_heap(ContiguousBigHeap::HeapCap))
  , m_limit(m_base + ContiguousBigHeap::HeapCap)
  , m_front(m_base){
}

ContiguousBigHeap::~ContiguousBigHeap() {
  free_heap(m_base, m_limit-m_base);
}

void ContiguousBigHeap::reset() {
  TRACE(1, "BigHeap-reset: %lu\n", m_front - m_base);
  // This fill0 now saves the memory manager from needing to fill0
  // when m_front grows.
  fill0(m_freebits, 0, chunk_index(m_front));
  purger.purge(m_base, m_front);
  m_front = m_base;
  assert(check_invariants());
}

void ContiguousBigHeap::flush() {
  assert(empty());
  purger.flush(m_base);
}

char* ContiguousBigHeap::grow(size_t size) {
  assert(size % ChunkSize == 0);
  assert(check_invariants());
  // Raise OOM error if it exceeds memory limit
  // TODO: We could add a configurable "soft" limit, which sets MemExceededFlag,
  // but proceeds to allocate successfully. PHP should then continue, and fatal
  // pretty soon. If it doesn't, we'll just die when we hit the hard limit.
  if (m_front + size > m_limit) {
    setSurpriseFlag(MemExceededFlag);
    if (RuntimeOption::LogNativeStackOnOOM) {
      std::string buffer = folly::sformat("Exceeded memory limit. m_front: {},"
                                          "size: {}, m_limit: {}",
                                          m_front, size, m_limit);
      log_native_stack(buffer.c_str());
    }
  }
  always_assert(m_front + size <= m_limit);
  auto p = m_front;
  m_front += size;
  // No need to fill0 because the m_freebits was initialized to 0
  // in range (m_front,m_limit)
  return p;
}

// Trying to find a contiguous space for n free chunks (n = size /ChunkSize),
// start searching from the first free Chunk up to m_front,
// if it fails to find enough space for all n chunks, then grow m_front.
char* ContiguousBigHeap::raw_alloc(size_t size) {
  assert(size >= ChunkSize && size % ChunkSize == 0);
  auto n = size/ChunkSize;
  // Scan forward looking for a range of n free chunks,
  // start from the first free chunk.  if not found, grow m_front.
  size_t endIndex = chunk_index(m_front);
  for (size_t startIndex = findFirst1(m_freebits, 0, endIndex),
      rangeEndIndex = startIndex + n;
      rangeEndIndex <= endIndex;
      rangeEndIndex = startIndex + n) {
    // Test if the last bit of range is free, if not, move startIndex to
    // the first free bit after range
    if (!m_freebits.test(rangeEndIndex - 1)) {
      startIndex = findFirst1(m_freebits, rangeEndIndex, endIndex);
    } else {
      auto lastUsedIndex = findLast0(m_freebits, startIndex, rangeEndIndex);
      // If the last used bit in this range is smaller than rangeEndIndex,
      // then this range is not valid for allocation. Move on to next range.
      if (lastUsedIndex < rangeEndIndex) {
        startIndex = lastUsedIndex + 1;
      } else {
        // Allocate [startIndex, rangeEndIndex)
        fill0(m_freebits, startIndex, rangeEndIndex);
        // Update the Hole header if the heap needs to split
        size_t holeEndIndex = findFirst0(m_freebits, rangeEndIndex, endIndex);
        if (holeEndIndex > rangeEndIndex) {
          FreeNode::InitFrom(m_base + rangeEndIndex * ChunkSize,
                            (holeEndIndex - rangeEndIndex) * ChunkSize,
                            HeaderKind::Hole);
        }
        return m_base + startIndex * ChunkSize;
      }
    }
  }
  return grow(size);
}

void ContiguousBigHeap::raw_free(char* p, size_t size) {
  auto startIndex = chunk_index(p), n = size/ChunkSize;
  assert(findFirst1(m_freebits, startIndex, startIndex + n) == startIndex + n);
  // Free [startIndex,startIndex+n), update the hole header
  fill1(m_freebits, startIndex, startIndex + n);
  FreeNode::InitFrom(m_base + startIndex * ChunkSize,
                    n * ChunkSize,
                    HeaderKind::Hole);
  auto endIndex = chunk_index(m_front);
  // The purger will try to purge contiguous free memory at the end of the
  // dirty page range, since the end part will be less likely to be reused.
  if (findFirst0(m_freebits, startIndex, endIndex) == endIndex) {
    purger.purge(p, m_front);
  }
}

MemBlock ContiguousBigHeap::allocSlab(size_t size, MemoryUsageStats& stats) {
  assert(size % ChunkSize == 0);
  auto p = raw_alloc(size);
  stats.heapAllocVolume += size;
  stats.capacity += size;
  stats.peakCap = std::max(stats.peakCap, stats.capacity);
  return {p, size};
}

//
// [MallocNode][usable memory]
// ^n          ^ptr          ^ptr+size == n+n->nbytes
//
// malloc:    [MallocNode/BigMalloc][data...]
// objMalloc: [MallocNode/BigObj   ][header...]
//            ^h1                   ^h2
//
//
// TODO(T20276438): this will work very badly for small blocks with
// m_bypassSlabAlloc because every block will be allocated through allocBig(),
// which means even really small blocks would be rounded up to at least one
// chunk, so we waste most of the memory space in chunk.
// And we should also figure out that how small can we make ChunkSize before
// performance starts to get really bad (in normal operation, and with
// m_bypassSlabAlloc=true)
MemBlock ContiguousBigHeap::allocBig(size_t bytes, HeaderKind kind,
                           type_scan::Index tyindex, MemoryUsageStats& stats) {
  // Round up to the nearest multiple of ChunkSize
  auto cap = (bytes + sizeof(MallocNode) + ChunkSize - 1) & ~(ChunkSize - 1);
  auto n = (MallocNode*)raw_alloc(cap);
  n->initHeader(tyindex, kind, 0);
  n->nbytes = cap;
  stats.mmUsage += cap;
  stats.capacity += cap;
  stats.heapAllocVolume += cap;
  return {n + 1, cap - sizeof(MallocNode)};
}

MemBlock ContiguousBigHeap::callocBig(size_t bytes, HeaderKind kind,
                            type_scan::Index tyindex, MemoryUsageStats& stats) {
  auto b = allocBig(bytes, kind, tyindex, stats);
  memset(b.ptr, 0, b.size);
  return b;
}

// Return true if the pointer is in the range and has not been freed
bool ContiguousBigHeap::contains(void* ptr) const {
  auto p = static_cast<char*>(ptr);
  auto index = chunk_index(p);
  return p >= m_base && p < m_front && !m_freebits.test(index);
}

void ContiguousBigHeap::freeBig(void* ptr) {
  auto n = static_cast<MallocNode*>(ptr) - 1;
  raw_free((char*)n, n->nbytes);
}

MemBlock ContiguousBigHeap::resizeBig(void* ptr, size_t newsize,
                                      MemoryUsageStats& stats) {
  auto newcap = (newsize + sizeof(MallocNode) + ChunkSize-1) & ~(ChunkSize-1);
  auto n = static_cast<MallocNode*>(ptr) - 1;
  if (newcap == n->nbytes) {
    // capacity and heapAllocVolume don't change
    return {n + 1, newcap - sizeof(MallocNode)};
  }
  auto old_size = n->nbytes;
  auto b = allocBig(newsize, n->kind(), n->typeIndex(), stats);
  memcpy(b.ptr, ptr, std::min(newcap, n->nbytes) - sizeof(MallocNode));
  raw_free((char*)n, n->nbytes);
  // Already add the stats in allocBig(), so just subtract the old stats
  stats.mmUsage -= old_size;
  stats.capacity -= old_size;
  stats.heapAllocVolume -= old_size;
  return b;
}

/*
 * To find `ptr', we first find the last Hole before p and start searching,
 * stop as soon as we pass p. If the search fails, return nullptr.
 */
HeapObject* ContiguousBigHeap::find(const void* ptr) {
  assert(check_invariants());
  auto p = (char*)ptr;
  auto pChunk = chunk_index(p);
  // If p is not within valid range or p is in a Hole
  if (p < m_base || p >= m_front || m_freebits.test(pChunk)) return nullptr;
  auto startIndex = findLast1(m_freebits, 0, pChunk);
  // Find the next chunk of the last free chunk as startIndex,
  // if didn't find, start from 0
  startIndex = startIndex == pChunk ? 0 : startIndex + 1;
  auto hdr = (HeapObject*) (m_base + startIndex*ChunkSize);
  while ((char*)hdr <= p) {
    auto nextHdr = (char*)hdr + allocSize(hdr);
    if (p < nextHdr) {   // find 'p'
        if (isBigKind(hdr->kind())) {
          auto innerHdr = (char*)hdr + sizeof(MallocNode);
          if (p >= innerHdr) return (HeapObject*) innerHdr;
        }
        return hdr;
    }
    hdr = (HeapObject*)nextHdr;   // go to next header
  }
  return nullptr;
}

// Make sure that there is no free Chunks after m_front
bool ContiguousBigHeap::check_invariants() const {
  assert(m_base <= m_front && m_front <= m_limit);
  for (size_t DEBUG_ONLY i = chunk_index(m_front),
              DEBUG_ONLY e = chunk_index(m_limit); i < e; ++i) {
    assert(!m_freebits.test(i));
  }
  return true;
}
}
