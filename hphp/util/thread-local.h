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

#ifndef incl_HPHP_THREAD_LOCAL_H_
#define incl_HPHP_THREAD_LOCAL_H_

#include <cerrno>
#include <pthread.h>
#include <type_traits>

#include <folly/String.h>

#include "hphp/util/exception.h"
#include "hphp/util/type-scan.h"

namespace HPHP {

// return the location of the current thread's tdata section
std::pair<void*,size_t> getCppTdata();

#ifdef _MSC_VER
extern "C" int _tls_index;
#endif

inline uintptr_t tlsBase() {
  uintptr_t retval;
#if defined(__x86_64__)
  asm ("movq %%fs:0, %0" : "=r" (retval));
#elif defined(__AARCH64EL__)
  // mrs == "move register <-- system"
  // tpidr_el0 == "thread process id register for exception level 0"
  asm ("mrs %0, tpidr_el0" : "=r" (retval));
#elif defined (__powerpc64__)
  asm ("xor %0,%0,%0\n\t"
       "or  %0,%0,13\n\t"
      : "=r" (retval));
#elif defined(_M_X64)
  retval = (uintptr_t)__readgsqword(88);
  retval = *(uintptr_t*)(retval + (_tls_index * 8));
#else
# error How do you access thread-local storage on this machine?
#endif
  return retval;
}

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
//
// __thread on cygwin and mingw uses pthreads emulation not native tls so
// the emulation for thread local must be used as well
//
// So we use __thread on gcc, icc and clang, unless we are on OSX. On OSX, we
// use our own emulation. Use the DECLARE_THREAD_LOCAL() and
// IMPLEMENT_THREAD_LOCAL() macros to access either __thread or the emulation
// as appropriate.

#if !defined(NO_TLS) &&                                       \
   ((__llvm__ && __clang__) ||                                \
   __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 3) ||   \
   __INTEL_COMPILER || defined(_MSC_VER))
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

#ifdef __APPLE__
typedef struct __darwin_pthread_handler_rec darwin_pthread_handler;
#endif

///////////////////////////////////////////////////////////////////////////////

/**
 * A thread-local object is a "global" object within a thread. This is useful
 * for writing apartment-threaded code, where nothing is actually shared
 * between different threads (hence no locking) but those variables are not
 * on stack in local scope. To use it, just do something like this,
 *
 *   IMPLEMENT_THREAD_LOCAL(MyClass, static_object);
 *     static_object->data_ = ...;
 *     static_object->doSomething();
 *
 *   IMPLEMENT_THREAD_LOCAL(int, static_number);
 *     int value = *static_number;
 *
 * So, syntax-wise it's similar to pointers. The type parameter can be a
 * primitive types. If it's a class, there has to be a default constructor.
 */

///////////////////////////////////////////////////////////////////////////////
#if defined(USE_GCC_FAST_TLS)

/**
 * We keep a linked list of destructors in ThreadLocalManager to be called on
 * thread exit. ThreadLocalNode is a node in this list.
 */
template <typename T>
struct ThreadLocalNode {
  T * m_p;
  void (*m_on_thread_exit_fn)(void * p);
  void * m_next;
  uint32_t m_size;
  type_scan::Index m_tyindex;
};

struct ThreadLocalManager {
  template<class T>
  static void PushTop(ThreadLocalNode<T>& node) {
    static_assert(sizeof(T) <= 0xffffffffu, "");
    PushTop(&node, sizeof(T), type_scan::getIndexForScan<T>());
  }
  template<class Fn> void iterate(Fn fn) const;
  static ThreadLocalManager& GetManager();

private:
  static void PushTop(void* node, uint32_t size, type_scan::Index);
  struct ThreadLocalList {
    void* head{nullptr};
#ifdef __APPLE__
    ThreadLocalList();
    darwin_pthread_handler handler;
#endif
  };
  static ThreadLocalList* getList(void* p) {
    return static_cast<ThreadLocalList*>(p);
  }
  ThreadLocalManager() : m_key(0) {
#ifdef __APPLE__
    ThreadLocalCreateKey(&m_key, nullptr);
#else
    ThreadLocalCreateKey(&m_key, ThreadLocalManager::OnThreadExit);
#endif
  };
  static void OnThreadExit(void *p);
  pthread_key_t m_key;
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

/**
 * The USE_GCC_FAST_TLS implementation of ThreadLocal is just a lazy-initialized
 * pointer wrapper. In this case, we have one ThreadLocal object per thread.
 */
template<typename T>
struct ThreadLocal {
  T *get() const {
    if (m_node.m_p == nullptr) {
      const_cast<ThreadLocal<T>*>(this)->create();
    }
    return m_node.m_p;
  }

  NEVER_INLINE void create();

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
    ThreadLocalManager::PushTop(m_node);
  }
  assert(m_node.m_p == nullptr);
  m_node.m_p = new T();
}

/**
 * ThreadLocalNoCheck is a pointer wrapper like ThreadLocal, except that it is
 * explicitly initialized with getCheck(), rather than being initialized when
 * it is first dereferenced.
 */
template<typename T>
struct ThreadLocalNoCheck {
  NEVER_INLINE T *getCheck() const;
  T* getNoCheck() const {
    assert(m_node.m_p);
    return m_node.m_p;
  }

  NEVER_INLINE void create();

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
private:
  void setNull() { m_node.m_p = nullptr; }
};

template<typename T>
void ThreadLocalNoCheck<T>::create() {
  if (m_node.m_on_thread_exit_fn == nullptr) {
    m_node.m_on_thread_exit_fn = ThreadLocalOnThreadExit<T>;
    ThreadLocalManager::PushTop(m_node);
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

/*
 * T must define:
 *
 *   static void Create(void* storage)
 *     which should call new (storage) T, and is called on first getCheck.
 *
 *   static void Delete(T* singleton), and
 *   static void OnThreadExit(T* singleton)
 *     which should both call singleton->~T; Delete is called on manual
 *     destruction, while OnThreadExit is called automatically. The argument
 *     'singleton' is redundant (getters still work), but is for convenience.
 *     These are only called if the singleton was actually created.
 */
template <typename T>
struct ThreadLocalSingleton {
  // Call once per process just to guarantee order of initialization.
  ThreadLocalSingleton() { s_inited = true; }

  NEVER_INLINE static T *getCheck();

  static T* getNoCheck() {
    assert(s_inited);
    assert(singleton() == (T*)&s_storage);
    return (T*)&s_storage;
  }

  static bool isNull() { return singleton() == nullptr; }

  static void destroy() {
    assert(!singleton() || singleton() == (T*)&s_storage);
    T* p = singleton();
    if (p) {
      T::Delete(p);
      s_node.m_p = nullptr;
    }
  }

  T *operator->() const {
    return getNoCheck();
  }

  T &operator*() const {
    return *getNoCheck();
  }

private:
  using NodeType = ThreadLocalNode<T>;

  static T* singleton() {
    return s_node.m_p;
  }

  static void OnThreadExit(void* p) {
    NodeType* pNode = (NodeType*)p;
    assert(pNode == &s_node);
    if (pNode->m_p) {
      T::OnThreadExit(pNode->m_p);
      pNode->m_p = nullptr;
    }
  }

  static __thread NodeType s_node;
  typedef typename std::aligned_storage<sizeof(T), 16>::type
          StorageType;
  static __thread StorageType s_storage;
  static bool s_inited; // no-fast-TLS requires construction so be consistent
};

template<typename T>
bool ThreadLocalSingleton<T>::s_inited = false;

template<typename T>
T *ThreadLocalSingleton<T>::getCheck() {
  assert(s_inited);
  if (!singleton()) {
    T* p = (T*) &s_storage;
    T::Create(p);
    s_node.m_p = p;
    // Register exit hook at most once; doing it twice makes TLM's list cyclic.
    if (!s_node.m_on_thread_exit_fn) {
      s_node.m_on_thread_exit_fn = OnThreadExit;
      ThreadLocalManager::PushTop(s_node);
    }
  }
  return singleton();
}

template<typename T> __thread typename ThreadLocalSingleton<T>::NodeType
                              ThreadLocalSingleton<T>::s_node;
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
 *   struct SomeClass {
 *     static DECLARE_THREAD_LOCAL(SomeFieldType, f);
 *   }
 *
 * Use IMPLEMENT_THREAD_LOCAL in the cpp file to implement the field:
 *   IMPLEMENT_THREAD_LOCAL(SomeFieldType, SomeClass::f);
 *
 * Remember: *Never* write IMPLEMENT_THREAD_LOCAL in a header file.
 */

#define DECLARE_THREAD_LOCAL(T, f) \
  __thread HPHP::ThreadLocal<T> f
#define IMPLEMENT_THREAD_LOCAL(T, f) \
  __thread HPHP::ThreadLocal<T> f

#define DECLARE_THREAD_LOCAL_NO_CHECK(T, f) \
  __thread HPHP::ThreadLocalNoCheck<T> f
#define IMPLEMENT_THREAD_LOCAL_NO_CHECK(T, f) \
  __thread HPHP::ThreadLocalNoCheck<T> f

#define DECLARE_THREAD_LOCAL_PROXY(T, N, f) \
  __thread HPHP::ThreadLocalProxy<T, N> f
#define IMPLEMENT_THREAD_LOCAL_PROXY(T, N, f) \
  __thread HPHP::ThreadLocalProxy<T, N> f

#else /* USE_GCC_FAST_TLS */

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
typedef struct __darwin_pthread_handler_rec darwin_pthread_handler;

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
template<typename T>
struct ThreadLocal {
  /**
   * Constructor that has to be called from a thread-neutral place.
   */
  ThreadLocal() : m_key(0) {
#ifdef __APPLE__
    ThreadLocalCreateKey(&m_key, nullptr);
    ThreadLocalCreateKey(&m_cleanup_key,
                         ThreadLocalOnThreadExit<darwin_pthread_handler>);
#else
    ThreadLocalCreateKey(&m_key, ThreadLocalOnThreadExit<T>);
#endif
  }

  T *get() const {
    T *obj = (T*)pthread_getspecific(m_key);
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
  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

private:
  pthread_key_t m_key;

#ifdef __APPLE__
  pthread_key_t m_cleanup_key;
#endif
};

template<typename T>
struct ThreadLocalNoCheck {
  /**
   * Constructor that has to be called from a thread-neutral place.
   */
  ThreadLocalNoCheck() : m_key(0) {
#ifdef __APPLE__
    ThreadLocalCreateKey(&m_key, nullptr);
    ThreadLocalCreateKey(&m_cleanup_key,
                         ThreadLocalOnThreadExit<darwin_pthread_handler>);
#else
    ThreadLocalCreateKey(&m_key, ThreadLocalOnThreadExit<T>);
#endif
  }

  NEVER_INLINE T *getCheck() const;

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
  void setNull() { ThreadLocalSetValue(m_key, nullptr); }
  pthread_key_t m_key;

#ifdef __APPLE__
  pthread_key_t m_cleanup_key;
#endif
};

template<typename T>
T *ThreadLocalNoCheck<T>::getCheck() const {
  T *obj = (T*)pthread_getspecific(m_key);
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

///////////////////////////////////////////////////////////////////////////////
// Singleton thread-local storage for T

template<typename T>
void ThreadLocalSingletonOnThreadExit(void *obj) {
  T::OnThreadExit((T*)obj);
  free(obj);
}

#ifdef __APPLE__
template<typename T>
void ThreadLocalSingletonOnThreadCleanup(void *key) {
  void *obj = pthread_getspecific((pthread_key_t)key);
  if (obj) {
    ThreadLocalSingletonOnThreadExit<T>(obj);
  }
}
#endif

/*
 * See fast-TlS version of ThreadLocalSingleton above for documentation.
 */
template<typename T>
struct ThreadLocalSingleton {
  ThreadLocalSingleton() { getKey(); }

  NEVER_INLINE static T *getCheck();
  static T* getNoCheck() {
    assert(s_inited);
    T *obj = (T*)pthread_getspecific(s_key);
    assert(obj);
    return obj;
  }

  static bool isNull() {
    return !s_inited || pthread_getspecific(s_key) == nullptr;
  }

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

#ifdef __APPLE__
  static pthread_key_t s_cleanup_key;
#endif

  static pthread_key_t getKey() {
    if (!s_inited) {
      s_inited = true;
#ifdef __APPLE__
      ThreadLocalCreateKey(&s_key, nullptr);
      ThreadLocalCreateKey(&s_cleanup_key,
                           ThreadLocalOnThreadExit<darwin_pthread_handler>);
#else
      ThreadLocalCreateKey(&s_key, ThreadLocalSingletonOnThreadExit<T>);
#endif
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
#ifdef __APPLE__
    ThreadLocalSetCleanupHandler(s_cleanup_key, s_key,
                                 ThreadLocalSingletonOnThreadCleanup<T>);
#endif
  }
  return obj;
}

template<typename T>
pthread_key_t ThreadLocalSingleton<T>::s_key;
template<typename T>
bool ThreadLocalSingleton<T>::s_inited = false;

#ifdef __APPLE__
template<typename T>
pthread_key_t ThreadLocalSingleton<T>::s_cleanup_key;
#endif

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
#define DECLARE_THREAD_LOCAL(T, f) HPHP::ThreadLocal<T> f
#define IMPLEMENT_THREAD_LOCAL(T, f) HPHP::ThreadLocal<T> f

#define DECLARE_THREAD_LOCAL_NO_CHECK(T, f) HPHP::ThreadLocalNoCheck<T> f
#define IMPLEMENT_THREAD_LOCAL_NO_CHECK(T, f) HPHP::ThreadLocalNoCheck<T> f

#define DECLARE_THREAD_LOCAL_PROXY(T, N, f) HPHP::ThreadLocalProxy<T, N> f
#define IMPLEMENT_THREAD_LOCAL_PROXY(T, N, f) HPHP::ThreadLocalProxy<T, N> f

#endif /* USE_GCC_FAST_TLS */

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_THREAD_LOCAL_H_
