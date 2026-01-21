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

#include <stdexcept>
#include <type_traits>
#include <variant>

#include <folly/ExceptionWrapper.h>
#include <folly/Indestructible.h>
#include <folly/Poly.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Type.h>

namespace apache::thrift::type::detail {

// The interface every data holder implements.
struct IAnyData {
  // Define the interface of AnyValueData:
  template <class Base>
  struct Interface : Base {
    bool empty() const { return folly::poly_call<0>(*this); }
    void clear() { folly::poly_call<1>(*this); }
    bool identical(const folly::PolySelf<Base>& other) const {
      return folly::poly_call<2>(*this, other);
    }
    folly::exception_wrapper asExceptionWrapper() const {
      return folly::poly_call<3>(*this);
    }
  };

  template <class T>
  using Members = folly::
      PolyMembers<&T::empty, &T::clear, &T::identical, &T::asExceptionWrapper>;
};

// A holder that implements IAnyDataHolder for any type T, that can be used with
// the Thrift op library, for the given type Tag.
//
// T can be a (const) value, reference or pointer type.
//
// For smart pointers, set isPointer = true.
template <
    typename Tag,
    typename T = native_type<Tag>,
    bool isPointer = std::is_pointer_v<T>>
struct AnyData {
  static_assert(is_concrete_v<Tag>);
  T data;

  T& get() { return data; }
  const T& get() const { return data; }

  constexpr bool empty() const noexcept { return op::isEmpty<Tag>(data); }
  constexpr void clear() {
    // For const types, we cannot clear the value. So do nothing and keep
    // method to satisfy interface requirements.
    if constexpr (!std::is_const_v<std::remove_reference_t<T>>) {
      op::clear<Tag>(data);
    } else {
      throw std::runtime_error("clear() is not supported on a const type");
    }
  }
  constexpr bool identical(const AnyData& other) const noexcept {
    return op::identical<Tag>(data, other.data);
  }
  folly::exception_wrapper asExceptionWrapper() const {
    if constexpr (!std::is_base_of_v<apache::thrift::TException, T>) {
      return {};
    } else {
      return data;
    }
  }
};
template <typename Tag, typename T>
struct AnyData<Tag, T, true> {
  static_assert(is_concrete_v<Tag>);
  T ptr;

  // Let the pointer decide the constness semantics.
  decltype(auto) get() { return *ptr; }
  decltype(auto) get() const { return *ptr; }

  // IAnyData impl
  constexpr bool empty() const noexcept { return op::isEmpty<Tag>(*ptr); }
  constexpr void clear() noexcept { op::clear<Tag>(*ptr); }
  constexpr bool identical(const AnyData& other) const noexcept {
    return op::identical<Tag>(*ptr, *other.ptr);
  }
  folly::exception_wrapper asExceptionWrapper() const { return {}; }
};

// void_t had no data.
template <typename T, bool isPointer>
struct AnyData<void_t, T, isPointer> {
  // Note: `get()` method is intentionally missing to cause a compile time error
  // if accessing the value is depended on.

  // IAnyData impl
  constexpr bool empty() const noexcept { return true; }
  constexpr void clear() noexcept {}
  constexpr bool identical(AnyData) const { return true; }
  folly::exception_wrapper asExceptionWrapper() const { return {}; }
};

template <bool Const, typename RefBase, typename... OtherBases>
class AnyRefBase;

// An abstract base class for all AnyData-based types.
template <typename I>
class AnyBase {
  static_assert(std::is_same_v<folly::remove_cvref_t<I>, IAnyData>);

  using Holder = folly::Poly<IAnyData>;

 protected:
  template <typename Tag>
  using AnyData = AnyData<Tag, folly::like_t<I, native_type<Tag>>>;

  constexpr const Type& type() const { return type_; }

  bool empty() const noexcept { return data_.empty(); }

  template <
      class U = I,
      std::enable_if_t<!std::is_const_v<std::remove_reference_t<U>>, bool> =
          true>
  void clear() noexcept {
    data_.clear();
  }

  // Throws folly::BadPolyCast if the underlying types are incompatible.
  bool identical(const AnyBase& other) const {
    return type_ == other.type_ && data_.identical(other.data_);
  }

  folly::exception_wrapper asExceptionWrapper() const {
    return data_.asExceptionWrapper();
  }

  // Dynamic type constructor.
  AnyBase(Type&& type, Holder&& data) noexcept
      : type_(std::move(type)), data_(std::move(data)) {}

  // Inplace constructor.
  template <ConcreteThriftTypeTag Tag, typename... Args>
  constexpr explicit AnyBase(Tag t, Args&&... args) noexcept
      : type_(t), data_(AnyData<Tag>{{std::forward<Args&&>(args)...}}) {}

  // Configure to be an abstract base class with value semantics.
  constexpr AnyBase() = default;
  AnyBase(const AnyBase&) = default;
  AnyBase(AnyBase&&) noexcept = default;
  ~AnyBase() = default;

  AnyBase& operator=(const AnyBase&) = default;
  AnyBase& operator=(AnyBase&&) noexcept = default;

  // Actual return types are determined by the AnyData
  // specialization being used.
  template <typename Tag>
  decltype(auto) as() & {
    return folly::poly_cast<AnyData<Tag>&>(data_).get();
  }
  template <typename Tag>
  decltype(auto) as() && {
    return folly::poly_cast<AnyData<Tag>&&>(data_).get();
  }
  template <typename Tag>
  decltype(auto) as() const& {
    return folly::poly_cast<const AnyData<Tag>&>(data_).get();
  }
  template <typename Tag>
  decltype(auto) as() const&& {
    return folly::poly_cast<const AnyData<Tag>&&>(data_).get();
  }

  template <typename Tag>
  decltype(auto) try_as() {
    // TODO(afuller): This is likely double checking the types match. Add a
    // non-throwing poly_cast overload?
    return typeid(AnyData<Tag>) == folly::poly_type(data_) ? &as<Tag>()
                                                           : nullptr;
  }

  template <typename Tag>
  decltype(auto) try_as() const {
    return typeid(AnyData<Tag>) == folly::poly_type(data_) ? &as<Tag>()
                                                           : nullptr;
  }

 private:
  Type type_;
  Holder data_ = apache::thrift::type::detail::AnyData<void_t, void>{};

  template <bool Const, typename RefBase, typename... OtherBases>
  friend class AnyRefBase;
};

// Base classes for the public classes.
using AnyValueBase = AnyBase<IAnyData>;

template <typename I>
class AnyRefWrapper : AnyBase<I> {
  using Base = AnyBase<I>;

 public:
  constexpr AnyRefWrapper() = default;
  AnyRefWrapper(const AnyRefWrapper&) = default;
  AnyRefWrapper(AnyRefWrapper&&) noexcept = default;
  ~AnyRefWrapper() = default;

  AnyRefWrapper& operator=(const AnyRefWrapper&) = default;
  AnyRefWrapper& operator=(AnyRefWrapper&&) noexcept = default;

  // Inplace constructor.
  template <ConcreteThriftTypeTag Tag, typename... Args>
  constexpr explicit AnyRefWrapper(Tag t, Args&&... args) noexcept
      : Base(std::move(t), std::forward<Args&&>(args)...) {}

  using Base::as;
  using Base::empty;
  using Base::type;
};

template <bool Const, typename RefBase, typename... OtherBases>
class AnyRefBase {
  using AnyValueRef =
      std::conditional_t<Const, const AnyValueBase&, AnyValueBase&>;
  using AnyValuePtr =
      std::conditional_t<Const, AnyValueBase const*, AnyValueBase*>;

  template <class Tag>
  using RefType =
      std::conditional_t<Const, const native_type<Tag>&, native_type<Tag>&>;

 public:
  AnyRefBase(const AnyRefBase&) = default;
  AnyRefBase(AnyRefBase&&) noexcept = default;
  ~AnyRefBase() = default;

  AnyRefBase& operator=(const AnyRefBase&) = default;
  AnyRefBase& operator=(AnyRefBase&&) noexcept = default;

  // Implicit initializer from Thrift types.
  template <typename T, typename Tag = infer_tag<T>>
  /* implicit */ AnyRefBase(T&& ref)
      : ref_(RefBase(Tag{}, std::forward<T&&>(ref))) {}

  // Implicit initializer from AnyValue.
  /* implicit */ AnyRefBase(AnyValueRef ref) : ref_(&ref) {}

  // Conversion from mutable Ref to ConstRef.
  template <bool Enabled = Const, std::enable_if_t<Enabled, bool> = true>
  /* implicit */ AnyRefBase(
      const AnyRefBase<false, AnyRefWrapper<IAnyData&>>& other) {
    *this = other;
  }

  template <bool Enabled = Const, std::enable_if_t<Enabled, bool> = true>
  AnyRefBase& operator=(
      const AnyRefBase<false, AnyRefWrapper<IAnyData&>>& other) {
    other.invoke([this](auto&& otherRef) {
      *this = otherRef;
      return true;
    });
    return *this;
  }

  constexpr const Type& type() const {
    return invoke([](auto&& arg) -> decltype(auto) { return arg.type(); });
  }

  bool empty() const noexcept {
    return invoke([](auto&& arg) -> decltype(auto) { return arg.empty(); });
  }

  template <typename Tag>
  decltype(auto) as() {
    return invoke(
        [](auto&& arg) -> RefType<Tag> { return arg.template as<Tag>(); });
  }
  template <typename Tag>
  decltype(auto) as() const {
    return invoke([](auto&& arg) -> const RefType<Tag> {
      return arg.template as<Tag>();
    });
  }

 private:
  std::variant<RefBase, AnyValuePtr, OtherBases...> ref_;

  template <class I>
  AnyRefBase& operator=(const AnyRefWrapper<I>& wrapper) {
    ref_ = wrapper;
    return *this;
  }

  template <typename F>
  decltype(auto) invoke(F&& f) {
    return std::visit(
        [&](auto&& arg) -> decltype(auto) {
          if constexpr (std::is_pointer_v<
                            std::remove_reference_t<decltype(arg)>>) {
            return f(*arg);
          } else {
            return f(arg);
          }
        },
        ref_);
  }

  template <typename F>
  decltype(auto) invoke(F&& f) const {
    return std::visit(
        [&](auto&& arg) -> decltype(auto) {
          if constexpr (std::is_pointer_v<
                            std::remove_reference_t<decltype(arg)>>) {
            return f(*arg);
          } else {
            return f(arg);
          }
        },
        ref_);
  }

  template <bool Const2, typename RefBase2, typename... OtherBases2>
  friend class AnyRefBase;
};

} // namespace apache::thrift::type::detail
