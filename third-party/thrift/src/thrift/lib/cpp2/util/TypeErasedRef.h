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
#include <type_traits>
#include <typeinfo>
#include <utility>

#include <folly/Traits.h>
#include <folly/lang/SafeAssert.h>

namespace apache::thrift::util {

/**
 * A type-safe type-erased const reference to an object.
 *
 * Unlike TypeErasedValue, the referred-to object is not owned by this class.
 * The user must ensure that the aforementioned object outlives any access
 * through an object of this class.
 *
 * Access is provided through the templated value<T>() method which checks that
 * the referred-to object is indeed of type T. This is the main difference
 * between TypeErasedRef and const void* — the latter has no safety checks.
 *
 * TypeErasedRef is copyable. The copy points to the same object as the
 * original.
 */
class TypeErasedRef {
 public:
  const std::type_info& type() const noexcept { return *typeInfo_; }
  const void* ptr() const noexcept { return ptr_; }

  template <class T>
  bool holds_alternative() const noexcept {
    return type() == typeid(T);
  }

  template <class T>
  const folly::remove_cvref_t<T>& value() const {
    if (!holds_alternative<T>()) {
      throw std::bad_cast();
    }
    return value_unchecked<T>();
  }

  template <class T>
  const folly::remove_cvref_t<T>& value_unchecked() const noexcept {
    FOLLY_SAFE_DCHECK(
        holds_alternative<T>(),
        "Tried to call value_unchecked() on TypeErasedRef with incompatible type");
    return *reinterpret_cast<const folly::remove_cvref_t<T>*>(ptr_);
  }

  TypeErasedRef(const TypeErasedRef& other) noexcept = default;
  TypeErasedRef& operator=(const TypeErasedRef& other) noexcept = default;

  template <class T>
  static TypeErasedRef of(folly::remove_cvref_t<T>&&) = delete;

  template <class T>
  static TypeErasedRef of(const folly::remove_cvref_t<T>& object) noexcept {
    return fromTypeInfoUnchecked(
        static_cast<const void*>(std::addressof(object)), typeid(T));
  }

  static TypeErasedRef fromTypeInfoUnchecked(
      const void* ptr, const std::type_info& typeInfo) noexcept {
    return TypeErasedRef(ptr, typeInfo);
  }

 private:
  TypeErasedRef(const void* ptr, const std::type_info& typeInfo) noexcept
      : ptr_(ptr), typeInfo_(&typeInfo) {}

  const void* ptr_;
  const std::type_info* typeInfo_;
};

} // namespace apache::thrift::util
