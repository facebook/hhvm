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
#include <thrift/compiler/whisker/detail/type_traits.h>

#include <cassert>
#include <exception>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <variant>

namespace whisker {

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
 *   - whisker::expected does not support value_or / error_or.
 *   - whisker::expected does not support monadic operations:
 *     - and_then
 *     - transform
 *     - or_else
 *     - transform_error
 */
template <typename T, typename E>
class [[nodiscard]] expected;

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

 private:
  E error_;
};

namespace detail {

// https://en.cppreference.com/w/cpp/utility/expected/unexpected#Template_parameters
template <typename E>
constexpr bool check_valid_error_type() {
  static_assert(std::is_object_v<E>);
  static_assert(!std::is_array_v<E>);
  static_assert(!std::is_const_v<E>);
  static_assert(!std::is_volatile_v<E>);
  static_assert(!is_specialization_v<E, unexpected>);
  return true;
}

// These are types whose main purpose is to select a specific constructor for
// expected<T, E>
template <typename T>
constexpr inline bool is_disambiguator_type =
    std::is_same_v<std::remove_cv_t<T>, std::in_place_t> ||
    std::is_same_v<std::remove_cv_t<T>, unexpect_t> ||
    is_specialization_v<std::remove_cv_t<T>, unexpected>;

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
      std::enable_if_t<
          !std::is_same_v<std::remove_cvref_t<Err>, unexpected> &&
              !std::is_same_v<std::remove_cvref_t<Err>, std::in_place_t> &&
              std::is_constructible_v<E, Err>,
          int> = 0>
  explicit unexpected(Err&& e) noexcept(std::is_nothrow_constructible_v<E, Err>)
      : error_(std::forward<Err>(e)) {}

  template <
      typename... Args,
      std::enable_if_t<std::is_constructible_v<E, Args...>, int> = 0>
  explicit unexpected(std::in_place_t, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<E, Args...>)
      : error_(std::forward<Args>(args)...) {}

  template <
      typename U,
      typename... Args,
      std::enable_if_t<
          std::is_constructible_v<E, std::initializer_list<U>&, Args...>,
          int> = 0>
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

  static_assert(detail::check_valid_error_type<E>());

  // Restrictions on constructor (6):
  //   https://en.cppreference.com/w/cpp/utility/expected/expected#Version_6
  template <typename U>
  static constexpr inline bool is_forward_constructible_from =
      std::is_constructible_v<T, U> && !detail::is_disambiguator_type<T> &&
      // copy constructor has its own overload
      !std::is_same_v<std::remove_cvref_t<U>, expected> &&
      // LWG-3836
      (!std::is_same_v<std::remove_cvref_t<T>, bool> ||
       !detail::is_specialization_v<std::remove_cvref_t<U>, expected>);

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

  // Restrictions on operator= (3)
  template <typename U>
  static constexpr inline bool is_forward_assignable_from =
      std::is_constructible_v<T, U> && std::is_assignable_v<T&, U> &&
      // operator=(const expected&) has own overload
      !std::is_same_v<std::remove_cvref_t<U>, expected> &&
      // operator=(const unexpected<G>&) has own overload
      !detail::is_specialization_v<std::remove_cvref_t<U>, unexpected> &&
      std::is_constructible_v<T, U> && std::is_assignable_v<T&, U> &&
      // re-init rollback possible in case of exception
      (std::is_nothrow_constructible_v<T, U> ||
       std::is_nothrow_move_constructible_v<T> ||
       std::is_nothrow_move_constructible_v<E>);

  // Restrictions on operator= (4) and (5)
  template <typename G>
  static constexpr inline bool is_forward_assignable_from_unexpected =
      std::is_constructible_v<E, G> && std::is_assignable_v<E&, G> &&
      // re-init rollback possible in case of exception
      (std::is_nothrow_constructible_v<E, G> ||
       std::is_nothrow_move_constructible_v<T> ||
       std::is_nothrow_move_constructible_v<E>);

 public:
  using value_type = T;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_1
  template <
      typename U = T,
      std::enable_if_t<std::is_default_constructible_v<U>, int> = 0>
  expected() noexcept(std::is_nothrow_default_constructible_v<T>)
      : storage_() {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_2
  expected(const expected& other) noexcept(
      std::is_nothrow_copy_constructible_v<T> &&
      std::is_nothrow_copy_constructible_v<E>)
      : storage_(from_other(other.storage_)) {
    // To properly SFINAE this, we need to move the copy ctor to a base class or
    // use C++20 concepts
    static_assert(
        std::is_copy_constructible_v<T> && std::is_copy_constructible_v<E>);
  }
  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_3
  expected(expected&& other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_constructible_v<E>)
      : storage_(from_other(std::move(other.storage_))) {
    // To properly SFINAE this, we need to move the move ctor to a base class or
    // use C++20 concepts
    static_assert(
        std::is_move_constructible_v<T> && std::is_move_constructible_v<E>);
  }

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_4
  // (implicit)
  template <
      class U,
      class G,
      std::enable_if_t<
          std::is_convertible_v<const U&, T> &&
              std::is_convertible_v<const G&, E> &&
              is_constructible_from_other<U, G, const U&, const G&>,
          int> = 0>
  /* implicit */ expected(const expected<U, G>& other) noexcept(
      std::is_nothrow_constructible_v<T, const U&> &&
      std::is_nothrow_constructible_v<E, const G&>)
      : storage_(from_other(other.storage_)) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_4
  // (explicit)
  template <
      class U,
      class G,
      std::enable_if_t<
          !(std::is_convertible_v<const U&, T> &&
            std::is_convertible_v<const G&, E>) &&
              is_constructible_from_other<U, G, const U&, const G&>,
          int> = 0>
  explicit expected(const expected<U, G>& other) noexcept(
      std::is_nothrow_constructible_v<T, const U&> &&
      std::is_nothrow_constructible_v<E, const G&>)
      : storage_(from_other(other.storage_)) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_5
  // (implicit)
  template <
      class U,
      class G,
      std::enable_if_t<
          std::is_convertible_v<U, T> && std::is_convertible_v<G, E> &&
              is_constructible_from_other<U, G, U, G>,
          int> = 0>
  /* implicit */ expected(expected<U, G>&& other) noexcept(
      std::is_nothrow_constructible_v<T, U> &&
      std::is_nothrow_constructible_v<E, G>)
      : storage_(from_other(std::move(other.storage_))) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_5
  // (explicit)
  template <
      class U,
      class G,
      std::enable_if_t<
          !(std::is_convertible_v<U, T> && std::is_convertible_v<G, E>) &&
              is_constructible_from_other<U, G, U, G>,
          int> = 0>
  explicit expected(expected<U, G>&& other) noexcept(
      std::is_nothrow_constructible_v<T, U> &&
      std::is_nothrow_constructible_v<E, G>)
      : storage_(from_other(std::move(other.storage_))) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_6
  // (implicit)
  template <
      typename U,
      std::enable_if_t<
          std::is_convertible_v<U, T> && is_forward_constructible_from<U>,
          int> = 0>
  /* implicit */ expected(U&& value) noexcept(
      std::is_nothrow_constructible_v<T, U>)
      : expected(std::in_place, std::forward<U>(value)) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_6
  // (explicit)
  template <
      typename U = T,
      std::enable_if_t<
          !std::is_convertible_v<U, T> && is_forward_constructible_from<U>,
          int> = 0>
  explicit expected(U&& value) noexcept(std::is_nothrow_constructible_v<T, U>)
      : expected(std::in_place, std::forward<U>(value)) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_7
  // (implicit)
  template <
      typename G,
      std::enable_if_t<
          std::is_convertible_v<const G&, E> &&
              std::is_constructible_v<E, const G&>,
          int> = 0>
  /* implicit */ expected(const unexpected<G>& error) noexcept(
      std::is_nothrow_constructible_v<E, const G&>)
      : storage_(std::in_place_type<unexpected<E>>, error.error()) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_7
  // (explicit)
  template <
      typename G,
      std::enable_if_t<
          !std::is_convertible_v<const G&, E> &&
              std::is_constructible_v<E, const G&>,
          int> = 0>
  explicit expected(const unexpected<G>& error) noexcept(
      std::is_nothrow_constructible_v<E, const G&>)
      : storage_(std::in_place_type<unexpected<E>>, error.error()) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_8
  // (implicit)
  template <
      typename G,
      std::enable_if_t<
          std::is_convertible_v<G, E> && std::is_constructible_v<E, G>,
          int> = 0>
  /* implicit */ expected(unexpected<G>&& error) noexcept(
      std::is_nothrow_constructible_v<E, G>)
      : storage_(std::in_place_type<unexpected<E>>, std::move(error).error()) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_8
  // (explicit)
  template <
      typename G,
      std::enable_if_t<
          !std::is_convertible_v<G, E> && std::is_constructible_v<E, G>,
          int> = 0>
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
      std::enable_if_t<std::is_constructible_v<T, Args...>, int> = 0>
  explicit expected(std::in_place_t, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<T, Args...>)
      : storage_(std::in_place_type<T>, std::forward<Args>(args)...) {}

  // https://en.cppreference.com/w/cpp/utility/expected/expected#Version_10
  template <
      typename U,
      typename... Args,
      std::enable_if_t<
          std::is_constructible_v<T, std::initializer_list<U>&, Args...>,
          int> = 0>
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
      std::enable_if_t<std::is_constructible_v<E, Args...>, int> = 0>
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
      std::enable_if_t<
          std::is_constructible_v<E, std::initializer_list<U>&, Args...>,
          int> = 0>
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

  expected& operator=(const expected& other) {
    // To properly SFINAE this, we need to move operator= to a base class
    static_assert(
        std::is_copy_assignable_v<T> && std::is_copy_constructible_v<T> &&
        std::is_copy_assignable_v<E> && std::is_copy_constructible_v<E>);
    static_assert(
        std::is_nothrow_move_constructible_v<T> ||
        std::is_nothrow_move_constructible_v<E>);
    if (other.has_value()) {
      if (has_value()) {
        **this = *other;
      } else {
        reinit<T>(*other);
      }
    } else {
      if (has_value()) {
        reinit<unexpected<E>>(other.error());
      } else {
        error() = other.error();
      }
    }
    return *this;
  }

  expected& operator=(expected&& other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_assignable_v<T> &&
      std::is_nothrow_move_constructible_v<E> &&
      std::is_nothrow_move_assignable_v<E>) {
    // To properly SFINAE this, we need to move operator= to a base class
    static_assert(
        std::is_move_assignable_v<T> && std::is_move_constructible_v<T> &&
        std::is_move_assignable_v<E> && std::is_move_constructible_v<E>);
    static_assert(
        std::is_nothrow_move_constructible_v<T> ||
        std::is_nothrow_move_constructible_v<E>);

    if (other.has_value()) {
      if (has_value()) {
        **this = *std::move(other);
      } else {
        reinit<T>(*std::move(other));
      }
    } else {
      if (has_value()) {
        reinit<unexpected<E>>(std::move(other).error());
      } else {
        error() = std::move(other).error();
      }
    }
    return *this;
  }

  template <
      typename U = T,
      std::enable_if_t<is_forward_assignable_from<U>, int> = 0>
  expected& operator=(U&& value) {
    if (has_value()) {
      **this = std::forward<U>(value);
    } else {
      reinit<T>(std::forward<U>(value));
    }
    return *this;
  }

  template <
      typename G,
      std::enable_if_t<is_forward_assignable_from_unexpected<const G&>, int> =
          0>
  expected& operator=(const unexpected<G>& e) {
    if (has_value()) {
      reinit<unexpected<E>>(e.error());
    } else {
      error() = e.error();
    }
    return *this;
  }

  template <
      typename G,
      std::enable_if_t<is_forward_assignable_from_unexpected<G>, int> = 0>
  expected& operator=(unexpected<G>&& e) {
    if (has_value()) {
      reinit<unexpected<E>>(std::move(e).error());
    } else {
      error() = std::move(e).error();
    }
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
      std::enable_if_t<std::is_nothrow_constructible_v<T, Args...>, int> = 0>
  T& emplace(Args&&... args) noexcept {
    return storage_.template emplace<T>(std::forward<Args>(args)...);
  }

  template <
      typename U,
      typename... Args,
      std::enable_if_t<
          std::
              is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>,
          int> = 0>
  T& emplace(std::initializer_list<U> ilist, Args&&... args) noexcept {
    return storage_.template emplace<T>(ilist, std::forward<Args>(args)...);
  }

  void swap(expected& other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_swappable_v<T> &&
      std::is_nothrow_move_constructible_v<E> &&
      std::is_nothrow_swappable_v<E>) {
    using std::swap;
    swap(storage_, other.storage_);
  }

  friend void swap(expected& lhs, expected& rhs) noexcept(
      noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
  }

  template <
      typename T2,
      typename E2,
      std::enable_if_t<!std::is_void_v<T2>, int> = 0>
  friend bool operator==(
      const expected<T, E>& lhs, const expected<T2, E2>& rhs) {
    return lhs.has_value() ? (rhs.has_value() && *lhs == *rhs)
                           : (!rhs.has_value() && lhs.error() == rhs.error());
  }
  template <typename T2>
  friend bool operator==(const expected<T, E>& lhs, const T2& rhs) {
    return lhs.has_value() && *lhs == rhs;
  }
  template <typename E2>
  friend bool operator==(const expected<T, E>& lhs, const unexpected<E2>& rhs) {
    return !lhs.has_value() && lhs.error() == rhs.error();
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

  // https://en.cppreference.com/w/cpp/utility/expected/operator%3D#Helper_function_template
  template <typename New, typename Current, typename... Args>
  void reinit_impl(Args&&... args) {
    assert(!storage_.valueless_by_exception());
    if constexpr (std::is_nothrow_constructible_v<New, Args...>) {
      storage_.template emplace<New>(std::forward<Args>(args)...);
    } else if constexpr (std::is_nothrow_move_constructible_v<New>) {
      New tmp(std::forward<Args>(args)...);
      storage_.template emplace<New>(std::move(tmp));
    } else {
      static_assert(std::is_nothrow_move_constructible_v<Current>);
      Current tmp(std::move(std::get<Current>(storage_)));
      try {
        storage_.template emplace<New>(std::forward<Args>(args)...);
      } catch (...) {
        storage_.template emplace<Current>(std::move(tmp));
        throw;
      }
    }
  }

  template <typename New, typename... Args>
  void reinit(Args&&... args) {
    using active_alternative =
        std::conditional_t<std::is_same_v<New, T>, unexpected<E>, T>;
    assert(std::holds_alternative<active_alternative>(storage_));
    reinit_impl<New, active_alternative>(std::forward<Args>(args)...);
  }

  std::variant<T, unexpected<E>> storage_;
};

/**
 * Returns true if the contained value is an error with precisely the given
 * type.
 */
template <typename E, typename T, typename... Errors>
[[nodiscard]] bool has_error(const expected<T, std::variant<Errors...>>& e) {
  return !e.has_value() && std::holds_alternative<E>(e.error());
}

/**
 * Returns the error of precisely the given type, if such error is the
 * contained.
 *
 * Pre-conditions:
 *   - has_error<E>(e) == true
 */
template <typename E, typename T, typename... Errors>
[[nodiscard]] const E& get_error(
    const expected<T, std::variant<Errors...>>& e) {
  return std::get<E>(e.error());
}
template <typename E, typename T, typename... Errors>
[[nodiscard]] E&& get_error(expected<T, std::variant<Errors...>>&& e) {
  return std::get<E>(std::move(e).error());
}

/**
 * Visits either the contained value or error.
 */
template <typename T, typename E, typename... Visitors>
decltype(auto) visit(const expected<T, E>& e, Visitors&&... visitors) {
  auto overloaded = detail::overload(std::forward<Visitors>(visitors)...);
  if (e.has_value()) {
    return overloaded(*e);
  }
  return overloaded(e.error());
}
template <typename T, typename E, typename... Visitors>
decltype(auto) visit(expected<T, E>&& e, Visitors&&... visitors) {
  auto overloaded = detail::overload(std::forward<Visitors>(visitors)...);
  if (e.has_value()) {
    return overloaded(*std::move(e));
  }
  return overloaded(std::move(e).error());
}

/**
 * Visits either the contained value or one of the error types.
 */
template <typename T, typename... Errors, typename... Visitors>
decltype(auto) visit(
    const expected<T, std::variant<Errors...>>& e, Visitors&&... visitors) {
  auto overloaded = detail::overload(std::forward<Visitors>(visitors)...);
  if (e.has_value()) {
    return overloaded(*e);
  }
  return std::visit(overloaded, e.error());
}
template <typename T, typename... Errors, typename... Visitors>
decltype(auto) visit(
    expected<T, std::variant<Errors...>>&& e, Visitors&&... visitors) {
  auto overloaded = detail::overload(std::forward<Visitors>(visitors)...);
  if (e.has_value()) {
    return overloaded(*std::move(e));
  }
  return std::visit(overloaded, std::move(e).error());
}

} // namespace whisker
