/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
#include <unordered_set>
#include <vector>

namespace apache {
namespace thrift {

template <class T>
struct BaseDistinctTablePolicy {
  template <class Hash, class Equal>
  using Index = std::unordered_set<size_t, Hash, Equal>;

  using Hash = std::hash<T>;
  using Equal = std::equal_to<T>;
  using Store = std::vector<T>;
};

/**
 * Accumulates only distinct values in an externally owned store.
 */
template <class T, template <class> class Policy = BaseDistinctTablePolicy>
class DistinctTable {
 public:
  typedef typename Policy<T>::Store Store;
  typedef typename Policy<T>::Hash Hash;
  typedef typename Policy<T>::Equal Equal;

  explicit DistinctTable(
      Store* store, Equal equal = Equal(), Hash hash = Hash())
      : store_(store),
        indexes_(0, HashIndirect(hash, store), EqualIndirect(equal, store)) {
    assert(store->empty());
  }

  /**
   * Construct a T given the arguments, return the index at which such a value
   * can now be found, either newly or previously constructed.
   */
  template <class... Args>
  size_t add(Args&&... args) {
    auto index = store_->size();
    store_->push_back(std::forward<Args>(args)...);
    auto insertion = indexes_.insert(index);
    if (insertion.second) {
      return index;
    } else {
      store_->pop_back();
      return *insertion.first;
    }
  }

 private:
  class HashIndirect : public Hash {
   public:
    HashIndirect(Hash _hash, Store* store)
        : Hash(std::move(_hash)), store_(store) {}

    size_t operator()(size_t i) const { return Hash::operator()((*store_)[i]); }

   private:
    Store* store_;
  };

  class EqualIndirect : public Equal {
   public:
    EqualIndirect(Equal equal, Store* store)
        : Equal(std::move(equal)), store_(store) {}

    bool operator()(size_t a, size_t b) const {
      return a == b || Equal::operator()((*store_)[a], (*store_)[b]);
    }

   private:
    Store* store_;
  };

  typedef typename Policy<T>::template Index<HashIndirect, EqualIndirect> Index;

  Store* store_;
  Index indexes_;
};

} // namespace thrift
} // namespace apache
