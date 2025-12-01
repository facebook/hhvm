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
#include <thrift/lib/cpp2/dynamic/detail/SmallBuffer.h>
#include <thrift/lib/cpp2/dynamic/fwd.h>

#include <cstddef>
#include <memory>
#include <memory_resource>
#include <optional>

namespace apache::thrift::dynamic {

namespace detail {
class IMap;
class IMapIterator;
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

  // Iterator support
  class Iterator;
  class ConstIterator;

  Iterator begin();
  Iterator end();
  ConstIterator begin() const;
  ConstIterator end() const;
  ConstIterator cbegin() const;
  ConstIterator cend() const;

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
 * Iterator for Map that dereferences to std::pair<DynamicConstRef, DynamicRef>.
 * Allows ranged for loops like: for (auto [key, value] : map)
 *
 * IMPORTANT: All iterators are invalidated by insertion or removal of elements
 * from the container.
 */
class Map::Iterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::pair<DynamicConstRef, DynamicRef>;
  using difference_type = std::ptrdiff_t;
  using pointer = void;
  using reference = value_type;

  Iterator();
  Iterator(const Iterator& other);
  Iterator(Iterator&&) noexcept;
  Iterator& operator=(const Iterator& other);
  Iterator& operator=(Iterator&&) noexcept;
  ~Iterator() = default;

  value_type operator*();
  DynamicConstRef key();
  DynamicRef value();

  Iterator& operator++();
  Iterator operator++(int);

  bool operator==(const Iterator& other) const;

 private:
  // Template constructor for creating iterator with concrete iterator type.
  // Defined in Map.cpp.
  template <typename IterType>
  Iterator(IterType&& it, const type_system::TypeRef::Map* mapType);

  detail::SmallBuffer<16> concreteIt_;
  const type_system::TypeRef::Map* mapType_;

  friend class Map;
};

/**
 * Const iterator for Map that dereferences to std::pair<DynamicConstRef,
 * DynamicConstRef>. Allows ranged for loops like: for (auto [key, value] : map)
 *
 * IMPORTANT: All iterators are invalidated by insertion or removal of elements
 * from the container.
 */
class Map::ConstIterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::pair<DynamicConstRef, DynamicConstRef>;
  using difference_type = std::ptrdiff_t;
  using pointer = void;
  using reference = value_type;

  ConstIterator();
  ConstIterator(const ConstIterator& other);
  ConstIterator(ConstIterator&&) noexcept;
  ConstIterator& operator=(const ConstIterator& other);
  ConstIterator& operator=(ConstIterator&&) noexcept;
  ~ConstIterator() = default;

  value_type operator*() const;
  DynamicConstRef key() const;
  DynamicConstRef value() const;

  ConstIterator& operator++();
  ConstIterator operator++(int);

  bool operator==(const ConstIterator& other) const;

  bool operator==(const Iterator& other) const;

 private:
  // Template constructor for creating iterator with concrete iterator type.
  // Defined in Map.cpp.
  template <typename IterType>
  ConstIterator(IterType&& it, const type_system::TypeRef::Map* mapType);

  detail::SmallBuffer<16> concreteIt_{};
  const type_system::TypeRef::Map* mapType_;

  friend class Map;
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
