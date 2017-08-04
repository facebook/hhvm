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
#include "hphp/runtime/base/apc-gc-manager.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/heap-scan.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/alloc.h"
#include "hphp/util/bloom-filter.h"
#include "hphp/util/cycles.h"
#include "hphp/util/process.h"
#include "hphp/util/ptr-map.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"
#include "hphp/util/type-scan.h"

#include <algorithm>
#include <iterator>
#include <vector>
#include <folly/Range.h>
#include <folly/portability/Unistd.h>

namespace HPHP {
TRACE_SET_MOD(gc);

namespace {

struct Counter {
  size_t count{0};
  size_t bytes{0};
  void operator+=(size_t n) {
    bytes += n;
    count++;
  }
};

bool hasNativeData(const HeapObject* h) {
  assert(isObjectKind(h->kind()));
  return static_cast<const ObjectData*>(h)->getAttribute(
      ObjectData::HasNativeData
  );
}

template<bool apcgc> struct Marker {
  explicit Marker(HeapImpl& heap) : heap_(heap) {}
  void init();
  void traceRoots();
  void trace();
  void sweep();

  // drain the scanner, enqueue pointers
  void finish_scan();

  // mark ambiguous pointers in the range [start,start+len)
  void conservativeScan(const void* start, size_t len);

  static bool marked(const HeapObject* h) { return h->marks() & GCBits::Mark; }
  bool mark(const HeapObject*, GCBits = GCBits::Mark);
  void checkedEnqueue(const void* p, GCBits bits);
  void finish_typescan();
  HdrBlock find(const void*);

  void enqueue(const HeapObject* h) {
    assert(h && h->kind() <= HeaderKind::BigMalloc &&
           h->kind() != HeaderKind::AsyncFuncWH &&
           h->kind() != HeaderKind::Closure);
    assert(!isObjectKind(h->kind()) || !hasNativeData(h));
    work_.push_back(h);
    max_worklist_ = std::max(max_worklist_, work_.size());
  }

  HeapImpl& heap_;
  Counter allocd_, marked_, pinned_, unknown_; // bytes
  Counter cscanned_roots_, cscanned_; // bytes
  Counter xscanned_roots_, xscanned_; // bytes
  size_t init_ns_, initfree_ns_, roots_ns_, mark_ns_, sweep_ns_;
  size_t max_worklist_{0}; // max size of work_
  size_t freed_bytes_{0};
  PtrMap<const HeapObject*> ptrs_;
  type_scan::Scanner type_scanner_;
  std::vector<const HeapObject*> work_;
  APCGCManager* apcgc_ ;
};

// TODO(T20460162): The contiguous heap has a bitmap of which chunks of memory
// are allocated/free. And it can efficiently find the start of an allocated
// object using bit operations. So there is an opportunity to speed this up by
// directly accessing the bitmap instead of using PtrMap.
template <bool apcgc>
HdrBlock Marker<apcgc>::find(const void* ptr) {
  if (auto r = ptrs_.region(ptr)) {
    auto h = const_cast<HeapObject*>(r->first);
    if (h->kind() == HeaderKind::Slab) {
      return Slab::fromHeader(h)->find(ptr);
    }
    if (h->kind() == HeaderKind::BigObj) {
      auto h2 = static_cast<MallocNode*>(h) + 1;
      return ptr >= h2 ? HdrBlock{h2, r->second - sizeof(MallocNode)} :
             HdrBlock{nullptr, 0};
    }
    assert(h->kind() == HeaderKind::BigMalloc);
    return {h, r->second};
  }
  return {nullptr, 0};
}

// Mark the object at h, return true if first time. For allocations that
// contain prefix data followed by an ObjectData, h should point to the
// start of the whole allocation, not the interior ObjectData.
template <bool apcgc>
inline bool Marker<apcgc>::mark(const HeapObject* h, GCBits marks) {
  assert(h->kind() <= HeaderKind::BigMalloc);
  assert(h->kind() != HeaderKind::AsyncFuncWH);
  assert(h->kind() != HeaderKind::Closure);
  return h->mark(marks) == GCBits::Unmarked;
}

DEBUG_ONLY bool checkEnqueuedKind(const HeapObject* h) {
  switch (h->kind()) {
    case HeaderKind::Apc:
    case HeaderKind::Globals:
    case HeaderKind::Proxy:
    case HeaderKind::Ref:
    case HeaderKind::Resource:
    case HeaderKind::Packed:
    case HeaderKind::Mixed:
    case HeaderKind::Dict:
    case HeaderKind::VecArray:
    case HeaderKind::Keyset:
    case HeaderKind::Empty:
    case HeaderKind::SmallMalloc:
    case HeaderKind::BigMalloc:
      break;
    case HeaderKind::Object:
    case HeaderKind::Vector:
    case HeaderKind::Map:
    case HeaderKind::Set:
    case HeaderKind::Pair:
    case HeaderKind::ImmVector:
    case HeaderKind::ImmMap:
    case HeaderKind::ImmSet:
    case HeaderKind::WaitHandle:
    case HeaderKind::AwaitAllWH:
      // Object kinds. None of these should have native-data, because if they
      // do, the mapped header should be for the NativeData prefix.
      assert(!hasNativeData(h));
      break;
    case HeaderKind::AsyncFuncFrame:
    case HeaderKind::NativeData:
    case HeaderKind::ClosureHdr:
      // these have inner objects, but we queued the outer one.
      break;
    case HeaderKind::String:
      // nothing to queue since strings don't have pointers
      break;
    case HeaderKind::Closure:
    case HeaderKind::AsyncFuncWH:
      // These header types should not be found during heap or slab iteration
      // because they are appended to ClosureHdr or AsyncFuncFrame.
    case HeaderKind::BigObj:
    case HeaderKind::Slab:
    case HeaderKind::Free:
    case HeaderKind::Hole:
      // These header types are not allocated objects; they are handled
      // earlier and should never be queued on the gc worklist.
      always_assert(false && "bad header kind");
      break;
  }
  return true;
}

template <bool apcgc>
void Marker<apcgc>::checkedEnqueue(const void* p, GCBits bits) {
  auto r = find(p);
  if (auto h = r.ptr) {
    // enqueue h the first time. If it's an object with no pointers (eg String),
    // we'll skip it when we process the queue.
    if (mark(h, bits)) {
      marked_ += r.size;
      pinned_ += bits == GCBits::Pin ? r.size : 0;
      enqueue(h);
      assert(checkEnqueuedKind(h));
    }
  } else {
      if (apcgc) {
        // If p doesn't belong to any APC data, APCGCManager won't do anything
        apcgc_->mark(p);
      }
  }
}

// mark ambigous pointers in the range [start,start+len). If the start or
// end is a partial word, don't scan that word.
template <bool apcgc>
void FOLLY_DISABLE_ADDRESS_SANITIZER
Marker<apcgc>::conservativeScan(const void* start, size_t len) {
  constexpr uintptr_t M{7}; // word size - 1
  auto s = (char**)((uintptr_t(start) + M) & ~M); // round up
  auto e = (char**)((uintptr_t(start) + len) & ~M); // round down
  cscanned_ += uintptr_t(e) - uintptr_t(s);
  for (; s < e; s++) {
    checkedEnqueue(
      // Mask off the upper 16-bits to handle things like
      // DiscriminatedPtr which stores things up there.
      (void*)(uintptr_t(*s) & (-1ULL >> 16)),
      GCBits::Pin
    );
  }
}

inline int64_t cpu_ns() {
  return HPHP::Timer::GetThreadCPUTimeNanos();
}

/*
 * If we have non-conservative scanners, we must treat all unknown
 * type-index allocations in the heap as roots. Why? The generated
 * scanners will only report a pointer if it knows the pointer can point
 * to an object on the request heap. It does this by tracking all types
 * which are allocated via the allocation functions via the type-index
 * mechanism. If an allocation has an unknown type-index, then by definition
 * we don't know which type it contains, and therefore the auto generated
 * scanners will never report a pointer to such a type.
 *
 * The only good way to solve this is to treat such allocations as roots
 * and conservative scan them. If we're conservative scanning everything,
 * we need to take no special action, as the above problem only applies to
 * auto generated scanners.
 */

// initially parse the heap to find valid objects and initialize metadata.
template <bool apcgc>
NEVER_INLINE void Marker<apcgc>::init() {
  auto const t0 = cpu_ns();
  SCOPE_EXIT { init_ns_ = cpu_ns() - t0; };
  MM().initFree();
  initfree_ns_ = cpu_ns() - t0;

  auto init = [&](HeapObject* h, size_t size) {
    h->clearMarks();
    allocd_ += size;
  };

  auto init_unknown = [&](HeapObject* h, size_t size) {
    // unknown type for a req::malloc'd block. See rationale above.
    unknown_ += size;
    h->mark(GCBits::Pin);
    enqueue(h);
  };

  heap_.iterate(
    [&](HeapObject* h, size_t size) { // onBig
      ptrs_.insert(h, size);
      if (h->kind() == HeaderKind::BigObj) {
        init(static_cast<MallocNode*>(h) + 1, size - sizeof(MallocNode));
      } else {
        assert(h->kind() == HeaderKind::BigMalloc);
        init(h, size);
        if (!type_scan::isKnownType(static_cast<MallocNode*>(h)->typeIndex())) {
          init_unknown(h, size);
        }
      }
    },
    [&](HeapObject* h, size_t size) { // onSlab
      ptrs_.insert(h, size);
      Slab::fromHeader(h)->initCrossingMap([&](HeapObject* h, size_t size) {
        init(h, size);
        if (h->kind() == HeaderKind::SmallMalloc &&
            !type_scan::isKnownType(static_cast<MallocNode*>(h)->typeIndex())) {
          init_unknown(h, size);
        }
      });
    }
  );
  ptrs_.prepare();
  if (apcgc) {
    apcgc_ = &APCGCManager::getInstance();
  }
}

template <bool apcgc>
void Marker<apcgc>::finish_typescan() {
  type_scanner_.finish(
    [this](const void* p) {
      xscanned_ += sizeof(p);
      checkedEnqueue(p, GCBits::Pin);
    },
    [this](const void* p, std::size_t size) {
      // we could extract the addresses of ambiguous ptrs, if desired.
      conservativeScan(p, size);
    },
    [this](const void** addr) {
      xscanned_ += sizeof(*addr);
      checkedEnqueue(*addr, GCBits::Mark);
    }
  );
}

template <bool apcgc>
NEVER_INLINE void Marker<apcgc>::traceRoots() {
  auto const t0 = cpu_ns();
  SCOPE_EXIT { roots_ns_ = cpu_ns() - t0; };
  iterateRoots([&](const void* p, size_t size, type_scan::Index tyindex) {
    type_scanner_.scanByIndex(tyindex, p, size);
    finish_typescan();
  });
  cscanned_roots_ = cscanned_;
  xscanned_roots_ = xscanned_;
}

template <bool apcgc>
NEVER_INLINE void Marker<apcgc>::trace() {
  auto const t0 = cpu_ns();
  SCOPE_EXIT { mark_ns_ = cpu_ns() - t0; };
  while (!work_.empty()) {
    auto h = work_.back();
    work_.pop_back();
    scanHeapObject(h, type_scanner_);
    finish_typescan();
  }
}

// another pass through the heap, this time using the PtrMap we computed
// in init(). Free and maybe quarantine unmarked objects.
template <bool apcgc>
NEVER_INLINE void Marker<apcgc>::sweep() {
  auto& mm = MM();
  auto const t0 = cpu_ns();
  auto const usage0 = mm.currentUsage();
  MemoryManager::FreelistArray quarantine;
  if (RuntimeOption::EvalQuarantine) quarantine = mm.beginQuarantine();
  SCOPE_EXIT {
    if (RuntimeOption::EvalQuarantine) mm.endQuarantine(std::move(quarantine));
    freed_bytes_ = usage0 - mm.currentUsage();
    sweep_ns_ = cpu_ns() - t0;
    assert(freed_bytes_ >= 0);
  };

  g_context->sweepDynPropTable([&](const ObjectData* obj) {
    auto r = find(obj);
    return !r.ptr || !marked(r.ptr);
  });

  mm.sweepApcArrays([&](APCLocalArray* a) {
    return !marked(a);
  });

  mm.sweepApcStrings([&](StringData* s) {
    return !marked(s);
  });

  mm.initFree();

  heap_.iterate(
    [&](HeapObject* big, size_t big_size) { // onBig
      if (big->kind() == HeaderKind::BigObj) {
        big = static_cast<MallocNode*>(big) + 1;
        if (!marked(big)) {
          mm.freeBigSize(big);
        }
      }
    },
    [&](HeapObject* big, size_t /*big_size*/) { // onSlab
      auto slab = Slab::fromHeader(big);
      slab->find_if((HeapObject*)slab->start(),
        [&](HeapObject* h, size_t h_size) {
          if (!marked(h) && !isFreeKind(h->kind()) &&
              h->kind() != HeaderKind::SmallMalloc) {
            mm.freeSmallSize(h, h_size);
          }
          return false;
        }
      );
    });
  if (apcgc) {
    // This should be removed after global GC API is provided
    // Currently we do this to sweeping only when script mode
    apcgc_->sweep();
  }
}

thread_local bool t_eager_gc{false};
thread_local BloomFilter<256*1024> t_surprise_filter;

// Structured Logging

thread_local std::atomic<size_t> g_req_num;
__thread size_t t_req_num; // snapshot thread-local copy of g_req_num;
__thread size_t t_gc_num; // nth collection in this request.
__thread bool t_enable_samples;
__thread size_t t_trigger;
__thread int64_t t_req_age;
__thread MemoryUsageStats t_pre_stats;

StructuredLogEntry logCommon() {
  StructuredLogEntry sample;
  sample.setInt("req_num", t_req_num);
  // MemoryUsageStats
  sample.setInt("memory_limit", MM().getMemoryLimit());
  sample.setInt("usage", t_pre_stats.usage());
  sample.setInt("mm_usage", t_pre_stats.mmUsage);
  sample.setInt("aux_usage", t_pre_stats.auxUsage());
  sample.setInt("mm_capacity", t_pre_stats.capacity);
  sample.setInt("peak_usage", t_pre_stats.peakUsage);
  sample.setInt("peak_capacity", t_pre_stats.peakCap);
  sample.setInt("total_alloc", t_pre_stats.totalAlloc);
  return sample;
}

template<bool apcgc>
void logCollection(const char* phase, const Marker<apcgc>& mkr) {
  // log stuff
  if (Trace::moduleEnabledRelease(Trace::gc, 1)) {
    Trace::traceRelease(
      "gc age %ldms mmUsage %luM trigger %luM init %lums mark %lums "
      "allocd %luM marked %.1f%% pinned %.1f%% free %.1fM "
      "cscan-heap %.1fM "
      "xscan-heap %.1fM\n",
      t_req_age,
      t_pre_stats.mmUsage/1024/1024,
      t_trigger/1024/1024,
      mkr.init_ns_/1000000,
      mkr.mark_ns_/1000000,
      mkr.allocd_.bytes/1024/1024,
      100.0 * mkr.marked_.bytes / mkr.allocd_.bytes,
      100.0 * mkr.pinned_.bytes / mkr.allocd_.bytes,
      mkr.freed_bytes_/1024.0/1024.0,
      (mkr.cscanned_.bytes - mkr.cscanned_roots_.bytes)/1024.0/1024.0,
      (mkr.xscanned_.bytes - mkr.xscanned_roots_.bytes)/1024.0/1024.0
    );
  }
  auto sample = logCommon();
  sample.setStr("phase", phase);
  std::string scanner(type_scan::hasNonConservative() ? "typescan" : "ts-cons");
  sample.setStr("scanner", !debug ? scanner : scanner + "-debug");
  sample.setInt("gc_num", t_gc_num);
  sample.setInt("req_age_micros", t_req_age);
  // timers of gc-sub phases
  sample.setInt("init_micros", mkr.init_ns_/1000);
  sample.setInt("initfree_micros", mkr.initfree_ns_/1000);
  sample.setInt("roots_micros", mkr.roots_ns_/1000);
  sample.setInt("mark_micros", mkr.mark_ns_/1000);
  sample.setInt("sweep_micros", mkr.sweep_ns_/1000);
  // size metrics gathered during gc
  sample.setInt("allocd_bytes", mkr.allocd_.bytes);
  sample.setInt("allocd_objects", mkr.allocd_.count);
  sample.setInt("allocd_span", mkr.ptrs_.span().second);
  sample.setInt("marked_bytes", mkr.marked_.bytes);
  sample.setInt("pinned_bytes", mkr.pinned_.bytes);
  sample.setInt("unknown_bytes", mkr.unknown_.bytes);
  sample.setInt("freed_bytes", mkr.freed_bytes_);
  sample.setInt("trigger_bytes", t_trigger);
  sample.setInt("cscanned_roots", mkr.cscanned_roots_.bytes);
  sample.setInt("xscanned_roots", mkr.xscanned_roots_.bytes);
  sample.setInt("cscanned_heap",
                mkr.cscanned_.bytes - mkr.cscanned_roots_.bytes);
  sample.setInt("xscanned_heap",
                mkr.xscanned_.bytes - mkr.xscanned_roots_.bytes);
  sample.setInt("rds_normal_size", rds::normalSection().size());
  sample.setInt("rds_normal_count", rds::detail::s_normal_alloc_descs.size());
  sample.setInt("rds_local_size", rds::localSection().size());
  sample.setInt("rds_local_count", rds::detail::s_local_alloc_descs.size());
  sample.setInt("max_worklist", mkr.max_worklist_);
  StructuredLog::log("hhvm_gc", sample);
}

void collectImpl(HeapImpl& heap, const char* phase) {
  VMRegAnchor _;
  if (t_eager_gc && RuntimeOption::EvalFilterGCPoints) {
    t_eager_gc = false;
    auto pc = vmpc();
    if (t_surprise_filter.test(pc)) return;
    t_surprise_filter.insert(pc);
    TRACE(2, "eager gc %s at %p\n", phase, pc);
    phase = "eager";
  } else {
    TRACE(2, "normal gc %s at %p\n", phase, vmpc());
  }
  if (t_gc_num == 0) {
    t_enable_samples = StructuredLog::coinflip(RuntimeOption::EvalGCSampleRate);
  }
  if (t_enable_samples) {
    t_pre_stats = MM().getStatsCopy(); // don't check or trigger OOM
  }
  if (RuntimeOption::EvalGCForAPC) {
    Marker<true> mkr(heap);
    mkr.init();
    mkr.traceRoots();
    mkr.trace();
    mkr.sweep();
    if (t_enable_samples) {
      logCollection(phase, mkr);
    }
    ++t_gc_num;
  } else {
    Marker<false> mkr(heap);
    mkr.init();
    mkr.traceRoots();
    mkr.trace();
    mkr.sweep();
    if (t_enable_samples) {
      logCollection(phase, mkr);
    }
    ++t_gc_num;
  }
}

}

void MemoryManager::resetGC() {
  t_req_num = ++g_req_num;
  t_gc_num = 0;
  updateNextGc();
}

void MemoryManager::resetEagerGC() {
  if (RuntimeOption::EvalEagerGC && RuntimeOption::EvalFilterGCPoints) {
    t_surprise_filter.clear();
  }
}

void MemoryManager::requestEagerGC() {
  if (RuntimeOption::EvalEagerGC && rds::header()) {
    t_eager_gc = true;
    setSurpriseFlag(PendingGCFlag);
  }
}

void MemoryManager::requestGC() {
  if (this->isGCEnabled() && rds::header()) {
    if (m_stats.mmUsage > m_nextGc) {
      setSurpriseFlag(PendingGCFlag);
    }
  }
}

void MemoryManager::updateNextGc() {
  auto stats = getStatsCopy();
  auto mm_limit = m_usageLimit - stats.auxUsage();
  int64_t delta = (mm_limit - stats.mmUsage) *
                  RuntimeOption::EvalGCTriggerPct;
  delta = std::max(delta, RuntimeOption::EvalGCMinTrigger);
  m_nextGc = stats.mmUsage + delta;
}

void MemoryManager::collect(const char* phase) {
  if (empty()) return;
  t_req_age = cpu_ns()/1000 - m_req_start_micros;
  t_trigger = m_nextGc;
  collectImpl(m_heap, phase);
  updateNextGc();
}

void MemoryManager::setMemoryLimit(size_t limit) {
  assert(limit <= (size_t)std::numeric_limits<int64_t>::max());
  m_usageLimit = limit;
  updateNextGc();
}

}
