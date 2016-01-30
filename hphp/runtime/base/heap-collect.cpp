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
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/heap-scan.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/alloc.h"
#include "hphp/util/trace.h"

#include <algorithm>
#include <iterator>
#include <vector>
#include <folly/Range.h>
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
  void trace();
  void sweep();

  // scanners can tell us where the pointers are seated.
  void where(const char* area) {}

  // mark exact pointers
  void operator()(const StringData*);
  void operator()(const ArrayData*);
  void operator()(const ObjectData*);
  void operator()(const ResourceData*);
  void operator()(const ResourceHdr*);
  void operator()(const RefData*);
  void operator()(const TypedValue&);
  void operator()(const TypedValueAux& v) { (*this)(*(const TypedValue*)&v); }
  void operator()(const NameValueTable*);
  void operator()(const VarEnv*);
  void operator()(const RequestEventHandler*);

  // mark ambiguous pointers in the range [start,start+len)
  void operator()(const void* start, size_t len);

  // classes containing exact pointers
  void operator()(const String&);
  void operator()(const Array&);
  void operator()(const ArrayNoDtor&);
  void operator()(const Object&);
  void operator()(const Resource&);
  void operator()(const Variant&);
  void operator()(const StringBuffer&);
  void operator()(const NameValueTable&);
  void operator()(const AsioContext& p) { scanner().scan(p, *this); }
  void operator()(const VarEnv& venv) { (*this)(&venv); }

  // treat implicit pointers the same as real pointers
  void implicit(const ObjectData* p) { (*this)(p); }
  void implicit(const ResourceHdr* p) { (*this)(p); }
  void implicit(const Array& p) { (*this)(p); }

  template<class T> void operator()(const req::ptr<T>& p) {
    (*this)(p.get());
  }
  template<class T> void operator()(const req::vector<T>& c) {
    for (auto& e : c) (*this)(e);
  }
  template<class T> void operator()(const req::set<T>& c) {
    for (auto& e : c) (*this)(e);
  }
  template<class T> void implicit(const req::set<T>& c) {
    for (auto& e : c) implicit(e);
  }
  template<class T,class U> void operator()(const std::pair<T,U>& p) {
    (*this)(p.first);
    (*this)(p.second);
  }
  template<class T,class U,class V,class W>
  void operator()(const req::hash_map<T,U,V,W>& c) {
    for (auto& e : c) (*this)(e); // each element is pair<T,U>
  }

  template <typename T>
  void operator()(const LowPtr<T>& p) {
    (*this)(p.get());
  }

  void operator()(const ArrayIter& iter) {
    scan(iter, *this);
  }
  void operator()(const MArrayIter& iter) {
    scan(iter, *this);
  }

  // TODO: these need to be implemented.
  void operator()(const ActRec&) { }
  void operator()(const Stack&) { }

  void operator()(const RequestEventHandler& h) { (*this)(&h); }

  // TODO (6512343): this needs to be hooked into scan methods for Extensions.
  void operator()(const Extension&) { }

  // Explicitly ignored field types.
  void operator()(const LowPtr<Class>&) {}
  void operator()(const Func*) {}
  void operator()(const Class*) {}
  void operator()(const Unit*) {}
  void operator()(const std::string&) {}
  void operator()(int) {}

private:
  template<class T> static bool counted(T* p) {
    return p && p->isRefCounted();
  }
  bool mark(const void*, GCBits = GCBits::Mark);
  bool inRds(const void* vp) {
    auto p = reinterpret_cast<const char*>(vp);
    return p >= rds_.begin() && p < rds_.end();
  }
  template<class T> void enqueue(const T* p) {
    auto h = reinterpret_cast<const Header*>(p);
    assert(h &&
           h->kind() <= HeaderKind::BigMalloc &&
           h->kind() != HeaderKind::ResumableFrame &&
           h->kind() != HeaderKind::NativeData);
    work_.push_back(h);
  }

private:
  PtrMap ptrs_;
  std::vector<const Header*> work_;
  folly::Range<const char*> rds_; // full mmap'd rds section.
  Counter total_;        // bytes allocated in heap
};

// mark the object at p, return true if first time.
bool Marker::mark(const void* p, GCBits marks) {
  assert(p && ptrs_.isHeader(p));
  auto h = static_cast<const Header*>(p);
  assert(h->kind() <= HeaderKind::BigMalloc &&
         h->kind() != HeaderKind::ResumableObj);
  auto old_marks = h->hdr_.marks;
  h->hdr_.marks = old_marks | marks;
  return old_marks == GCBits::Unmarked;
}

// Utility to just extract the kind field from an arbitrary Header ptr.
inline DEBUG_ONLY HeaderKind kind(const void* p) {
  return static_cast<const Header*>(p)->kind();
}

void Marker::operator()(const ObjectData* p) {
  if (!p) return;
  assert(isObjectKind(p->headerKind()));
  auto kind = p->headerKind();
  if (kind == HeaderKind::ResumableObj) {
    // Resumable object, prefixed by a ResumableNode header, which is what
    // we need to mark.
    // [ResumableNode][locals][Resumable][ObjectData<ResumableObj>]
    auto r = Resumable::FromObj(p);
    auto frame = reinterpret_cast<const TypedValue*>(r) -
                 r->actRec()->func()->numSlotsInFrame();
    auto node = reinterpret_cast<const ResumableNode*>(frame) - 1;
    assert(node->hdr.kind == HeaderKind::ResumableFrame);
    if (mark(node)) {
      // mark the ResumableFrame prefix, but enqueue the ObjectData* to scan
      enqueue(p);
    }
    assert(!p->getVMClass()->getNativeDataInfo());
  } else if (p->getAttribute(ObjectData::HasNativeData)) {
    // HNI style native object; mark the NativeNode header, queue the object.
    // [NativeNode][NativeData][ObjectData][props] is one allocation.
    // For generators -
    // [NativeNode][locals][Resumable][GeneratorData][ObjectData]
    assert(p->getVMClass()->getNativeDataInfo() != nullptr);
    auto h = Native::getNativeNode(p, p->getVMClass()->getNativeDataInfo());
    assert(h->hdr.kind == HeaderKind::NativeData);
    if (mark(h)) {
      enqueue(p);
    }
  } else {
    // Ordinary non-builtin object subclass, or IDL-style native object.
    if (mark(p)) {
      enqueue(p);
    }
  }
}

void Marker::operator()(const ResourceHdr* p) {
  if (p && mark(p)) {
    assert(kind(p) == HeaderKind::Resource);
    enqueue(p);
  }
}

void Marker::operator()(const ResourceData* r) {
  if (r && mark(r->hdr())) {
    assert(kind(r->hdr()) == HeaderKind::Resource);
    enqueue(r->hdr());
  }
}

// ArrayData objects could be static
void Marker::operator()(const ArrayData* p) {
  if (p && counted(p) && mark(p)) {
    assert(isArrayKind(kind(p)));
    enqueue(p);
  }
}

// RefData objects contain at most one ptr, scan it eagerly.
void Marker::operator()(const RefData* p) {
  if (!p) return;
  if (inRds(p)) {
    // p is a static local, initialized by RefData::initInRDS().
    // we already scanned p's body as part of scanning RDS.
    return;
  }
  if (mark(p)) {
    assert(kind(p) == HeaderKind::Ref);
    enqueue(p);
  }
}

// The only thing interesting in a string is a possible APCString*,
// which is not a request-local allocation.
void Marker::operator()(const StringData* p) {
  if (p && counted(p)) {
    assert(kind(p) == HeaderKind::String);
    mark(p);
  }
}

// NVTs live inside VarEnv, and GlobalsArray has an interior ptr to one.
// ignore the interior pointer; NVT should be scanned by VarEnv::scan.
void Marker::operator()(const NameValueTable* p) {}

// VarEnvs are allocated with req::make, so they aren't first-class heap
// objects. assume a VarEnv* is a unique ptr, and scan it eagerly.
void Marker::operator()(const VarEnv* p) {
  if (p) p->scan(*this);
}

void Marker::operator()(const RequestEventHandler* p) {
  p->scan(*this);
}

void Marker::operator()(const String& p)    { (*this)(p.get()); }
void Marker::operator()(const Array& p)     { (*this)(p.get()); }
void Marker::operator()(const ArrayNoDtor& p) { (*this)(p.arr()); }
void Marker::operator()(const Object& p)    { (*this)(p.get()); }
void Marker::operator()(const Resource& p)  { (*this)(p.hdr()); }
void Marker::operator()(const Variant& p)   { (*this)(*p.asTypedValue()); }

void Marker::operator()(const StringBuffer& p) { p.scan(*this); }
void Marker::operator()(const NameValueTable& p) { p.scan(*this); }

// mark a TypedValue or TypedValueAux. taking tv by value would exclude aux.
void Marker::operator()(const TypedValue& tv) {
  switch (tv.m_type) {
    case KindOfString:    return (*this)(tv.m_data.pstr);
    case KindOfArray:     return (*this)(tv.m_data.parr);
    case KindOfObject:    return (*this)(tv.m_data.pobj);
    case KindOfResource:  return (*this)(tv.m_data.pres);
    case KindOfRef:       return (*this)(tv.m_data.pref);
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentString:
    case KindOfPersistentArray:
    case KindOfClass: // only in eval stack
      return;
  }
}

// mark ambigous pointers in the range [start,start+len). If the start or
// end is a partial word, don't scan that word.
void FOLLY_DISABLE_ADDRESS_SANITIZER
Marker::operator()(const void* start, size_t len) {
  constexpr uintptr_t M{7}; // word size - 1
  auto s = (char**)((uintptr_t(start) + M) & ~M); // round up
  auto e = (char**)((uintptr_t(start) + len) & ~M); // round down
  for (; s < e; s++) {
    auto p = *s;
    auto h = ptrs_.header(p);
    if (!h) continue;
    // mark p if it's an interesting kind. since we have metadata for it,
    // it must have a valid header.
    if (!mark(h, GCBits::CMark)) continue; // skip if already marked.
    switch (h->kind()) {
      case HeaderKind::Apc:
      case HeaderKind::Globals:
      case HeaderKind::Proxy:
      case HeaderKind::Ref:
      case HeaderKind::Resource:
      case HeaderKind::Packed:
      case HeaderKind::Struct:
      case HeaderKind::Mixed:
      case HeaderKind::Empty:
      case HeaderKind::SmallMalloc:
      case HeaderKind::BigMalloc:
        enqueue(h);
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
        enqueue(h);
        break;
      case HeaderKind::ResumableFrame:
        enqueue(h->resumableObj());
        break;
      case HeaderKind::NativeData:
        enqueue(h->nativeObj());
        break;
      case HeaderKind::String:
        // nothing to queue since strings don't have pointers
        break;
      case HeaderKind::ResumableObj:
      case HeaderKind::BigObj:
      case HeaderKind::Free:
      case HeaderKind::Hole:
        // None of these kinds should be encountered because they're either not
        // interesting to begin with, or are mapped to different headers, so we
        // shouldn't get these from the pointer map.
        always_assert(false && "bad header kind");
        break;
    }
  }
}

// initially parse the heap to find valid objects and initialize metadata.
void Marker::init() {
  rds_ = folly::Range<const char*>((char*)rds::header(),
                                   RuntimeOption::EvalJitTargetCacheSize);
  MM().forEachHeader([&](Header* h) {
    if (h->kind() == HeaderKind::Free) return;
    h->hdr_.marks = GCBits::Unmarked;
    total_ += h->size();
    switch (h->kind()) {
      case HeaderKind::Apc:
      case HeaderKind::Globals:
      case HeaderKind::Proxy:
      case HeaderKind::Packed:
      case HeaderKind::Mixed:
      case HeaderKind::Struct:
      case HeaderKind::Empty:
      case HeaderKind::String:
      case HeaderKind::Ref:
      case HeaderKind::Resource:
        ptrs_.insert(h);
        break;
      case HeaderKind::Object:
      case HeaderKind::Vector:
      case HeaderKind::Map:
      case HeaderKind::Set:
      case HeaderKind::Pair:
      case HeaderKind::ImmVector:
      case HeaderKind::ImmMap:
      case HeaderKind::ImmSet:
      case HeaderKind::AwaitAllWH:
      case HeaderKind::WaitHandle:
        assert(!h->obj_.getAttribute(ObjectData::HasNativeData) &&
               "object with NativeData from forEachHeader");
        ptrs_.insert(h);
        break;
      case HeaderKind::ResumableFrame: {
        // Pointers to either the frame or object will be mapped to the frame.
        ptrs_.insert(h);
        auto obj = reinterpret_cast<const Header*>(h->resumableObj());
        obj->hdr_.marks = GCBits::Unmarked;
        break;
      }
      case HeaderKind::NativeData: {
        // Pointers to either the native data or the object will be mapped to
        // the native data.
        ptrs_.insert(h);
        auto obj = reinterpret_cast<const Header*>(h->nativeObj());
        obj->hdr_.marks = GCBits::Unmarked;
        break;
      }
      case HeaderKind::SmallMalloc:
      case HeaderKind::BigMalloc:
        ptrs_.insert(h);
        break;
      case HeaderKind::ResumableObj:
        // ResumableObj should not be encountered on their own, they should
        // always be prefixed by a ResumableFrame allocation.
      case HeaderKind::Free:
      case HeaderKind::Hole:
      case HeaderKind::BigObj:
        // Hole and BigObj are skipped in ForEachHeader. Free is skipped above.
        assert(false && "skipped by forEachHeader()");
        break;
    }
  });
  ptrs_.prepare();
}

void Marker::trace() {
  scanRoots(*this);
  while (!work_.empty()) {
    auto h = work_.back();
    work_.pop_back();
    scanHeader(h, *this);
  }
}

// check that headers have a "sensible" state during sweeping.
DEBUG_ONLY bool check_sweep_header(const Header* h) {
  switch (h->kind()) {
    case HeaderKind::Packed:
    case HeaderKind::Struct:
    case HeaderKind::Mixed:
    case HeaderKind::Empty:
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
    case HeaderKind::ResumableFrame:
    case HeaderKind::NativeData:
      // not counted but marked when embedded object is marked
      break;
    case HeaderKind::SmallMalloc:
    case HeaderKind::BigMalloc:
      // not counted but can be marked.
      break;
    case HeaderKind::Free:
      // free memory; these should not be marked.
      assert(!(h->hdr_.marks & GCBits::Mark));
      break;
    case HeaderKind::ResumableObj:
    case HeaderKind::BigObj:
    case HeaderKind::Hole:
      // These should never be encountered because they don't represent
      // independent allocations.
      assert(false && "invalid header kind");
      break;
  }
  return true;
}

// another pass through the heap now that everything is marked.
void Marker::sweep() {
  Counter marked, ambig, freed;
  std::vector<Header*> reaped;
  auto& mm = MM();
  mm.iterate([&](Header* h) {
    assert(check_sweep_header(h));
    auto size = h->size(); // internal size
    if (h->hdr_.marks != GCBits::Unmarked) {
      if (h->hdr_.marks & GCBits::Mark) marked += size;
      else if (h->hdr_.marks & GCBits::CMark) ambig += size;
      return; // continue foreach loop
    }
    // when freeing objects below, do not run their destructors! we don't
    // want to execute cascading decrefs or anything. the normal release()
    // methods of refcounted classes aren't usable because they run dtors.
    // also, if freeing the current object causes other objects to be freed,
    // then must initialize the FreeNode header on them, in order to continue
    // parsing. For now, defer freeing those kinds of objects to after parsing.
    switch (h->kind()) {
      case HeaderKind::Packed:
      case HeaderKind::Struct:
      case HeaderKind::Mixed:
      case HeaderKind::Empty:
      case HeaderKind::Globals:
      case HeaderKind::Proxy:
      case HeaderKind::Resource:
      case HeaderKind::Ref:
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
      case HeaderKind::ResumableFrame:
      case HeaderKind::NativeData:
      case HeaderKind::Apc:
      case HeaderKind::String:
        freed += size;
        reaped.push_back(h);
        break;
      case HeaderKind::SmallMalloc:
      case HeaderKind::BigMalloc:
        // Don't free malloc-ed allocations even if they're not reachable.
        break;
      case HeaderKind::Free:
        break;
      case HeaderKind::Hole:
      case HeaderKind::BigObj:
      case HeaderKind::ResumableObj:
        assert(false && "skipped by forEachHeader()");
        break;
    }
  });
  if (freed.count > 1) {
    TRACE(1, "sweep tot %lu(%lu) mk %lu(%lu) amb %lu(%lu) free %lu(%lu)\n",
          total_.count, total_.bytes,
          marked.count, marked.bytes,
          ambig.count, ambig.bytes,
          freed.count, freed.bytes);
  }
  // once we're done iterating the heap, it's safe to free unreachable objects.
  const bool use_quarantine = debug && RuntimeOption::EvalEagerGC;
  if (use_quarantine) mm.beginQuarantine();
  SCOPE_EXIT { if (use_quarantine) mm.endQuarantine(); };
  for (auto h : reaped) {
    if (h->kind() == HeaderKind::Apc) {
      // frees localCache, delists, decref apc-array, free array
      h->apc_.reap();
    } else if (h->kind() == HeaderKind::String) {
      // decref apc shared str, free str
      h->str_.release(); // no destructor can run, so release() is safe.
    } else if (auto obj = h->obj()) {
      if (obj->getAttribute(ObjectData::HasDynPropArr)) {
        // dynPropTable is a req::hash_map, so this will req::free junk
        g_context->dynPropTable.erase(obj);
      }
      mm.objFree(h, h->size());
    } else {
      mm.objFree(h, h->size());
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

void collectImpl(const char* phase) {
  VMRegAnchor _;
  if (t_eager_gc && RuntimeOption::EvalFilterGCPoints) {
    t_eager_gc = false;
    auto pc = vmpc();
    if (t_surprise_filter.test(pc)) return;
    t_surprise_filter.insert(pc);
    TRACE(2, "eager gc %s at %p\n", phase, pc);
  } else {
    TRACE(2, "normal gc %s at %p\n", phase, vmpc());
  }
  Marker mkr;
  mkr.init();
  mkr.trace();
  mkr.sweep();
}

}

void MemoryManager::resetEagerGC() {
  if (RuntimeOption::EvalEagerGC && RuntimeOption::EvalFilterGCPoints) {
    t_surprise_filter.clear();
  }
}

void MemoryManager::checkEagerGC() {
  if (RuntimeOption::EvalEagerGC && rds::header()) {
    t_eager_gc = true;
    setSurpriseFlag(PendingGCFlag);
  }
}

void MemoryManager::collect(const char* phase) {
  if (!RuntimeOption::EvalEnableGC || empty()) return;
  collectImpl(phase);
}

}
