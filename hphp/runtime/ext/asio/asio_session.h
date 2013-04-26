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

#ifndef incl_HPHP_EXT_ASIO_SESSION_H_
#define incl_HPHP_EXT_ASIO_SESSION_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <runtime/base/base_includes.h>
#include <runtime/ext/asio/asio_context.h>
#include <runtime/ext/ext_closure.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FORWARD_DECLARE_CLASS_BUILTIN(WaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(GenArrayWaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(SetResultToRefWaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(ContinuationWaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(ExternalThreadEventWaitHandle);

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

    c_ContinuationWaitHandle* getCurrentWaitHandle() {
      assert(!isInContext() || getCurrentContext()->isRunning());
      return isInContext() ? getCurrentContext()->getCurrent() : nullptr;
    }

    uint16_t getCurrentWaitHandleDepth();

    // external thread event
    c_ExternalThreadEventWaitHandle* getReadyExternalThreadEvents() {
      auto ready = m_readyExternalThreadEvents.exchange(nullptr);
      assert(ready != k_waitingForExternalThreadEvents);
      return ready;
    }

    c_ExternalThreadEventWaitHandle* waitForExternalThreadEvents();
    void enqueueExternalThreadEvent(c_ExternalThreadEventWaitHandle* wait_handle);

    // callback: on failed
    void setOnFailedCallback(ObjectData* on_failed_callback) {
      assert(!on_failed_callback || on_failed_callback->instanceof(c_Closure::s_cls));
      m_onFailedCallback = on_failed_callback;
    }
    void onFailed(CObjRef exception);

    // ContinuationWaitHandle callbacks:
    void setOnContinuationCreateCallback(ObjectData* on_start) {
      assert(!on_start || on_start->instanceof(c_Closure::s_cls));
      m_onContinuationCreateCallback = on_start;
    }
    void setOnContinuationYieldCallback(ObjectData* on_yield) {
      assert(!on_yield || on_yield->instanceof(c_Closure::s_cls));
      m_onContinuationYieldCallback = on_yield;
    }
    void setOnContinuationSuccessCallback(ObjectData* on_success) {
      assert(!on_success || on_success->instanceof(c_Closure::s_cls));
      m_onContinuationSuccessCallback = on_success;
    }
    void setOnContinuationFailCallback(ObjectData* on_fail) {
      assert(!on_fail || on_fail->instanceof(c_Closure::s_cls));
      m_onContinuationFailCallback = on_fail;
    }
    bool hasOnContinuationCreateCallback() { return m_onContinuationCreateCallback.get(); }
    bool hasOnContinuationYieldCallback() { return m_onContinuationYieldCallback.get(); }
    bool hasOnContinuationSuccessCallback() { return m_onContinuationSuccessCallback.get(); }
    bool hasOnContinuationFailCallback() { return m_onContinuationFailCallback.get(); }
    void onContinuationCreate(c_ContinuationWaitHandle* cont);
    void onContinuationYield(c_ContinuationWaitHandle* cont, c_WaitHandle* child);
    void onContinuationSuccess(c_ContinuationWaitHandle* cont, CVarRef result);
    void onContinuationFail(c_ContinuationWaitHandle* cont, CObjRef exception);

    // WaitHandle callbacks:
    void setOnJoinCallback(ObjectData* on_join) {
      assert(!on_join || on_join->instanceof(c_Closure::s_cls));
      m_onJoinCallback = on_join;
    }
    bool hasOnJoinCallback() { return m_onJoinCallback.get(); }
    void onJoin(c_WaitHandle* wait_handle);

    // GenArrayWaitHandle callbacks:
    void setOnGenArrayCreateCallback(ObjectData* on_create) {
      assert(!on_create || on_create->instanceof(c_Closure::s_cls));
      m_onGenArrayCreateCallback = on_create;
    }
    bool hasOnGenArrayCreateCallback() { return m_onGenArrayCreateCallback.get(); }
    void onGenArrayCreate(c_GenArrayWaitHandle* wait_handle, CVarRef dependencies);

    // SetResultToRefWaitHandle callbacks:
    void setOnSetResultToRefCreateCallback(ObjectData* on_create) {
      assert(!on_create || on_create->instanceof(c_Closure::s_cls));
      m_onSetResultToRefCreateCallback = on_create;
    }
    bool hasOnSetResultToRefCreateCallback() { return m_onSetResultToRefCreateCallback.get(); }
    void onSetResultToRefCreate(c_SetResultToRefWaitHandle* wait_handle, CObjRef child);


  private:
    static DECLARE_THREAD_LOCAL_PROXY(AsioSession, false, s_current);

    static constexpr c_ExternalThreadEventWaitHandle* k_waitingForExternalThreadEvents = static_cast<c_ExternalThreadEventWaitHandle*>((void*)1L);

    AsioSession();

    smart::vector<AsioContext*> m_contexts;

    std::atomic<c_ExternalThreadEventWaitHandle*> m_readyExternalThreadEvents;
    std::mutex m_readyExternalThreadEventsMutex;
    std::condition_variable m_readyExternalThreadEventsCondition;

    Object m_onContinuationCreateCallback;
    Object m_onContinuationYieldCallback;
    Object m_onContinuationSuccessCallback;
    Object m_onContinuationFailCallback;
    Object m_onGenArrayCreateCallback;
    Object m_onSetResultToRefCreateCallback;
    Object m_onJoinCallback;

    // Legacy callback for backwards compatibility.
    Object m_onFailedCallback;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_SESSION_H_
