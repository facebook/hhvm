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

#include <folly/Demangle.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/dynamic/detail/SmallBuffer.h>

#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace apache::thrift::fast_thrift::channel_pipeline {

// BytesPtr is the standard buffer type for pipeline messages
using BytesPtr = std::unique_ptr<folly::IOBuf>;

#ifndef NDEBUG
/**
 * Exception thrown in debug mode when TypeErasedBox::get() or take() is called
 * with the wrong type. Provides detailed error message with both the requested
 * type and the actual type stored in the box.
 *
 * This exception is ONLY thrown in debug builds. In release builds, type
 * mismatches result in undefined behavior (for zero-overhead).
 */
class TypeErasedBoxTypeMismatch : public std::runtime_error {
 public:
  TypeErasedBoxTypeMismatch(
      const char* operation,
      const std::type_info& requestedType,
      const std::type_info* actualType)
      : std::runtime_error(buildMessage(operation, requestedType, actualType)),
        requestedType_(requestedType),
        actualType_(actualType) {}

  const std::type_info& requestedType() const noexcept {
    return requestedType_;
  }

  const std::type_info* actualType() const noexcept { return actualType_; }

 private:
  static std::string buildMessage(
      const char* operation,
      const std::type_info& requestedType,
      const std::type_info* actualType) {
    std::string msg = "TypeErasedBox::";
    msg += operation;
    msg += "() type mismatch: requested type '";
    msg += folly::demangle(requestedType.name()).toStdString();
    msg += "' but box contains '";
    if (actualType) {
      msg += folly::demangle(actualType->name()).toStdString();
    } else {
      msg += "<empty>";
    }
    msg += "'";
    return msg;
  }

  const std::type_info& requestedType_;
  const std::type_info* actualType_;
};

/**
 * Exception thrown in debug mode when TypeErasedBox::get() or take() is called
 * on an empty box.
 */
class TypeErasedBoxEmptyAccess : public std::runtime_error {
 public:
  explicit TypeErasedBoxEmptyAccess(const char* operation)
      : std::runtime_error(buildMessage(operation)) {}

 private:
  static std::string buildMessage(const char* operation) {
    std::string msg = "TypeErasedBox::";
    msg += operation;
    msg += "() called on empty box";
    return msg;
  }
};
#endif // NDEBUG

/**
 * TypeErasedBox - Zero-cost wrapper over SmallBuffer for pipeline messages.
 *
 * This class provides:
 * - Compile-time size enforcement via static_assert (won't compile if too big)
 * - Move-only semantics (deleted copy operations)
 * - Debug type checking with descriptive exceptions (zero cost in release)
 * - Zero runtime overhead in release builds (sizeof == sizeof(SmallBuffer))
 *
 * Storage: 120 bytes inline capacity in a 128-byte struct (93.75% efficiency)
 * This fits: BytesPtr (8B), ParsedFrame (~40B), frame wrappers with metadata
 *
 * Unlike std::any which does runtime type checking, TypeErasedBox trusts
 * that handlers know what type they're receiving. Debug builds throw
 * TypeErasedBoxTypeMismatch on type mismatch.
 *
 * Performance:
 * - Construction: ~1-2ns (placement new)
 * - get<T>(): Zero overhead in release (just reinterpret_cast)
 * - take<T>(): Zero overhead in release (move + reset)
 *
 * Debug Mode Behavior:
 * - get<T>() throws TypeErasedBoxEmptyAccess if box is empty
 * - get<T>() throws TypeErasedBoxTypeMismatch if T doesn't match stored type
 * - take<T>() throws TypeErasedBoxEmptyAccess if box is empty
 * - take<T>() throws TypeErasedBoxTypeMismatch if T doesn't match stored type
 */
class TypeErasedBox : private apache::thrift::dynamic::detail::
                          SmallBuffer<120, alignof(void*), true, true> {
  using Base = apache::thrift::dynamic::detail::
      SmallBuffer<120, alignof(void*), true, true>;

 public:
  // 120 bytes usable inline storage
  // Total struct size = 128 bytes = two L1 cache lines
  static constexpr size_t kInlineCapacity = 120;
  static constexpr size_t kInlineAlign = alignof(void*);

  /**
   * Check if type T fits in inline storage.
   * Types that don't fit will cause a compile-time error.
   */
  template <typename T>
  static constexpr bool fits_inline() {
    return sizeof(T) <= kInlineCapacity && alignof(T) <= kInlineAlign &&
        std::is_nothrow_move_constructible_v<T>;
  }

  TypeErasedBox() = default;

  template <typename T>
  explicit TypeErasedBox(T&& value) {
    using DecayedT = std::decay_t<T>;

    static_assert(
        fits_inline<DecayedT>(),
        "\n\n"
        "============================================================\n"
        "TypeErasedBox: Type exceeds 120-byte inline storage capacity.\n"
        "============================================================\n"
        "\n"
        "All pipeline messages must fit within 120 bytes for zero-allocation\n"
        "message passing. To fix this, you must either:\n"
        "\n"
        "  1. Reduce your type's size to 120 bytes or less, OR\n"
        "\n"
        "  2. Use explicit heap allocation by wrapping in a unique_ptr:\n"
        "     erase_and_box(std::make_unique<YourLargeType>(...))\n"
        "\n"
        "This compile-time enforcement ensures predictable performance\n"
        "and cache-friendly message passing in the pipeline.\n"
        "\n");

    Base::emplace<DecayedT>(std::forward<T>(value));

#ifndef NDEBUG
    type_ = &typeid(DecayedT);
#endif
  }

  ~TypeErasedBox() = default; // SmallBuffer handles destruction

  // Move constructor - delegates to SmallBuffer
  TypeErasedBox(TypeErasedBox&& other) noexcept
      : Base(std::move(other))
#ifndef NDEBUG
        ,
        // NOLINTNEXTLINE(bugprone-use-after-move)
        type_(std::exchange(other.type_, nullptr))
#endif
  {
  }

  // Move assignment - delegates to SmallBuffer
  TypeErasedBox& operator=(TypeErasedBox&& other) noexcept {
    if (this != &other) {
#ifndef NDEBUG
      type_ = std::exchange(other.type_, nullptr);
#endif
      Base::operator=(std::move(other));
    }
    return *this;
  }

  // Non-copyable (SmallBuffer<..., MoveOnly=true> is also non-copyable)
  TypeErasedBox(const TypeErasedBox&) = delete;
  TypeErasedBox& operator=(const TypeErasedBox&) = delete;

  /**
   * Get reference to stored value.
   *
   * In release builds: Zero overhead, undefined behavior if type is wrong.
   * In debug builds: Throws TypeErasedBoxEmptyAccess if box is empty.
   *                  Throws TypeErasedBoxTypeMismatch if T doesn't match.
   */
  template <typename T>
#ifdef NDEBUG
  T& get() noexcept {
    return Base::as<T>();
  }
#else
  T& get() {
    checkAccess<T>("get");
    return Base::as<T>();
  }
#endif

  template <typename T>
#ifdef NDEBUG
  const T& get() const noexcept {
    return Base::as<T>();
  }
#else
  const T& get() const {
    checkAccess<T>("get");
    return Base::as<T>();
  }
#endif

  /**
   * Extract and take ownership of stored value. Box becomes empty.
   *
   * In release builds: Zero overhead, undefined behavior if type is wrong.
   * In debug builds: Throws TypeErasedBoxEmptyAccess if box is empty.
   *                  Throws TypeErasedBoxTypeMismatch if T doesn't match.
   */
  template <typename T>
#ifdef NDEBUG
  T take() noexcept {
    T result = std::move(Base::as<T>());
    Base::reset();
    return result;
  }
#else
  T take() {
    checkAccess<T>("take");
    type_ = nullptr;
    T result = std::move(Base::as<T>());
    Base::reset();
    return result;
  }
#endif

  /**
   * Reset box to empty state, destroying any held value.
   */
  void reset() noexcept {
    Base::reset();
#ifndef NDEBUG
    type_ = nullptr;
#endif
  }

  // Expose empty() from base - zero cost
  using Base::empty;

  explicit operator bool() const noexcept { return !empty(); }

#ifndef NDEBUG
  /**
   * Get human-readable name of stored type (debug only).
   * Returns nullptr if box is empty.
   */
  std::string typeName() const {
    if (type_) {
      return folly::demangle(type_->name()).toStdString();
    }
    return "<empty>";
  }

  /**
   * Get the type_info of the stored type (debug only).
   * Returns nullptr if box is empty.
   */
  const std::type_info* storedType() const noexcept { return type_; }
#endif

 private:
#ifndef NDEBUG
  std::type_info const* type_{nullptr};

  template <typename T>
  void checkAccess(const char* operation) const {
    if (Base::empty()) {
      throw TypeErasedBoxEmptyAccess(operation);
    }
    if (type_ == nullptr || *type_ != typeid(T)) {
      throw TypeErasedBoxTypeMismatch(operation, typeid(T), type_);
    }
  }
#endif
};

// Verify zero-cost in release builds
// Release: 128 bytes (same as SmallBuffer<120, 8, true>)
// Debug: 136 bytes (+8 for type_ pointer only)
static_assert(
    sizeof(TypeErasedBox) == 128 || sizeof(TypeErasedBox) == 136,
    "TypeErasedBox size unexpected - should be 128 bytes (release) or 136 bytes (debug)");

/**
 * Helper function to create a TypeErasedBox from any value.
 */
template <typename T>
TypeErasedBox erase_and_box(T&& t) {
  return TypeErasedBox(std::forward<T>(t));
}

} // namespace apache::thrift::fast_thrift::channel_pipeline
