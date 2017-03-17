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
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/named-entity-defs.h"
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
  if (m_partially_inited) {
    // we don't know which properties are initialized yet
    scanner.conservative(props, m_cls->numDeclProperties() * sizeof(*props));
  } else {
    scanner.scan(*props, m_cls->numDeclProperties() * sizeof(*props));
  }
  if (getAttribute(HasDynPropArr)) {
    // nb: dynamic property arrays are in ExecutionContext::dynPropTable,
    // which is not marked as a root. Scan the entry pair, so both the key
    // and value are scanned.
    auto iter = g_context->dynPropTable.find(this);
    scanner.scan(*iter); // *iter is pair<ObjectData*,ArrayNoDtor>
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

// Descriptive wrapper types to annotate root nodes
struct PhpStack    { TYPE_SCAN_CONSERVATIVE_ALL; void* dummy; };
struct CppStack    { TYPE_SCAN_CONSERVATIVE_ALL; void* dummy; };
struct CppTls      { TYPE_SCAN_CONSERVATIVE_ALL; void* dummy; };

// call fn(ptr,size,tyindex) for each thing in RDS, including the php stack
// and the rds local segment. The RDS shared part cannot contain pointers.
template<class Fn>
inline void iterateRds(rds::Header* rds, Fn fn) {
  // header and normal section
  fn(rds, sizeof(*rds), type_scan::getIndexForScan<rds::Header>());

  // Normal section.
  rds::forEachNormalAlloc(fn);

  // Local section (mainly static properties).
  // static properties have a per-class, versioned, bool in rds::Normal,
  // tracked by Class::m_sPropCacheInit, plus one TypedValue in rds::Local
  // for each property, tracked in Class::m_sPropCache. Just scan the
  // properties in rds::Local. We ignore the state of the bool, because it
  // is not initialized until after all sprops are initialized, and it's
  // necessary to scan static properties *during* initialization.
  rds::forEachLocalAlloc(fn);

  // php stack TODO #6509338 exactly scan the php stack.
  auto stack_end = rds->vmRegs.stack.getStackHighAddress();
  auto sp = rds->vmRegs.stack.top();
  fn(sp, uintptr_t(stack_end) - uintptr_t(sp),
     type_scan::getIndexForScan<PhpStack>());
}

template<class T> struct EphemeralPtrWrapper {
  T ptr;
  TYPE_SCAN_CUSTOM_FIELD(ptr) { scanner.enqueue(ptr); }
};

template<class Fn>
void MemoryManager::iterateRoots(Fn fn) const {
  using Wrapper = EphemeralPtrWrapper<Header*>;
  for (auto s = m_sweepables.next(); s != &m_sweepables; s = s->next()) {
    if (auto h = static_cast<Header*>(s->owner())) {
      assert(h->kind() == HeaderKind::Resource || isObjectKind(h->kind()));
      auto w = Wrapper{h};
      fn(&w, sizeof(w), type_scan::getIndexForScan<Wrapper>());
    }
  }
  for (auto& node: m_natives) {
    fn(&node, sizeof(node), type_scan::getIndexForScan<NativeNode*>());
  }
  for (const auto root : m_root_handles) {
    root->iterate(fn);
  }
}

template<class Fn> void ThreadLocalManager::iterate(Fn fn) const {
  auto list = getList(pthread_getspecific(m_key));
  if (!list) return;
  for (auto p = list->head; p != nullptr;) {
    auto node = static_cast<ThreadLocalNode<void>*>(p);
    if (node->m_p) {
      fn(node->m_p, node->m_size, node->m_tyindex);
    }
    p = node->m_next;
  }
}

// Visit request-local roots. Each invocation of fn represents one root
// instance of a given type and size, containing potentially several
// pointers.
template<class Fn> void iterateRoots(Fn fn) {
  if (auto rds = rds::header()) {
    iterateRds(rds, fn);
  }

  // cpp stack. ensure stack contains callee-saved registers.
  CALLEE_SAVED_BARRIER();
  auto sp = stack_top_ptr();
  fn(sp, s_stackLimit + s_stackSize - uintptr_t(sp),
     type_scan::getIndexForScan<CppStack>());

  // C++ threadlocal data, but don't scan MemoryManager
  auto tdata = getCppTdata(); // tdata = { ptr, size }
  if (tdata.second > 0) {
    auto tm = (char*)tdata.first;
    auto tm_end = tm + tdata.second;
    auto mm = (char*)&MM();
    auto mm_end = mm + sizeof(MemoryManager);
    if (mm >= tm && mm_end <= tm_end) {
      fn(tm, mm - tm, type_scan::getIndexForScan<CppTls>());
      fn(mm_end, tm_end - mm_end, type_scan::getIndexForScan<CppTls>());
    } else {
      fn(tm, tdata.second, type_scan::getIndexForScan<CppTls>());
    }
  }

  // ThreadLocal nodes (but skip MemoryManager)
  ThreadLocalManager::GetManager().iterate(fn);

  // Root handles & sweep lists
  MM().iterateRoots(fn);

  // asio session
  if (auto asio = AsioSession::Get()) {
    // ThreadLocalProxy<T> instances aren't in ThreadLocalManager.
    // asio was created with req::make_raw<AsioSession>, but we dont have
    // the address of the thread local AsioSession*.
    using Wrapper = EphemeralPtrWrapper<AsioSession*>;
    auto w = Wrapper{asio};
    fn(&w, sizeof(w), type_scan::getIndexForScan<Wrapper>());
  }

  // Zend compat resources
#ifdef ENABLE_ZEND_COMPAT
  ts_iterate_resources(fn);
#endif
}

}
#endif
