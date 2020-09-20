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

namespace detail {

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
  /* implicit */ LowPtrImpl(Unchecked, T* px) : m_s{to_low_unchecked(px)} {}

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
  T* get() const {
    return reinterpret_cast<T*>(S::get(m_s));
  }

  T& operator*() const {
    return *get();
  }

  T* operator->() const {
    return get();
  }

  /* implicit */ operator T*() const {
    return get();
  }

  explicit operator bool() const {
    return get();
  }

  /*
   * Modifiers.
   */
  void reset() {
    operator=(nullptr);
  }

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
  /*
   * Lowness.
   */
  static bool is_low(T* px) {
    typename S::raw_type ones = ~0;
    auto ptr = reinterpret_cast<uintptr_t>(px);
    return (ptr & ones) == ptr;
  }

  static typename S::raw_type to_low(T* px) {
    always_assert(is_low(px));
    return (typename S::raw_type)(reinterpret_cast<uintptr_t>(px));
  }

  static typename S::raw_type to_low_unchecked(T* px) {
    assertx(is_low(px));
    return (typename S::raw_type)(reinterpret_cast<uintptr_t>(px));
  }

protected:
  typename S::storage_type m_s{0};
};

///////////////////////////////////////////////////////////////////////////////

template <class S>
struct RawStorage {
  using raw_type = S;
  using storage_type = S;

  static ALWAYS_INLINE raw_type get(const storage_type& s) {
    return s;
  }
  static ALWAYS_INLINE void set(storage_type& s, raw_type r) {
    s = r;
  }
};

template <class S, std::memory_order read_order, std::memory_order write_order>
struct AtomicStorage {
  using raw_type = S;
  using storage_type = std::atomic<S>;

  static ALWAYS_INLINE raw_type get(const storage_type& s) {
    return s.load(read_order);
  }
  static ALWAYS_INLINE void set(storage_type& s, raw_type r) {
    s.store(r, write_order);
  }
};

///////////////////////////////////////////////////////////////////////////////

}

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

template<class T>
using LowPtr =
  detail::LowPtrImpl<T, detail::RawStorage<detail::low_storage_t>>;

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

