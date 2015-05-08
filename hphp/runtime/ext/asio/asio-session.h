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

  void* operator new(size_t size) { return smart_malloc(size); }
  void operator delete(void* ptr) { smart_free(ptr); }

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
  void setOnIOWaitEnterCallback(const Variant& callback);
  void setOnIOWaitExitCallback(const Variant& callback);
  void setOnJoinCallback(const Variant& callback);
  bool hasOnIOWaitEnterCallback() { return !m_onIOWaitEnterCallback.isNull(); }
  bool hasOnIOWaitExitCallback() { return !m_onIOWaitExitCallback.isNull(); }
  bool hasOnJoinCallback() { return !m_onJoinCallback.isNull(); }
  void onIOWaitEnter();
  void onIOWaitExit();
  void onJoin(c_WaitHandle* waitHandle);

  // ResumableWaitHandle callbacks:
  void setOnResumableCreateCallback(const Variant& callback);
  void setOnResumableAwaitCallback(const Variant& callback);
  void setOnResumableSuccessCallback(const Variant& callback);
  void setOnResumableFailCallback(const Variant& callback);
  bool hasOnResumableCreateCallback() { return !!m_onResumableCreateCallback; }
  bool hasOnResumableAwaitCallback() { return !!m_onResumableAwaitCallback; }
  bool hasOnResumableSuccessCallback() { return !!m_onResumableSuccessCallback;}
  bool hasOnResumableFailCallback() { return !!m_onResumableFailCallback; }
  void onResumableCreate(c_ResumableWaitHandle*, c_WaitableWaitHandle* child);
  void onResumableAwait(c_ResumableWaitHandle*, c_WaitableWaitHandle* child);
  void onResumableSuccess(c_ResumableWaitHandle* cont, const Variant& result);
  void onResumableFail(c_ResumableWaitHandle* cont, const Object& exception);
  void updateEventHookState();

  // AwaitAllWaitHandle callbacks:
  void setOnAwaitAllCreateCallback(const Variant& callback);
  bool hasOnAwaitAllCreateCallback() { return !!m_onAwaitAllCreateCallback; }
  void onAwaitAllCreate(c_AwaitAllWaitHandle* wh, const Variant& dependencies);

  // GenArrayWaitHandle callbacks:
  void setOnGenArrayCreateCallback(const Variant& callback);
  bool hasOnGenArrayCreateCallback() { return !!m_onGenArrayCreateCallback; }
  void onGenArrayCreate(c_GenArrayWaitHandle* wh, const Variant& dependencies);

  // GenMapWaitHandle callbacks:
  void setOnGenMapCreateCallback(const Variant& callback);
  bool hasOnGenMapCreateCallback() { return !!m_onGenMapCreateCallback; }
  void onGenMapCreate(c_GenMapWaitHandle* wh, const Variant& dependencies);

  // GenVectorWaitHandle callbacks:
  void setOnGenVectorCreateCallback(const Variant& callback);
  bool hasOnGenVectorCreateCallback() { return !!m_onGenVectorCreateCallback; }
  void onGenVectorCreate(c_GenVectorWaitHandle* wh, const Variant& deps);

  // ConditionWaitHandle callbacks:
  void setOnConditionCreateCallback(const Variant& callback);
  bool hasOnConditionCreateCallback() { return !!m_onConditionCreateCallback; }
  void onConditionCreate(c_ConditionWaitHandle* wh, c_WaitableWaitHandle*);

  // ExternalThreadEventWaitHandle callbacks:
  void setOnExternalThreadEventCreateCallback(const Variant& callback);
  void setOnExternalThreadEventSuccessCallback(const Variant& callback);
  void setOnExternalThreadEventFailCallback(const Variant& callback);
  bool hasOnExternalThreadEventCreateCallback() {
    return !m_onExternalThreadEventCreateCallback.isNull();
  }
  bool hasOnExternalThreadEventSuccessCallback() {
    return !m_onExternalThreadEventSuccessCallback.isNull();
  }
  bool hasOnExternalThreadEventFailCallback() {
    return !m_onExternalThreadEventFailCallback.isNull();
  }
  void onExternalThreadEventCreate(c_ExternalThreadEventWaitHandle* waitHandle);
  void onExternalThreadEventSuccess(c_ExternalThreadEventWaitHandle* waitHandle,
                                    const Variant& result);
  void onExternalThreadEventFail(c_ExternalThreadEventWaitHandle* waitHandle,
                                 const Object& exception);

  // SleepWaitHandle callbacks:
  void setOnSleepCreateCallback(const Variant& callback);
  void setOnSleepSuccessCallback(const Variant& callback);
  bool hasOnSleepCreateCallback() { return !!m_onSleepCreateCallback; }
  bool hasOnSleepSuccessCallback() { return !!m_onSleepSuccessCallback; }
  void onSleepCreate(c_SleepWaitHandle* waitHandle);
  void onSleepSuccess(c_SleepWaitHandle* waitHandle);

private:
  AsioSession();

private:
  static DECLARE_THREAD_LOCAL_PROXY(AsioSession, false, s_current);
  smart::vector<AsioContext*> m_contexts;
  smart::vector<c_SleepWaitHandle*> m_sleepEvents;
  AsioExternalThreadEventQueue m_externalThreadEventQueue;

  Object m_abruptInterruptException;
  Object m_onIOWaitEnterCallback;
  Object m_onIOWaitExitCallback;
  Object m_onJoinCallback;
  Object m_onResumableCreateCallback;
  Object m_onResumableAwaitCallback;
  Object m_onResumableSuccessCallback;
  Object m_onResumableFailCallback;
  Object m_onAwaitAllCreateCallback;
  Object m_onGenArrayCreateCallback;
  Object m_onGenMapCreateCallback;
  Object m_onGenVectorCreateCallback;
  Object m_onConditionCreateCallback;
  Object m_onExternalThreadEventCreateCallback;
  Object m_onExternalThreadEventSuccessCallback;
  Object m_onExternalThreadEventFailCallback;
  Object m_onSleepCreateCallback;
  Object m_onSleepSuccessCallback;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_SESSION_H_
