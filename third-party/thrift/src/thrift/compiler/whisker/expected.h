/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <thrift/compiler/whisker/detail/overload.h>

#include <cassert>
#include <exception>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace whisker {

// This macro greatly simplifies the SFINAE tricks required by expected<T, E>.
#define WHISKER_EXPECTED_REQUIRES(...) std::enable_if_t<__VA_ARGS__>* = nullptr

/**
 * A type that mimics std::expected<T, E>:
 *   https://en.cppreference.com/w/cpp/utility/expected
 *
 * This class implements a subset of the std::expected API. In other words,
 * std::expected is a drop-in replacement for whisker::expected, but not the
 * other way around. This is a trade-off between code complexity and usability,
 * with the expectation that this can be replaced with std::expected with C++23.
 *
 * The following features stand out as missing from whisker::expected<T, E>:
 *   - whisker::expected is not constexpr-compatible.
 *   - whisker::expected does not support void value types (use std::monostate).
 *   - whisker::expected does not have the triviality guarantees of
 *     std::expected. whisker::expected<T, E> is never trivially constructible.
 *   - whisker::expected does not have the same noexcept guarantees as
 *     std::expected.
 *   - whisker::expected does not support value_or / error_or.
 *   - whisker::expected does not support monadic operations:
 *     - and_then
 *     - transform
 *     - or_else
 *     - transform_error
 */
template <typename T, typename E>
class expected;

/**
 * A type that mimics std::unexpected<E>:
 *   https://en.cppreference.com/w/cpp/utility/expected/unexpected
 */
template <typename E>
class unexpected;

/**
 * A type that mimics std::unexpect_t:
 *   https://en.cppreference.com/w/cpp/utility/expected/unexpect_t
 */
struct unexpect_t {
  explicit unexpect_t() = default;
};
constexpr inline unexpect_t unexpect{};

/**
 * A type that mimics std::bad_expected_access:
 *   https://en.cppreference.com/w/cpp/utility/expected/bad_expected_access
 */
template <typename E>
class bad_expected_access;

template <>
class bad_expected_access<void> : public std::exception {
 public:
  const char* what() const noexcept override { return "bad expected access"; }
};

template <typename E>
class bad_expected_access : public bad_expected_access<void> {
 public:
  explicit bad_expected_access(E error) : error_(std::move(error)) {}

  const E& error() const& noexcept { return error_; }
  E& error() & noexcept { return error_; }
  E&& error() && noexcept { return std::move(error_); }
  const E&& error() const&& noexcept { return static_cast<const E&&>(error_); }

 private:
  E error_;
};

namespace detail {

// std::remove_cvref_t was added in C++20
template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T, template <typename...> typename Template>
constexpr inline bool is_specialization = false;
template <template <typename...> typename Template, typename... Types>
constexpr inline bool is_specialization<Template<Types...>, Template> = true;

// https://en.cppreference.com/w/cpp/utility/expected/unexpected#Template_parameters
template <typename E>
constexpr bool check_valid_error_type() {
  static_assert(std::is_object_v<E>);
  static_assert(!std::is_array_v<E>);
  static_assert(!std::is_const_v<E>);
  static_assert(!std::is_volatile_v<E>);
  static_assert(!is_specialization<E, unexpected>);
  return true;
}

// These are types whose main purpose is to select a specific constructor for
// expected<T, E>
template <typename T>
constexpr inline bool is_disambiguator_type =
    std::is_same_v<std::remove_cv_t<T>, std::in_place_t> ||
    std::is_same_v<std::remove_cv_t<T>, unexpect_t> ||
    is_specialization<std::remove_cv_t<T>, unexpected>;

// https://en.cppreference.com/w/cpp/utility/expected#Template_parameters
template <typename T>
constexpr bool check_valid_value_type() {
  // Destructible
  static_assert(!std::is_reference_v<T>);
  static_assert(!std::is_function_v<T>);
  static_assert(!std::is_array_v<T>);

  static_assert(!is_disambiguator_type<T>);
  return true;
}

} // namespace detail

template <typename E>
class unexpected {
  static_assert(detail::check_valid_error_type<E>());

 public:
  unexpected(const unexpected&) = default;
  unexpected(unexpected&&) = default;

  // https://en.cppreference.com/w/cpp/utility/expected/unexpected#ctor
  template <
      typename Err = E,
      WHISKER_EXPECTED_REQUIRES(
          !std::is_same_v<detail::remove_cvref_t<Err>, unexpected>),
      WHISKER_EXPECTED_REQUIRES(
          !std::is_same_v<detail::remove_cvref_t<Err>, std::in_place_t>),
      WHISKER_EXPECTED_REQUIRES(std::is_constructible_v<E, Err>)>
  explicit unexpected(Err&& e) noexcept(std::is_nothrow_constructible_v<E, Err>)
      : error_(std::forward<Err>(e)) {}

  template <
      typename... Args,
      WHISKER_EXPECTED_REQUIRES(std::is_constructible_v<E, Args...>)>
  explicit unexpected(std::in_place_t, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<E, Args...>)
      : error_(std::forward<Args>(args)...) {}

  template <
      typename U,
      typename... Args,
      WHISKER_EXPECTED_REQUIRES(
          std::is_constructible_v<E, std::initializer_list<U>&, Args...>)>
  explicit unexpected(
      std::in_place_t,
      std::initializer_list<U> ilist,
      Args&&... args) noexcept(std::
                                   is_nothrow_constructible_v<
                                       E,
                                       std::initializer_list<U>&,
                                       Args...>)
      : error_(ilist, std::forward<Args>(args)...) {}

  unexpected& operator=(const unexpected&) = default;
  unexpected& operator=(unexpected&&) = default;

  const E& error() const& noexcept { return error_; }
  E& error() & noexcept { return error_; }
  E&& error() && noexcept { return std::move(error_); }
  const E&& error() const&& noexcept { return static_cast<const E&&>(error_); }

  void swap(unexpected& other) noexcept(std::is_nothrow_swappable_v<E>) {
    using std::swap;
    swap(error_, other.error_);
  }

  friend void swap(unexpected& lhs, unexpected& rhs) noexcept(
      noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
  }

  template <typename G>
  friend bool operator==(const unexpected& lhs, const unexpected<G>& rhs) {
    return lhs.error_ == rhs.error_;
  }

 private:
  E error_;
};

template <typename E>
unexpected(E) -> unexpected<E>;

template <typename T, typename E>
class expected {
  static_assert(detail::check_valid_value_type<T>());
  static_assert(!std::is_void_v<T>, "Use std::monostate instead.");
  static_assert(std::is_nothrow_move_constructible_v<T>);
  static_assert(std::is_nothrow_move_assignable_v<T>);

  static_assert(detail::check_valid_error_type<E>());
  static_assert(std::is_nothrow_move_constructible_v<E>);
  static_assert(std::is_nothrow_move_assignable_v<E>);

  // Restrictions on constructor (6):
  //   https://en.cppreference.com/w/cpp/utility/expected/expected#Version_6
  template <typename U>
  static constexpr inline bool is_forward_constructible_from =
      std::is_constructible_v<T, U> && !detail::is_disambiguator_type<T> &&
      // copy constructor has its own overload
      !std::is_same_v<detail::remove_cvref_t<U>, expected> &&
      // LWG-3836
      (!std::is_same_v<detail::remove_cvref_t<T>, bool> ||
       !detail::is_specialization<detail::remove_cvref_t<U>, expected>);

  // Restrictions on constructors (4) and (5):
  template <
      typename U,
      typename G,
      typename UWithQualifiers, // const U& or U&&
      typename GWithQualifiers> // const G& or G&&
  static constexpr inline bool is_constructible_from_other =
      std::is_constructible_v<T, UWithQualifiers> &&
      std::is_constructible_v<E, GWithQualifiers> &&
      // LWG-3836
      (std::is_same_v<std::remove_cv_t<T>, bool> ||
       !(std::is_constructible_v<T, expected<U, G>&> ||
         std::is_constructible_v<T, expected<U, G>> ||
         std::is_constructible_v<T, const expected<U, G>&> ||
         std::is_constructible_v<T, const expected<U, G>> ||
         std::is_convertible_v<expected<U, G>&, T> ||
         std::is_convertible_v<expected<U, G>&&, T> ||
         std::is_convertible_v<const expected<U, G>&, T> ||
         std::is_convertible_v<const expected<U, G>&&, T>)) &&
      !(std::is_constructible_v<unexpected<E>, expected<U, G>&> ||
        std::is_constructible_v<unexpected<E>, expected<U, G>> ||
        std::is_constructible_v<unexpected<E>, const expected<U, G>&> ||
        std::is_constructible_v<unexpected<E>, const expected<U, G>>);

  template <typename U>
  static constexpr inline bool is_forward_assignable_from =
      std::is_constructible_v<T, U> && std::is_assignable_v<T&, U> &&
      // operator=(const expected&) has own overload
      !std::is_same_v<detail::remove_cvref_t<U>, expected> &&
      // operator=(const unexpected_type&) has own overload
      !std::is_same_v<detail::remove_cvref_t<U>, unexpected<E>>;

 public:
  using value_type = T;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_1
  template <
      typename U = T,
      WHISKER_EXPECTED_REQUIRES(std::is_default_constructible_v<U>)>
  expected() noexcept(std::is_nothrow_default_constructible_v<T>)
      : storage_() {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_2
  expected(const expected& other) noexcept(
      std::is_nothrow_copy_constructible_v<T> &&
      std::is_nothrow_copy_constructible_v<E>) = default;
  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_3
  expected(expected&& other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_constructible_v<E>) = default;

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_4
  // (implicit)
  template <
      class U,
      class G,
      WHISKER_EXPECTED_REQUIRES(
          (std::is_convertible_v<const U&, T> &&
           std::is_convertible_v<const G&, E>)),
      WHISKER_EXPECTED_REQUIRES(
          is_constructible_from_other<U, G, const U&, const G&>)>
  /* implicit */ expected(const expected<U, G>& other) noexcept(
      std::is_nothrow_constructible_v<T, const U&> &&
      std::is_nothrow_constructible_v<E, const G&>)
      : storage_(from_other(other.storage_)) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_4
  // (explicit)
  template <
      class U,
      class G,
      WHISKER_EXPECTED_REQUIRES(
          !(std::is_convertible_v<const U&, T> &&
            std::is_convertible_v<const G&, E>)),
      WHISKER_EXPECTED_REQUIRES(
          is_constructible_from_other<U, G, const U&, const G&>)>
  explicit expected(const expected<U, G>& other) noexcept(
      std::is_nothrow_constructible_v<T, const U&> &&
      std::is_nothrow_constructible_v<E, const G&>)
      : storage_(from_other(other.storage_)) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_5
  // (implicit)
  template <
      class U,
      class G,
      WHISKER_EXPECTED_REQUIRES(
          (std::is_convertible_v<U, T> && std::is_convertible_v<G, E>)),
      WHISKER_EXPECTED_REQUIRES(is_constructible_from_other<U, G, U, G>)>
  /* implicit */ expected(expected<U, G>&& other) noexcept(
      std::is_nothrow_constructible_v<T, U> &&
      std::is_nothrow_constructible_v<E, G>)
      : storage_(from_other(std::move(other.storage_))) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_5
  // (explicit)
  template <
      class U,
      class G,
      WHISKER_EXPECTED_REQUIRES(
          !(std::is_convertible_v<U, T> && std::is_convertible_v<G, E>)),
      WHISKER_EXPECTED_REQUIRES(is_constructible_from_other<U, G, U, G>)>
  explicit expected(expected<U, G>&& other) noexcept(
      std::is_nothrow_constructible_v<T, U> &&
      std::is_nothrow_constructible_v<E, G>)
      : storage_(from_other(std::move(other.storage_))) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_6
  // (implicit)
  template <
      typename U,
      WHISKER_EXPECTED_REQUIRES(std::is_convertible_v<U, T>),
      WHISKER_EXPECTED_REQUIRES(is_forward_constructible_from<U>)>
  /* implicit */ expected(U&& value) noexcept(
      std::is_nothrow_constructible_v<T, U>)
      : expected(std::in_place, std::forward<U>(value)) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_6
  // (explicit)
  template <
      typename U = T,
      WHISKER_EXPECTED_REQUIRES(!std::is_convertible_v<U, T>),
      WHISKER_EXPECTED_REQUIRES(is_forward_constructible_from<U>)>
  explicit expected(U&& value) noexcept(std::is_nothrow_constructible_v<T, U>)
      : expected(std::in_place, std::forward<U>(value)) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_7
  // (implicit)
  template <
      typename G,
      WHISKER_EXPECTED_REQUIRES(std::is_convertible_v<const G&, E>),
      WHISKER_EXPECTED_REQUIRES(std::is_constructible_v<E, const G&>)>
  /* implicit */ expected(const unexpected<G>& error) noexcept(
      std::is_nothrow_constructible_v<E, const G&>)
      : storage_(std::in_place_type<unexpected<E>>, error.error()) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_7
  // (explicit)
  template <
      typename G,
      WHISKER_EXPECTED_REQUIRES(!std::is_convertible_v<const G&, E>),
      WHISKER_EXPECTED_REQUIRES(std::is_constructible_v<E, const G&>)>
  explicit expected(const unexpected<G>& error) noexcept(
      std::is_nothrow_constructible_v<E, const G&>)
      : storage_(std::in_place_type<unexpected<E>>, error.error()) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_8
  // (implicit)
  template <
      typename G,
      WHISKER_EXPECTED_REQUIRES(std::is_convertible_v<G, E>),
      WHISKER_EXPECTED_REQUIRES(std::is_constructible_v<E, G>)>
  /* implicit */ expected(unexpected<G>&& error) noexcept(
      std::is_nothrow_constructible_v<E, G>)
      : storage_(std::in_place_type<unexpected<E>>, std::move(error).error()) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_8
  // (explicit)
  template <
      typename G,
      WHISKER_EXPECTED_REQUIRES(!std::is_convertible_v<G, E>),
      WHISKER_EXPECTED_REQUIRES(std::is_constructible_v<E, G>)>
  explicit expected(unexpected<G>&& error) noexcept(
      std::is_nothrow_constructible_v<E, G>)
      : storage_(std::in_place_type<unexpected<E>>, std::move(error).error()) {}

  /* implicit */ expected(const unexpected<E>& error)
      : storage_(std::in_place_type<unexpected<E>>, error) {}

  /* implicit */ expected(unexpected<E>&& error)
      : storage_(std::in_place_type<unexpected<E>>, std::move(error)) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_9
  template <
      typename... Args,
      WHISKER_EXPECTED_REQUIRES(std::is_constructible_v<T, Args...>)>
  explicit expected(std::in_place_t, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<T, Args...>)
      : storage_(std::in_place_type<T>, std::forward<Args>(args)...) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_10
  template <
      typename U,
      typename... Args,
      WHISKER_EXPECTED_REQUIRES(
          std::is_constructible_v<T, std::initializer_list<U>&, Args...>)>
  explicit expected(
      std::in_place_t,
      std::initializer_list<U> ilist,
      Args&&... args) noexcept(std::
                                   is_nothrow_constructible_v<
                                       T,
                                       std::initializer_list<U>&,
                                       Args...>)
      : storage_(std::in_place_type<T>, ilist, std::forward<Args>(args)...) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_11
  template <
      typename... Args,
      WHISKER_EXPECTED_REQUIRES(std::is_constructible_v<E, Args...>)>
  explicit expected(unexpect_t, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<E, Args...>)
      : storage_(
            std::in_place_type<unexpected<E>>,
            std::in_place,
            std::forward<Args>(args)...) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_12
  template <
      typename U,
      typename... Args,
      WHISKER_EXPECTED_REQUIRES(
          std::is_constructible_v<E, std::initializer_list<U>&, Args...>)>
  explicit expected(
      unexpect_t,
      std::initializer_list<U> ilist,
      Args&&... args) noexcept(std::
                                   is_nothrow_constructible_v<
                                       E,
                                       std::initializer_list<U>&,
                                       Args...>)
      : storage_(
            std::in_place_type<unexpected<E>>,
            std::in_place,
            ilist,
            std::forward<Args>(args)...) {}

  expected& operator=(const expected& other) = default;
  expected& operator=(expected&& other) noexcept = default;

  template <
      typename U = T,
      WHISKER_EXPECTED_REQUIRES(is_forward_assignable_from<U>)>
  expected& operator=(U&& value) {
    storage_.template emplace<T>(std::forward<U>(value));
    return *this;
  }

  expected& operator=(const unexpected<E>& error) {
    storage_.template emplace<unexpected<E>>(error);
    return *this;
  }

  expected& operator=(unexpected<E>&& error) {
    storage_.template emplace<unexpected<E>>(std::move(error));
    return *this;
  }

  bool has_value() const noexcept {
    return std::holds_alternative<T>(storage_);
  }
  explicit operator bool() const noexcept { return has_value(); }

  const T* operator->() const noexcept {
    assert(has_value());
    return std::get_if<T>(&storage_);
  }
  T* operator->() noexcept {
    assert(has_value());
    return std::get_if<T>(&storage_);
  }
  const T& operator*() const& noexcept {
    assert(has_value());
    return *std::get_if<T>(&storage_);
  }
  T& operator*() & noexcept {
    assert(has_value());
    return *std::get_if<T>(&storage_);
  }
  T&& operator*() && noexcept {
    assert(has_value());
    return std::move(*std::get_if<T>(&storage_));
  }

  const T& value() const& {
    static_assert(std::is_copy_constructible_v<E>);
    if (!has_value()) {
      throw bad_expected_access(std::as_const(error()));
    }
    return **this;
  }
  T& value() & {
    static_assert(std::is_copy_constructible_v<E>);
    if (!has_value()) {
      throw bad_expected_access(std::as_const(error()));
    }
    return **this;
  }
  T&& value() && {
    static_assert(std::is_move_constructible_v<E>);
    if (!has_value()) {
      throw bad_expected_access(std::move(*this).error());
    }
    return *(std::move(*this));
  }

  const E& error() const& noexcept {
    assert(!has_value());
    return std::get_if<unexpected<E>>(&storage_)->error();
  }
  E& error() & noexcept {
    assert(!has_value());
    return std::get_if<unexpected<E>>(&storage_)->error();
  }
  E&& error() && noexcept {
    assert(!has_value());
    return std::move(*std::get_if<unexpected<E>>(&storage_)).error();
  }

  template <
      typename... Args,
      WHISKER_EXPECTED_REQUIRES(std::is_nothrow_constructible_v<T, Args...>)>
  T& emplace(Args&&... args) noexcept {
    return storage_.template emplace<T>(std::forward<Args>(args)...);
  }

  void swap(expected& other) {
    using std::swap;
    swap(storage_, other.storage_);
  }

  friend void swap(expected& lhs, expected& rhs) { lhs.swap(rhs); }

  template <
      typename U,
      typename G,
      WHISKER_EXPECTED_REQUIRES(!std::is_void_v<U>)>
  friend bool operator==(const expected<T, E>& lhs, const expected<U, G>& rhs) {
    return lhs.storage_ == rhs.storage_;
  }
  template <typename U>
  friend bool operator==(const expected<T, E>& lhs, U&& rhs) {
    return lhs.has_value() && *lhs == std::forward<U>(rhs);
  }
  friend bool operator==(const expected<T, E>& lhs, const unexpected<E>& rhs) {
    return !lhs.has_value() && lhs.error() == rhs.error();
  }

  // Before C++20, operator!= is not synthesized from operator==.
  template <
      typename U,
      typename G,
      WHISKER_EXPECTED_REQUIRES(!std::is_void_v<U>)>
  friend bool operator!=(const expected<T, E>& lhs, const expected<U, G>& rhs) {
    return !(lhs == rhs);
  }
  template <typename U>
  friend bool operator!=(const expected<T, E>& lhs, U&& rhs) {
    return !(lhs == std::forward<U>(rhs));
  }
  friend bool operator!=(const expected<T, E>& lhs, const unexpected<E>& rhs) {
    return !(lhs == rhs);
  }

 private:
  template <typename U, typename G>
  friend class expected;

  template <typename U, typename G>
  static std::variant<T, unexpected<E>> from_other(
      const std::variant<U, G>& other) {
    using result = std::variant<T, unexpected<E>>;
    return detail::variant_match(
        other,
        [](const U& u) -> result { return T(u); },
        [](const G& g) -> result { return unexpected<E>(g.error()); });
  }

  template <typename U, typename G>
  static std::variant<T, unexpected<E>> from_other(std::variant<U, G>&& other) {
    using result = std::variant<T, unexpected<E>>;
    return detail::variant_match(
        std::move(other),
        [](std::remove_reference_t<U>&& u) -> result {
          return T(std::move(u));
        },
        [](std::remove_reference_t<G>&& g) -> result {
          return unexpected<E>(std::move(g).error());
        });
  }

  std::variant<T, unexpected<E>> storage_;
};

#undef WHISKER_EXPECTED_REQUIRES

} // namespace whisker
