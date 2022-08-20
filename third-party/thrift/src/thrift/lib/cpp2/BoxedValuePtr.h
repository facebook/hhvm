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

#include <memory>

#include <folly/CPortability.h>

namespace apache {
namespace thrift {
namespace detail {

template <typename T>
class boxed_value_ptr {
 public:
  using element_type = T;

  FOLLY_ERASE constexpr boxed_value_ptr() noexcept = default;
  FOLLY_ERASE constexpr boxed_value_ptr(const boxed_value_ptr& other)
      : ptr_(other.copy()) {}
  FOLLY_ERASE constexpr boxed_value_ptr(boxed_value_ptr&& other) noexcept =
      default;
  FOLLY_ERASE constexpr boxed_value_ptr(std::nullptr_t) : ptr_(nullptr) {}

  FOLLY_ERASE constexpr T& operator*() const noexcept { return *ptr_; }
  FOLLY_ERASE constexpr T* operator->() const noexcept { return ptr_.get(); }
  FOLLY_ERASE constexpr explicit operator bool() const noexcept {
    return bool(ptr_);
  }
  FOLLY_ERASE constexpr boxed_value_ptr& operator=(
      boxed_value_ptr&& other) noexcept = default;
  FOLLY_ERASE
  constexpr boxed_value_ptr& operator=(const boxed_value_ptr& other) {
    return (ptr_ = other.copy(), *this);
  }
  FOLLY_ERASE constexpr boxed_value_ptr& operator=(std::nullptr_t) noexcept {
    return (ptr_ = nullptr, *this);
  }

  FOLLY_ERASE constexpr void reset(T* p = nullptr) noexcept { ptr_.reset(p); }

 private:
  friend void swap(boxed_value_ptr& lhs, boxed_value_ptr& rhs) noexcept {
    using std::swap;
    swap(lhs.ptr_, rhs.ptr_);
  }

  friend constexpr bool operator==(
      const boxed_value_ptr& lhs, const boxed_value_ptr& rhs) noexcept {
    return lhs.ptr_ == rhs.ptr_;
  }
  friend constexpr bool operator==(
      std::nullptr_t, const boxed_value_ptr& rhs) noexcept {
    return nullptr == rhs.ptr_;
  }
  friend constexpr bool operator==(
      const boxed_value_ptr& lhs, std::nullptr_t) noexcept {
    return lhs.ptr_ == nullptr;
  }
  friend constexpr bool operator!=(
      const boxed_value_ptr& lhs, const boxed_value_ptr& rhs) noexcept {
    return !(lhs == rhs);
  }
  friend constexpr bool operator!=(
      const boxed_value_ptr& lhs, std::nullptr_t) noexcept {
    return lhs.ptr_ != nullptr;
  }
  friend constexpr bool operator!=(
      std::nullptr_t, const boxed_value_ptr& rhs) noexcept {
    return nullptr != rhs.ptr_;
  }

  std::unique_ptr<T> ptr_;

  std::unique_ptr<T> copy() const {
    return ptr_ == nullptr ? nullptr : std::make_unique<T>(*ptr_);
  }

 public:
  // TODO(afuller): This implicitly inplace implicit constructor
  // should be removed as it lets T intercept any constructor call, which is
  // extremely bug prone.
  template <typename... Args, typename = decltype(T(std::declval<Args>()...))>
  FOLLY_ERASE boxed_value_ptr(Args&&... args)
      : ptr_(std::make_unique<T>(std::forward<Args>(args)...)) {}

  // TODO(afuller): This implicit creation and value assignment should be
  // removed as it lets T intercept assignment, which is extremely bug prone.
  template <typename U>
  FOLLY_ERASE
      std::enable_if_t<std::is_assignable<T&, U&&>::value, boxed_value_ptr&>
      operator=(U&& value) {
    if (ptr_ == nullptr) {
      ptr_ = std::make_unique<T>();
    }

    *ptr_ = std::forward<U>(value);
    return *this;
  }

  // TODO(afuller): This is not a typical smart pointer feature, and
  // likely should only exist on value-semantic types like std::optional.
  template <typename... Args>
  FOLLY_ERASE T& emplace(Args&&... args) {
    if (ptr_) {
      *ptr_ = T(std::forward<Args>(args)...);
    } else {
      ptr_ = std::make_unique<T>(std::forward<Args>(args)...);
    }
    return *ptr_;
  }
};

} // namespace detail
} // namespace thrift
} // namespace apache
