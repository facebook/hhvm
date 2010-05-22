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

#ifndef __THREAD_LOCAL_H__
#define __THREAD_LOCAL_H__

#include <pthread.h>
#include "exception.h"
#include <errno.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// Only gcc >= 4.3.0 supports the '__thread' keyword for thread locals

#if !defined(NO_TLS) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 3))
#define USE_GCC_FAST_TLS
#endif

///////////////////////////////////////////////////////////////////////////////
// helper

inline void ThreadLocalCreateKey(pthread_key_t *key, void (*del)(void*)) {
  int ret = pthread_key_create(key, del);
  if (ret != 0) {
    const char *msg = "(unknown error)";
    switch (ret) {
    case EAGAIN:
      msg = "PTHREAD_KEYS_MAX (1024) is exceeded";
      break;
    case ENOMEM:
      msg = "Out-of-memory";
      break;
    }
    throw Exception("pthread_key_create", ret, msg);
  }
}

///////////////////////////////////////////////////////////////////////////////

/**
 * A thread-local object is a "global" object within a thread. This is useful
 * for writing apartment-threaded code, where nothing is actullay shared
 * between different threads (hence no locking) but those variables are not
 * on stack in local scope. To use it, just do something like this,
 *
 *   ThreadLocal<MyClass> static_object;
 *     static_object->data_ = ...;
 *     static_object->doSomething();
 *
 *   ThreadLocal<int> static_number;
 *     int value = *static_number;
 *
 * So, syntax-wise it's similar to pointers. T can be primitive types, and if
 * it's a class, there has to be a default constructor.
 */

///////////////////////////////////////////////////////////////////////////////
#if defined(USE_GCC_FAST_TLS)

template<typename T>
class ThreadLocalBase {
public:
  /**
   * Constructor that has to be called from a thread-neutral place.
   */
  ThreadLocalBase() : m_key(0) {
    ThreadLocalCreateKey(&m_key, OnThreadExit);
  }

  virtual ~ThreadLocalBase() { }

  virtual void reset() = 0;

public:
  static void OnThreadExit(void *obj) {
    ((ThreadLocalBase<T> *)obj)->reset();
  }

  pthread_key_t m_key;
};

template<typename T, int CNT>
static T *&_tls_get() {
  static __attribute__ ((tls_model ("initial-exec"))) __thread T *p = NULL;
  return p;
}

template<typename T>
class ThreadLocal : public ThreadLocalBase<T> {
public:
  ThreadLocal(T *&(*f)(void)) : m_get(f) {}

  T *get() const {
    T *&p = m_get();
    if (p == NULL) {
      return createKey();
    }
    return p;
  }

  T* createKey() const __attribute__((noinline));

  virtual void reset() {
    T *&p = m_get();
    delete p;
    p = NULL;
    pthread_setspecific(ThreadLocalBase<T>::m_key, NULL);
  }

  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

private:
  T *&(*m_get)(void);
};

template<typename T>
T* ThreadLocal<T>::createKey() const {
  T *&p = m_get();
  if (p == NULL) {
    p = new T();
    pthread_setspecific(ThreadLocalBase<T>::m_key,
                        (ThreadLocalBase<T> *)this);
  }

  return p;
}

template<typename T>
class ThreadLocalCreate : public ThreadLocalBase<T> {
public:
  ThreadLocalCreate(T *&(*f)(void)) { m_get = f; }

  T *get() const {
    T *&p = m_get();
    if (p == NULL) {
      p = T::Create(m_createInfo);
      pthread_setspecific(ThreadLocalBase<T>::m_key,
                          (ThreadLocalBase<T> *)this);
    }
    return p;
  }

  virtual void reset() {
    T *&p = m_get();
    delete p;
    p = NULL;
    pthread_setspecific(ThreadLocalBase<T>::m_key, NULL);
  }

  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

  void *m_createInfo;

private:
  T *&(*m_get)(void);
};

///////////////////////////////////////////////////////////////////////////////
// Singleton thread-local storage for T

template<typename T>
class ThreadLocalSingleton {
public:
  T *get() const {
    T *&p = get_loc();
    if (p == NULL) {
      p = T::Create();
    }
    return p;
  }

  void reset() {
    T *&p = get_loc();
    T::Delete(p);
    p = NULL;
  }

  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

private:
  static T *&get_loc() {
    static __attribute__ ((tls_model ("initial-exec"))) __thread T *p = NULL;
    return p;
  }
};

///////////////////////////////////////////////////////////////////////////////
// some classes don't need new/delete at all

template<typename T, bool throwOnNull = true>
class ThreadLocalProxy {
public:
  ThreadLocalProxy(T *&(*f)(void)) : m_get(f) {}

  T *get() const {
    T *obj = m_get();
    if (obj == NULL && throwOnNull) {
      throw Exception("ThreadLocalProxy::get() called before set()");
    }
    return obj;
  }

  void set(T* obj) {
    T *&p = m_get();
    p = obj;
  }

  void reset() {
    T *&p = m_get();
    p = NULL;
  }

  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

private:
  T *&(*m_get)(void);
};

/*
 * How to use the thread-local macros:
 *
 * Use DECLARE_THREAD_LOCAL to declare a *static* class field as thread local:
 *   class SomeClass {
 *     static DECLARE_THREAD_LOCAL(SomeFieldType, f);
 *   }
 *
 * Use IMPLEMENT_THREAD_LOCAL in the cpp file to implement the field:
 *   IMPLEMENT_THREAD_LOCAL(SomeFieldType, SomeClass::f);
 *
 * Remember: *Never* write IMPLEMENT_THREAD_LOCAL in a header file.
 */

#define DECLARE_THREAD_LOCAL(T, f) ThreadLocal<T> f
#define IMPLEMENT_THREAD_LOCAL(T, f) \
  ThreadLocal<T> f(_tls_get<T, __COUNTER__>)

#define DECLARE_THREAD_LOCAL_CREATE(T, f) ThreadLocalCreate<T> f
#define IMPLEMENT_THREAD_LOCAL_CREATE(T, f) \
  ThreadLocalCreate<T> f(_tls_get<T, __COUNTER__>)

#define DECLARE_THREAD_LOCAL_PROXY(T, N, f) ThreadLocalProxy<T, N> f
#define IMPLEMENT_THREAD_LOCAL_PROXY(T, N, f) \
  ThreadLocalProxy<T, N> f(_tls_get<T, __COUNTER__>)

#else /* USE_GCC_FAST_TLS */

template<typename T>
class ThreadLocalBase {
public:
  /**
   * Constructor that has to be called from a thread-neutral place.
   */
  ThreadLocalBase() : m_key(0) {
    ThreadLocalCreateKey(&m_key, OnThreadExit);
  }

public:
  static void OnThreadExit(void *obj) {
    delete (T *)obj;
  }

  pthread_key_t m_key;
};

///////////////////////////////////////////////////////////////////////////////
// most classes can do new() without parameters

template<typename T>
class ThreadLocal : public ThreadLocalBase<T> {
public:
  T *get() const {
    T *obj = (T*)pthread_getspecific(ThreadLocalBase<T>::m_key);
    if (obj == NULL) {
      obj = new T();
      pthread_setspecific(ThreadLocalBase<T>::m_key, obj);
    }
    return obj;
  }

  void reset() {
    delete (T*)pthread_getspecific(ThreadLocalBase<T>::m_key);
    pthread_setspecific(ThreadLocalBase<T>::m_key, NULL);
  }

  /**
   * Access object's member or method through this operator overload.
   */
  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }
};

///////////////////////////////////////////////////////////////////////////////
// some classes need two-phase object creation

template<typename T>
class ThreadLocalCreate : public ThreadLocalBase<T> {
public:
  T *get() const {
    T *obj = (T*)pthread_getspecific(ThreadLocalBase<T>::m_key);
    if (obj == NULL) {
      obj = T::Create(m_createInfo);
      pthread_setspecific(ThreadLocalBase<T>::m_key, obj);
    }
    return obj;
  }

  void reset() {
    delete (T*)pthread_getspecific(ThreadLocalBase<T>::m_key);
    pthread_setspecific(ThreadLocalBase<T>::m_key, NULL);
  }

  /**
   * Access object's member or method through this operator overload.
   */
  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

  void *m_createInfo;
};

///////////////////////////////////////////////////////////////////////////////
// Singleton thread-local storage for T

template<typename T>
class ThreadLocalSingleton {
public:
  ThreadLocalSingleton() : key_(0) {
    key_ = getKey();
  }

  T *get() const {
    T *obj = (T*)pthread_getspecific(key_);
    if (obj == NULL) {
      obj = T::Create();
      pthread_setspecific(key_, obj);
    }
    return obj;
  }

  void reset() {
    T::Delete((T*)pthread_getspecific(key_));
    pthread_setspecific(key_, NULL);
  }

  /**
   * Access object's member or method through this operator overload.
   */
  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

private:
  pthread_key_t key_;

  static pthread_key_t getKey() {
    static pthread_key_t key = 0;
    if (key == 0) {
      ThreadLocalCreateKey(&key, NULL);
    }
    return key;
  }
};

///////////////////////////////////////////////////////////////////////////////
// some classes don't need new/delete at all

template<typename T, bool throwOnNull = true>
class ThreadLocalProxy {
public:
  /**
   * Constructor that has to be called from a thread-neutral place.
   */
  ThreadLocalProxy() : m_key(0) {
    ThreadLocalCreateKey(&m_key, OnThreadExit);
  }

  T *get() const {
    T *obj = (T*)pthread_getspecific(m_key);
    if (obj == NULL && throwOnNull) {
      throw Exception("ThreadLocalProxy::get() called before set()");
    }
    return obj;
  }

  void set(T* obj) {
    pthread_setspecific(m_key, obj);
  }

  void reset() {
    pthread_setspecific(m_key, NULL);
  }

  /**
   * Access object's member or method through this operator overload.
   */
  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

public:
  static void OnThreadExit(void *obj) {
    // do nothing
  }

  pthread_key_t m_key;
};

#define DECLARE_THREAD_LOCAL(T, f) ThreadLocal<T> f
#define IMPLEMENT_THREAD_LOCAL(T, f) ThreadLocal<T> f

#define DECLARE_THREAD_LOCAL_CREATE(T, f) ThreadLocalCreate<T> f
#define IMPLEMENT_THREAD_LOCAL_CREATE(T, f) ThreadLocalCreate<T> f

#define DECLARE_THREAD_LOCAL_PROXY(T, N, f) ThreadLocalProxy<T, N> f
#define IMPLEMENT_THREAD_LOCAL_PROXY(T, N, f) ThreadLocalProxy<T, N> f

#endif /* USE_GCC_FAST_TLS */

///////////////////////////////////////////////////////////////////////////////
}

#endif // __THREAD_LOCAL_H__
