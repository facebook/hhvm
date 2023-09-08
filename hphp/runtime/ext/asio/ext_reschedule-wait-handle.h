/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_EXT_ASIO_RESCHEDULE_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_RESCHEDULE_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class RescheduleWaitHandle

/**
 * A wait handle that is enqueued into a given priority queue and once desired
 * execution priority is eligible for execution, it succeeds with a null result.
 *
 * RescheduleWaitHandle is guaranteed to never finish immediately.
 */
struct c_RescheduleWaitHandle final :
    c_WaitableWaitHandle,
    SystemLib::ClassLoader<"HH\\RescheduleWaitHandle"> {
  using SystemLib::ClassLoader<"HH\\RescheduleWaitHandle">::classof;
  using SystemLib::ClassLoader<"HH\\RescheduleWaitHandle">::className;
  WAITHANDLE_DTOR(RescheduleWaitHandle);

  explicit c_RescheduleWaitHandle()
    : c_WaitableWaitHandle(classof(), HeaderKind::WaitHandle,
                     type_scan::getIndexForMalloc<c_RescheduleWaitHandle>()) {}
  ~c_RescheduleWaitHandle() {}

 public:
  void run();
  String getName();
  void exitContext(context_idx_t ctx_idx);
  void scheduleInContext();

 private:
  void setState(uint8_t state) { setKindState(Kind::Reschedule, state); }
  void initialize(uint32_t queue, int64_t priority);

  friend Object HHVM_STATIC_METHOD(RescheduleWaitHandle, create,
                                   int64_t queue, int64_t priority);

  uint32_t m_queue;
  int64_t m_priority;

 public:
  static const int8_t STATE_SCHEDULED = 2; // waiting in priority queue
};

inline c_RescheduleWaitHandle* c_Awaitable::asReschedule() {
  assertx(getKind() == Kind::Reschedule);
  return static_cast<c_RescheduleWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_RESCHEDULE_WAIT_HANDLE_H_
