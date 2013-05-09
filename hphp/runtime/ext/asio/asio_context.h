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
#include <runtime/base/base_includes.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FORWARD_DECLARE_CLASS_BUILTIN(WaitableWaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(ContinuationWaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(RescheduleWaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(ExternalThreadEventWaitHandle);

typedef uint8_t context_idx_t;

class AsioContext {
  public:
    void* operator new(size_t size) { return smart_malloc(size); }
    void operator delete(void* ptr) { smart_free(ptr); }

    AsioContext() : m_current(nullptr) {}
    void exit(context_idx_t ctx_idx);

    bool isRunning() { return m_current; }
    c_ContinuationWaitHandle* getCurrent() { return m_current; }

    void schedule(c_ContinuationWaitHandle* wait_handle);
    void schedule(c_RescheduleWaitHandle* wait_handle, uint32_t queue, uint32_t priority);
    uint32_t registerExternalThreadEvent(c_ExternalThreadEventWaitHandle* wait_handle);
    void unregisterExternalThreadEvent(uint32_t ete_idx);
    void runUntil(c_WaitableWaitHandle* wait_handle);

    static const uint32_t QUEUE_DEFAULT       = 0;
    static const uint32_t QUEUE_NO_PENDING_IO = 1;

  private:
    typedef smart::map<uint32_t, smart::queue<c_RescheduleWaitHandle*>>
      reschedule_priority_queue_t;

    bool runSingle(reschedule_priority_queue_t& queue);

    c_ContinuationWaitHandle* m_current;

    // queue of ContinuationWaitHandles ready for immediate execution
    smart::queue<c_ContinuationWaitHandle*> m_runnableQueue;

    // queue of RescheduleWaitHandles scheduled in default mode
    reschedule_priority_queue_t m_priorityQueueDefault;

    // queue of RescheduleWaitHandles scheduled to be run once there is no pending I/O
    reschedule_priority_queue_t m_priorityQueueNoPendingIO;

    // list of all pending ExternalThreadEventWaitHandles
    smart::vector<c_ExternalThreadEventWaitHandle*> m_externalThreadEvents;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_CONTEXT_H_
