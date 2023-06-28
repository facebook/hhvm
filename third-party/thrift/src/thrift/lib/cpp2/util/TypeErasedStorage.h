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

#include <string>
#include <typeinfo>
#include <glog/logging.h>

namespace apache::thrift::util {

namespace detail {

class VTable {
 public:
  explicit VTable(const std::type_info& typeInfo) noexcept
      : typeInfo_(typeInfo) {}
  virtual void destroy(std::byte*) const noexcept = 0;
  virtual void move(std::byte* dst, std::byte* src) const noexcept = 0;
  virtual void move_assign(std::byte* dst, std::byte* src) const noexcept = 0;

  template <class T>
  bool holds_alternative() const noexcept {
    return typeid(T) == typeInfo_;
  }

  virtual ~VTable() = default;

 private:
  const std::type_info& typeInfo_;
};

template <class T>
class VTableImpl final : public VTable {
 public:
  VTableImpl() noexcept : VTable(typeid(T)) {}

  void destroy(std::byte* storage) const noexcept override final {
    getValue(storage).~T();
  }
  void move(
      std::byte* origin, std::byte* destination) const noexcept override final {
    new (destination) T(std::move(getValue(origin)));
  }
  void move_assign(
      std::byte* origin, std::byte* destination) const noexcept override final {
    getValue(destination) = std::move(getValue(origin));
  }

 private:
  static T& getValue(std::byte* storage) noexcept {
    return *reinterpret_cast<T*>(storage);
  }
};
template <class T>
inline const VTableImpl<T> vTableImpl;

class TypeErasedStorageImplBase {
 public:
  bool has_value() const noexcept { return vtable_; }

  template <class T>
  bool holds_alternative() const noexcept {
    return has_value() && vtable_->holds_alternative<T>();
  }

 protected:
  TypeErasedStorageImplBase() noexcept = default;

  TypeErasedStorageImplBase(TypeErasedStorageImplBase&& other) noexcept =
      default;
  TypeErasedStorageImplBase& operator=(
      TypeErasedStorageImplBase&& other) noexcept = default;

  /* Delete copy Ctors */
  TypeErasedStorageImplBase(const TypeErasedStorageImplBase& other) = delete;
  TypeErasedStorageImplBase& operator=(const TypeErasedStorageImplBase& other) =
      delete;

  const VTable* vtable_{nullptr};
};

/*
 * Internal fields are allocated on the stack, alongside other variables related
 * to Cpp2ConnContext and Cpp2RequestContext.
 *
 * They are used to store non-OSS components in a "safe ish" way.
 */
template <std::size_t Size, std::size_t Align>
class TypeErasedStorageImpl : public TypeErasedStorageImplBase {
 private:
  alignas(Align) std::byte storage_[Size];

  template <class T>
  constexpr void validate() const noexcept {
    static_assert(
        sizeof(T) <= getSize(), "Class is too large to fit in TypeErasedValue");
    static_assert(alignof(T) <= getAlign(), "Class alignment mismatch");
    static_assert(
        std::is_nothrow_move_constructible<T>::value,
        "Class move constructor cannot throw");
    static_assert(
        std::is_nothrow_destructible<T>::value,
        "Class destructor cannot throw");
  }

 protected:
  TypeErasedStorageImpl() noexcept = default;

 public:
  TypeErasedStorageImpl(TypeErasedStorageImpl&& other) noexcept
      : TypeErasedStorageImplBase(std::move(other)) {
    if (has_value()) {
      vtable_->move(other.storage_, storage_);
      other.reset();
    }
  }

  TypeErasedStorageImpl& operator=(TypeErasedStorageImpl&& other) noexcept {
    vtable_ = other.vtable_;
    if (has_value()) {
      vtable_->move_assign(other.storage_, storage_);
      other.reset();
    }
    return *this;
  }

  static constexpr size_t getSize() noexcept { return Size; }
  static constexpr size_t getAlign() noexcept { return Align; }

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

  ~TypeErasedStorageImpl() noexcept { reset(); }

  template <class T>
  const T& value() const {
    validate<T>();
    CHECK(has_value());
    if (!holds_alternative<T>()) {
      throw std::bad_cast();
    }
    return value_unchecked<T>();
  }

  template <class T>
  T& value() {
    validate<T>();
    CHECK(has_value());
    if (!holds_alternative<T>()) {
      throw std::bad_cast();
    }
    return value_unchecked<T>();
  }

  template <class T>
  const T& value_unchecked() const noexcept {
    validate<T>();
    DCHECK(holds_alternative<T>());
    return *reinterpret_cast<const T*>(storage_);
  }

  template <class T>
  T& value_unchecked() noexcept {
    validate<T>();
    DCHECK(holds_alternative<T>());
    return *reinterpret_cast<T*>(storage_);
  }
};

} // namespace detail

// TypeErasedStorage is intended for use in situations where:
//
// - You need stack allocation (can't use `std::unique_ptr<void>`)
// - The type of the object you want to store is not well known.
//
// API:
// .emplace<T>(...) constructs a new value in place
// .value<T>() -> gets the value, but does expensive runtime type validation
// .value_unchecked<T>() -> gets the value, no type validation
template <std::size_t Size, std::size_t Align>
class TypeErasedStorage final
    : public detail::TypeErasedStorageImpl<Size, Align> {};

// TypeErasedValue is similar to TypeErasedStorage, except:
//
// - The size is known at construction time
// - You never want the storage to be empty.
//
template <std::size_t Size, std::size_t Align>
class TypeErasedValue final
    : private detail::TypeErasedStorageImpl<Size, Align> {
 public:
  template <class T, class... Args>
  static TypeErasedValue make(Args&&... args) {
    TypeErasedValue value;
    value.template emplace<T>(std::forward<Args>(args)...);
    return value;
  }

  static TypeErasedValue makeEmpty() { return make<std::monostate>(); }

  using detail::TypeErasedStorageImpl<Size, Align>::emplace;
  using detail::TypeErasedStorageImpl<Size, Align>::holds_alternative;
  using detail::TypeErasedStorageImpl<Size, Align>::value;
  using detail::TypeErasedStorageImpl<Size, Align>::value_unchecked;
  using detail::TypeErasedStorageImpl<Size, Align>::getSize;
  using detail::TypeErasedStorageImpl<Size, Align>::getAlign;

  /* Prevent construction outside of `make` */
 private:
  TypeErasedValue() noexcept = default;
};

} // namespace apache::thrift::util
