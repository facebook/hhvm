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
#include "hphp/runtime/ext/strobelight/ext_strobelight.h"
#include "hphp/runtime/ext/xenon/ext_xenon.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  template<class TWaitHandle>
  void exitContextQueue(context_idx_t ctx_idx,
                        req::deque<TWaitHandle*>& queue) {
    while (!queue.empty()) {
      auto wait_handle = queue.front();
      queue.pop_front();
      wait_handle->exitContext(ctx_idx);
      decRefObj(wait_handle);
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

  inline void onIOWaitExit(AsioSession* session, AsioContext* context) {
    // The web request may have timed out while we were waiting for I/O.  Fail
    // early to avoid further execution of PHP code.  We limit I/O waiting to
    // the time currently remaining in the request (see
    // AsioSession::getLatestWakeTime).
    if (UNLIKELY(checkSurpriseFlags())) {
      c_WaitableWaitHandle* wait_handle = context->getBlamedWaitHandle();

      // First, process the Xenon event. If we are going to time out, we want
      // to attribute the time spent towards the time out to the I/O operation
      // before the fatal exception is thrown.
      if (getSurpriseFlag(XenonSignalFlag)) {
        if (Strobelight::active()) {
          Strobelight::getInstance().log(Xenon::IOWaitSample, wait_handle);
        } else {
          Xenon::getInstance().log(
            Xenon::IOWaitSample,
            EventHook::Source::Asio,
            wait_handle
          );
        }
      }

      auto const flags = handle_request_surprise(wait_handle);
      if (flags & IntervalTimerFlag) {
        IntervalTimer::RunCallbacks(IntervalTimer::IOWaitSample, wait_handle);
      }
      if (flags & TimedOutFlag) {
        RID().invokeUserTimeoutCallback(wait_handle);
      }
    }

    if (UNLIKELY(session->hasOnIOWaitExit())) {
      session->onIOWaitExit(context->getBlamedWaitHandle());
    }
  }
}

void AsioContext::exit(context_idx_t ctx_idx) {
  assertx(AsioSession::Get()->getContext(ctx_idx) == this);

  exitContextVector(ctx_idx, m_runnableQueue);
  exitContextVector(ctx_idx, m_fastRunnableQueue);

  for (auto& it : m_priorityQueueDefault) {
    exitContextQueue(ctx_idx, it.second);
  }
  for (auto& it : m_priorityQueueNoPendingIO) {
    exitContextQueue(ctx_idx, it.second);
  }

  exitContextVector(ctx_idx, m_sleepEvents);
  exitContextVector(ctx_idx, m_externalThreadEvents);
}

void AsioContext::schedule(c_RescheduleWaitHandle* wait_handle, uint32_t queue,
                           int64_t priority) {
  assertx(queue == QUEUE_DEFAULT || queue == QUEUE_NO_PENDING_IO);
  assertx(!(priority < 0));

  auto& dst_queue = queue == QUEUE_DEFAULT ? m_priorityQueueDefault :
                    m_priorityQueueNoPendingIO;

  // creates a new per-prio queue if necessary
  dst_queue[priority].push_back(wait_handle);
  wait_handle->incRefCount();
}

c_AsyncFunctionWaitHandle* AsioContext::maybePopFast() {
  assertx(this == AsioSession::Get()->getCurrentContext());

  while (!m_fastRunnableQueue.empty()) {
    auto wh = m_fastRunnableQueue.back();
    m_fastRunnableQueue.pop_back();

    if (wh->getState() == c_ResumableWaitHandle::STATE_READY &&
        wh->isFastResumable()) {
      // We only call maybePopFast() on the current context.  Since `wh' was
      // scheduled in this context at some point, it must still be scheduled
      // here now, since the only way it could leave the context is if the
      // context was destroyed.  (Being scheduled here supercedes it having
      // been scheduled in earlier contexts.)
      assertx(wh->getContextIdx() ==
              AsioSession::Get()->getCurrentContextIdx());
      return wh;
    } else {
      // `wh' is blocked or finished in some other context.
      m_fastRunnableQueue.push_back(wh);
      return nullptr;
    }
  }
  return nullptr;
}

void AsioContext::runUntil(c_WaitableWaitHandle* wait_handle) {
  assertx(wait_handle);
  assertx(wait_handle->getContext() == this);

  auto session = AsioSession::Get();
  auto ete_queue = session->getExternalThreadEventQueue();

  if (!session->hasAbruptInterruptException()) {
    session->initAbruptInterruptException();
  }

  while (!wait_handle->isFinished()) {
    // Run queue of ready async functions once.
    if (!m_runnableQueue.empty()) {
      auto wh = m_runnableQueue.back();
      m_runnableQueue.pop_back();
      if (wh->getState() != c_ResumableWaitHandle::STATE_READY) {
        // may happen if wh was scheduled in multiple contexts
        decRefObj(wh);
      } else {
        wh->resume();
      }
      continue;
    }

    if (!m_fastRunnableQueue.empty()) {
      auto wh = m_fastRunnableQueue.back();
      m_fastRunnableQueue.pop_back();
      if (wh->getState() != c_ResumableWaitHandle::STATE_READY) {
        // may happen if wh was scheduled in multiple contexts
        decRefObj(wh);
      } else {
        wh->resume();
      }
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

      onIOWaitEnter(session);
      // check if onIOWaitEnter callback unblocked any wait handles
      if (LIKELY(m_runnableQueue.empty() &&
                 m_fastRunnableQueue.empty() &&
                 !m_externalThreadEvents.empty() &&
                 !ete_queue->hasReceived() &&
                 m_priorityQueueDefault.empty())) {
        auto waketime = session->sleepWakeTime();
        ete_queue->receiveSomeUntil(waketime);
      }
      onIOWaitExit(session, this);

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
      // check if onIOWaitEnter callback unblocked any wait handles
      if (LIKELY(m_runnableQueue.empty() &&
                 m_fastRunnableQueue.empty() &&
                 m_externalThreadEvents.empty() &&
                 m_priorityQueueDefault.empty() &&
                 !m_sleepEvents.empty())) {
        std::this_thread::sleep_until(session->sleepWakeTime());
      }
      onIOWaitExit(session, this);

      session->processSleepEvents();
      continue;
    }

    // Run no-pending-io priority queue once.
    if (runSingle(m_priorityQueueNoPendingIO)) {
      continue;
    }

    // What? The wait handle did not finish? We know it is part of the current
    // context and since there is nothing else to run, it cannot be in RUNNING
    // or READY/SCHEDULED state. So it must be BLOCKED on something. Apparently,
    // the same logic can be used recursively on the something, so there is an
    // infinite chain of blocked wait handles. But our memory is not infinite.
    // What could it possibly mean? I think we are in a deep sh^H^Hcycle.
    // But we can't, the cycles are detected and avoided at blockOn() time.
    // So, looks like it's not cycle, but the word I started typing first.
    assertx(false);
    raise_fatal_error(
      "Invariant violation: queues are empty, but wait handle did not finish");
  }
}

c_WaitableWaitHandle* AsioContext::getBlamedWaitHandle() {
  // first let's try to find wait handle, responsible for wakeup
  auto session = AsioSession::Get();
  auto ete_queue = session->getExternalThreadEventQueue();

  c_ExternalThreadEventWaitHandle* ewh = ete_queue->lastReceived();
  if (ewh != nullptr &&
      ewh->getContextIdx() == session->getCurrentContextIdx()) {
    return ewh;
  }

  // may return cancelled wait handle, which no longer has contextIdx
  c_SleepWaitHandle* swh = session->nextSleepEvent();
  if (swh != nullptr && !swh->isFinished() &&
      swh->getContextIdx() == session->getCurrentContextIdx() &&
      swh->getWakeTime() <= AsioSession::TimePoint::clock::now()) {
    return swh;
  }

  // not found? let's find a wait handle from current context to blame
  if (!m_externalThreadEvents.empty()) {
    return m_externalThreadEvents[0];
  }

  if (!m_sleepEvents.empty()) {
    return m_sleepEvents[0];
  }

  return nullptr;
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
