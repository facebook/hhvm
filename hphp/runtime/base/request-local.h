/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
 *   class MyRequestLocalClass : public RequestEventHandler {
 *   public:
 *     virtual void requestInit() {...}
 *     virtual void requestShutdown() {...}
 *   };
 *   static RequestLocal<MyRequestLocalClass> s_data;
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
      m_node.m_p->requestInit();
      // this registration makes sure m_p->requestShutdown() will be called
      g_context->registerRequestEventHandler(m_node.m_p);
    }
    return m_node.m_p;
  }

  void create() NEVER_INLINE;

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
    m_node.m_next = ThreadLocalManager::s_manager.getTop();
    ThreadLocalManager::s_manager.setTop((void*)(&m_node));
  }
  assert(m_node.m_p == nullptr);
  m_node.m_p = new T();
}

#define IMPLEMENT_REQUEST_LOCAL(T,f) \
  __thread RequestLocal<T> f

#define DECLARE_STATIC_REQUEST_LOCAL(T,f) \
  static __thread RequestLocal<T> f

#define IMPLEMENT_STATIC_REQUEST_LOCAL(T,f) \
  static __thread RequestLocal<T> f

#define DECLARE_EXTERN_REQUEST_LOCAL(T,f) \
  extern __thread RequestLocal<T> f

#else // defined(USE_GCC_FAST_TLS)

template<typename T>
class RequestLocal {
public:
  RequestLocal(ThreadLocal<T> & tl) : m_tlsObjects(tl) {}
  T *operator->() const { return get();}
  T &operator*() const { return *get();}

  T *get() const {
    T *obj = m_tlsObjects.get();
    if (!obj->getInited()) {
      obj->setInited(true);
      obj->requestInit();

      // this registration makes sure obj->requestShutdown() will be called
      g_context->registerRequestEventHandler(obj);
    }
    return obj;
  }

private:
  ThreadLocal<T> & m_tlsObjects;
};

/*
 * How to use the request-local macros:
 *
 * Use DECLARE_STATIC_REQUEST_LOCAL to declare a *static* class field as
 * request local:
 *   class SomeClass {
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

#define IMPLEMENT_REQUEST_LOCAL(T,f)     \
  IMPLEMENT_THREAD_LOCAL(T, f ## __tl);  \
  RequestLocal<T> f(f ## __tl)

#define DECLARE_STATIC_REQUEST_LOCAL(T,f)    \
  static DECLARE_THREAD_LOCAL(T,f ## __tl);  \
  static RequestLocal<T> f

#define IMPLEMENT_STATIC_REQUEST_LOCAL(T,f)     \
  static IMPLEMENT_THREAD_LOCAL(T, f ## __tl);  \
  static RequestLocal<T> f(f ## __tl)

#define DECLARE_EXTERN_REQUEST_LOCAL(T,f)    \
  extern DECLARE_THREAD_LOCAL(T,f ## __tl);  \
  extern RequestLocal<T> f

#endif // defined(USE_GCC_FAST_TLS)



///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_REQUEST_LOCAL_H_
