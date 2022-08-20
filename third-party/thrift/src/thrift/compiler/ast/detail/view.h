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

#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace apache {
namespace thrift {
namespace compiler {
namespace ast_detail {

template <typename Derived, typename T>
class base_view {
 public:
  using element_type = T;
  using value_type = std::remove_cv_t<T>;

  constexpr auto rbegin() const noexcept {
    return std::make_reverse_iterator(derived().end());
  }
  constexpr auto rend() const noexcept {
    return std::make_reverse_iterator(derived().begin());
  }
  constexpr bool empty() const noexcept { return derived().size() == 0; }

 private:
  Derived& derived() { return *static_cast<Derived*>(this); }
  const Derived& derived() const { return *static_cast<const Derived*>(this); }
};

template <typename Derived, typename T>
class base_span : public base_view<Derived, T> {
  using base = base_view<Derived, T>;

 public:
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using reference = T&;
  using iterator = pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_iterator = iterator;

  constexpr base_span(pointer data, std::size_t size) noexcept
      : data_(data), size_(size) {}

  constexpr iterator begin() const noexcept { return data_; }
  constexpr iterator end() const noexcept { return data_ + size_; }
  constexpr size_type size() const noexcept { return size_; }
  constexpr reference operator[](size_type pos) const {
    assert(pos < size_);
    return *(data_ + pos);
  }
  constexpr reference front() const { return operator[](0); }
  constexpr reference back() const {
    assert(size_ > 0);
    return operator[](size_ - 1);
  }

 private:
  pointer data_;
  size_type size_;
};

} // namespace ast_detail
} // namespace compiler
} // namespace thrift
} // namespace apache
