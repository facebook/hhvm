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

#include <memory>
#include <stdexcept>
#include <vector>

namespace apache {
namespace thrift {
namespace compiler {

// A list of AST nodes.
template <typename T>
using node_list = std::vector<std::unique_ptr<T>>;

// A view of a node_list of nodes of type T.
//
// If T is const, only provides const access to the nodes in the node_list.
//
// Like std::ranges::view, std::span and std::string_view, this class provides
// access to memory it does not own and must not be accessed after the
// associated node_list is destroyed.
template <typename T>
class node_list_view {
  using node_list_type = node_list<std::remove_cv_t<T>>;

 public:
  using size_type = typename node_list_type::size_type;
  using difference_type = typename node_list_type::difference_type;

  using value_type = std::remove_const_t<T>;
  using pointer = T*;
  using reference = T&;

  class iterator {
    friend class node_list_view;
    using itr_type = typename node_list_type::const_iterator;
    /* implicit */ constexpr iterator(itr_type itr) : itr_(std::move(itr)) {}

   public:
    using iterator_category = typename itr_type::iterator_category;
    using difference_type = typename itr_type::difference_type;
    using value_type = node_list_view::value_type;
    using pointer = node_list_view::pointer;
    using reference = node_list_view::reference;

    reference operator*() const noexcept { return itr_->operator*(); }
    reference operator[](difference_type n) const noexcept { return *itr_[n]; }

    iterator operator++(int) noexcept { return {itr_++}; }
    iterator operator--(int) noexcept { return {itr_--}; }
    iterator& operator++() noexcept {
      ++itr_;
      return *this;
    }
    iterator& operator--() noexcept {
      --itr_;
      return *this;
    }
    iterator& operator+=(difference_type n) noexcept {
      itr_ += n;
      return *this;
    }
    iterator& operator-=(difference_type n) noexcept {
      itr_ -= n;
      return *this;
    }

    friend iterator operator-(
        const iterator& a, const difference_type& n) noexcept {
      return {a.itr_ - n};
    }
    friend iterator operator+(
        const iterator& a, const difference_type& n) noexcept {
      return {a.itr_ + n};
    }
    friend iterator operator+(
        const difference_type& n, const iterator& b) noexcept {
      return {n + b.itr_};
    }

   private:
    itr_type itr_;

#define __FBTHRIFT_NODE_SPAN_ITR_FWD_OP(op)                       \
  friend auto operator op(const iterator& a, const iterator& b) { \
    return a.itr_ op b.itr_;                                      \
  }
    __FBTHRIFT_NODE_SPAN_ITR_FWD_OP(-)
    __FBTHRIFT_NODE_SPAN_ITR_FWD_OP(==)
    __FBTHRIFT_NODE_SPAN_ITR_FWD_OP(!=)
    __FBTHRIFT_NODE_SPAN_ITR_FWD_OP(<)
    __FBTHRIFT_NODE_SPAN_ITR_FWD_OP(<=)
    __FBTHRIFT_NODE_SPAN_ITR_FWD_OP(>)
    __FBTHRIFT_NODE_SPAN_ITR_FWD_OP(>=)
#undef __FBTHRIFT_NODE_SPAN_ITR_FWD_OP
  };

  constexpr node_list_view() = default;
  /* implicit */ constexpr node_list_view(const node_list_type& list) noexcept
      : begin_(list.begin()), size_(list.size()) {}

  constexpr node_list_view(const node_list_view&) noexcept = default;
  constexpr node_list_view& operator=(const node_list_view&) noexcept = default;

  constexpr reference front() const { return *begin_; }
  constexpr reference back() const { return begin_[size_ - 1]; }
  constexpr reference operator[](std::size_t pos) const { return at(pos); }
  constexpr iterator begin() const noexcept { return begin_; }
  constexpr iterator end() const noexcept { return begin_ + size_; }
  constexpr std::size_t size() const noexcept { return size_; }
  constexpr bool empty() const noexcept { return size_ == 0; }

  constexpr auto rbegin() const noexcept {
    return std::make_reverse_iterator(end());
  }
  constexpr auto rend() const noexcept {
    return std::make_reverse_iterator(begin());
  }

  // Create an std::vector with the same contents as this span.
  constexpr std::vector<T*> copy() const noexcept {
    std::vector<T*> result;
    for (size_t i = 0; i < size_; ++i) {
      result.emplace_back(&begin_[i]);
    }
    return result;
  }

  // Provided for backwards compatibility with std::vector API.
  using const_iterator = iterator;
  using const_reference = reference;
  using const_pointer = pointer;
  constexpr iterator cbegin() const noexcept { return begin_; }
  constexpr iterator cend() const noexcept { return begin_ + size_; }
  constexpr reference at(std::size_t pos) const {
    if (pos >= size_) {
      throw std::out_of_range("index out of range");
    }
    return begin_[pos];
  }

 private:
  iterator begin_;
  size_t size_ = 0;
};

} // namespace compiler
} // namespace thrift
} // namespace apache
