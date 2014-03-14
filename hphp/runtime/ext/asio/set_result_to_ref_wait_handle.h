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

#ifndef incl_HPHP_EXT_ASIO_SET_RESULT_TO_REF_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_SET_RESULT_TO_REF_WAIT_HANDLE_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class SetResultToRefWaitHandle

/**
 * A wait handle that waits for a given dependency and sets its result to
 * a given reference once completed.
 */
FORWARD_DECLARE_CLASS(SetResultToRefWaitHandle);
class c_SetResultToRefWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(SetResultToRefWaitHandle)

  explicit c_SetResultToRefWaitHandle(Class* cls =
      c_SetResultToRefWaitHandle::classof())
    : c_BlockableWaitHandle(cls)
    , m_child()
    , m_ref()
  {}
  ~c_SetResultToRefWaitHandle() {
    if (m_ref) decRefRef(m_ref);
  }
  void t___construct();
  static void ti_setoncreatecallback(const Variant& callback);
  static Object ti_create(const Object& wait_handle, VRefParam ref);


 public:
  String getName();

 protected:
  void onUnblocked();
  c_WaitableWaitHandle* getChild();
  void enterContextImpl(context_idx_t ctx_idx);

 private:
  void initialize(c_WaitableWaitHandle* wait_handle, RefData* ref);
  void markAsSucceeded(const Cell& result);
  void markAsFailed(const Object& exception);

  p_WaitableWaitHandle m_child;
  RefData* m_ref;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_SET_RESULT_TO_REF_WAIT_HANDLE_H_
