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

#ifndef incl_HPHP_EXT_ASIO_RESUMABLE_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_RESUMABLE_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"

namespace HPHP {

struct ActRec;

///////////////////////////////////////////////////////////////////////////////
// class ResumableWaitHandle

/**
 * A resumable wait handle is a wait handle that represents asynchronous
 * execution of PHP code that can be resumed once the result of awaited
 * WaitHandle becomes available.
 */
class c_ResumableWaitHandle : public c_WaitableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(ResumableWaitHandle)

  explicit c_ResumableWaitHandle(Class* cls = c_ResumableWaitHandle::classof(),
                                 HeaderKind kind = HeaderKind::Object) noexcept
    : c_WaitableWaitHandle(cls, kind) {}
  ~c_ResumableWaitHandle() {}
  static void ti_setoncreatecallback(const Variant& callback);
  static void ti_setonawaitcallback(const Variant& callback);
  static void ti_setonsuccesscallback(const Variant& callback);
  static void ti_setonfailcallback(const Variant& callback);

 public:
  static c_ResumableWaitHandle* getRunning(ActRec* fp);
  void resume();
  void exitContext(context_idx_t ctx_idx);

  static const int8_t STATE_BLOCKED   = 2;
  static const int8_t STATE_SCHEDULED = 3;
  static const int8_t STATE_RUNNING   = 4;
};

inline c_ResumableWaitHandle* c_WaitHandle::asResumable() {
  assert(getKind() == Kind::AsyncFunction || getKind() == Kind::AsyncGenerator);
  return static_cast<c_ResumableWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_RESUMABLE_WAIT_HANDLE_H_
