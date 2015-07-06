/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
  DECLARE_CLASS_NO_SWEEP(AwaitAllWaitHandle)

  explicit c_AwaitAllWaitHandle(Class* cls = c_AwaitAllWaitHandle::classof())
    : c_AwaitAllWaitHandle(0, cls)
  {}

  explicit c_AwaitAllWaitHandle(unsigned cap,
                                Class* cls = c_AwaitAllWaitHandle::classof())
    : c_WaitableWaitHandle(cls, HeaderKind::AwaitAllWH)
    , m_cap(cap)
    , m_cur(cap - 1)
  {}
  ~c_AwaitAllWaitHandle() {}

  static void ti_setoncreatecallback(const Variant& callback);
  static Object ti_fromarray(const Array& dependencies);
  static Object ti_frommap(const Variant& dependencies);
  static Object ti_fromvector(const Variant& dependencies);

 public:
  static constexpr ptrdiff_t blockableOff() {
    return offsetof(c_AwaitAllWaitHandle, m_blockable);
  }

  String getName();
  void onUnblocked();
  c_WaitableWaitHandle* getChild();
  template<typename T> void forEachChild(T fn);

  size_t heapSize() const { return heapSize(m_cap); }
  static size_t heapSize(unsigned count) {
    return sizeof(c_AwaitAllWaitHandle) + count * sizeof(c_WaitableWaitHandle*);
  }

 private:
  static Object FromPackedArray(const ArrayData* dependencies);
  static Object FromMixedArray(const MixedArray* dependencies);
  static Object FromMap(const BaseMap* dependencies);
  static Object FromVector(const BaseVector* dependencies);
  static req::ptr<c_AwaitAllWaitHandle> Alloc(int32_t cnt);
  void initialize(context_idx_t ctx_idx);
  template<bool checkCycle> void blockOnCurrent();
  void markAsFailed(const Object& exception);
  void setState(uint8_t state) { setKindState(Kind::AwaitAll, state); }

 private:
  uint32_t const m_cap; // how many children we have room for.
  int32_t m_cur; // index of last child
  AsioBlockable m_blockable;
  c_WaitableWaitHandle* m_children[0]; // allocated off the end

 public:
  static const int8_t STATE_BLOCKED = 2;
};

inline c_AwaitAllWaitHandle* c_WaitHandle::asAwaitAll() {
  assert(getKind() == Kind::AwaitAll);
  return static_cast<c_AwaitAllWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/ext/asio/ext_await-all-wait-handle-inl.h"

#endif // incl_HPHP_EXT_ASIO_AWAIT_ALL_WAIT_HANDLE_H_
