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

#include "hphp/runtime/ext/asio/asio_session.h"

#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_array_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_map_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_vector_wait_handle.h"
#include "hphp/runtime/ext/asio/set_result_to_ref_wait_handle.h"
#include "hphp/runtime/ext/asio/sleep_wait_handle.h"
#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/system/systemlib.h"

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
    : m_contexts(), m_externalThreadEventQueue() {
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

void AsioSession::initAbruptInterruptException() {
  assert(!hasAbruptInterruptException());
  m_abruptInterruptException = SystemLib::AllocInvalidOperationExceptionObject(
    "The request was abruptly interrupted.");
}

void AsioSession::onAsyncFunctionCreate(c_AsyncFunctionWaitHandle* cont) {
  assert(m_onAsyncFunctionCreateCallback.get());
  try {
    vm_call_user_func(
      m_onAsyncFunctionCreateCallback,
      Array::Create(cont));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by AsyncFunctionWaitHandle::onCreate callback");
  }
}

void AsioSession::onAsyncFunctionAwait(c_AsyncFunctionWaitHandle* cont, c_WaitHandle* child) {
  assert(m_onAsyncFunctionAwaitCallback.get());
  try {
    vm_call_user_func(
      m_onAsyncFunctionAwaitCallback,
      make_packed_array(cont, child));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by AsyncFunctionWaitHandle::onAwait callback");
  }
}

void AsioSession::onAsyncFunctionSuccess(c_AsyncFunctionWaitHandle* cont, CVarRef result) {
  assert(m_onAsyncFunctionSuccessCallback.get());
  try {
    vm_call_user_func(
      m_onAsyncFunctionSuccessCallback,
      make_packed_array(cont, result));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by AsyncFunctionWaitHandle::onSuccess callback");
  }
}

void AsioSession::onAsyncFunctionFail(c_AsyncFunctionWaitHandle* cont, CObjRef exception) {
  assert(m_onAsyncFunctionFailCallback.get());
  try {
    vm_call_user_func(
      m_onAsyncFunctionFailCallback,
      make_packed_array(cont, exception));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by AsyncFunctionWaitHandle::onFail callback");
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
      make_packed_array(wait_handle, dependencies));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by GenArrayWaitHandle::onCreate callback");
  }
}

void AsioSession::onGenMapCreate(c_GenMapWaitHandle* wait_handle, CVarRef dependencies) {
  assert(m_onGenMapCreateCallback.get());
  try {
    vm_call_user_func(
      m_onGenMapCreateCallback,
      make_packed_array(wait_handle, dependencies));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by GenMapWaitHandle::onCreate callback");
  }
}

void AsioSession::onGenVectorCreate(c_GenVectorWaitHandle* wait_handle, CVarRef dependencies) {
  assert(m_onGenVectorCreateCallback.get());
  try {
    vm_call_user_func(
      m_onGenVectorCreateCallback,
      make_packed_array(wait_handle, dependencies));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by GenVectorWaitHandle::onCreate callback");
  }
}

void AsioSession::onSetResultToRefCreate(c_SetResultToRefWaitHandle* wait_handle, CObjRef child) {
  assert(m_onSetResultToRefCreateCallback.get());
  try {
    vm_call_user_func(
      m_onSetResultToRefCreateCallback,
      make_packed_array(wait_handle, child));
  } catch (const Object& callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by SetResultToRefWaitHandle::onCreate callback");
  }
}

bool AsioSession::sleep_wh_greater::operator() (const c_SleepWaitHandle* x,
                                                const c_SleepWaitHandle* y) {
  return x->getWakeTime() > y->getWakeTime();
}

bool AsioSession::processSleepEvents() {
  if (m_sleepEventQueue.empty()) {
    return false;
  }

  bool woken = false;
  auto now = TimePoint::clock::now();

  while (!m_sleepEventQueue.empty()) {
    auto wh = m_sleepEventQueue.top();

    if (wh->getWakeTime() > now) {
      break;
    }
    woken = true;

    wh->process();
    decRefObj(wh);
    m_sleepEventQueue.pop();
  }

  return woken;
}

///////////////////////////////////////////////////////////////////////////////
}
