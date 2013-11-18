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

#ifndef incl_HPHP_THREAD_LOCAL_H_
#define incl_HPHP_THREAD_LOCAL_H_

#include <pthread.h>
#include "hphp/util/exception.h"
#include <errno.h>
#include "hphp/util/util.h"
#include "folly/String.h"
#include <boost/aligned_storage.hpp>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// gcc >= 4.3.0 supports the '__thread' keyword for thread locals
//
// Clang seems to have added this feature, or at the very least it is ignoring
// __thread keyword and compiling anyway
//
// On OSX, gcc does emulate TLS but in a manner that invalidates assumptions
// we have made about __thread and makes accessing thread-local variables in a
// JIT-friendly fashion difficult (as the compiler is doing a lot of magic that
// is not contractual or documented that we would need to duplicate in emitted
// code) so for now we're not going to use it. One possibility if we really
// want to do this is to generate functions that access variables of interest
// in ThreadLocal* (all of them are NoCheck right now) and use the bytes of
// gcc's compiled functions to find the values we would need to pass to
// __emutls_get_address.
//
// icc 13.0.0 appears to support it as well but we end up with
// assembler warnings of unknown importance about incorrect section
// types

#if !defined(NO_TLS) && !defined(__APPLE__) &&                  \
  ((__llvm__ && __clang__) ||                                   \
   __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 3) ||     \
   __INTEL_COMPILER)
#define USE_GCC_FAST_TLS
#endif

///////////////////////////////////////////////////////////////////////////////
// helper

inline void ThreadLocalCheckReturn(int ret, const char *funcName) {
  if (ret != 0) {
    // This is used from global constructors so the safest thing to do is just
    // print to stderr and exit().
    fprintf(stderr, "%s returned %d: %s", funcName, ret,
            folly::errnoStr(ret).c_str());
    exit(1);
  }
}

inline void ThreadLocalCreateKey(pthread_key_t *key, void (*del)(void*)) {
  int ret = pthread_key_create(key, del);
  ThreadLocalCheckReturn(ret, "pthread_key_create");
}

inline void ThreadLocalSetValue(pthread_key_t key, const void* value) {
  int ret = pthread_setspecific(key, value);
  ThreadLocalCheckReturn(ret, "pthread_setspecific");
}

///////////////////////////////////////////////////////////////////////////////

/**
 * A thread-local object is a "global" object within a thread. This is useful
 * for writing apartment-threaded code, where nothing is actually shared
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
    ThreadLocalSetValue(m_key, p);
  }
  static void OnThreadExit(void *p);
  pthread_key_t m_key;

  static ThreadLocalManager s_manager;
};

///////////////////////////////////////////////////////////////////////////////
// ThreadLocal allocates by calling new without parameters and frees by calling
// delete

template<typename T>
void ThreadLocalOnThreadExit(void * p) {
  ThreadLocalNode<T> * pNode = (ThreadLocalNode<T>*)p;
  delete pNode->m_p;
  pNode->m_p = nullptr;
}

template<typename T>
struct ThreadLocal {
  T *get() const {
    if (m_node.m_p == nullptr) {
      const_cast<ThreadLocal<T>*>(this)->create();
    }
    return m_node.m_p;
  }

  void create() NEVER_INLINE;

  bool isNull() const { return m_node.m_p == nullptr; }

  void destroy() {
    delete m_node.m_p;
    m_node.m_p = nullptr;
  }

  void nullOut() {
    m_node.m_p = nullptr;
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
void ThreadLocal<T>::create() {
  if (m_node.m_on_thread_exit_fn == nullptr) {
    m_node.m_on_thread_exit_fn = ThreadLocalOnThreadExit<T>;
    m_node.m_next = ThreadLocalManager::s_manager.getTop();
    ThreadLocalManager::s_manager.setTop((void*)(&m_node));
  }
  assert(m_node.m_p == nullptr);
  m_node.m_p = new T();
}

template<typename T>
struct ThreadLocalNoCheck {
  T *getCheck() const NEVER_INLINE;
  T* getNoCheck() const {
    assert(m_node.m_p);
    return m_node.m_p;
  }

  void create() NEVER_INLINE;

  bool isNull() const { return m_node.m_p == nullptr; }

  void destroy() {
    delete m_node.m_p;
    m_node.m_p = nullptr;
  }

  T *operator->() const {
    return getNoCheck();
  }

  T &operator*() const {
    return *getNoCheck();
  }

  ThreadLocalNode<T> m_node;
};

template<typename T>
void ThreadLocalNoCheck<T>::create() {
  if (m_node.m_on_thread_exit_fn == nullptr) {
    m_node.m_on_thread_exit_fn = ThreadLocalOnThreadExit<T>;
    m_node.m_next = ThreadLocalManager::s_manager.getTop();
    ThreadLocalManager::s_manager.setTop((void*)(&m_node));
  }
  assert(m_node.m_p == nullptr);
  m_node.m_p = new T();
}
template<typename T>
T *ThreadLocalNoCheck<T>::getCheck() const {
  if (m_node.m_p == nullptr) {
    const_cast<ThreadLocalNoCheck<T>*>(this)->create();
  }
  return m_node.m_p;
}


///////////////////////////////////////////////////////////////////////////////
// Singleton thread-local storage for T

template<typename T>
void ThreadLocalSingletonOnThreadExit(void *obj) {
  T::OnThreadExit((T*)obj);
}

// ThreadLocalSingleton has NoCheck property
template <typename T>
class ThreadLocalSingleton {
public:
  ThreadLocalSingleton() { s_inited = true; }

  static T *getCheck() NEVER_INLINE;

  static T* getNoCheck() {
    assert(s_inited);
    assert(s_singleton == (T*)&s_storage);
    return (T*)&s_storage;
  }

  static bool isNull() { return s_singleton == nullptr; }

  static void destroy() {
    assert(!s_singleton || s_singleton == (T*)&s_storage);
    T* p = s_singleton;
    if (p) {
      T::Delete(p);
      s_singleton = nullptr;
    }
  }

  T *operator->() const {
    return getNoCheck();
  }

  T &operator*() const {
    return *getNoCheck();
  }

private:
  static __thread T *s_singleton;
  typedef typename boost::aligned_storage<sizeof(T), sizeof(void*)>::type
          StorageType;
  static __thread StorageType s_storage;
  static bool s_inited; // no-fast-TLS requires construction so be consistent
};

template<typename T>
bool ThreadLocalSingleton<T>::s_inited = false;

template<typename T>
T *ThreadLocalSingleton<T>::getCheck() {
  assert(s_inited);
  if (!s_singleton) {
    T* p = (T*) &s_storage;
    T::Create(p);
    s_singleton = p;
  }
  return s_singleton;
}

template<typename T> __thread T *ThreadLocalSingleton<T>::s_singleton;
template<typename T> __thread typename ThreadLocalSingleton<T>::StorageType
                              ThreadLocalSingleton<T>::s_storage;


///////////////////////////////////////////////////////////////////////////////
// some classes don't need new/delete at all

template<typename T, bool throwOnNull = true>
struct ThreadLocalProxy {
  T *get() const {
    if (m_p == nullptr && throwOnNull) {
      throw Exception("ThreadLocalProxy::get() called before set()");
    }
    return m_p;
  }

  void set(T* obj) {
    m_p = obj;
  }

  bool isNull() const { return m_p == nullptr; }

  void destroy() {
    m_p = nullptr;
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
  __thread ThreadLocal<T> f
#define IMPLEMENT_THREAD_LOCAL(T, f) \
  __thread HPHP::ThreadLocal<T> f

#define DECLARE_THREAD_LOCAL_NO_CHECK(T, f) \
  __thread ThreadLocalNoCheck<T> f
#define IMPLEMENT_THREAD_LOCAL_NO_CHECK(T, f) \
  __thread ThreadLocalNoCheck<T> f

#define DECLARE_THREAD_LOCAL_PROXY(T, N, f) \
  __thread ThreadLocalProxy<T, N> f
#define IMPLEMENT_THREAD_LOCAL_PROXY(T, N, f) \
  __thread ThreadLocalProxy<T, N> f

#else /* USE_GCC_FAST_TLS */

///////////////////////////////////////////////////////////////////////////////
// ThreadLocal allocates by calling new() without parameters

template<typename T>
void ThreadLocalOnThreadExit(void *p) {
  delete (T*)p;
}

template<typename T>
class ThreadLocal {
public:
  /**
   * Constructor that has to be called from a thread-neutral place.
   */
  ThreadLocal() : m_key(0) {
    ThreadLocalCreateKey(&m_key, ThreadLocalOnThreadExit<T>);
  }

  T *get() const {
    T *obj = (T*)pthread_getspecific(m_key);
    if (obj == nullptr) {
      obj = new T();
      ThreadLocalSetValue(m_key, obj);
    }
    return obj;
  }

  bool isNull() const { return pthread_getspecific(m_key) == nullptr; }

  void destroy() {
    delete (T*)pthread_getspecific(m_key);
    ThreadLocalSetValue(m_key, nullptr);
  }

  void nullOut() {
    ThreadLocalSetValue(m_key, nullptr);
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

template<typename T>
class ThreadLocalNoCheck {
public:
  /**
   * Constructor that has to be called from a thread-neutral place.
   */
  ThreadLocalNoCheck() : m_key(0) {
    ThreadLocalCreateKey(&m_key, ThreadLocalOnThreadExit<T>);
  }

  T *getCheck() const NEVER_INLINE;

  T* getNoCheck() const {
    T *obj = (T*)pthread_getspecific(m_key);
    assert(obj);
    return obj;
  }

  bool isNull() const { return pthread_getspecific(m_key) == nullptr; }

  void destroy() {
    delete (T*)pthread_getspecific(m_key);
    ThreadLocalSetValue(m_key, nullptr);
  }

  /**
   * Access object's member or method through this operator overload.
   */
  T *operator->() const {
    return getNoCheck();
  }

  T &operator*() const {
    return *getNoCheck();
  }

public:
  pthread_key_t m_key;
};

template<typename T>
T *ThreadLocalNoCheck<T>::getCheck() const {
  T *obj = (T*)pthread_getspecific(m_key);
  if (obj == nullptr) {
    obj = new T();
    ThreadLocalSetValue(m_key, obj);
  }
  return obj;
}

///////////////////////////////////////////////////////////////////////////////
// Singleton thread-local storage for T

template<typename T>
void ThreadLocalSingletonOnThreadExit(void *obj) {
  T::OnThreadExit((T*)obj);
  free(obj);
}

// ThreadLocalSingleton has NoCheck property
template<typename T>
class ThreadLocalSingleton {
public:
  ThreadLocalSingleton() { getKey(); }

  static T *getCheck() NEVER_INLINE;
  static T* getNoCheck() {
    assert(s_inited);
    T *obj = (T*)pthread_getspecific(s_key);
    assert(obj);
    return obj;
  }

  static bool isNull() { return pthread_getspecific(s_key) == nullptr; }

  static void destroy() {
    void* p = pthread_getspecific(s_key);
    T::Delete((T*)p);
    free(p);
    ThreadLocalSetValue(s_key, nullptr);
  }

  T *operator->() const {
    return getNoCheck();
  }

  T &operator*() const {
    return *getNoCheck();
  }

private:
  static pthread_key_t s_key;
  static bool s_inited; // pthread_key_t has no portable valid sentinel

  static pthread_key_t getKey() {
    if (!s_inited) {
      s_inited = true;
      ThreadLocalCreateKey(&s_key, ThreadLocalSingletonOnThreadExit<T>);
    }
    return s_key;
  }
};

template<typename T>
T *ThreadLocalSingleton<T>::getCheck() {
  assert(s_inited);
  T *obj = (T*)pthread_getspecific(s_key);
  if (obj == nullptr) {
    obj = (T*)malloc(sizeof(T));
    T::Create(obj);
    ThreadLocalSetValue(s_key, obj);
  }
  return obj;
}

template<typename T>
pthread_key_t ThreadLocalSingleton<T>::s_key;
template<typename T>
bool ThreadLocalSingleton<T>::s_inited = false;

///////////////////////////////////////////////////////////////////////////////
// some classes don't need new/delete at all

template<typename T, bool throwOnNull = true>
class ThreadLocalProxy {
public:
  /**
   * Constructor that has to be called from a thread-neutral place.
   */
  ThreadLocalProxy() : m_key(0) {
    ThreadLocalCreateKey(&m_key, nullptr);
  }

  T *get() const {
    T *obj = (T*)pthread_getspecific(m_key);
    if (obj == nullptr && throwOnNull) {
      throw Exception("ThreadLocalProxy::get() called before set()");
    }
    return obj;
  }

  void set(T* obj) {
    ThreadLocalSetValue(m_key, obj);
  }

  bool isNull() const { return pthread_getspecific(m_key) == nullptr; }

  void destroy() {
    ThreadLocalSetValue(m_key, nullptr);
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

#define DECLARE_THREAD_LOCAL_NO_CHECK(T, f) ThreadLocalNoCheck<T> f
#define IMPLEMENT_THREAD_LOCAL_NO_CHECK(T, f) ThreadLocalNoCheck<T> f

#define DECLARE_THREAD_LOCAL_PROXY(T, N, f) ThreadLocalProxy<T, N> f
#define IMPLEMENT_THREAD_LOCAL_PROXY(T, N, f) ThreadLocalProxy<T, N> f

#endif /* USE_GCC_FAST_TLS */

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_THREAD_LOCAL_H_
