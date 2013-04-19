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

#include <runtime/ext/ext_asio.h>
#include <runtime/ext/asio/asio_context.h>
#include <runtime/ext/asio/asio_session.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  template<class TWaitHandle>
  void exitContextQueue(context_idx_t ctx_idx, smart::queue<TWaitHandle*> &queue) {
    while (!queue.empty()) {
      auto wait_handle = queue.front();
      queue.pop();
      wait_handle->exitContext(ctx_idx);
      decRefObj(wait_handle);
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
}

void AsioContext::schedule(c_ContinuationWaitHandle* wait_handle) {
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

  while (!wait_handle->isFinished()) {
    // run queue of ready continuations
    while (!m_runnableQueue.empty()) {
      auto current = m_runnableQueue.front();
      m_runnableQueue.pop();
      m_current = current;
      m_current->run();
      m_current = nullptr;
      decRefObj(current);

      if (wait_handle->isFinished()) {
        return;
      }
    }

    // run default priority queue once
    if (runSingle(m_priorityQueueDefault)) {
      continue;
    }

    // run no-pending-io priority queue once
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
