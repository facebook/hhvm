/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#pragma once

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"
#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class ConcurrentWaitHandle

/**
 * A wait handle that waits for a list of wait handles. The wait handle succeeds
 * with null once all given wait handles are finished (succeeded or failed).
 */
struct c_ConcurrentWaitHandle final :
    c_WaitableWaitHandle,
    SystemLib::ClassLoader<"HH\\ConcurrentWaitHandle"> {
  using SystemLib::ClassLoader<"HH\\ConcurrentWaitHandle">::classof;
  using SystemLib::ClassLoader<"HH\\ConcurrentWaitHandle">::className;
  static void instanceDtor(ObjectData* obj, const Class*) {
    auto wh = wait_handle<c_ConcurrentWaitHandle>(obj);
    auto const sz = wh->heapSize();
    wh->~c_ConcurrentWaitHandle();
    tl_heap->objFree(obj, sz);
  }

  explicit c_ConcurrentWaitHandle(unsigned cap = 0)
    : c_WaitableWaitHandle(classof(), HeaderKind::ConcurrentWH,
        type_scan::getIndexForMalloc<
          c_ConcurrentWaitHandle,
          type_scan::Action::WithSuffix<Node>
        >())
    , m_cap(cap)
    , m_unfinished(cap - 1)
  {}
  ~c_ConcurrentWaitHandle() {
    assertx(isFinished());
    for (int32_t i = 0; i < m_cap; i++) {
      assertx(isFailed() || m_children[i].m_child->isFinished());
      decRefObj(m_children[i].m_child);
    }
  }

  // [frame, last) are the set of locals, and cnt is number of
  // non-finished wait handles in that range.
  static ObjectData* fromFrameNoCheck(const ActRec* fp,
                                      uint32_t first,
                                      uint32_t last,
                                      uint32_t cnt);

 public:
  struct Node final {
    static constexpr ptrdiff_t blockableOff() {
      return offsetof(Node, m_blockable);
    }

    uint32_t getChildIdx() {
      return m_index;
    }

    inline c_ConcurrentWaitHandle* getWaitHandle() {
      return reinterpret_cast<c_ConcurrentWaitHandle*>(const_cast<char*>(
        reinterpret_cast<const char*>(this - getChildIdx())
        - c_ConcurrentWaitHandle::childrenOff()));
    }

    bool isFirstUnfinishedChild() {
      return getChildIdx() == getWaitHandle()->m_unfinished;
    }

    void onUnblocked() {
      getWaitHandle()->onUnblocked(getChildIdx());
    }

    AsioBlockable m_blockable;
    c_WaitableWaitHandle* m_child;
    uint32_t m_index;
  };

  static constexpr ptrdiff_t childrenOff() {
    return offsetof(c_ConcurrentWaitHandle, m_children);
  }

  String getName();
  void onUnblocked(uint32_t idx);
  c_WaitableWaitHandle* getChild();
  template<typename T> void forEachChild(T fn);

  size_t heapSize() const { return heapSize(m_cap); }
  static size_t heapSize(unsigned count) {
    return sizeof(c_ConcurrentWaitHandle) + count * sizeof(Node);
  }
  void scan(type_scan::Scanner&) const;

 private:
  static req::ptr<c_ConcurrentWaitHandle> Alloc(int32_t cnt);
  void initialize(context_idx_t ctx_idx);
  void markAsFinished(void);
  void markAsFailed(const Object& exception);
  void setState(uint8_t state) { setKindState(Kind::Concurrent, state); }

 private:
  uint32_t const m_cap; // how many children we have room for.
  uint32_t m_unfinished; // index of the first unfinished child
  Node m_children[0]; // allocated off the end
  TYPE_SCAN_FLEXIBLE_ARRAY_FIELD(m_children);

 public:
  static const int8_t STATE_BLOCKED = 2;
};

inline c_ConcurrentWaitHandle* c_Awaitable::asConcurrent() {
  assertx(getKind() == Kind::Concurrent);
  return static_cast<c_ConcurrentWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_EXT_ASIO_CONCURRENT_WAIT_HANDLE_H_
#include "hphp/runtime/ext/asio/ext_concurrent-wait-handle-inl.h"
#undef incl_HPHP_EXT_ASIO_CONCURRENT_WAIT_HANDLE_H_
