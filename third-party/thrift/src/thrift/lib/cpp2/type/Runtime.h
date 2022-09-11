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

#include <cstddef>
#include <string>
#include <typeinfo>
#include <utility>

#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/op/detail/AnyOp.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/cpp2/type/detail/Runtime.h>

namespace apache {
namespace thrift {
namespace type {

// A light weight (pass-by-value), non-owning *const* reference to a runtime
// Thrift value.
//
// Should typically be passed by value as it only holds two
// ponters; a pointer to the value being reference and a pointer to the static
// runtime metadata associated with the type of the value.
class ConstRef final : public detail::BaseRef<ConstRef> {
  using Base = detail::BaseRef<ConstRef>;

 public:
  ConstRef() noexcept = default;
  // 'capture' any other runtime type.
  /* implicit */ ConstRef(const detail::Dyn& other) noexcept : Base(other) {}
  // 'capture' any native type we can safely infer the tag for.
  template <typename T, typename Tag = infer_tag<T>>
  /* implicit */ ConstRef(T&& val) noexcept
      : ConstRef(Tag{}, std::forward<T>(val)) {}
  // 'capture' a std::string.
  // TODO: infer_tag a string_view type instead.
  /* implicit */ ConstRef(const std::string& val) noexcept
      : ConstRef(binary_t{}, val) {}

 private:
  friend Base;
  template <typename, typename, typename>
  friend class detail::BaseDyn;

  template <typename Tag, typename T>
  ConstRef(Tag, T&& val) : Base(op::detail::getAnyType<Tag, T>(), &val) {}
};

// A light weight (pass-by-value), non-owning reference to a runtime Thrift
// value.
//
// Should typically be passed by value as it only holds two
// ponters; a pointer to the value being reference and a pointer to the static
// runtime metadata associated with the type of the value.
class Ref final : public detail::BaseRef<Ref, ConstRef> {
  using Base = detail::BaseRef<Ref, ConstRef>;
  using Dyn = detail::Dyn;

 public:
  Ref() noexcept = default;

  // Sets the referenced value to it's intrinsic default (e.g. ignoring custom
  // field defaults).
  using Base::clear;

  // Append to a list, string, etc.
  using Base::append;

  // Add to a set, number, etc.
  using Base::add;

  // Put a key-value pair, overwriting any existing entry in a map, struct, etc.
  //
  // Returns true if an existing value was replaced.
  using Base::put;

  // Add a key-value pair, if not already present, using the given default if
  // provided.
  //
  // Returns a reference to the associated value.
  using Base::ensure;

  // Throws on mismatch or if const.
  template <typename Tag>
  native_type<Tag>& mut() {
    return Base::mut<Tag>();
  }

  // Returns nullptr on mismatch or if const.
  template <typename Tag>
  native_type<Tag>* tryMut() noexcept {
    return Base::tryMut<Tag>();
  }

 private:
  friend class detail::Ptr;
  template <typename, typename, typename>
  friend class detail::BaseDyn;
  friend Base;
  using Base::asRef;

  explicit Ref(detail::Ptr data) noexcept : Base(data) {}
  template <typename Tag, typename T>
  Ref(Tag, T&& val) : Base(op::detail::getAnyType<Tag>(), &val) {}

  friend bool operator==(const ConstRef& lhs, const Ref& rhs) {
    return lhs.equal(rhs);
  }
  friend bool operator==(const Ref& lhs, const ConstRef& rhs) {
    return lhs.equal(rhs);
  }
  friend bool operator!=(const ConstRef& lhs, const Ref& rhs) {
    return !lhs.equal(rhs);
  }
  friend bool operator!=(const Ref& lhs, const ConstRef& rhs) {
    return !lhs.equal(rhs);
  }
  friend bool operator<(const ConstRef& lhs, const Ref& rhs) {
    return op::detail::is_lt(lhs.compare(rhs));
  }
  friend bool operator<(const Ref& lhs, const ConstRef& rhs) {
    return op::detail::is_lt(lhs.compare(rhs));
  }
  friend bool operator<=(const ConstRef& lhs, const Ref& rhs) {
    return op::detail::is_lteq(lhs.compare(rhs));
  }
  friend bool operator<=(const Ref& lhs, const ConstRef& rhs) {
    return op::detail::is_lteq(lhs.compare(rhs));
  }
  friend bool operator>(const ConstRef& lhs, const Ref& rhs) {
    return op::detail::is_gt(lhs.compare(rhs));
  }
  friend bool operator>(const Ref& lhs, const ConstRef& rhs) {
    return op::detail::is_gt(lhs.compare(rhs));
  }
  friend bool operator>=(const ConstRef& lhs, const Ref& rhs) {
    return op::detail::is_gteq(lhs.compare(rhs));
  }
  friend bool operator>=(const Ref& lhs, const ConstRef& rhs) {
    return op::detail::is_gteq(lhs.compare(rhs));
  }
};

namespace detail {
inline Ref Ptr::operator*() const noexcept {
  return Ref{*this};
}
} // namespace detail

// A runtime Thrift value that owns it's own memory.
//
// TODO(afuller): Store small values in-situ.
class Value : public detail::BaseDyn<ConstRef, Ref, Value> {
  using Base = detail::BaseDyn<ConstRef, Ref, Value>;
  using Dyn = detail::Dyn;

 public:
  template <typename Tag>
  static Value create() {
    return Value{Tag{}, nullptr};
  }
  template <typename Tag>
  static Value of(const native_type<Tag>& val) {
    return {Tag{}, val};
  }
  template <typename Tag>
  static Value of(native_type<Tag>&& val) {
    return {Tag{}, std::move(val)};
  }
  template <typename Tag>
  static Value of(std::unique_ptr<native_type<Tag>> val) {
    if (val == nullptr) {
      return {};
    }
    return Value{Tag{}, std::move(val)};
  }

  Value() noexcept = default; // A void/null value.
  Value(const Value& other) noexcept
      : Base(other.type_, other.type_->make(other.ptr_, false)) {}

  Value(Value&& other) noexcept : Base(other.type_, other.ptr_) {
    other.Base::reset();
  }
  ~Value() { reset(); }

  Value& operator=(const Value& other) noexcept {
    reset();
    Base::reset(other.type_, other.type_->make(other.ptr_, false));
    return *this;
  }

  Value& operator=(Value&& other) noexcept {
    reset();
    Base::reset(other.type_, other.ptr_);
    other.Base::reset();
    return *this;
  }

  void reset() {
    if (ptr_ != nullptr) {
      type_->delete_(ptr_);
      Base::reset();
    }
  }
  using Base::as;
  template <typename Tag>
  native_type<Tag>& as() & {
    return type_->as<native_type<Tag>>(ptr_);
  }
  template <typename Tag>
  native_type<Tag>&& as() && {
    return std::move(type_->as<native_type<Tag>>(ptr_));
  }

  // Returns nullptr on mismatch.
  template <typename Tag>
  native_type<Tag>* tryAs() noexcept {
    return type_->tryAs<native_type<Tag>>(ptr_);
  }
  using Base::tryAs;

  // Sets the referenced value to it's intrinsic default (e.g. ignoring custom
  // field defaults).
  using Base::clear;

  // Append to a list, string, etc.
  using Base::append;

  // Add to a set, number, etc.
  using Base::add;

  // Put a key-value pair, overwriting any existing entry in a map, struct, etc.
  //
  // Returns true if an existing value was replaced.
  using Base::put;

  // Add a key-value pair, if not already present, using the given default if
  // provided.
  //
  // Returns a reference to the associated value.
  using Base::ensure;

 private:
  using Base::Base;

  template <typename Tag, typename T = native_type<Tag>>
  explicit Value(Tag, std::nullptr_t)
      : Base(
            op::detail::getAnyType<Tag, T>(),
            op::detail::getAnyType<Tag, T>()->make(nullptr, false)) {
    assert(ptr_ != nullptr);
  }
  template <typename Tag, typename T>
  Value(Tag, std::unique_ptr<T> val)
      : Base(op::detail::getAnyType<Tag, T>(), val.release()) {
    assert(ptr_ != nullptr);
  }
  template <typename Tag, typename T>
  Value(Tag, const T& val)
      : Base(
            op::detail::getAnyType<Tag, T>(),
            op::detail::getAnyType<Tag, T>()->make(&val, false)) {
    assert(ptr_ != nullptr);
  }
  template <typename Tag, typename T>
  Value(Tag, T&& val)
      : Base(
            op::detail::getAnyType<Tag, T>(),
            op::detail::getAnyType<Tag, T>()->make(&val, true)) {
    assert(ptr_ != nullptr);
  }

  static ConstRef asRef(const std::string& name) {
    return ConstRef::to<type::string_t>(name);
  }
};

} // namespace type
} // namespace thrift
} // namespace apache
