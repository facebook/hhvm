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

#ifndef incl_HPHP_EXT_ASIO_CONTEXT_H_
#define incl_HPHP_EXT_ASIO_CONTEXT_H_

#include <functional>
#include <queue>
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/req-containers.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ActRec;
class c_WaitableWaitHandle;
class c_ResumableWaitHandle;
class c_RescheduleWaitHandle;
class c_SleepWaitHandle;
class c_ExternalThreadEventWaitHandle;

typedef uint8_t context_idx_t;

struct AsioContext final {
  void* operator new(size_t size) { return req::malloc(size); }
  void operator delete(void* ptr) { req::free(ptr); }

  explicit AsioContext(ActRec* savedFP) : m_savedFP(savedFP) {}
  void exit(context_idx_t ctx_idx);

  ActRec* getSavedFP() const { return m_savedFP; }

  void schedule(c_ResumableWaitHandle* wait_handle) {
    m_runnableQueue.push_back(wait_handle);
  }
  void schedule(c_RescheduleWaitHandle* wait_handle, uint32_t queue,
                int64_t priority);

  template <class TWaitHandle>
  uint32_t registerTo(req::vector<TWaitHandle*>& vec, TWaitHandle* wh);

  template <class TWaitHandle>
  void unregisterFrom(req::vector<TWaitHandle*>& vec, uint32_t idx);

  req::vector<c_SleepWaitHandle*>& getSleepEvents() {
    return m_sleepEvents;
  };
  req::vector<c_ExternalThreadEventWaitHandle*>& getExternalThreadEvents() {
    return m_externalThreadEvents;
  };

  void runUntil(c_WaitableWaitHandle* wait_handle);

  static constexpr uint32_t QUEUE_DEFAULT       = 0;
  static constexpr uint32_t QUEUE_NO_PENDING_IO = 1;

private:
  typedef req::map<int64_t, req::deque<c_RescheduleWaitHandle*>>
    reschedule_priority_queue_t;

  bool runSingle(reschedule_priority_queue_t& queue);

private:
  // Frame pointer to the ActRec of the WaitHandle::join() call.
  ActRec* m_savedFP;

  // stack of ResumableWaitHandles ready for immediate execution
  req::vector<c_ResumableWaitHandle*> m_runnableQueue;

  // queue of RescheduleWaitHandles scheduled in default mode
  reschedule_priority_queue_t m_priorityQueueDefault;

  // queue of RescheduleWaitHandles scheduled to be run once there is no
  // pending I/O
  reschedule_priority_queue_t m_priorityQueueNoPendingIO;

  // pending wait handles
  req::vector<c_SleepWaitHandle*> m_sleepEvents;
  req::vector<c_ExternalThreadEventWaitHandle*> m_externalThreadEvents;
};

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/ext/asio/asio-context-inl.h"

#endif // incl_HPHP_EXT_ASIO_CONTEXT_H_
