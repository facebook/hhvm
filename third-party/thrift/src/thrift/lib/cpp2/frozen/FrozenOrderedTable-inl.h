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

#include <type_traits>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>

namespace apache {
namespace thrift {
namespace frozen {
namespace detail {

template <typename Container>
using detect_key_compare = typename Container::key_compare;

template <typename Container>
constexpr bool isOrderedContainer() {
  if constexpr (folly::is_detected_v<detect_key_compare, Container>) {
    return std::is_same_v<
        typename Container::key_compare,
        std::less<typename Container::key_type>>;
  } else {
    return false;
  }
}

/**
 * Layout specialization for unique ordered range types which support
 * binary-search based lookup.
 */
template <class T, class Item, class KeyExtractor, class Key = T>
struct SortedTableLayout : public ArrayLayout<T, Item> {
 private:
  static bool containerIsSorted(const T& cont) {
    return std::is_sorted(
        cont.begin(), cont.end(), [](const Item& a, const Item& b) {
          return KeyExtractor::getKey(a) < KeyExtractor::getKey(b);
        });
  }

 public:
  typedef ArrayLayout<T, Item> Base;
  typedef SortedTableLayout LayoutSelf;

  void thaw(ViewPosition self, T& out) const {
    auto v = view(self);
    if constexpr (HasSortedUniqueCtor<T>::value) {
      typename T::container_type outBuffer;
      outBuffer.reserve(v.size());
      for (auto it = v.begin(); it != v.end(); ++it) {
        outBuffer.emplace_back(it.thaw());
      }
      out = T{folly::sorted_unique, std::move(outBuffer)};
    } else {
      out.clear();
      apache::thrift::detail::pm::reserve_if_possible(&out, v.size());
      for (auto it = v.begin(); it != v.end(); ++it) {
        out.insert(out.end(), it.thaw());
      }
    }
    if constexpr (isOrderedContainer<T>()) {
      DCHECK(containerIsSorted(out));
    }
  }

  // provide indirect sort table if the collection isn't pre-sorted
  static void maybeIndex(const T& coll, std::vector<const Item*>& index) {
    index.clear();
    if constexpr (isOrderedContainer<T>()) {
      DCHECK(containerIsSorted(coll));
      return;
    } else {
      if (containerIsSorted(coll)) {
        return;
      }
      index.reserve(coll.size());
      for (decltype(auto) item : coll) {
        index.push_back(KeyExtractor::getPointer(item));
      }
      std::sort(index.begin(), index.end(), [](const Item* pa, const Item* pb) {
        return KeyExtractor::getKey(*pa) < KeyExtractor::getKey(*pb);
      });
    }
  }

  static void ensureDistinctKeys(
      const typename KeyExtractor::KeyType& key1,
      const typename KeyExtractor::KeyType& key2) {
    if (!(key1 < key2)) {
      throw std::domain_error("Input collection is not distinct");
    }
  }

  FieldPosition layoutItems(
      LayoutRoot& root,
      const T& coll,
      LayoutPosition /* self */,
      FieldPosition pos,
      LayoutPosition write,
      FieldPosition writeStep) final {
    std::vector<const Item*> index;
    maybeIndex(coll, index);

    FieldPosition noField; // not really used
    const typename KeyExtractor::KeyType* lastKey = nullptr;
    if (index.empty()) {
      // either the collection was already sorted or it's empty
      for (decltype(auto) item : coll) {
        root.layoutField(write, noField, this->itemField, item);
        write = write(writeStep);
        const typename KeyExtractor::KeyType* itemKey =
            &KeyExtractor::getKey(item);
        if (lastKey) {
          ensureDistinctKeys(*lastKey, *itemKey);
        }
        lastKey = itemKey;
      }
    } else {
      if constexpr (!isOrderedContainer<T>()) {
        // collection was non-empty and non-sorted, needs indirection table.
        for (auto ptr : index) {
          root.layoutField(write, noField, this->itemField, *ptr);
          write = write(writeStep);
          const typename KeyExtractor::KeyType* itemKey =
              &KeyExtractor::getKey(*ptr);
          if (lastKey) {
            ensureDistinctKeys(*lastKey, *itemKey);
          }
          lastKey = itemKey;
        }
      } else {
        LOG(FATAL) << "can't happen!";
      }
    }

    return pos;
  }

  void freezeItems(
      FreezeRoot& root,
      const T& coll,
      FreezePosition /* self */,
      FreezePosition write,
      FieldPosition writeStep) const final {
    std::vector<const Item*> index;
    maybeIndex(coll, index);

    FieldPosition noField; // not really used
    if (index.empty()) {
      // either the collection was already sorted or it's empty
      for (decltype(auto) item : coll) {
        root.freezeField(write, this->itemField, item);
        write = write(writeStep);
      }
    } else {
      // collection was non-empty and non-sorted, needs indirection table.
      for (auto ptr : index) {
        root.freezeField(write, this->itemField, *ptr);
        write = write(writeStep);
      }
    }
  }

  class View : public Base::View {
    typedef typename Layout<Key>::View KeyView;
    typedef typename Layout<Item>::View ItemView;

   public:
    View() {}
    View(const LayoutSelf* layout, ViewPosition position)
        : Base::View(layout, position) {}

    typedef typename Base::View::iterator iterator;

    void operator[](size_t) = delete;

    /// Returns an iterator pointing to the first element that compares not less
    /// to the value `key`. This allows finding a frozen element in the ordered
    /// table without freezing the key.
    template <
        typename K,
        class = std::enable_if_t<!std::is_convertible_v<K, KeyView>, void>>
    iterator lower_bound(const K& key) const {
      return std::lower_bound(
          this->begin(), this->end(), key, [](ItemView a, const K& b) {
            return KeyExtractor::getViewKey(a) < b;
          });
    }

    iterator lower_bound(const KeyView& key) const {
      return lower_bound<KeyView, void>(key);
    }

    /// Returns an iterator pointing to the first element that compares greater
    /// to the value `key`. This allows finding a frozen element in the ordered
    /// table without freezing the key.
    template <
        typename K,
        class = std::enable_if_t<!std::is_convertible_v<K, KeyView>, void>>
    iterator upper_bound(const K& key) const {
      return std::upper_bound(
          this->begin(), this->end(), key, [](const K& a, ItemView b) {
            return a < KeyExtractor::getViewKey(b);
          });
    }

    iterator upper_bound(const KeyView& key) const {
      return upper_bound<KeyView, void>(key);
    }

    /// Returns a range containing all elements that compares equal to the value
    /// `key`. This allows finding a frozen element in the ordered table without
    /// freezing the key.
    template <
        typename K,
        class = std::enable_if_t<!std::is_convertible_v<K, KeyView>, void>>
    std::pair<iterator, iterator> equal_range(const K& key) const {
      auto found = lower_bound(key);
      if (found != this->end() && KeyExtractor::getViewKey(*found) == key) {
        auto next = found;
        return std::make_pair(found, ++next);
      } else {
        return std::make_pair(found, found);
      }
    }

    std::pair<iterator, iterator> equal_range(const KeyView& key) const {
      return equal_range<KeyView, void>(key);
    }

    /// Finds an element with key that compares equivalent to the value `key`.
    /// This allows finding a frozen element in the ordered table without
    /// freezing the key.
    template <
        typename K,
        class = std::enable_if_t<!std::is_convertible_v<K, KeyView>, void>>
    iterator find(const K& key) const {
      auto found = lower_bound(key);
      if (found != this->end() && KeyExtractor::getViewKey(*found) == key) {
        return found;
      } else {
        return this->end();
      }
    }

    iterator find(const KeyView& key) const { return find<KeyView, void>(key); }

    /// Returns the number of elements with key that compares equivalent to the
    /// value `key`. This allows finding a frozen element in the ordered table
    /// without freezing the key.
    template <
        typename K,
        class = std::enable_if_t<!std::is_convertible_v<K, KeyView>, void>>
    size_t count(const K& key) const {
      return find(key) == this->end() ? 0 : 1;
    }

    size_t count(const KeyView& key) const { return count<KeyView, void>(key); }

    T thaw() const {
      T ret;
      static_cast<const SortedTableLayout*>(this->layout_)
          ->thaw(this->position_, ret);
      return ret;
    }
  };

  View view(ViewPosition self) const { return View(this, self); }
};

} // namespace detail
} // namespace frozen
} // namespace thrift
} // namespace apache
