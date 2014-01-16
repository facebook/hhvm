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

#ifndef incl_HPHP_EXT_ASIO_SESSION_SCOPED_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_SESSION_SCOPED_WAIT_HANDLE_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/asio/waitable_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class SessionScopedWaitHandle

/**
 * A wait handle whose execution transcends context-scope.
 */
FORWARD_DECLARE_CLASS(SessionScopedWaitHandle);
class c_SessionScopedWaitHandle : public c_WaitableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(SessionScopedWaitHandle)

  explicit c_SessionScopedWaitHandle(Class* cls =
      c_SessionScopedWaitHandle::classof())
    : c_WaitableWaitHandle(cls)
  {}
  ~c_SessionScopedWaitHandle() {}

  void t___construct();

 public:
  void exitContext(context_idx_t ctx_idx);

 protected:
  void enterContextImpl(context_idx_t ctx_idx);
  virtual void registerToContext() = 0;
  virtual void unregisterFromContext() = 0;

  static const int8_t STATE_WAITING = 3;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_SESSION_SCOPED_WAIT_HANDLE_H_
