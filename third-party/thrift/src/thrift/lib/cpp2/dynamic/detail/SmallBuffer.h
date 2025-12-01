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
#include <type_traits>
#include <utility>

namespace apache::thrift::dynamic::detail {

// Helper type to store polymorphic member without allocating.
template <size_t InlineSize, size_t Alignment = alignof(void*)>
struct SmallBuffer {
  // Access
  template <typename T>
  T& as() {
    checkInvariants<T>();
    return *reinterpret_cast<T*>(inline_);
  }
  template <typename T>
  const T& as() const {
    checkInvariants<T>();
    return *reinterpret_cast<const T*>(inline_);
  }

  // Lifecycle
  template <typename T, typename... Args>
  T& emplace(Args&&... args) {
    checkInvariants<T>();
    return *new (&as<T>()) T(std::forward<Args>(args)...);
  }
  // Only supports trivially destructible types, so no destroy needed.

  // Only trivially copyable types are supported.
  SmallBuffer() = default;
  SmallBuffer(SmallBuffer const&) = default;
  SmallBuffer& operator=(SmallBuffer const&) = default;
  ~SmallBuffer() = default;

 private:
  alignas(Alignment) std::byte inline_[InlineSize];

  template <typename T>
  static constexpr void checkInvariants() {
    static_assert(sizeof(T) <= InlineSize);
    static_assert(alignof(T) <= Alignment);
    static_assert(std::is_trivially_destructible_v<T>);
    static_assert(std::is_trivially_copyable_v<T>);
  }
};

} // namespace apache::thrift::dynamic::detail
