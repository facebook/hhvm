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
#include <fmt/core.h>
#include <folly/ConstexprMath.h>
#include <folly/lang/Bits.h>
#include <folly/lang/Exception.h>

namespace apache::thrift::type {
class Ref;
}

namespace apache::thrift::type::detail {

/**
 * A pointer for a type that has sufficent alignment to store information in the
 * lower bits.
 *
 * Template Parameters:
 * 1. `T`: Underlying type, whose pointer will be held (i.e., effectively a
 *    `T*`).
 * 2. `TagBits`: Number of bits to reserve at the lower end of the given `T*`
 *    for storing application-specific "tags". Must be a (strictly) positive
 *    integral value that is at most `MaxTagBits` (see below). This defaults to
 *    the number of (least significant) bits that are guaranteed to be unused in
 *    any `T` address, due to its natural alignment (i.e., `alignof(T)`). For
 *    example, if `alignof(T) == 8` (bytes), `TagBits` will default to `3`
 *    (bits).
 * 3. `MaxTagBits`: Maximum number of bits to reserve for tagging (i.e.,
 *    `TagBits`). This defaults to the natural alignment of `T` (`alignof(T)`),
 *     and technically is only safe if it is at most `alignof(T)`. In practice
 *     most allocators and compilers provide a stricter (i.e., larger) alignment
 *     such as 8 or 16 bytes (for x86-32 and x86-64, respectively), providing at
 *     least 3 or 4 "tag bits" regardless of the natural alignment of `T`. This
 *     parameter allows clients to explicitly relax the upper bound on tag bits
 *     that would be provided by `alignof(T)`, potentially at the risk of
 *     abnormal program termination (see validation in code below).
 *     `MaxTagBits` must be a (non-negative) integral value.
 */
template <
    typename T,
    size_t TagBits = folly::constexpr_log2(alignof(T)),
    size_t MaxTagBits = folly::constexpr_log2(alignof(T))>
class AlignedPtr {
 public:
  static_assert(MaxTagBits >= 0, "Invalid MaxTagBits: cannot be negative.");
  static_assert(TagBits > 0, "Invalid TagBits: must be strictly positive.");
  static_assert(
      TagBits <= MaxTagBits,
      "Invalid TagBits: cannot be greater than MaxTagBits.");

  /**
   * A pointer mask whose `TagBits` least-significant bits are cleared (i.e., 0)
   * and the remaining bits are set (i.e., 1).
   *
   * This corresponds to the bits (in the stored ptr value) that correspond to
   * the actual T* (i.e., excluding any "tags").
   *
   * eg., for TagBits == 4: `kPointerMask == 0xFF...F0`
   */
  constexpr static std::uintptr_t kPointerMask = ~std::uintptr_t{} << TagBits;

  constexpr AlignedPtr() noexcept = default;

  /* implicit */ constexpr AlignedPtr(
      T* ptr, std::uintptr_t tagBits = {}) noexcept
      : ptr_(
            (folly::bit_cast<std::uintptr_t>(ptr) & kPointerMask) |
            (tagBits & ~kPointerMask)) {
    maybeCheckPtrBits(ptr);
    checkTagBits(tagBits);
  }

  T* get() const noexcept { return reinterpret_cast<T*>(ptr_ & kPointerMask); }

  std::uintptr_t getTag() const noexcept { return ptr_ & ~kPointerMask; }

  void clear() noexcept { ptr_ = 0; }

  void clearTag() noexcept { ptr_ &= kPointerMask; }

  void set(T* ptr, std::uintptr_t tagBits = {}) noexcept {
    maybeCheckPtrBits(ptr);
    checkTagBits(tagBits);
    ptr_ = reinterpret_cast<std::uintptr_t>(ptr) | tagBits;
  }

  void setTag(std::uintptr_t tagBits) noexcept {
    checkTagBits(tagBits);
    ptr_ = (ptr_ & kPointerMask) | tagBits;
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
  constexpr static std::uintptr_t bitMask() noexcept {
    static_assert(Bit < TagBits, "out of range");
    return std::uintptr_t{1} << Bit;
  }

  /**
   * Aborts the process if the given `tagBits` do not fit in the reserved bits
   * for this `AlignedPtr`.
   */
  static void checkTagBits(std::uintptr_t tagBits) noexcept {
    if ((tagBits & kPointerMask) != 0) {
      folly::terminate_with<std::invalid_argument>(fmt::format(
          "Cannot initialize AlignPtr: tagBits exceeds ({}) TagBits: {}",
          TagBits,
          tagBits));
    }
  }

  /**
   * Aborts the process if the given `ptr` would conflict with the reserved tag
   * bits for this `AlignedPtr` (which could cause severe memory corruption).
   *
   * NOTE: this method is a no-op if the `TagBits` template argument for this
   * `AlignedPtr` does not require more bits that would be provided by the
   * natural alignment of type `T`. Indeed, in that case, the C++ standard
   * guarantees that any valid `ptr` will not conflict.
   */
  static void maybeCheckPtrBits(T* ptr) noexcept {
    if constexpr (TagBits > folly::constexpr_log2(alignof(T))) {
      // The number of bits allocated for tagging exceeds the number of bits
      // available from the natural alignment of type `T`. Consequently, the
      // C++ standard does not guarantee that the `TagBits` lower bits of `ptr`
      // are clear.
      // While technically undefined behavior, in practice this may be OK (see
      // `MaxTagBits` documentation above).
      // This branch runs additional tests to ensure the given `ptr` does not
      // use any of the reserved tag bits.

      if ((folly::bit_cast<std::uintptr_t>(ptr) & ~kPointerMask) != 0) {
        folly::terminate_with<std::invalid_argument>(fmt::format(
            "Cannot initialize AlignPtr: ptr uses lower ({}) tag bits: {}",
            TagBits,
            reinterpret_cast<const void*>(ptr)));
      }
    }
  }
};

} // namespace apache::thrift::type::detail
