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

#ifndef incl_HPHP_LOW_PTR_H_
#define incl_HPHP_LOW_PTR_H_

#include "hphp/util/assertions.h"
#include "hphp/util/portability.h"

#include <folly/CPortability.h> // FOLLY_SANITIZE_ADDRESS

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
  /*
   * Constructors.
   */
  LowPtrImpl() {}

  /* implicit */ LowPtrImpl(T* px) : m_s{to_low(px)} {}

  /* implicit */ LowPtrImpl(std::nullptr_t px) : m_s{0} {}

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

  LowPtrImpl& operator=(std::nullptr_t px) {
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

protected:
  typename S::storage_type m_s;
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

template <class S>
struct AtomicStorage {
  using raw_type = S;
  using storage_type = std::atomic<S>;

  static ALWAYS_INLINE raw_type get(const storage_type& s) {
    return s.load(std::memory_order_relaxed);
  }
  static ALWAYS_INLINE void set(storage_type& s, raw_type r) {
    s.store(r, std::memory_order_relaxed);
  }
};

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

#ifdef FOLLY_SANITIZE_ADDRESS
#undef USE_LOWPTR
#endif

#ifdef USE_LOWPTR
constexpr bool use_lowptr = true;

template<class T>
using LowPtr       = detail::LowPtrImpl<T, detail::RawStorage<uint32_t>>;
template<class T>
using AtomicLowPtr = detail::LowPtrImpl<T, detail::AtomicStorage<uint32_t>>;

#else
constexpr bool use_lowptr = false;

template<class T>
using LowPtr       = detail::LowPtrImpl<T, detail::RawStorage<uintptr_t>>;
template<class T>
using AtomicLowPtr = detail::LowPtrImpl<T, detail::AtomicStorage<uintptr_t>>;

#endif

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_LOW_PTR_H_
