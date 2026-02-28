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

namespace apache::thrift::dynamic {

namespace detail {
class ISet;
class ISetIterator;
struct FreeDeleter;
} // namespace detail

/**
 * Set value type that stores a unique_ptr to ISet implementation.
 * This allows Set to be stored inline in Datum while heap-allocating
 * the actual set contents. Empty sets don't allocate impl_.
 */
class Set final {
 public:
  explicit Set(
      type_system::TypeRef::Set setType,
      std::pmr::memory_resource* mr = nullptr);

  Set(const Set& other);
  Set(Set&&) noexcept;
  Set& operator=(const Set& other);
  Set& operator=(Set&&) noexcept;
  ~Set();

  /**
   * Insert an element into the set.
   * Returns true if the element was inserted, false if it already existed.
   * Throws:
   *   - std::runtime_error if value type doesn't match element type
   */
  bool insert(DynamicValue value);

  /**
   * Remove an element from the set.
   * Returns true if the element was removed, false if it didn't exist.
   */
  bool erase(const DynamicConstRef& value);

  /**
   * Check if an element exists in the set.
   */
  bool contains(const DynamicConstRef& value) const;

  /**
   * Returns the number of elements in the set.
   */
  size_t size() const;

  /**
   * Returns true if the set is empty.
   */
  bool isEmpty() const;

  /**
   * Removes all elements from the set.
   */
  void clear();

  /**
   * Reserves capacity for at least `capacity` elements.
   */
  void reserve(size_t capacity);

  type_system::TypeRef elementType() const;
  type_system::TypeRef type() const;

  // Iterator support
  class ConstIterator;

  ConstIterator begin() const;
  ConstIterator end() const;
  ConstIterator cbegin() const;
  ConstIterator cend() const;

  friend bool operator==(const Set& lhs, const Set& rhs);

  template <typename ProtocolWriter>
  friend void serialize(ProtocolWriter& writer, const Set& set);

  template <typename ProtocolReader>
  friend Set deserialize(
      ProtocolReader& reader,
      const type_system::TypeRef::Set& type,
      std::pmr::memory_resource* alloc);

  template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>
  friend Set deserialize(
      ProtocolReader& reader,
      const type_system::TypeRef::Set& type,
      std::pmr::memory_resource* alloc,
      Callbacks& callbacks);

 private:
  explicit Set(std::unique_ptr<detail::ISet, detail::FreeDeleter> impl);

  detail::ISet& ensureInit();

  type_system::TypeRef::Set setType_;
  std::pmr::memory_resource* mr_;
  std::unique_ptr<detail::ISet, detail::FreeDeleter> impl_;
};

/**
 * Const iterator for Set that dereferences to DynamicConstRef.
 *
 * IMPORTANT: All iterators are invalidated by insertion or removal of elements
 * from the container.
 */
class Set::ConstIterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = DynamicConstRef;
  using difference_type = std::ptrdiff_t;
  using pointer = void; // Not applicable for proxy iterators
  using reference = DynamicConstRef;

  ConstIterator();
  ConstIterator(const ConstIterator& other);
  ConstIterator(ConstIterator&&) noexcept;
  ConstIterator& operator=(const ConstIterator& other);
  ConstIterator& operator=(ConstIterator&&) noexcept;
  ~ConstIterator() = default;

  DynamicConstRef operator*() const;
  ConstIterator& operator++();
  ConstIterator operator++(int);

  bool operator==(const ConstIterator& other) const;

 private:
  // Template constructor for creating iterator with concrete iterator type.
  // Defined in Set.cpp.
  template <typename IterType>
  ConstIterator(IterType&& it, const type_system::TypeRef::Set* setType);

  detail::F14IteratorBuffer concreteIt_{};
  const type_system::TypeRef::Set* setType_;

  friend class Set;
};

/**
 * Factory function to create a new Set with the given set type and allocator.
 */
Set makeSet(
    type_system::TypeRef::Set setType, std::pmr::memory_resource* mr = nullptr);

/**
 * Creates a Set from a SerializableRecord.
 */
Set fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::TypeRef::Set& setType,
    std::pmr::memory_resource* mr);

} // namespace apache::thrift::dynamic
