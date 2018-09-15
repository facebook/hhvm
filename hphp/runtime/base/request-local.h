/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_REQUEST_LOCAL_H_
#define incl_HPHP_REQUEST_LOCAL_H_

#include "hphp/runtime/base/execution-context.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A RequestLocal<T> is automatically thread local, plus it has two handlers
 * to do extra work on request init and shutdown times. T needs to derive from
 * RequestEventHandler, so it will register itself with execution engines to
 * be called at request shutdown time.
 *
 * Example:
 *
 *   struct MyRequestLocalClass final : RequestEventHandler {
 *     void requestInit() override {...}
 *     void requestShutdown() override {...}
 *   };
 *   IMPLEMENT_STATIC_REQUEST_LOCAL(MyRequestLocalClass, s_data);
 *
 * How to use the request-local macros:
 *
 * Use DECLARE_STATIC_REQUEST_LOCAL to declare a *static* class field as
 * request local:
 *   struct SomeClass {
 *     DECLARE_STATIC_REQUEST_LOCAL(SomeFieldType, f);
 *   }
 *
 * Use IMPLEMENT_STATIC_REQUEST_LOCAL in the cpp file to implement the field:
 *   IMPLEMENT_STATIC_REQUEST_LOCAL(SomeFieldType, SomeClass::f);
 *
 * The DECLARE_REQUEST_LOCAL and IMPLEMENT_REQUEST_LOCAL macros are provided
 * for declaring/implementing request locals in the global scope.
 *
 * Remember: *Never* write IMPLEMENT_STATIC_REQUEST_LOCAL in a header file.
 */

#if defined(USE_GCC_FAST_TLS)

template<typename T>
struct RequestLocal {
  T *get() const {
    if (m_node.m_p == nullptr) {
      const_cast<RequestLocal<T>*>(this)->create();
    }
    if (!m_node.m_p->getInited()) {
      m_node.m_p->setInited(true);
      // This registration makes sure obj->requestShutdown() will be called. Do
      // it before calling requestInit() so that obj is reachable to the GC no
      // matter what the callback does.
      auto index = g_context->registerRequestEventHandler(m_node.m_p);
      try {
        m_node.m_p->requestInit();
      } catch (...) {
        m_node.m_p->setInited(false);
        g_context->unregisterRequestEventHandler(m_node.m_p, index);
        throw;
      }
    }
    return m_node.m_p;
  }

  bool getInited() const {
    return (m_node.m_p != nullptr) && m_node.m_p->getInited();
  }

  NEVER_INLINE void create();

  static void OnThreadExit(void * p) {
    ThreadLocalNode<T> * pNode = (ThreadLocalNode<T>*)p;
    delete pNode->m_p;
    pNode->m_p = nullptr;
  }

  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

  ThreadLocalNode<T> m_node;
};

template<typename T>
void RequestLocal<T>::create() {
  if (m_node.m_on_thread_exit_fn == nullptr) {
    m_node.m_on_thread_exit_fn = RequestLocal<T>::OnThreadExit;
    ThreadLocalManager::PushTop(m_node);
  }
  assertx(m_node.m_p == nullptr);
  m_node.m_p = new T();
}

#define IMPLEMENT_REQUEST_LOCAL(T,f) \
  __thread HPHP::RequestLocal<T> f

#define DECLARE_STATIC_REQUEST_LOCAL(T,f) \
  static __thread HPHP::RequestLocal<T> f

#define IMPLEMENT_STATIC_REQUEST_LOCAL(T,f) \
  static __thread HPHP::RequestLocal<T> f

#define DECLARE_EXTERN_REQUEST_LOCAL(T,f) \
  extern __thread HPHP::RequestLocal<T> f

#else // defined(USE_GCC_FAST_TLS)

template<typename T>
struct RequestLocal {
  explicit RequestLocal(ThreadLocal<T> & tl) : m_tlsObjects(tl) {}

  bool getInited() const {
    return !m_tlsObjects.isNull() && m_tlsObjects.get()->getInited();
  }

  T *operator->() const { return get();}
  T &operator*() const { return *get();}

  T *get() const {
    T *obj = m_tlsObjects.get();
    if (!obj->getInited()) {
      obj->setInited(true);
      // This registration makes sure obj->requestShutdown() will be called. Do
      // it before calling requestInit() so that obj is reachable to the GC no
      // matter what the callback does.
      auto index = g_context->registerRequestEventHandler(obj);
      try {
        obj->requestInit();
      } catch (...) {
        obj->setInited(false);
        g_context->unregisterRequestEventHandler(obj, index);
        throw;
      }
    }
    return obj;
  }

private:
  ThreadLocal<T> & m_tlsObjects;
};


#define IMPLEMENT_REQUEST_LOCAL(T,f)     \
  THREAD_LOCAL(T, f ## __tl);  \
  HPHP::RequestLocal<T> f(f ## __tl)

#define DECLARE_STATIC_REQUEST_LOCAL(T,f)    \
  static THREAD_LOCAL(T,f ## __tl);  \
  static HPHP::RequestLocal<T> f

#define IMPLEMENT_STATIC_REQUEST_LOCAL(T,f)     \
  static THREAD_LOCAL(T, f ## __tl);  \
  static HPHP::RequestLocal<T> f(f ## __tl)

#define DECLARE_EXTERN_REQUEST_LOCAL(T,f)    \
  extern THREAD_LOCAL(T,f ## __tl);  \
  extern HPHP::RequestLocal<T> f

#endif // defined(USE_GCC_FAST_TLS)



///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_REQUEST_LOCAL_H_
