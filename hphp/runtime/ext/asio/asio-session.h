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

#ifndef incl_HPHP_EXT_ASIO_SESSION_H_
#define incl_HPHP_EXT_ASIO_SESSION_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event-queue.h"
#include "hphp/runtime/ext/ext_closure.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ActRec;
class c_WaitHandle;
class c_AwaitAllWaitHandle;
class c_GenArrayWaitHandle;
class c_GenMapWaitHandle;
class c_GenVectorWaitHandle;
class c_ConditionWaitHandle;
class c_ResumableWaitHandle;

struct AsioSession final {
  static void Init();
  static AsioSession* Get() { return s_current.get(); }

  void* operator new(size_t size) { return req::malloc(size); }
  void operator delete(void* ptr) { req::free(ptr); }

  // context
  void enterContext(ActRec* savedFP);
  void exitContext();

  bool isInContext() {
    return !m_contexts.empty();
  }

  AsioContext* getContext(context_idx_t ctx_idx) {
    assert(ctx_idx <= m_contexts.size());
    return ctx_idx ? m_contexts[ctx_idx - 1] : nullptr;
  }

  AsioContext* getCurrentContext() {
    assert(isInContext());
    return m_contexts.back();
  }

  context_idx_t getCurrentContextIdx() {
    assert(static_cast<context_idx_t>(m_contexts.size()) == m_contexts.size());
    return static_cast<context_idx_t>(m_contexts.size());
  }

  // External thread events.
  AsioExternalThreadEventQueue* getExternalThreadEventQueue() {
    return &m_externalThreadEventQueue;
  }

  // Meager time abstractions.
  typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

  // The latest time we will wait for an I/O operation to complete.  If this
  // time is exceeded, onIOWaitExit will throw after checking surprise.
  static TimePoint getLatestWakeTime() {
    auto now = std::chrono::steady_clock::now();
    auto info = ThreadInfo::s_threadInfo.getNoCheck();
    auto& data = info->m_reqInjectionData;
    if (!data.getTimeout()) {
      // Don't wait for over nine thousand hours.
      return now + std::chrono::hours(9000);
    }
    auto remaining = int64_t(data.getRemainingTime());
    return now + std::chrono::seconds(remaining);
  }

  // Sleep event management.
  void enqueueSleepEvent(c_SleepWaitHandle* h);
  bool processSleepEvents();
  TimePoint sleepWakeTime();

  // Abrupt interrupt exception.
  ObjectData* getAbruptInterruptException() {
    return m_abruptInterruptException.get();
  }
  bool hasAbruptInterruptException() { return !!m_abruptInterruptException; }
  void initAbruptInterruptException();

  // WaitHandle callbacks:
  void setOnIOWaitEnter(const Variant& callback);
  void setOnIOWaitExit(const Variant& callback);
  void setOnJoin(const Variant& callback);
  bool hasOnIOWaitEnter() { return !!m_onIOWaitEnter; }
  bool hasOnIOWaitExit() { return !!m_onIOWaitExit; }
  bool hasOnJoin() { return !!m_onJoin; }
  void onIOWaitEnter();
  void onIOWaitExit();
  void onJoin(c_WaitHandle* waitHandle);

  // ResumableWaitHandle callbacks:
  void setOnResumableCreate(const Variant& callback);
  void setOnResumableAwait(const Variant& callback);
  void setOnResumableSuccess(const Variant& callback);
  void setOnResumableFail(const Variant& callback);
  bool hasOnResumableCreate() { return !!m_onResumableCreate; }
  bool hasOnResumableAwait() { return !!m_onResumableAwait; }
  bool hasOnResumableSuccess() { return !!m_onResumableSuccess; }
  bool hasOnResumableFail() { return !!m_onResumableFail; }
  void onResumableCreate(c_ResumableWaitHandle*, c_WaitableWaitHandle* child);
  void onResumableAwait(c_ResumableWaitHandle*, c_WaitableWaitHandle* child);
  void onResumableSuccess(c_ResumableWaitHandle* cont, const Variant& result);
  void onResumableFail(c_ResumableWaitHandle* cont, const Object& exception);
  void updateEventHookState();

  // AwaitAllWaitHandle callbacks:
  void setOnAwaitAllCreate(const Variant& callback);
  bool hasOnAwaitAllCreate() { return !!m_onAwaitAllCreate; }
  void onAwaitAllCreate(c_AwaitAllWaitHandle* wh, const Variant& dependencies);

  // GenArrayWaitHandle callbacks:
  void setOnGenArrayCreate(const Variant& callback);
  bool hasOnGenArrayCreate() { return !!m_onGenArrayCreate; }
  void onGenArrayCreate(c_GenArrayWaitHandle* wh, const Variant& dependencies);

  // GenMapWaitHandle callbacks:
  void setOnGenMapCreate(const Variant& callback);
  bool hasOnGenMapCreate() { return !!m_onGenMapCreate; }
  void onGenMapCreate(c_GenMapWaitHandle* wh, const Variant& dependencies);

  // GenVectorWaitHandle callbacks:
  void setOnGenVectorCreate(const Variant& callback);
  bool hasOnGenVectorCreate() { return !!m_onGenVectorCreate; }
  void onGenVectorCreate(c_GenVectorWaitHandle* wh, const Variant& deps);

  // ConditionWaitHandle callbacks:
  void setOnConditionCreate(const Variant& callback);
  bool hasOnConditionCreate() { return !!m_onConditionCreate; }
  void onConditionCreate(c_ConditionWaitHandle* wh, c_WaitableWaitHandle*);

  // ExternalThreadEventWaitHandle callbacks:
  void setOnExternalThreadEventCreate(const Variant& callback);
  void setOnExternalThreadEventSuccess(const Variant& callback);
  void setOnExternalThreadEventFail(const Variant& callback);
  bool hasOnExternalThreadEventCreate() { return !!m_onExtThreadEventCreate; }
  bool hasOnExternalThreadEventSuccess() { return !!m_onExtThreadEventSuccess; }
  bool hasOnExternalThreadEventFail() { return !!m_onExtThreadEventFail; }
  void onExternalThreadEventCreate(c_ExternalThreadEventWaitHandle* waitHandle);
  void onExternalThreadEventSuccess(c_ExternalThreadEventWaitHandle* waitHandle,
                                    const Variant& result);
  void onExternalThreadEventFail(c_ExternalThreadEventWaitHandle* waitHandle,
                                 const Object& exception);

  // SleepWaitHandle callbacks:
  void setOnSleepCreate(const Variant& callback);
  void setOnSleepSuccess(const Variant& callback);
  bool hasOnSleepCreate() { return !!m_onSleepCreate; }
  bool hasOnSleepSuccess() { return !!m_onSleepSuccess; }
  void onSleepCreate(c_SleepWaitHandle* waitHandle);
  void onSleepSuccess(c_SleepWaitHandle* waitHandle);

private:
  AsioSession();

private:
  static DECLARE_THREAD_LOCAL_PROXY(AsioSession, false, s_current);
  req::vector<AsioContext*> m_contexts;
  req::vector<c_SleepWaitHandle*> m_sleepEvents;
  AsioExternalThreadEventQueue m_externalThreadEventQueue;

  Object m_abruptInterruptException;
  Object m_onIOWaitEnter;
  Object m_onIOWaitExit;
  Object m_onJoin;
  Object m_onResumableCreate;
  Object m_onResumableAwait;
  Object m_onResumableSuccess;
  Object m_onResumableFail;
  Object m_onAwaitAllCreate;
  Object m_onGenArrayCreate;
  Object m_onGenMapCreate;
  Object m_onGenVectorCreate;
  Object m_onConditionCreate;
  Object m_onExtThreadEventCreate;
  Object m_onExtThreadEventSuccess;
  Object m_onExtThreadEventFail;
  Object m_onSleepCreate;
  Object m_onSleepSuccess;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_SESSION_H_
