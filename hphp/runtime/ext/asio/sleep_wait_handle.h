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

#ifndef incl_HPHP_EXT_ASIO_SLEEP_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_SLEEP_WAIT_HANDLE_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/asio/session_scoped_wait_handle.h"
#include "hphp/runtime/ext/asio/asio_session.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class SleepWaitHandle

/**
 * A wait handle that sleeps until a give time passes.
 */
FORWARD_DECLARE_CLASS(SleepWaitHandle);
class c_SleepWaitHandle : public c_SessionScopedWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(SleepWaitHandle);

  explicit c_SleepWaitHandle(Class* cls = c_SleepWaitHandle::classof())
    : c_SessionScopedWaitHandle(cls)
  {}
  ~c_SleepWaitHandle() {}
  void t___construct();
  static Object ti_create(int64_t usecs);

 public:
  void process();
  String getName();
  AsioSession::TimePoint getWakeTime() const { return m_waketime; };

  void setIndex(uint32_t ev_idx) {
    assert(getState() == STATE_WAITING);
    m_index = ev_idx;
  }

 protected:
  void registerToContext();
  void unregisterFromContext();

 private:
  void initialize(int64_t usecs);

  AsioSession::TimePoint m_waketime;
  uint32_t m_index;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_SLEEP_WAIT_HANDLE_H_
