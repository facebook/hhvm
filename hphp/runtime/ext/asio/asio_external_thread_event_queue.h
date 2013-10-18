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

#ifndef incl_HPHP_EXT_ASIO_EXTERNAL_THREAD_EVENT_QUEUE_H_
#define incl_HPHP_EXT_ASIO_EXTERNAL_THREAD_EVENT_QUEUE_H_

#include "hphp/runtime/base/base-includes.h"
#include <atomic>
#include <condition_variable>
#include <mutex>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FORWARD_DECLARE_CLASS(ExternalThreadEventWaitHandle);

/* This is not an optimal solution
 * This value is in principle a constexp, but the integer-to-pointer cast would
 * require a reinterpret cast, which is not allowed in a constexpr
 */
#define K_CONSUMER_WAITING (static_cast<c_ExternalThreadEventWaitHandle*>((void*)1L))

class AsioExternalThreadEventQueue {
  public:
    AsioExternalThreadEventQueue();

    bool hasReceived() { return m_received; }
    void processAllReceived();
    bool abandonAllReceived(c_ExternalThreadEventWaitHandle* wait_handle);

    bool tryReceiveSome();
    bool receiveSomeUntil(
        std::chrono::time_point<std::chrono::steady_clock> waketime);
    void receiveSome();
    void send(c_ExternalThreadEventWaitHandle* wait_handle);

  private:
    c_ExternalThreadEventWaitHandle* m_received;
    std::atomic<c_ExternalThreadEventWaitHandle*> m_queue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCondition;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_EXTERNAL_THREAD_EVENT_QUEUE_H_
