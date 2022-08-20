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
 * Layout specialization for enum values, simply delegates to Integer layout.
 */
template <
    class T,
    class Underlying = typename std::enable_if<
        std::is_enum<T>::value,
        typename std::underlying_type<T>::type>::type>
struct EnumLayout : public PackedIntegerLayout<Underlying> {
  typedef PackedIntegerLayout<Underlying> Base;
  EnumLayout() : Base(typeid(T)) {}

  FieldPosition maximize() { return Base::maximize(); }

  FieldPosition layout(LayoutRoot& root, const T& o, LayoutPosition self) {
    return Base::layout(root, static_cast<Underlying>(o), self);
  }

  void freeze(FreezeRoot& root, const T& o, FreezePosition self) const {
    Base::freeze(root, static_cast<Underlying>(o), self);
  }

  void thaw(ViewPosition self, T& out) const {
    Underlying x;
    Base::thaw(self, x);
    out = T(x);
  }

  void print(std::ostream& os, int level) const override {
    Base::print(os, level);
    os << " as enum " << folly::demangle(this->type.name());
  }

  typedef T View;

  View view(ViewPosition self) const {
    View v;
    thaw(self, v);
    return v;
  }

  static size_t hash(const T& v) { return std::hash<T>()(v); }
};
} // namespace detail

template <class T>
struct Layout<T, typename std::enable_if<std::is_enum<T>::value>::type>
    : public apache::thrift::frozen::detail::EnumLayout<T> {};
} // namespace frozen
} // namespace thrift
} // namespace apache
