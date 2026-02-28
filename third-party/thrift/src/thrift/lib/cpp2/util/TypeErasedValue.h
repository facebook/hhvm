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

#include <new>
#include <typeinfo>
#include <utility>

#include <folly/Portability.h>
#include <folly/lang/SafeAssert.h>

namespace apache::thrift::util {

namespace detail {

class VTable {
 public:
  explicit VTable(const std::type_info& typeInfo) noexcept
      : typeInfo_(typeInfo) {}
  virtual void destroy(std::byte*) const noexcept = 0;
  virtual void move(std::byte* dst, std::byte* src) const noexcept = 0;

  const std::type_info& type() const noexcept { return typeInfo_; }

  virtual ~VTable() = default;

 private:
  const std::type_info& typeInfo_;
};

template <class T>
class VTableImpl final : public VTable {
 public:
  VTableImpl() noexcept : VTable(typeid(T)) {}

  void destroy(std::byte* storage) const noexcept final { value(storage).~T(); }
  void move(std::byte* origin, std::byte* destination) const noexcept final {
    new (destination) T(std::move(value(origin)));
  }

 private:
  static T& value(std::byte* storage) noexcept {
    return *std::launder(reinterpret_cast<T*>(storage));
  }
};

template <class T>
inline const VTableImpl<T> vTableImpl;

} // namespace detail

/*
 * A type-erased storage type (like std::any) but with guaranteed sizing and
 * alignment. This is useful, for example, for a closed source type that should
 * be carried through Thrift's runtime but the overhead of a heap allocation to
 * hide the implementation is too high.
 *
 * Unlike `std::any`, which has an unspecified small-buffer, `TypeErasedValue`
 * is ONLY the small buffer. This ensures that there are no hidden heap
 * allocations. Trying to emplace an object that is too large for the buffer is
 * a compile-time error.
 *
 * For the most part, the API of `TypeErasedValue` is quite similar to
 * `std::any`. The main differences versus std::any are:
 *   - `TypeErasedValue` is move-only. This is intentional so that users can
 *     choose to use `std::unique_ptr` to move data to the heap in case the data
 *     is too large.
 *   - `TypeErasedValue` does not have `any_cast<T>()`. Instead there are member
 *     functions, `value<T>()` and `value_unchecked<T>()`.
 *   - `TypeErasedValue`'s constructor does not offer all the variants that
 *     `std::any` does (although this is not a technical limitation).
 *   - `TypeErasedValue` has a member function `holds_alternative<T>()`.
 */
template <std::size_t kSize, std::size_t kAlign = alignof(std::max_align_t)>
class TypeErasedValue final {
 public:
  static constexpr size_t max_size() noexcept { return kSize; }
  static constexpr size_t alignment() noexcept { return kAlign; }

 private:
  alignas(alignment()) std::byte storage_[max_size()];
  const detail::VTable* vtable_{nullptr};

  template <class T>
  FOLLY_ERASE static constexpr void constexpr_validate() noexcept {
    static_assert(sizeof(T) <= max_size(), "Size of type is too large");
    static_assert(alignof(T) <= alignment(), "Alignment of type is too large");
    static_assert(
        std::is_nothrow_move_constructible_v<T>,
        "Class move constructor cannot throw");
    static_assert(
        std::is_nothrow_destructible_v<T>, "Class destructor cannot throw");
  }

 public:
  bool has_value() const noexcept { return vtable_ != nullptr; }

  const std::type_info& type() const noexcept {
    return has_value() ? vtable_->type() : typeid(void);
  }

  template <class T>
  bool holds_alternative() const noexcept {
    return type() == typeid(T);
  }

  void reset() noexcept {
    if (has_value()) {
      vtable_->destroy(storage_);
      vtable_ = nullptr;
    }
  }

  template <class T, class... Args>
  std::decay_t<T>& emplace(Args&&... args) {
    reset();
    new (&storage_) T(std::forward<Args>(args)...);
    vtable_ = &detail::vTableImpl<T>;
    return value_unchecked<T>();
  }

  template <class T>
  const T& value() const {
    constexpr_validate<T>();
    if (!holds_alternative<T>()) {
      throw std::bad_cast();
    }
    return value_unchecked<T>();
  }

  template <class T>
  T& value() {
    constexpr_validate<T>();
    if (!holds_alternative<T>()) {
      throw std::bad_cast();
    }
    return value_unchecked<T>();
  }

  template <class T>
  const T& value_unchecked() const noexcept {
    constexpr_validate<T>();
    FOLLY_SAFE_DCHECK(
        holds_alternative<T>(),
        "Tried to call value_unchecked() on TypeErasedValue with incompatible type");
    return *std::launder(reinterpret_cast<const T*>(storage_));
  }

  template <class T>
  T& value_unchecked() noexcept {
    constexpr_validate<T>();
    FOLLY_SAFE_DCHECK(
        holds_alternative<T>(),
        "Tried to call value_unchecked() on TypeErasedValue with incompatible type");
    return *std::launder(reinterpret_cast<T*>(storage_));
  }

 public:
  TypeErasedValue() noexcept = default;

  template <class T, class... Args>
  explicit TypeErasedValue(std::in_place_type_t<T>, Args&&... args) {
    this->emplace<T>(std::forward<Args>(args)...);
  }

  TypeErasedValue(TypeErasedValue&& other) noexcept
      : vtable_(std::move(other.vtable_)) {
    if (has_value()) {
      vtable_->move(other.storage_, storage_);
      other.reset();
    }
  }

  TypeErasedValue& operator=(TypeErasedValue&& other) noexcept {
    reset();
    vtable_ = other.vtable_;
    if (has_value()) {
      vtable_->move(other.storage_, storage_);
      other.reset();
    }
    return *this;
  }

  ~TypeErasedValue() noexcept { reset(); }
};

} // namespace apache::thrift::util
