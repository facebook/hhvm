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

#ifndef incl_HPHP_THREAD_LOCAL_EMULATE_H_
#define incl_HPHP_THREAD_LOCAL_EMULATE_H_

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// ThreadLocal allocates by calling new() without parameters

template<typename T>
void ThreadLocalOnThreadExit(void *p) {
  delete (T*)p;
}

#ifdef __APPLE__
// The __thread variables in class T will be freed when pthread calls
// the destructor function on Mac. We can register a handler in
// pthread_t->__cleanup_stack similar to pthread_cleanup_push(). The handler
// will be called earlier so the __thread variables will still exist in the
// handler when the thread exits.
//
// See the details at:
// https://github.com/facebook/hhvm/issues/4444#issuecomment-92497582

template<typename T>
void ThreadLocalOnThreadCleanup(void *key) {
  void *obj = pthread_getspecific((pthread_key_t)key);
  if (obj) {
    ThreadLocalOnThreadExit<T>(obj);
  }
}

inline void ThreadLocalSetCleanupHandler(pthread_key_t cleanup_key,
                                         pthread_key_t key,
                                         void (*del)(void*)) {
  // Prevent from adding the handler for multiple times.
  darwin_pthread_handler *handler =
      (darwin_pthread_handler*)pthread_getspecific(cleanup_key);
  if (handler)
    return;

  pthread_t self = pthread_self();

  handler = new darwin_pthread_handler();
  handler->__routine = del;
  handler->__arg = (void*)key;
  handler->__next = self->__cleanup_stack;
  self->__cleanup_stack = handler;

  ThreadLocalSetValue(cleanup_key, handler);
}
#endif

/**
 * This is the emulation version of ThreadLocal. In this case, the ThreadLocal
 * object is a true global, and the get() method returns a thread-dependent
 * pointer from pthread's thread-specific data management.
 */
template<bool Check, typename T>
struct ThreadLocalImpl {
  /**
   * Constructor that has to be called from a thread-neutral place.
   */
  ThreadLocalImpl() : m_key(0) {
#ifdef __APPLE__
    ThreadLocalCreateKey(&m_key, nullptr);
    ThreadLocalCreateKey(&m_cleanup_key,
                         ThreadLocalOnThreadExit<darwin_pthread_handler>);
#else
    ThreadLocalCreateKey(&m_key, ThreadLocalOnThreadExit<T>);
#endif
  }

  NEVER_INLINE T* getCheck() const {
    return get();
  }

  T* getNoCheck() const {
    auto obj = (T*)pthread_getspecific(m_key);
    assert(obj);
    return obj;
  }

  T* get() const {
    auto obj = (T*)pthread_getspecific(m_key);
    if (obj == nullptr) {
      obj = new T();
      ThreadLocalSetValue(m_key, obj);
#ifdef __APPLE__
      ThreadLocalSetCleanupHandler(m_cleanup_key, m_key,
                                   ThreadLocalOnThreadCleanup<T>);
#endif
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
  T* operator->() const {
    return Check ? get() : getNoCheck();
  }

  T& operator*() const {
    return Check ? *get() : *getNoCheck();
  }

private:
  pthread_key_t m_key;

#ifdef __APPLE__
  pthread_key_t m_cleanup_key;
#endif
};

template<typename T> using ThreadLocal = ThreadLocalImpl<true,T>;
template<typename T> using ThreadLocalNoCheck = ThreadLocalImpl<false,T>;

///////////////////////////////////////////////////////////////////////////////
// some classes don't need new/delete at all

template<typename T, bool throwOnNull = true>
struct ThreadLocalProxy {
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

/**
 * The emulation version of the thread-local macros
 */
#define THREAD_LOCAL(T, f) HPHP::ThreadLocal<T> f
#define THREAD_LOCAL_NO_CHECK(T, f) HPHP::ThreadLocalNoCheck<T> f
#define THREAD_LOCAL_PROXY(T, N, f) HPHP::ThreadLocalProxy<T, N> f
}

#endif
