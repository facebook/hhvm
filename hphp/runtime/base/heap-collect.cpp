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
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/heap-scan.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/util/alloc.h"
#include "hphp/util/trace.h"
#include "hphp/scan-methods/all-scan-decl.h"
#include "hphp/scan-methods/all-scan.h"

#include <vector>
#include <unordered_map>
#include <folly/Range.h>

namespace HPHP {
TRACE_SET_MOD(heaptrace);
using HK = HeaderKind;

namespace {

// information about heap objects, indexed by valid object starts.
struct PtrMap {
  void insert(const Header* p) {
    meta_[p] = Meta{};
  }
  const Header* header(const void* p) const {
    auto it = meta_.find(p);
    return it != meta_.end() ? static_cast<const Header*>(p) : nullptr;
  }
  bool contains(const void* p) const {
    return header(p) != nullptr;
  }
private:
  struct Meta {};
  std::unordered_map<const void*,Meta> meta_;
};

struct Marker {
  explicit Marker() {}
  void init();
  void trace();
  void sweep();

  // mark exact pointers
  void operator()(const StringData*);
  void operator()(const ArrayData*);
  void operator()(const ObjectData*);
  void operator()(const ResourceData*);
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

  template<class T> void operator()(const req::vector<T>& c) {
    for (auto& e : c) (*this)(e);
  }
  template<class T> void operator()(const req::set<T>& c) {
    for (auto& e : c) (*this)(e);
  }
  template<class T,class U> void operator()(const std::pair<T,U>& p) {
    (*this)(p.first);
    (*this)(p.second);
  }
  template<class T,class U,class V,class W>
  void operator()(const req::hash_map<T,U,V,W>& c) {
    for (auto& e : c) (*this)(e); // each element is pair<T,U>
  }

  // Explicitly ignored field types.
  void operator()(const LowPtr<Class>&) {}
  void operator()(const Unit*) {}
  void operator()(int) {}

private:
  template<class T> static bool counted(T* p) {
    return p && p->getCount() >= 0;
  }
  bool mark(const void*);
  bool inRds(const void* vp) {
    auto p = reinterpret_cast<const char*>(vp);
    return p >= rds_.begin() && p < rds_.end();
  }
  template<class T> void enqueue(T* p) {
    work_.push_back(reinterpret_cast<const Header*>(p));
  }

private:
  PtrMap ptrs_;
  std::vector<const Header*> work_;
  folly::Range<const char*> rds_; // full mmap'd rds section.
  size_t total_;        // bytes allocated in heap
  size_t marked_;       // bytes marked exactly
  size_t ambig_marked_; // bytes marked ambiguously
};

// mark the object at p, return true if first time.
bool Marker::mark(const void* p) {
  assert(ptrs_.contains(p));
  auto h = static_cast<const Header*>(p);
  auto first = !h->hdr_.mark;
  h->hdr_.mark = true;
  return first;
}

void Marker::operator()(const ObjectData* p) {
  if (!p) return;
  if (p->getAttribute(ObjectData::HasNativeData)) {
    // HNI style native object; mark the NativeNode header, queue the object.
    // [NativeNode][NativeData][ObjectData][props] is one allocation.
    // For generators -
    // [NativeNode][locals][Resumable][GeneratorData][ObjectData]
    auto h = Native::getNativeNode(p, p->getVMClass()->getNativeDataInfo());
    if (mark(h)) {
      enqueue(p);
    }
  } else if (reinterpret_cast<const Header*>(p)->kind() == HK::ResumableObj) {
    // Resumable object. we also need to scan the actrec, locals,
    // and iterators attached to it. It's wrapped by a ResumableNode header,
    // which is what we need to mark.
    // [ResumableNode][locals][Resumable][ObjectData<ResumableObj>]
    auto r = Resumable::FromObj(p);
    auto frame = reinterpret_cast<const TypedValue*>(r) -
                 r->actRec()->func()->numSlotsInFrame();
    auto node = reinterpret_cast<const ResumableNode*>(frame) - 1;
    assert(node->hdr.kind == HK::ResumableFrame);
    if (mark(node)) {
      enqueue(p);
    }
  } else {
    // Ordinary non-builtin object subclass, or IDL-style native object.
    if (mark(p)) {
      enqueue(p);
    }
  }
}

// Utility to just extract the kind field from an arbitrary Header ptr.
inline HeaderKind kind(const void* p) {
  return static_cast<const Header*>(p)->kind();
}

void Marker::operator()(const ResourceData* p) {
  if (p && mark(p)) {
    assert(kind(p) == HK::Resource);
    enqueue(p);
  }
}

// ArrayData objects could be static
void Marker::operator()(const ArrayData* p) {
  if (counted(p) && mark(p)) {
    enqueue(p);
  }
}

// RefData objects contain at most one ptr, scan it eagerly.
void Marker::operator()(const RefData* p) {
  if (inRds(p)) {
    // p is a static local, initialized by RefData::initInRDS().
    // we already scanned p's body as part of scanning RDS.
    return;
  }
  if (mark(p)) {
    enqueue(p);
  }
}

// The only thing interesting in a string is a possible APCString*,
// which is not a request-local allocation.
void Marker::operator()(const StringData* p) {
  if (counted(p)) mark(p);
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
void Marker::operator()(const Resource& p)  { (*this)(deref<ResourceData>(p)); }
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
    case KindOfStaticString:
    case KindOfClass: // only in eval stack
      return;
  }
}

// mark ambigous pointers in the range [start,start+len). If the start or
// end is a partial word, don't scan that word.
void Marker::operator()(const void* start, size_t len) {
  const uintptr_t M{7}; // word size - 1
  auto s = (char**)((uintptr_t(start) + M) & ~M); // round up
  auto e = (char**)((uintptr_t(start) + len) & ~M); // round down
  for (; s < e; s++) {
    auto p = *s;
    auto h = ptrs_.header(p);
    if (!h) continue;
    // mark p if it's an interesting kind. since we have metadata for it,
    // it must have a valid header.
    switch (h->kind()) {
      case HK::Apc:
      case HK::Globals:
      case HK::Proxy:
      case HK::Ref:
      case HK::Resource:
      case HK::Packed:
      case HK::Struct:
      case HK::Mixed:
      case HK::Empty:
      case HK::Object:
      case HK::ResumableObj:
      case HK::AwaitAllWH:
      case HK::Vector:
      case HK::Map:
      case HK::Set:
      case HK::Pair:
      case HK::ImmVector:
      case HK::ImmMap:
      case HK::ImmSet:
      case HK::ResumableFrame:
      case HK::BigObj: // hmm.. what lives inside this?
        h->hdr_.cmark = true;
        if (mark(p)) {
          enqueue(p);
        }
        break;
      case HK::NativeData:
        h->hdr_.cmark = true;
        if (mark(p)) {
          enqueue(Native::obj(&h->native_));
        }
        break;
      case HK::String:
        h->hdr_.cmark = true;
        mark(p);
        break;
      case HK::SmallMalloc:
      case HK::BigMalloc:
      case HK::Free:
      case HK::Hole:
        break;
    }
    // for ObjectData embedded after NativeNode, ResumableNode, BigObj,
    // do we want meta entries for them, directly? probably, then we can
    // deal with pointers to either the ObjectData or the wrapper.
  }
}

// initially parse the heap to find valid objects and initialize metadata.
// Certain objects can have count==0
// * StringData owned by StringBuffer
// * ArrayData owned by ArrayInit
// * Object ctors allocating memory in ctor (while count still==0).
void Marker::init() {
  rds_ = folly::Range<const char*>((char*)rds::header(),
                                   RuntimeOption::EvalJitTargetCacheSize);
  total_ = 0;
  MM().forEachHeader([&](Header* h) {
    ptrs_.insert(h);
    h->hdr_.mark = h->hdr_.cmark = false;
    switch (h->kind()) {
      case HK::Apc:
      case HK::Globals:
      case HK::Proxy:
        assert(h->hdr_.count > 0);
        total_ += h->size();
        break;
      case HK::Ref:
        // EZC non-ref refdatas sometimes have count==0
        assert(h->hdr_.count > 0 || !h->ref_.zIsRef());
        total_ += h->size();
        break;
      case HK::Resource:
        // ZendNormalResourceData objects sometimes never incref'd
        // TODO: t5969922, t6545412 might be a real bug.
        total_ += h->size();
        break;
      case HK::Packed:
      case HK::Struct:
      case HK::Mixed:
      case HK::Empty:
      case HK::String:
      case HK::Object:
      case HK::Vector:
      case HK::Map:
      case HK::Set:
      case HK::Pair:
      case HK::ImmVector:
      case HK::ImmMap:
      case HK::ImmSet:
      case HK::ResumableObj:
      case HK::AwaitAllWH:
        // count==0 can be witnessed, see above
        total_ += h->size();
        break;
      case HK::ResumableFrame:
      case HK::NativeData:
      case HK::SmallMalloc:
      case HK::BigMalloc:
      case HK::BigObj: // hmm.. what lives inside this?
        total_ += h->size();
        break;
      case HK::Free:
      case HK::Hole:
        break;
    }
  });
}

void Marker::trace() {
  scanRoots(*this);
  while (!work_.empty()) {
    auto h = work_.back();
    work_.pop_back();
    scanHeader(h, *this);
  }
}

// another pass through the heap now that everything is marked.
void Marker::sweep() {
  marked_ = ambig_marked_ = 0;
  MM().iterate([&](Header* h) {
    if (h->hdr_.cmark) ambig_marked_ += h->size();
    if (h->hdr_.mark) marked_ += h->size();
    switch (h->kind()) {
      case HK::Packed:
      case HK::Struct:
      case HK::Mixed:
      case HK::Empty:
      case HK::Apc:
      case HK::Globals:
      case HK::Proxy:
      case HK::String:
      case HK::Object:
      case HK::Vector:
      case HK::Map:
      case HK::Set:
      case HK::Pair:
      case HK::ImmVector:
      case HK::ImmMap:
      case HK::ImmSet:
      case HK::ResumableObj:
      case HK::AwaitAllWH:
      case HK::Resource:
      case HK::Ref:
        // ordinary counted objects
        break;
      case HK::ResumableFrame:
      case HK::NativeData:
        // not counted but marked when attached object is marked
        break;
      case HK::BigObj:
        // these are headers that should wrap a markable or countable thing.
        assert(!h->hdr_.mark);
        break;
      case HK::SmallMalloc:
      case HK::BigMalloc:
        // these are managed by req::malloc and should not have been marked.
        assert(!h->hdr_.mark);
        break;
      case HK::Free:
      case HK::Hole:
        // free memory; mark implies dangling pointer bug. cmark is ok because
        // dangling ambiguous pointers are not bugs, e.g. on the stack.
        assert(!h->hdr_.mark);
        break;
    }
  });
  TRACE(1, "sweep total %lu marked %lu ambig-marked %lu\n",
        total_, marked_, ambig_marked_);
}
}

void MemoryManager::collect() {
  if (!RuntimeOption::EvalEnableGC || empty()) return;
  Marker mkr;
  mkr.init();
  mkr.trace();
  mkr.sweep();
}

}
