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

#ifndef incl_HPHP_RDS_LOCAL_H_
#define incl_HPHP_RDS_LOCAL_H_

#include "hphp/util/assertions.h"
#include "hphp/util/thread-local.h"

#include "hphp/runtime/base/rds.h"

namespace HPHP {

/**
 * RDSLocals are stored in RDS.  Their destructor is called as a RDS is
 * cleaned up. They expose the same API as thread locals, and can be used in
 * a similar manner.  The core difference is that they are tied to a request
 * data segment rather than a thread local section.
 * There are two options for how RDS locals should be initialized:
 *   - FirstUse
 *     - Similar to standard thread locals.
 *   - Explicitly
 *     - Similar to thread local no check.
 *
 * RDSLocals that inherit from RequestEventHandler have their requestInit, and
 * requestEnd methods called at first use in a request, and at RequestFini.
 * For that reason, they cannot be tagged as explicitly being initialized.
 * This is to support legacy behavior.  Favor using RDS locals that do not
 * inherit from RequestEventHandler.  Init and destroy logic can be done in
 * InitFiniNodes and extension initialisation.
 *
 * Finally, for performance purposes, extremely hot RDSLocals can be added to
 * the HotRDSLocals struct, which stores the data flat in thread local storage.
 * Swapping RDS sections requires copying this data in and out of backing RDS
 * storage.  Avoid using these if possible.
 *
 *
 * How to use rds local macros:
 *
 * Use RDS_LOCAL to declare a *static* class field or global as RDS local
 *   struct SomeClass {
 *     static RDS_LOCAL(SomeFieldType, field);
 *   };
 *   extern RDS_LOCAL(SomeGlobalType, rl_myGlobal);
 *
 * Use RDS_LOCAL in the cpp file to implement the field/global:
 *   RDS_LOCAL(SomeFieldType, SomeClass::field);
 *   RDS_LOCAL(SomeGlobalType, rl_myGlobal);
 */

///////////////////////////////////////////////////////////////////////////////
// RDS Local Hot
//
// How to use hot RDS local macros:
//
// In a header:
// extern DECLARE_RDS_LOCAL_HOTVALUE(SomeType, rl_hotGlobal);
//
// In an implementation file:
// IMPLEMENT_RDS_LOCAL_HOTVALUE(SomeType, rl_hotGlobal);
//
// Finally add rl_hotGlobal as a member to the HotRDSLocals struct.
//

#define DECLARE_RDS_LOCAL_HOTVALUE(T, f) \
  struct RLHotWrapper_ ## f { \
    RLHotWrapper_ ## f& operator=(T&& v) { \
      ::HPHP::RDSLocalDetail::rl_hotSection.f = v; \
      return *this; \
    } \
    operator T&() { \
      return ::HPHP::RDSLocalDetail::rl_hotSection.f; \
    } \
  } f;

#define IMPLEMENT_RDS_LOCAL_HOTVALUE(T, f) \
  RLHotWrapper_ ## f f;

namespace RDSLocalDetail {

struct HotRDSLocals {
  void* tl_base_copy;  // This copy of tl_base will not be VMProtected.

  TYPE_SCAN_IGNORE_FIELD(tl_base_copy);
};
static_assert(sizeof(HotRDSLocals) <= 16,
              "It is essential HotRDSLocals is small.  Consider using "
              "normal rds locals if possible.  Hot rds locals are copied "
              "every user level context switch.");

extern __thread HotRDSLocals rl_hotSection;
extern size_t s_rds_local_usedbytes;

}

///////////////////////////////////////////////////////////////////////////////
// RDS Local

#define RDS_LOCAL(T, f) \
  ::HPHP::RDSLocal<T, Initialize::FirstUse> f

#define RDS_LOCAL_NO_CHECK(T, f) \
  ::HPHP::RDSLocal<T, Initialize::Explicitly> f

struct RequestEventHandler;

namespace RDSLocalDetail {

struct RDSLocalNode {
  virtual void allocInit() = 0;
  virtual void rdsInit() = 0;
  virtual void rdsFini() = 0;
  RDSLocalNode* m_next;
};
extern RDSLocalNode* head;

enum InitType {
  RDSAllocInit,
  RDSInit,
  RDSFini,
};

template<InitType IT>
void iterate() {
  for (auto p = head; p != nullptr; p = p->m_next) {
    switch (IT) {
      case RDSAllocInit:
        p->allocInit();
        break;
      case RDSInit:
        p->rdsInit();
        break;
      case RDSFini:
        p->rdsFini();
        break;
    }
  }
}

void initializeRequestEventHandler(RequestEventHandler* h);

///////////////////////////////////////////////////////////////////////////////
}

enum class Initialize : uint8_t {
  FirstUse,
  Explicitly,
};

template<typename T, Initialize Init>
struct RDSLocal : private RDSLocalDetail::RDSLocalNode {
  static auto constexpr REH = std::is_base_of<RequestEventHandler, T>::value;
  template<typename T1>
  using REH_t = std::enable_if_t<REH, T1>;
  template<typename T1>
  using NREH_t = std::enable_if_t<!REH, T1>;
  static_assert(Init != Initialize::Explicitly || !REH,
                "Can't explicilty initialize a subtype of RequestEventHandler");

  RDSLocal();

  NEVER_INLINE void create();
  void destroy();

  bool isNull() const { return !(RDSLocalDetail::rl_hotSection.tl_base_copy &&
                                 node().has_value()); }
  explicit operator bool() const { return !isNull(); }

  T* operator->() const { return get(); }
  T& operator*() const { return *get(); }

  T* getCheck() const;
  T* get() const;

  template<typename T1 = T*>
  NREH_t<T1> getNoCheck() const {
    assertx(!isNull());
    return &node().value();
  }

  template<typename B = bool>
  NREH_t<B> getInited() const {
    !isNull();
  }

  template<typename B = bool>
  REH_t<B> getInited() const {
    return !isNull() && node().value().getInited();
  }

  size_t getRDSOffset() const {
    return m_link.handle();
  }

private:
  void allocInit() override {
    m_link = rds::alloc<Node, rds::Mode::Local>();
    RDSLocalDetail::s_rds_local_usedbytes = rds::usedLocalBytes();
  }
  void rdsInit() override {
    if (RDSLocalDetail::rl_hotSection.tl_base_copy) {
      // Initialize the Node so that it is unset.
      new (m_link.get()) Node();
    }
  }
  void rdsFini() override {
    destroy();
  }

  template<typename T1 = T*>
  REH_t<T1> getNoCheck() const {
    assertx(!isNull());
    return &node().value();
  }

  template<typename V = void>
  REH_t<V> rehInit() const {
    if (!getInited()) {
      RDSLocalDetail::initializeRequestEventHandler(getNoCheck());
    }
  }
  template<typename V = void>
  NREH_t<V> rehInit() const {}

  // Node reimplements parts of what folly::Optional supplies, however it does
  // so trusting that the user will check values are present as appropriate.
  struct Node {
    Node() : hasValue(false) {}
    ~Node() { clear(); }
    bool has_value() const noexcept { return hasValue; }
    T& value() noexcept {
      assertx(hasValue);
      return storage;
    }
    void emplace() {
      clear();
      const void* ptr = &storage;
      new (const_cast<void*>(ptr)) T();
      hasValue = true;
    }
    void clear() {
      if (hasValue) {
        hasValue = false;
        storage.~T();
      }
    }
    TYPE_SCAN_CUSTOM() {
      if (hasValue) {
        scanner.scan(storage);
      }
    }
  private:
    union {
      char empty;
      T storage;
    };
    bool hasValue;
  };

  Node& node() const {
    return *rds::handleToPtr<Node, rds::Mode::Local>(
      RDSLocalDetail::rl_hotSection.tl_base_copy,
      m_link.handle());
  }

  rds::Link<Node, rds::Mode::Local> m_link;
};

// The RDS space backing these locals is reserved as the RDS allocator is
// initialized.  The space will be zeroed as each RDS is created.  Any remaining
// RDS local will be destroyed at RDS destruction.  The initialisation of the
// RDS locals will happen at first use, or through explicit initialisation
// calls.  RDS locals are stored in the local section, so they are not
// assigned generation numbers, and are not invalidated at the end of a request.

template<typename T, Initialize Init>
RDSLocal<T, Init>::RDSLocal() {
  m_next = RDSLocalDetail::head;
  RDSLocalDetail::head = this;
}

template<typename T, Initialize Init>
void RDSLocal<T, Init>::create() {
  node().emplace();
}

template<typename T, Initialize Init>
void RDSLocal<T, Init>::destroy() {
  if (!isNull()) {
    node().clear();
  }
}

template<typename T, Initialize Init>
T* RDSLocal<T, Init>::getCheck() const {
  assertx(RDSLocalDetail::rl_hotSection.tl_base_copy);
  if (!node().has_value()) {
    const_cast<RDSLocal*>(this)->create();
  }
  rehInit();
  return getNoCheck();
}

template<typename T, Initialize Init>
T* RDSLocal<T, Init>::get() const {
  if (Init == Initialize::Explicitly) {
    return getNoCheck();
  }
  return getCheck();
}

///////////////////////////////////////////////////////////////////////////////
// Legacy Request Local Macros (DEPRECATED)
//
// These are implemented using the same structures as above.  Please avoid
// using these.

#define IMPLEMENT_REQUEST_LOCAL(T,f)     \
  HPHP::RDSLocal<T, Initialize::FirstUse> f

#define DECLARE_STATIC_REQUEST_LOCAL(T,f)    \
  static HPHP::RDSLocal<T, Initialize::FirstUse> f

#define IMPLEMENT_STATIC_REQUEST_LOCAL(T,f)     \
  static HPHP::RDSLocal<T, Initialize::FirstUse> f

#define DECLARE_EXTERN_REQUEST_LOCAL(T,f)    \
  extern HPHP::RDSLocal<T, Initialize::FirstUse> f


/**
 * Old documentation for request locals:
 *
 * vvvvvv DEPRECATED vvvvvv
 * A RequestLocal<T> is automatically thread local, plus it has two handlers
 * to do extra work on request init and shutdown times. T needs to derive from
 * RequestEventHandler, so it will register itself with execution engines to
 * be called at request shutdown time.
 *
 * Example:
 *
 *   struct MyRequestLocalClass final : RequestEventHandler {
 *     void requestInit() override {...}
 *     void requestShutdown() override {...}
 *   };
 *   IMPLEMENT_STATIC_REQUEST_LOCAL(MyRequestLocalClass, s_data);
 *
 * How to use the request-local macros:
 *
 * Use DECLARE_STATIC_REQUEST_LOCAL to declare a *static* class field as
 * request local:
 *   struct SomeClass {
 *     DECLARE_STATIC_REQUEST_LOCAL(SomeFieldType, f);
 *   }
 *
 * Use IMPLEMENT_STATIC_REQUEST_LOCAL in the cpp file to implement the field:
 *   IMPLEMENT_STATIC_REQUEST_LOCAL(SomeFieldType, SomeClass::f);
 *
 * The DECLARE_REQUEST_LOCAL and IMPLEMENT_REQUEST_LOCAL macros are provided
 * for declaring/implementing request locals in the global scope.
 *
 * Remember: *Never* write IMPLEMENT_STATIC_REQUEST_LOCAL in a header file.
 * ^^^^^^ DEPRECATED ^^^^^^
 */

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RDS_LOCAL_H_
