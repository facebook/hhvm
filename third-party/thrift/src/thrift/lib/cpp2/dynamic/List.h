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
#include <folly/Overload.h>
#include <folly/Traits.h>
#include <folly/lang/Assume.h>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/detail/ListIteratorBase.h>
#include <thrift/lib/cpp2/dynamic/fwd.h>

#include <fmt/core.h>

#include <stddef.h>
#include <compare>
#include <cstddef>
#include <memory>
#include <memory_resource>
#include <utility>

namespace apache::thrift::dynamic {

// Forward declaration of IList in detail namespace
namespace detail {
class IList;
struct FreeDeleter;
} // namespace detail

/**
 * List value type that stores a unique_ptr to IList implementation.
 * This allows List to be stored inline in Datum while heap-allocating
 * the actual list contents. Empty lists don't allocate impl_.
 */
class List final {
 public:
  explicit List(
      type_system::TypeRef::List listType,
      std::pmr::memory_resource* mr = nullptr)
      : listType_(listType), mr_(mr), impl_(nullptr) {}

  List(const List& other);
  List(List&&) noexcept = default;
  List& operator=(const List& other);
  List& operator=(List&&) noexcept = default;
  ~List();

  /**
   * Get element at index.
   * Throws:
   *   - std::out_of_range if index >= size()
   */
  DynamicRef at(size_t index);
  DynamicConstRef at(size_t index) const;
  DynamicRef operator[](size_t index);
  DynamicConstRef operator[](size_t index) const;

  /**
   * Set element at index.
   * Throws:
   *   - std::out_of_range if index >= size()
   *   - std::runtime_error if value type doesn't match element type
   */
  void set(size_t index, DynamicValue value);

  /**
   * Returns the number of elements in the list.
   */
  size_t size() const;

  /**
   * Returns true if the list is empty.
   */
  bool isEmpty() const;

  /**
   * Appends an element to the end of the list.
   * Throws:
   *   - std::runtime_error if value type doesn't match element type
   */
  void push_back(DynamicValue value);

  /**
   * Prepends an element to the beginning of the list.
   * Throws:
   *   - std::runtime_error if value type doesn't match element type
   */
  void push_front(DynamicValue value);

  /**
   * Inserts an element at the specified index.
   * Throws:
   *   - std::out_of_range if index > size()
   *   - std::runtime_error if value type doesn't match element type
   */
  void insertAtIndex(size_t index, DynamicValue value);

  /**
   * Discards any existing elements and fills list with `count` copies of
   * `value`.
   * Throws:
   *   - std::runtime_error if value type doesn't match element type
   */
  void fill(size_t count, DynamicValue value);

  /**
   * Extends this list with elements from another list.
   * Throws:
   *   - std::runtime_error if element types don't match
   */
  void extend(const List& other);

  /**
   * Creates a new list containing elements from [startIdx, endIdxExclusive).
   * Throws:
   *   - std::out_of_range if indices are invalid
   */
  List slice(size_t startIdx, size_t endIdxExclusive) const;

  /**
   * Removes all elements from the list.
   */
  void clear();

  /**
   * Reserves capacity for at least `capacity` elements.
   */
  void reserve(size_t capacity);

  type_system::TypeRef elementType() const;
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

  friend bool operator==(const List& lhs, const List& rhs);

  template <typename ProtocolWriter>
  friend void serialize(ProtocolWriter& writer, const List& list);

  template <typename ProtocolReader>
  friend List deserialize(
      ProtocolReader& reader,
      const type_system::TypeRef::List& type,
      std::pmr::memory_resource* alloc);

  template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>
  friend List deserialize(
      ProtocolReader& reader,
      const type_system::TypeRef::List& type,
      std::pmr::memory_resource* alloc,
      Callbacks& callbacks);

 private:
  explicit List(std::unique_ptr<detail::IList, detail::FreeDeleter> impl);

  detail::IList& ensureInit();

  type_system::TypeRef::List listType_;
  std::pmr::memory_resource* mr_;
  std::unique_ptr<detail::IList, detail::FreeDeleter> impl_;
};

/**
 * Iterator for List that dereferences to DynamicRef.
 *
 * IMPORTANT: All iterators are invalidated by insertion or removal of elements
 * from the container.
 */
class List::Iterator
    : public detail::ListIteratorBase<Iterator, List*, DynamicRef> {
 public:
  using Base = detail::ListIteratorBase<Iterator, List*, DynamicRef>;
  using Base::Base;

 private:
  friend class ConstIterator;
};

/**
 * Const iterator for List that dereferences to DynamicConstRef.
 *
 * IMPORTANT: All iterators are invalidated by insertion or removal of elements
 * from the container.
 */
class List::ConstIterator
    : public detail::
          ListIteratorBase<ConstIterator, const List*, DynamicConstRef> {
 public:
  using Base =
      detail::ListIteratorBase<ConstIterator, const List*, DynamicConstRef>;
  using Base::Base;
};

/**
 * Factory function to create a new List with the given list type and allocator.
 */
List makeList(
    type_system::TypeRef::List listType,
    std::pmr::memory_resource* mr = nullptr);

/**
 * Creates a List from a SerializableRecord.
 */
List fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::TypeRef::List& listType,
    std::pmr::memory_resource* mr);

namespace detail {

// Storage type trait: Maps bool to std::byte to avoid vector<bool>
// specialization
template <typename T>
struct storage_type {
  using type = T;
};

template <>
struct storage_type<bool> {
  using type = std::byte;
};

template <typename T>
using storage_type_t = typename storage_type<T>::type;

/**
 * Type-erased list interface. This is used to represent lists of any type.
 */
class IList {
 public:
  using Ptr = std::unique_ptr<IList, FreeDeleter>;
  explicit IList(type_system::TypeRef::List listType) : listType_(listType) {}

  virtual ~IList() = default;

  /**
   * Free this list implementation. This must be called instead of delete
   * because implementations may use custom allocation strategies.
   */
  virtual void free() = 0;

  virtual DynamicRef operator[](size_t index) = 0;
  virtual DynamicConstRef operator[](size_t index) const = 0;
  virtual void set(size_t index, DynamicValue value) = 0;
  virtual size_t size() const = 0;
  virtual bool isEmpty() const = 0;
  virtual void push_back(DynamicValue value) = 0;
  virtual void push_front(DynamicValue value) = 0;
  virtual void insertAtIndex(size_t index, DynamicValue value) = 0;
  virtual void fill(size_t count, DynamicValue value) = 0;
  virtual void extend(const IList& other) = 0;
  virtual Ptr slice(size_t startIdx, size_t endIdxExclusive) const = 0;
  virtual void clear() = 0;
  virtual void reserve(size_t capacity) = 0;
  virtual Ptr clone() const = 0;

  template <typename... F>
  decltype(auto) visit(F&&... visitors) const;

  template <typename... F>
  decltype(auto) visit(F&&... visitors);

  type_system::TypeRef elementType() const {
    return listType_.asListUnchecked().elementType();
  }

  type_system::TypeRef type() const { return listType_; }

  virtual bool operator==(const IList& other) const = 0;

 protected:
  IList(const IList&) = default;
  IList& operator=(const IList&) = default;
  IList(IList&&) = default;
  IList& operator=(IList&&) = default;

  type_system::TypeRef listType_;
};

/**
 * Typed list implementation.
 * To prevent unbounded specialization, nested containers set T to the
 * type-erased interface (e.g. `ConcreteList<List>`).
 *
 * Storage uses storage_type_t<T> to avoid std::vector<bool> specialization
 * which returns proxy references that cannot bind to lvalue references.
 */
template <typename T>
class ConcreteList final : public IList {
 public:
  using Storage = std::pmr::vector<storage_type_t<T>>;

  explicit ConcreteList(
      type_system::TypeRef::List listType,
      std::pmr::memory_resource* mr = nullptr)
      : IList(std::move(listType)),
        elementType_(this->listType_.asListUnchecked().elementType()),
        mr_(mr),
        elements_(mr_ ? mr_ : std::pmr::get_default_resource()) {}
  ConcreteList(const ConcreteList&) = default;
  ConcreteList(ConcreteList&&) = default;
  ConcreteList& operator=(const ConcreteList&) = default;
  ConcreteList& operator=(ConcreteList&&) = default;
  ~ConcreteList() override = default;

  /**
   * Direct access to contiguous range of elements.
   */
  Storage& elements() { return elements_; }
  const Storage& elements() const { return elements_; }

  // IList interface implementation
  void free() override;
  DynamicRef operator[](size_t index) override;
  DynamicConstRef operator[](size_t index) const override;
  void set(size_t index, DynamicValue value) override;
  size_t size() const override;
  bool isEmpty() const override;
  void push_back(DynamicValue value) override;
  void push_front(DynamicValue value) override;
  void insertAtIndex(size_t index, DynamicValue value) override;
  void fill(size_t count, DynamicValue value) override;
  void extend(const IList& other) override;
  Ptr slice(size_t startIdx, size_t endIdxExclusive) const override;
  void clear() override;
  void reserve(size_t capacity) override;
  bool operator==(const IList& other) const override;

  Ptr clone() const override;

 private:
  /**
   * Helper to get element reference, handling bool -> std::byte conversion.
   * Returns bool& when storage is std::byte, otherwise returns T&.
   */
  T& getElement(size_t index);
  const T& getElement(size_t index) const;

  type_system::TypeRef elementType_;
  std::pmr::memory_resource* mr_;
  Storage elements_;
};

} // namespace detail

} // namespace apache::thrift::dynamic
