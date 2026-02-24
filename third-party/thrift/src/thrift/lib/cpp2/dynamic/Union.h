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

#include <folly/CPortability.h>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/fwd.h>

#include <fmt/core.h>

#include <memory_resource>
#include <string_view>

namespace apache::thrift::dynamic {

/**
 * Type-erased union interface. A union has at most one active field stored
 * inline, or may be empty (no active field).
 */
class Union final {
 public:
  using FieldId = ::apache::thrift::FieldId;

  /**
   * Get the active field by name.
   * The returned reference is invalidated when setField is called.
   * Throws:
   *   - std::out_of_range if the field doesn't exist in the union type
   *   - std::runtime_error if this field is not the active field
   */
  DynamicRef getField(std::string_view name);
  DynamicConstRef getField(std::string_view name) const;

  /**
   * Get the active field by ID.
   * The returned reference is invalidated when setField is called.
   * Throws:
   *   - std::out_of_range if the field doesn't exist in the union type
   *   - std::runtime_error if this field is not the active field
   */
  DynamicRef getField(FieldId id);
  DynamicConstRef getField(FieldId id) const;

  /**
   * Get the active field by handle.
   * The returned reference is invalidated when setField is called.
   * The handle must be valid for this union type.
   * Throws:
   *   - std::runtime_error if this field is not the active field
   */
  DynamicRef getField(type_system::FastFieldHandle handle);
  DynamicConstRef getField(type_system::FastFieldHandle handle) const;

  /**
   * Set field by name, making it the active field.
   * Invalidates references to other fields.
   * Throws:
   *   - std::out_of_range if the field doesn't exist
   *   - std::runtime_error if value type doesn't match field type
   */
  void setField(std::string_view name, DynamicValue value);

  /**
   * Set field by ID, making it the active field.
   * Invalidates references to other fields.
   * Throws:
   *   - std::out_of_range if the field doesn't exist
   *   - std::runtime_error if value type doesn't match field type
   */
  void setField(FieldId id, DynamicValue value);

  /**
   * Set field by handle, making it the active field.
   * Invalidates references to other fields.
   * The handle must be valid for this union type.
   * Throws:
   *   - std::runtime_error if value type doesn't match field type
   */
  void setField(type_system::FastFieldHandle handle, DynamicValue value);

  /**
   * Check if this field is the active field.
   */
  bool hasField(std::string_view name) const;
  bool hasField(FieldId id) const;

  /**
   * Check if this field is the active field by handle.
   * The handle must be valid for this union type.
   */
  bool hasField(type_system::FastFieldHandle handle) const;

  /**
   * Check if the union is empty (no active field).
   */
  bool isEmpty() const { return activeFieldDef_ == nullptr; }

  /**
   * Get the active field handle, if any.
   * Returns an invalid handle if the union is empty.
   */
  type_system::FastFieldHandle activeField() const;

  /**
   * Clear the union, making it empty.
   */
  void clear();

  /**
   * Returns the union type.
   */
  type_system::TypeRef type() const { return unionType_; }

  /**
   * Compares two unions for equality.
   */
  friend bool operator==(const Union& lhs, const Union& rhs) noexcept;

  /**
   * Constructor
   */
  explicit Union(
      type_system::UnionNode const& unionType,
      std::pmr::memory_resource* mr = nullptr);
  Union(const Union&);
  Union(Union&&) noexcept;
  Union& operator=(const Union&);
  Union& operator=(Union&&) noexcept;

  /**
   * Destructor - defined in .cpp to avoid requiring complete Datum type
   */
  ~Union();

 private:
  // Helper to create a Datum using the memory resource
  detail::Datum* makeDatumPtr(detail::Datum&& datum);
  void deleteDatumPtr(detail::Datum* ptr);

  type_system::TypeRef unionType_;
  // Memory resource for allocations
  std::pmr::memory_resource* mr_;
  // The active field's data, or nullptr if union is empty
  detail::Datum* activeFieldData_;
  // Pointer to the active field definition
  // nullptr indicates the union is empty (no active field)
  const type_system::FieldDefinition* activeFieldDef_;

  template <typename ProtocolWriter>
  friend void serialize(ProtocolWriter& writer, const Union& unionValue);
  template <typename ProtocolReader>
  friend Union deserialize(
      ProtocolReader& reader,
      const type_system::UnionNode& type,
      std::pmr::memory_resource* mr);
  template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>
  friend Union deserialize(
      ProtocolReader& reader,
      const type_system::UnionNode& type,
      std::pmr::memory_resource* mr,
      Callbacks& callbacks);
  friend Union fromRecord(
      const type_system::SerializableRecord& r,
      const type_system::UnionNode& unionType,
      std::pmr::memory_resource* mr);
};

/**
 * Factory function to create a new Union with the given type and allocator.
 */
Union makeUnion(
    type_system::UnionNode const& unionType,
    std::pmr::memory_resource* mr = nullptr);

// Forward declarations for SerDe (implementations in Serialization.h)
template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const Union& unionValue);

template <typename ProtocolReader>
Union deserialize(
    ProtocolReader& reader,
    const type_system::UnionNode& type,
    std::pmr::memory_resource* mr);

/**
 * Creates a Union from a SerializableRecord.
 */
Union fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::UnionNode& unionType,
    std::pmr::memory_resource* mr);

} // namespace apache::thrift::dynamic
