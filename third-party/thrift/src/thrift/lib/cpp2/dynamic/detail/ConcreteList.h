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

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/List.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>

namespace apache::thrift::dynamic {
namespace detail {

// ConcreteList template method implementations
// These are in a separate header because they need Datum and DynamicValue
// to be complete types

// Helper to extract value from Datum, handling bool -> std::byte conversion
template <typename T>
storage_type_t<T> extractValue(detail::Datum&& datum) {
  using StorageType = storage_type_t<T>;
  if constexpr (std::is_same_v<StorageType, std::byte>) {
    // Storage is std::byte, means we're storing bools
    // Extract the bool from Datum and cast to std::byte
    return static_cast<std::byte>(std::move(datum).as<bool>());
  } else {
    return std::move(datum).as<T>();
  }
}

template <typename T>
T& ConcreteList<T>::getElement(size_t index) {
  using StorageType = storage_type_t<T>;
  if constexpr (std::is_same_v<StorageType, std::byte>) {
    // Storage is std::byte, means logical type is bool, return bool&
    return reinterpret_cast<bool&>(elements_[index]);
  } else {
    return elements_[index];
  }
}

template <typename T>
const T& ConcreteList<T>::getElement(size_t index) const {
  using StorageType = storage_type_t<T>;
  if constexpr (std::is_same_v<StorageType, std::byte>) {
    // Storage is std::byte, means logical type is bool, return const bool&
    return reinterpret_cast<const bool&>(elements_[index]);
  } else {
    return elements_[index];
  }
}

template <typename T>
DynamicRef ConcreteList<T>::operator[](size_t index) {
  if (index >= elements_.size()) {
    throw std::out_of_range("Index out of range in List::operator[]");
  }
  return DynamicRef(elementType_, getElement(index));
}

template <typename T>
DynamicConstRef ConcreteList<T>::operator[](size_t index) const {
  if (index >= elements_.size()) {
    throw std::out_of_range("Index out of range in List::operator[]");
  }
  return DynamicConstRef(elementType_, getElement(index));
}

template <typename T>
void ConcreteList<T>::set(size_t index, DynamicValue value) {
  if (index >= elements_.size()) {
    throw std::out_of_range("Index out of range in List::set");
  }
  expectType(elementType(), value.type());
  auto extractedValue = extractValue<T>(std::move(value).datum());
  elements_[index] = extractedValue;
}

template <typename T>
size_t ConcreteList<T>::size() const {
  return elements_.size();
}

template <typename T>
bool ConcreteList<T>::isEmpty() const {
  return elements_.empty();
}

template <typename T>
void ConcreteList<T>::push_back(DynamicValue value) {
  expectType(elementType(), value.type());
  elements_.push_back(extractValue<T>(std::move(value).datum()));
}

template <typename T>
void ConcreteList<T>::push_front(DynamicValue value) {
  expectType(elementType(), value.type());
  auto extractedValue = extractValue<T>(std::move(value).datum());
  elements_.insert(elements_.begin(), extractedValue);
}

template <typename T>
void ConcreteList<T>::insertAtIndex(size_t index, DynamicValue value) {
  if (index > elements_.size()) {
    throw std::out_of_range("Index out of range in List::insertAtIndex");
  }
  expectType(elementType(), value.type());
  auto extractedValue = extractValue<T>(std::move(value).datum());
  elements_.insert(elements_.begin() + index, extractedValue);
}

template <typename T>
void ConcreteList<T>::fill(size_t count, DynamicValue value) {
  expectType(elementType(), value.type());
  auto extractedValue = extractValue<T>(std::move(value).datum());
  elements_.assign(count, extractedValue);
}

template <typename T>
void ConcreteList<T>::free() {
  if (mr_) {
    std::pmr::polymorphic_allocator<>(mr_).delete_object(this);
  } else {
    delete this;
  }
}

template <typename T>
IList::Ptr ConcreteList<T>::clone() const {
  if (mr_) {
    return Ptr(
        std::pmr::polymorphic_allocator<>(mr_)
            .template new_object<ConcreteList>(*this));

  } else {
    return Ptr(new ConcreteList(*this));
  }
}

template <typename T>
void ConcreteList<T>::extend(const IList& other) {
  other.visit([this]<typename U>(const ConcreteList<U>& otherList) {
    if constexpr (std::is_same_v<T, U>) {
      elements_.insert(
          elements_.end(),
          otherList.elements().begin(),
          otherList.elements().end());
    } else {
      throw std::runtime_error("Type mismatch in List::extend");
    }
  });
}

template <typename T>
IList::Ptr ConcreteList<T>::slice(
    size_t startIdx, size_t endIdxExclusive) const {
  if (startIdx > elements_.size()) {
    throw std::out_of_range("Start index out of range in List::slice");
  }
  if (endIdxExclusive > elements_.size()) {
    throw std::out_of_range("End index out of range in List::slice");
  }
  if (startIdx > endIdxExclusive) {
    throw std::out_of_range("Start index must be <= end index in List::slice");
  }

  ConcreteList* ret;
  if (mr_) {
    ret = std::pmr::polymorphic_allocator<>(mr_)
              .template new_object<ConcreteList>(*this);

  } else {
    ret = new ConcreteList(*this);
  }
  ret->elements_.assign(
      elements_.begin() + startIdx, elements_.begin() + endIdxExclusive);
  return IList::Ptr(ret);
}

template <typename T>
void ConcreteList<T>::clear() {
  elements_.clear();
}

template <typename T>
void ConcreteList<T>::reserve(size_t capacity) {
  elements_.reserve(capacity);
}

template <typename T>
bool ConcreteList<T>::operator==(const IList& other) const {
  return other.visit(
      [this](const ConcreteList<T>& otherList) -> bool {
        return elements_.size() == otherList.elements().size() &&
            std::equal(
                   elements_.begin(),
                   elements_.end(),
                   otherList.elements().begin());
      },
      []<typename U>(const ConcreteList<U>&) -> bool { return false; });
}

} // namespace detail

namespace detail {

// Visit implementations
template <typename... F>
decltype(auto) IList::visit(F&&... visitors) const {
  auto visitor = folly::overload(std::forward<F>(visitors)...);
  return elementType().matchKind(
      [&]<type_system::TypeRef::Kind k>(type_system::TypeRef::KindConstant<k>) {
        return std::move(visitor)(
            static_cast<const ConcreteList<type_of_type_kind<k>>&>(*this));
      });
}

template <typename... F>
decltype(auto) IList::visit(F&&... visitors) {
  auto visitor = folly::overload(std::forward<F>(visitors)...);
  return elementType().matchKind(
      [&]<type_system::TypeRef::Kind k>(type_system::TypeRef::KindConstant<k>) {
        return std::move(visitor)(
            static_cast<ConcreteList<type_of_type_kind<k>>&>(*this));
      });
}

} // namespace detail
} // namespace apache::thrift::dynamic
