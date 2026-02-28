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

template <
    class T,
    class = typename std::enable_if<std::is_integral<T>::value>::type>
size_t bitsNeeded(const T& x) {
  using UT = typename std::make_unsigned<T>::type;
  auto ux = static_cast<UT>(x);
  return ux
      ? folly::findLastSet(
            std::is_signed<T>::value ? static_cast<UT>(ux ^ (ux << 1)) : ux)
      : 0;
}

/**
 * Specialized layout for packing integers by truncating unused high bits.
 */
template <class T>
struct PackedIntegerLayout : public LayoutBase {
  typedef LayoutBase Base;
  PackedIntegerLayout() : LayoutBase(typeid(T)) {}
  explicit PackedIntegerLayout(const std::type_info& _type)
      : LayoutBase(_type) {}

  FieldPosition maximize() {
    FieldPosition pos = startFieldPosition();
    pos.bitOffset += sizeof(T) * 8;
    return pos;
  }

  FieldPosition layout(LayoutRoot&, const T& x, LayoutPosition /* self */) {
    FieldPosition pos = startFieldPosition();
    if (x) {
      pos.bitOffset += bitsNeeded(x);
    }
    return pos;
  }

  void freeze(FreezeRoot&, const T& x, FreezePosition self) const {
    if (bitsNeeded(x) > bits) {
      throw LayoutException();
    }
    if (!bits) {
      return;
    }
    folly::Bits<folly::Unaligned<T>>::set(
        reinterpret_cast<folly::Unaligned<T>*>(self.start),
        self.bitOffset,
        bits,
        x);
  }

  void thaw(ViewPosition self, T& out) const {
    if (!bits) {
      out = 0;
      return;
    }
    out = folly::Bits<folly::Unaligned<T>>::get(
        reinterpret_cast<const folly::Unaligned<T>*>(self.start),
        self.bitOffset,
        this->bits);
  }

  void print(std::ostream& os, int level) const override {
    LayoutBase::print(os, level);
    os << "packed " << (std::is_signed<T>::value ? "signed" : "unsigned") << " "
       << folly::demangle(type.name());
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
struct Layout<T, typename std::enable_if<std::is_integral<T>::value>::type>
    : apache::thrift::frozen::detail::PackedIntegerLayout<T> {};
} // namespace frozen
} // namespace thrift
} // namespace apache
