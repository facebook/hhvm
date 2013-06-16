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

#include "hphp/runtime/ext/asio/asio_external_thread_event_queue.h"
#include "hphp/runtime/ext/ext_asio.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

AsioExternalThreadEventQueue::AsioExternalThreadEventQueue()
    : m_queue(nullptr), m_queueMutex(), m_queueCondition() {
}

c_ExternalThreadEventWaitHandle* AsioExternalThreadEventQueue::consumeMulti() {
  // try check for ready external thread events without grabbing lock
  auto ready = m_queue.exchange(nullptr);
  if (ready != nullptr) {
    assert(ready != k_consumerWaiting);
    return ready;
  }

  // no ready external thread events available, synchronization needed
  std::unique_lock<std::mutex> lock(m_queueMutex);

  // transition from empty to WAITING
  if (m_queue.compare_exchange_strong(ready, k_consumerWaiting)) {
    // wait for transition from WAITING to non-empty
    do {
      m_queueCondition.wait(lock);
    } while (m_queue.load() == k_consumerWaiting);
  } else  {
    // external thread transitioned from empty to non-empty while grabbing lock
  }

  ready = m_queue.exchange(nullptr);
  assert(ready != nullptr);
  assert(ready != k_consumerWaiting);
  return ready;
}

void AsioExternalThreadEventQueue::produce(c_ExternalThreadEventWaitHandle* wait_handle) {
  auto next = m_queue.load();
  while (true) {
    while (next != k_consumerWaiting) {
      wait_handle->setNextToProcess(next);
      if (m_queue.compare_exchange_weak(next, wait_handle)) {
        return;
      }
    }

    // try to transition from WAITING to non-empty
    wait_handle->setNextToProcess(nullptr);
    if (m_queue.compare_exchange_weak(next, wait_handle)) {
      // succeeded, notify condition
      std::unique_lock<std::mutex> lock(m_queueMutex);
      m_queueCondition.notify_one();
      return;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
