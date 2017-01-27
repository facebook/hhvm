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

inline void scanFrameSlots(const ActRec* ar, type_scan::Scanner& scanner) {
  auto num_slots = ar->func()->numSlotsInFrame();
  auto slots = reinterpret_cast<const TypedValue*>(ar) - num_slots;
  scanner.conservative(slots, num_slots * sizeof(TypedValue));
}

inline void scanNative(const NativeNode* node, type_scan::Scanner& scanner) {
  auto obj = Native::obj(node);
  auto ndi = obj->getVMClass()->getNativeDataInfo();
  auto data = (const char*)obj - ndi->sz;
  scanner.scanByIndex(node->typeIndex(), data, ndi->sz);
  if (auto off = node->arOff()) {
    scanFrameSlots((const ActRec*)((const char*)node + off), scanner);
  }
}

inline void scanResumable(const Resumable* r, type_scan::Scanner& scanner) {
  scanFrameSlots(r->actRec(), scanner);
  scanner.scan(*r);
}

inline void scanAFWH(const ObjectData* obj, type_scan::Scanner& scanner) {
  assert(!obj->getAttribute(ObjectData::HasNativeData));
  // scan ResumableHeader before object
  scanResumable(Resumable::FromObj(obj), scanner);
  // scan C++ properties after [ObjectData] header. should pick up
  // unioned and bit-packed fields
  scanner.conservative(obj + 1,
                       sizeof(c_AsyncFunctionWaitHandle) - sizeof(*obj));
  return obj->scan(scanner);
}

inline void scanHeader(const Header* h, type_scan::Scanner& scanner) {
  switch (h->kind()) {
    case HeaderKind::Proxy:
      return h->proxy_.scan(scanner);
    case HeaderKind::Empty:
      return;
    case HeaderKind::Packed:
    case HeaderKind::VecArray:
      return PackedArray::scan(&h->arr_, scanner);
    case HeaderKind::Mixed:
    case HeaderKind::Dict:
      return h->mixed_.scan(scanner);
    case HeaderKind::Keyset:
      return h->set_.scan(scanner);
    case HeaderKind::Apc:
      return h->apc_.scan(scanner);
    case HeaderKind::Globals:
      return h->globals_.scan(scanner);
    case HeaderKind::Closure:
      scanner.scan(*h->closure_.hdr());
      return h->closure_.scan(scanner); // ObjectData::scan
    case HeaderKind::Object:
      // native objects should hit the NativeData case below.
      assert(!h->obj_.getAttribute(ObjectData::HasNativeData));
      return h->obj_.scan(scanner);
    case HeaderKind::WaitHandle:
    case HeaderKind::AwaitAllWH: {
      // scan C++ properties after [ObjectData] header. should pick up
      // unioned and bit-packed fields
      auto obj = &h->obj_;
      assert(!obj->getAttribute(ObjectData::HasNativeData));
      scanner.conservative(obj + 1, asio_object_size(obj) - sizeof(*obj));
      return obj->scan(scanner);
    }
    case HeaderKind::AsyncFuncWH:
      return scanAFWH(&h->obj_, scanner);
    case HeaderKind::NativeData:
      scanNative(&h->native_, scanner);
      return Native::obj(&h->native_)->scan(scanner);
    case HeaderKind::AsyncFuncFrame:
      return scanAFWH(h->asyncFuncWH(), scanner);
    case HeaderKind::ClosureHdr:
      scanner.scan(h->closure_hdr_);
      return h->closureObj()->scan(scanner); // ObjectData::scan
    case HeaderKind::Pair:
      return h->pair_.scan(scanner);
    case HeaderKind::Vector:
    case HeaderKind::ImmVector:
      return h->vector_.scan(scanner);
    case HeaderKind::Map:
    case HeaderKind::ImmMap:
    case HeaderKind::Set:
    case HeaderKind::ImmSet:
      return h->hashcoll_.scan(scanner);
    case HeaderKind::Resource:
      return scanner.scanByIndex(
        h->res_.typeIndex(),
        h->res_.data(),
        h->res_.heapSize() - sizeof(ResourceHdr)
      );
    case HeaderKind::Ref:
      scanner.scan(*h->ref_.tv());
      return;
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

inline void ObjectData::scan(type_scan::Scanner& scanner) const {
  auto props = propVec();
  if (m_hdr.partially_inited) {
    // we don't know which properties are initialized yet
    scanner.conservative(props, m_cls->numDeclProperties() * sizeof(*props));
  } else {
    scanner.scan(*props, m_cls->numDeclProperties() * sizeof(*props));
  }
  if (getAttribute(HasDynPropArr)) {
    // nb: dynamic property arrays are in ExecutionContext::dynPropTable,
    // which is not marked as a root. Mark the array when scanning the object.
    scanner.scan(g_context->dynPropTable[this]);
  }
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
inline void scanRds(rds::Header* rds, type_scan::Scanner& scanner) {
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
  scanner.where("RdsLocal");
  auto r = rds::localSection();
  scanner.conservative(r.begin(), r.size());

  // php stack TODO #6509338 exactly scan the php stack.
  scanner.where("PhpStack");
  auto stack_end = rds->vmRegs.stack.getStackHighAddress();
  auto sp = rds->vmRegs.stack.top();
  scanner.conservative(sp, uintptr_t(stack_end) - uintptr_t(sp));
}

inline void MemoryManager::scanSweepLists(type_scan::Scanner& scanner) const {
  for (auto s = m_sweepables.next(); s != &m_sweepables; s = s->next()) {
    if (auto h = static_cast<Header*>(s->owner())) {
      assert(h->kind() == HeaderKind::Resource || isObjectKind(h->kind()));
      scanner.enqueue(h); // used to be mark.implicit
    }
  }
  for (auto node: m_natives) {
    scanner.enqueue(node); // used to be mark.implicit(Native::obj(node))
  }
}

inline void MemoryManager::scanRootMaps(type_scan::Scanner& scanner) const {
  // these all used to call mark.implicit
  if (m_objectRoots) scanner.scan(*m_objectRoots);
  if (m_resourceRoots) scanner.scan(*m_resourceRoots);
  for (const auto root : m_root_handles) root->scan(scanner);
}

inline void ThreadLocalManager::scan(type_scan::Scanner& scanner) const {
  auto list = getList(pthread_getspecific(m_key));
  if (!list) return;
  for (auto p = list->head; p != nullptr;) {
    auto node = static_cast<ThreadLocalNode<void>*>(p);
    if (node->m_p) {
      scanner.scanByIndex(node->m_tyindex, node->m_p, node->m_size);
    }
    p = node->m_next;
  }
}

// Scan request-local roots
inline void scanRoots(type_scan::Scanner& scanner) {
  // rds, including php stack
  if (auto rds = rds::header()) {
    scanRds(rds, scanner);
  }
  // C++ stack
  scanner.where("CppStack");
  CALLEE_SAVED_BARRIER(); // ensure stack contains callee-saved registers
  auto sp = stack_top_ptr();
  scanner.conservative(sp, s_stackLimit + s_stackSize - uintptr_t(sp));
  // C++ threadlocal data, but don't scan MemoryManager
  scanner.where("CppTls");
  auto tdata = getCppTdata(); // tdata = { ptr, size }
  if (tdata.second > 0) {
    auto tm = (char*)tdata.first;
    auto tm_end = tm + tdata.second;
    auto mm = (char*)&MM();
    auto mm_end = mm + sizeof(MemoryManager);
    assert(mm >= tm && mm_end <= tm_end);
    scanner.conservative(tm, mm - tm);
    scanner.conservative(mm_end, tm_end - mm_end);
  }
  // ThreadLocal nodes (but skip MemoryManager)
  scanner.where("ThreadLocalManager");
  ThreadLocalManager::GetManager().scan(scanner);
  // Root maps
  scanner.where("RootMaps");
  MM().scanRootMaps(scanner);
  // treat sweep lists as roots until we are ready to test what happens
  // when we start calling various sweep() functions early.
  scanner.where("SweepLists");
  MM().scanSweepLists(scanner);
  if (auto asio = AsioSession::Get()) {
    // ThreadLocalProxy<T> instances aren't in ThreadLocalManager
    scanner.enqueue(asio); // ptr was created with req::make_raw<AsioSession>
  }
#ifdef ENABLE_ZEND_COMPAT
  scanner.where("EzcResources");
  ts_scan_resources(scanner);
#endif
}

}
#endif
