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

#include "hphp/runtime/ext/asio/asio-session.h"

#include <limits>
#include <algorithm>

#include <folly/String.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/recorder.h"
#include "hphp/runtime/base/replayer.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_concurrent-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_condition-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/core/ext_core_closure.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

RDS_LOCAL_NO_CHECK(AsioSession*, AsioSession::s_current)(nullptr);

namespace {
  const context_idx_t MAX_CONTEXT_DEPTH =
    std::numeric_limits<context_idx_t>::max();

  Object checkCallback(const Variant& callback, char* name) {
    if (!callback.isNull() &&
        (!callback.isObject() ||
         !callback.getObjectData()->instanceof(c_Closure::classof()))) {
      auto msg = folly::format(
        "Unable to set {}: callback not a closure",
        name
      ).str();
      SystemLib::throwInvalidArgumentExceptionObject(msg);
    }
    return Object{callback.getObjectDataOrNull()};
  }

  void runCallback(const Object& function, const Array& params, char* name) {
    assertx(function.get());
    try {
      vm_call_user_func(function, params);
    } catch (const Object&) {
      try {
        raise_warning("[asio] Ignoring exception thrown by %s callback", name);
      } catch (const Object&) {
        // Swallow the exception. Callers are not designed to deal with
        // PHP exceptions.
      }
    }
  }
}

void AsioSession::Init() {
  *s_current = req::make_raw<AsioSession>();
}

AsioSession::AsioSession()
    : m_contexts(), m_externalThreadEventQueue() {
}

void AsioSession::enterContext(ActRec* savedFP) {
  if (UNLIKELY(getCurrentContextIdx() >= MAX_CONTEXT_DEPTH)) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Unable to enter asio context: too many contexts open");
  }

  m_contexts.push_back(req::make_raw<AsioContext>(savedFP));

  assertx(static_cast<context_idx_t>(m_contexts.size()) == m_contexts.size());
  assertx(isInContext());
}

void AsioSession::exitContext() {
  assertx(isInContext());

  m_contexts.back()->exit(m_contexts.size());
  req::destroy_raw(m_contexts.back());
  m_contexts.pop_back();
}

void AsioSession::initAbruptInterruptException() {
  assertx(!hasAbruptInterruptException());
  m_abruptInterruptException =
    SystemLib::AllocInvalidOperationExceptionObject(
      "The request was abruptly interrupted."
    );
}

void AsioSession::setOnIOWaitEnter(const Variant& callback) {
  m_onIOWaitEnter = checkCallback(callback, "Awaitable::onIOWaitEnter");
}

void AsioSession::setOnIOWaitExit(const Variant& callback) {
  m_onIOWaitExit = checkCallback(callback, "Awaitable::onIOWaitExit");
}

void AsioSession::setOnJoin(const Variant& callback) {
  m_onJoin = checkCallback(callback, "Awaitable::onJoin");
}

void AsioSession::onIOWaitEnter() {
  runCallback(m_onIOWaitEnter, empty_vec_array(), "Awaitable::onIOWaitEnter");
}

void AsioSession::onIOWaitExit(c_WaitableWaitHandle* waitHandle) {
  runCallback(
    m_onIOWaitExit,
    make_vec_array(waitHandle),
    "Awaitable::onIOWaitExit"
  );
}

void AsioSession::onJoin(c_WaitableWaitHandle* waitHandle) {
  runCallback(m_onJoin, make_vec_array(waitHandle), "Awaitable::onJoin");
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
    make_vec_array(resumable, child),
    "ResumableWaitHandle::onCreate"
  );
}

void AsioSession::onResumableAwait(
  c_ResumableWaitHandle* resumable,
  c_WaitableWaitHandle* child
) {
  runCallback(
    m_onResumableAwait,
    make_vec_array(resumable, child),
    "ResumableWaitHandle::onAwait"
  );
}

void AsioSession::onResumableSuccess(
  c_ResumableWaitHandle* resumable,
  const Variant& result
) {
  runCallback(
    m_onResumableSuccess,
    make_vec_array(resumable, result),
    "ResumableWaitHandle::onSuccess"
  );
}

void AsioSession::onResumableFail(
  c_ResumableWaitHandle* resumable,
  const Object& exception
) {
  runCallback(
    m_onResumableFail,
    make_vec_array(resumable, exception),
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
  Array&& dependencies
) {
  runCallback(
    m_onAwaitAllCreate,
    make_vec_array(waitHandle, std::move(dependencies)),
    "AwaitAllWaitHandle::onCreate"
  );
}

void AsioSession::setOnConcurrentCreate(const Variant& callback) {
  m_onConcurrentCreate = checkCallback(
    callback,
    "ConcurrentWaitHandle::onCreate"
  );
}

void AsioSession::onConcurrentCreate(
  c_ConcurrentWaitHandle* waitHandle,
  Array&& dependencies
) {
  runCallback(
    m_onConcurrentCreate,
    make_vec_array(waitHandle, std::move(dependencies)),
    "ConcurrentWaitHandle::onCreate"
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
    make_vec_array(waitHandle, child),
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
    make_vec_array(waitHandle),
    "ExternalThreadEventWaitHandle::onCreate"
  );
}

void AsioSession::onExternalThreadEventSuccess(
  c_ExternalThreadEventWaitHandle* waitHandle,
  const Variant& result,
  int64_t finish_time
) {
  runCallback(
    m_onExtThreadEventSuccess,
    make_vec_array(waitHandle, result, finish_time),
    "ExternalThreadEventWaitHandle::onSuccess"
  );
}

void AsioSession::onExternalThreadEventFail(
  c_ExternalThreadEventWaitHandle* waitHandle,
  const Object& exception,
  int64_t finish_time
) {
  runCallback(
    m_onExtThreadEventFail,
    make_vec_array(waitHandle, exception, finish_time),
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
    make_vec_array(waitHandle),
    "SleepWaitHandle::onCreate"
  );
}

void AsioSession::onSleepSuccess(
  c_SleepWaitHandle* waitHandle,
  int64_t finish_time
) {
  runCallback(
    m_onSleepSuccess,
    make_vec_array(waitHandle, finish_time),
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

  if (UNLIKELY(RO::EvalRecordReplay)) {
    if (RO::EvalRecordSampleRate) {
      Recorder::onProcessSleepEvents(now.time_since_epoch().count());
    } else if (RO::EvalReplay) {
      now = TimePoint{TimePoint::duration{Replayer::onProcessSleepEvents()}};
    }
  }

  while (!m_sleepEvents.empty()) {
    auto wh = m_sleepEvents.front();
    if (wh->getWakeTime() > now) {
      break;
    }
    if (wh->process()) {
      woken = true;
    }
    decRefObj(wh);
    std::pop_heap(m_sleepEvents.begin(), m_sleepEvents.end(), sleep_compare);
    m_sleepEvents.pop_back();
  }

  return woken;
}

AsioSession::TimePoint AsioSession::sleepWakeTime() {
  auto const timeout = getLatestWakeTime();
  return m_sleepEvents.empty() ? timeout :
         min(timeout, m_sleepEvents.front()->getWakeTime());
}

c_SleepWaitHandle* AsioSession::nextSleepEvent() {
  if (m_sleepEvents.empty()) {
    return nullptr;
  }
  return m_sleepEvents.front();
}

///////////////////////////////////////////////////////////////////////////////
}
