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

#include <folly/ConstexprMath.h>
#include <folly/lang/Bits.h>

namespace apache::thrift::type {
class Ref;
namespace detail {

// A pointer for a type that has sufficent alignment to store information
// in the lower bits.
template <
    typename T,
    size_t Bits = folly::constexpr_log2(alignof(T)),
    size_t MaxBits = folly::constexpr_log2(alignof(T))>
class AlignedPtr {
 public:
  static_assert(Bits > 0 && Bits <= MaxBits, "insufficent alignment");
  constexpr static std::uintptr_t kMask = ~std::uintptr_t{} << Bits;

  constexpr AlignedPtr() noexcept = default;
  /* implicit */ constexpr AlignedPtr(T* ptr, std::uintptr_t bits = {}) noexcept
      : ptr_((folly::bit_cast<std::uintptr_t>(ptr) & kMask) | (bits & ~kMask)) {
    assert((bits & kMask) == 0); // Programming error.
    // Never happens because of T's alignment.
    assert((folly::bit_cast<std::uintptr_t>(ptr) & ~kMask) == 0);
  }

  T* get() const noexcept { return reinterpret_cast<T*>(ptr_ & kMask); }

  std::uintptr_t getTag() const noexcept { return ptr_ & ~kMask; }

  void clear() noexcept { ptr_ = 0; }

  void clearTag() noexcept { ptr_ &= kMask; }

  void set(T* ptr, std::uintptr_t tagBits = {}) {
    assert(
        reinterpret_cast<std::uintptr_t>(ptr) ==
        (reinterpret_cast<std::uintptr_t>(ptr) & kMask));
    assert(tagBits == (tagBits & ~kMask));
    ptr_ = reinterpret_cast<std::uintptr_t>(ptr) | tagBits;
  }

  void setTag(std::uintptr_t tagBits) {
    assert(tagBits == (tagBits & ~kMask));
    ptr_ = (ptr_ & kMask) | tagBits;
  }

  template <size_t Bit>
  constexpr bool get() const noexcept {
    return ptr_ & bitMask<Bit>();
  }

  template <size_t Bit>
  constexpr void set() noexcept {
    ptr_ |= bitMask<Bit>();
  }

  template <size_t Bit>
  constexpr void clear() noexcept {
    ptr_ &= ~bitMask<Bit>();
  }

 private:
  std::uintptr_t ptr_ = {};

  template <size_t Bit>
  constexpr static auto bitMask() noexcept {
    static_assert(Bit < Bits, "out of range");
    return std::uintptr_t{1} << Bit;
  }
};

} // namespace detail
} // namespace apache::thrift::type
