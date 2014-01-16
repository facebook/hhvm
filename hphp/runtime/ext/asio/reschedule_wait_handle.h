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

#ifndef incl_HPHP_EXT_ASIO_RESCHEDULE_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_RESCHEDULE_WAIT_HANDLE_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/asio/waitable_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class RescheduleWaitHandle

extern const int64_t q_RescheduleWaitHandle$$QUEUE_DEFAULT;
extern const int64_t q_RescheduleWaitHandle$$QUEUE_NO_PENDING_IO;

/**
 * A wait handle that is enqueued into a given priority queue and once desired
 * execution priority is eligible for execution, it succeeds with a null result.
 *
 * RescheduleWaitHandle is guaranteed to never finish immediately.
 */
FORWARD_DECLARE_CLASS(RescheduleWaitHandle);
class c_RescheduleWaitHandle : public c_WaitableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(RescheduleWaitHandle)

  explicit c_RescheduleWaitHandle(Class* cls =
      c_RescheduleWaitHandle::classof())
    : c_WaitableWaitHandle(cls)
  {}
  ~c_RescheduleWaitHandle() {}

  void t___construct();
  static Object ti_create(int64_t queue, int priority);

 public:
  void run();
  String getName();
  void exitContext(context_idx_t ctx_idx);

 protected:
  void enterContextImpl(context_idx_t ctx_idx);

 private:
  void initialize(uint32_t queue, uint32_t priority);

  uint32_t m_queue;
  uint32_t m_priority;

  static const int8_t STATE_SCHEDULED = 3;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_RESCHEDULE_WAIT_HANDLE_H_
