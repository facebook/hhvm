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

namespace detail {

/**
 * Layout specialization a pair of values, layed out as simple fields.
 */
template <class First, class Second>
struct PairLayout : public LayoutBase {
  typedef LayoutBase Base;
  typedef std::pair<First, Second> T;
  typedef PairLayout LayoutSelf;
  typedef typename std::decay<First>::type FirstDecayed;
  typedef typename std::decay<Second>::type SecondDecayed;
  Field<FirstDecayed> firstField;
  Field<SecondDecayed> secondField;

  PairLayout()
      : LayoutBase(typeid(T)),
        firstField(1, "first"),
        secondField(2, "second") {}

  FieldPosition maximize() {
    FieldPosition pos = startFieldPosition();
    FROZEN_MAXIMIZE_FIELD(first);
    FROZEN_MAXIMIZE_FIELD(second);
    return pos;
  }

  FieldPosition layout(
      LayoutRoot& root,
      const std::pair<First, Second>& o,
      LayoutPosition self) {
    FieldPosition pos = startFieldPosition();
    pos = root.layoutField(self, pos, firstField, o.first);
    pos = root.layoutField(self, pos, secondField, o.second);
    return pos;
  }

  void freeze(
      FreezeRoot& root,
      const std::pair<First, Second>& o,
      FreezePosition self) const {
    root.freezeField(self, firstField, o.first);
    root.freezeField(self, secondField, o.second);
  }

  void thaw(ViewPosition self, std::pair<First, Second>& out) const {
    thawField(self, firstField, const_cast<FirstDecayed&>(out.first));
    thawField(self, secondField, const_cast<SecondDecayed&>(out.second));
  }

  FROZEN_VIEW(FROZEN_VIEW_FIELD(first, FirstDecayed)
                  FROZEN_VIEW_FIELD(second, SecondDecayed))

  void print(std::ostream& os, int level) const final {
    LayoutBase::print(os, level);
    os << folly::demangle(type.name());
    firstField.print(os, level + 1);
    secondField.print(os, level + 1);
  }

  void clear() final {
    firstField.clear();
    secondField.clear();
  }

  FROZEN_SAVE_INLINE(FROZEN_SAVE_FIELD(first) FROZEN_SAVE_FIELD(second))

  FROZEN_LOAD_INLINE(FROZEN_LOAD_FIELD(first, 1) FROZEN_LOAD_FIELD(second, 2))
};

} // namespace detail

template <class First, class Second>
struct Layout<std::pair<First, Second>>
    : public apache::thrift::frozen::detail::PairLayout<First, Second> {};

} // namespace frozen
} // namespace thrift
} // namespace apache
