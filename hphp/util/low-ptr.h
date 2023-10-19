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

#include "hphp/util/low-ptr-def.h"
#include "hphp/util/assertions.h"
#include "hphp/util/portability.h"

#include <folly/Format.h>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#ifdef USE_LOWPTR
constexpr bool use_lowptr = true;

namespace detail {
using low_storage_t = uint32_t;
}
#else
constexpr bool use_lowptr = false;

namespace detail {
using low_storage_t = uintptr_t;
}
#endif

inline bool is_low_mem(void* m) {
  assertx(use_lowptr);
  const uint32_t mask = ~0;
  auto const i = reinterpret_cast<intptr_t>(m);
  return (mask & i) == i;
}

///////////////////////////////////////////////////////////////////////////////
namespace detail {

template<class T>
inline bool is_low(T* px) {
  low_storage_t ones = ~0;
  auto ptr = reinterpret_cast<uintptr_t>(px);
  return (ptr & ones) == ptr;
}

template<class T>
inline low_storage_t to_low(T* px) {
  assertx(is_low(px));
  return (low_storage_t)(reinterpret_cast<uintptr_t>(px));
}

///////////////////////////////////////////////////////////////////////////////


/**
 * Low memory pointer template.
 */
template <class T, class S>
struct LowPtrImpl {
  using storage_type = typename S::storage_type;
  enum class Unchecked {};

  /*
   * Constructors.
   */
  LowPtrImpl() {}

  /* implicit */ LowPtrImpl(T* px) : m_s{to_low(px)} {}
  /* implicit */ LowPtrImpl(std::nullptr_t /*px*/) : m_s{ 0 } {}

  LowPtrImpl(const LowPtrImpl<T, S>& r) : m_s{S::get(r.m_s)} {}

  LowPtrImpl(LowPtrImpl<T, S>&& r) : m_s{S::get(r.m_s)} {
    S::set(r.m_s, 0);
  }

  /*
   * Assignments.
   */
  LowPtrImpl& operator=(T* px) {
    S::set(m_s, to_low(px));
    return *this;
  }

  LowPtrImpl& operator=(std::nullptr_t /*px*/) {
    S::set(m_s, 0);
    return *this;
  }

  LowPtrImpl& operator=(const LowPtrImpl<T, S>& r) {
    S::set(m_s, S::get(r.m_s));
    return *this;
  }

  template <typename Q = S>
  typename std::enable_if<
    std::is_move_assignable<typename Q::storage_type>::value,
    LowPtrImpl&
  >::type operator=(LowPtrImpl<T, S>&& r) {
    m_s = std::move(r.m_s);
    return *this;
  }

  template <typename Q = S>
  typename std::enable_if<
    !std::is_move_assignable<typename Q::storage_type>::value,
    LowPtrImpl&
  >::type operator=(LowPtrImpl<T, S>&& r) {
    S::set(m_s, S::get(r.m_s));
    S::set(r.m_s, 0);
    return *this;
  }

  /*
   * Observers.
   */
  T* get() const { return reinterpret_cast<T*>(S::get(m_s)); }
  T& operator*()               const { return *get(); }
  T* operator->()              const { return get(); }
  /* implicit */ operator T*() const { return get(); }
  explicit operator bool()     const { return get(); }

  /*
   * Modifiers.
   */
  void reset() { operator=(nullptr); }

  template <typename Q = T>
  typename std::enable_if<
    std::is_move_assignable<typename Q::storage_type>::value,
    void
  >::type swap(LowPtrImpl& r) {
    std::swap(m_s, r.m_s);
  }

  template <typename Q = T>
  typename std::enable_if<
    !std::is_move_assignable<typename Q::storage_type>::value,
    void
  >::type swap(LowPtrImpl& r) {
    auto const tmp = S::get(m_s);
    S::set(m_s, S::get(r.m_s));
    S::set(r.m_s, tmp);
  }

private:
  typename S::storage_type m_s{0};
};

///////////////////////////////////////////////////////////////////////////////

template <class S, std::memory_order read_order, std::memory_order write_order>
struct AtomicStorage {
  using storage_type = std::atomic<S>;

  static ALWAYS_INLINE low_storage_t get(const storage_type& s) {
    return s.load(read_order);
  }
  static ALWAYS_INLINE void set(storage_type& s, low_storage_t r) {
    s.store(r, write_order);
  }
};

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

template<class T>
struct LowPtr {
  using storage_type = detail::low_storage_t;
  enum class Unchecked {};

  /*
   * Constructors.
   */
  LowPtr() = default;

  /* implicit */ LowPtr(T* px) : m_s{detail::to_low(px)} {}
  /* implicit */ LowPtr(std::nullptr_t /*px*/) : m_s{0} {}

  LowPtr(const LowPtr<T>&) = default;
  LowPtr(LowPtr<T>&&) = default;

  /*
   * Assignments.
   */
  LowPtr& operator=(T* px) {
    m_s = detail::to_low(px);
    return *this;
  }
  LowPtr& operator=(const LowPtr<T>&) = default;
  LowPtr& operator=(std::nullptr_t /*px*/) {
    m_s = 0;
    return *this;
  }
  LowPtr& operator=(LowPtr<T>&&) = default;

  /*
   * Observers.
   */
  T* get()                     const { return reinterpret_cast<T*>(m_s); }
  T& operator*()               const { return *get(); }
  T* operator->()              const { return get(); }
  /* implicit */ operator T*() const { return get(); }
  explicit operator bool()     const { return get(); }

  /*
   * Modifiers.
   */
  void reset() { operator=(nullptr); }

private:
  storage_type m_s{0};
};

///////////////////////////////////////////////////////////////////////////////

/* TagType must be an enum with values 0-3 to fit in two bits */
template<class T, class TagType = uint8_t>
struct TaggedLowPtr {
  using storage_type = detail::low_storage_t;
  static constexpr storage_type kMaxEnumValue = 0b11;

  /*
   * Constructors.
   */
  TaggedLowPtr() = default;
  explicit TaggedLowPtr(LowPtr<T> px, TagType t) : m_s{pack(px, t)} {}
  TaggedLowPtr(const TaggedLowPtr<T, TagType>&) = default;
  TaggedLowPtr(TaggedLowPtr<T, TagType>&&) = default;

  /*
   * Assignments.
   */
  TaggedLowPtr& operator=(const TaggedLowPtr<T, TagType>&) = default;
  TaggedLowPtr& operator=(TaggedLowPtr<T, TagType>&&) = default;

  /*
   * Methods.
   */
  LowPtr<T> get() const {
    return reinterpret_cast<T*>(m_s & ~kMaxEnumValue);
  }
  TagType kind() const {
    return static_cast<TagType>(m_s & kMaxEnumValue);
  }
  void set(LowPtr<T> px, TagType t) {
    m_s = pack(px, t);
  }
  void reset() { m_s = 0; }

  /**
   * Conversions.
   */
  T& operator*()               const { return *get(); }
  T* operator->()              const { return get(); }
  /* implicit */ operator T*() const { return get(); }
  explicit operator bool()     const { return get(); }

private:
  storage_type pack(LowPtr<T> px, TagType t) {
    auto const iptr = detail::to_low(static_cast<T*>(px));
    assertx((iptr & kMaxEnumValue) == 0); // LowPtr is 4-byte aligned

    auto const itag = static_cast<storage_type>(t);
    assertx(0 <= itag);
    assertx(itag <= kMaxEnumValue);

    return iptr | itag;
  }
  storage_type m_s{0};
};

template<class T,
         std::memory_order read_order = std::memory_order_relaxed,
         std::memory_order write_order = std::memory_order_relaxed>
using AtomicLowPtr =
  detail::LowPtrImpl<T, detail::AtomicStorage<detail::low_storage_t,
                                              read_order,
                                              write_order>>;

template<class T> struct lowptr_traits : std::false_type {};
template<class T> struct lowptr_traits<LowPtr<T>> : std::true_type {
  using element_type = T;
  using pointer = T*;
};
template<class T, std::memory_order R, std::memory_order W>
struct lowptr_traits<AtomicLowPtr<T, R, W>> : std::true_type {
  using element_type = T;
  using pointer = T*;
};
template<class T> constexpr bool is_lowptr_v = lowptr_traits<T>::value;

///////////////////////////////////////////////////////////////////////////////
}

namespace folly {
template<class T> class FormatValue<HPHP::LowPtr<T>> {
 public:
  explicit FormatValue(HPHP::LowPtr<T> v) : m_val(v) {}

  template<typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    FormatValue<T*>(m_val.get()).format(arg, cb);
  }

 private:
  const HPHP::LowPtr<T> m_val;
};
}

