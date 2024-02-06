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

#include <folly/Traits.h>
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

class Fast64BitRemainderCalculator {
#if FOLLY_HAVE_INT128_T
 public:
  Fast64BitRemainderCalculator() = default;
  explicit Fast64BitRemainderCalculator(uint64_t divisor)
      : fastRemainderConstant_(
            divisor ? (~folly::uint128_t(0) / divisor + 1) : 0) {
#ifndef NDEBUG
    divisor_ = divisor;
#endif
  }

  size_t remainder(size_t lhs, size_t rhs) const {
    const folly::uint128_t lowBits = fastRemainderConstant_ * lhs;
    auto result = mul128_u64(lowBits, rhs);
    assert(rhs == divisor_);
    assert(result == lhs % rhs);
    return result;
  }

 private:
  static uint64_t mul128_u64(folly::uint128_t lowbits, uint64_t d) {
    folly::uint128_t bottom = ((lowbits & 0xFFFFFFFFFFFFFFFFUL) * d) >> 64;
    folly::uint128_t top = (lowbits >> 64) * d;
    return static_cast<uint64_t>((bottom + top) >> 64);
  }
  folly::uint128_t fastRemainderConstant_ = 0;
#ifndef NDEBUG
  size_t divisor_ = 0;
#endif
#else
 public:
  Fast64BitRemainderCalculator() = default;
  explicit Fast64BitRemainderCalculator(size_t) {}

  auto remainder(size_t lhs, size_t rhs) const { return lhs % rhs; }
#endif
};

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
            KeyLayout::hash(KeyExtractor::getViewKey(
                this->itemField.layout.view({write.start, write.bitOffset}))));
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
    Fast64BitRemainderCalculator remainderCalculator_;

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

    iterator find(const KeyView& key) const {
      const auto buckets = table_.size() * Block::bits;
      auto bucket = KeyLayout::hash(key) * 5; // spread out clumped values
      for (size_t p = 0; p < buckets; bucket += ++p) { // quadratic probing
        bucket = remainderCalculator_.remainder(bucket, buckets);
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

    size_t count(const KeyView& key) const {
      return find(key) == this->end() ? 0 : 1;
    }

    T thaw() const {
      T ret;
      static_cast<const HashTableLayout*>(this->layout_)
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
