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

class FOLLY_EXPORT FixedSizeMismatchException : public std::length_error {
 public:
  FixedSizeMismatchException(size_t expected, size_t actual)
      : std::length_error(folly::to<std::string>(
            "Size mismatch. FixedSizeString specifies ",
            expected,
            ", actual size is ",
            actual)) {}
};

/**
 * Serializes a string blob with a fixed size. Similar to TrivialLayout, but
 * uses folly::ByteRange as the view. During freezing, an exception will be
 * thrown if the actual size doesn't match what's specified in the IDL schema.
 */
template <typename T>
struct FixedSizeStringLayout : public LayoutBase {
  using Base = LayoutBase;
  FixedSizeStringLayout() : LayoutBase(typeid(T)) {}

  FieldPosition maximize() { return FieldPosition(T::kFixedSize, 0); }

  FieldPosition layout(LayoutRoot&, const T&, LayoutPosition /* start */) {
    return maximize();
  }

  void freeze(FreezeRoot&, const T& o, FreezePosition self) const {
    if (o.size() == T::kFixedSize) {
      memcpy(self.start, o.data(), o.size());
    } else {
      throw FixedSizeMismatchException(T::kFixedSize, o.size());
    }
  }

  void thaw(ViewPosition self, T& out) const {
    if (size == T::kFixedSize) {
      out.resize(T::kFixedSize);
      memcpy(&out[0], self.start, T::kFixedSize);
    }
  }

  void print(std::ostream& os, int level) const override {
    LayoutBase::print(os, level);
    os << folly::demangle(type.name());
  }

  struct View : public folly::ByteRange {
   public:
    using folly::ByteRange::ByteRange;

    View(folly::ByteRange bytes) : folly::ByteRange(bytes) {}

    bool operator==(View rhs) {
      return memcmp(this->data(), rhs.data(), T::kFixedSize) == 0;
    }

    using folly::ByteRange::toString;
  };

  View view(ViewPosition self) const { return View(self.start, T::kFixedSize); }

  static size_t hash(const T& value) {
    return FixedSizeStringHash<T::kFixedSize, T>::hash(value);
  }

  static size_t hash(const View& value) {
    return FixedSizeStringHash<T::kFixedSize, View>::hash(value);
  }
};

} // namespace detail

template <size_t kSize>
class FixedSizeString;

template <size_t kSize>
struct Layout<FixedSizeString<kSize>>
    : detail::FixedSizeStringLayout<FixedSizeString<kSize>> {};
} // namespace frozen
} // namespace thrift
} // namespace apache
