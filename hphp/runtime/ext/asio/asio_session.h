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

#ifndef __EXT_ASIO_SESSION_H__
#define __EXT_ASIO_SESSION_H__

#include <runtime/base/base_includes.h>
#include <runtime/ext/asio/asio_context.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FORWARD_DECLARE_CLASS_BUILTIN(ContinuationWaitHandle);

class AsioSession {
  public:
    static void Init();
    static inline AsioSession* Get() { return s_current.get(); }

    void* operator new(size_t size) { return smart_malloc(size); }
    void operator delete(void* ptr) { smart_free(ptr); }

    // context
    inline void enterContext() {
      assert(!isInContext() || getCurrentContext()->isRunning());
      m_contexts.push_back(new AsioContext());
      assert(static_cast<context_idx_t>(m_contexts.size()) == m_contexts.size());
    }

    inline void exitContext() {
      assert(isInContext());
      m_contexts.back()->exit(m_contexts.size());
      delete m_contexts.back();
      m_contexts.pop_back();
    }

    inline bool isInContext() {
      return !m_contexts.empty();
    }

    inline AsioContext* getContext(context_idx_t ctx_idx) {
      assert(ctx_idx <= m_contexts.size());
      return ctx_idx ? m_contexts[ctx_idx - 1] : nullptr;
    }

    inline AsioContext* getCurrentContext() {
      return m_contexts.empty() ? nullptr : m_contexts.back();
    }

    inline context_idx_t getCurrentContextIdx() {
      assert(static_cast<context_idx_t>(m_contexts.size()) == m_contexts.size());
      return static_cast<context_idx_t>(m_contexts.size());
    }

    inline c_ContinuationWaitHandle* getCurrentWaitHandle() {
      return m_contexts.empty() ? nullptr : m_contexts.back()->getCurrent();
    }

    uint16_t getCurrentWaitHandleDepth();

    // callback: on failed
    void setOnFailedCallback(ObjectData* on_failed_callback);
    void onFailed(CObjRef exception);

  private:
    static DECLARE_THREAD_LOCAL_PROXY(AsioSession, false, s_current);

    AsioSession();

    smart::vector<AsioContext*> m_contexts;
    ObjectData* m_onFailedCallback;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_ASIO_SESSION_H__
