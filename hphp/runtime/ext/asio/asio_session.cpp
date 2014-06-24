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

#include "hphp/runtime/ext/asio/asio_session.h"

#include <limits>

#include <folly/String.h>

#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/await_all_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_array_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_map_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_vector_wait_handle.h"
#include "hphp/runtime/ext/asio/sleep_wait_handle.h"
#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_PROXY(AsioSession, false, AsioSession::s_current);

namespace {
  const context_idx_t MAX_CONTEXT_DEPTH =
    std::numeric_limits<context_idx_t>::max();

  ObjectData* checkCallback(const Variant& callback, char* name) {
    if (!callback.isNull() &&
        (!callback.isObject() ||
         !callback.getObjectData()->instanceof(c_Closure::classof()))) {
      auto msg = folly::format(
        "Unable to set {}: callback not a closure",
        name
      ).str();
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(msg));
      throw e;
    }
    return callback.getObjectDataOrNull();
  }

  void runCallback(const Object& function, const Array& params, char* name) {
    assert(function.get());
    try {
      vm_call_user_func(function, params);
    } catch (const Object& exception) {
      try {
        raise_warning("[asio] Ignoring exception thrown by %s callback", name);
      } catch (const Object& exception) {
        // Swallow the exception. Callers are not designed to deal with
        // PHP exceptions.
      }
    }
  }
}

void AsioSession::Init() {
  s_current.set(new AsioSession());
}

AsioSession::AsioSession()
    : m_contexts(), m_externalThreadEventQueue() {
}

void AsioSession::enterContext(ActRec* savedFP) {
  if (UNLIKELY(getCurrentContextIdx() >= MAX_CONTEXT_DEPTH)) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      "Unable to enter asio context: too many contexts open"));
    throw e;
  }

  m_contexts.push_back(new AsioContext(savedFP));

  assert(static_cast<context_idx_t>(m_contexts.size()) == m_contexts.size());
  assert(isInContext());
}

void AsioSession::exitContext() {
  assert(isInContext());

  m_contexts.back()->exit(m_contexts.size());
  delete m_contexts.back();
  m_contexts.pop_back();
}

void AsioSession::initAbruptInterruptException() {
  assert(!hasAbruptInterruptException());
  m_abruptInterruptException = SystemLib::AllocInvalidOperationExceptionObject(
    "The request was abruptly interrupted.");
}

void AsioSession::setOnIOWaitEnterCallback(const Variant& callback) {
  m_onIOWaitEnterCallback = checkCallback(
    callback,
    "WaitHandle::onIOWaitEnter"
  );
}

void AsioSession::setOnIOWaitExitCallback(const Variant& callback) {
  m_onIOWaitExitCallback = checkCallback(
    callback,
    "WaitHandle::onIOWaitExit"
  );
}

void AsioSession::setOnJoinCallback(const Variant& callback) {
  m_onJoinCallback = checkCallback(callback, "WaitHandle::onJoin");
}

void AsioSession::onIOWaitEnter() {
  runCallback(
    m_onIOWaitEnterCallback,
    empty_array(),
    "WaitHandle::onIOWaitEnter"
  );
}

void AsioSession::onIOWaitExit() {
  runCallback(
    m_onIOWaitExitCallback,
    empty_array(),
    "WaitHandle::onIOWaitExit"
  );
}

void AsioSession::onJoin(c_WaitHandle* waitHandle) {
  runCallback(
    m_onJoinCallback,
    make_packed_array(waitHandle),
    "WaitHandle::onJoin"
  );
}

void AsioSession::setOnResumableCreateCallback(const Variant& callback) {
  m_onResumableCreateCallback = checkCallback(
    callback,
    "ResumableWaitHandle::onCreate"
  );
  updateEventHookState();
}

void AsioSession::setOnResumableAwaitCallback(const Variant& callback) {
  m_onResumableAwaitCallback = checkCallback(
    callback,
    "ResumableWaitHandle::onAwait"
  );
  updateEventHookState();
}

void AsioSession::setOnResumableSuccessCallback(const Variant& callback) {
  m_onResumableSuccessCallback = checkCallback(
    callback,
    "ResumableWaitHandle::onSuccess"
  );
  updateEventHookState();
}

void AsioSession::setOnResumableFailCallback(const Variant& callback) {
  m_onResumableFailCallback = checkCallback(
    callback,
    "ResumableWaitHandle::onFail"
  );
}

void AsioSession::onResumableCreate(
  c_ResumableWaitHandle* resumable,
  c_WaitableWaitHandle* child
) {
  runCallback(
    m_onResumableCreateCallback,
    make_packed_array(resumable, child),
    "ResumableWaitHandle::onCreate"
  );
}

void AsioSession::onResumableAwait(
  c_ResumableWaitHandle* resumable,
  c_WaitableWaitHandle* child
) {
  runCallback(
    m_onResumableAwaitCallback,
    make_packed_array(resumable, child),
    "ResumableWaitHandle::onAwait"
  );
}

void AsioSession::onResumableSuccess(
  c_ResumableWaitHandle* resumable,
  const Variant& result
) {
  runCallback(
    m_onResumableSuccessCallback,
    make_packed_array(resumable, result),
    "ResumableWaitHandle::onSuccess"
  );
}

void AsioSession::onResumableFail(
  c_ResumableWaitHandle* resumable,
  const Object& exception
) {
  runCallback(
    m_onResumableFailCallback,
    make_packed_array(resumable, exception),
    "ResumableWaitHandle::onFail"
  );
}

void AsioSession::updateEventHookState() {
  if (hasOnResumableCreateCallback() ||
      hasOnResumableAwaitCallback() ||
      hasOnResumableSuccessCallback()) {
    EventHook::EnableAsync();
  } else {
    EventHook::DisableAsync();
  }
}

void AsioSession::setOnAwaitAllCreateCallback(const Variant& callback) {
  m_onAwaitAllCreateCallback = checkCallback(
    callback,
    "AwaitAllWaitHandle::onCreate"
  );
}

void AsioSession::onAwaitAllCreate(
  c_AwaitAllWaitHandle* waitHandle,
  const Variant &dependencies
) {
  runCallback(
    m_onAwaitAllCreateCallback,
    make_packed_array(waitHandle, dependencies),
    "AwaitAllWaitHandle::onCreate"
  );
}

void AsioSession::setOnGenArrayCreateCallback(const Variant& callback) {
  m_onGenArrayCreateCallback = checkCallback(
    callback,
    "GenArrayWaitHandle::onCreate"
  );
}

void AsioSession::onGenArrayCreate(
  c_GenArrayWaitHandle* waitHandle,
  const Variant& dependencies
) {
  runCallback(
    m_onGenArrayCreateCallback,
    make_packed_array(waitHandle, dependencies),
    "GenArrayWaitHandle::onCreate"
  );
}

void AsioSession::setOnGenMapCreateCallback(const Variant& callback) {
  m_onGenMapCreateCallback = checkCallback(
    callback,
    "GenMapWaitHandle::onCreate"
  );
}

void AsioSession::onGenMapCreate(
  c_GenMapWaitHandle* waitHandle,
  const Variant& dependencies
) {
  runCallback(
    m_onGenMapCreateCallback,
    make_packed_array(waitHandle, dependencies),
    "GenMapWaitHandle::onCreate"
  );
}

void AsioSession::setOnGenVectorCreateCallback(const Variant& callback) {
  m_onGenVectorCreateCallback = checkCallback(
    callback,
    "GenVectorWaitHandle::onCreate"
  );
}

void AsioSession::onGenVectorCreate(
  c_GenVectorWaitHandle* waitHandle,
  const Variant& dependencies
) {
  runCallback(
    m_onGenVectorCreateCallback,
    make_packed_array(waitHandle, dependencies),
    "GenVectorWaitHandle::onCreate"
  );
}

void AsioSession::setOnExternalThreadEventCreateCallback(
  const Variant& callback
) {
  m_onExternalThreadEventCreateCallback = checkCallback(
    callback,
    "ExternalThreadEventWaitHandle::onCreate"
  );
}

void AsioSession::setOnExternalThreadEventSuccessCallback(
  const Variant& callback
) {
  m_onExternalThreadEventSuccessCallback = checkCallback(
    callback,
    "ExternalThreadEventWaitHandle::onSuccess"
  );
}

void AsioSession::setOnExternalThreadEventFailCallback(
  const Variant& callback
) {
  m_onExternalThreadEventFailCallback = checkCallback(
    callback,
    "ExternalThreadEventWaitHandle::onFail"
  );
}

void AsioSession::onExternalThreadEventCreate(
  c_ExternalThreadEventWaitHandle* waitHandle
) {
  runCallback(
    m_onExternalThreadEventCreateCallback,
    make_packed_array(waitHandle),
    "ExternalThreadEventWaitHandle::onCreate"
  );
}

void AsioSession::onExternalThreadEventSuccess(
  c_ExternalThreadEventWaitHandle* waitHandle,
  const Variant& result
) {
  runCallback(
    m_onExternalThreadEventSuccessCallback,
    make_packed_array(waitHandle, result),
    "ExternalThreadEventWaitHandle::onSuccess"
  );
}

void AsioSession::onExternalThreadEventFail(
  c_ExternalThreadEventWaitHandle* waitHandle,
  const Object& exception
) {
  runCallback(
    m_onExternalThreadEventFailCallback,
    make_packed_array(waitHandle, exception),
    "ExternalThreadEventWaitHandle::onFail"
  );
}

void AsioSession::setOnSleepCreateCallback(const Variant& callback) {
  m_onSleepCreateCallback = checkCallback(
    callback,
    "SleepWaitHandle::onCreate"
  );
}

void AsioSession::setOnSleepSuccessCallback(const Variant& callback) {
  m_onSleepSuccessCallback = checkCallback(
    callback,
    "SleepWaitHandle::onSuccess"
  );
}

void AsioSession::onSleepCreate(c_SleepWaitHandle* waitHandle) {
  runCallback(
    m_onSleepCreateCallback,
    make_packed_array(waitHandle),
    "SleepWaitHandle::onCreate"
  );
}

void AsioSession::onSleepSuccess(c_SleepWaitHandle* waitHandle) {
  runCallback(
    m_onSleepSuccessCallback,
    make_packed_array(waitHandle),
    "SleepWaitHandle::onSuccess"
  );
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
