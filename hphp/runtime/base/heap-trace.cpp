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
#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/heap-scan.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/util/alloc.h"
#include "hphp/util/trace.h"

#include <vector>
#include <boost/dynamic_bitset.hpp>
#include <unordered_map>

namespace HPHP {
TRACE_SET_MOD(heaptrace);
using HK = HeaderKind;

namespace {

// metadata about an allocated object
struct Meta {
  bool mark;   // exactly marked
  bool cmark;  // conservatively marked
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

  template<class T> void operator()(const smart::vector<T>& c) {
    for (auto& e : c) (*this)(e);
  }
  template<class T> void operator()(const smart::set<T>& c) {
    for (auto& e : c) (*this)(e);
  }
  template<class T,class U> void operator()(const std::pair<T,U>& p) {
    (*this)(p.first);
    (*this)(p.second);
  }
  template<class T,class U,class V,class W>
  void operator()(const smart::hash_map<T,U,V,W>& c) {
    for (auto& e : c) (*this)(e); // each element is pair<T,U>
  }

  // Explicitly ignored field types.
  void operator()(const LowClassPtr&) {}
  void operator()(const Unit*) {}
  void operator()(int) {}

private:
  template<class T> static bool counted(T* p) {
    return p && p->getCount() >= 0;
  }
  bool mark(const void*);

private:
  // information about heap objects, indexed by valid object starts.
  std::unordered_map<const void*,Meta> meta_;

  std::vector<const ArrayData*> arrs_;
  std::vector<const ObjectData*> objs_;
  std::vector<const ResourceData*> ress_;

  size_t total_;        // bytes allocated in heap
  size_t marked_;       // bytes marked exactly
  size_t ambig_marked_; // bytes marked ambiguously
};

// mark the object at p, return true if first time.
bool Marker::mark(const void* p) {
  assert(meta_.find(p) != meta_.end());
  auto& meta = meta_[p];
  auto first = !meta.mark;
  meta.mark = true;
  return first;
}

void Marker::operator()(const ObjectData* p) {
  if (!p) return;
  if (p->getAttribute(ObjectData::HasNativeData)) {
    auto h = Native::getNativeNode(p, p->getVMClass()->getNativeDataInfo());
    if (mark(h)) objs_.push_back(p);
  } else if (reinterpret_cast<const Header*>(p)->kind_ == HK::ResumableObj) {
    auto r = Resumable::FromObj(p);
    auto frame = reinterpret_cast<const TypedValue*>(r) -
                 r->actRec()->func()->numSlotsInFrame();
    auto node = reinterpret_cast<const ResumableNode*>(frame) - 1;
    assert(node->kind == HK::Resumable);
    if (mark(node)) objs_.push_back(p);
    // p points to a resumable object. when we scan it, we also need to
    // scan the actrec, locals, and iterators attached to it, in case they
    // aren't reached while scanning the stack.
  } else {
    if (mark(p)) objs_.push_back(p);
  }
}

void Marker::operator()(const ResourceData* p) {
  if (p && mark(p)) ress_.push_back(p);
}

// ArrayData objects could be static
void Marker::operator()(const ArrayData* p) {
  if (counted(p) && mark(p)) arrs_.push_back(p);
}

// RefData objects contain at most one ptr, scan it eagerly.
void Marker::operator()(const RefData* p) {
  if (mark(p)) p->scan(*this);
}

// The only thing interesting in a string is a possible APCString*,
// which is not a request-local allocation.
void Marker::operator()(const StringData* p) {
  if (counted(p)) mark(p);
}

// NVTs live inside VarEnv, and GlobalsArray has an interior ptr to one.
// ignore the interior pointer; NVT should be scanned by VarEnv::scan.
void Marker::operator()(const NameValueTable* p) {}

// VarEnvs are allocated with smart_new, so they aren't first-class heap
// objects. assume a VarEnv* is a unique ptr, and scan it eagerly.
void Marker::operator()(const VarEnv* p) {
  if (p) p->scan(*this);
}

void Marker::operator()(const String& p)    { (*this)(p.get()); }
void Marker::operator()(const Array& p)     { (*this)(p.get()); }
void Marker::operator()(const ArrayNoDtor& p) { (*this)(p.arr()); }
void Marker::operator()(const Object& p)    { (*this)(p.get()); }
void Marker::operator()(const Resource& p)  { (*this)(p.get()); }
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
    if (meta_.find(p) == meta_.end()) continue;
    // mark p. but we don't know if it's pointing to a real object.
    // need to scan the heap to find real objects.
    // maybe truncate low bits of p, e.g. when it's this|class
    auto& meta = meta_[p];
    meta.cmark = true;
    // this should queue the object to be scanned, but how do we know its type?
  }
}

// initially parse the heap to find valid objects and initialize metadata.
// Certain objects can have count==0
// * StringData owned by StringBuffer
// * ArrayData owned by ArrayInit
// * Object ctors allocating memory in ctor (while count still==0).
void Marker::init() {
  total_ = 0;
  MM().forEachHeader([&](Header* h) {
    meta_[h] = Meta{false, false};
    switch (h->kind_) {
      case HK::Apc:
      case HK::Globals:
      case HK::Proxy:
      case HK::Resource:
      case HK::Ref:
        assert(h->count_ > 0);
        total_ += h->size();
        break;
      case HK::Packed:
      case HK::Struct:
      case HK::Mixed:
      case HK::Empty:
      case HK::String:
      case HK::Object:
      case HK::ResumableObj:
      case HK::AwaitAllWH:
        // count==0 can be witnessed, see above
        total_ += h->size();
        break;
      case HK::Resumable:
      case HK::Native:
      case HK::SmallMalloc:
      case HK::BigMalloc:
      case HK::BigObj: // hmm.. what lives inside this?
        total_ += h->size();
        break;
      case HK::Free:
      case HK::Hole:
      case HK::Debug:
        break;
    }
  });
}

template<class T> T* take(std::vector<T*>& v) {
  if (v.empty()) return nullptr;
  auto e = v.back();
  v.pop_back();
  return e;
}

void Marker::trace() {
  scanRoots(*this);
  do {
    while (auto arr = take(arrs_)) arr->scan(*this);
    while (auto obj = take(objs_)) obj->scan(*this);
    while (auto res = take(ress_)) res->scan(*this);
  } while (!arrs_.empty() || !objs_.empty());
}

// another pass through the heap now that everything is marked.
void Marker::sweep() {
  marked_ = ambig_marked_ = 0;
  MM().forEachHeader([&](Header* h) {
    UNUSED auto& meta = meta_[h];
    if (meta.cmark) ambig_marked_ += h->size();
    if (meta.mark) marked_ += h->size();
    switch (h->kind_) {
      case HK::Packed:
      case HK::Struct:
      case HK::Mixed:
      case HK::Empty:
      case HK::Apc:
      case HK::Globals:
      case HK::Proxy:
      case HK::String:
      case HK::Object:
      case HK::ResumableObj:
      case HK::AwaitAllWH:
      case HK::Resource:
      case HK::Ref:
        // ordinary counted objects
        break;
      case HK::Resumable:
      case HK::Native:
        // not counted but marked when attached object is marked
        break;
      case HK::BigObj:
        // these are headers that should wrap a markable or countable thing.
        assert(!meta.mark);
        break;
      case HK::SmallMalloc:
      case HK::BigMalloc:
        // these are managed by smart_malloc and should not have been marked.
        assert(!meta.mark);
        break;
      case HK::Free:
      case HK::Hole:
      case HK::Debug:
        // free memory; mark implies dangling pointer bug. cmark is ok because
        // dangling ambiguous pointers are not bugs, e.g. on the stack.
        assert(!meta.mark);
        break;
    }
  });
  TRACE(1, "sweep total %lu marked %lu cmarked %lu\n",
        total_, marked_, ambig_marked_);
}
}

void MemoryManager::traceHeap() {
  if (!RuntimeOption::EvalTraceHeap) return;
  if (empty()) return;
  Marker mkr;
  mkr.init();
  mkr.trace();
  mkr.sweep();
}

}
