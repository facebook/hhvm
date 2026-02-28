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
/**
 * Layout for thrift reference types, aka std::unique_ptr and std::shared_ptr
 * @author Andy Wei<aw7@fb.com>
 *
 * --Motivations--
 * Thrift supports std::unique_ptr and std::shared_ptr via field annotations,
 * including cpp.ref = "true", cpp2.ref = "true", cpp2.ref_type = "shared" and
 * cpp2.ref_type = "unique, which bring powerful data structures (linked lists,
 * trees, graphs) and effieciency gains to applications, so it would be nice to
 * have them available in Frozen.
 *
 * Below are the key problems and the related design decisions.
 *
 * --Types graph--
 * If we model every type as a node, and model the dependence relationship as
 * the edge between them, all the involved types form a directed graph.
 * Furthermore, there could be circles in the graph - a classic LinkedListNode
 * type has a next pointer points to another LinkedListNode, so in the type
 * graph that dependent edge ends on itself.
 *
 * The solution to type graph is FieldCycleHolder defined in "Frozen.h".
 * It a map of stacks, the key is type_id, the value is shared Field of that
 * type, so all the nodes on a linked list or a tree share the same Field and
 * layout, and only the list head or tree root has a non-null ownedField_. As
 * this solution reduces number of fields, it also reduces the serialized frozen
 * string size.
 *
 * --Objects graph--
 * Frozen deals with every no-pointer thrift struct as a tree. Introducing the
 * pointer concept also introduces complexity: a tree can point to another tree,
 * and so on, so we'll see a linked list of trees; when a node has more than one
 * pointers, we'll see a tree of trees; as there could be cycles, then we'll see
 * a graph of trees.
 *
 * Upon layout and freeze, we need to traverse the whole graph. To detect loops
 * and make sure every node will only be freeze once, every "visited" objects
 * needs to be recorded, which is done by invoking the registerLayoutPosition()
 * and registerFreezePosition(), hence there are calls
 * to check and reuse the registered positions.
 *
 * --Schema Evolution--
 * Thrift allows user add or remove fields into/from any struct/union, which is
 * schema evolution. For Frozen, survive schema evolution means a Frozen string
 * generated under the old schema should be able to be mapped back to a Frozen
 * object of the new schema. When the added/removed fields are reference types,
 * this backward compatibility should not be compromised. Those constrains are
 * handled in save() and load().
 *
 * -Offset-
 * To save space, we use variable length (packed integer) for offset field.
 * Offset is encoded distance between the position of this reference field and
 * the position of the value field. The encode rules are:
 *     if the pointer is nullptr, offset = 0;
 *     else, offset = 2 * distance + 1. This rule makes sure a meaningful 0
 * distance will not be treated as nullptr.
 */

namespace apache {
namespace thrift {
namespace frozen {
namespace detail {

static constexpr int64_t kOffsetNull = 0;

/**
 * Layout for smart pointers
 */
template <class T>
struct RefLayout : public LayoutBase {
  typedef LayoutBase Base;

  Field<int64_t> offsetField;

  /**
   * The raw pointer of the real owner's ownedField_.
   */
  Field<T>* valueField_{nullptr};

  RefLayout() : LayoutBase(typeid(T)), offsetField(1, "offset") {}

  FieldPosition maximize() {
    FieldPosition pos = startFieldPosition();
    FROZEN_MAXIMIZE_FIELD(offset);
    return pos;
  }

  FieldPosition layoutValueField(
      LayoutRoot& root, LayoutPosition self, FieldPosition pos, const T* ptr) {
    assert(ptr);
    assert(valueField_);
    size_t valueBytes = valueField_->layout.size;
    size_t valueBits = valueBytes ? 0 : valueField_->layout.bits;
    size_t align = IsBlitType<T>::value ? alignof(T) : 1;
    size_t dist = root.layoutBytesDistance(
        self.start, valueBits ? (valueBits + 7) / 8 : valueBytes, align);
    pos = root.layoutField(self, pos, offsetField, encodeOffset(dist));

    LayoutPosition write{self.start + dist, 0};
    root.registerLayoutPosition(ptr, write);
    FieldPosition noField; // not really used
    root.layoutField(write, noField, *valueField_, *ptr);
    return pos;
  }

  void freezeValueField(
      FreezeRoot& root, FreezePosition self, const T* ptr) const {
    assert(ptr);
    assert(valueField_);
    size_t valueBytes = valueField_->layout.size;
    size_t valueBits = valueBytes ? 0 : valueField_->layout.bits;
    size_t align = IsBlitType<T>::value ? alignof(T) : 1;
    size_t dist;
    folly::MutableByteRange range;
    root.appendBytes(
        self.start,
        valueBits ? (valueBits + 7) / 8 : valueBytes,
        range,
        dist,
        align);
    root.freezeField(self, offsetField, encodeOffset(dist));
    FreezePosition write{range.begin(), 0};
    root.registerFreezePosition(ptr, write);
    root.freezeField(write, *valueField_, *ptr);
  }

  void print(std::ostream& os, int level) const final {
    Base::print(os, level);
    os << "ref of " << folly::demangle(type.name());
    offsetField.print(os, level + 1);
    if (valueField_) {
      valueField_->print(os, level + 1);
    }
  }

  void clear() final {
    offsetField.clear();
    valueField_ = nullptr;
  }

  inline int64_t encodeOffset(size_t dist) const {
    return static_cast<int64_t>(dist * 2) + 1;
  }

  inline int64_t encodeOffset(int64_t byteOffset) const {
    return byteOffset * 2 + 1;
  }

  inline int64_t decodeOffset(int64_t encoded) const {
    return (encoded - 1) / 2;
  }

  /**
   * A view of a smart pointer, which produces a pointer-like object
   */
  class View : public ViewBase<View, RefLayout, T> {
    typedef typename Layout<T>::View ValueView;

   public:
    View() {}
    View(const RefLayout* layout, ViewPosition self)
        : ViewBase<View, RefLayout, T>(layout, self) {
      if (layout->valueField_) {
        int64_t offset = 0;
        thawField(self, layout->offsetField, offset);
        if (offset != kOffsetNull) {
          ViewPosition that = self(layout->decodeOffset(offset));
          valueView_ =
              layout->valueField_->layout.view(that(layout->valueField_->pos));
        }
      }
    }

    // operators
    inline const ValueView& operator*() const {
      assert(valueView_);
      return *valueView_;
    }

    inline const ValueView* operator->() const {
      assert(valueView_);
      return &*valueView_;
    }

    inline explicit operator bool() const noexcept { return bool(valueView_); }

    inline bool operator==(const View& that) const noexcept {
      if (valueView_) {
        return that.valueView_ &&
            valueView_->getPosition() == that.valueView_->getPosition();
      } else {
        return !that.valueView_;
      }
    }

    inline bool operator!=(const View& that) const noexcept {
      if (valueView_) {
        return !that.valueView_ ||
            valueView_->getPosition() != that.valueView_->getPosition();
      } else {
        return bool(that.valueView_);
      }
    }

    inline bool operator==(std::nullptr_t) const noexcept {
      return !valueView_;
    }

    inline bool operator!=(std::nullptr_t) const noexcept {
      return bool(valueView_);
    }

    friend inline bool operator==(
        const std::nullptr_t, const View& x) noexcept {
      return !x.valueView_;
    }

    friend inline bool operator!=(
        const std::nullptr_t, const View& x) noexcept {
      return bool(x.valueView_);
    }

   private:
    folly::Optional<ValueView> valueView_;
  };

  View view(ViewPosition self) const { return View(this, self); }
};

/**
 * Layout specialization for std::shared_ptr<[const] T>
 */
template <class T>
struct SharedRefLayout : public RefLayout<T> {
  std::shared_ptr<Field<T>> ownedField_;

  template <class U>
  FieldPosition layout(
      LayoutRoot& root,
      const std::shared_ptr<U>& pointer,
      LayoutPosition self) {
    FieldPosition pos = this->startFieldPosition();
    if (!pointer) {
      pos = root.layoutField(self, pos, this->offsetField, kOffsetNull);
      return pos;
    }

    this->valueField_ = root.pushCycle(this->ownedField_, 2, "owned");
    auto ptr = pointer.get();
    auto sharedField = root.sharedFieldOf(ptr);
    if (auto alreadyLayout = root.layoutPositionOf(ptr)) {
      assert(sharedField);
      int64_t offset = alreadyLayout->byteOffset(self);
      pos = root.layoutField(
          self, pos, this->offsetField, this->encodeOffset(offset));
      if (this->ownedField_) {
        // when ptr is one of the multiple shared_ptrs point to the same node,
        // reuse the shared field
        this->ownedField_ = sharedField;
        this->valueField_ = this->ownedField_.get();
        assert(this->valueField_);
        root.updateCycle(this->ownedField_);
      }
    } else {
      if (this->ownedField_) {
        root.shareField(ptr, this->ownedField_);
      }
      pos = this->layoutValueField(root, self, pos, ptr);
    }
    root.popCycle(this->ownedField_);
    return pos;
  }

  template <class U>
  void freeze(
      FreezeRoot& root,
      const std::shared_ptr<U>& ptr,
      FreezePosition self) const {
    if (!ptr) {
      root.freezeField(self, this->offsetField, kOffsetNull);
      return;
    }

    if (auto alreadyFrozen = root.freezePositionOf(ptr.get())) {
      int64_t offset = alreadyFrozen->byteOffset(self);
      root.freezeField(self, this->offsetField, this->encodeOffset(offset));
    } else {
      this->freezeValueField(root, self, ptr.get());
    }
  }

  void thaw(ViewPosition self, std::shared_ptr<T>& out) const {
    if (this->valueField_) {
      int64_t offset;
      thawField(self, this->offsetField, offset);
      if (offset != kOffsetNull) {
        out = std::make_shared<T>();
        thawField(self(this->decodeOffset(offset)), *this->valueField_, *out);
      }
    }
  }

  void thaw(ViewPosition self, std::shared_ptr<const T>& out) const {
    if (this->valueField_) {
      std::shared_ptr<T> tmp;
      thaw(self, tmp);
      out = std::move(tmp);
    }
  }

  template <typename SchemaInfo>
  inline void save(
      typename SchemaInfo::Schema& schema,
      typename SchemaInfo::Layout& layout,
      typename SchemaInfo::Helper& helper) const {
    LayoutBase::template save<SchemaInfo>(schema, layout, helper);
    this->offsetField.template save<SchemaInfo>(schema, layout, helper);
    if (ownedField_) {
      ownedField_->template save<SchemaInfo>(schema, layout, helper);
    }
  }

  template <typename SchemaInfo>
  inline void load(
      const typename SchemaInfo::Schema& schema,
      const typename SchemaInfo::Layout& layout,
      LoadRoot& root) {
    LayoutBase::template load<SchemaInfo>(schema, layout, root);
    this->valueField_ = root.pushCycle(ownedField_, 2, "owned");
    for (const auto& field : layout.getFields()) {
      switch (field.getId()) {
        case 1: {
          this->offsetField.template load<SchemaInfo>(schema, field, root);
          break;
        }
        case 2: {
          CHECK(ownedField_);
          ownedField_->template load<SchemaInfo>(schema, field, root);
          break;
        }
      }
    }
    root.popCycle(ownedField_);
  }

  using View = typename RefLayout<T>::View;

  View view(ViewPosition self) const { return View(this, self); }
};

/**
 * Layout specialization for std::unique_ptr<T> and boxed field.
 */
template <class T>
struct UniqueRefLayout : public RefLayout<T> {
  // Serve the same purpose as the ownedField_ in SharedRefLayout, but define it
  // as unique_ptr to save runtime space
  std::unique_ptr<Field<T>> ownedField_;

  FieldPosition layout(LayoutRoot& root, const T* ptr, LayoutPosition self) {
    FieldPosition pos = this->startFieldPosition();
    if (!ptr) {
      pos = root.layoutField(self, pos, this->offsetField, kOffsetNull);
      return pos;
    }

    this->valueField_ = root.pushCycle(this->ownedField_, 2, "owned");
    if (auto alreadyLayout = root.layoutPositionOf(ptr)) {
      int64_t offset = alreadyLayout->byteOffset(self);
      pos = root.layoutField(
          self, pos, this->offsetField, this->encodeOffset(offset));
    } else {
      pos = this->layoutValueField(root, self, pos, ptr);
    }
    root.popCycle(this->ownedField_);
    return pos;
  }

  template <typename D>
  FieldPosition layout(
      LayoutRoot& root, const std::unique_ptr<T, D>& ptr, LayoutPosition self) {
    return layout(root, ptr.get(), self);
  }

  FieldPosition layout(
      LayoutRoot& root,
      apache::thrift::optional_boxed_field_ref<
          const apache::thrift::detail::boxed_value_ptr<T>&> ref,
      LayoutPosition self) {
    return layout(root, ref ? &*ref : nullptr, self);
  }

  void freeze(FreezeRoot& root, const T* ptr, FreezePosition self) const {
    if (!ptr) {
      root.freezeField(self, this->offsetField, kOffsetNull);
      return;
    }

    if (auto alreadyFrozen = root.freezePositionOf(ptr)) {
      int64_t offset = alreadyFrozen->byteOffset(self);
      root.freezeField(self, this->offsetField, this->encodeOffset(offset));
    } else {
      this->freezeValueField(root, self, ptr);
    }
  }

  template <typename D>
  void freeze(
      FreezeRoot& root,
      const std::unique_ptr<T, D>& ptr,
      FreezePosition self) const {
    freeze(root, ptr.get(), self);
  }

  void freeze(
      FreezeRoot& root,
      apache::thrift::optional_boxed_field_ref<
          const apache::thrift::detail::boxed_value_ptr<T>&> ref,
      FreezePosition self) const {
    freeze(root, ref ? &*ref : nullptr, self);
  }

  template <typename D>
  void thaw(ViewPosition self, std::unique_ptr<T, D>& out) const {
    if (this->valueField_) {
      int64_t offset;
      thawField(self, this->offsetField, offset);
      if (offset != kOffsetNull) {
        out = std::make_unique<T>();
        thawField(self(this->decodeOffset(offset)), *this->valueField_, *out);
      }
    }
  }

  void thaw(
      ViewPosition self,
      optional_boxed_field_ref<apache::thrift::detail::boxed_value_ptr<T>&> ref)
      const {
    if (this->valueField_) {
      int64_t offset;
      thawField(self, this->offsetField, offset);
      if (offset != kOffsetNull) {
        ref.emplace();
        thawField(self(this->decodeOffset(offset)), *this->valueField_, *ref);
      }
    }
  }

  template <typename SchemaInfo>
  inline void save(
      typename SchemaInfo::Schema& schema,
      typename SchemaInfo::Layout& layout,
      typename SchemaInfo::Helper& helper) const {
    LayoutBase::template save<SchemaInfo>(schema, layout, helper);
    this->offsetField.template save<SchemaInfo>(schema, layout, helper);
    if (ownedField_) {
      ownedField_->template save<SchemaInfo>(schema, layout, helper);
    }
  }

  template <typename SchemaInfo>
  inline void load(
      const typename SchemaInfo::Schema& schema,
      const typename SchemaInfo::Layout& layout,
      LoadRoot& root) {
    LayoutBase::template load<SchemaInfo>(schema, layout, root);
    this->valueField_ = root.pushCycle(ownedField_, 2, "owned");
    for (const auto& field : layout.getFields()) {
      switch (field.getId()) {
        case 1: {
          this->offsetField.template load<SchemaInfo>(schema, field, root);
          break;
        }
        case 2: {
          CHECK(ownedField_);
          ownedField_->template load<SchemaInfo>(schema, field, root);
          break;
        }
      }
    }
    root.popCycle(ownedField_);
  }

  using View = typename RefLayout<T>::View;

  View view(ViewPosition self) const { return View(this, self); }
};
} // namespace detail

template <class T>
struct Layout<std::shared_ptr<T>>
    : public apache::thrift::frozen::detail::SharedRefLayout<T> {};

template <class T>
struct Layout<std::shared_ptr<const T>>
    : public apache::thrift::frozen::detail::SharedRefLayout<T> {};

template <class T, class D>
struct Layout<
    std::unique_ptr<T, D>,
    std::enable_if_t<!std::is_same<T, folly::IOBuf>::value>>
    : public apache::thrift::frozen::detail::UniqueRefLayout<T> {};

// Re-use UniqueRefLayout for boxed fields, as
// apache::thrift::detail::boxed_value_ptr simply wraps around std::unique_ptr.
template <class T>
struct Layout<apache::thrift::detail::boxed_value_ptr<T>>
    : public apache::thrift::frozen::detail::UniqueRefLayout<T> {};

} // namespace frozen
} // namespace thrift
} // namespace apache
