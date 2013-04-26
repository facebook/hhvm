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
#include <runtime/ext/asio/asio_session.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_PROXY(AsioSession, false, AsioSession::s_current);

namespace {
  const context_idx_t MAX_CONTEXT_DEPTH = std::numeric_limits<context_idx_t>::max();
}

void AsioSession::Init() {
  s_current.set(new AsioSession());
}

AsioSession::AsioSession()
    : m_contexts(), m_readyExternalThreadEvents(nullptr),
      m_readyExternalThreadEventsMutex(),
      m_readyExternalThreadEventsCondition() {
}

void AsioSession::enterContext() {
  assert(!isInContext() || getCurrentContext()->isRunning());

  if (UNLIKELY(getCurrentContextIdx() >= MAX_CONTEXT_DEPTH)) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      "Unable to enter asio context: too many contexts open"));
    throw e;
  }

  m_contexts.push_back(new AsioContext());

  assert(static_cast<context_idx_t>(m_contexts.size()) == m_contexts.size());
  assert(isInContext());
  assert(!getCurrentContext()->isRunning());
}

void AsioSession::exitContext() {
  assert(isInContext());
  assert(!getCurrentContext()->isRunning());

  m_contexts.back()->exit(m_contexts.size());
  delete m_contexts.back();
  m_contexts.pop_back();

  assert(!isInContext() || getCurrentContext()->isRunning());
}

uint16_t AsioSession::getCurrentWaitHandleDepth() {
  assert(!isInContext() || getCurrentContext()->isRunning());
  return isInContext() ? getCurrentWaitHandle()->getDepth() : 0;
}

c_ExternalThreadEventWaitHandle* AsioSession::waitForExternalThreadEvents() {
  // try check for ready external thread events without grabbing lock
  auto ready = m_readyExternalThreadEvents.exchange(nullptr);
  if (ready != nullptr) {
    assert(ready != k_waitingForExternalThreadEvents);
    return ready;
  }

  // no ready external thread events available, synchronization needed
  std::unique_lock<std::mutex> lock(m_readyExternalThreadEventsMutex);

  // transition from empty to WAITING
  if (m_readyExternalThreadEvents.compare_exchange_strong(ready, k_waitingForExternalThreadEvents)) {
    // wait for transition from WAITING to non-empty
    do {
      m_readyExternalThreadEventsCondition.wait(lock);
    } while (m_readyExternalThreadEvents.load() == k_waitingForExternalThreadEvents);
  } else  {
    // external thread transitioned from empty to non-empty while grabbing lock
  }

  ready = m_readyExternalThreadEvents.exchange(nullptr);
  assert(ready != nullptr);
  assert(ready != k_waitingForExternalThreadEvents);
  return ready;
}

void AsioSession::enqueueExternalThreadEvent(c_ExternalThreadEventWaitHandle* wait_handle) {
  auto next = m_readyExternalThreadEvents.load();
  while (true) {
    while (next != k_waitingForExternalThreadEvents) {
      wait_handle->setNextToProcess(next);
      if (m_readyExternalThreadEvents.compare_exchange_weak(next, wait_handle)) {
        return;
      }
    }

    // try to transition from WAITING to non-empty
    wait_handle->setNextToProcess(nullptr);
    if (m_readyExternalThreadEvents.compare_exchange_weak(next, wait_handle)) {
      // succeeded, notify condition
      m_readyExternalThreadEventsCondition.notify_one();
      return;
    }
  }
}

void AsioSession::onFailed(CObjRef exception) {
  if (m_onFailedCallback.get()) {
    try {
      vm_call_user_func(m_onFailedCallback, Array::Create(exception));
    } catch (const Object& callback_exception) {
      raise_warning("[asio] Ignoring exception thrown by onFailed callback");
    }
  }
}

void AsioSession::onContinuationCreate(c_ContinuationWaitHandle* cont) {
  assert(m_onContinuationCreateCallback.get());
  try {
    vm_call_user_func(
      m_onContinuationCreateCallback,
      Array::Create(cont));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by ContinuationWaitHandle::onCreate callback");
  }
}

void AsioSession::onContinuationYield(c_ContinuationWaitHandle* cont, c_WaitHandle* child) {
  assert(m_onContinuationYieldCallback.get());
  try {
    vm_call_user_func(
      m_onContinuationYieldCallback,
      CREATE_VECTOR2(cont, child));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by ContinuationWaitHandle::onYield callback");
  }
}

void AsioSession::onContinuationSuccess(c_ContinuationWaitHandle* cont, CVarRef result) {
  assert(m_onContinuationSuccessCallback.get());
  try {
    vm_call_user_func(
      m_onContinuationSuccessCallback,
      CREATE_VECTOR2(cont, result));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by ContinuationWaitHandle::onSuccess callback");
  }
}

void AsioSession::onContinuationFail(c_ContinuationWaitHandle* cont, CObjRef exception) {
  assert(m_onContinuationFailCallback.get());
  try {
    vm_call_user_func(
      m_onContinuationFailCallback,
      CREATE_VECTOR2(cont, exception));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by ContinuationWaitHandle::onFail callback");
  }
}

void AsioSession::onJoin(c_WaitHandle* wait_handle) {
  assert(m_onJoinCallback.get());
  try {
    vm_call_user_func(m_onJoinCallback, Array::Create(wait_handle));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by WaitHandle::onJoin callback");
  }
}

void AsioSession::onGenArrayCreate(c_GenArrayWaitHandle* wait_handle, CVarRef dependencies) {
  assert(m_onGenArrayCreateCallback.get());
  try {
    vm_call_user_func(
      m_onGenArrayCreateCallback,
      CREATE_VECTOR2(wait_handle, dependencies));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by GenArrayWaitHandle::onCreate callback");
  }
}

void AsioSession::onSetResultToRefCreate(c_SetResultToRefWaitHandle* wait_handle, CObjRef child) {
  assert(m_onSetResultToRefCreateCallback.get());
  try {
    vm_call_user_func(
      m_onSetResultToRefCreateCallback,
      CREATE_VECTOR2(wait_handle, child));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by SetResultToRefWaitHandle::onCreate callback");
  }
}

///////////////////////////////////////////////////////////////////////////////
}
