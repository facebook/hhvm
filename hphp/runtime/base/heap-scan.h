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
#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/server/server-note.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event-queue.h"
#include "hphp/runtime/ext/asio/ext_reschedule-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/generator/ext_generator.h"

#include "hphp/util/hphp-config.h"

#ifdef ENABLE_ZEND_COMPAT
#include "hphp/runtime/ext_zend_compat/php-src/TSRM/TSRM.h"
#endif

#include "hphp/util/type-scan.h"

namespace HPHP {

template<class F> void scanFrameSlots(const ActRec* ar, F& mark) {
  auto num_slots = ar->func()->numSlotsInFrame();
  auto slots = reinterpret_cast<const TypedValue*>(ar) - num_slots;
  mark(slots, num_slots * sizeof(TypedValue));
}

template<class F>
void scanNative(const NativeNode* node, F& mark, type_scan::Scanner& scanner) {
  auto obj = Native::obj(node);
  auto ndi = obj->getVMClass()->getNativeDataInfo();
  auto data = (const char*)obj - ndi->sz;
  scanner.scanByIndex(node->typeIndex(), data, ndi->sz);
  if (auto off = node->arOff()) {
    scanFrameSlots((const ActRec*)((const char*)node + off), mark);
  }
}

template<class F>
void scanResumable(const Resumable* r, F& mark, type_scan::Scanner& scanner) {
  scanFrameSlots(r->actRec(), mark);
  scanner.scan(*r);
}

template<class F> void scanHeader(const Header* h, F& mark,
                                  type_scan::Scanner& scanner) {
  switch (h->kind()) {
    case HeaderKind::Proxy:
      return h->proxy_.scan(mark);
    case HeaderKind::Empty:
      return;
    case HeaderKind::Packed:
    case HeaderKind::VecArray:
      return PackedArray::scan(&h->arr_, mark);
    case HeaderKind::Mixed:
    case HeaderKind::Dict:
      return h->mixed_.scan(mark);
    case HeaderKind::Keyset:
      return h->set_.scan(mark);
    case HeaderKind::Apc:
      return h->apc_.scan(mark);
    case HeaderKind::Globals:
      return h->globals_.scan(mark);
    case HeaderKind::Closure:
      scanner.scan(*h->closure_.hdr());
      return h->closure_.scan(mark); // ObjectData::scan
    case HeaderKind::Object:
    case HeaderKind::WaitHandle:
    case HeaderKind::AwaitAllWH:
      if (h->obj_.getAttribute(ObjectData::HasNativeData)) {
        auto ndi = h->obj_.getVMClass()->getNativeDataInfo();
        scanNative(Native::getNativeNode(&h->obj_, ndi), mark, scanner);
      }
      return h->obj_.scan(mark);
    case HeaderKind::AsyncFuncWH:
      scanResumable(Resumable::FromObj(&h->obj_), mark, scanner);
      return h->obj_.scan(mark);
    case HeaderKind::NativeData:
      scanNative(&h->native_, mark, scanner);
      return Native::obj(&h->native_)->scan(mark);
    case HeaderKind::AsyncFuncFrame:
      scanResumable(Resumable::FromObj(h->asyncFuncWH()), mark, scanner);
      return h->asyncFuncWH()->scan(mark);
    case HeaderKind::ClosureHdr:
      scanner.scan(h->closure_hdr_);
      return h->closureObj()->scan(mark);
    case HeaderKind::Pair:
      return h->pair_.scan(mark);
    case HeaderKind::Vector:
    case HeaderKind::ImmVector:
      return h->vector_.scan(mark);
    case HeaderKind::Map:
    case HeaderKind::ImmMap:
    case HeaderKind::Set:
    case HeaderKind::ImmSet:
      return h->hashcoll_.scan(mark);
    case HeaderKind::Resource:
      return scanner.scanByIndex(
        h->res_.typeIndex(),
        h->res_.data(),
        h->res_.heapSize() - sizeof(ResourceHdr)
      );
    case HeaderKind::Ref:
      return h->ref_.scan(mark);
    case HeaderKind::SmallMalloc:
    case HeaderKind::BigMalloc:
      return scanner.scanByIndex(
        h->malloc_.typeIndex(),
        (&h->malloc_)+1,
        h->malloc_.nbytes - sizeof(MallocNode)
      );
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
  if (m_hdr.kind == HeaderKind::WaitHandle ||
      m_hdr.kind == HeaderKind::AwaitAllWH) {
    // scan C++ properties after [ObjectData] header. should pick up
    // unioned and bit-packed fields
    mark(this + 1, asio_object_size(this) - sizeof(*this));
  } else if (m_hdr.kind == HeaderKind::AsyncFuncWH) {
    // scan C++ properties after [ObjectData] header. should pick up
    // unioned and bit-packed fields
    mark(this + 1, sizeof(c_AsyncFunctionWaitHandle) - sizeof(*this));
  }

  auto props = propVec();
  if (m_hdr.partially_inited) {
    // we don't know which properties are initialized yet
    mark(props, m_cls->numDeclProperties() * sizeof(TypedValue));
  } else {
    for (size_t i = 0, n = m_cls->numDeclProperties(); i < n; ++i) {
      mark(props[i]);
    }
  }
  if (getAttribute(HasDynPropArr)) {
    // nb: dynamic property arrays are in ExecutionContext::dynPropTable,
    // which is not marked as a root. Mark the array when scanning the object.
    mark.implicit(g_context->dynPropTable[this].arr());
  }
}

template<class F> void scan_ezc_resources(F& mark) {
#ifdef ENABLE_ZEND_COMPAT
  ExtMarker<F> bridge(mark);
  ts_scan_resources(bridge);
#endif
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
template<class F> void scanRds(F& mark, rds::Header* rds,
                               type_scan::Scanner& scanner) {
  // rds sections

  auto markSection = [&](folly::Range<const char*> r) {
    mark(r.begin(), r.size());
  };

  scanner.scan(*rds::header());

  rds::forEachNormalAlloc(
    [&](const void* p, std::size_t size, type_scan::Index index) {
      scanner.scanByIndex(index, p, size);
    }
  );

  // Class stores pointers to request allocated memory in the local
  // section. Depending on circumstances, this may or may not be valid data,
  // so scan it conservatively for now. TODO #12203436
  // Persistent shouldn't contain pointers to any request allocated memory.
  mark.where(RootKind::RdsLocal);
  markSection(rds::localSection());

  // php stack TODO #6509338 exactly scan the php stack.
  mark.where(RootKind::PhpStack);
  auto stack_end = rds->vmRegs.stack.getStackHighAddress();
  auto sp = rds->vmRegs.stack.top();
  mark(sp, stack_end);
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
void MemoryManager::scanRootMaps(F& mark, type_scan::Scanner& scanner) const {
  if (m_objectRoots) {
    for(const auto& root : *m_objectRoots) {
      mark(root.second);
    }
  }
  if (m_resourceRoots) {
    for(const auto& root : *m_resourceRoots) {
      mark(root.second);
    }
  }
  for (const auto root : m_root_handles) {
    root->scan(scanner);
  }
}

inline void ThreadLocalManager::scan(type_scan::Scanner& scanner) const {
  auto list = getList(pthread_getspecific(m_key));
  if (!list) return;
  // Skip MemoryManager. TODO(9923909): Type-specific scan, cf. NativeData.
  auto mm = (void*)&MM();
  for (auto p = list->head; p != nullptr;) {
    auto node = static_cast<ThreadLocalNode<void>*>(p);
    if (node->m_p && node->m_p != mm) {
      scanner.scanByIndex(node->m_tyindex, node->m_p, node->m_size);
    }
    p = node->m_next;
  }
}

// Scan request-local roots
template<class F> void scanRoots(F& mark, type_scan::Scanner& scanner) {
  // rds, including php stack
  if (auto rds = rds::header()) {
    scanRds(mark, rds, scanner);
  }
  // ExecutionContext
  if (!g_context.isNull()) {
    mark.where(RootKind::ExecutionContext);
    g_context->scan(mark);
  }
  // ThreadInfo
  mark.where(RootKind::ThreadInfo);
  if (!ThreadInfo::s_threadInfo.isNull()) {
    TI().scan(mark);
  }
  // C++ stack
  mark.where(RootKind::CppStack);
  CALLEE_SAVED_BARRIER(); // ensure stack contains callee-saved registers
  auto sp = stack_top_ptr();
  mark(sp, s_stackLimit + s_stackSize - uintptr_t(sp));
  // C++ threadlocal data, but don't scan MemoryManager
  mark.where(RootKind::CppTls);
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
  // ThreadLocal nodes (but skip MemoryManager)
  mark.where(RootKind::ThreadLocalManager);
  ThreadLocalManager::GetManager().scan(scanner);
  // Extension thread locals
  mark.where(RootKind::Extensions);
  ExtMarker<F> xm(mark);
  ExtensionRegistry::scanExtensions(xm);
  // Root maps
  mark.where(RootKind::RootMaps);
  MM().scanRootMaps(mark, scanner);
  // treat sweep lists as roots until we are ready to test what happens
  // when we start calling various sweep() functions early.
  mark.where(RootKind::SweepLists);
  MM().scanSweepLists(mark);
  // these have rogue thread_local stuff
  if (auto asio = AsioSession::Get()) {
    mark.where(RootKind::AsioSession);
    asio->scan(mark);
  }
  mark.where(RootKind::GetServerNote);
  get_server_note()->scan(mark);
  mark.where(RootKind::EzcResources);
  scan_ezc_resources(mark);
}

template <typename T, typename F>
void scan(const req::ptr<T>& ptr, F& mark) {
  ptr->scan(mark);
}

}
#endif
