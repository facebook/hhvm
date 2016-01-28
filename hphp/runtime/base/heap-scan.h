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
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/imarker.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/server/server-note.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event-queue.h"
#include "hphp/runtime/ext/asio/ext_reschedule-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"

#ifdef ENABLE_ZEND_COMPAT
#include "hphp/runtime/ext_zend_compat/php-src/TSRM/TSRM.h"
#endif

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
    case HeaderKind::WaitHandle:
    case HeaderKind::ResumableObj:
    case HeaderKind::AwaitAllWH:
    case HeaderKind::Vector:
    case HeaderKind::Map:
    case HeaderKind::Set:
    case HeaderKind::ImmVector:
    case HeaderKind::ImmMap:
    case HeaderKind::ImmSet:
      return h->obj_.scan(mark);
    case HeaderKind::Pair:
      return h->pair_.scan(mark);
    case HeaderKind::Resource:
      return h->res_.data()->scan(mark);
    case HeaderKind::Ref:
      return h->ref_.scan(mark);
    case HeaderKind::SmallMalloc:
      return mark((&h->small_)+1, h->small_.padbytes - sizeof(SmallNode));
    case HeaderKind::BigMalloc:
      return mark((&h->big_)+1, h->big_.nbytes - sizeof(BigNode));
    case HeaderKind::NativeData:
      return h->nativeObj()->scan(mark);
    case HeaderKind::ResumableFrame:
      return h->resumableObj()->scan(mark);
    case HeaderKind::String:
    case HeaderKind::Free:
      // these don't have pointers. some clients might generically
      // scan them even if they aren't interesting.
      return;
    case HeaderKind::BigObj:
    case HeaderKind::Hole:
      // these aren't legitimate headers, and heap iteration should skip them.
      break;
  }
  always_assert(false && "corrupt header in worklist");
}

template<class F> void ObjectData::scan(F& mark) const {
  if (m_hdr.kind == HeaderKind::ResumableObj) {
    // scan the frame locals, iterators, and Resumable
    auto r = Resumable::FromObj(this);
    auto frame = reinterpret_cast<const TypedValue*>(r) -
                 r->actRec()->func()->numSlotsInFrame();
    mark(frame, uintptr_t(this) - uintptr_t(frame));
    auto node = reinterpret_cast<const ResumableNode*>(frame) - 1;
    mark(this + 1, uintptr_t(node) + r->size() - uintptr_t(this + 1));
  } else if (m_hdr.kind == HeaderKind::WaitHandle) {
    // scan C++ properties after [ObjectData] header
    mark(this + 1, asio_object_size(this) - sizeof(*this));
  } else if (m_hdr.kind == HeaderKind::AwaitAllWH) {
    auto wh = static_cast<const c_AwaitAllWaitHandle*>(this);
    wh->scanChildren(mark);
  }

  if (getAttribute(HasNativeData)) {
    // [NativeNode][NativeData][ObjectData][props]
    Native::nativeDataScan(this, mark);
  }

  auto props = propVec();
  if (getAttribute(IsCppBuiltin)) {
    // [ObjectData][C++ fields][props]
    // TODO t6169228 virtual call for exact marking
    mark(this + 1, uintptr_t(props) - uintptr_t(this + 1));
  }
  for (size_t i = 0, n = m_cls->numDeclProperties(); i < n; ++i) {
    mark(props[i]);
  }
  if (getAttribute(HasDynPropArr)) {
    // nb: dynamic property arrays are in ExecutionContext::dynPropTable,
    // which is not marked as a root. Mark the array when scanning the object.
    mark.implicit(g_context->dynPropTable[this].arr());
  }
}

template<class F> void ResourceData::scan(F& mark) const {
  ExtMarker<F> bridge(mark);
  vscan(bridge);
}

template<class F> void RequestEventHandler::scan(F& mark) const {
  ExtMarker<F> bridge(mark);
  vscan(bridge);
}

template<class F> void scan_ezc_resources(F& mark) {
#ifdef ENABLE_ZEND_COMPAT
  ExtMarker<F> bridge(mark);
  ts_scan_resources(bridge);
#endif
}

template<class F> void req::root_handle::scan(F& mark) const {
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
  mark.where("rds-normal");
  markSection(rds::normalSection());
  mark.where("rds-local");
  markSection(rds::localSection());
  mark.where("rds-persistent");
  markSection(rds::persistentSection());
  // php stack TODO #6509338 exactly scan the php stack.
  mark.where("php-stack");
  auto stack_end = (TypedValue*)rds->vmRegs.stack.getStackHighAddress();
  auto sp = rds->vmRegs.stack.top();
  mark(sp, (stack_end - sp) * sizeof(*sp));
}

template<class F>
void MemoryManager::scanSweepLists(F& mark) const {
  for (auto s = m_sweepables.next(); s != &m_sweepables; s = s->next()) {
    if (auto h = static_cast<Header*>(s->owner())) {
      assert(h->kind() == HeaderKind::Resource || isObjectKind(h->kind()));
      if (isObjectKind(h->kind())) {
        mark.implicit(&h->obj_);
      } else {
        mark.implicit(&h->res_);
      }
    }
  }
  for (auto node: m_natives) {
    mark.implicit(Native::obj(node));
  }
}

template <typename F>
void MemoryManager::scanRootMaps(F& m) const {
  if (m_objectRoots) {
    for(const auto& root : *m_objectRoots) {
      scan(root.second, m);
    }
  }
  if (m_resourceRoots) {
    for(const auto& root : *m_resourceRoots) {
      scan(root.second, m);
    }
  }
  for (const auto root : m_root_handles) {
    root->scan(m);
  }
}

template<class F>
void ThreadLocalManager::scan(F& mark) const {
  auto list = getList(pthread_getspecific(m_key));
  if (!list) return;
  for (auto p = list->head; p != nullptr;) {
    auto node = static_cast<ThreadLocalNode<void>*>(p);
    if (node->m_p) mark(node->m_p, node->m_size);
    p = node->m_next;
  }
}

// Scan request-local roots
template<class F> void scanRoots(F& mark) {
  // rds, including php stack
  if (auto rds = rds::header()) {
    scanRds(mark, rds);
  }
  // ExecutionContext
  if (!g_context.isNull()) {
    mark.where("g_context");
    g_context->scan(mark);
  }
  // ThreadInfo
  mark.where("ThreadInfo");
  if (!ThreadInfo::s_threadInfo.isNull()) {
    TI().scan(mark);
  }
  // C++ stack
  mark.where("cpp-stack");
  CALLEE_SAVED_BARRIER(); // ensure stack contains callee-saved registers
  auto sp = stack_top_ptr();
  mark(sp, s_stackLimit + s_stackSize - uintptr_t(sp));
  // C++ threadlocal data, but don't scan MemoryManager
  mark.where("cpp-tdata");
  auto tdata = getCppTdata(); // tdata = { ptr, size }
  if (tdata.second > 0) {
    auto tm = (char*)tdata.first;
    auto tm_end = tm + tdata.second;
    auto mm = (char*)&MM();
    auto mm_end = mm + sizeof(MemoryManager);
    assert(mm >= tm && mm_end <= tm_end);
    mark(tm, mm - tm);
    mark(mm_end, tm_end - mm_end);
  }
  // ThreadLocal nodes
  mark.where("ThreadLocalManager");
  ThreadLocalManager::GetManager().scan(mark);
  // Extension thread locals
  mark.where("extensions");
  ExtMarker<F> xm(mark);
  ExtensionRegistry::scanExtensions(xm);
  // Root maps
  mark.where("rootmaps");
  MM().scanRootMaps(mark);
  // treat sweep lists as roots until we are ready to test what happens
  // when we start calling various sweep() functions early.
  mark.where("sweeplists");
  MM().scanSweepLists(mark);
  // these have rogue thread_local stuff
  if (auto asio = AsioSession::Get()) {
    mark.where("AsioSession");
    asio->scan(mark);
  }
  mark.where("get_server_note");
  get_server_note()->scan(mark);
  mark.where("ezc resources");
  scan_ezc_resources(mark);
}

template <typename T, typename F>
void scan(const req::ptr<T>& ptr, F& mark) {
  ptr->scan(mark);
}

}
#endif
