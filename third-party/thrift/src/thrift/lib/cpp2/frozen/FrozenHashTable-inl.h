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

// IWYU pragma: private, include "thrift/lib/cpp2/frozen/Frozen.h"

#include <cstdint>
#include <type_traits>
#include <folly/math/Division.h>
#include <thrift/lib/cpp2/FieldRef.h>

namespace apache {
namespace thrift {
namespace frozen {
namespace detail {

struct Block {
  uint64_t mask = 0;
  size_t offset = 0;
  static constexpr size_t bits = 64;

  auto mask_ref() { return required_field_ref<uint64_t&>{mask}; }
  auto mask_ref() const { return required_field_ref<const uint64_t&>{mask}; }
  auto offset_ref() { return required_field_ref<size_t&>{offset}; }
  auto offset_ref() const { return required_field_ref<const size_t&>{offset}; }
};

struct BlockLayout : public LayoutBase {
  typedef LayoutBase Base;
  typedef Block T;
  typedef BlockLayout LayoutSelf;

  Field<uint64_t, TrivialLayout<uint64_t>> maskField;
  Field<uint64_t> offsetField;

  BlockLayout()
      : LayoutBase(typeid(T)), maskField(1, "mask"), offsetField(2, "offset") {}

  FieldPosition maximize();
  FieldPosition layout(LayoutRoot& root, const T& o, LayoutPosition self);
  void freeze(FreezeRoot& root, const T& o, FreezePosition self) const;
  void print(std::ostream& os, int level) const final;
  void clear() final;

  FROZEN_SAVE_INLINE(FROZEN_SAVE_FIELD(mask) FROZEN_SAVE_FIELD(offset))

  FROZEN_LOAD_INLINE(FROZEN_LOAD_FIELD(mask, 1) FROZEN_LOAD_FIELD(offset, 2))

  struct View : public ViewBase<View, LayoutSelf, T> {
    View() {}
    View(const LayoutSelf* layout, ViewPosition position)
        : ViewBase<View, LayoutSelf, T>(layout, position) {}

    uint64_t mask() const {
      return this->layout_->maskField.layout.view(
          this->position_(this->layout_->maskField.pos));
    }
    uint64_t offset() const {
      return this->layout_->offsetField.layout.view(
          this->position_(this->layout_->offsetField.pos));
    }
  };

  View view(ViewPosition self) const { return View(this, self); }
};
} // namespace detail

template <>
struct Layout<apache::thrift::frozen::detail::Block>
    : apache::thrift::frozen::detail::BlockLayout {};

namespace detail {

/**
 * Layout specialization for range types which support unique hash lookup.
 */
template <class T, class Item, class KeyExtractor, class Key>
struct HashTableLayout : public ArrayLayout<T, Item> {
  typedef ArrayLayout<T, Item> Base;
  Field<std::vector<Block>> sparseTableField;
  typedef Layout<Key> KeyLayout;
  typedef HashTableLayout LayoutSelf;

  HashTableLayout()
      : sparseTableField(
            4,
            "sparseTable") // continue field ids from ArrayLayout
  {}

  FieldPosition maximize() {
    FieldPosition pos = ArrayLayout<T, Item>::maximize();
    FROZEN_MAXIMIZE_FIELD(sparseTable);
    return pos;
  }

  static size_t blockCount(size_t size) {
    // LF = Load Factor, BPE = bits/entry
    // 1.5 => 66% LF => 3 bpe, 3 probes expected
    // 2.0 => 50% LF => 4 bpe, 2 probes expected
    // 2.5 => 40% LF => 5 bpe, 1.6 probes expected
    auto rv = size_t(size * 2.5 + Block::bits - 1) / Block::bits;

    // For integer keys that don't have entropy in the bottom bits we
    // will be in trouble if blockCount is a power of 2. If we always use
    // an odd blockCount then that case degenerates to probes averaging
    // Block::bits * LF / 2 = 12.8, which is quite bad but could be worse.
    // The problem can also occur if the hash code doesn't have entropy
    // in the top bits and the bucket count ends up being a multiple of 5,
    // due to the multiplier applied to the hash.
    rv |= 1;
    if ((rv % 5) == 0) {
      rv += 2;
    }
    return rv;
  }

  static void ensureDistinctKeys(
      const typename KeyExtractor::KeyType& key1,
      const typename KeyExtractor::KeyType& key2) {
    if (key1 == key2) {
      throw std::domain_error("Input collection is not distinct");
    }
  }

  static void buildIndex(
      const T& coll,
      std::vector<const Item*>& index,
      std::vector<Block>& sparseTable) {
    auto blocks = blockCount(coll.size());
    size_t buckets = blocks * Block::bits;
    sparseTable.resize(blocks);
    index.resize(buckets);
    for (auto& item : coll) {
      const typename KeyExtractor::KeyType* itemKey =
          &KeyExtractor::getKey(item);
      size_t h = KeyLayout::hash(*itemKey);
      h *= 5; // spread out clumped hash values
      for (size_t p = 0;; h += ++p) { // quadratic probing
        size_t bucket = h % buckets;
        const Item** slot = &index[bucket];
        if (*slot) {
          if (p == buckets) {
            throw std::out_of_range("All buckets full!");
          }
          ensureDistinctKeys(*itemKey, KeyExtractor::getKey(**slot));
          continue;
        } else {
          *slot = KeyExtractor::getPointer(item);
          break;
        }
      }
    }
    size_t count = 0;
    for (size_t blockIndex = 0; blockIndex < blocks; ++blockIndex) {
      Block& block = sparseTable[blockIndex];
      block.offset = count;
      for (size_t offset = 0; offset < Block::bits; ++offset) {
        if (index[blockIndex * Block::bits + offset]) {
          block.mask |= uint64_t(1) << offset;
          ++count;
        }
      }
    }
  }

  FieldPosition layoutItems(
      LayoutRoot& root,
      const T& coll,
      LayoutPosition self,
      FieldPosition pos,
      LayoutPosition write,
      FieldPosition writeStep) final {
    std::vector<const Item*> index;
    std::vector<Block> sparseTable;
    buildIndex(coll, index, sparseTable);

    pos = root.layoutField(self, pos, this->sparseTableField, sparseTable);

    FieldPosition noField; // not really used
    for (auto& it : index) {
      if (it) {
        root.layoutField(write, noField, this->itemField, *it);
        write = write(writeStep);
      }
    }

    return pos;
  }

  void freezeItems(
      FreezeRoot& root,
      const T& coll,
      FreezePosition self,
      FreezePosition write,
      FieldPosition writeStep) const final {
    std::vector<const Item*> index;
    std::vector<Block> sparseTable;
    buildIndex(coll, index, sparseTable);

    assert(index.empty() == sparseTable.empty());
    root.freezeField(self, this->sparseTableField, sparseTable);

    FieldPosition noField; // not really used
    for (auto& it : index) {
      if (it) {
        root.freezeField(write, this->itemField, *it);

        // Hash specializations must produce identical hashes for thawed and
        // frozen representations of the same value. Note that this hash must
        // also be robust in the presence of versioning; the addition of a new,
        // unset field must produce the same hashes as before the newly
        // introduced field.
        assert(
            KeyLayout::hash(KeyExtractor::getKey(*it)) ==
            KeyLayout::hash(
                KeyExtractor::getViewKey(this->itemField.layout.view(
                    {write.start, write.bitOffset}))));
        write = write(writeStep);
      }
    }
  }

  void thaw(ViewPosition self, T& out) const {
    out.clear();
    auto v = view(self);
    out.reserve(v.size());
    for (auto it = v.begin(); it != v.end(); ++it) {
      out.insert(it.thaw());
    }
  }

  void print(std::ostream& os, int level) const override {
    Base::print(os, level);
    sparseTableField.print(os, level + 1);
  }

  void clear() final {
    Base::clear();
    sparseTableField.clear();
  }

  FROZEN_SAVE_INLINE(FROZEN_SAVE_FIELD(sparseTable))

  FROZEN_LOAD_INLINE(FROZEN_LOAD_FIELD(sparseTable, 4))

  class View : public Base::View {
    typedef typename Layout<Key>::View KeyView;
    typedef typename Layout<Item>::View ItemView;
    typedef typename Layout<std::vector<Block>>::View TableView;

    TableView table_;
    folly::uint_divisor<std::uint64_t>::calc remainderCalculator_;

   public:
    View() {}
    View(const LayoutSelf* layout, ViewPosition self)
        : Base::View(layout, self),
          table_(layout->sparseTableField.layout.view(
              self(layout->sparseTableField.pos))),
          remainderCalculator_(table_.size() * Block::bits) {}

    typedef typename Base::View::iterator iterator;

    void operator[](size_t) = delete;

    std::pair<iterator, iterator> equal_range(const KeyView& key) const {
      auto found = find(key);
      if (found != this->end()) {
        auto next = found;
        return std::make_pair(found, ++next);
      } else {
        return std::make_pair(found, found);
      }
    }

    /// An opaquely-wrapped hash of a key, produced by prehash() and accepted
    /// by prefetch() and find() in place of rehashing the key.
    ///
    /// Mirrors folly::F14HashToken.
    class HashToken final {
     public:
      constexpr HashToken() = default;

     private:
      friend class View;
      constexpr explicit HashToken(size_t hash) noexcept : hash_(hash) {}
      size_t hash_{};
    };

    /// Hashes `key` and returns a token carrying that hash, so that a later
    /// find() on the same key can skip rehashing it.
    ///
    /// Useful whenever a hash would otherwise be recomputed: looking one key
    /// up in several maps, storing tokens alongside keys held in an auxiliary
    /// structure and looked up repeatedly, or composing with prefetch() to
    /// pipeline a batch of lookups.
    HashToken prehash(const KeyView& key) const {
      return HashToken{KeyLayout::hash(key)};
    }

    /// Same, for a key that has not been frozen. Mirrors the find() overload
    /// set so that both accept exactly the same key types.
    template <
        typename K,
        class = std::enable_if_t<!std::is_convertible_v<K, KeyView>, void>>
    HashToken prehash(const K& key) const {
      return HashToken{Layout<K>::hash(key)};
    }

    /// Issues a hardware prefetch for the sparse-table block that a subsequent
    /// find(token, ...) will probe first, without reading that block.
    ///
    /// Composes with prehash(): hash each key once, prefetch each token, then
    /// find each (token, key). The first probe's address is arithmetic over
    /// the token plus the array's own size and address -- no part of the array
    /// itself is read -- so the prefetches for a batch of keys can all be
    /// issued up front and their cache misses overlapped, in place of one
    /// dependent memory stall per key. Only the first probe is prefetched:
    /// later probes and the item fetch depend on the block's contents, and on
    /// a miss (the common case) the first block is the only line touched.
    ///
    /// Purely a performance hint -- it never faults and cannot change what
    /// find() returns.
    ///
    /// Example:
    ///
    ///   auto tok = map.prehash(key);
    ///   map.prefetch(tok);
    ///   ... other work here ...
    ///   ... perhaps this next find() is accelerated ...
    ///   auto it = map.find(tok, key);
    ///   if (it != map.end()) {
    ///     utilize(it->second);
    ///   }
    void prefetch(HashToken token) const {
      const auto buckets = table_.size() * Block::bits;
      if (buckets == 0) {
        return;
      }
      // The condition and Block::bits == 64 guarantee buckets > 1.
      const auto bucket =
          remainderCalculator_.rem_gt1(token.hash_ * 5, buckets);
      table_.prefetchItem(bucket / Block::bits);
    }

    iterator find(const KeyView& key) const { return find(prehash(key), key); }

    /// Finds an element with key that compares equivalent to the value `key`.
    /// This allows finding a frozen element in the hash table without freezing
    /// the key. Similar to heterogenous lookups in C++20.
    template <
        typename K,
        class = std::enable_if_t<!std::is_convertible_v<K, KeyView>, void>>
    iterator find(const K& key) const {
      return find(prehash(key), key);
    }

    /// Finds using a hash already computed by prehash(). `token` must be the
    /// result of prehash(key) for this same `key` -- it is not a hint, and
    /// passing a token belonging to a different key is a bug.
    iterator find(HashToken token, const KeyView& key) const {
      return findImpl(key, token.hash_);
    }

    template <
        typename K,
        class = std::enable_if_t<!std::is_convertible_v<K, KeyView>, void>>
    iterator find(HashToken token, const K& key) const {
      return findImpl(key, token.hash_);
    }

    size_t count(const KeyView& key) const { return count<KeyView, void>(key); }

    /// Finds the number of elements with key that compares equivalent to the
    /// value `key`. This allows finding a frozen element in the hash table
    /// without freezing the key. Similar to heterogenous lookups in C++20.
    template <
        typename K,
        class = std::enable_if_t<!std::is_convertible_v<K, KeyView>, void>>
    size_t count(const K& key) const {
      return find(key) == this->end() ? 0 : 1;
    }

    T thaw() const {
      T ret;
      static_cast<const HashTableLayout*>(this->layout_)
          ->thaw(this->position_, ret);
      return ret;
    }

   private:
    template <typename K>
    iterator findImpl(const K& key, const size_t keyHash) const {
      const auto buckets = table_.size() * Block::bits;
      auto bucket = keyHash * 5; // spread out clumped values
      for (size_t p = 0; p < buckets; bucket += ++p) { // quadratic probing
        // The loop condition and Block::bits == 64 guarantee buckets > 1.
        bucket = remainderCalculator_.rem_gt1(bucket, buckets);
        const auto& block = table_[bucket / Block::bits]; // major block
        auto mask = block.mask();
        auto offset = block.offset();
        auto minor = bucket % Block::bits;
        for (;;) {
          if (0 == (1 & (mask >> minor))) {
            return this->end();
          }
          auto found = this->begin() + offset +
              folly::popcount(mask & ((1ULL << minor) - 1)) /* subOffset */;
          if (KeyExtractor::getViewKey(*found) == key) {
            return found;
          }
          minor += ++p;
          if (LIKELY(minor < Block::bits)) {
            bucket += p; // same block shortcut
          } else {
            --p; // undo
            break;
          }
        }
      }
      return this->end();
    }
  };

  View view(ViewPosition self) const { return View(this, self); }
};
} // namespace detail
} // namespace frozen
} // namespace thrift
} // namespace apache
