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

#include <initializer_list>
#include <optional>
#include <type_traits>

#include "hphp/util/assertions.h"

/*
 * Drop in replacement for std::optional
 *
 * This is meant to be used instead of std::optional or
 * folly::Optional. It replicates the interface of std::optional.
 *
 * Both std::optional and folly::Optional have undesirable behavior
 * for accessing the value when the option is disengaged.
 * folly::Optional throws an exception, while it is merely undefined
 * for std::optional. Throwing an exception is particularly
 * problematic, as it makes it hard to debug (by the time you catch
 * it, the stack is unwind), and causes issues with noexcept
 * functions.
 *
 * Instead we choose to always_assert if you attempt to access the
 * value when disengaged. This is no more expensive than
 * folly::Optional, but makes it much easier to debug.
 *
 * HPHP::Optional just wraps a std::optional, but adds the proper
 * asserts around the right functions.
 */

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Optional {
  // These all intentionally match the interface for std::optional:

  constexpr const T& operator*() const& {
    always_assert(m_opt.has_value());
    return *m_opt;
  }
  constexpr T& operator*() & {
    always_assert(m_opt.has_value());
    return *m_opt;
  }
  constexpr const T&& operator*() const&& {
    always_assert(m_opt.has_value());
    return *std::move(m_opt);
  }
  constexpr T&& operator*() && {
    always_assert(m_opt.has_value());
    return *std::move(m_opt);
  }

  constexpr const T* operator->() const {
    always_assert(m_opt.has_value());
    return m_opt.operator->();
  }
  constexpr T* operator->() {
    always_assert(m_opt.has_value());
    return m_opt.operator->();
  }

  constexpr explicit operator bool() const noexcept { return m_opt.has_value(); }
  constexpr bool has_value() const noexcept { return m_opt.has_value(); }

  // value() throws an exception even in std::optional. We don't want
  // exceptions, so treat it as a synonym for operator*.
  constexpr T& value() & {
    always_assert(m_opt.has_value());
    return *m_opt;
  }
  constexpr const T& value() const & {
    always_assert(m_opt.has_value());
    return *m_opt;
  }
  constexpr T&& value() && {
    always_assert(m_opt.has_value());
    return *std::move(m_opt);
  }
  constexpr const T&& value() const && {
    always_assert(m_opt.has_value());
    return *std::move(m_opt);
  }

  template <typename U>
  constexpr T value_or(U&& default_value) const& {
    return m_opt.value_or(std::move(default_value));
  }
  template <typename U>
  constexpr T value_or(U&& default_value) && {
    return std::move(m_opt).value_or(std::move(default_value));
  }

  // std::optional doesn't have this, but folly::Optional
  // does. Support it for compatibility.
  T* get_pointer() {
    return m_opt.has_value() ? &*m_opt : nullptr;
  }
  const T* get_pointer() const {
    return m_opt.has_value() ? &*m_opt : nullptr;
  }

  void swap(Optional& o)
    noexcept(noexcept(std::declval<decltype(o.m_opt)>().swap(o.m_opt))) {
    m_opt.swap(o.m_opt);
  }

  void reset() noexcept { m_opt.reset(); }

  template<typename... Args> T& emplace(Args&&... args) {
    return m_opt.emplace(std::forward<Args>(args)...);
  }
  template<typename U, typename... Args>
  T& emplace(std::initializer_list<U> ilist, Args&&... args) {
    return m_opt.emplace(ilist, std::forward<Args>(args)...);
  }

private:
  // SFINAE stuff to support the required ctor and assignment
  // overloads.
  template<typename... Cond>
  using requires = std::enable_if_t<std::conjunction_v<Cond...>, bool>;

  template<typename U>
  using remove_cvref_t =
    typename std::remove_cv<typename std::remove_reference<U>::type>::type;

  template<typename U>
  using not_self = std::negation<std::is_same<Optional, remove_cvref_t<U>>>;
  template<typename U>
  using not_tag =
    std::negation<std::is_same<std::in_place_t, remove_cvref_t<U>>>;

  template<typename U, typename V>
  using converts_from_optional =
    std::disjunction<std::is_constructible<U, const Optional<V>&>,
                     std::is_constructible<U, Optional<V>&>,
                     std::is_constructible<U, const Optional<V>&&>,
                     std::is_constructible<U, Optional<V>&&>,
                     std::is_convertible<const Optional<V>&, U>,
                     std::is_convertible<Optional<V>&, U>,
                     std::is_convertible<const Optional<V>&&, U>,
                     std::is_convertible<Optional<V>&&, U>>;

  template<typename U, typename V>
  using assigns_from_optional =
    std::disjunction<std::is_assignable<U&, const Optional<V>&>,
                     std::is_assignable<U&, Optional<V>&>,
                     std::is_assignable<U&, const Optional<V>&&>,
                     std::is_assignable<U&, Optional<V>&&>>;

public:

  // These ctor and assignment operator overloads are a
  // mess. Unfortunately we cannot just forward arguments to
  // std::optional and have to replicate all the overloads with the
  // same SFINAE logic.

  // NB: folly::Optional resets when moved-from. std::optional remains
  // engaged with the contained value in a moved-from state. We rely
  // on folly::Optional's behavior, so we emulate it here.

  constexpr Optional() noexcept = default;
  constexpr Optional(std::nullopt_t) noexcept
    : m_opt(std::nullopt) {}

  constexpr Optional(const Optional& o)
    : m_opt(o.m_opt) {}

  constexpr Optional(Optional&& o)
  noexcept(noexcept(decltype(o.m_opt)(std::move(o.m_opt))))
    : m_opt(std::move(o.m_opt)) { o.m_opt.reset(); }

  template <typename U = T,
            requires<not_self<U>,
                     not_tag<U>,
                     std::is_constructible<T, U>,
                     std::is_convertible<U&&, T>> = true>
  constexpr Optional(U&& v)
    : m_opt(std::in_place, std::forward<U>(v)) {}

  template <typename U = T,
            requires<not_self<U>,
                     not_tag<U>,
                     std::is_constructible<T, U>,
                     std::negation<std::is_convertible<U&&, T>>> = true>
  explicit constexpr Optional(U&& v)
    : m_opt(std::in_place, std::forward<U>(v)) {}

  template <typename U,
            requires<std::negation<std::is_same<T, U>>,
                     std::is_constructible<T, const U&>,
                     std::is_convertible<const U&, T>,
                     std::negation<converts_from_optional<T, U>>> = true>
  constexpr Optional(const Optional<U>& o)
    : m_opt(o.m_opt) {}

  template <typename U,
             requires<std::negation<std::is_same<T, U>>,
                      std::is_constructible<T, const U&>,
                      std::negation<std::is_convertible<const U&, T>>,
                      std::negation<converts_from_optional<T, U>>> = true>
  explicit constexpr Optional(const Optional<U>& o)
    : m_opt(o.m_opt) {}

  template <typename U,
            requires<std::negation<std::is_same<T, U>>,
                     std::is_constructible<T, U&&>,
                     std::is_convertible<U&&, T>,
                     std::negation<converts_from_optional<T, U>>> = true>
  constexpr Optional(Optional<U>&& o)
    : m_opt(std::move(o.m_opt)) { o.m_opt.reset(); }

  template <typename U,
            requires<std::negation<std::is_same<T, U>>,
                     std::is_constructible<T, U&&>,
                     std::negation<std::is_convertible<U&&, T>>,
                     std::negation<converts_from_optional<T, U>>> = true>
  explicit constexpr Optional(Optional<U>&& o)
    : m_opt(std::move(o.m_opt)) { o.m_opt.reset(); }

  template<typename... Args,
           requires<std::is_constructible<T, Args&&...>> = true>
  explicit constexpr Optional(std::in_place_t, Args&&... args)
    : m_opt(std::in_place, std::forward<Args>(args)...) {}

  template<typename U, typename... Args,
           requires<std::is_constructible<T,
                                          std::initializer_list<U>&,
                                          Args&&...>> = true>
  explicit constexpr Optional(std::in_place_t,
                              std::initializer_list<U> il,
                              Args&&... args)
    : m_opt(std::in_place, il, std::forward<Args>(args)...) {}

  Optional& operator=(const Optional& o) {
    m_opt = o.m_opt;
    return *this;
  }

  Optional& operator=(Optional&& o)
    noexcept(
      noexcept(std::declval<decltype(o.m_opt)>().operator=(std::move(o.m_opt)))
    ) {
    m_opt = std::move(o.m_opt);
    o.m_opt.reset();
    return *this;
  }

  Optional& operator=(std::nullopt_t) noexcept {
    m_opt = std::nullopt;
    return *this;
  }

  template<typename U = T>
  std::enable_if_t<
    std::conjunction_v<
      not_self<U>,
      std::negation<
        std::conjunction<
          std::is_scalar<T>,
          std::is_same<T, std::decay_t<U>>
        >
      >,
      std::is_constructible<T, U>,
      std::is_assignable<T&, U>
    >,
    Optional&
  >
  operator=(U&& u) {
    m_opt = std::forward<U>(u);
    return *this;
  }

  template<typename U>
  std::enable_if_t<
    std::conjunction_v<
      std::negation<std::is_same<T, U>>,
      std::is_constructible<T, const U&>,
      std::is_assignable<T&, U>,
      std::negation<converts_from_optional<T, U>>,
      std::negation<assigns_from_optional<T, U>>
    >,
    Optional&
  >
  operator=(const Optional<U>& u) {
    m_opt = u.m_opt;
    return *this;
  }

  template<typename U>
  std::enable_if_t<
    std::conjunction_v<
      std::negation<std::is_same<T, U>>,
      std::is_constructible<T, U>,
      std::is_assignable<T&, U>,
      std::negation<converts_from_optional<T, U>>,
      std::negation<assigns_from_optional<T, U>>
    >,
    Optional&
  >
  operator=(Optional<U>&& u) {
    m_opt = std::move(u.m_opt);
    u.m_opt.reset();
    return *this;
  }

private:
  std::optional<T> m_opt;

  friend std::hash<Optional>;

#define OPERATORS                               \
  O(==)                                         \
  O(!=)                                         \
  O(<)                                          \
  O(<=)                                         \
  O(>)                                          \
  O(>=)

#define O(op)                                                                  \
   template <typename U, typename V>                                           \
   friend constexpr bool operator op(const Optional<U>&, const V&);            \
   template <typename U, typename V>                                           \
   friend constexpr bool operator op(const U&, const Optional<V>&);            \
   template <typename T1, typename T2>                                         \
   friend constexpr bool operator op(const Optional<T1>&, const Optional<T2>&);
OPERATORS
#undef O
};

#define O(op)                                                           \
  template <typename T, typename V>                                     \
  constexpr bool operator op(const Optional<T>& t, const V& v) {        \
    return t.m_opt op v;                                                \
  }                                                                     \
  template <typename T, typename U>                                     \
  constexpr bool operator op(const U& u, const Optional<T>& t) {        \
    return u op t.m_opt;                                                \
  }                                                                     \
  template <typename T1, typename T2>                                   \
  constexpr bool operator op(const Optional<T1>& t1,                    \
                             const Optional<T2>& t2) {                  \
    return t1.m_opt op t2.m_opt;                                        \
  }
OPERATORS
#undef O

#undef OPERATORS

template <typename T1, typename T2>
void swap(Optional<T1>& t1, Optional<T2>& t2) noexcept(noexcept(t1.swap(t2))) {
  return t1.swap(t2);
}

template<typename T>
constexpr Optional<std::decay_t<T>>
make_optional(T&& t) {
  return Optional<std::decay_t<T>>{ std::forward<T>(t) };
}
template<typename T, typename ...Args>
constexpr Optional<T>
make_optional(Args&&... args) {
  return Optional<T>{ std::in_place, std::forward<Args>(args)... };
}
template<typename T, typename U, typename ...Args>
constexpr Optional<T>
make_optional(std::initializer_list<U> il, Args&&... args) {
  return Optional<T>{ std::in_place, il, std::forward<Args>(args)... };
}

///////////////////////////////////////////////////////////////////////////////
}

namespace std {
///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct hash<HPHP::Optional<T>> {
  size_t operator()(const HPHP::Optional<T>& o) const {
    return m_hash(o.m_opt);
  }
private:
  hash<std::optional<T>> m_hash;
};

///////////////////////////////////////////////////////////////////////////////
}
