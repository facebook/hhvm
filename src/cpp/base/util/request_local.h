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

#include <cpp/base/execution_context.h>

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
  RequestLocal() {
  }

  T *operator->() const { return get();}
  T &operator*() const { return *get();}

  T *get() const {
    T *obj = m_tlsObjects.get();
    if (!obj->getInited()) {
      obj->requestInit();
      obj->setInited(true);

      // this registration makes sure obj->requestShutdown() will be called
      g_context->registerRequestEventHandler(obj);
    }
    return obj;
  }

  void reset() {
    m_tlsObjects.reset();
  }

private:
  ThreadLocal<T> m_tlsObjects;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_REQUEST_LOCAL_H__
