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
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

AsioContext::AsioContext(AsioContext* parent)
    : m_contextDepth(parent ? parent->m_contextDepth + 1 : 0)
    , m_waitHandleDepth(parent ? parent->getCurrentWaitHandleDepth() : 0)
    , m_parent(parent), m_current(nullptr) {
}

AsioContext* AsioContext::Enter(AsioContext* parent) {
  return new AsioContext(parent);
}

AsioContext* AsioContext::exit() {
  AsioContext* parent = m_parent;

  c_ContinuationWaitHandle* wait_handle;

  while (!m_default_queue.empty()) {
    wait_handle = m_default_queue.front();
    wait_handle->exitContext(this);
    m_default_queue.pop();
    decRefObj(wait_handle);
  }

  for (auto it = m_priority_queue.begin(); it != m_priority_queue.end(); ++it) {
    auto& queue = it->second;

    while (!queue.empty()) {
      wait_handle = queue.front();
      wait_handle->exitContext(this);
      queue.pop();
      decRefObj(wait_handle);
    }
  }

  // release memory explicitly and make sure we are not sweeped
  unregister();
  delete this;

  return parent;
}

void AsioContext::schedule(c_ContinuationWaitHandle* wait_handle, uint32_t prio) {
  if (prio == 0) {
    m_default_queue.push(wait_handle);
    wait_handle->incRefCount();
  } else {
    // creates a new per-prio queue if necessary
    m_priority_queue[prio].push(wait_handle);
    wait_handle->incRefCount();
  }
}

void AsioContext::runUntil(c_WaitableWaitHandle* wait_handle) {
  assert(!m_current);
  assert(wait_handle);
  assert(wait_handle->getContext() == this);

  while (true) {
    // run default queue
    while (!m_default_queue.empty()) {
      auto current = m_default_queue.front();
      m_default_queue.pop();
      m_current = current;
      m_current->run();
      m_current = nullptr;
      decRefObj(current);

      if (wait_handle->isFinished()) {
        return;
      }
    }

    // run priority queue once (it can schedule new default queue events)
    if (!m_priority_queue.empty()) {
      auto top_queue_iter = m_priority_queue.begin();
      auto& top_queue = top_queue_iter->second;
      auto current = top_queue.front();
      top_queue.pop();
      m_current = current;
      m_current->run();
      m_current = nullptr;
      decRefObj(current);

      if (top_queue.empty()) {
        m_priority_queue.erase(top_queue_iter);
      }

      if (wait_handle->isFinished()) {
        return;
      }

      continue;
    }

    // What? The wait handle did not finish? We know it is part of the current
    // context and since there is nothing else to run, it cannot be in RUNNING
    // or SCHEDULED state. So it must be BLOCKED on something. Apparently, the
    // same logic can be used recursively on the something, so there is an
    // infinite chain of blocked wait handles. But our memory is not infinite.
    // What does it mean? We are in deep sh^H^Hcycle. Let's find and kill it.
    auto blockable_wh = dynamic_cast<c_BlockableWaitHandle*>(wait_handle);
    if (!blockable_wh) {
      throw FatalErrorException(
          "Invariant violation: queues are empty, but non-blockable wait "
          "handle did not finish");
    }
    blockable_wh->killCycle();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
