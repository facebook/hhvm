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

template <typename T>
struct ThreadLocalNode {
  T * m_p;
  void (*m_on_thread_exit_fn)(void * p);
  void * m_next;
};

struct ThreadLocalManager {
  ThreadLocalManager() : m_key(0) {
    ThreadLocalCreateKey(&m_key, ThreadLocalManager::OnThreadExit);
  };
  void * getTop() {
    return pthread_getspecific(m_key);
  }
  void setTop(void * p) {
    pthread_setspecific(m_key, p);
  }
  static void OnThreadExit(void *p);
  pthread_key_t m_key;

  static ThreadLocalManager s_manager;
};

///////////////////////////////////////////////////////////////////////////////
// ThreadLocal allocates by calling new without parameters and frees by calling
// delete

template<typename T>
struct ThreadLocal {
  T *get() const {
    if (m_node.m_p == NULL) {
      const_cast<ThreadLocal<T>*>(this)->createKey();
    }
    return m_node.m_p;
  }

  void createKey() __attribute__((noinline));

  bool isNull() const { return m_node.m_p == NULL; }

  void reset() {
    delete m_node.m_p;
    m_node.m_p = NULL;
  }

  static void OnThreadExit(void * p) {
    ThreadLocalNode<T> * pNode = (ThreadLocalNode<T>*)p;
    delete pNode->m_p;
    pNode->m_p = NULL;
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
void ThreadLocal<T>::createKey() {
  if (m_node.m_on_thread_exit_fn == NULL) {
    m_node.m_on_thread_exit_fn = ThreadLocal<T>::OnThreadExit;
    m_node.m_next = ThreadLocalManager::s_manager.getTop();
    ThreadLocalManager::s_manager.setTop((void*)(&m_node));
  }
  if (m_node.m_p == NULL) {
    m_node.m_p = new T();
  }
}

///////////////////////////////////////////////////////////////////////////////
// ThreadLocalCreate allocates by calling T::Create without parameters and
// frees by calling T::Delete

template<typename T>
struct ThreadLocalCreate {
  T *get() const {
    if (m_node.m_p == NULL) {
      const_cast<ThreadLocalCreate<T>*>(this)->createKey();
    }
    return m_node.m_p;
  }

  void createKey() __attribute__((noinline));

  bool isNull() const { return m_node.m_p == NULL; }

  void reset() {
    T::Delete(m_node.m_p);
    m_node.m_p = NULL;
  }

  static void OnThreadExit(void * p) {
    ThreadLocalNode<T> * pNode = (ThreadLocalNode<T>*)p;
    T::OnThreadExit(pNode->m_p);
    pNode->m_p = NULL;
  }

  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

  void *m_createInfo;

  ThreadLocalNode<T> m_node;
};

template<typename T>
void ThreadLocalCreate<T>::createKey() {
  if (m_node.m_on_thread_exit_fn == NULL) {
    m_node.m_on_thread_exit_fn = ThreadLocalCreate<T>::OnThreadExit;
    m_node.m_next = ThreadLocalManager::s_manager.getTop();
    ThreadLocalManager::s_manager.setTop((void*)(&m_node));
  }
  if (m_node.m_p == NULL) {
    m_node.m_p = T::Create(m_createInfo);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Singleton thread-local storage for T

template<typename T>
class ThreadLocalSingleton {
public:
  ThreadLocalSingleton() : m_key(0) {
    m_key = getKey();
  }

  T *get() const {
    T *& p = getSingleton();
    if (p == NULL) {
      createKey(p);
    }
    return p;
  }

  void createKey(T *& p) const __attribute__((noinline));

  bool isNull() const { return getSingleton() == NULL; }

  void reset() {
    T *& p = getSingleton();
    T::Delete(p);
    p = NULL;
    pthread_setspecific(m_key, NULL);
  }

  static void OnThreadExit(void *obj) {
    T::OnThreadExit((T*)obj);
  }

  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

private:
  pthread_key_t m_key;

  static T *& getSingleton() {
    static __attribute__ ((tls_model ("initial-exec"))) __thread
      T * s_singleton = NULL;
    return s_singleton;
  }

  static pthread_key_t getKey() {
    static pthread_key_t key = 0;
    if (key == 0) {
      ThreadLocalCreateKey(&key, ThreadLocalSingleton<T>::OnThreadExit);
    }
    return key;
  }
};

template<typename T>
void ThreadLocalSingleton<T>::createKey(T *& p) const {
  p = T::Create();
  pthread_setspecific(m_key, p);
}

///////////////////////////////////////////////////////////////////////////////
// some classes don't need new/delete at all

template<typename T, bool throwOnNull = true>
struct ThreadLocalProxy {
  T *get() const {
    if (m_p == NULL && throwOnNull) {
      throw Exception("ThreadLocalProxy::get() called before set()");
    }
    return m_p;
  }

  void set(T* obj) {
    m_p = obj;
  }

  bool isNull() const { return m_p == NULL; }

  void reset() {
    m_p = NULL;
  }

  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

  T * m_p;
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

#define DECLARE_THREAD_LOCAL(T, f) \
  __attribute__((tls_model ("initial-exec"))) __thread \
  ThreadLocal<T> f
#define IMPLEMENT_THREAD_LOCAL(T, f) \
  __attribute__((tls_model ("initial-exec"))) __thread \
  ThreadLocal<T> f = { { NULL, NULL, NULL } }

#define DECLARE_THREAD_LOCAL_CREATE(T, f) \
  __attribute__((tls_model ("initial-exec"))) __thread \
  ThreadLocalCreate<T> f
#define IMPLEMENT_THREAD_LOCAL_CREATE(T, f) \
  __attribute__((tls_model ("initial-exec"))) __thread \
  ThreadLocalCreate<T> f = { { NULL, NULL, NULL } }

#define DECLARE_THREAD_LOCAL_PROXY(T, N, f) \
  __attribute__((tls_model ("initial-exec"))) __thread \
  ThreadLocalProxy<T, N> f
#define IMPLEMENT_THREAD_LOCAL_PROXY(T, N, f) \
  __attribute__((tls_model ("initial-exec"))) __thread \
  ThreadLocalProxy<T, N> f = { NULL }

#else /* USE_GCC_FAST_TLS */

///////////////////////////////////////////////////////////////////////////////
// ThreadLocal allocates by calling new() without parameters

template<typename T>
class ThreadLocal {
public:
  /**
   * Constructor that has to be called from a thread-neutral place.
   */
  ThreadLocal() : m_key(0) {
    ThreadLocalCreateKey(&m_key, ThreadLocal<T>::OnThreadExit);
  }

  T *get() const {
    T *obj = (T*)pthread_getspecific(m_key);
    if (obj == NULL) {
      obj = new T();
      pthread_setspecific(m_key, obj);
    }
    return obj;
  }

  bool isNull() const { return pthread_getspecific(m_key) == NULL; }

  void reset() {
    delete (T*)pthread_getspecific(m_key);
    pthread_setspecific(m_key, NULL);
  }

  static void OnThreadExit(void *p) {
    delete (T*)p;
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
  pthread_key_t m_key;
};

///////////////////////////////////////////////////////////////////////////////
// ThreadLocalCreate allocates by calling T::Create without parameters and
// frees by calling T::Delete

template<typename T>
class ThreadLocalCreate {
public:
  /**
   * Constructor that has to be called from a thread-neutral place.
   */
  ThreadLocalCreate() : m_key(0) {
    ThreadLocalCreateKey(&m_key, ThreadLocalCreate<T>::OnThreadExit);
  }

  T *get() const {
    T *obj = (T*)pthread_getspecific(m_key);
    if (obj == NULL) {
      obj = T::Create(m_createInfo);
      pthread_setspecific(m_key, obj);
    }
    return obj;
  }

  bool isNull() const { return pthread_getspecific(m_key) == NULL; }

  void reset() {
    T::Delete((T*)pthread_getspecific(m_key));
    pthread_setspecific(m_key, NULL);
  }

  static void OnThreadExit(void *obj) {
    T::OnThreadExit((T*)obj);
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

private:
  pthread_key_t m_key;
};

///////////////////////////////////////////////////////////////////////////////
// Singleton thread-local storage for T

template<typename T>
class ThreadLocalSingleton {
public:
  ThreadLocalSingleton() : m_key(0) {
    m_key = getKey();
  }

  T *get() const {
    T *obj = (T*)pthread_getspecific(m_key);
    if (obj == NULL) {
      obj = T::Create();
      pthread_setspecific(m_key, obj);
    }
    return obj;
  }

  bool isNull() const { return pthread_getspecific(m_key) == NULL; }

  void reset() {
    T::Delete((T*)pthread_getspecific(m_key));
    pthread_setspecific(m_key, NULL);
  }

  static void OnThreadExit(void *obj) {
    T::OnThreadExit((T*)obj);
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
  pthread_key_t m_key;

  static pthread_key_t getKey() {
    static pthread_key_t key = 0;
    if (key == 0) {
      ThreadLocalCreateKey(&key, ThreadLocalSingleton<T>::OnThreadExit);
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
    ThreadLocalCreateKey(&m_key, NULL);
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

  bool isNull() const { return pthread_getspecific(m_key) == NULL; }

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
