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

#include <type_traits>

#include <folly/ExceptionWrapper.h>
#include <folly/Poly.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/type/NativeType.h>

namespace apache {
namespace thrift {
namespace type {
namespace detail {

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
  constexpr void clear() noexcept { op::clear<Tag>(data); }
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

// An abstract base class for all AnyData-based types.
template <typename I>
class AnyBase {
  using Holder = folly::Poly<I>;

 protected:
  template <typename Tag>
  using AnyData = AnyData<Tag, folly::like_t<I, native_type<Tag>>>;

  constexpr const Type& type() const { return type_; }

  bool empty() const noexcept { return data_.empty(); }
  void clear() noexcept { data_.clear(); }

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
  template <typename Tag, typename = if_concrete<Tag>, typename... Args>
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
  Holder data_ = AnyData<void_t>{};
};

// Base classes for the public classes.
using AnyValueBase = AnyBase<IAnyData>;
// TODO(afuller): Implement AnyRef.
// using AnyaRefBase = AnyBase<IAnyData&>;

} // namespace detail
} // namespace type
} // namespace thrift
} // namespace apache
