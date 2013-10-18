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

#ifndef incl_HPHP_EXT_ASIO_SESSION_H_
#define incl_HPHP_EXT_ASIO_SESSION_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_external_thread_event_queue.h"
#include "hphp/runtime/ext/ext_closure.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FORWARD_DECLARE_CLASS(WaitHandle);
FORWARD_DECLARE_CLASS(GenArrayWaitHandle);
FORWARD_DECLARE_CLASS(GenMapWaitHandle);
FORWARD_DECLARE_CLASS(GenVectorWaitHandle);
FORWARD_DECLARE_CLASS(SetResultToRefWaitHandle);
FORWARD_DECLARE_CLASS(AsyncFunctionWaitHandle);

class AsioSession {
  public:
    static void Init();
    static AsioSession* Get() { return s_current.get(); }

    void* operator new(size_t size) { return smart_malloc(size); }
    void operator delete(void* ptr) { smart_free(ptr); }

    // context
    void enterContext();
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

    c_AsyncFunctionWaitHandle* getCurrentWaitHandle() {
      assert(!isInContext() || getCurrentContext()->isRunning());
      return isInContext() ? getCurrentContext()->getCurrent() : nullptr;
    }

    uint16_t getCurrentWaitHandleDepth();

    // External thread events.
    AsioExternalThreadEventQueue* getExternalThreadEventQueue() {
      return &m_externalThreadEventQueue;
    }

    // Meager time abstractions.
    typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

    static TimePoint getLatestWakeTime() {
      // Don't wait for over nine thousand hours.
      return std::chrono::steady_clock::now() + std::chrono::hours(9000);
    }

    // Sleep event management.
    struct sleep_wh_greater {
      bool operator() (const c_SleepWaitHandle* x, const c_SleepWaitHandle* y);
    };

    smart::priority_queue<c_SleepWaitHandle*, sleep_wh_greater>&
    getSleepEventQueue() {
      return m_sleepEventQueue;
    }

    bool processSleepEvents();

    // Abrupt interrupt exception.
    CObjRef getAbruptInterruptException() {
      return m_abruptInterruptException;
    }

    bool hasAbruptInterruptException() {
      return m_abruptInterruptException.get();
    }

    void initAbruptInterruptException();

    // AsyncFunctionWaitHandle callbacks:
    void setOnAsyncFunctionCreateCallback(ObjectData* on_start) {
      assert(!on_start || on_start->instanceof(c_Closure::classof()));
      m_onAsyncFunctionCreateCallback = on_start;
    }
    void setOnAsyncFunctionAwaitCallback(ObjectData* on_await) {
      assert(!on_await || on_await->instanceof(c_Closure::classof()));
      m_onAsyncFunctionAwaitCallback = on_await;
    }
    void setOnAsyncFunctionSuccessCallback(ObjectData* on_success) {
      assert(!on_success || on_success->instanceof(c_Closure::classof()));
      m_onAsyncFunctionSuccessCallback = on_success;
    }
    void setOnAsyncFunctionFailCallback(ObjectData* on_fail) {
      assert(!on_fail || on_fail->instanceof(c_Closure::classof()));
      m_onAsyncFunctionFailCallback = on_fail;
    }
    bool hasOnAsyncFunctionCreateCallback() { return m_onAsyncFunctionCreateCallback.get(); }
    bool hasOnAsyncFunctionAwaitCallback() { return m_onAsyncFunctionAwaitCallback.get(); }
    bool hasOnAsyncFunctionSuccessCallback() { return m_onAsyncFunctionSuccessCallback.get(); }
    bool hasOnAsyncFunctionFailCallback() { return m_onAsyncFunctionFailCallback.get(); }
    void onAsyncFunctionCreate(c_AsyncFunctionWaitHandle* cont);
    void onAsyncFunctionAwait(c_AsyncFunctionWaitHandle* cont, c_WaitHandle* child);
    void onAsyncFunctionSuccess(c_AsyncFunctionWaitHandle* cont, CVarRef result);
    void onAsyncFunctionFail(c_AsyncFunctionWaitHandle* cont, CObjRef exception);

    // WaitHandle callbacks:
    void setOnJoinCallback(ObjectData* on_join) {
      assert(!on_join || on_join->instanceof(c_Closure::classof()));
      m_onJoinCallback = on_join;
    }
    bool hasOnJoinCallback() { return m_onJoinCallback.get(); }
    void onJoin(c_WaitHandle* wait_handle);

    // GenArrayWaitHandle callbacks:
    void setOnGenArrayCreateCallback(ObjectData* on_create) {
      assert(!on_create || on_create->instanceof(c_Closure::classof()));
      m_onGenArrayCreateCallback = on_create;
    }
    bool hasOnGenArrayCreateCallback() { return m_onGenArrayCreateCallback.get(); }
    void onGenArrayCreate(c_GenArrayWaitHandle* wait_handle, CVarRef dependencies);

    // GenMapWaitHandle callbacks:
    void setOnGenMapCreateCallback(ObjectData* on_create) {
      assert(!on_create || on_create->instanceof(c_Closure::classof()));
      m_onGenMapCreateCallback = on_create;
    }
    bool hasOnGenMapCreateCallback() { return m_onGenMapCreateCallback.get(); }
    void onGenMapCreate(c_GenMapWaitHandle* wait_handle, CVarRef dependencies);

    // GenVectorWaitHandle callbacks:
    void setOnGenVectorCreateCallback(ObjectData* on_create) {
      assert(!on_create || on_create->instanceof(c_Closure::classof()));
      m_onGenVectorCreateCallback = on_create;
    }
    bool hasOnGenVectorCreateCallback() { return m_onGenVectorCreateCallback.get(); }
    void onGenVectorCreate(c_GenVectorWaitHandle* wait_handle, CVarRef dependencies);

    // SetResultToRefWaitHandle callbacks:
    void setOnSetResultToRefCreateCallback(ObjectData* on_create) {
      assert(!on_create || on_create->instanceof(c_Closure::classof()));
      m_onSetResultToRefCreateCallback = on_create;
    }
    bool hasOnSetResultToRefCreateCallback() { return m_onSetResultToRefCreateCallback.get(); }
    void onSetResultToRefCreate(c_SetResultToRefWaitHandle* wait_handle, CObjRef child);

  private:
    static DECLARE_THREAD_LOCAL_PROXY(AsioSession, false, s_current);

    AsioSession();

    smart::vector<AsioContext*> m_contexts;
    smart::priority_queue<c_SleepWaitHandle*,
                          sleep_wh_greater> m_sleepEventQueue;
    AsioExternalThreadEventQueue m_externalThreadEventQueue;

    Object m_abruptInterruptException;

    Object m_onAsyncFunctionCreateCallback;
    Object m_onAsyncFunctionAwaitCallback;
    Object m_onAsyncFunctionSuccessCallback;
    Object m_onAsyncFunctionFailCallback;
    Object m_onGenArrayCreateCallback;
    Object m_onGenMapCreateCallback;
    Object m_onGenVectorCreateCallback;
    Object m_onSetResultToRefCreateCallback;
    Object m_onJoinCallback;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_SESSION_H_
