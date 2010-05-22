/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_REQUEST_LOCAL_H__
#define __HPHP_REQUEST_LOCAL_H__

#include <runtime/base/execution_context.h>

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

  void reset() {
    m_tlsObjects.reset();
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

#define DECLARE_REQUEST_LOCAL(T,f)    \
  DECLARE_THREAD_LOCAL(T,f ## __tl);  \
  RequestLocal<T> f

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

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_REQUEST_LOCAL_H__
