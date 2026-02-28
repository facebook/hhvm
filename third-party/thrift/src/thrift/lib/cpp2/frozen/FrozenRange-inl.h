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

#include <deque>

namespace apache {
namespace thrift {
namespace frozen {

namespace detail {

/**
 * Layout specialization for range types, excluding those covered by 'string'
 * type. Frozen arrays support random-access and iteration without thawing.
 */
template <class T, class Item>
struct ArrayLayout : public LayoutBase {
  typedef LayoutBase Base;
  typedef ArrayLayout LayoutSelf;

  Field<size_t> distanceField;
  Field<size_t> countField;
  Field<Item> itemField;

  ArrayLayout()
      : LayoutBase(typeid(T)),
        distanceField(1, "distance"),
        countField(2, "count"),
        itemField(3, "item") {}

  FieldPosition maximize() {
    FieldPosition pos = startFieldPosition();
    pos = maximizeField(pos, distanceField);
    pos = maximizeField(pos, countField);
    maximizeField(FieldPosition(), itemField);
    return pos;
  }

  FieldPosition layout(LayoutRoot& root, const T& coll, LayoutPosition self) {
    FieldPosition pos = startFieldPosition();
    size_t n = coll.size();
    pos = root.layoutField(self, pos, countField, n);
    if (!n) {
      pos = root.layoutField(self, pos, distanceField, 0);
      return pos;
    }
    size_t itemBytes = itemField.layout.size;
    size_t itemBits = itemBytes ? 0 : itemField.layout.bits;
    size_t align = IsBlitType<Item>::value ? alignof(Item) : 1;
    size_t dist = root.layoutBytesDistance(
        self.start, itemBits ? (n * itemBits + 7) / 8 : n * itemBytes, align);

    pos = root.layoutField(self, pos, distanceField, dist);

    LayoutPosition write{self.start + dist, 0};
    FieldPosition writeStep(itemBytes, itemBits);
    return layoutItems(root, coll, self, pos, write, writeStep);
  }

  virtual FieldPosition layoutItems(
      LayoutRoot& root,
      const T& coll,
      LayoutPosition /* self */,
      FieldPosition pos,
      LayoutPosition write,
      FieldPosition writeStep) {
    FieldPosition noField; // not really used
    for (const auto& it : coll) {
      root.layoutField(write, noField, this->itemField, it);
      write = write(writeStep);
    }

    return pos;
  }

  void freeze(FreezeRoot& root, const T& coll, FreezePosition self) const {
    size_t n = coll.size();
    root.freezeField(self, countField, n);
    if (!n) {
      root.freezeField(self, distanceField, 0);
      return;
    }
    size_t itemBytes = itemField.layout.size;
    size_t itemBits = itemBytes ? 0 : itemField.layout.bits;
    folly::MutableByteRange range;
    size_t dist;
    size_t align = IsBlitType<Item>::value ? alignof(Item) : 1;
    root.appendBytes(
        self.start,
        itemBits ? (n * itemBits + 7) / 8 : n * itemBytes,
        range,
        dist,
        align);

    root.freezeField(self, distanceField, dist);

    FreezePosition write{range.begin(), 0};
    FieldPosition writeStep(itemBytes, itemBits);
    freezeItems(root, coll, self, write, writeStep);
  }

  virtual void freezeItems(
      FreezeRoot& root,
      const T& coll,
      FreezePosition /* self */,
      FreezePosition write,
      FieldPosition writeStep) const {
    for (const auto& it : coll) {
      root.freezeField(write, itemField, it);
      write = write(writeStep);
    }
  }

  void thaw(ViewPosition self, T& out) const {
    out.clear();
    auto outIt = std::back_inserter(out);
    auto v = view(self);
    for (auto it = v.begin(); it != v.end(); ++it) {
      *outIt++ = it.thaw();
    }
  }

  void print(std::ostream& os, int level) const override {
    LayoutBase::print(os, level);
    os << "range of " << folly::demangle(type.name());
    distanceField.print(os, level + 1);
    countField.print(os, level + 1);
    itemField.print(os, level + 1);
  }

  void clear() override {
    LayoutBase::clear();
    distanceField.clear();
    countField.clear();
    itemField.clear();
  }

  FROZEN_SAVE_INLINE(FROZEN_SAVE_FIELD(distance) FROZEN_SAVE_FIELD(count)
                         FROZEN_SAVE_FIELD(item))

  FROZEN_LOAD_INLINE(FROZEN_LOAD_FIELD(distance, 1) FROZEN_LOAD_FIELD(count, 2)
                         FROZEN_LOAD_FIELD(item, 3))

  /**
   * A view of a range, which produces views of items upon indexing or iterator
   * dereference
   */
  class View : public ViewBase<View, ArrayLayout, T> {
    typedef typename Layout<Item>::View ItemView;
    class Iterator;

    static ViewPosition indexPosition(
        const byte* start, size_t i, const LayoutBase& itemLayout) {
      if (itemLayout.size) {
        return ViewPosition{start + itemLayout.size * i, 0};
      } else {
        return ViewPosition{start, itemLayout.bits * i};
      }
    }

    const Layout<Item>& itemLayout() const {
      return this->layout_->itemField.layout;
    }

   public:
    typedef ItemView value_type;
    typedef ItemView reference_type;
    typedef Iterator iterator;
    typedef Iterator const_iterator;

    View() {}
    View(const LayoutSelf* layout, ViewPosition self)
        : ViewBase<View, ArrayLayout, T>(layout, self) {
      thawField(self, layout->countField, count_);
      if (count_) {
        size_t dist;
        thawField(self, layout->distanceField, dist);
        data_ = self.start + dist;
      }
    }

    ItemView operator[](size_t index) const {
      return itemLayout().view(indexPosition(data_, index, itemLayout()));
    }

    ItemView front() const {
      assert(!empty());
      return (*this)[0];
    }

    ItemView back() const {
      assert(!empty());
      return (*this)[size() - 1];
    }

    const_iterator begin() const { return const_iterator(*this, 0); }

    const_iterator end() const { return const_iterator(*this); }

    size_t count() const { return count_; }

    bool empty() const { return !count_; }
    size_t size() const { return count_; }

    folly::Range<const Item*> range() const {
      static_assert(apache::thrift::frozen::detail::IsBlitType<Item>::value);
      auto data = reinterpret_cast<const Item*>(data_);
      return {data, data + count_};
    }

   private:
    /**
     * Simple iterator on a range, with additional '.thaw()' member for thawing
     * a single member.
     */
    class Iterator {
     public:
      using difference_type = ptrdiff_t;
      using value_type = ItemView;
      using pointer = const value_type*;
      using reference = value_type;
      using iterator_category = std::random_access_iterator_tag;

      Iterator() {}

      Iterator(const View& outer, size_t index)
          : outer_(outer), index_(index) {}

      explicit Iterator(const View& outer)
          : outer_(outer), index_(outer.count_) {}

      ItemView operator*() const {
        DCHECK_LT(index_, outer_.count_);
        return outer_.itemLayout().view(position());
      }

      struct ArrowProxy {
        explicit ArrowProxy(ItemView item) : item_(std::move(item)) {}
        const ItemView* operator->() const { return &item_; }
        ItemView item_;
      };
      ArrowProxy operator->() const { return ArrowProxy(operator*()); }

      Item thaw() const {
        Item item;
        outer_.itemLayout().thaw(position(), item);
        return item;
      }

      ptrdiff_t operator-(const Iterator& other) const {
        return index_ - other.index_;
      }

      Iterator& operator++() {
        ++index_;
        return *this;
      }
      Iterator& operator+=(ptrdiff_t delta) {
        index_ += delta;
        return *this;
      }
      Iterator& operator--() {
        --index_;
        return *this;
      }
      Iterator& operator-=(ptrdiff_t delta) {
        index_ -= delta;
        return *this;
      }
      Iterator operator++(int) {
        Iterator ret(*this);
        ++*this;
        return ret;
      }
      Iterator operator--(int) {
        Iterator ret(*this);
        --*this;
        return ret;
      }
      Iterator operator+(ptrdiff_t delta) const {
        Iterator ret(*this);
        ret += delta;
        return ret;
      }
      Iterator operator-(ptrdiff_t delta) const {
        Iterator ret(*this);
        ret -= delta;
        return ret;
      }

      bool operator==(const Iterator& other) const {
        return index_ == other.index_;
      }

      bool operator!=(const Iterator& other) const { return !(*this == other); }

      bool operator<(const Iterator& other) const {
        return index_ < other.index_;
      }

      bool operator<=(const Iterator& other) const {
        return index_ <= other.index_;
      }

      bool operator>(const Iterator& other) const {
        return index_ > other.index_;
      }

      bool operator>=(const Iterator& other) const {
        return index_ >= other.index_;
      }

     private:
      ViewPosition position() const {
        return indexPosition(outer_.data_, index_, outer_.itemLayout());
      }

      View outer_;
      // NB: Begin/End addresses won't work with Frozen because of the
      // possibility of zero-byte items.
      size_t index_{0};
    };

    const byte* data_{nullptr};
    size_t count_{0};
  };

  View view(ViewPosition self) const { return View(this, self); }
};

} // namespace detail

template <class T>
struct Layout<T, typename std::enable_if<IsList<T>::value>::type>
    : public apache::thrift::frozen::detail::
          ArrayLayout<T, typename T::value_type> {};
} // namespace frozen
} // namespace thrift
} // namespace apache

THRIFT_DECLARE_TRAIT_TEMPLATE(IsList, std::vector)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsList, std::deque)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsList, folly::fbvector)
