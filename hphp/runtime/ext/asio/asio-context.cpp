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
#include "hphp/runtime/ext/asio/asio-context.h"

#include <thread>

#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event-queue.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_reschedule-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle-defs.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"
#include "hphp/runtime/ext/intervaltimer/ext_intervaltimer.h"
#include "hphp/runtime/ext/xenon/ext_xenon.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  template<bool decRef, class TWaitHandle>
  void exitContextQueue(context_idx_t ctx_idx,
                        req::deque<TWaitHandle*>& queue) {
    while (!queue.empty()) {
      auto wait_handle = queue.front();
      queue.pop_front();
      wait_handle->exitContext(ctx_idx);
      if (decRef) decRefObj(wait_handle);
    }
  }

  template<class TWaitHandle>
  void exitContextVector(context_idx_t ctx_idx,
                         req::vector<TWaitHandle*> &vector) {
    while (!vector.empty()) {
      auto wait_handle = vector.back();
      vector.pop_back();
      wait_handle->exitContext(ctx_idx);
    }
  }

  inline void onIOWaitEnter(AsioSession* session) {
    if (UNLIKELY(session->hasOnIOWaitEnter())) {
      session->onIOWaitEnter();
    }
  }

  inline void onIOWaitExit(AsioSession* session) {
    // The web request may have timed out while we were waiting for I/O.  Fail
    // early to avoid further execution of PHP code.  We limit I/O waiting to
    // the time currently remaining in the request (see
    // AsioSession::getLatestWakeTime).
    if (UNLIKELY(checkSurpriseFlags())) {
      auto const flags = check_request_surprise();
      if (flags & XenonSignalFlag) {
        Xenon::getInstance().log(Xenon::IOWaitSample);
      }
      if (flags & IntervalTimerFlag) {
        IntervalTimer::RunCallbacks(IntervalTimer::IOWaitSample);
      }
    }

    if (UNLIKELY(session->hasOnIOWaitExit())) {
      session->onIOWaitExit();
    }
  }
}

void AsioContext::exit(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx) == this);

  exitContextVector(ctx_idx, m_runnableQueue);

  for (auto& it : m_priorityQueueDefault) {
    exitContextQueue<true>(ctx_idx, it.second);
  }
  for (auto& it : m_priorityQueueNoPendingIO) {
    exitContextQueue<true>(ctx_idx, it.second);
  }

  exitContextVector(ctx_idx, m_sleepEvents);
  exitContextVector(ctx_idx, m_externalThreadEvents);
}

void AsioContext::schedule(c_RescheduleWaitHandle* wait_handle, uint32_t queue,
                           int64_t priority) {
  assert(queue == QUEUE_DEFAULT || queue == QUEUE_NO_PENDING_IO);
  assert(!(priority < 0));

  auto& dst_queue = queue == QUEUE_DEFAULT ? m_priorityQueueDefault :
                    m_priorityQueueNoPendingIO;

  // creates a new per-prio queue if necessary
  dst_queue[priority].push_back(wait_handle);
  wait_handle->incRefCount();
}

void AsioContext::runUntil(c_WaitableWaitHandle* wait_handle) {
  assert(wait_handle);
  assert(wait_handle->getContext() == this);

  auto session = AsioSession::Get();
  auto ete_queue = session->getExternalThreadEventQueue();

  if (!session->hasAbruptInterruptException()) {
    session->initAbruptInterruptException();
  }

  while (!wait_handle->isFinished()) {
    // Run queue of ready async functions once.
    if (!m_runnableQueue.empty()) {
      auto current = m_runnableQueue.back();
      m_runnableQueue.pop_back();
      current->resume();
      continue;
    }

    // Process all sleep handles that have completed their sleep.
    if (session->processSleepEvents()) {
      continue;
    }

    // Process all external thread events that have completed their operation.
    // Queue may contain received unprocessed events from failed runUntil().
    if (UNLIKELY(ete_queue->hasReceived()) || ete_queue->tryReceiveSome()) {
      ete_queue->processAllReceived();
      continue;
    }

    // Run default priority queue once.
    if (runSingle(m_priorityQueueDefault)) {
      continue;
    }

    // Wait for pending external thread events...
    if (!m_externalThreadEvents.empty()) {
      // ...but only until the next sleeper (from any context) finishes.
      auto waketime = session->sleepWakeTime();

      // Wait if necessary.
      if (LIKELY(!ete_queue->hasReceived())) {
        onIOWaitEnter(session);
        ete_queue->receiveSomeUntil(waketime);
        onIOWaitExit(session);
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
      onIOWaitEnter(session);
      std::this_thread::sleep_until(session->sleepWakeTime());
      onIOWaitExit(session);

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
  top_queue.pop_front();
  reschedule_wait_handle->run();
  decRefObj(reschedule_wait_handle);

  if (top_queue.empty()) {
    queue.erase(top_queue_iter);
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
