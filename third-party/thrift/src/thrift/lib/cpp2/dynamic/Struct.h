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

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/fwd.h>

#include <cstddef>
#include <memory>
#include <memory_resource>
#include <optional>
#include <string_view>

namespace apache::thrift::dynamic {

/**
 * Type-erased struct interface using pimpl idiom.
 * This is used to represent structs of any type.
 */
class Struct final {
 public:
  using FieldId = ::apache::thrift::FieldId;

  /**
   * Get field by name.
   * Returns empty optional if the field is an unset optional field.
   * Throws:
   *   - std::out_of_range if the field doesn't exist
   */
  std::optional<DynamicRef> getField(std::string_view name);
  std::optional<DynamicConstRef> getField(std::string_view name) const;

  /**
   * Get field by ID.
   * Returns empty optional if the field is an unset optional field.
   * Throws:
   *   - std::out_of_range if the field doesn't exist
   */
  std::optional<DynamicRef> getField(FieldId id);
  std::optional<DynamicConstRef> getField(FieldId id) const;

  /**
   * Get field by handle.
   * Returns empty optional if the field is an unset optional field.
   * The handle must be valid for this struct type.
   */
  std::optional<DynamicRef> getField(type_system::FastFieldHandle handle);
  std::optional<DynamicConstRef> getField(
      type_system::FastFieldHandle handle) const;

  /**
   * Set field by name.
   * Throws:
   *   - std::out_of_range if the field doesn't exist
   *   - std::runtime_error if value type doesn't match field type
   */
  void setField(std::string_view name, DynamicValue value);

  /**
   * Set field by ID.
   * Throws:
   *   - std::out_of_range if the field doesn't exist
   *   - std::runtime_error if value type doesn't match field type
   */
  void setField(FieldId id, DynamicValue value);

  /**
   * Set field by handle.
   * The handle must be valid for this struct type.
   */
  void setField(type_system::FastFieldHandle handle, DynamicValue value);

  /**
   * Check if a field is set (non-null).
   */
  bool hasField(std::string_view name) const;
  bool hasField(FieldId id) const;

  /**
   * Check if a field is set (non-null) by handle.
   * The handle must be valid for this struct type.
   */
  bool hasField(type_system::FastFieldHandle handle) const;

  /**
   * Clear an optional field by name.
   * Throws:
   *   - std::out_of_range if the field doesn't exist
   *   - std::runtime_error if the field is not optional
   */
  void clearOptionalField(std::string_view name);

  /**
   * Clear an optional field by ID.
   * Throws:
   *   - std::out_of_range if the field doesn't exist
   *   - std::runtime_error if the field is not optional
   */
  void clearOptionalField(FieldId id);

  /**
   * Clear an optional field by handle.
   * The handle must be valid for this struct type.
   * Throws:
   *   - std::runtime_error if the field is not optional
   */
  void clearOptionalField(type_system::FastFieldHandle handle);

  /**
   * Returns the struct type.
   */
  type_system::TypeRef type() const;

  /**
   * Compares two structs for equality.
   */
  friend bool operator==(const Struct& lhs, const Struct& rhs) noexcept;

  /**
   * Constructor
   */
  explicit Struct(
      const type_system::StructNode& structType,
      std::pmr::memory_resource* mr = nullptr);
  Struct(const Struct&);
  Struct(Struct&&) noexcept;
  Struct& operator=(const Struct&);
  Struct& operator=(Struct&&) noexcept;

  /**
   * Destructor - defined in .cpp to avoid requiring complete IStruct type
   */
  ~Struct();

 private:
  using IStructPtr = std::unique_ptr<detail::IStruct, detail::FreeDeleter>;
  // Pimpl pointer to the implementation with custom deleter
  IStructPtr impl_;

  // Constructor for internal use (takes ownership of impl)
  explicit Struct(IStructPtr impl);

  template <typename ProtocolWriter>
  friend void serialize(ProtocolWriter& writer, const Struct& structValue);
  template <typename ProtocolReader>
  friend Struct deserialize(
      ProtocolReader& reader,
      const type_system::StructNode& type,
      std::pmr::memory_resource* mr);
  friend Struct fromRecord(
      const type_system::SerializableRecord& r,
      const type_system::StructNode& structType,
      std::pmr::memory_resource* mr);
};

/**
 * Factory function to create a new Struct with the given type and allocator.
 */
Struct makeStruct(
    const type_system::StructNode& structType,
    std::pmr::memory_resource* mr = nullptr);

// Forward declarations for SerDe (implementations in Serialization.h)
template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const Struct& structValue);

template <typename ProtocolReader>
Struct deserialize(
    ProtocolReader& reader,
    const type_system::StructNode& type,
    std::pmr::memory_resource* mr);

/**
 * Creates a Struct from a SerializableRecord.
 */
Struct fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::StructNode& structType,
    std::pmr::memory_resource* mr);

} // namespace apache::thrift::dynamic
