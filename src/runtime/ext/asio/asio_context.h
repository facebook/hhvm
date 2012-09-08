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

class AsioContext : public Sweepable {
  public:
    static AsioContext* Enter(AsioContext* parent);
    AsioContext* exit();

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

    void schedule(c_ContinuationWaitHandle* wait_handle, uint32_t prio);
    void runUntil(c_WaitableWaitHandle* wait_handle);

  private:
    AsioContext(AsioContext* parent);

    uint16_t m_contextDepth;
    uint16_t m_waitHandleDepth;
    AsioContext* m_parent;
    c_ContinuationWaitHandle* m_current;

    // queue for Continuations scheduled with default priority (0)
    std::queue<c_ContinuationWaitHandle*> m_default_queue;

    // queue for Continuations scheduled with other priority
    std::map<uint32_t, std::queue<c_ContinuationWaitHandle*>> m_priority_queue;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_ASIO_CONTEXT_H__
