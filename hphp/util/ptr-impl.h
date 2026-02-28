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

#include "hphp/util/assertions.h"
#include "hphp/util/compilation-flags.h"

#include <cstdint>
#include <folly/Format.h>

namespace HPHP::ptrimpl {

///////////////////////////////////////////////////////////////////////////////

/*
 * This file provides a set of pointer types but you most likely want to use ptr.h
 * instead.
 *
 * The 2 storage types are normal and atomic.
 * And they can address 64 bits, 35 bits or 32 bits.
 * The 35 bit addressing is done by packing the pointer into 32 bits and it can
 * be done because the address has to be 8 byte aligned which means the lowest
 * 3 bits are always 0. The class does this transparently.
 */

///////////////////////////////////////////////////////////////////////////////

struct Normal {
  template <typename P>
  using storage_type = P::storage_type;

  template <typename P>
  static ALWAYS_INLINE P::storage_type get(const storage_type<P>& s) {
    return s;
  }

  template <typename P>
  static ALWAYS_INLINE void set(storage_type<P>& s, P::storage_type r) {
    s = r;
  }
};

struct Atomic {
  template <typename P>
  using storage_type = std::atomic<typename P::storage_type>;

  template <typename P>
  static ALWAYS_INLINE P::storage_type get(const storage_type<P>& s) {
    return s.load(std::memory_order_acquire);
  }

  template <typename P>
  static ALWAYS_INLINE void set(storage_type<P>& s, P::storage_type r) {
    s.store(r, std::memory_order_release);
  }
};

struct UInt32Packed {
  using storage_type = uint32_t;
  constexpr static uint32_t bits = 35;
  constexpr static uint64_t max = (1ull << bits) - 1;

  template <typename T>
  static void validatePtr(const T* px) {
    DEBUG_ONLY auto ptr = reinterpret_cast<uintptr_t>(px);
    assert_flog(ptr < (1ull << bits), "ptr {} is too large", ptr);
    assert_flog((ptr & ((1ull << 3) - 1)) == 0, "ptr {} is not 8 byte aligned", ptr);
  }

  template <typename T>
  static ALWAYS_INLINE storage_type toStorage(T* px) {
    if constexpr (debug) validatePtr(px);
    return (storage_type)(reinterpret_cast<uintptr_t>(px) >> 3);
  }

  template <typename T>
  static ALWAYS_INLINE T* fromStorage(storage_type mem) {
    uintptr_t ptr = (uintptr_t)mem << 3;
    T* px = reinterpret_cast<T*>(ptr);
    if constexpr (debug) validatePtr(px);
    return px;
  }
};

struct UInt32 {
  using storage_type = uint32_t;
  constexpr static uint32_t bits = 32;
  constexpr static uint64_t max = (1ull << bits) - 1;

  template <typename T>
  static void validatePtr(T* px) {
    DEBUG_ONLY auto ptr = reinterpret_cast<uintptr_t>(px);
    assert_flog(ptr < (1ull << bits), "ptr {} is too large", ptr);
  }

  template <typename T>
  static ALWAYS_INLINE storage_type toStorage(T* px) {
    if constexpr (debug) validatePtr(px);
    return (storage_type)reinterpret_cast<uintptr_t>(px);
  }

  template <typename T>
  static ALWAYS_INLINE T* fromStorage(storage_type mem) {
    uintptr_t ptr = (uintptr_t)mem;
    T* px = reinterpret_cast<T*>(ptr);
    if constexpr (debug) validatePtr(px);
    return px;
  }
};

struct UInt64 {
  using storage_type = uintptr_t;
  constexpr static uint32_t bits = 64;
  constexpr static uint64_t max = UINT64_MAX;

  template <typename T>
  static ALWAYS_INLINE storage_type toStorage(T* px) {
    return reinterpret_cast<uintptr_t>(px);
  }

  template <typename T>
  static ALWAYS_INLINE T* fromStorage(storage_type mem) {
    return reinterpret_cast<T*>(mem);
  }
};

template <typename S>
inline constexpr bool is_normal = std::is_same_v<S, Normal>;

template <typename S, typename P>
inline constexpr bool is_movable = std::is_move_assignable<typename S::template storage_type<P>>::value;

///////////////////////////////////////////////////////////////////////////////

template<class T, typename S, typename P, bool Init = true>
struct PtrImpl {
  constexpr static uint32_t bits = P::bits;
  constexpr static uint64_t max = P::max;
  using storage_type = typename S::template storage_type<P>;

  /*
   * Constructors.
   */
  PtrImpl() requires (Init) : PtrImpl(nullptr) {};

  PtrImpl() requires (!Init) = default;

  /* implicit */ PtrImpl(T* px) {
    S::template set<P>(m_s, P::toStorage(px));
  }
  /* implicit */ PtrImpl(std::nullptr_t /*px*/) {
    S::template set<P>(m_s, 0);
  }

  PtrImpl(const PtrImpl<T, S, P, Init>& other) requires (ptrimpl::is_normal<S>) = default;

  PtrImpl(const PtrImpl<T, S, P, Init>& other) requires (!ptrimpl::is_normal<S>) {
    S::template set<P>(m_s, S::template get<P>(other.m_s));
  }

  PtrImpl(PtrImpl<T, S, P, Init>&& other) requires ptrimpl::is_normal<S> = default;

  PtrImpl(PtrImpl<T, S, P, Init>&& r)
      noexcept requires (!ptrimpl::is_normal<S> && ptrimpl::is_movable<S, P>) {
    m_s = std::move(r.m_s);
  }

  PtrImpl(PtrImpl<T, S, P, Init>&& r)
      noexcept requires (!ptrimpl::is_normal<S> && !ptrimpl::is_movable<S, P>) {
    S::set(m_s, S::get(r.m_s));
    S::set(r.m_s, 0);
  }

  ~PtrImpl() = default;

  /*
   * Assignments.
   */
  PtrImpl& operator=(T* px) {
    S::template set<P>(m_s, P::toStorage(px));
    return *this;
  }
  PtrImpl& operator=(std::nullptr_t /*px*/) {
    S::template set<P>(m_s, 0);
    return *this;
  }

  PtrImpl& operator=(const PtrImpl<T, S, P, Init>& other)
      requires ptrimpl::is_normal<S> = default;

  PtrImpl& operator=(const PtrImpl<T, S, P, Init>& other)
      requires (!ptrimpl::is_normal<S>) {
    S::template set<P>(m_s, S::template get<P>(other.m_s));
    return *this;
  };

  PtrImpl& operator=(PtrImpl<T, S, P, Init>&& other)
      requires ptrimpl::is_normal<S> = default;

  PtrImpl& operator=(PtrImpl<T, S, P, Init>&& r)
      noexcept requires (!ptrimpl::is_normal<S> && ptrimpl::is_movable<S, P>) {
    m_s = std::move(r.m_s);
    return *this;
  }

  PtrImpl& operator=(PtrImpl<T, S, P, Init>&& r)
      noexcept requires (!ptrimpl::is_normal<S> && !ptrimpl::is_movable<S, P>) {
    S::set(m_s, S::get(r.m_s));
    S::set(r.m_s, 0);
    return *this;
  }

  /*
   * Observers.
   */
  T* get()                     const { return P::template fromStorage<T>(S::template get<P>(m_s)); }
  T& operator*()               const { return *get(); }
  T* operator->()              const { return get(); }
  /* implicit */ operator T*() const { return get(); }
  explicit operator bool()     const { return get(); }

  P::storage_type getRaw() const {
    return S::template get<P>(m_s);
  }

  static T* fromRaw(P::storage_type raw) {
    return P::template fromStorage<T>(raw);
  }

  static P::storage_type toRaw(const T* px) {
    return P::toStorage(px);
  }

  bool test_and_set(T* expected, T* desired) requires (!ptrimpl::is_normal<S>) {
    auto e = P::toStorage(expected);
    auto d = P::toStorage(desired);
    return m_s.compare_exchange_strong(e, d, std::memory_order_acq_rel);
  }

  /*
   * Modifiers.
   */
  void reset() { operator=(nullptr); }

private:
  storage_type m_s;
};

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

namespace folly {

template<class T, typename S, typename P> class FormatValue<HPHP::ptrimpl::PtrImpl<T, S, P>> {
 public:
  explicit FormatValue(HPHP::ptrimpl::PtrImpl<T, S, P> v) : m_val(v) {}

  template<typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    FormatValue<T*>(m_val.get()).format(arg, cb);
  }

 private:
  const HPHP::ptrimpl::PtrImpl<T, S, P> m_val;
};

}
