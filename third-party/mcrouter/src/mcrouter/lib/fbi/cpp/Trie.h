/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <array>
#include <memory>
#include <string>
#include <utility>

#include <folly/Range.h>

namespace facebook {
namespace memcache {

/**
 * Simple Trie implementation.
 *
 * @param Value type of stored value.
 */
template <class Value>
class Trie {
  static_assert(
      sizeof(folly::StringPiece::value_type) == 1,
      "Trie works only with 8 bit character types");

  /**
   * iterator implementation
   * @param T underlying container type
   * @param V value type
   */
  template <class T, class V>
  class iterator_base;

 public:
  typedef std::pair<const std::string, Value> value_type;
  typedef Value mapped_type;

  /**
   * Forward const iterator for Trie. Enumerates values stored in Trie in
   * lexicographic order of keys.
   */
  typedef iterator_base<const Trie, const value_type> const_iterator;
  /**
   * Forward iterator for Trie. Enumerates values stored in Trie in
   * lexicographic order of keys.
   */
  typedef iterator_base<Trie, value_type> iterator;

  Trie() = default;

  Trie(const Trie& other);

  Trie(Trie&& other) noexcept;

  Trie& operator=(const Trie& other);

  Trie& operator=(Trie&& other);

  /**
   * Return iterator for given key
   *
   * @return end() if no key found, iterator for given key otherwise
   */
  inline const_iterator find(folly::StringPiece key) const;

  inline iterator find(folly::StringPiece key);

  /**
   * Set value for key
   *
   * @param key
   * @param value
   */
  void emplace(folly::StringPiece key, Value value);

  /**
   * Get value of longest prefix stored in Trie
   *
   * @param key   String with any characters
   *
   * @return Iterator to the element with the longest prefix.
   *         If no such element is found, past-the-end (i.e. end()) iterator
   *         is returned.
   */
  iterator findPrefix(folly::StringPiece key);

  const_iterator findPrefix(folly::StringPiece key) const;

  inline const_iterator begin() const;

  inline iterator begin();

  inline const_iterator end() const;

  inline iterator end();

  inline const_iterator cbegin() const;

  inline const_iterator cend() const;

  void clear();

 private:
  // total number of children in one node
  static const size_t kNumChars = 16;

  // edges of this node (points to nodes with keys longer by one character)
  std::array<std::unique_ptr<Trie>, kNumChars> next_;

  // value stored in the node
  std::unique_ptr<value_type> value_;

  Trie* parent_{nullptr};
  char c_{0};

  const Trie* findImpl(folly::StringPiece key) const;

  const Trie* findPrefixImpl(folly::StringPiece key) const;

  static constexpr size_t getTopHalf(unsigned char c) {
    return c >> 4;
  }

  static constexpr size_t getBottomHalf(unsigned char c) {
    return c & 15;
  }
};
} // namespace memcache
} // namespace facebook

#include "Trie-inl.h"
