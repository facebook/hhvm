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

#include <iterator>
#include <memory>

#include <folly/CPortability.h>
#include <folly/portability/SysTypes.h>

/**
 * Similar to Python's enumerate(), folly::enumerate() can be used to
 * iterate a range with a for-range loop, and it also allows to
 * retrieve the count of iterations so far.
 *
 * For example:
 *
 * for (const auto& [index, element] : folly::enumerate(vec)) {
 *   // index is a reference to a size_t
 *   // element is a reference to the type contained within vec
 * }
 *
 * It can also be used as follows:
 *
 * for (auto&& it : folly::enumerate(vec)) {
 *   // *it is a reference to the current element. Const if vec is const.
 *   // it->member can be used as well.
 *   // it.index contains the iteration count.
 * }
 *
 * If the iteration variable is const, the reference is too.
 *
 * for (const auto&& it : folly::enumerate(vec)) {
 *   // *it is always a const reference.
 * }
 *
 * @author Giuseppe Ottaviano <ott@fb.com>
 */

namespace folly {

namespace detail {

template <class T>
struct MakeConst {
  using type = const T;
};
template <class T>
struct MakeConst<T&> {
  using type = const T&;
};
template <class T>
struct MakeConst<T*> {
  using type = const T*;
};

template <class Iterator>
class Enumerator {
 public:
  explicit Enumerator(Iterator it) : it_(std::move(it)) {}

  class Proxy {
   public:
    using difference_type = ssize_t;
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using reference = typename std::iterator_traits<Iterator>::reference;
    using pointer = typename std::iterator_traits<Iterator>::pointer;
    using iterator_category = std::input_iterator_tag;

    FOLLY_ALWAYS_INLINE explicit Proxy(const Enumerator& e)
        : index(e.idx_), element(*e.it_) {}

    // Non-const Proxy: Forward constness from Iterator.
    FOLLY_ALWAYS_INLINE reference operator*() { return element; }
    FOLLY_ALWAYS_INLINE pointer operator->() { return std::addressof(element); }

    // Const Proxy: Force const references.
    FOLLY_ALWAYS_INLINE typename MakeConst<reference>::type operator*() const {
      return element;
    }
    FOLLY_ALWAYS_INLINE typename MakeConst<pointer>::type operator->() const {
      return std::addressof(element);
    }

   public:
    const size_t index;
    reference element;
  };

  FOLLY_ALWAYS_INLINE Proxy operator*() const { return Proxy(*this); }

  FOLLY_ALWAYS_INLINE Enumerator& operator++() {
    ++it_;
    ++idx_;
    return *this;
  }

  template <typename OtherIterator>
  FOLLY_ALWAYS_INLINE bool operator==(
      const Enumerator<OtherIterator>& rhs) const {
    return it_ == rhs.it_;
  }

  template <typename OtherIterator>
  FOLLY_ALWAYS_INLINE bool operator!=(
      const Enumerator<OtherIterator>& rhs) const {
    return !(it_ == rhs.it_);
  }

 private:
  template <typename OtherIterator>
  friend class Enumerator;

  Iterator it_;
  size_t idx_ = 0;
};

template <class Range>
class RangeEnumerator {
  Range r_;
  using BeginIteratorType = decltype(std::declval<Range>().begin());
  using EndIteratorType = decltype(std::declval<Range>().end());

 public:
  explicit RangeEnumerator(Range&& r) : r_(std::forward<Range>(r)) {}

  Enumerator<BeginIteratorType> begin() {
    return Enumerator<BeginIteratorType>(r_.begin());
  }
  Enumerator<EndIteratorType> end() {
    return Enumerator<EndIteratorType>(r_.end());
  }
};

} // namespace detail

template <class Range>
detail::RangeEnumerator<Range> enumerate(Range&& r) {
  return detail::RangeEnumerator<Range>(std::forward<Range>(r));
}

} // namespace folly
