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

#ifndef incl_HPHP_EXT_ASIO_SLEEP_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_SLEEP_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class SleepWaitHandle

/**
 * A wait handle that sleeps until a give time passes.
 */
class c_SleepWaitHandle final : public c_WaitableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(SleepWaitHandle);

  explicit c_SleepWaitHandle(Class* cls = c_SleepWaitHandle::classof())
    : c_WaitableWaitHandle(cls) {}
  ~c_SleepWaitHandle() {}
  static void ti_setoncreatecallback(const Variant& callback);
  static void ti_setonsuccesscallback(const Variant& callback);
  static Object ti_create(int64_t usecs);

 public:
  void process();
  String getName();
  void exitContext(context_idx_t ctx_idx);
  AsioSession::TimePoint getWakeTime() const { return m_waketime; };
  void registerToContext();
  void unregisterFromContext();

 private:
  void setState(uint8_t state) { setKindState(Kind::Sleep, state); }
  void initialize(int64_t usecs);

  AsioSession::TimePoint m_waketime;

 public:
  static const int8_t STATE_WAITING = 2;
};

inline c_SleepWaitHandle* c_WaitHandle::asSleep() {
  assert(getKind() == Kind::Sleep);
  return static_cast<c_SleepWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_SLEEP_WAIT_HANDLE_H_
