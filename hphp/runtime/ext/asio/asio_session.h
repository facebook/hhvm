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

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <runtime/base/base_includes.h>
#include <runtime/ext/asio/asio_context.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

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
    void setOnFailedCallback(ObjectData* on_failed_callback);
    void onFailed(CObjRef exception);

    // callback: on started
    void setOnStartedCallback(ObjectData* on_started_callback);
    void onStarted(CObjRef wait_handle);
    bool hasOnStartedCallback() { return m_onStartedCallback; }

  private:
    static DECLARE_THREAD_LOCAL_PROXY(AsioSession, false, s_current);

    static constexpr c_ExternalThreadEventWaitHandle* k_waitingForExternalThreadEvents = static_cast<c_ExternalThreadEventWaitHandle*>((void*)1L);

    AsioSession();

    smart::vector<AsioContext*> m_contexts;

    std::atomic<c_ExternalThreadEventWaitHandle*> m_readyExternalThreadEvents;
    std::mutex m_readyExternalThreadEventsMutex;
    std::condition_variable m_readyExternalThreadEventsCondition;

    ObjectData* m_onFailedCallback;
    ObjectData* m_onStartedCallback;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_SESSION_H_
