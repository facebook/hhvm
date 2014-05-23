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

#ifndef incl_HPHP_EXT_ASIO_ASYNC_FUNCTION_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_ASYNC_FUNCTION_WAIT_HANDLE_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/asio/resumable_wait_handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class AsyncFunctionWaitHandle

/**
 * An async function wait handle represents a basic unit of asynchronous
 * execution. A dependency on another wait handle is set up by awaiting such
 * wait handle, giving control of the execution back to the asio framework.
 */
FORWARD_DECLARE_CLASS(AsyncFunctionWaitHandle);
class c_AsyncFunctionWaitHandle : public c_ResumableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(AsyncFunctionWaitHandle)

  explicit c_AsyncFunctionWaitHandle(
    Class* cls = c_AsyncFunctionWaitHandle::classof());
  ~c_AsyncFunctionWaitHandle();
  static void ti_setoncreatecallback(const Variant& callback);
  static void ti_setonawaitcallback(const Variant& callback);
  static void ti_setonsuccesscallback(const Variant& callback);
  static void ti_setonfailcallback(const Variant& callback);
  Object t_getprivdata();
  void t_setprivdata(const Object& data);

 public:
  static constexpr ptrdiff_t resumableOff() { return -sizeof(Resumable); }
  static constexpr ptrdiff_t arOff() {
    return resumableOff() + Resumable::arOff();
  }
  static constexpr ptrdiff_t resumeAddrOff() {
    return resumableOff() + Resumable::resumeAddrOff();
  }
  static constexpr ptrdiff_t resumeOffsetOff() {
    return resumableOff() + Resumable::resumeOffsetOff();
  }
  static constexpr ptrdiff_t childOff() {
    return offsetof(c_AsyncFunctionWaitHandle, m_child);
  }
  static c_AsyncFunctionWaitHandle* Create(const ActRec* origFp,
                                           size_t numSlots,
                                           JIT::TCA resumeAddr,
                                           Offset resumeOffset,
                                           ObjectData* child);
  void resume();
  void onUnblocked();
  void ret(Cell& result);
  String getName();
  c_WaitableWaitHandle* getChild();
  void enterContextImpl(context_idx_t ctx_idx);
  void exitContext(context_idx_t ctx_idx);
  bool isRunning() { return getState() == STATE_RUNNING; }
  void suspend(JIT::TCA resumeAddr, Offset resumeOffset,
               c_WaitableWaitHandle* child);
  String getFileName();
  Offset getNextExecutionOffset();
  int getLineNumber();

  Resumable* resumable() const {
    return reinterpret_cast<Resumable*>(
      const_cast<char*>(reinterpret_cast<const char*>(this) + resumableOff()));
  }

  ActRec* actRec() const {
    return resumable()->actRec();
  }

 private:
  void setState(uint8_t state) { setKindState(Kind::AsyncFunction, state); }
  void initialize(c_WaitableWaitHandle* child);
  void markAsSucceeded();
  void markAsFailed(const Object& exception);
  c_WaitableWaitHandle* child() { return m_child; }

  // valid if STATE_SCHEDULED || STATE_BLOCKED
  c_WaitableWaitHandle* m_child;

  Object m_privData;

  static const int8_t STATE_SCHEDULED = 3;
  static const int8_t STATE_RUNNING   = 4;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_ASYNC_FUNCTION_WAIT_HANDLE_H_
