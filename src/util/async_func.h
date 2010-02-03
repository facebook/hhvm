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

#ifndef __CONCURRENCY_ASYNC_FUNC_H__
#define __CONCURRENCY_ASYNC_FUNC_H__

#include "base.h"
#include <pthread.h>
#include <exception>
#include "synchronizable.h"
#include "lock.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Invokes a function asynchrously. For example,
 *
 *   class MyClass {
 *    public:
 *      void doJob();
 *   };
 *
 *   MyClass obj;
 *   AsyncFunc<MyClass> func(&obj, &MyClasss::doJob);
 *   func.start(); // this will call obj.doJob() in a separate thread
 *   // do something else
 *   func.waitForEnd();
 *
 * Asynchronous function is a slightly different way of thinking about threads.
 * Maybe this can help people understand asynchronous function is actually a
 * broader/identical view of running threads,
 *
 *   class MyRunnable {
 *    public:
 *      void run();
 *   };
 *
 *   MyRunnable thread;
 *   AsyncFunc<Runnable> func(&thread, &MyRunnable::run);
 *   thread.run();
 *
 * Well, asynchronous function is sometimes more flexible in writing a server,
 * because it can bind different threads to methods on the same object:
 *
 *   class MyServer {
 *    public:
 *      void thread1();
 *      void thread2();
 *   };
 *
 *   MyServer server;
 *   AsyncFunc<MyServer> func1(&server, &MyServer::thread1);
 *   AsyncFunc<MyServer> func2(&server, &MyServer::thread2);
 *   func1.start();
 *   func2.start();
 *   ...now both threads are running, accessing the same server object.
 *
 * There is nothing wrong embedding the async function object itself in the
 * class like this,
 *
 *   class MyServer {
 *    public:
 *      MyServer()
 *       : m_thread1(this, &MyServer::thread1),
 *       : m_thread2(this, &MyServer::thread2) {
 *      }
 *
 *      void thread1();
 *      void thread2();
 *
 *      void start() {
 *        m_thread1.start();
 *        m_thread2.start();
 *      }
 *
 *    private:
 *      AsyncFunc<MyServer> m_thread1;
 *      AsyncFunc<MyServer> m_thread2;
 *   };
 *
 */
class AsyncFuncImpl {
public:
  typedef void PFN_THREAD_FUNC(void *);

  /**
   * The global static to feed into pthread_create(), and this will delegate
   * the work to AsyncFuncImpl::threadFuncImpl().
   */
  static void *ThreadFunc(void *obj) {
    ((AsyncFuncImpl*)obj)->threadFuncImpl();
    return NULL;
  }

  /**
   * Called by AsyncFunc<T> so we can call func(obj) back on thread running.
   */
  AsyncFuncImpl(void *obj, PFN_THREAD_FUNC *func)
    : m_stopped(false), m_obj(obj), m_func(func), m_threadId(0),
      m_exceptioned(false) {
  }

  /**
   * Starts this thread.
   */
  void start() {
    pthread_create(&m_threadId, NULL, ThreadFunc, (void*)this);
    ASSERT(m_threadId);
  }

  /**
   * Waits until this thread finishes running.
   */
  void waitForEnd() {
    if (m_threadId == 0) return;

    {
      Lock lock(m_stopMonitor.getMutex());
      while (!m_stopped) {
        m_stopMonitor.wait();
      }
    }

    void *ret = NULL;
    pthread_join(m_threadId, &ret);
    m_threadId = 0;

    if (m_exceptioned) {
      m_exceptioned = false;
      throw m_exception;
    }
  }

  /**
   * Starts and waits until this thread finishes running.
   */
  void run() {
    start();
    waitForEnd();
  }

private:
  Synchronizable m_stopMonitor;
  bool m_stopped;

  void *m_obj;
  PFN_THREAD_FUNC *m_func;
  pthread_t m_threadId;
  bool m_exceptioned;
  std::exception m_exception; // exception was thrown and thread was terminated

  /**
   * Called by ThreadFunc() to delegate the work.
   */
  void threadFuncImpl() {
    try {
      m_func(m_obj);
    } catch (std::exception &e) {
      m_exceptioned = true;
      m_exception = e;
      // fall through, since we don't want a thread to abort abruptly
    }
    {
      Lock lock(m_stopMonitor.getMutex());
      m_stopped = true;
      m_stopMonitor.notify();
    }
  }
};

///////////////////////////////////////////////////////////////////////////////

/**
 * We could have written AysncFunc<T> directly with those methods implemented
 * inside AsyncFuncImpl class, but this way we reduce sizes of our code by
 * only templatizing a very minimal piece of code, sharing everything inside
 * AsyncFuncImpl by all AsyncFunc<T> classes.
 */
template<class T>
class AsyncFunc : public AsyncFuncImpl {
public:
  AsyncFunc(T *obj, void (T::*member_func)())
    : AsyncFuncImpl((void*)this, run_), m_obj(obj), m_memberFunc(member_func) {
  }

  static void run_(void *obj) {
    AsyncFunc<T> *p = (AsyncFunc<T>*)obj;
    (p->m_obj->*(p->m_memberFunc))();
  }

private:
  T *m_obj;
  void (T::*m_memberFunc)();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __CONCURRENCY_ASYNC_FUNC_H__
