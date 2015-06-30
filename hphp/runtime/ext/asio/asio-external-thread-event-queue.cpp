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

#include "hphp/runtime/ext/asio/asio-external-thread-event-queue.h"
#include <mutex>

#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

AsioExternalThreadEventQueue::AsioExternalThreadEventQueue()
    : m_received(nullptr), m_queue(nullptr), m_queueMutex(),
      m_queueCondition() {
}

/**
 * Process all received finished events.
 *
 * May throw C++ exception that may leave some events unprocessed.
 */
void AsioExternalThreadEventQueue::processAllReceived() {
  assert(m_received);
  do {
    auto ete_wh = m_received;
    m_received = m_received->getNextToProcess();
    ete_wh->process();
  } while (m_received);
}

/**
 * Abandon all received finished events.
 *
 * Returns true iff provided wait handle was abandoned.
 */
bool AsioExternalThreadEventQueue::abandonAllReceived(c_ExternalThreadEventWaitHandle* wait_handle) {
  assert(m_received);
  bool seen = false;
  do {
    auto ete_wh = m_received;
    m_received = m_received->getNextToProcess();
    ete_wh->abandon(true);
    seen |= ete_wh == wait_handle;
  } while (m_received);
  return seen;
}

/**
 * Try to receive finished events without blocking.
 *
 * Returns true iff at least one event was received.
 */
bool AsioExternalThreadEventQueue::tryReceiveSome() {
  assert(!m_received);
  m_received = m_queue.exchange(nullptr);
  assert(m_received != K_CONSUMER_WAITING);
  return m_received;
}

/**
 * Receive at least one finished event, or until waketime (a steady_clock time)
 * is reached.
 *
 * Returns true if events were received; if utime is passed as zero, no timeout
 * is set and this method is guaranteed to return true.
 */
bool AsioExternalThreadEventQueue::receiveSomeUntil(
    std::chrono::time_point<std::chrono::steady_clock> waketime) {
  assert(!m_received);

  // try receive external thread events without grabbing lock
  m_received = m_queue.exchange(nullptr);
  if (m_received) {
    assert(m_received != K_CONSUMER_WAITING);
    return true;
  }

  // no external thread events received, synchronization needed
  std::unique_lock<std::mutex> lock(m_queueMutex);

  // transition from empty to WAITING
  if (m_queue.compare_exchange_strong(m_received, K_CONSUMER_WAITING)) {
    // wait for transition from WAITING to non-empty
    do {
      std::cv_status status = m_queueCondition.wait_until(lock, waketime);

      if (status == std::cv_status::timeout) {
        // We timed out without receiving events.  Unflag ourselves as
        // waiting.
        m_received = m_queue.exchange(nullptr);
        assert(m_received);

        // If we were still waiting on an event, reset our state and return;
        // otherwise, a send() must have completed, so run with the received
        // event even if timeout occurred.
        if (m_received == K_CONSUMER_WAITING) {
          m_received = nullptr;
          return false;
        } else {
          return true;
        }
      }
    } while (m_queue.load() == K_CONSUMER_WAITING);
  } else  {
    // external thread transitioned from empty to non-empty while grabbing lock
  }

  m_received = m_queue.exchange(nullptr);
  assert(m_received);
  assert(m_received != K_CONSUMER_WAITING);

  return true;
}

/**
 * Receive at least one finished event.
 */
void AsioExternalThreadEventQueue::receiveSome() {
  bool received UNUSED = receiveSomeUntil(AsioSession::getLatestWakeTime());
  assert(received);
}

/**
 * Send finished event from the processing thread to the web request thread.
 */
void AsioExternalThreadEventQueue::send(c_ExternalThreadEventWaitHandle* wait_handle) {
  auto next = m_queue.load();
  while (true) {
    while (next != K_CONSUMER_WAITING) {
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
