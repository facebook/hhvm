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
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/heap-scan.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/base/vanilla-dict-defs.h"
#include "hphp/runtime/base/weakref-data.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/alloc.h"
#include "hphp/util/cycles.h"
#include "hphp/util/ptr-map.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"
#include "hphp/util/type-scan.h"

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <folly/portability/Unistd.h>
#include <folly/Range.h>
#include <iterator>
#include <vector>

namespace HPHP {
TRACE_SET_MOD(gc);

RDS_LOCAL_NO_CHECK(RequestLocalGCData, rl_gcdata);
IMPLEMENT_RDS_LOCAL_HOTVALUE(bool, t_eager_gc);

namespace {

struct Counter {
  size_t count{0};
  size_t bytes{0};
  void operator+=(size_t n) {
    bytes += n;
    count++;
  }
};

constexpr auto MinMark = GCBits(1);
constexpr auto MaxMark = GCBits(3);

/*
 * GC Runtime Options
 *
 * Eval.EnableGC - Default value of the per-request MemoryManager::m_gc_enabled
 * flag. This flag can be dynamically set/cleared by PHP via
 * ini_set("zend.enable_gc"). In turn, m_gc_enabled enables automatic background
 * garbage collection. If not enabled, gc_collect_cycles() won't run.
 *
 * Eval.EagerGC - If set, trigger collection after every allocation, in debug
 * builds. Has no effect in opt builds or when m_gc_enabled == false.
 *
 * Eval.FilterGCPoints - If true, use a bloom filter to only do an eager
 * collection once per unique VMPC. This makes eager mode fast enough to be
 * usable for unit tests, and almost tolerable for large integration tests.
 *
 * Eval.GCSampleRate - per *request* sample rate to enable GC logging.
 * If coinflip is true, every GC for the current request will be logged.
 * Note this is not the per-collection sample rate: we do one coinflip per
 * request.
 *
 * Eval.GCMinTrigger - Minimum heap growth, in bytes since the last collection,
 * before triggering the next collection. See MemoryManager::updateNextGc().
 *
 * Eval.GCTriggerPct - Minimum heap growth, as a percent of remaining heap
 * space, before triggering the next collection. see updateNextGC().
 *
 * Eval.Quarantine - If true, objects swept by GC will be trash filled and
 * leaked, never reallocated.
 *
 * Experimental options
 *
 * Eval.TwoPhaseGC - perform tracing in two phases, the second of which
 * must only encounter exactly-scanned pointers, to enable object copying.
 */

/*
 * Collector state needed during a single whole-heap mark-sweep collection.
 */
struct Collector {
  explicit Collector(HeapImpl& heap, GCBits mark_version)
    : heap_(heap), mark_version_{mark_version}
  {}
  void collect();
  void init();
  void sweep();
  void traceAll();
  void traceConservative();
  void traceExact();

  // mark ambiguous pointers in the range [start,start+len)
  void conservativeScan(const void* start, size_t len);

  bool marked(const HeapObject* h) {
    return h->marks() == mark_version_;
  }
  void checkedEnqueue(const void* p);
  void exactEnqueue(const void* p);
  HeapObject* find(const void*);

  size_t slab_index(const void* h) {
    assertx((char*)h >= (char*)slabs_range_.ptr &&
           (char*)h < (char*)slabs_range_.ptr + slabs_range_.size);
    return (uintptr_t(h) - uintptr_t(slabs_range_.ptr)) >> kLgSlabSize;
  }

  HeapImpl& heap_;
  GCBits const mark_version_;
  size_t num_small_{0}, num_big_{0}, num_slabs_{0};
  size_t marked_{0}, pinned_{0}, unknown_{0}; // object counts
  Counter cscanned_roots_, cscanned_; // bytes
  Counter xscanned_roots_, xscanned_; // bytes
  size_t init_ns_, initfree_ns_, roots_ns_{0}, mark_ns_{0}, sweep_ns_;
  size_t max_worklist_{0}; // max size of cwork_ + xwork_
  size_t freed_bytes_{0};
  PtrMap<const HeapObject*> ptrs_;
  MemBlock slabs_range_;
  boost::dynamic_bitset<> slab_map_; // 1 bit per 2M
  type_scan::Scanner type_scanner_;
  std::vector<const HeapObject*> cwork_, xwork_;
};

HeapObject* Collector::find(const void* ptr) {
  if (uintptr_t(ptr) - uintptr_t(slabs_range_.ptr) < slabs_range_.size &&
      slab_map_.test(slab_index(ptr))) {
    return Slab::fromPtr(ptr)->find(ptr);
  }
  return const_cast<HeapObject*>(ptrs_.start(ptr));
}

DEBUG_ONLY bool checkEnqueuedKind(const HeapObject* h) {
  switch (h->kind()) {
    case HeaderKind::Resource:
    case HeaderKind::ClsMeth:
    case HeaderKind::RClsMeth:
    case HeaderKind::Cpp:
    case HeaderKind::SmallMalloc:
    case HeaderKind::BigMalloc:
    case HeaderKind::String:
    case HeaderKind::Vec:
    case HeaderKind::Dict:
    case HeaderKind::Keyset:
    case HeaderKind::BespokeVec:
    case HeaderKind::BespokeDict:
    case HeaderKind::BespokeKeyset:
    case HeaderKind::RFunc:
      break;
    case HeaderKind::Free:
    case HeaderKind::Hole:
      // these can be on the worklist because we don't expect to find
      // dangling pointers. they are ignored when popped from the worklist.
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
    case HeaderKind::ConcurrentWH:
      // Object kinds. None of these have native-data, because if they
      // do, the mapped header should be for the NativeData prefix.
      break;
    case HeaderKind::AsyncFuncFrame:
    case HeaderKind::NativeData:
    case HeaderKind::ClosureHdr:
    case HeaderKind::MemoData:
      // these have inner objects, but we queued the outer one.
      break;
    case HeaderKind::Closure:
    case HeaderKind::AsyncFuncWH:
    case HeaderKind::NativeObject:
      // These header types should not be found during heap or slab iteration
      // because they are appended to ClosureHdr or AsyncFuncFrame.
    case HeaderKind::Slab:
      // These header types are not allocated objects; they are handled
      // earlier and should never be queued on the gc worklist.
      always_assert(false && "bad header kind");
      break;
  }
  return true;
}

bool willScanConservative(const HeapObject* h) {
  return (h->kind() == HeaderKind::SmallMalloc ||
          h->kind() == HeaderKind::BigMalloc) &&
         type_scan::hasConservativeScanner(
             static_cast<const MallocNode*>(h)->typeIndex()
         );
}

void Collector::checkedEnqueue(const void* p) {
  if (auto h = find(p)) {
    // enqueue h the first time. If it's an object with no pointers (eg String),
    // we'll skip it when we process the queue.
    auto old = h->marks();
    if (old != mark_version_) {
      h->setmarks(mark_version_);
      ++marked_;
      auto& work = willScanConservative(h) ? cwork_ : xwork_;
      work.push_back(h);
      max_worklist_ = std::max(max_worklist_, cwork_.size() + xwork_.size());
      assertx(checkEnqueuedKind(h));
    }
  }
}

// It is correct to ignore willScanConservative(h) in phase 2 because:
// * target is !type_scan::isKnownType, making it an "unknown" root,
// and scanned & pinned in phase 1; OR
// * target is a marked (thus pinned) req::container buffer, found in phase 1,
// so we can disregard this pointer to it, since it won't move; OR
// * target is an unmarked req::container buffer. p is a (possibly interior)
// pointer into it. p shouldn't keep the buffer alive, since whoever
// owns it, will scan it using the container's iterator api; OR
// * p could be a stale pointer of any interesting type, that randomly
// is pointing to recycled memory. ignoring it is actually desireable.
void Collector::exactEnqueue(const void* p) {
  if (auto h = find(p)) {
    auto old = h->marks();
    if (old != mark_version_ && !willScanConservative(h)) {
      h->setmarks(mark_version_);
      ++marked_;
      xwork_.push_back(h);
      max_worklist_ = std::max(max_worklist_, xwork_.size());
      assertx(checkEnqueuedKind(h));
    }
  }
}

// mark ambiguous pointers in the range [start,start+len). If the start or
// end is a partial word, don't scan that word.
void FOLLY_DISABLE_ADDRESS_SANITIZER
Collector::conservativeScan(const void* start, size_t len) {
  if (len < sizeof(uintptr_t)) return;
  cscanned_ += len - sizeof(uintptr_t) + 1;
  auto s = (char*)start;
  auto const e = s + len - sizeof(uintptr_t);
  for (; s <= e; ++s) {
    checkedEnqueue(
      // Mask off the upper 16-bits to handle things like
      // DiscriminatedPtr which stores things up there.
      (void*)(*(uintptr_t*)s & (-1ULL >> 16))
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
NEVER_INLINE void Collector::init() {
  auto const t0 = cpu_ns();
  SCOPE_EXIT {
    init_ns_ = cpu_ns() - t0;
  };
  tl_heap->initFree();
  initfree_ns_ = cpu_ns() - t0;

  slabs_range_ = heap_.slab_range();
  slab_map_.resize((slabs_range_.size + kSlabSize - 1) >> kLgSlabSize);

  heap_.iterate(
    [&](HeapObject* h, size_t size) { // onBig
      ptrs_.insert(h, size);
      if (h->kind() == HeaderKind::BigMalloc &&
          !type_scan::isKnownType(static_cast<MallocNode*>(h)->typeIndex())) {
        ++unknown_;
        h->setmarks(mark_version_);
        cwork_.push_back(h);
      }
    },
    [&](HeapObject* h, size_t size) { // onSlab
      slab_map_.set(slab_index(h));
    }
  );
  ptrs_.prepare();
}

// Collect the heap using mark/sweep.
//
// Init: prepare object-start bitmaps, and mark/enqueue unknown allocations.
//
// Trace (two-phase):
// 1. scan all conservative roots, or hybrid roots which might have
//    conservative fields. Also scan any conservative heap objects reached
//    via conservative scanning. After phase 1, all conservative scanning is
//    done and it's safe to move objects while tracing.
// 2. scan all exact roots and exact heap objects. Ignore any exactly scanned
//    pointers to conservatively scanned objects (see comments in exactEnqueue()
//    this is safe).
//
// Trace (one-phase). This is used if no exact type_scanners are available.
// 1. scan all roots, then the transitive closures of all heap objects,
//    with no moving.
//
// Sweep:
// 1. iterate through any tables holding "weak" pointers, clearing entries
//    if the target(s) aren't marked, including nulling out WeakRef objects.
// 2. free all unmarked objects, except SmallMalloc/BigMalloc nodes: We don't
//    sweep "unknown" allocations or req::container buffers, because we don't
//    expect to have found all pointers to them. Any other objects allocated
//    this way are treated similarly.

void Collector::collect() {
#if FOLLY_SANITIZE
  // TODO(#31665421)
  return;
#endif
  init();
  if (type_scan::hasNonConservative() && RuntimeOption::EvalTwoPhaseGC) {
    traceConservative();
    traceExact();
  } else {
    traceAll();
  }
  sweep();
}

// Phase 1: Scan only conservative or mixed conservative/exact roots, plus any
// malloc'd heap objects that are themselves fully conservatively scanned.
NEVER_INLINE void Collector::traceConservative() {
  auto finish = [&] {
    for (auto r : type_scanner_.m_conservative) {
      conservativeScan(r.first, r.second);
    }
    type_scanner_.m_conservative.clear();
    // Accumulate m_addrs until traceExact()
    // Accumulate m_weak until sweep()
  };
  auto const t0 = cpu_ns();
  iterateConservativeRoots(
    [&](const void* p, size_t size, type_scan::Index tyindex) {
      type_scanner_.scanByIndex(tyindex, p, size);
      finish();
    });
  auto const t1 = cpu_ns();
  roots_ns_ += t1 - t0;
  cscanned_roots_ = cscanned_;
  while (!cwork_.empty()) {
    auto h = cwork_.back();
    cwork_.pop_back();
    scanHeapObject(h, type_scanner_);
    finish();
  }
  mark_ns_ += cpu_ns() - t1;
  pinned_ = marked_;
}

// Phase 2: Scan pointers deferred from phase 1, exact roots, and the remainder
// of the heap, which is expected to be fully exactly-scannable. Assert if
// any conservatively-scanned regions are found in this phase. Any unmarked
// objects found in this phase may be safely copied.
NEVER_INLINE void Collector::traceExact() {
  auto finish = [&] {
    assertx(cwork_.empty() && type_scanner_.m_conservative.empty());
    for (auto addr : type_scanner_.m_addrs) {
      xscanned_ += sizeof(*addr);
      exactEnqueue(*addr);
    }
    type_scanner_.m_addrs.clear();
    // Accumulate m_weak until sweep()
  };
  auto const t0 = cpu_ns();
  finish(); // from phase 1
  iterateExactRoots(
    [&](const void* p, size_t size, type_scan::Index tyindex) {
      type_scanner_.scanByIndex(tyindex, p, size);
      finish();
    });
  auto const t1 = cpu_ns();
  roots_ns_ += t1 - t0;
  xscanned_roots_ = xscanned_;
  while (!xwork_.empty()) {
    auto h = xwork_.back();
    xwork_.pop_back();
    scanHeapObject(h, type_scanner_);
    finish();
  }
  mark_ns_ += cpu_ns() - t1;
}

// Scan all roots & heap in one pass
NEVER_INLINE void Collector::traceAll() {
  auto finish = [&] {
    for (auto r : type_scanner_.m_conservative) {
      conservativeScan(r.first, r.second);
    }
    type_scanner_.m_conservative.clear();
    for (auto addr : type_scanner_.m_addrs) {
      xscanned_ += sizeof(*addr);
      checkedEnqueue(*addr);
    }
    type_scanner_.m_addrs.clear();
    // Accumulate m_weak until sweep()
  };
  auto const t0 = cpu_ns();
  iterateRoots([&](const void* p, size_t size, type_scan::Index tyindex) {
    type_scanner_.scanByIndex(tyindex, p, size);
    finish();
  });
  auto const t1 = cpu_ns();
  roots_ns_ += t1 - t0;
  cscanned_roots_ = cscanned_;
  xscanned_roots_ = xscanned_;
  while (!cwork_.empty() || !xwork_.empty()) {
    auto& work = !cwork_.empty() ? cwork_ : xwork_;
    auto h = work.back();
    work.pop_back();
    scanHeapObject(h, type_scanner_);
    finish();
  }
  mark_ns_ += cpu_ns() - t1;
  pinned_ = marked_;
}

// another pass through the heap, this time using the PtrMap we computed
// in init(). Free and maybe quarantine unmarked objects.
NEVER_INLINE void Collector::sweep() {
#if FOLLY_SANITIZE
  // TODO(#31665421)
  return;
#endif
  auto& mm = *tl_heap;
  auto const t0 = cpu_ns();
  auto const usage0 = mm.currentUsage();
  MemoryManager::FreelistArray quarantine;
  if (RuntimeOption::EvalQuarantine) quarantine = mm.beginQuarantine();
  SCOPE_EXIT {
    if (RuntimeOption::EvalQuarantine) mm.endQuarantine(std::move(quarantine));
    freed_bytes_ = usage0 - mm.currentUsage();
    sweep_ns_ = cpu_ns() - t0;
    assertx(freed_bytes_ >= 0);
  };

  // Clear weak references as needed.
  for (auto w : type_scanner_.m_weak) {
    auto wr_data = static_cast<const WeakRefData*>(w);
    auto type = wr_data->pointee.m_type;
    if (type == KindOfObject) {
      auto h = find(wr_data->pointee.m_data.pobj);
      if (!marked(h)) {
        // It's important we invalidate the pointer stored in the weakref, and
        // not the start of the allocation.  In the case of objects with
        // native data, the start of allocation may not be the start of the
        // ObjectData*.
        WeakRefData::invalidateWeakRef(uintptr_t(wr_data->pointee.m_data.pobj));
        mm.reinitFree();
      }
      continue;
    }
    assertx(type == KindOfNull || type == KindOfUninit);
  }
  type_scanner_.m_weak.clear();

  bool need_reinit_free = false;
  g_context->sweepDynPropTable([&](const ObjectData* obj) {
    if (need_reinit_free) mm.reinitFree();
    auto h = find(obj);
    // if we return true, call reinitFree() before calling find() again,
    // to ensure the heap remains walkable.
    return need_reinit_free = !h || !marked(h);
  });

  mm.reinitFree();

  heap_.iterate(
    [&](HeapObject* big, size_t big_size) { // onBig
      ++num_big_;
      auto kind = big->kind();
      if (kind != HeaderKind::BigMalloc && kind != HeaderKind::SmallMalloc &&
          !marked(big)) {
        // NB: kind == SmallMalloc occurs when tl_heap->m_bypassSlabAlloc==true
        mm.freeBigSize(big);
      }
    },
    [&](HeapObject* big, size_t /*big_size*/) { // onSlab
      ++num_slabs_;
      auto slab = Slab::fromHeader(big);
      HeapObject* h = (HeapObject*)slab->start();
      HeapObject* end = (HeapObject*)slab->end();
      do {
        auto size = allocSize(h);
        ++num_small_;
        auto kind = h->kind();
        if (!isFreeKind(kind) && kind != HeaderKind::SmallMalloc &&
            !marked(h)) {
          mm.freeSmallSize(h, allocSize(h));
        }
        h = (HeapObject*)((char*)h + size);
      } while (h < end);
      assertx(h == end); // otherwise, last object was truncated
    });
}

StructuredLogEntry logCommon() {
  StructuredLogEntry sample;
  sample.setInt("req_num", rl_gcdata->t_req_num);
  // MemoryUsageStats
  sample.setInt("memory_limit", tl_heap->getMemoryLimit());
  sample.setInt("usage", rl_gcdata->t_pre_stats.usage());
  sample.setInt("mm_usage", rl_gcdata->t_pre_stats.mmUsage());
  sample.setInt("mm_allocated", rl_gcdata->t_pre_stats.mmAllocated());
  sample.setInt("aux_usage", rl_gcdata->t_pre_stats.auxUsage());
  sample.setInt("mm_capacity", rl_gcdata->t_pre_stats.capacity());
  sample.setInt("peak_usage", rl_gcdata->t_pre_stats.peakUsage);
  sample.setInt("peak_capacity", rl_gcdata->t_pre_stats.peakCap);
  sample.setInt("total_alloc", rl_gcdata->t_pre_stats.totalAlloc);
  return sample;
}

void traceCollection(const Collector& collector) {
  constexpr auto MB = 1024 * 1024;
  auto const cscanned_heap = collector.cscanned_.bytes -
    collector.cscanned_roots_.bytes;
  auto const xscanned_heap = collector.xscanned_.bytes -
    collector.xscanned_roots_.bytes;
  auto const total_ns = collector.init_ns_ + collector.initfree_ns_ +
    collector.roots_ns_ + collector.mark_ns_ + collector.sweep_ns_;
  Trace::ftraceRelease(
    "gc age {}ms mmUsage {}M trigger {}M "
    "init {}ms mark {}ms sweep {}ms total {}ms "
    "marked {} pinned {} free {:.1f}M "
    "cscan-heap {:.1f}M xscan-heap {:.1f}M\n",
    rl_gcdata->t_req_age,
    rl_gcdata->t_pre_stats.mmUsage() / MB,
    rl_gcdata->t_trigger / MB,
    collector.init_ns_ / 1000000,
    collector.mark_ns_ / 1000000,
    collector.sweep_ns_ / 1000000,
    total_ns / 1000000,
    collector.marked_,
    collector.pinned_,
    double(collector.freed_bytes_) / MB,
    double(cscanned_heap) / MB,
    double(xscanned_heap) / MB
  );
}

void logCollection(const char* phase, const Collector& collector) {
  auto sample = logCommon();
  sample.setStr("phase", phase);
  std::string scanner(type_scan::hasNonConservative() ? "typescan" : "ts-cons");
  sample.setStr("scanner", !debug ? scanner : scanner + "-debug");
  sample.setInt("gc_num", rl_gcdata->t_gc_num);
  sample.setInt("req_age_micros", rl_gcdata->t_req_age);
  // timers of gc-sub phases
  sample.setInt("init_micros", collector.init_ns_/1000);
  sample.setInt("initfree_micros", collector.initfree_ns_/1000);
  sample.setInt("roots_micros", collector.roots_ns_/1000);
  sample.setInt("mark_micros", collector.mark_ns_/1000);
  sample.setInt("sweep_micros", collector.sweep_ns_/1000);
  // object metrics counted at sweep time
  sample.setInt("slab_count", collector.num_slabs_);
  sample.setInt("small_count", collector.num_small_);
  sample.setInt("big_count", collector.num_big_);
  // size metrics gathered during gc
  sample.setInt("allocd_span", collector.ptrs_.span().second);
  sample.setInt("marked_count", collector.marked_);
  sample.setInt("pinned_count", collector.pinned_);
  sample.setInt("unknown_count", collector.unknown_);
  sample.setInt("freed_bytes", collector.freed_bytes_);
  sample.setInt("trigger_bytes", rl_gcdata->t_trigger);
  sample.setInt("trigger_allocated", rl_gcdata->t_trigger_allocated);
  sample.setInt("cscanned_roots", collector.cscanned_roots_.bytes);
  sample.setInt("xscanned_roots", collector.xscanned_roots_.bytes);
  sample.setInt("cscanned_heap",
                collector.cscanned_.bytes - collector.cscanned_roots_.bytes);
  sample.setInt("xscanned_heap",
                collector.xscanned_.bytes - collector.xscanned_roots_.bytes);
  sample.setInt("rds_normal_size", rds::normalSection().size());
  sample.setInt("rds_normal_count", rds::detail::s_normal_alloc_descs.size());
  sample.setInt("rds_local_size", rds::localSection().size());
  sample.setInt("rds_local_count", rds::detail::s_local_alloc_descs.size());
  sample.setInt("max_worklist", collector.max_worklist_);
  StructuredLog::log("hhvm_gc", sample);
}

void collectImpl(HeapImpl& heap, const char* phase, GCBits& mark_version) {
  VMRegAnchor _;
  if (t_eager_gc && RuntimeOption::EvalFilterGCPoints) {
    t_eager_gc = false;
    auto const pc = vmpc();
    if (rl_gcdata->t_surprise_filter.test(pc)) return;
    rl_gcdata->t_surprise_filter.insert(pc);
    TRACE(2, "eager gc %s at %p\n", phase, pc);
    phase = "eager";
  } else {
    TRACE(2, "normal gc %s at %p\n", phase, vmpc());
  }
  if (rl_gcdata->t_gc_num == 0) {
    rl_gcdata->t_enable_samples =
      StructuredLog::coinflip(RuntimeOption::EvalGCSampleRate);
  }
  rl_gcdata->t_pre_stats =
    tl_heap->getStatsCopy(); // don't check or trigger OOM
  mark_version = (mark_version == MaxMark) ? MinMark :
                 GCBits(uint8_t(mark_version) + 1);
  Collector collector(heap, mark_version);
  collector.collect();
  if (Trace::moduleEnabledRelease(Trace::gc, 1)) {
    traceCollection(collector);
  }
  if (rl_gcdata->t_enable_samples) {
    logCollection(phase, collector);
  }
  rl_gcdata->t_total_gc_ns += collector.init_ns_ + collector.initfree_ns_
    + collector.roots_ns_ + collector.mark_ns_ + collector.sweep_ns_;
  ++rl_gcdata->t_gc_num;
}

}

void MemoryManager::resetGC() {
  rl_gcdata->t_req_num = ++(rl_gcdata->g_req_num);
  rl_gcdata->t_gc_num = 0;
  if (rds::header()) updateNextGc();
}

void MemoryManager::resetEagerGC() {
  if (RuntimeOption::EvalEagerGC && RuntimeOption::EvalFilterGCPoints) {
    rl_gcdata->t_surprise_filter.clear();
  }
}

void MemoryManager::requestEagerGC() {
  if (RuntimeOption::EvalEagerGC && rds::header()) {
    t_eager_gc = true;
    setSurpriseFlag(PendingGCFlag);
  }
}

void MemoryManager::checkGC() {
  if (m_stats.mmUsage() > m_nextGC) {
    assertx(rds::header());
    setSurpriseFlag(PendingGCFlag);
    if (rl_gcdata->t_trigger_allocated == -1) {
      rl_gcdata->t_trigger_allocated = m_stats.mmAllocated();
    }
  }
}

/*
 * Compute the next threshold to trigger GC. We wish to ignore auxUsage
 * for the purpose of this calculation, even though auxUsage is counted
 * against the request for the sake of OOM. To accomplish this, subtract
 * auxUsage from the heap limit, before our calculations.
 *
 * GC will then be triggered the next time we notice mmUsage > m_nextGc (see
 * checkGC()).
 */
void MemoryManager::updateNextGc() {
  rl_gcdata->t_trigger_allocated = -1;
  if (!isGCEnabled()) {
    m_nextGC = kNoNextGC;
    updateMMDebt();
    return;
  }

  auto const stats = getStatsCopy();
  auto const clearance =
    static_cast<uint64_t>(m_usageLimit) -
    stats.auxUsage() - stats.mmUsage();

  int64_t delta = clearance > std::numeric_limits<int64_t>::max() ?
    0 : clearance * RuntimeOption::EvalGCTriggerPct;
  delta = std::max(delta, RuntimeOption::EvalGCMinTrigger);
  m_nextGC = stats.mmUsage() + delta;
  updateMMDebt();
}

void MemoryManager::collect(const char* phase) {
  if (!isGCEnabled()) return;
  if (empty()) return;
  rl_gcdata->t_req_age = cpu_ns()/1000 - m_req_start_micros;
  rl_gcdata->t_trigger = m_nextGC;
  collectImpl(m_heap, phase, m_mark_version);
  updateNextGc();
}

void MemoryManager::setMemoryLimit(size_t limit) {
  assertx(limit <= (size_t)std::numeric_limits<int64_t>::max());
  m_usageLimit = limit;
  updateNextGc();
}

}
