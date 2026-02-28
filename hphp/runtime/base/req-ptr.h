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

#pragma once

#include "hphp/runtime/base/countable.h"
#include "hphp/util/portability.h"
#include "hphp/util/compilation-flags.h"
#include <algorithm>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Because of type punning with the *NR classes (ArrNR, StrNR, etc),
 * we want to assert that offsetof(T, m_px) is a specific value in all
 * these classes.
 */
const std::size_t kExpectedMPxOffset = 0;

namespace req {

template<typename T> struct ptr final {
  ptr() : m_px(nullptr) {}
  /* implicit */ ptr(std::nullptr_t) : m_px(nullptr) { }
  explicit ptr(T* px) : m_px(px) {
    if (LIKELY(m_px != nullptr)) m_px->incRefCount();
  }
  ptr(const ptr<T>& src) : m_px(src.get()) {
    if (LIKELY(m_px != nullptr)) m_px->incRefCount();
  }

  enum class NoIncRef {};
  explicit ptr(T* px, NoIncRef) : m_px(px) {}

  enum class NonNull {};
  explicit ptr(T* px, NonNull) : m_px(px) {
    assertx(px);
    m_px->incRefCount();
  }

  // Move ctor
  ptr(ptr&& src) noexcept : m_px(src.get()) {
    src.m_px = nullptr;
  }

  template<class Y>
  ptr(const ptr<Y>& src) : m_px(src.get()) {
    if (LIKELY(m_px != nullptr)) m_px->incRefCount();
  }
  // Move ctor for derived types
  template<class Y>
  ptr(ptr<Y>&& src) : m_px(src.get()) {
    src.m_px = nullptr;
  }

  ~ptr() {
    decRefPtr(m_px);
    if (debug) {
      m_px = reinterpret_cast<T*>(0xdeadbeeffaceb004);
    }
  }

  /**
   * Assignments.
   */
  ptr& operator=(const ptr<T>& src) {
    return operator=(src.m_px);
  }
  // Move assignment
  ptr& operator=(ptr&& src) {
    std::swap(m_px, src.m_px);
    return *this;
  }
  template<class Y>
  ptr& operator=(const ptr<Y>& src) {
    return operator=(src.get());
  }
  // Move assignment for derived types
  template<class Y>
  ptr& operator=(ptr<Y>&& src) {
    assertx((void*)this != (void*)&src);
    // Update m_px before releasing the goner
    auto goner = m_px;
    m_px = src.m_px;
    src.m_px = nullptr;
    decRefPtr(goner);
    return *this;
  }
  ptr& operator=(T* px) {
    // Works with self-assignment because incRefCount is
    // called before decRefCount.
    if (LIKELY(px != nullptr)) px->incRefCount();
    auto goner = m_px;
    m_px = px;
    decRefPtr(goner);
    return *this;
  }

  /**
   * Swap two pointers
   */
  void swap(ptr& rhs) {
    std::swap(m_px, rhs.m_px);
  }

  /**
   * Safe bool cast.
   */
  explicit operator bool() const { return m_px != nullptr; }

  /**
   * Magic delegation.
   */
  T* operator->() const {
    return m_px; // intentionally skip nullptr check.
  }

  /**
   * Get the raw pointer.
   */
  T* get() const {
    return m_px;
  }

  /**
   * Reset the raw pointer.  This will increment the ref count on ptr
   * (when it is non-null) and decrement the ref count on the old pointer
   * being replaced.
   */
  void reset(T* ptr = nullptr) {
    operator=(ptr);
  }

  /**
   * Release the raw pointer.
   * Note: be careful when using this.  It does not decrement the
   * ref count on m_px.
   */
  T* detach() {
    T* ptr = m_px;
    m_px = nullptr;
    return ptr;
  }

  /**
   * Take ownership of a pointer without touching the ref count.
   */
  template <typename Y>
  static ptr<Y> attach(Y* y) {
    ptr<Y> py;
    py.m_px = y;
    return py;
  }

  /**
   * Modify the value contained using the operation provided. The operation is
   * assumed to decrement the ref count on the old value if it returns a new
   * pointer.
   */
  template <typename X>
  void mutateInPlace(X op) {
    auto const newVal = op(m_px);
    m_px = newVal;
  }

private:
  // For templatized ptr<Y> move constructor.
  template <typename Y> friend struct ptr;

  static ALWAYS_INLINE void decRefPtr(T* ptr) {
    if (LIKELY(ptr != nullptr)) ptr->decRefAndRelease();
  }
  static void compileTimeAssertions() {
    static_assert(offsetof(ptr, m_px) == kExpectedMPxOffset, "");
  }

  T* m_px;  // raw pointer
};

///////////////////////////////////////////////////////////////////////////////
// req::ptr helper functions

// Note: don't define the other relational (<,>,<=,=>) operators in
// terms of the underlying pointer.  If the underlying pointer is
// moved (e.g. by GC) the relation may no longer hold.

template <typename T>
struct isValidPtrRelopArg {
  enum { value = false };
};

template <>
struct isValidPtrRelopArg<std::nullptr_t> {
  enum { value = true };
};

template <typename T>
struct isValidPtrRelopArg<req::ptr<T>> {
  enum { value = true };
};

template <typename T>
struct isValidPtrRelopArg<T*> {
  enum { value = true };
};

template <typename T>
struct isValidPtrRelopArg<const T*> {
  enum { value = true };
};

} // namespace req

template <typename T>
inline T* deref(const req::ptr<T>& p) { return p.get(); }

template <typename T>
inline const T* deref(const T* p) { return p; }

inline std::nullptr_t deref(std::nullptr_t) { return nullptr; }

template <typename A, typename B>
inline
typename std::enable_if<
  (req::isValidPtrRelopArg<A>::value && req::isValidPtrRelopArg<B>::value),
  bool
>::type operator==(const A& a, const B& b) {
  return deref(a) == deref(b);
}

template <typename A, typename B>
inline
typename std::enable_if<
  (req::isValidPtrRelopArg<A>::value && req::isValidPtrRelopArg<B>::value),
  bool
>::type operator!=(const A& a, const B& b) {
  return !(a == b);
}

template <typename T, typename P>
inline auto detach(P&& p) -> decltype(P().detach()) {
  return p.detach();
}

template <typename T, typename P>
inline auto deref(const P& p) -> decltype(P().get()) {
  return p.get();
}

struct ResourceData;
struct ObjectData;
struct OptResource;
struct Object;
struct Variant;

[[noreturn]] extern void throw_null_pointer_exception();
[[noreturn]] void throw_invalid_object_type(ResourceData* p);
[[noreturn]] void throw_invalid_object_type(ObjectData* p);
[[noreturn]] void throw_invalid_object_type(const Variant& p);
[[noreturn]] void throw_invalid_object_type(const OptResource& p);
[[noreturn]] void throw_invalid_object_type(const Object& p);
template <typename T>
[[noreturn]] void throw_invalid_object_type(const req::ptr<T>& p);
template <typename T>
void throw_invalid_object_type(const req::ptr<T>& p) {
  throw_invalid_object_type(p.get());
}

template <typename T>
inline bool ptr_is_null(const T& p) {
  return !p;
}

template <typename T, typename P>
inline bool isa_non_null(const P& p) {
  assertx(!ptr_is_null(p));
  return p->template instanceof<T>();
}

// Is pointer contained in p castable to a T?
template <typename T, typename P>
inline bool isa(const P& p) {
  return !ptr_is_null(p) && isa_non_null<T>(p);
}

// Is pointer contained in p null or castable to a T?
template <typename T, typename P>
inline bool isa_or_null(const P& p) {
  return ptr_is_null(p) || isa_non_null<T>(p);
}

// Perform an unsafe cast operation on p.  The value p is assumed
// to be castable to T (checked by assertion).  Null pointers will
// be passed through.
template <typename T, typename P>
struct unsafe_cast_helper {
  req::ptr<T> operator()(P&& p) const {
    return req::ptr<T>::attach(static_cast<T*>(detach<T>(std::move(p))));
  }
  req::ptr<T> operator()(const P& p) const {
    return req::ptr<T>(static_cast<T*>(deref<T>(p)));
  }
};

template <typename T, typename P>
inline req::ptr<T> unsafe_cast_or_null(P&& p) {
  using decayedP = typename std::decay<P>::type;
  return unsafe_cast_helper<T,decayedP>()(std::forward<P>(p));
}

// Perform an unsafe cast operation on p.  The value p is assumed
// to be castable to T (checked by assertion).  Null pointers will
// result in an exception.
template <typename T, typename P>
inline req::ptr<T> unsafe_cast(P&& p) {
  if (!ptr_is_null(p)) {
    return unsafe_cast_or_null<T>(std::forward<P>(p));
  }
  throw_null_pointer_exception();
}

// Perform a cast operation on p.  If p is null or not castable to
// a T, an exception will be thrown.
template <typename T, typename P>
inline req::ptr<T> cast(P&& p) {
  if (!ptr_is_null(p)) {
    if (isa_non_null<T>(p)) {
      return unsafe_cast_or_null<T>(std::forward<P>(p));
    }
  }
  throw_invalid_object_type(p);
}

// Perform a cast operation on p.  If p is not castable to
// a T, an exception will be thrown.  Null pointers will be
// passed through.
template <typename T, typename P>
inline req::ptr<T> cast_or_null(P&& p) {
  if (!ptr_is_null(p)) {
    if (isa_non_null<T>(p)) {
      return unsafe_cast_or_null<T>(std::forward<P>(p));
    }
    throw_invalid_object_type(p);
  }
  return nullptr;
}

// Perform a cast operation on p.  If p not castable to
// a T, a null value is returned.  If p is null, an exception
// is thrown.
template <typename T, typename P>
inline req::ptr<T> dyn_cast(P&& p) {
  if (!ptr_is_null(p)) {
    if (isa_non_null<T>(p)) {
      return unsafe_cast_or_null<T>(std::forward<P>(p));
    }
    return nullptr;
  }
  throw_null_pointer_exception();
}

// Perform a cast operation on p.  If p is null or not castable to
// a T, a null value is returned.
template <typename T, typename P>
inline req::ptr<T> dyn_cast_or_null(P&& p) {
  return isa<T>(p) ? unsafe_cast_or_null<T>(std::forward<P>(p)) : nullptr;
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

namespace std {
template<class T>
struct hash<HPHP::req::ptr<T>> {
  size_t operator()(const HPHP::req::ptr<T>& p) const {
    return std::hash<T*>()(p.get());
  }
};
}

///////////////////////////////////////////////////////////////////////////////

