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
#include "hphp/runtime/ext/extension-registry.h"

namespace HPHP {

// defined here because we need definitions of all ArrayData subclasses.
template<class F> void ArrayData::scan(F& mark) const {
  switch (m_kind) {
    case ArrayData::kProxyKind:
      return static_cast<const ProxyArray*>(this)->scan(mark);
    case ArrayData::kEmptyKind:
      return;
    case ArrayData::kPackedKind:
      for (unsigned i = 0, n = getSize(); i < n; ++i) {
        mark(packedData(this)[i]);
      }
      return;
    case ArrayData::kStructKind:
      return StructArray::asStructArray(this)->scan(mark);
    case ArrayData::kMixedKind:
      return MixedArray::asMixed(this)->scan(mark);
    case ArrayData::kApcKind:
      return APCLocalArray::asApcArray(this)->scan(mark);
    case ArrayData::kGlobalsKind:
      return static_cast<const GlobalsArray*>(this)->scan(mark);
    case ArrayData::kNumKinds:
      always_assert(false);
      return;
  };
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
  void operator()(const String& p) override { mark_(p); }
  void operator()(const Variant& p) override { mark_(p); }
  void operator()(const void* start, size_t len) override {
    mark_(start, len);
  }
private:
  F& mark_;
};

template<class F> void ResourceData::scan(F& mark) const {
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
  ExtensionRegistry::scanExtensionRoots(xm);
}

}
#endif
