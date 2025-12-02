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

#include <thrift/lib/cpp2/dynamic/fwd.h>
#include <thrift/lib/cpp2/type/Any.h>

#include <memory_resource>
#include <folly/lang/Exception.h>

namespace apache::thrift::dynamic {

/**
 * An Any type that wraps type::AnyData for the dynamic value system.
 * This allows storing type-erased values with full serialization information.
 */
class Any final {
 public:
  // Constructors
  Any() = default;
  explicit Any(std::pmr::memory_resource* mr) : mr_(mr) {}
  explicit Any(type::AnyData data, std::pmr::memory_resource* mr = nullptr)
      : data_(std::move(data)), mr_(mr) {
    // Invariant: if hasValue() is true, the value must be valid
    if (data_.hasValue() && !data_.isValid()) {
      folly::throw_exception<std::runtime_error>(
          "Cannot create Any with invalid data");
    }
  }

  // Copy and move
  Any(const Any&) = default;
  Any(Any&&) noexcept = default;
  Any& operator=(const Any&) = default;
  Any& operator=(Any&&) noexcept = default;
  ~Any() = default;

  // Check if the Any has a value (is non-empty)
  // Invariant: if hasValue() is true, the value is guaranteed to be valid
  bool hasValue() const noexcept { return data_.hasValue(); }

  // Get the type information of the stored value
  const type::Type& type() const { return data_.type(); }

  // Get the protocol used to serialize the stored value
  const type::Protocol& protocol() const { return data_.protocol(); }

  /**
   * Load (deserialize) the contained value into a DynamicValue.
   * The type information is extracted from the stored AnyData.
   *
   * Throws:
   *   - std::runtime_error if the Any is empty or cannot be deserialized
   *   - std::invalid_argument if the type contains user-defined types not in
   * the TypeSystem
   */
  DynamicValue load(
      const type_system::TypeSystem& typeSystem,
      std::pmr::memory_resource* mr = nullptr) const;

  /**
   * Load (deserialize) the contained value as a specific type.
   * Validates that the provided TypeRef matches the stored type.
   *
   * Throws:
   *   - std::runtime_error if the Any is empty, type doesn't match,
   *     or cannot be deserialized
   */
  DynamicValue load(
      type_system::TypeRef typeRef,
      std::pmr::memory_resource* mr = nullptr) const;

  /**
   * Store a DynamicValue into an Any.
   * This serializes the value with the specified protocol and captures the type
   * information.
   *
   * Throws:
   *   - std::runtime_error if serialization fails
   */
  static Any store(
      const DynamicValue& value,
      type::StandardProtocol protocol = type::StandardProtocol::Compact,
      std::pmr::memory_resource* mr = nullptr);

  // Comparison
  friend bool operator==(const Any& lhs, const Any& rhs) noexcept {
    return lhs.data_ == rhs.data_;
  }

 private:
  type::AnyData data_;
  std::pmr::memory_resource* mr_ = nullptr;

  template <typename ProtocolWriter>
  friend void serialize(ProtocolWriter& writer, const Any& any);
  template <typename ProtocolReader>
  friend Any deserialize(
      ProtocolReader& reader,
      type_system::TypeRef::Any,
      std::pmr::memory_resource* mr);
  friend struct detail::DatumHash;
  friend struct detail::DatumEqual;
};

} // namespace apache::thrift::dynamic
