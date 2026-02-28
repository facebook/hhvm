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

#include <compare>
#include <cstddef>
#include <iterator>

namespace apache::thrift::dynamic::detail {

/**
 * CRTP base class for List iterators, providing shared implementation
 * for both Iterator and ConstIterator.
 *
 * Template parameters:
 *   Derived - The derived iterator type (for CRTP)
 *   ListPtr - The list pointer type (List* or const List*)
 *   RefType - The reference type returned by operator* (DynamicRef or
 *             DynamicConstRef)
 */
template <typename Derived, typename ListPtr, typename RefType>
class ListIteratorBase {
 public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = RefType;
  using difference_type = std::ptrdiff_t;
  using pointer = void; // Not applicable for proxy iterators
  using reference = RefType;

  ListIteratorBase() : list_(nullptr), index_(0) {}
  ListIteratorBase(ListPtr list, size_t index) : list_(list), index_(index) {}

  RefType operator*() const { return (*list_)[index_]; }

  Derived& operator++() {
    ++index_;
    return static_cast<Derived&>(*this);
  }

  Derived operator++(int) {
    Derived tmp = static_cast<Derived&>(*this);
    ++index_;
    return tmp;
  }

  Derived& operator--() {
    --index_;
    return static_cast<Derived&>(*this);
  }

  Derived operator--(int) {
    Derived tmp = static_cast<Derived&>(*this);
    --index_;
    return tmp;
  }

  Derived& operator+=(difference_type n) {
    index_ += n;
    return static_cast<Derived&>(*this);
  }

  Derived& operator-=(difference_type n) {
    index_ -= n;
    return static_cast<Derived&>(*this);
  }

  Derived operator+(difference_type n) const {
    return Derived(list_, index_ + n);
  }

  Derived operator-(difference_type n) const {
    return Derived(list_, index_ - n);
  }

  difference_type operator-(const Derived& other) const {
    return index_ - other.index_;
  }

  RefType operator[](difference_type n) const { return (*list_)[index_ + n]; }

  bool operator==(const ListIteratorBase& other) const {
    return list_ == other.list_ && index_ == other.index_;
  }

  std::strong_ordering operator<=>(const ListIteratorBase& other) const {
    return index_ <=> other.index_;
  }

 protected:
  ListPtr list_;
  size_t index_;
};

} // namespace apache::thrift::dynamic::detail
