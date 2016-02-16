/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_ASIO_AWAIT_ALL_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_AWAIT_ALL_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class AwaitAllWaitHandle

/**
 * A wait handle that waits for a list of wait handles. The wait handle succeeds
 * with null once all given wait handles are finished (succeeded or failed).
 */
struct c_AwaitAllWaitHandle final : c_WaitableWaitHandle {
  WAITHANDLE_CLASSOF(AwaitAllWaitHandle);
  static void instanceDtor(ObjectData* obj, const Class*) {
    auto wh = wait_handle<c_AwaitAllWaitHandle>(obj);
    auto const sz = wh->heapSize();
    wh->~c_AwaitAllWaitHandle();
    MM().objFree(obj, sz);
  }

  explicit c_AwaitAllWaitHandle(Class* cls = c_AwaitAllWaitHandle::classof())
    : c_AwaitAllWaitHandle(0, cls)
  {}

  explicit c_AwaitAllWaitHandle(unsigned cap,
                                Class* cls = c_AwaitAllWaitHandle::classof())
    : c_WaitableWaitHandle(cls, HeaderKind::AwaitAllWH)
    , m_cap(cap)
    , m_unfinished(cap - 1)
  {}
  ~c_AwaitAllWaitHandle() {}

 public:
  class Node final {
   public:
    static constexpr ptrdiff_t blockableOff() {
      return offsetof(Node, m_blockable);
    }

    uint32_t getChildIdx() {
      return m_blockable.getExtraData();
    }

    inline c_AwaitAllWaitHandle* getWaitHandle() {
      return reinterpret_cast<c_AwaitAllWaitHandle*>(const_cast<char*>(
        reinterpret_cast<const char*>(this - getChildIdx())
        - c_AwaitAllWaitHandle::childrenOff()));
    }

    bool isFirstUnfinishedChild() {
      return getChildIdx() == getWaitHandle()->m_unfinished;
    }

    void onUnblocked() {
      getWaitHandle()->onUnblocked(getChildIdx());
    }

    AsioBlockable m_blockable;
    c_WaitableWaitHandle* m_child;
  };

  static constexpr ptrdiff_t childrenOff() {
    return offsetof(c_AwaitAllWaitHandle, m_children);
  }

  String getName();
  void onUnblocked(uint32_t idx);
  c_WaitableWaitHandle* getChild();
  template<typename T> void forEachChild(T fn);
  template<class F> void scanChildren(F&) const;

  size_t heapSize() const { return heapSize(m_cap); }
  static size_t heapSize(unsigned count) {
    return sizeof(c_AwaitAllWaitHandle) + count * sizeof(Node);
  }

 private:
  template<typename T, typename F1, typename F2>
  static Object createAAWH(T start, T stop, F1 iterNext, F2 getCell);
  static Object FromPackedArray(const ArrayData* dependencies);
  static Object FromStructArray(const StructArray* dependencies);
  static Object FromMixedArray(const MixedArray* dependencies);
  static Object FromMap(const BaseMap* dependencies);
  static Object FromVector(const BaseVector* dependencies);
  static req::ptr<c_AwaitAllWaitHandle> Alloc(int32_t cnt);
  void initialize(context_idx_t ctx_idx);
  void markAsFinished(void);
  void markAsFailed(const Object& exception);
  void setState(uint8_t state) { setKindState(Kind::AwaitAll, state); }

  friend Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromArray,
                                   const Array& dependencies);
  friend Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromMap,
                          const Variant& dependencies);
  friend Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromVector,
                          const Variant& dependencies);
 private:
  static constexpr uint32_t kMaxNodes = AsioBlockable::kExtraInfoMax + 1;
  uint32_t const m_cap; // how many children we have room for.
  uint32_t m_unfinished; // index of the first unfinished child
  Node m_children[0]; // allocated off the end

 public:
  static const int8_t STATE_BLOCKED = 2;
};

void HHVM_STATIC_METHOD(AwaitAllWaitHandle, setOnCreateCallback,
                        const Variant& callback);
Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromArray,
                          const Array& dependencies);
Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromMap,
                          const Variant& dependencies);
Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromVector,
                          const Variant& dependencies);

inline c_AwaitAllWaitHandle* c_WaitHandle::asAwaitAll() {
  assert(getKind() == Kind::AwaitAll);
  return static_cast<c_AwaitAllWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/ext/asio/ext_await-all-wait-handle-inl.h"

#endif // incl_HPHP_EXT_ASIO_AWAIT_ALL_WAIT_HANDLE_H_
