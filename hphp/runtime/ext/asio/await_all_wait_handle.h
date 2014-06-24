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

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class AwaitAllWaitHandle

/**
 * A wait handle that waits for a list of wait handles. The wait handle succeeds
 * with null once all given wait handles are finished (succeeded or failed).
 */
FORWARD_DECLARE_CLASS(AwaitAllWaitHandle);
class c_AwaitAllWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(AwaitAllWaitHandle)

  explicit c_AwaitAllWaitHandle(Class* cls = c_AwaitAllWaitHandle::classof())
    : c_BlockableWaitHandle(cls)
  {}
  ~c_AwaitAllWaitHandle() {}

  static void ti_setoncreatecallback(const Variant& callback);
  static Object ti_fromarray(const Array& dependencies);
  static Object ti_frommap(const Variant& dependencies);
  static Object ti_fromvector(const Variant& dependencies);

 public:
  String getName();
  void onUnblocked();
  c_WaitableWaitHandle* getChild();
  void enterContextImpl(context_idx_t ctx_idx);

 private:
  static Object FromPackedArray(const ArrayData* dependencies);
  static Object FromMixedArray(const MixedArray* dependencies);
  static Object FromMap(const BaseMap* dependencies);
  static Object FromVector(const BaseVector* dependencies);
  static c_AwaitAllWaitHandle* Alloc(int32_t cnt);
  void initialize();
  template<bool checkCycle> void blockOnCurrent();
  void markAsFailed(const Object& exception);

 private:
  void setState(uint8_t state) { setKindState(Kind::AwaitAll, state); }
  int32_t m_cur;
  int32_t m_size;
  c_WaitableWaitHandle* m_children[0];
};

inline c_AwaitAllWaitHandle* c_WaitHandle::asAwaitAll() {
  assert(getKind() == Kind::AwaitAll);
  return static_cast<c_AwaitAllWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_AWAIT_ALL_WAIT_HANDLE_H_
