/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_SMART_PTR_H_
#define incl_HPHP_SMART_PTR_H_

#include "hphp/runtime/base/types.h"
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

template<typename T>
class SmartPtr final {
public:
  SmartPtr() : m_px(nullptr) {}
  /* implicit */ SmartPtr(std::nullptr_t) : m_px(nullptr) { }
  explicit SmartPtr(T* px) : m_px(px) {
    if (LIKELY(m_px != nullptr)) m_px->incRefCount();
  }
  SmartPtr(const SmartPtr<T>& src) : m_px(src.get()) {
    if (LIKELY(m_px != nullptr)) m_px->incRefCount();
  }

  enum class IsUnowned {};
  SmartPtr(T* px, IsUnowned) : m_px(px) {
    assert(!m_px || m_px->getCount() == 0);
    if (LIKELY(m_px != nullptr)) m_px->setRefCount(1);
  }

  enum class NoIncRef {};
  explicit SmartPtr(T* px, NoIncRef) : m_px(px) {}

  enum class NonNull {};
  explicit SmartPtr(T* px, NonNull) : m_px(px) {
    assert(px);
    m_px->incRefCount();
  }

  enum class UnownedAndNonNull {};
  SmartPtr(T* px, UnownedAndNonNull) : m_px(px) {
    assert(m_px && m_px->getCount() == 0);
    m_px->setRefCount(1);
  }

  // Move ctor
  SmartPtr(SmartPtr&& src) noexcept : m_px(src.get()) {
    src.m_px = nullptr;
  }

  template<class Y>
  SmartPtr(const SmartPtr<Y>& src) : m_px(src.get()) {
    if (LIKELY(m_px != nullptr)) m_px->incRefCount();
  }
  // Move ctor for derived types
  template<class Y>
  SmartPtr(SmartPtr<Y>&& src) : m_px(src.get()) {
    src.m_px = nullptr;
  }

  ~SmartPtr() {
    decRefPtr(m_px);
    if (debug) {
      m_px = reinterpret_cast<T*>(0xdeadbeeffaceb004);
    }
  }

  /**
   * Assignments.
   */
  SmartPtr& operator=(const SmartPtr<T>& src) {
    return operator=(src.m_px);
  }
  // Move assignment
  SmartPtr& operator=(SmartPtr&& src) {
    std::swap(m_px, src.m_px);
    return *this;
  }
  template<class Y>
  SmartPtr& operator=(const SmartPtr<Y>& src) {
    return operator=(src.get());
  }
  // Move assignment for derived types
  template<class Y>
  SmartPtr& operator=(SmartPtr<Y>&& src) {
    assert((void*)this != (void*)&src);
    // Update m_px before releasing the goner
    auto goner = m_px;
    m_px = src.m_px;
    src.m_px = nullptr;
    decRefPtr(goner);
    return *this;
  }
  SmartPtr& operator=(T* px) {
    // Works with self-assignment because incRefCount is
    // called before decRefCount.
    if (LIKELY(px != nullptr)) px->incRefCount();
    auto goner = m_px;
    m_px = px;
    decRefPtr(goner);
    return *this;
  }

  /**
   * Swap two smart pointers
   */
  void swap(SmartPtr& rhs) {
    std::swap(m_px, rhs.m_px);
  }

  /**
   * Safe bool cast.
   */
  explicit operator bool() const { return m_px != nullptr; }

  /**
   * Get count.
   */
  RefCount use_count() const { return m_px ? m_px->getCount() : 0; }

  /**
   * Check if it is unique.
   */
  bool unique() const { return m_px && m_px->hasExactlyOneRef(); }

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
  static SmartPtr<Y> attach(Y* ptr) {
    SmartPtr<Y> sp;
    sp.m_px = ptr;
    return sp;
  }

private:
  // For templatized SmartPtr<Y> move constructor.
  template <typename Y> friend class SmartPtr;

  static ALWAYS_INLINE void decRefPtr(T* ptr) {
    if (LIKELY(ptr != nullptr)) ptr->decRefAndRelease();
  }
  static void compileTimeAssertions() {
    static_assert(offsetof(SmartPtr, m_px) == kExpectedMPxOffset, "");
  }

  T* m_px;  // raw pointer
};

///////////////////////////////////////////////////////////////////////////////
// SmartPtr helper functions

// Note: don't define the other relational (<,>,<=,=>) operators in
// terms of the underlying pointer.  If the underlying pointer is
// moved (e.g. by GC) the relation may no longer hold.

template <typename T>
struct isValidSmartPtrRelopArg {
  enum { value = false };
};

template <>
struct isValidSmartPtrRelopArg<std::nullptr_t> {
  enum { value = true };
};

template <typename T>
struct isValidSmartPtrRelopArg<SmartPtr<T>> {
  enum { value = true };
};

template <typename T>
struct isValidSmartPtrRelopArg<T*> {
  enum { value = true };
};

template <typename T>
struct isValidSmartPtrRelopArg<const T*> {
  enum { value = true };
};

template <typename T>
struct isSmartPtr {
  enum { value = false };
};

template <typename T>
struct isSmartPtr<SmartPtr<T>> {
  enum { value = true };
};

template <typename T>
inline T* deref(const SmartPtr<T>& p) { return p.get(); }

template <typename T>
inline const T* deref(const T* p) { return p; }

inline std::nullptr_t deref(std::nullptr_t) { return nullptr; }

template <typename A, typename B>
inline
typename std::enable_if<
  (isValidSmartPtrRelopArg<A>::value && isValidSmartPtrRelopArg<B>::value),
  bool
>::type operator==(const A& a, const B& b) {
  return deref(a) == deref(b);
}

template <typename A, typename B>
inline
typename std::enable_if<
  (isValidSmartPtrRelopArg<A>::value && isValidSmartPtrRelopArg<B>::value),
  bool
>::type operator!=(const A& a, const B& b) {
  return !(a == b);
}

template <typename T, typename P>
inline auto detach(P&& p) -> decltype(p.detach()) {
  return p.detach();
}

template <typename T, typename P>
inline auto deref(const P& p) -> decltype(p.get()) {
  return p.get();
}

const char* getClassNameCstr(const ResourceData* r);
const char* getClassNameCstr(const ObjectData* o);

template <typename T, typename P>
inline const char* getClassNameCstr(const P& v) {
  return getClassNameCstr(deref<T>(v));
}

extern void throw_null_pointer_exception() ATTRIBUTE_NORETURN;
extern void throw_invalid_object_type(const char* clsName) ATTRIBUTE_NORETURN;

template <typename T>
inline bool is_null(const T& p) {
  return !p;
}

template <typename T, typename P>
inline bool isa_non_null(const P& p) {
  assert(!is_null(p));
  return p->template instanceof<T>();
}

// Is pointer contained in p castable to a T?
template <typename T, typename P>
inline bool isa(const P& p) {
  return !is_null(p) && isa_non_null<T>(p);
}

// Is pointer contained in p null or castable to a T?
template <typename T, typename P>
inline bool isa_or_null(const P& p) {
  return is_null(p) || isa_non_null<T>(p);
}

// Perform an unsafe cast operation on p.  The value p is assumed
// to be castable to T (checked by assertion).  Null pointers will
// be passed through.
template <typename T, typename P>
struct unsafe_cast_helper {
  SmartPtr<T> operator()(P&& p) const {
    return SmartPtr<T>::attach(static_cast<T*>(detach<T>(std::move(p))));
  }
  SmartPtr<T> operator()(const P& p) const {
    return SmartPtr<T>(static_cast<T*>(deref<T>(p)));
  }
};

template <typename T, typename P>
inline SmartPtr<T> unsafe_cast_or_null(P&& p) {
  using decayedP = typename std::decay<P>::type;
  return unsafe_cast_helper<T,decayedP>()(std::forward<P>(p));
}

// Perform an unsafe cast operation on p.  The value p is assumed
// to be castable to T (checked by assertion).  Null pointers will
// result in an exception.
template <typename T, typename P>
inline SmartPtr<T> unsafe_cast(P&& p) {
  if (!is_null(p)) {
    return unsafe_cast_or_null<T>(std::forward<P>(p));
  }
  throw_null_pointer_exception();
}

// Perform a cast operation on p.  If p is null or not castable to
// a T, an exception will be thrown.
template <typename T, typename P>
inline SmartPtr<T> cast(P&& p) {
  if (!is_null(p)) {
    if (isa_non_null<T>(p)) {
      return unsafe_cast_or_null<T>(std::forward<P>(p));
    }
    throw_invalid_object_type(getClassNameCstr<T>(p));
  }
  throw_null_pointer_exception();
}

// Perform a cast operation on p.  If p is not castable to
// a T, an exception will be thrown.  Null pointers will be
// passed through.
template <typename T, typename P>
inline SmartPtr<T> cast_or_null(P&& p) {
  if (!is_null(p)) {
    if (isa_non_null<T>(p)) {
      return unsafe_cast_or_null<T>(std::forward<P>(p));
    }
    throw_invalid_object_type(getClassNameCstr<T>(p));
  }
  return nullptr;
}

// Perform a cast operation on p.  If p not castable to
// a T, a null value is returned.  If p is null, an exception
// is thrown.
template <typename T, typename P>
inline SmartPtr<T> dyn_cast(P&& p) {
  if (!is_null(p)) {
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
inline SmartPtr<T> dyn_cast_or_null(P&& p) {
  return isa<T>(p) ? unsafe_cast_or_null<T>(std::forward<P>(p)) : nullptr;
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

namespace std {
template<class T>
struct hash<HPHP::SmartPtr<T>> {
  size_t operator()(const HPHP::SmartPtr<T>& p) const {
    return std::hash<T*>()(p.get());
  }
};
}

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_SMART_PTR_H_
