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

#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/cpp2/type/detail/AnyValue.h>

namespace apache {
namespace thrift {
namespace type {

// A type-erased Thrift value.
//
// TODO(afuller): Harden and stress test type::Value and migrage usage once
// satisfied.
class AnyValue : public detail::AnyValueBase {
  using Base = detail::AnyValueBase;

 public:
  // The null, nil, None, falsum (or whatever you want to call it) value.
  AnyValue() = default;

  template <typename Tag, typename... Args>
  static AnyValue create(Args&&... args) {
    return AnyValue{Tag{}, std::forward<Args>(args)...};
  }

  // Type safe access to the internal storage.
  //
  // Throws folly::BadPolyCast if the wrong Tag is used.
  //
  //   template<typename Tag> native_type<Tag>& as() &;
  //   template<typename Tag> native_type<Tag>&& as() &&;
  //   template<typename Tag> const native_type<Tag>& as() const &;
  //   template<typename Tag> const native_type<Tag>&& as() const &&;
  //
  using Base::as;

  // Type safe access to the internal storage.
  //
  // Returns nullptr if the wrong Tag is used.
  //
  //   template<typename Tag> native_type<Tag>* try_as() noexcept;
  //   template<typename Tag> const native_type<Tag>* try_as() const noexcept;
  //
  using Base::try_as;

  // The runtime type of the stored value (or void_t{} if no value is stored)
  //
  //   const Type& type() const noexcept;
  //
  using Base::type;

  // If the stored value is 'empty'. See `op::empty`.
  //
  //   bool empty() const noexcept;
  //
  using Base::empty;

  // Determines if the give AnyValue is identical to this one.
  //
  // Two AnyValues are 'identical' if they have the same `type()` and
  // the values stored are also identical. See `op::identical`.
  //
  // Throws folly::BadPolyCast if the underlying Tags are different, but have
  // the same Type.
  //
  //   bool identical(const AnyValue& other);
  //
  using Base::identical;

  // Clears the underlying value, leaving it equal it to it's intrinsic default.
  //
  // See `op::clear`
  //
  //   void clear() noexcept;
  //
  using Base::clear;

  // Returns held Thrift exception wrapped in folly::exception_wrapper.
  //
  // Returns an empty folly::exception_wrapper iff held type is not a Thrift
  // exception.
  using Base::asExceptionWrapper;

 private:
  using Base::Base;
};

} // namespace type
} // namespace thrift
} // namespace apache
