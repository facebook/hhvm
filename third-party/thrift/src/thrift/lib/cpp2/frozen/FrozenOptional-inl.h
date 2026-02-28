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

namespace apache {
namespace thrift {
namespace frozen {

/**
 * A view of an optional Frozen field. It provides a std::optional-like API
 * and intentionally disallows folly::Optional extensions.
 */
template <typename T>
class OptionalFieldView : private folly::Optional<T> {
 private:
  const folly::Optional<T>& toFolly() const { return *this; }

 public:
  using folly::Optional<T>::operator->;
  using folly::Optional<T>::operator*;
  using folly::Optional<T>::operator bool;
  using folly::Optional<T>::has_value;
  using folly::Optional<T>::value;
  using folly::Optional<T>::value_or;
  using folly::Optional<T>::reset;
  using folly::Optional<T>::emplace;

  template <typename L, typename R>
  friend bool operator==(
      const OptionalFieldView<L>& lhs, const OptionalFieldView<R>& rhs);
  template <typename L, typename R>
  friend bool operator!=(
      const OptionalFieldView<L>& lhs, const OptionalFieldView<R>& rhs);

  template <typename L, typename R>
  friend bool operator==(const L& lhs, const OptionalFieldView<R>& rhs);
  template <typename L, typename R>
  friend bool operator==(const OptionalFieldView<L>& lhs, const R& rhs);
  template <typename L, typename R>
  friend bool operator!=(const L& lhs, const OptionalFieldView<R>& rhs);
  template <typename L, typename R>
  friend bool operator!=(const OptionalFieldView<L>& lhs, const R& rhs);

  template <typename U>
  friend bool operator==(
      const OptionalFieldView<U>& lhs, const folly::Optional<U>& rhs);
  template <typename U>
  friend bool operator==(
      const folly::Optional<U>& lhs, const OptionalFieldView<U>& rhs);
};

template <typename L, typename R>
bool operator==(
    const OptionalFieldView<L>& lhs, const OptionalFieldView<R>& rhs) {
  return lhs.toFolly() == rhs.toFolly();
}
template <typename L, typename R>
bool operator!=(
    const OptionalFieldView<L>& lhs, const OptionalFieldView<R>& rhs) {
  return lhs.toFolly() != rhs.toFolly();
}

template <typename L, typename R>
bool operator==(const L& lhs, const OptionalFieldView<R>& rhs) {
  return lhs == rhs.toFolly();
}
template <typename L, typename R>
bool operator==(const OptionalFieldView<L>& lhs, const R& rhs) {
  return lhs.toFolly() == rhs;
}
template <typename L, typename R>
bool operator!=(const L& lhs, const OptionalFieldView<R>& rhs) {
  return lhs != rhs.toFolly();
}
template <typename L, typename R>
bool operator!=(const OptionalFieldView<L>& lhs, const R& rhs) {
  return lhs.toFolly() != rhs;
}

template <typename U>
[[deprecated("comparison with folly::Optional is deprecated")]] bool operator==(
    const OptionalFieldView<U>& lhs, const folly::Optional<U>& rhs) {
  return lhs.toFolly() == rhs;
}
template <typename U>
[[deprecated("comparison with folly::Optional is deprecated")]] bool operator==(
    const folly::Optional<U>& lhs, const OptionalFieldView<U>& rhs) {
  return lhs == rhs.toFolly();
}

namespace detail {

/**
 * Layout specialization for Optional<T>, which is used in the codegen for
 * optional fields. Just a boolean and a value.
 */
template <class T>
struct OptionalLayout : public LayoutBase {
  typedef LayoutBase Base;
  Field<bool> issetField;
  Field<T> valueField;

  OptionalLayout()
      : LayoutBase(typeid(T)), issetField(1, "isset"), valueField(2, "value") {}

  FieldPosition maximize() {
    FieldPosition pos = startFieldPosition();
    FROZEN_MAXIMIZE_FIELD(isset);
    FROZEN_MAXIMIZE_FIELD(value);
    return pos;
  }

  FieldPosition layout(
      LayoutRoot& root, const folly::Optional<T>& o, LayoutPosition self) {
    FieldPosition pos = startFieldPosition();
    pos = root.layoutField(self, pos, issetField, o.hasValue());
    if (o) {
      pos = root.layoutField(self, pos, valueField, o.value());
    }
    return pos;
  }

  FieldPosition layout(LayoutRoot& root, const T& o, LayoutPosition self) {
    FieldPosition pos = startFieldPosition();
    pos = root.layoutField(self, pos, issetField, true);
    pos = root.layoutField(self, pos, valueField, o);
    return pos;
  }

  void freeze(
      FreezeRoot& root,
      const folly::Optional<T>& o,
      FreezePosition self) const {
    root.freezeField(self, issetField, o.hasValue());
    if (o) {
      root.freezeField(self, valueField, o.value());
    }
  }

  void freeze(FreezeRoot& root, const T& o, FreezePosition self) const {
    root.freezeField(self, issetField, true);
    root.freezeField(self, valueField, o);
  }

  void thaw(ViewPosition self, folly::Optional<T>& out) const {
    bool set;
    thawField(self, issetField, set);
    if (set) {
      out.emplace();
      thawField(self, valueField, out.value());
    }
  }

  using View = OptionalFieldView<typename Layout<T>::View>;

  View view(ViewPosition self) const {
    View v;
    bool set;
    thawField(self, issetField, set);
    if (set) {
      v.emplace(valueField.layout.view(self(valueField.pos)));
    }
    return v;
  }

  void print(std::ostream& os, int level) const final {
    LayoutBase::print(os, level);
    os << "optional " << folly::demangle(type.name());
    issetField.print(os, level + 1);
    valueField.print(os, level + 1);
  }

  void clear() final {
    issetField.clear();
    valueField.clear();
  }

  FROZEN_SAVE_INLINE(FROZEN_SAVE_FIELD(isset) FROZEN_SAVE_FIELD(value))

  FROZEN_LOAD_INLINE(FROZEN_LOAD_FIELD(isset, 1) FROZEN_LOAD_FIELD(value, 2))
};
} // namespace detail

template <class T>
struct Layout<folly::Optional<T>>
    : public apache::thrift::frozen::detail::OptionalLayout<T> {};

} // namespace frozen
} // namespace thrift
} // namespace apache
