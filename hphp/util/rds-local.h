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

#include "hphp/util/alloc.h"
#include "hphp/util/assertions.h"
#include "hphp/util/optional.h"
#include "hphp/util/thread-local.h"

namespace HPHP {

struct RequestEventHandler;
struct ArrayData;

namespace rds {
namespace local {

/**
 * RDSLocals are stored in RDS on request threads, and in a malloc'ed buffer on
 * non request threads.  Their destructor is called as a RDS is cleaned up, or
 * as the backing space is cleaned up. They expose the same API as thread
 * locals, and can be used in a similar manner.  The core difference is that
 * they will be tied to a request data segment on request threads, rather than
 * a thread local section.
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
      ::HPHP::rds::local::detail::rl_hotSection.f = v; \
      return *this; \
    } \
    operator T&() { \
      return ::HPHP::rds::local::detail::rl_hotSection.f; \
    } \
  } f;

#define IMPLEMENT_RDS_LOCAL_HOTVALUE(T, f) \
  RLHotWrapper_ ## f f;

namespace detail {
struct HotRDSLocals {
  void* rdslocal_base;
  void* g_context;

  bool t_eager_gc;

  TYPE_SCAN_IGNORE_FIELD(rdslocal_base);
  TYPE_SCAN_IGNORE_FIELD(g_context);
};
static_assert(sizeof(HotRDSLocals) <= 64,
              "It is essential HotRDSLocals is small.  Consider using "
              "normal rds locals if possible.  Hot rds locals are copied "
              "every user level context switch.");

extern __thread HotRDSLocals rl_hotSection;
extern uint32_t s_usedbytes;

}

///////////////////////////////////////////////////////////////////////////////
// RDS Local

#define RDS_LOCAL(T, f) \
  ::HPHP::rds::local::RDSLocal<T, ::HPHP::rds::local::Initialize::FirstUse> f

#define RDS_LOCAL_NO_CHECK(T, f) \
  ::HPHP::rds::local::RDSLocal<T, ::HPHP::rds::local::Initialize::Explicitly> f

namespace detail {

struct RDSLocalNode {
  virtual ~RDSLocalNode() {}

  virtual void init() = 0;
  virtual void fini() = 0;

  virtual void* nodeLocation() = 0;
  virtual size_t nodeSize() = 0;
  virtual type_scan::Index nodeTypeIndex() = 0;


  RDSLocalNode* m_next;
  // s_RDSLocalsBase is the base of the RDSLocal section in RDS.  Its only
  // used for computing RDS offsets for use in the JIT.
  static uint32_t s_RDSLocalsBase;

  template<typename Fn>
  void iterateRoot(Fn fn) {
    fn(nodeLocation(), nodeSize(), nodeTypeIndex());
  }
};

extern RDSLocalNode* head;

template<typename Fn>
void iterate(Fn fn) {
  for (auto p = head; p != nullptr; p = p->m_next) {
    fn(p);
  }
}

void initializeRequestEventHandler(RequestEventHandler* h);

template<typename T, typename Enable = void>
struct RDSLocalBase;

template<typename T>
struct RDSLocalBase<T, std::enable_if_t<std::is_copy_constructible<T>::value>>
    : RDSLocalNode {
  Optional<T> m_defaultValue;
  void setDefault(const T& v) {
    m_defaultValue = v;
  }
  Optional<T> getDefault() const {
    return m_defaultValue;
  }
};

template<typename T>
struct RDSLocalBase<T, std::enable_if_t<!std::is_copy_constructible<T>::value>>
  : RDSLocalNode {
  void setDefault(const T& v) {}
  Optional<T> getDefault() const { return std::nullopt; }
};

///////////////////////////////////////////////////////////////////////////////
}

struct Configuration {
  // Returns a handle to the base of the RDS locals in the RDS region.  If no
  // RDS section is used and RDS locals are backed by other allocation, this
  // can be left as nullptr.  The parameter is the space required for the
  // RDS locals.
  uint32_t(*rdsInitFunc)(size_t) = nullptr;
  // Initializes a pointer to the base of the RDS locals area to be used by the
  // calling thread/fiber.  It is passed the size required for the RDS locals
  // (guaranteed to be identical to the size that was passed to rdsInitFunc),
  // and the handle that rdsInitFunc returned.
  void*(*initFunc)(size_t, uint32_t) = nullptr;
  // Call to deallocate the region passed as the parameter.  The parameter is
  // guaranteed to be a pointer returned by initFunc earlier.  It also will
  // not be in RDS.
  void(*finiFunc)(void*) = nullptr;
  // Returns true if the region passed as a parameter lives within RDS.  If
  // RDS is not in use, this may be left as nullptr.
  bool(*inRdsFunc)(void*, size_t) = nullptr;
  // Register the parameter to have its requestInit, and requestShutdown
  // methods called at request start and end.  If no RequestEventHandlers are
  // stored in RDS locals, then this may be left as nullptr.
  void(*initRequestEventHandler)(RequestEventHandler*) = nullptr;
};

namespace detail {
extern Configuration g_config;
}

struct RegisterConfig {
  explicit RegisterConfig(Configuration&& config) {
    detail::g_config = std::move(config);
  }
};

// RDSInit is called to configure offsets into the rds local area.
void RDSInit();
// init is called to allocate and initialize the rdslocals.
void init();
// fini deallocates the rdslocals.
void fini(bool inrds = false);

template<typename Fn>
void iterateRoots(Fn fn) {
  detail::iterate([&](detail::RDSLocalNode* p) {
    p->iterateRoot(fn);
  });
}

enum class Initialize : uint8_t {
  FirstUse,
  Explicitly,
};

template<typename T, Initialize Init>
struct RDSLocal : private detail::RDSLocalBase<T> {
  static auto constexpr REH = std::is_base_of<RequestEventHandler, T>::value;
  template<typename T1>
  using REH_t = std::enable_if_t<REH, T1>;
  template<typename T1>
  using NREH_t = std::enable_if_t<!REH, T1>;
  static_assert(Init != Initialize::Explicitly || !REH,
                "Can't explicilty initialize a subtype of RequestEventHandler");

  RDSLocal();

  template<typename T1 = T>
  explicit RDSLocal(
    const T& v,
    std::enable_if_t<std::is_copy_constructible<T1>::value>* = 0)
  : RDSLocal() {
    this->setDefault(v);
  }

  NEVER_INLINE void create();
  void destroy();
  void nullOut();

  bool isNull() const { return !(detail::rl_hotSection.rdslocal_base &&
                                 node().has_value()); }
  explicit operator bool() const { return !isNull(); }

  template<typename T1 = T, typename = decltype(std::declval<T1&>()[0])>
  decltype(std::declval<T1&>()[0]) operator[](size_t i) {
    return (*get())[i];
  }

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
    return !isNull();
  }

  template<typename B = bool>
  REH_t<B> getInited() const {
    return !isNull() && node().value().getInited();
  }

  size_t getRDSOffset() const {
    return this->s_RDSLocalsBase + m_offset;
  }

  size_t getRawOffset() const {
    return m_offset;
  }

  template <typename... Args>
  void emplace(Args&& ...args);

protected:
  template<typename T1 = T>
  std::enable_if_t<!std::is_copy_constructible<T1>::value> defaultInit() {}
  template<typename T1 = T>
  std::enable_if_t<std::is_copy_constructible<T1>::value> defaultInit() {
    if (auto const& defaultValue = this->getDefault()) {
      emplace(*defaultValue);
    }
  }

  void init() override {
    assertx(detail::rl_hotSection.rdslocal_base);
    // Initialize the Node so that it is unset.
    new (&node()) Node();
    defaultInit();
  }
  void fini() override {
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
      assertx(detail::g_config.initRequestEventHandler);
      detail::g_config.initRequestEventHandler(getNoCheck());
    }
  }
  template<typename V = void>
  NREH_t<V> rehInit() const {}

  // Node reimplements parts of what Optional supplies, however it does
  // so trusting that the user will check values are present as appropriate.
  struct Node {
    Node() : hasValue(false) {}
    ~Node() { clear(); }
    bool has_value() const noexcept { return hasValue; }
    T& value() noexcept {
      assertx(hasValue);
      return storage;
    }
    template<typename... Args>
    void emplace(Args&&... args) {
      clear();
      const void* ptr = &storage;
      new (const_cast<void*>(ptr)) T(std::forward<Args>(args)...);
      hasValue = true;
    }
    template<typename T1 = T>
    std::enable_if_t<!std::is_trivially_destructible<T1>::value, void> clear() {
      if (hasValue) {
        hasValue = false;
        storage.~T();
      }
    }
    template<typename T1 = T>
    std::enable_if_t<std::is_trivially_destructible<T1>::value, void> clear() {
      hasValue = false;
    }
    void nullOut() {
      hasValue = false;
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

  virtual Node& node() const {
    return *reinterpret_cast<Node*>(
      ((char*)detail::rl_hotSection.rdslocal_base + m_offset));
  }

  void* nodeLocation() override { return &node(); }
  size_t nodeSize() override { return sizeof(Node); }
  type_scan::Index nodeTypeIndex() override {
    return type_scan::getIndexForScan<Node>();
  }

  uint32_t m_offset;
};

// The initialisation of the RDS locals will happen at first use, or through
// explicit initialisation calls.  RDS locals are stored in the rds local
// section on request threads, so they are not assigned generation numbers, and
// are not invalidated at the end of a request.
//
// On non request threads RDS locals are stored in a block allocated in the
// heap.  This is destroyed as rds::local::fini is called.

template<typename T, Initialize Init>
RDSLocal<T, Init>::RDSLocal() {
  this->m_next = detail::head;
  detail::head = this;
  auto const align = folly::nextPowTwo(alignof(T)) - 1;
  always_assert(align < 16);
  detail::s_usedbytes = (detail::s_usedbytes + align) & ~align;
  m_offset = detail::s_usedbytes;
  detail::s_usedbytes += sizeof(Node);
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
void RDSLocal<T, Init>::nullOut() {
  if (!isNull()) {
    node().nullOut();
  }
}

template<typename T, Initialize Init>
T* RDSLocal<T, Init>::getCheck() const {
  assertx(detail::rl_hotSection.rdslocal_base);
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

template<typename T, Initialize Init>
template<typename... Args>
void RDSLocal<T, Init>::emplace(Args&&... args) {
  node().emplace(std::forward<Args>(args) ... );
}


// This aliased rds local type is the same as an rds local, except it also
// stores a pointer to the rds storage in the hot rds locals struct.  This
// saves up to 1 instruction and 1 load per access.  g_context serves as an
// example for how it is used.  It should be used sparingly as it requires
// storing a pointer in HotRDSLocals.
template<typename T, Initialize Init,
         void* detail::HotRDSLocals::*ptr>
struct AliasedRDSLocal : RDSLocal<T, Init> {
  void init() override {
    detail::rl_hotSection.*ptr = &RDSLocal<T, Init>::node();
    RDSLocal<T, Init>::init();
  }

  void fini() override {
    RDSLocal<T, Init>::fini();
    detail::rl_hotSection.*ptr = nullptr;
  }

  typename RDSLocal<T, Init>::Node& node() const override {
    assertx(detail::rl_hotSection.*ptr);
    return *(typename RDSLocal<T, Init>::Node*)(detail::rl_hotSection.*ptr);
  }
};


///////////////////////////////////////////////////////////////////////////////
// Legacy Request Local Macros (DEPRECATED)
//
// These are implemented using the same structures as above.  Please avoid
// using these.

#define IMPLEMENT_REQUEST_LOCAL(T,f)     \
  ::HPHP::rds::local::RDSLocal<T, ::HPHP::rds::local::Initialize::FirstUse> f

#define DECLARE_STATIC_REQUEST_LOCAL(T,f)    \
  static ::HPHP::rds::local::RDSLocal< \
    T, \
    ::HPHP::rds::local::Initialize::FirstUse \
  > f

#define IMPLEMENT_STATIC_REQUEST_LOCAL(T,f)     \
  static ::HPHP::rds::local::RDSLocal< \
    T, \
    ::HPHP::rds::local::Initialize::FirstUse \
  > f

#define DECLARE_EXTERN_REQUEST_LOCAL(T,f)    \
  extern ::HPHP::rds::local::RDSLocal< \
    T, \
    ::HPHP::rds::local::Initialize::FirstUse \
  > f


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
}}}

#endif // incl_HPHP_RDS_LOCAL_H_
