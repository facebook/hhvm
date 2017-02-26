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
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/heap-scan.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/alloc.h"
#include "hphp/util/process.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/trace.h"
#include "hphp/util/type-scan.h"
#include "hphp/util/cycles.h"
#include "hphp/util/timer.h"

#include <algorithm>
#include <iterator>
#include <vector>
#include <folly/Range.h>
#include <folly/portability/Unistd.h>
#include <boost/dynamic_bitset.hpp>

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

struct Marker {
  explicit Marker() {}
  void init();
  void traceRoots();
  void trace();
  void sweep();

  // drain the scanner, enqueue pointers
  void finish_scan();

  // mark ambiguous pointers in the range [start,start+len)
  void conservativeScan(const void* start, size_t len);

  bool mark(const void*, GCBits = GCBits::Mark);
  void checkedEnqueue(const void* p, GCBits bits);
  void finish_typescan();

  void enqueue(const Header* h) {
    assert(h && h->kind() <= HeaderKind::BigMalloc &&
           h->kind() != HeaderKind::AsyncFuncWH &&
           h->kind() != HeaderKind::Closure);
    assert(!isObjectKind(h->kind()) ||
           !h->obj_.getAttribute(ObjectData::HasNativeData));
    work_.push_back(h);
    max_worklist_ = std::max(max_worklist_, work_.size());
  }

  // Whether the object with the given type-index should be recorded as an
  // "unknown" object.
  bool typeIndexIsUnknown(type_scan::Index tyindex) const {
    return type_scan::hasNonConservative() &&
      tyindex == type_scan::kIndexUnknown;
  }

  Counter allocd_, marked_, pinned_, freed_, unknown_; // bytes
  Counter cscanned_roots_, cscanned_; // bytes
  Counter xscanned_roots_, xscanned_; // bytes
  size_t init_us_, initfree_us_, roots_us_, mark_us_, unknown_us_, sweep_us_;
  size_t max_worklist_{0}; // max size of work_
  PtrMap ptrs_;
  type_scan::Scanner type_scanner_;
  std::vector<const Header*> work_;
  std::vector<const Header*> unknown_objects_; // objs w/ unknown typescan id
};

// mark the object at p, return true if first time.
inline bool Marker::mark(const void* p, GCBits marks) {
  assert(p && ptrs_.isHeader(p));
  auto h = static_cast<const Header*>(p);
  assert(h->kind() <= HeaderKind::BigMalloc &&
         h->kind() != HeaderKind::AsyncFuncWH);
  return h->hdr_.mark(marks) == GCBits::Unmarked;
}

DEBUG_ONLY bool checkEnqueuedKind(const Header* h) {
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
      assert(!h->obj_.getAttribute(ObjectData::HasNativeData));
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
    case HeaderKind::BigObj:
    case HeaderKind::Free:
    case HeaderKind::Hole:
      // None of these kinds should be encountered because they're either not
      // interesting to begin with, or are mapped to different headers, so we
      // shouldn't get these from the pointer map.
      always_assert(false && "bad header kind");
      break;
  }
  return true;
}

void Marker::checkedEnqueue(const void* p, GCBits bits) {
  if (auto h = ptrs_.header(p)) {
    // enqueue h the first time. If it's an object with no pointers (eg String),
    // we'll skip it when we process the queue.
    if (mark(h, bits)) {
      enqueue(h);
      assert(checkEnqueuedKind(h));
    }
  }
}

// mark ambigous pointers in the range [start,start+len). If the start or
// end is a partial word, don't scan that word.
void FOLLY_DISABLE_ADDRESS_SANITIZER
Marker::conservativeScan(const void* start, size_t len) {
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

inline int64_t cpu_micros() {
  return HPHP::Timer::GetThreadCPUTimeNanos() / 1000;
}

// initially parse the heap to find valid objects and initialize metadata.
NEVER_INLINE void Marker::init() {
  auto const t0 = cpu_micros();
  SCOPE_EXIT { init_us_ = cpu_micros() - t0; };
  MM().initFree();
  initfree_us_ = cpu_micros() - t0;
  auto tyindex_max = type_scan::Index(
      type_scan::detail::g_metadata_table_size
  );
  MM().iterate([&](Header* h, size_t allocSize) {
    auto kind = h->kind();
    if (kind == HeaderKind::Free) return;
    h->hdr_.clearMarks();
    allocd_ += allocSize;
    ptrs_.insert(h, allocSize);
    if (type_scan::hasNonConservative()) {
      auto tyindex = kind == HeaderKind::Resource ? h->res_.typeIndex() :
                     kind == HeaderKind::NativeData ? h->native_.typeIndex() :
                     kind == HeaderKind::SmallMalloc ? h->malloc_.typeIndex() :
                     kind == HeaderKind::BigMalloc ? h->malloc_.typeIndex() :
                     tyindex_max;
      if (tyindex == type_scan::kIndexUnknown) {
        unknown_objects_.emplace_back(h);
        unknown_ += allocSize;
      }
    }
  });
  ptrs_.prepare();
}

void Marker::finish_typescan() {
  type_scanner_.finish(
    [this](const void* p, const char*) {
      xscanned_ += sizeof(p);
      checkedEnqueue(p, GCBits::Pin);
    },
    [this](const void* p, std::size_t size, const char*) {
      // we could extract the addresses of ambiguous ptrs, if desired.
      conservativeScan(p, size);
    },
    [this](const void** addr, const char*) {
      xscanned_ += sizeof(*addr);
      checkedEnqueue(*addr, GCBits::Mark);
    }
  );
}

NEVER_INLINE void Marker::traceRoots() {
  auto const t0 = cpu_micros();
  SCOPE_EXIT { roots_us_ = cpu_micros() - t0; };
  scanRoots(type_scanner_);
  finish_typescan();
  cscanned_roots_ = cscanned_;
  xscanned_roots_ = xscanned_;
}

NEVER_INLINE void Marker::trace() {
  auto const t0 = cpu_micros();
  SCOPE_EXIT { mark_us_ = cpu_micros() - t0; };
  const auto process_worklist = [this](){
    while (!work_.empty()) {
      auto h = work_.back();
      work_.pop_back();
      scanHeader(h, type_scanner_);
      finish_typescan();
    }
  };

  process_worklist();

  /*
   * If the type-scanners has non-conservative scanners, we must treat all
   * unknown type-index allocations in the heap as roots. Why? The auto
   * generated scanners will only report a pointer if it knows the pointer can
   * point to an object on the request heap. It does this by tracking all types
   * which are allocated via the allocation functions via the type-index
   * mechanism. If an allocation has an unknown type-index, then by definition
   * we don't know which type it contains, and therefore the auto generated
   * scanners will never report a pointer to such a type. So, if there's a
   * countable object which is only reachable via one of these unknown
   * type-index allocations, we'll garbage collect that countable object even if
   * the unknown type-index allocation is reachable. The only good way to solve
   * this is to treat such allocations as roots and always conservative scan
   * them. If we're conservative scanning everything, we need to take no special
   * action, as the above problem only applies to auto generated scanners.
   *
   * Do this after draining the worklist, as we want to prefer discovering
   * things via non-conservative means.
   */
  if (!unknown_objects_.empty()) {
    auto const t0 = cpu_micros();
    SCOPE_EXIT { unknown_us_ = cpu_micros() - t0; };
    for (const auto h : unknown_objects_) {
      if (mark(h, GCBits::Pin)) {
        enqueue(h);
      }
    }
    process_worklist();
  } else {
    unknown_us_ = 0;
  }
}

// check that headers have a "sensible" state during sweeping.
DEBUG_ONLY bool check_sweep_header(const Header* h) {
  switch (h->kind()) {
    case HeaderKind::Packed:
    case HeaderKind::Mixed:
    case HeaderKind::Dict:
    case HeaderKind::Empty:
    case HeaderKind::VecArray:
    case HeaderKind::Keyset:
    case HeaderKind::Apc:
    case HeaderKind::Globals:
    case HeaderKind::Proxy:
    case HeaderKind::String:
    case HeaderKind::Resource:
    case HeaderKind::Ref:
      // ordinary counted objects
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
      // objects; should not have native-data
      assert(!h->obj_.getAttribute(ObjectData::HasNativeData));
      break;
    case HeaderKind::AsyncFuncFrame:
    case HeaderKind::NativeData:
    case HeaderKind::ClosureHdr:
      // not counted but marked when embedded object is marked
      break;
    case HeaderKind::SmallMalloc:
    case HeaderKind::BigMalloc:
      // not counted but can be marked.
      break;
    case HeaderKind::Free:
      // free memory; these should not be marked.
      assert(!(h->hdr_.marks() & GCBits::Mark));
      break;
    case HeaderKind::AsyncFuncWH:
    case HeaderKind::Closure:
    case HeaderKind::BigObj:
    case HeaderKind::Hole:
      // These should never be encountered because they don't represent
      // independent allocations.
      assert(false && "invalid header kind");
      break;
  }
  return true;
}

// another pass through the heap, this time using the PtrMap we computed
// in init(). Free and maybe quarantine unmarked objects.
NEVER_INLINE void Marker::sweep() {
  auto const t0 = cpu_micros();
  SCOPE_EXIT { sweep_us_ = cpu_micros() - t0; };
  auto& mm = MM();
  const bool use_quarantine = RuntimeOption::EvalQuarantine;
  if (use_quarantine) mm.beginQuarantine();
  SCOPE_EXIT { if (use_quarantine) mm.endQuarantine(); };
  std::deque<std::pair<Header*,size_t>> defer;
  ptrs_.iterate([&](const Header* hdr, size_t h_size) {
    assert(check_sweep_header(hdr));
    if (hdr->hdr_.marks() & GCBits::Mark) {
      marked_ += h_size;
      if (hdr->hdr_.marks() == GCBits::Pin) pinned_ += h_size;
      return; // continue iterate loop
    }
    // when freeing objects below, do not run their destructors! we don't
    // want to execute cascading decrefs or anything. the normal release()
    // methods of refcounted classes aren't usable because they run dtors.
    auto h = const_cast<Header*>(hdr);
    switch (h->kind()) {
      case HeaderKind::Packed:
      case HeaderKind::Mixed:
      case HeaderKind::Dict:
      case HeaderKind::Empty:
      case HeaderKind::VecArray:
      case HeaderKind::Keyset:
      case HeaderKind::Globals:
      case HeaderKind::Proxy:
      case HeaderKind::Resource:
      case HeaderKind::Ref:
        freed_ += h_size;
        mm.objFree(h, h_size);
        break;
      case HeaderKind::Object:
      case HeaderKind::WaitHandle:
      case HeaderKind::AwaitAllWH:
      case HeaderKind::Vector:
      case HeaderKind::Map:
      case HeaderKind::Set:
      case HeaderKind::Pair:
      case HeaderKind::ImmVector:
      case HeaderKind::ImmMap:
      case HeaderKind::ImmSet:
      case HeaderKind::AsyncFuncFrame:
      case HeaderKind::ClosureHdr:
      case HeaderKind::NativeData: {
        auto obj = h->obj();
        if (obj->getAttribute(ObjectData::HasDynPropArr)) {
          defer.push_back({h, h_size});
        } else {
          freed_ += h_size;
          mm.objFree(h, h_size);
        }
        break;
      }
      case HeaderKind::Apc:
        defer.push_back({h, h_size});
        break;
      case HeaderKind::String:
        freed_ += h_size;
        h->str_.release(); // also maybe atomic-dec APCString
        break;
      case HeaderKind::SmallMalloc:
      case HeaderKind::BigMalloc:
        // Don't free malloc-ed allocations even if they're not reachable.
        // NativeData types might leak these
        break;
      case HeaderKind::Free:
        // should not be in ptrmap; fall through to assert
      case HeaderKind::Hole:
      case HeaderKind::BigObj:
      case HeaderKind::AsyncFuncWH:
      case HeaderKind::Closure:
        assert(false && "skipped by forEachHeader()");
        break;
    }
  });
  // deferred items explicitly free auxilary blocks, so it's unsafe to
  // sweep them while iterating over ptrs_.
  for (auto& e : defer) {
    auto h = e.first;
    auto h_size = e.second;
    freed_ += h_size;
    if (isObjectKind(h->kind()) || h->kind() == HeaderKind::NativeData ||
        h->kind() == HeaderKind::AsyncFuncFrame) {
      assert(h->obj()->getAttribute(ObjectData::HasDynPropArr));
      auto obj = h->obj();
      // dynPropTable is a req::hash_map, so this will req::free junk
      g_context->dynPropTable.erase(obj);
      mm.objFree(h, h_size);
    } else if (h->kind() == HeaderKind::Apc) {
      h->apc_.reap(); // also frees localCache and atomic-dec APCArray
    } else {
      always_assert(false && "what other kinds need deferral?");
    }
  }
}

template<size_t NBITS> struct BloomFilter {
  BloomFilter() : bits_{NBITS} {}
  using T = const void*;
  static size_t h1(size_t h) { return h % NBITS; }
  static size_t h2(size_t h) { return (h / NBITS) % NBITS; }
  void insert(T x) {
    auto h = hash_int64(intptr_t(x));
    bits_.set(h1(h)).set(h2(h));
  }
  bool test(T x) const {
    auto h = hash_int64(intptr_t(x));
    return bits_.test(h1(h)) & bits_.test(h2(h));
  }
  void clear() {
    bits_.reset();
    static_assert(NBITS < (1LL << 32), "");
  }
private:
  boost::dynamic_bitset<> bits_;
};

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
  sample.setInt("memory_limit", t_pre_stats.limit);
  sample.setInt("usage", t_pre_stats.usage());
  sample.setInt("mm_usage", t_pre_stats.mmUsage);
  sample.setInt("aux_usage", t_pre_stats.auxUsage);
  sample.setInt("mm_capacity", t_pre_stats.capacity);
  sample.setInt("peak_usage", t_pre_stats.peakUsage);
  sample.setInt("peak_capacity", t_pre_stats.peakCap);
  sample.setInt("total_alloc", t_pre_stats.totalAlloc);
  return sample;
}

void logCollection(const char* phase, const Marker& mkr) {
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
      mkr.init_us_/1000,
      mkr.mark_us_/1000,
      mkr.allocd_.bytes/1024/1024,
      100.0 * mkr.marked_.bytes / mkr.allocd_.bytes,
      100.0 * mkr.pinned_.bytes / mkr.allocd_.bytes,
      mkr.freed_.bytes/1024.0/1024.0,
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
  sample.setInt("init_micros", mkr.init_us_);
  sample.setInt("initfree_micros", mkr.initfree_us_);
  sample.setInt("roots_micros", mkr.roots_us_);
  sample.setInt("mark_micros", mkr.mark_us_); // includes unknown
  sample.setInt("unknown_micros", mkr.unknown_us_);
  sample.setInt("sweep_micros", mkr.sweep_us_);
  // size metrics gathered during gc
  sample.setInt("allocd_bytes", mkr.allocd_.bytes);
  sample.setInt("allocd_objects", mkr.allocd_.count);
  sample.setInt("allocd_span", mkr.ptrs_.span().second);
  sample.setInt("marked_bytes", mkr.marked_.bytes);
  sample.setInt("pinned_bytes", mkr.pinned_.bytes);
  sample.setInt("unknown_bytes", mkr.unknown_.bytes);
  sample.setInt("freed_bytes", mkr.freed_.bytes);
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

void collectImpl(const char* phase) {
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
  Marker mkr;
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
  auto mm_limit = m_stats.limit - m_stats.auxUsage;
  int64_t delta = (mm_limit - m_stats.mmUsage) *
                  RuntimeOption::EvalGCTriggerPct;
  delta = std::max(delta, RuntimeOption::EvalGCMinTrigger);
  m_nextGc = m_stats.mmUsage + delta;
}

void MemoryManager::collect(const char* phase) {
  if (empty()) return;
  t_req_age = cpu_micros() - m_req_start_micros;
  t_trigger = m_nextGc;
  collectImpl(phase);
  updateNextGc();
}

void MemoryManager::setMemoryLimit(size_t limit) {
  m_stats.limit = limit;
  updateNextGc();
}

}
