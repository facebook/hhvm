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

#include "hphp/runtime/ext/asio/asio-session.h"

#include <limits>
#include <algorithm>

#include <folly/String.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_condition-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_gen-array-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_gen-map-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_gen-vector-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
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
      SystemLib::throwInvalidArgumentExceptionObject(msg);
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
    SystemLib::throwInvalidOperationExceptionObject(
      "Unable to enter asio context: too many contexts open");
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
  m_abruptInterruptException =
    SystemLib::AllocInvalidOperationExceptionObject(
      "The request was abruptly interrupted."
    );
}

void AsioSession::setOnIOWaitEnter(const Variant& callback) {
  m_onIOWaitEnter = checkCallback(callback, "WaitHandle::onIOWaitEnter");
}

void AsioSession::setOnIOWaitExit(const Variant& callback) {
  m_onIOWaitExit = checkCallback(callback, "WaitHandle::onIOWaitExit");
}

void AsioSession::setOnJoin(const Variant& callback) {
  m_onJoin = checkCallback(callback, "WaitHandle::onJoin");
}

void AsioSession::onIOWaitEnter() {
  runCallback(m_onIOWaitEnter, empty_array(), "WaitHandle::onIOWaitEnter");
}

void AsioSession::onIOWaitExit() {
  runCallback(m_onIOWaitExit, empty_array(), "WaitHandle::onIOWaitExit");
}

void AsioSession::onJoin(c_WaitHandle* waitHandle) {
  runCallback(m_onJoin, make_packed_array(waitHandle), "WaitHandle::onJoin");
}

void AsioSession::setOnResumableCreate(const Variant& callback) {
  m_onResumableCreate = checkCallback(callback,"ResumableWaitHandle::onCreate");
  updateEventHookState();
}

void AsioSession::setOnResumableAwait(const Variant& callback) {
  m_onResumableAwait = checkCallback(callback, "ResumableWaitHandle::onAwait");
  updateEventHookState();
}

void AsioSession::setOnResumableSuccess(const Variant& callback) {
  m_onResumableSuccess = checkCallback(callback,
                                       "ResumableWaitHandle::onSuccess");
  updateEventHookState();
}

void AsioSession::setOnResumableFail(const Variant& callback) {
  m_onResumableFail = checkCallback(callback, "ResumableWaitHandle::onFail");
}

void AsioSession::onResumableCreate(
  c_ResumableWaitHandle* resumable,
  c_WaitableWaitHandle* child
) {
  runCallback(m_onResumableCreate,
    make_packed_array(resumable, child),
    "ResumableWaitHandle::onCreate"
  );
}

void AsioSession::onResumableAwait(
  c_ResumableWaitHandle* resumable,
  c_WaitableWaitHandle* child
) {
  runCallback(
    m_onResumableAwait,
    make_packed_array(resumable, child),
    "ResumableWaitHandle::onAwait"
  );
}

void AsioSession::onResumableSuccess(
  c_ResumableWaitHandle* resumable,
  const Variant& result
) {
  runCallback(
    m_onResumableSuccess,
    make_packed_array(resumable, result),
    "ResumableWaitHandle::onSuccess"
  );
}

void AsioSession::onResumableFail(
  c_ResumableWaitHandle* resumable,
  const Object& exception
) {
  runCallback(
    m_onResumableFail,
    make_packed_array(resumable, exception),
    "ResumableWaitHandle::onFail"
  );
}

void AsioSession::updateEventHookState() {
  if (hasOnResumableCreate() ||
      hasOnResumableAwait() ||
      hasOnResumableSuccess()) {
    EventHook::EnableAsync();
  } else {
    EventHook::DisableAsync();
  }
}

void AsioSession::setOnAwaitAllCreate(const Variant& callback) {
  m_onAwaitAllCreate = checkCallback(
    callback,
    "AwaitAllWaitHandle::onCreate"
  );
}

void AsioSession::onAwaitAllCreate(
  c_AwaitAllWaitHandle* waitHandle,
  const Variant &dependencies
) {
  runCallback(
    m_onAwaitAllCreate,
    make_packed_array(waitHandle, dependencies),
    "AwaitAllWaitHandle::onCreate"
  );
}

void AsioSession::setOnGenArrayCreate(const Variant& callback) {
  m_onGenArrayCreate = checkCallback(callback, "GenArrayWaitHandle::onCreate");
}

void AsioSession::onGenArrayCreate(
  c_GenArrayWaitHandle* waitHandle,
  const Variant& dependencies
) {
  runCallback(
    m_onGenArrayCreate,
    make_packed_array(waitHandle, dependencies),
    "GenArrayWaitHandle::onCreate"
  );
}

void AsioSession::setOnGenMapCreate(const Variant& callback) {
  m_onGenMapCreate = checkCallback(callback, "GenMapWaitHandle::onCreate");
}

void AsioSession::onGenMapCreate(
  c_GenMapWaitHandle* waitHandle,
  const Variant& dependencies
) {
  runCallback(m_onGenMapCreate,
    make_packed_array(waitHandle, dependencies),
    "GenMapWaitHandle::onCreate"
  );
}

void AsioSession::setOnGenVectorCreate(const Variant& callback) {
  m_onGenVectorCreate= checkCallback(callback, "GenVectorWaitHandle::onCreate");
}

void AsioSession::onGenVectorCreate(
  c_GenVectorWaitHandle* waitHandle,
  const Variant& dependencies
) {
  runCallback(
    m_onGenVectorCreate,
    make_packed_array(waitHandle, dependencies),
    "GenVectorWaitHandle::onCreate"
  );
}

void AsioSession::setOnConditionCreate(const Variant& callback) {
  m_onConditionCreate = checkCallback(callback,"ConditionWaitHandle::onCreate");
}

void AsioSession::onConditionCreate(
  c_ConditionWaitHandle* waitHandle,
  c_WaitableWaitHandle* child
) {
  runCallback(
    m_onConditionCreate,
    make_packed_array(waitHandle, child),
    "ConditionWaitHandle::onCreate"
  );
}

void AsioSession::setOnExternalThreadEventCreate(
  const Variant& callback
) {
  m_onExtThreadEventCreate = checkCallback(
    callback,
    "ExternalThreadEventWaitHandle::onCreate"
  );
}

void AsioSession::setOnExternalThreadEventSuccess(
  const Variant& callback
) {
  m_onExtThreadEventSuccess = checkCallback(
    callback,
    "ExternalThreadEventWaitHandle::onSuccess"
  );
}

void AsioSession::setOnExternalThreadEventFail(
  const Variant& callback
) {
  m_onExtThreadEventFail = checkCallback(
    callback,
    "ExternalThreadEventWaitHandle::onFail"
  );
}

void AsioSession::onExternalThreadEventCreate(
  c_ExternalThreadEventWaitHandle* waitHandle
) {
  runCallback(
    m_onExtThreadEventCreate,
    make_packed_array(waitHandle),
    "ExternalThreadEventWaitHandle::onCreate"
  );
}

void AsioSession::onExternalThreadEventSuccess(
  c_ExternalThreadEventWaitHandle* waitHandle,
  const Variant& result
) {
  runCallback(
    m_onExtThreadEventSuccess,
    make_packed_array(waitHandle, result),
    "ExternalThreadEventWaitHandle::onSuccess"
  );
}

void AsioSession::onExternalThreadEventFail(
  c_ExternalThreadEventWaitHandle* waitHandle,
  const Object& exception
) {
  runCallback(
    m_onExtThreadEventFail,
    make_packed_array(waitHandle, exception),
    "ExternalThreadEventWaitHandle::onFail"
  );
}

void AsioSession::setOnSleepCreate(const Variant& callback) {
  m_onSleepCreate = checkCallback(
    callback,
    "SleepWaitHandle::onCreate"
  );
}

void AsioSession::setOnSleepSuccess(const Variant& callback) {
  m_onSleepSuccess = checkCallback(
    callback,
    "SleepWaitHandle::onSuccess"
  );
}

void AsioSession::onSleepCreate(c_SleepWaitHandle* waitHandle) {
  runCallback(
    m_onSleepCreate,
    make_packed_array(waitHandle),
    "SleepWaitHandle::onCreate"
  );
}

void AsioSession::onSleepSuccess(c_SleepWaitHandle* waitHandle) {
  runCallback(
    m_onSleepSuccess,
    make_packed_array(waitHandle),
    "SleepWaitHandle::onSuccess"
  );
}

inline
bool sleep_compare(const c_SleepWaitHandle* x, const c_SleepWaitHandle* y) {
  return x->getWakeTime() > y->getWakeTime();
}

void AsioSession::enqueueSleepEvent(c_SleepWaitHandle* h) {
  m_sleepEvents.push_back(h);
  std::push_heap(m_sleepEvents.begin(), m_sleepEvents.end(), sleep_compare);
}

bool AsioSession::processSleepEvents() {
  if (m_sleepEvents.empty()) {
    return false;
  }

  bool woken = false;
  auto now = TimePoint::clock::now();

  while (!m_sleepEvents.empty()) {
    auto wh = m_sleepEvents.front();
    if (wh->getWakeTime() > now) {
      break;
    }
    woken = true;
    wh->process();
    decRefObj(wh);
    std::pop_heap(m_sleepEvents.begin(), m_sleepEvents.end(), sleep_compare);
    m_sleepEvents.pop_back();
  }

  return woken;
}

AsioSession::TimePoint AsioSession::sleepWakeTime() {
  return m_sleepEvents.empty() ? getLatestWakeTime() :
         m_sleepEvents.front()->getWakeTime();
}

///////////////////////////////////////////////////////////////////////////////
}
