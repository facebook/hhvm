/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <boost/iterator/iterator_facade.hpp>

#include <glog/logging.h>

namespace facebook {
namespace memcache {

template <class Value>
Trie<Value>::Trie(const Trie& other) : parent_(other.parent_), c_(other.c_) {
  if (other.value_) {
    value_ = std::make_unique<value_type>(*other.value_);
  }

  for (size_t edge = 0; edge < kNumChars; ++edge) {
    if (other.next_[edge]) {
      next_[edge] = std::make_unique<Trie>(*other.next_[edge]);
      next_[edge]->parent_ = this;
    }
  }
}

template <class Value>
Trie<Value>::Trie(Trie&& other) noexcept
    : next_(std::move(other.next_)),
      value_(std::move(other.value_)),
      parent_(other.parent_),
      c_(other.c_) {
  for (size_t edge = 0; edge < kNumChars; ++edge) {
    if (next_[edge]) {
      next_[edge]->parent_ = this;
    }
  }
}

template <class Value>
Trie<Value>& Trie<Value>::operator=(const Trie& other) {
  return *this = Trie(other);
}

template <class Value>
Trie<Value>& Trie<Value>::operator=(Trie&& other) {
  assert(this != &other);
  next_ = std::move(other.next_);
  value_ = std::move(other.value_);
  parent_ = other.parent_;
  c_ = other.c_;

  for (size_t edge = 0; edge < kNumChars; ++edge) {
    if (next_[edge]) {
      next_[edge]->parent_ = this;
    }
  }

  return *this;
}

template <class Value>
typename Trie<Value>::const_iterator Trie<Value>::find(
    folly::StringPiece key) const {
  return const_iterator(findImpl(key));
}

template <class Value>
typename Trie<Value>::iterator Trie<Value>::find(folly::StringPiece key) {
  return iterator(const_cast<Trie*>(findImpl(key)));
}

template <class Value>
const Trie<Value>* Trie<Value>::findImpl(folly::StringPiece key) const {
  auto node = this;
  for (const auto c : key) {
    node = node->next_[getTopHalf(c)].get();
    if (node == nullptr) {
      return nullptr;
    }
    node = node->next_[getBottomHalf(c)].get();
    if (node == nullptr) {
      return nullptr;
    }
  }
  return node->value_ ? node : nullptr;
}

template <class Value>
void Trie<Value>::emplace(folly::StringPiece key, Value val) {
  auto node = this;
  for (const auto c : key) {
    size_t steps[] = {getTopHalf(c), getBottomHalf(c)};
    for (auto ci : steps) {
      auto& nx = node->next_[ci];
      if (!nx) {
        nx = std::make_unique<Trie>();
        nx->parent_ = node;
        nx->c_ = ci;
      }
      node = nx.get();
    }
  }

  node->value_ = std::make_unique<value_type>(key.str(), std::move(val));
}

template <class Value>
typename Trie<Value>::const_iterator Trie<Value>::findPrefix(
    folly::StringPiece key) const {
  return const_iterator(findPrefixImpl(key));
}

template <class Value>
typename Trie<Value>::iterator Trie<Value>::findPrefix(folly::StringPiece key) {
  return iterator(const_cast<Trie*>(findPrefixImpl(key)));
}

template <class Value>
const Trie<Value>* Trie<Value>::findPrefixImpl(folly::StringPiece key) const {
  auto node = this;
  const Trie* result = nullptr;
  for (const auto c : key) {
    if (node->value_) {
      result = node;
    }
    node = node->next_[getTopHalf(c)].get();
    if (node == nullptr) {
      return result;
    }
    node = node->next_[getBottomHalf(c)].get();
    if (node == nullptr) {
      return result;
    }
  }
  return node->value_ ? node : result;
}

template <class Value>
typename Trie<Value>::const_iterator Trie<Value>::begin() const {
  return cbegin();
}

template <class Value>
typename Trie<Value>::iterator Trie<Value>::begin() {
  iterator result{this};
  if (!value_) {
    ++result;
  }
  return result;
}

template <class Value>
typename Trie<Value>::const_iterator Trie<Value>::end() const {
  return cend();
}

template <class Value>
typename Trie<Value>::iterator Trie<Value>::end() {
  return iterator();
}

template <class Value>
typename Trie<Value>::const_iterator Trie<Value>::cbegin() const {
  const_iterator result{this};
  if (!value_) {
    ++result;
  }
  return result;
}

template <class Value>
typename Trie<Value>::const_iterator Trie<Value>::cend() const {
  return const_iterator();
}

template <class Value>
void Trie<Value>::clear() {
  value_.reset();
  for (auto& edge : next_) {
    edge.reset();
  }
  parent_ = nullptr;
  c_ = 0;
}

/* iterator_base */

template <class Value>
template <class T, class V>
class Trie<Value>::iterator_base
    : public boost::iterator_facade<
          /* Derived */ iterator_base<T, V>,
          /* Value */ V,
          /* CategoryOrTraversal */ boost::forward_traversal_tag> {
 public:
  iterator_base() = default;

 private:
  friend class Trie<Value>;
  friend class boost::iterator_core_access;
  T* t_{nullptr};

  explicit iterator_base(T* trie) : t_(trie) {}

  void increment() {
    // nonrecursive DFS over Trie.
    size_t edge = 0;
    while (t_ != nullptr) {
      // try to go "deeper"
      while (edge < kNumChars) {
        // found a child, so make key longer
        if (t_->next_[edge]) {
          t_ = t_->next_[edge].get();
          // found a child with value
          if (t_->value_) {
            return;
          }
          edge = 0;
        } else {
          ++edge;
        }
      }
      // nothing in this subtree, go back
      do {
        edge = t_->c_ + 1;
        t_ = t_->parent_;
      } while (t_ != nullptr && edge == kNumChars);
    }
    // here we have t_ == nullptr. It is equal to end() iterator of Trie.
  }

  bool equal(const iterator_base& other) const {
    return this->t_ == other.t_;
  }

  V& dereference() const {
    return *t_->value_;
  }
};

} // namespace memcache
} // namespace facebook
