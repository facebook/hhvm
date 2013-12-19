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

#ifndef incl_HPHP_EXT_ASIO_CONTEXT_H_
#define incl_HPHP_EXT_ASIO_CONTEXT_H_

#include <functional>
#include <queue>
#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/smart-containers.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FORWARD_DECLARE_CLASS(WaitableWaitHandle);
FORWARD_DECLARE_CLASS(AsyncFunctionWaitHandle);
FORWARD_DECLARE_CLASS(RescheduleWaitHandle);
FORWARD_DECLARE_CLASS(SleepWaitHandle);
FORWARD_DECLARE_CLASS(ExternalThreadEventWaitHandle);

typedef uint8_t context_idx_t;

class AsioContext {
  public:
    void* operator new(size_t size) { return smart_malloc(size); }
    void operator delete(void* ptr) { smart_free(ptr); }

    AsioContext() : m_current(nullptr) {}
    void exit(context_idx_t ctx_idx);

    bool isRunning() { return m_current; }
    c_AsyncFunctionWaitHandle* getCurrent() { return m_current; }

    void schedule(c_AsyncFunctionWaitHandle* wait_handle);
    void schedule(c_RescheduleWaitHandle* wait_handle, uint32_t queue, uint32_t priority);

    template <class TWaitHandle>
    uint32_t registerTo(smart::vector<TWaitHandle*>& vec, TWaitHandle* wh);

    template <class TWaitHandle>
    void unregisterFrom(smart::vector<TWaitHandle*>& vec, uint32_t ev_idx);

    smart::vector<c_SleepWaitHandle*>& getSleepEvents() {
      return m_sleepEvents;
    };
    smart::vector<c_ExternalThreadEventWaitHandle*>& getExternalThreadEvents() {
      return m_externalThreadEvents;
    };

    void runUntil(c_WaitableWaitHandle* wait_handle);

    static const uint32_t QUEUE_DEFAULT       = 0;
    static const uint32_t QUEUE_NO_PENDING_IO = 1;

  private:
    typedef smart::map<uint32_t, smart::queue<c_RescheduleWaitHandle*>>
      reschedule_priority_queue_t;

    bool runSingle(reschedule_priority_queue_t& queue);

    c_AsyncFunctionWaitHandle* m_current;

    // queue of AsyncFunctionWaitHandles ready for immediate execution
    smart::queue<c_AsyncFunctionWaitHandle*> m_runnableQueue;

    // queue of RescheduleWaitHandles scheduled in default mode
    reschedule_priority_queue_t m_priorityQueueDefault;

    // queue of RescheduleWaitHandles scheduled to be run once there is no pending I/O
    reschedule_priority_queue_t m_priorityQueueNoPendingIO;

    // pending wait handles
    smart::vector<c_SleepWaitHandle*> m_sleepEvents;
    smart::vector<c_ExternalThreadEventWaitHandle*> m_externalThreadEvents;
};

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/ext/asio/asio_context-inl.h"

#endif // incl_HPHP_EXT_ASIO_CONTEXT_H_
