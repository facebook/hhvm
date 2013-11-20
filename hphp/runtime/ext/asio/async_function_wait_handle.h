/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class AsyncFunctionWaitHandle

/**
 * A continuation wait handle represents a basic unit of asynchronous execution
 * powered by continuation object. An asynchronous program can be written using
 * continuations; a dependency on another wait handle is set up by awaiting such
 * wait handle, giving control of the execution back to the asio framework.
 */
FORWARD_DECLARE_CLASS(Continuation);
FORWARD_DECLARE_CLASS(AsyncFunctionWaitHandle);
class c_AsyncFunctionWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(AsyncFunctionWaitHandle)

  explicit c_AsyncFunctionWaitHandle(
    Class* cls = c_AsyncFunctionWaitHandle::classof());
  ~c_AsyncFunctionWaitHandle() {}
  void t___construct();
  static void ti_setoncreatecallback(CVarRef callback);
  static void ti_setonawaitcallback(CVarRef callback);
  static void ti_setonsuccesscallback(CVarRef callback);
  static void ti_setonfailcallback(CVarRef callback);
  Object t_getprivdata();
  void t_setprivdata(CObjRef data);

 public:
  static void Create(c_Continuation* continuation);
  void run();
  uint16_t getDepth() { return m_depth; }
  String getName();
  void enterContext(context_idx_t ctx_idx);
  void exitContext(context_idx_t ctx_idx);
  bool isRunning() { return getState() == STATE_RUNNING; }
  String getFileName();
  int getLineNumber();
  const ActRec* getActRec();

 protected:
  void onUnblocked();
  c_WaitableWaitHandle* getChild();

 private:
  void initialize(c_Continuation* continuation, uint16_t depth);
  void markAsSucceeded(const Cell& result);
  void markAsFailed(CObjRef exception);

  p_Continuation m_continuation;
  p_WaitableWaitHandle m_child;
  Object m_privData;
  uint16_t m_depth;

  static const int8_t STATE_SCHEDULED = 4;
  static const int8_t STATE_RUNNING   = 5;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_ASYNC_FUNCTION_WAIT_HANDLE_H_
