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
#include <type_traits>

#include <folly/CPortability.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/type/AlignedPtr.h>

namespace apache {
namespace thrift {
namespace detail {

// boxed_value_ptr will soon be replaced with boxed_ptr (defined below).
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

template <typename T>
class boxed_ptr {
 public:
  static_assert(
      std::is_constructible<T, T&>::value ||
          std::is_constructible<T, const T&>::value,
      "Boxed type must be copy constructible");

  using element_type = T;

  FOLLY_ERASE constexpr boxed_ptr() noexcept = default;

  FOLLY_ERASE constexpr boxed_ptr(const boxed_ptr& other) {
    if (other.getMode() == Mode::MutOwned) {
      ptr_ = other.copy();
    } else {
      ptr_ = other.ptr_;
    }
  }

  FOLLY_ERASE constexpr boxed_ptr(boxed_ptr&& other) noexcept : boxed_ptr() {
    std::swap(ptr_, other.ptr_);
  }

  FOLLY_ERASE constexpr boxed_ptr(const T* p) noexcept {
    // The const-ness of the pointer will be tracked using the tag
    // Mode::ConstUnowned. The only mutable accessor method is
    // boxed_ptr<T>::mut(), which will copy *p in order to return a
    // mutable pointer.
    ptr_.set(
        const_cast<T*>(p), static_cast<std::uintptr_t>(Mode::ConstUnowned));
  }

  FOLLY_ERASE ~boxed_ptr() noexcept { destroy(); }

  FOLLY_ERASE constexpr const T& operator*() const noexcept {
    return *ptr_.get();
  }

  FOLLY_ERASE constexpr const T* operator->() const noexcept {
    return ptr_.get();
  }

  FOLLY_ERASE constexpr explicit operator bool() const noexcept {
    return bool(ptr_.get());
  }

  FOLLY_ERASE constexpr boxed_ptr& operator=(boxed_ptr&& other) noexcept {
    std::swap(ptr_, other.ptr_);
    return *this;
  }

  FOLLY_ERASE
  constexpr boxed_ptr& operator=(const boxed_ptr& other) {
    boxed_ptr<T> tmp{other};
    std::swap(ptr_, tmp.ptr_);
    return *this;
  }

  FOLLY_ERASE
  constexpr boxed_ptr& operator=(const T* p) {
    boxed_ptr<T> tmp{p};
    std::swap(ptr_, tmp.ptr_);
    return *this;
  }

  FOLLY_ERASE constexpr T* mut() noexcept {
    if (getMode() == Mode::ConstUnowned) {
      ptr_ = copy();
    }
    return ptr_.get();
  }

  FOLLY_ERASE constexpr void reset(T* p = nullptr) noexcept {
    destroy();
    ptr_.set(p, static_cast<std::uintptr_t>(Mode::MutOwned));
  }

 private:
  static constexpr int kModeBits = 3;
  using aligned_pointer_type =
      type::detail::AlignedPtr<T, kModeBits, kModeBits>;

  enum Mode : std::uintptr_t {
    MutOwned = 0, // Unique ownership
    ConstUnowned = 1,
  };

  friend void swap(boxed_ptr& lhs, boxed_ptr& rhs) noexcept {
    using std::swap;
    swap(lhs.ptr_, rhs.ptr_);
  }

  friend constexpr bool operator==(
      const boxed_ptr& lhs, const boxed_ptr& rhs) noexcept {
    return lhs.ptr_.get() == rhs.ptr_.get();
  }
  friend constexpr bool operator==(
      std::nullptr_t, const boxed_ptr& rhs) noexcept {
    return nullptr == rhs.ptr_.get();
  }
  friend constexpr bool operator==(
      const boxed_ptr& lhs, std::nullptr_t) noexcept {
    return lhs.ptr_.get() == nullptr;
  }
  friend constexpr bool operator!=(
      const boxed_ptr& lhs, const boxed_ptr& rhs) noexcept {
    return !(lhs == rhs);
  }
  friend constexpr bool operator!=(
      const boxed_ptr& lhs, std::nullptr_t) noexcept {
    return lhs.ptr_.get() != nullptr;
  }
  friend constexpr bool operator!=(
      std::nullptr_t, const boxed_ptr& rhs) noexcept {
    return nullptr != rhs.ptr_.get();
  }

  Mode getMode() const { return static_cast<Mode>(ptr_.getTag()); }

  void destroy() {
    if (getMode() == Mode::MutOwned) {
      delete ptr_.get();
    }
    ptr_.clear();
  }

  aligned_pointer_type copy() const {
    // This leaves the tag region cleared indicating the new value is
    // Mode::MutOwned.
    aligned_pointer_type ptr =
        ptr_.get() == nullptr ? nullptr : new T(*ptr_.get());
    assert(ptr.getTag() == Mode::MutOwned);
    return ptr;
  }

  aligned_pointer_type ptr_;

 public:
  // TODO(afuller): This implicitly inplace implicit constructor
  // should be removed as it lets T intercept any constructor call, which is
  // extremely bug prone.
  template <typename... Args, typename = decltype(T(std::declval<Args>()...))>
  FOLLY_ERASE boxed_ptr(Args&&... args)
      : ptr_(new T(std::forward<Args>(args)...), Mode::MutOwned) {}

  // TODO(afuller): This implicit creation and value assignment should be
  // removed as it lets T intercept assignment, which is extremely bug prone.
  //
  // TODO(lpe): Replace this with make_boxed<T>.
  template <typename U>
  FOLLY_ERASE std::enable_if_t<std::is_assignable<T&, U&&>::value, boxed_ptr&>
  operator=(U&& value) {
    if (ptr_.get()) {
      *ptr_.get() = std::forward<U>(value);
    } else {
      ptr_.set(new T(std::forward<U>(value)), Mode::MutOwned);
    }
    return *this;
  }

  // TODO(afuller): This is not a typical smart pointer feature, and
  // likely should only exist on value-semantic types like std::optional.
  template <typename... Args>
  FOLLY_ERASE T& emplace(Args&&... args) {
    if (ptr_.get()) {
      *ptr_.get() = T(std::forward<Args>(args)...);
    } else {
      ptr_.set(new T(std::forward<Args>(args)...), Mode::MutOwned);
    }
    return *ptr_.get();
  }
};

} // namespace detail
} // namespace thrift
} // namespace apache
