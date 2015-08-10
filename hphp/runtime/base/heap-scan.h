/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef HPHP_HEAP_SCAN_H
#define HPHP_HEAP_SCAN_H

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/apc-local-array-defs.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/imarker.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/base/request-event-handler.h"

namespace HPHP {

template<class F> void scanHeader(const Header* h, F& mark) {
  switch (h->kind()) {
    case HeaderKind::Proxy:
      return h->proxy_.scan(mark);
    case HeaderKind::Empty:
      return;
    case HeaderKind::Packed:
      return PackedArray::scan(&h->arr_, mark);
    case HeaderKind::Struct:
      return h->struct_.scan(mark);
    case HeaderKind::Mixed:
      return h->mixed_.scan(mark);
    case HeaderKind::Apc:
      return h->apc_.scan(mark);
    case HeaderKind::Globals:
      return h->globals_.scan(mark);
    case HeaderKind::Object:
    case HeaderKind::ResumableObj:
    case HeaderKind::AwaitAllWH:
    case HeaderKind::Vector:
    case HeaderKind::Map:
    case HeaderKind::Set:
    case HeaderKind::Pair:
    case HeaderKind::ImmVector:
    case HeaderKind::ImmMap:
    case HeaderKind::ImmSet:
      return h->obj_.scan(mark);
    case HeaderKind::Resource:
      return h->res_.data()->scan(mark);
    case HeaderKind::Ref:
      return h->ref_.scan(mark);
    case HeaderKind::String:
    case HeaderKind::SmallMalloc:
    case HeaderKind::BigMalloc:
    case HeaderKind::BigObj:
    case HeaderKind::Free:
    case HeaderKind::ResumableFrame:
    case HeaderKind::NativeData:
    case HeaderKind::Hole:
      always_assert(false && "unexpected header in worklist");
      break;
  }
  always_assert(false && "corrupt header in worklist");
}

template<class F> void ObjectData::scan(F& mark) const {
  auto props = propVec();
  if (getAttribute(HasNativeData)) {
    // [NativeNode][NativeData][ObjectData][props]
    // TODO t6169196 indirect NativeDataInfo call for exact marking
    auto ndi = m_cls->getNativeDataInfo();
    auto size = alignTypedValue(ndi->sz);
    mark((char*)this - size, ndi->sz);
  } else if (getAttribute(IsCppBuiltin)) {
    // [ObjectData][C++ fields][props]
    // TODO t6169228 virtual call for exact marking
    mark(this + 1, uintptr_t(props) - uintptr_t(this + 1));
  }
  mark(m_cls);
  for (size_t i = 0, n = m_cls->numDeclProperties(); i < n; ++i) {
    mark(props[i]);
  }
  // nb: dynamic property arrays are pointed to by ExecutionContext,
  // which is marked as a root.
}

// bridge between the templated-based marker interface and the
// virtual-call based marker interface.
template<class F> struct ExtMarker final: IMarker {
  explicit ExtMarker(F& mark) : mark_(mark) {}
  void operator()(const Array& p) override { mark_(p); }
  void operator()(const Object& p) override { mark_(p); }
  void operator()(const Resource& p) override { mark_(p); }
  void operator()(const String& p) override { mark_(p); }
  void operator()(const Variant& p) override { mark_(p); }
  void operator()(const ArrayIter& p) override { mark_(p); }
  void operator()(const MArrayIter& p) override { mark_(p); }
  void operator()(const StringBuffer& p) override { mark_(p); }
  void operator()(const ActRec& p) override { mark_(p); }
  void operator()(const Stack& p) override { mark_(p); }
  void operator()(const VarEnv& p) override { mark_(p); }
  void operator()(const RequestEventHandler& p) override { mark_(p); }
  void operator()(const AsioContext& p) override { mark_(p); }

  void operator()(const StringData* p) override { mark_(p); }
  void operator()(const ArrayData* p) override { mark_(p); }
  void operator()(const ObjectData* p) override { mark_(p); }
  void operator()(const ResourceData* p) override { mark_(p); }
  void operator()(const RefData* p) override { mark_(p); }
  void operator()(const Func* p) override { mark_(p); }
  void operator()(const Class* p) override { mark_(p); }
  void operator()(const TypedValue* p) override { mark_(*p); }
  void operator()(const NameValueTable* p) override { mark_(*p); }
  void operator()(const Unit* p) override { mark_(p); }

  void operator()(const void* start, size_t len) override { mark_(start, len); }
private:
  F& mark_;
};

template<class F> void ResourceData::scan(F& mark) const {
  ExtMarker<F> bridge(mark);
  vscan(bridge);
}

template<class F> void RequestEventHandler::scan(F& mark) const {
  ExtMarker<F> bridge(mark);
  vscan(bridge);
}

//   [<-stack[iters[locals[params[ActRec[stack[iters[locals[ActRec...
//   ^m_top                      ^fp                       ^firstAR
//
//               +-----------------------+
//   top ->      | current eval stack    |
//               +-----------------------+
//               | iterators             |
//               +-----------------------+
//               | locals, params        |
//               +-----------------------+
//   fp ->       | current ActRec        |
//               +-----------------------+
//               | caller's eval stack   |
//               +-----------------------+
//                 ...
//               +-----------------------+
//   firstAR ->  | ActRec                |
//               +-----------------------+
//
// fp{sfp}... forms the true execution stack, but it chains
// in and out of resumables. sp always points into the vm stack section.
// locals/iterators/actrec are missing in some stack frames.
//
// ActRec:
//     m_sfp                prev ActRec*
//     m_savedRIP           return addr
//     m_func               Func*
//     m_soff               return vmpc
//     m_argc_flags
//     m_this|m_cls         ObjectData* or Class*
//     m_varenv|extraArgs
//
// for reference, see vm/unwind.cpp and visitStackElms() in bytecode.h

// scan the whole rds, including the php stack and the rds segment.
//  * stack: should be exact-scannable.
//  * rds threadlocal part: conservative scan until we teach the
//    jit to tell us where the pointers live.
//  * rds shared part: should not contain heap pointers!
template<class F> void scanRds(F& mark, rds::Header* rds) {
  // rds sections
  auto markSection = [&](folly::Range<const char*> r) {
    mark(r.begin(), r.size());
  };
  markSection(rds::normalSection());
  markSection(rds::localSection());
  markSection(rds::persistentSection());
  // php stack TODO #6509338 exactly scan the php stack.
  auto stack_end = (TypedValue*)rds->vmRegs.stack.getStackHighAddress();
  auto sp = rds->vmRegs.stack.top();
  mark(sp, (stack_end - sp) * sizeof(*sp));
}

// Scan request-local roots
template<class F> void scanRoots(F& mark) {
  // ExecutionContext
  if (!g_context.isNull()) g_context->scan(mark);
  // rds, including php stack
  if (auto rds = rds::header()) scanRds(mark, rds);
  // C++ stack
  auto sp = stack_top_ptr();
  mark(sp, s_stackLimit + s_stackSize - uintptr_t(sp));
  // Extension thread locals
  ExtMarker<F> xm(mark);
  ExtensionRegistry::scanExtensions(xm);
  // Root maps
  MM().scanRootMaps(mark);
}

template <typename T, typename F>
void scan(const req::ptr<T>& ptr, F& mark) {
  ptr->scan(mark);
}

}
#endif
