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
  asm ("mr  %0,13" : "=r" (retval));
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
// So we use __thread on gcc, icc and clang, and OSX, falling back to
// emulation on unsupported platforms. Use the THREAD_LOCAL() macro to
// hide the decl details.
//
// See thread-local-emulate.h for the emulated versions; they're in a
// separate header to avoid confusion; this is a long header file and it's
// easy to lose track of which version you're looking at.

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

} // namespace HPHP

///////////////////////////////////////////////////////////////////////////////

/**
 * A thread-local object is a "global" object within a thread. This is useful
 * for writing apartment-threaded code, where nothing is actually shared
 * between different threads (hence no locking) but those variables are not
 * on stack in local scope. To use it, just do something like this,
 *
 *   THREAD_LOCAL(MyClass, static_object);
 *     static_object->data_ = ...;
 *     static_object->doSomething();
 *
 *   THREAD_LOCAL(int, static_number);
 *     int value = *static_number;
 *
 * So, syntax-wise it's similar to pointers. The type parameter can be a
 * primitive types. If it's a class, there has to be a default constructor.
 */

///////////////////////////////////////////////////////////////////////////////
#if defined(USE_GCC_FAST_TLS)

namespace HPHP {
/**
 * We keep a linked list of destructors in ThreadLocalManager to be called on
 * thread exit. ThreadLocalNode is a node in this list.
 */
template <typename T>
struct ThreadLocalNode {
  T* m_p;
  void (*m_on_thread_exit_fn)(void * p);
  void (*m_init_tyindex)(void*);
  void* m_next;
  uint32_t m_size;
  type_scan::Index m_tyindex;
  static void init_tyindex(void* node_ptr) {
    auto node = static_cast<ThreadLocalNode<T>*>(node_ptr);
    node->m_tyindex = type_scan::getIndexForScan<T>();
  }
};

struct ThreadLocalManager {
  template<class T>
  static void PushTop(ThreadLocalNode<T>& node) {
    static_assert(sizeof(T) <= 0xffffffffu, "");
    PushTop(&node, sizeof(T), type_scan::getIndexForScan<T>());
    node.m_init_tyindex = &node.init_tyindex;
  }
  template<class Fn> void iterate(Fn fn) const;
  void initTypeIndices();
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
template<bool Check, typename T>
struct ThreadLocalImpl {

  NEVER_INLINE T* getCheck() const {
    return get();
  }

  T* getNoCheck() const {
    assert(m_node.m_p);
    return m_node.m_p;
  }

  T* get() const {
    if (m_node.m_p == nullptr) {
      const_cast<ThreadLocalImpl<Check,T>*>(this)->create();
    }
    return m_node.m_p;
  }

  bool isNull() const { return m_node.m_p == nullptr; }

  void destroy() {
    delete m_node.m_p;
    m_node.m_p = nullptr;
  }

  void nullOut() {
    m_node.m_p = nullptr;
  }

  T* operator->() const {
    return Check ? get() : getNoCheck();
  }

  T& operator*() const {
    return Check ? *get() : *getNoCheck();
  }

  static size_t node_ptr_offset() {
    using Self = ThreadLocalImpl<Check,T>;
    return offsetof(Self, m_node) + offsetof(ThreadLocalNode<T>, m_p);
  }

 private:
  NEVER_INLINE void create();
  ThreadLocalNode<T> m_node;
};

template<bool Check, typename T>
void ThreadLocalImpl<Check,T>::create() {
  if (m_node.m_on_thread_exit_fn == nullptr) {
    m_node.m_on_thread_exit_fn = ThreadLocalOnThreadExit<T>;
    ThreadLocalManager::PushTop(m_node);
  }
  assert(m_node.m_p == nullptr);
  m_node.m_p = new T();
}

template<typename T> using ThreadLocal = ThreadLocalImpl<true,T>;
template<typename T> using ThreadLocalNoCheck = ThreadLocalImpl<false,T>;

///////////////////////////////////////////////////////////////////////////////
// some classes don't need new/delete at all

template<typename T>
struct ThreadLocalProxy {
  T *get() const {
    return m_p;
  }

  void set(T* obj) {
    if (!m_node.m_on_thread_exit_fn) {
      m_node.m_on_thread_exit_fn = onThreadExit;
      ThreadLocalManager::PushTop(m_node);
      assert(!m_node.m_p);
      m_node.m_p = this;
    } else {
      assert(m_node.m_p == this);
    }
    m_p = obj;
  }

  bool isNull() const { return m_p == nullptr; }

  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

  static void onThreadExit(void* p) {
    auto node = (ThreadLocalNode<ThreadLocalProxy<T>>*)p;
    node->m_p = nullptr;
  }

  T* m_p;
  ThreadLocalNode<ThreadLocalProxy<T>> m_node;
  TYPE_SCAN_IGNORE_FIELD(m_node);
};

// Wraps a __thread storage instance of T with a similar api to
// ThreadLocalProxy. Importantly, inlining a method of T via operator-> or
// operator* allows the C++ compiler to emit direct access to fields of
// T using the native thread-local segment pointer.
template<typename T>
struct ThreadLocalFlat {
  T* get() const { return (T*)&m_value; }
  bool isNull() const { return !m_node.m_p; }
  T* getNoCheck() const { return get(); }
  T* operator->() const { return get(); }
  T& operator*() const { return *get(); }
  explicit operator bool() const { return !isNull(); }
  static void onThreadExit(void* p) {
    auto node = (ThreadLocalNode<ThreadLocalFlat<T>>*)p;
    auto value = (T*)&node->m_p->m_value;
    value->~T();
    node->m_p = nullptr;
  }
  T* getCheck() {
    if (!m_node.m_p) {
      if (!m_node.m_on_thread_exit_fn) {
        m_node.m_on_thread_exit_fn = onThreadExit;
        ThreadLocalManager::PushTop(m_node);
      }
      new (&m_value) T();
      m_node.m_p = this;
    } else {
      assert(m_node.m_p == this);
      assert(m_node.m_on_thread_exit_fn);
    }
    return get();
  }

  void destroy() {
    if (m_node.m_p) {
      get()->~T();
      m_node.m_p = nullptr;
    }
  }

  // We manage initialization explicitly
  typename std::aligned_storage<sizeof(T),alignof(T)>::type m_value;
  ThreadLocalNode<ThreadLocalFlat<T>> m_node;
  TYPE_SCAN_CUSTOM() {
    scanner.scan(*get());
  }
};

/*
 * How to use the thread-local macros:
 *
 * Use THREAD_LOCAL to declare a *static* class field or global as thread local:
 *   struct SomeClass {
 *     static THREAD_LOCAL(SomeFieldType, field);
 *   };
 *   extern THREAD_LOCAL(SomeGlobal, tl_myglobal);
 *
 * Use THREAD_LOCAL in the cpp file to implement the field:
 *   THREAD_LOCAL(SomeFieldType, SomeClass::f);
 *   THREAD_LOCAL(SomeGlobal, tl_myglobal);
 */

#define THREAD_LOCAL(T, f) __thread HPHP::ThreadLocal<T> f
#define THREAD_LOCAL_NO_CHECK(T, f) __thread HPHP::ThreadLocalNoCheck<T> f
#define THREAD_LOCAL_PROXY(T, f) __thread HPHP::ThreadLocalProxy<T> f
#define THREAD_LOCAL_FLAT(T, f) __thread HPHP::ThreadLocalFlat<T> f

} // namespace HPHP

#else /* USE_GCC_FAST_TLS */
#include "hphp/util/thread-local-emulate.h"
#endif /* USE_GCC_FAST_TLS */

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_THREAD_LOCAL_H_
