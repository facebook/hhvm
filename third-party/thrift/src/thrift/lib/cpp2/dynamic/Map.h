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

namespace apache::thrift::dynamic {

namespace detail {
class IMap;
struct FreeDeleter;
} // namespace detail

/**
 * Map value type that stores a unique_ptr to IMap implementation.
 * This allows Map to be stored inline in Datum while heap-allocating
 * the actual map contents. Empty maps don't allocate impl_.
 */
class Map final {
 public:
  explicit Map(
      type_system::TypeRef::Map mapType,
      std::pmr::memory_resource* mr = nullptr);

  Map(const Map& other);
  Map(Map&&) noexcept;
  Map& operator=(const Map& other);
  Map& operator=(Map&&) noexcept;
  ~Map();

  /**
   * Get value for a key.
   * Returns empty optional if the key doesn't exist.
   */
  std::optional<DynamicRef> get(const DynamicConstRef& key);
  std::optional<DynamicConstRef> get(const DynamicConstRef& key) const;

  /**
   * Insert or update a key-value pair.
   * Throws:
   *   - std::runtime_error if key/value types don't match
   */
  void insert(DynamicValue key, DynamicValue value);

  /**
   * Remove a key-value pair.
   * Returns true if the key was removed, false if it didn't exist.
   */
  bool erase(const DynamicConstRef& key);

  /**
   * Check if a key exists in the map.
   */
  bool contains(const DynamicConstRef& key) const;

  /**
   * Returns the number of key-value pairs in the map.
   */
  size_t size() const;

  /**
   * Returns true if the map is empty.
   */
  bool isEmpty() const;

  /**
   * Removes all key-value pairs from the map.
   */
  void clear();

  /**
   * Reserves capacity for at least `capacity` key-value pairs.
   */
  void reserve(size_t capacity);

  type_system::TypeRef keyType() const;
  type_system::TypeRef valueType() const;
  type_system::TypeRef type() const;

  friend bool operator==(const Map& lhs, const Map& rhs);

  template <typename ProtocolWriter>
  friend void serialize(ProtocolWriter& writer, const Map& map);

  template <typename ProtocolReader>
  friend Map deserialize(
      ProtocolReader& reader,
      const type_system::TypeRef::Map& type,
      std::pmr::memory_resource* alloc);

 private:
  explicit Map(std::unique_ptr<detail::IMap, detail::FreeDeleter> impl);

  detail::IMap& ensureInit();

  type_system::TypeRef::Map mapType_;
  std::pmr::memory_resource* mr_;
  std::unique_ptr<detail::IMap, detail::FreeDeleter> impl_;
};

/**
 * Factory function to create a new Map with the given map type and allocator.
 */
Map makeMap(
    type_system::TypeRef::Map mapType, std::pmr::memory_resource* mr = nullptr);

/**
 * Creates a Map from a SerializableRecord.
 */
Map fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::TypeRef::Map& mapType,
    std::pmr::memory_resource* mr);

} // namespace apache::thrift::dynamic

// foo
