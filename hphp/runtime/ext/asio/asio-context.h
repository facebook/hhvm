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

#ifndef incl_HPHP_EXT_ASIO_CONTEXT_H_
#define incl_HPHP_EXT_ASIO_CONTEXT_H_

#include "hphp/runtime/base/req-deque.h"
#include "hphp/runtime/base/req-map.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/ext/asio/asio-context-index.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ActRec;
struct c_WaitableWaitHandle;
struct c_ResumableWaitHandle;
struct c_RescheduleWaitHandle;
struct c_SleepWaitHandle;
struct c_ExternalThreadEventWaitHandle;
struct c_AsyncFunctionWaitHandle;

using reschedule_priority_queue_t = req::map<
  int64_t, req::deque<c_RescheduleWaitHandle*>
>;

// Context state holding WH queues
struct AsioContextState {
private:
  explicit AsioContextState() {}

  bool hasWork() {
    // Notably, do not check m_priorityQueueNoPendingIO here. This is handled in
    // the context by runSingle() to ensure proper batching of I/O.
    return !m_runnableQueue.empty() ||
      !m_fastRunnableQueue.empty() ||
      !m_priorityQueueDefault.empty();
  }

  // stack of ResumableWaitHandles ready for immediate execution
  req::vector<c_ResumableWaitHandle*> m_runnableQueue;

  // stack of AsyncFunctionWaitHandles ready for immediate execution
  req::vector<c_AsyncFunctionWaitHandle*> m_fastRunnableQueue;

  // queue of RescheduleWaitHandles scheduled in default mode
  reschedule_priority_queue_t m_priorityQueueDefault;

  // queue of RescheduleWaitHandles scheduled to be run once there is no
  // pending I/O
  reschedule_priority_queue_t m_priorityQueueNoPendingIO;

  friend struct AsioContext;
};

struct AsioContext final {
  explicit AsioContext(ActRec* savedFP)
    : m_savedFP(savedFP), m_regularState(), m_lowState() {}
  void exit(ContextIndex ContextIndex);

  ActRec* getSavedFP() const { return m_savedFP; }

  void schedule(c_ResumableWaitHandle* wait_handle);
  void scheduleFast(c_AsyncFunctionWaitHandle* wait_handle);
  void schedule(c_RescheduleWaitHandle* wait_handle, uint32_t queue,
                int64_t priority);

  c_AsyncFunctionWaitHandle* maybePopFast();

  template <class TWaitHandle>
  uint32_t registerTo(req::vector<TWaitHandle*>& vec, TWaitHandle* wh);

  template <class TWaitHandle>
  void unregisterFrom(req::vector<TWaitHandle*>& vec, uint32_t idx);

  req::vector<c_SleepWaitHandle*>& getSleepEvents() {
    return m_sleepEvents;
  }
  req::vector<c_ExternalThreadEventWaitHandle*>& getExternalThreadEvents() {
    return m_externalThreadEvents;
  }

  void runUntil(c_WaitableWaitHandle* wait_handle);

  // used for back-traces, when surprise flag is handled
  // from inside Asio scheduler
  c_WaitableWaitHandle* getBlamedWaitHandle();

  static constexpr uint32_t QUEUE_DEFAULT       = 0;
  static constexpr uint32_t QUEUE_NO_PENDING_IO = 1;

private:
  // Frame pointer to the ActRec of the \HH\Asio\join() call.
  ActRec* m_savedFP;

  // Context states for managing priority contexts
  AsioContextState m_regularState;
  AsioContextState m_lowState;

  // pending wait handles
  req::vector<c_SleepWaitHandle*> m_sleepEvents;
  req::vector<c_ExternalThreadEventWaitHandle*> m_externalThreadEvents;

  bool runSingle(reschedule_priority_queue_t& queue);

  AsioContextState* getContextState(ContextStateIndex ctxStateIdx);
};

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/ext/asio/asio-context-inl.h"

#endif // incl_HPHP_EXT_ASIO_CONTEXT_H_
