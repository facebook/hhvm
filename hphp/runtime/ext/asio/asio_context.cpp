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
#include "hphp/runtime/ext/asio/asio_context.h"

#include <thread>

#include "hphp/runtime/ext/asio/asio_external_thread_event_queue.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/external_thread_event_wait_handle.h"
#include "hphp/runtime/ext/asio/sleep_wait_handle.h"
#include "hphp/runtime/ext/asio/reschedule_wait_handle.h"
#include "hphp/runtime/ext/asio/waitable_wait_handle.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  template<class TWaitHandle>
  void exitContextQueue(context_idx_t ctx_idx,
                        smart::queue<TWaitHandle*> &queue) {
    while (!queue.empty()) {
      auto wait_handle = queue.front();
      queue.pop();
      wait_handle->exitContext(ctx_idx);
      decRefObj(wait_handle);
    }
  }

  template<bool decRef, class TWaitHandle>
  void exitContextVector(context_idx_t ctx_idx,
                         smart::vector<TWaitHandle*> &vector) {
    while (!vector.empty()) {
      auto wait_handle = vector.back();
      vector.pop_back();
      wait_handle->exitContext(ctx_idx);
      if (decRef) decRefObj(wait_handle);
    }
  }
}

void AsioContext::exit(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx) == this);
  assert(!m_current);

  exitContextQueue(ctx_idx, m_runnableQueue);

  for (auto it : m_priorityQueueDefault) {
    exitContextQueue(ctx_idx, it.second);
  }
  for (auto it : m_priorityQueueNoPendingIO) {
    exitContextQueue(ctx_idx, it.second);
  }

  exitContextVector<false>(ctx_idx, m_sleepEvents);
  exitContextVector<false>(ctx_idx, m_externalThreadEvents);
}

void AsioContext::schedule(c_AsyncFunctionWaitHandle* wait_handle) {
  m_runnableQueue.push(wait_handle);
  wait_handle->incRefCount();
}

void AsioContext::schedule(c_RescheduleWaitHandle* wait_handle, uint32_t queue, uint32_t priority) {
  assert(queue == QUEUE_DEFAULT || queue == QUEUE_NO_PENDING_IO);

  reschedule_priority_queue_t& dst_queue =
    (queue == QUEUE_DEFAULT)
      ? m_priorityQueueDefault
      : m_priorityQueueNoPendingIO;

  // creates a new per-prio queue if necessary
  dst_queue[priority].push(wait_handle);
  wait_handle->incRefCount();
}

void AsioContext::runUntil(c_WaitableWaitHandle* wait_handle) {
  assert(!m_current);
  assert(wait_handle);
  assert(wait_handle->getContext() == this);

  uint8_t check_ete_counter = 0;
  auto session = AsioSession::Get();
  auto ete_queue = session->getExternalThreadEventQueue();
  auto& sleep_queue = session->getSleepEventQueue();

  if (!session->hasAbruptInterruptException()) {
    session->initAbruptInterruptException();
  }

  while (!wait_handle->isFinished()) {
    // Process ready external thread and sleep events once per 256 other events
    // (when 8-bit check_ete_counter overflows).
    if (!++check_ete_counter) {
      // Process any sleep handles that have completed their sleep.
      session->processSleepEvents();

      // Queue may contain received unprocessed events from failed runUntil().
      if (UNLIKELY(ete_queue->hasReceived()) || ete_queue->tryReceiveSome()) {
        ete_queue->processAllReceived();
      }
    }

    // Run queue of ready continuations once.
    if (!m_runnableQueue.empty()) {
      auto current = m_runnableQueue.front();
      m_runnableQueue.pop();
      m_current = current;
      auto exit_guard = folly::makeGuard([&] {
        m_current = nullptr;
        decRefObj(current);
      });

      m_current->run();
      continue;
    }

    // Run default priority queue once.
    if (runSingle(m_priorityQueueDefault)) {
      continue;
    }

    // Continue if any sleep handles can be processed now.
    if (session->processSleepEvents()) {
      continue;
    }

    // Wait for pending external thread events...
    if (!m_externalThreadEvents.empty()) {
      // ...but only until the next sleeper (from any context) finishes.
      AsioSession::TimePoint waketime;
      if (sleep_queue.empty()) {
        waketime = AsioSession::getLatestWakeTime();
      } else {
        waketime = sleep_queue.top()->getWakeTime();
      }

      // Wait if necessary.
      if (LIKELY(!ete_queue->hasReceived())) {
        ete_queue->receiveSomeUntil(waketime);
      }

      if (ete_queue->hasReceived()) {
        // Either we didn't have to wait, or we waited but no sleeper timed us
        // out, so just handle the ETEs.
        ete_queue->processAllReceived();
      } else {
        // No received events means the next-to-wake sleeper timed us out.
        session->processSleepEvents();
      }
      continue;
    }

    // If we're here, then the only things left are sleepers.  Wait for one to
    // be ready (in any context).
    if (!m_sleepEvents.empty()) {
      std::this_thread::sleep_until(sleep_queue.top()->getWakeTime());
      session->processSleepEvents();
      continue;
    }

    // Run no-pending-io priority queue once.
    if (runSingle(m_priorityQueueNoPendingIO)) {
      continue;
    }

    // What? The wait handle did not finish? We know it is part of the current
    // context and since there is nothing else to run, it cannot be in RUNNING
    // or SCHEDULED state. So it must be BLOCKED on something. Apparently, the
    // same logic can be used recursively on the something, so there is an
    // infinite chain of blocked wait handles. But our memory is not infinite.
    // What could it possibly mean? I think we are in a deep sh^H^Hcycle.
    // But we can't, the cycles are detected and avoided at blockOn() time.
    // So, looks like it's not cycle, but the word I started typing first.
    assert(false);
    throw FatalErrorException(
      "Invariant violation: queues are empty, but wait handle did not finish");
  }
}

/**
 * Try to run single RescheduleWaitHandle from the queue.
 */
bool AsioContext::runSingle(reschedule_priority_queue_t& queue) {
  if (queue.empty()) {
    // nothing to run
    return false;
  }

  auto top_queue_iter = queue.begin();
  auto& top_queue = top_queue_iter->second;
  auto reschedule_wait_handle = top_queue.front();
  top_queue.pop();
  reschedule_wait_handle->run();
  decRefObj(reschedule_wait_handle);

  if (top_queue.empty()) {
    queue.erase(top_queue_iter);
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
