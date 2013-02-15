/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __EXT_ASIO_CONTEXT_H__
#define __EXT_ASIO_CONTEXT_H__

#include <functional>
#include <queue>
#include <runtime/base/base_includes.h>
#include <runtime/ext/ext_asio.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FORWARD_DECLARE_CLASS_BUILTIN(WaitableWaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(ContinuationWaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(RescheduleWaitHandle);

class AsioContext {
  public:
    static AsioContext* Enter(AsioContext* parent);
    AsioContext* exit();

    void* operator new(size_t size) { return smart_malloc(size); }
    void operator delete(void* ptr) { smart_free(ptr); }

    inline uint16_t getContextDepth() { return m_contextDepth; }
    inline uint16_t getWaitHandleDepth() { return m_waitHandleDepth; }
    inline uint16_t getCurrentWaitHandleDepth() {
      return m_current ? m_current->getDepth() : m_waitHandleDepth;
    }

    inline AsioContext* getParent() { return m_parent; }
    inline c_ContinuationWaitHandle* getCurrent() { return m_current; }

    static void RunUntil(c_WaitableWaitHandle* wait_handle);

    inline bool includes(AsioContext* ctx) {
      return
        LIKELY(this == ctx) || (ctx && m_contextDepth < ctx->m_contextDepth);
    }

    void schedule(c_ContinuationWaitHandle* wait_handle);
    void schedule(c_RescheduleWaitHandle* wait_handle, uint32_t queue, uint32_t priority);
    void runUntil(c_WaitableWaitHandle* wait_handle);

    static const uint32_t QUEUE_DEFAULT       = 0;
    static const uint32_t QUEUE_NO_PENDING_IO = 1;

  private:
    typedef smart::map<uint32_t, smart::queue<c_RescheduleWaitHandle*>>
      reschedule_priority_queue_t;

    AsioContext(AsioContext* parent);

    bool runSingle(reschedule_priority_queue_t& queue);

    uint16_t m_contextDepth;
    uint16_t m_waitHandleDepth;
    AsioContext* m_parent;
    c_ContinuationWaitHandle* m_current;

    // queue of ContinuationWaitHandles ready for immediate execution
    smart::queue<c_ContinuationWaitHandle*> m_queue_ready;

    // queue of RescheduleWaitHandles scheduled in default mode
    reschedule_priority_queue_t m_priority_queue_default;

    // queue of RescheduleWaitHandles scheduled to be run once there is no pending I/O
    reschedule_priority_queue_t m_priority_queue_no_pending_io;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_ASIO_CONTEXT_H__
