/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <sys/user.h>
#include <pthread.h>
#include "synchronizable.h"
#include "lock.h"
#include "exception.h"
#include "alloc.h"

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
    pthread_attr_t *attr;
    size_t stacksize, guardsize;
    void *stackaddr;

    attr = ((AsyncFuncImpl*)obj)->getThreadAttr();
    pthread_attr_getstack(attr, &stackaddr, &stacksize);

    // Get the guard page's size, because the stack address returned
    // above starts at the guard page, so the thread's stack limit is
    // stackaddr + guardsize.
    if (pthread_attr_getguardsize(attr, &guardsize) != 0)
      guardsize = 0;

    ASSERT(stackaddr != NULL);
    ASSERT(stacksize >= PTHREAD_STACK_MIN);
    Util::s_stackLimit = uintptr_t(stackaddr) + guardsize;
    Util::s_stackSize = stacksize;

    ((AsyncFuncImpl*)obj)->threadFuncImpl();
    return NULL;
  }

  /**
   * Called by AsyncFunc<T> so we can call func(obj) back on thread running.
   */
  AsyncFuncImpl(void *obj, PFN_THREAD_FUNC *func)
      : m_stopped(false),
        m_obj(obj), m_func(func), m_threadId(0), m_exceptioned(false),
        m_threadStack(NULL), m_stackSize(0) {
  }

  /**
   * Starts this thread.
   */
  void start() {
    struct rlimit rlim;

    // Allocate the thread-stack
    pthread_attr_init(&m_attr);

    if (getrlimit(RLIMIT_STACK, &rlim) != 0 || rlim.rlim_cur == RLIM_INFINITY ||
        rlim.rlim_cur < m_stackSizeMinimum) {
      rlim.rlim_cur = m_stackSizeMinimum;
    }

    // On Success use the allocated memory for the thread's stack
    if (posix_memalign(&m_threadStack, PAGE_SIZE, rlim.rlim_cur) == 0) {
      pthread_attr_setstack(&m_attr, m_threadStack, rlim.rlim_cur);
    }

    pthread_create(&m_threadId, &m_attr, ThreadFunc, (void*)this);
    ASSERT(m_threadId);
  }

  /**
   * Waits until this thread finishes running.
   */
  bool waitForEnd(int seconds = 0) {
    if (m_threadId == 0) return true;

    {
      Lock lock(m_stopMonitor.getMutex());
      while (!m_stopped) {
        if (seconds > 0) {
          if (!m_stopMonitor.wait(seconds)) {
            // wait timed out
            return false;
          }
        } else {
          m_stopMonitor.wait();
        }
      }
    }

    void *ret = NULL;
    pthread_join(m_threadId, &ret);
    m_threadId = 0;

    if (m_threadStack != NULL) {
      free(m_threadStack);
      m_threadStack = NULL;
    }

    if (m_exceptioned) {
      m_exceptioned = false;
      throw m_exception;
    }

    return true;
  }

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

private:
  Synchronizable m_stopMonitor;
  bool m_stopped;

  void *m_obj;
  PFN_THREAD_FUNC *m_func;
  static PFN_THREAD_FUNC *s_initFunc;
  static PFN_THREAD_FUNC *s_finiFunc;
  static void* s_initFuncArg;
  static void* s_finiFuncArg;
  pthread_t m_threadId;
  bool m_exceptioned;
  void *m_threadStack;
  size_t m_stackSize;
  Exception m_exception; // exception was thrown and thread was terminated
  pthread_attr_t m_attr;

  static const size_t m_stackSizeMinimum = 8388608; // 8MB

  /**
   * Called by ThreadFunc() to delegate the work.
   */
  void threadFuncImpl() {
    if (s_initFunc) {
      s_initFunc(s_initFuncArg);
    }
    try {
      m_func(m_obj);
    } catch (Exception &e) {
      m_exceptioned = true;
      m_exception = e;
    } catch (std::exception &e) {
      m_exceptioned = true;
      m_exception.setMessage(e.what());
    } catch (...) {
      m_exceptioned = true;
      m_exception.setMessage("(unknown exception)");
    }
    {
      Lock lock(m_stopMonitor.getMutex());
      m_stopped = true;
      m_stopMonitor.notify();
    }
    if (s_finiFunc) {
      s_finiFunc(s_finiFuncArg);
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
