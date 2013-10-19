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

#ifndef incl_HPHP_CONCURRENCY_ASYNC_FUNC_H_
#define incl_HPHP_CONCURRENCY_ASYNC_FUNC_H_

#include "hphp/util/base.h"
#include <pthread.h>
#include "hphp/util/synchronizable.h"
#include "hphp/util/lock.h"
#include "hphp/util/exception.h"
#include "hphp/util/alloc.h"

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
  static void *ThreadFunc(void *obj);

  /**
   * Called by AsyncFunc<T> so we can call func(obj) back on thread running.
   */
  AsyncFuncImpl(void *obj, PFN_THREAD_FUNC *func);
  ~AsyncFuncImpl();

  /**
   * Starts this thread.
   */
  void start();

  /**
   * Sends a cancellation request to the thread. NB: Do not use this unless
   * the function is known to support cancellation and known to leave shared
   * state in a consistent state (alternatively, the caller should proceed to
   * shut down the process as well). Also, call waitForEnd following this call
   * before proceeding as if the async func has stopped executing.
   */
  void cancel();

  /**
   * Waits until this thread finishes running.
   */
  bool waitForEnd(int seconds = 0);

  /**
   * Starts and waits until this thread finishes running.
   */
  void run() {
    start();
    waitForEnd();
  }

  pthread_attr_t *getThreadAttr() {
    return &m_attr;
  }

  static void SetThreadInitFunc(PFN_THREAD_FUNC* func, void *arg) {
    s_initFunc = func;
    s_initFuncArg = arg;
  }

  static void SetThreadFiniFunc(PFN_THREAD_FUNC* func, void *arg) {
    s_finiFunc = func;
    s_finiFuncArg = arg;
  }

  static PFN_THREAD_FUNC* GetThreadInitFunc() {
    return s_initFunc;
  }

  static PFN_THREAD_FUNC* GetThreadFiniFunc() {
    return s_finiFunc;
  }

  void setNoInit() { m_noInit = true; }
private:
  Synchronizable m_stopMonitor;

  void* m_obj;
  PFN_THREAD_FUNC* m_func;
  static PFN_THREAD_FUNC* s_initFunc;
  static PFN_THREAD_FUNC* s_finiFunc;
  static void* s_initFuncArg;
  static void* s_finiFuncArg;
  void* m_threadStack;
  pthread_attr_t m_attr;
  pthread_t m_threadId;
  Exception* m_exception; // exception was thrown and thread was terminated
  int m_node;
  bool m_stopped;
  bool m_noInit;

  static const size_t m_stackSizeMinimum = 8388608; // 8MB

  /**
   * Called by ThreadFunc() to delegate the work.
   */
  void threadFuncImpl();
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

#endif // incl_HPHP_CONCURRENCY_ASYNC_FUNC_H_
