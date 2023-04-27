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

class LayoutExcludedException : public std::runtime_error {
 public:
  LayoutExcludedException()
      : std::runtime_error(
            "This type was excluded from layout, but present in input") {}
};

namespace detail {

/**
 * Layout specialization for types excluded from freezing. Values must never be
 * set, otherwise layout/freeze will throw.
 */
template <class T>
struct ExcludedLayout : public LayoutBase {
  typedef LayoutBase Base;
  ExcludedLayout() : Base(typeid(T)) {}

  FieldPosition maximize() { return startFieldPosition(); }

  FieldPosition layout(LayoutRoot&, const T&, LayoutPosition) {
    throw LayoutExcludedException();
  }

  void freeze(FreezeRoot&, const T&, FreezePosition) const {
    throw LayoutExcludedException();
  }

  void thaw(ViewPosition, T&) const { throw LayoutExcludedException(); }

  void print(std::ostream& os, int level) const override {
    Base::print(os, level);
    os << " excluded " << folly::demangle(this->type.name());
  }

  typedef T View;

  T view(ViewPosition) const { throw LayoutExcludedException(); }
};
} // namespace detail

template <class T>
struct Layout<T, typename std::enable_if<IsExcluded<T>::value>::type>
    : public apache::thrift::frozen::detail::ExcludedLayout<T> {};

} // namespace frozen
} // namespace thrift
} // namespace apache
