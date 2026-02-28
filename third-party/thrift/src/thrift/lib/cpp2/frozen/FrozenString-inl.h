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

namespace folly {
class IOBuf;
}

namespace apache {
namespace thrift {
namespace frozen {

namespace detail {

template <class T>
struct BufferHelpers {
  typedef typename T::value_type Item;
  static_assert(
      std::is_arithmetic<Item>::value || std::is_enum<Item>::value,
      "String storage requires simple item types");
  static size_t size(const T& src) { return src.size(); }
  static void copyTo(const T& src, folly::Range<Item*> dst) {
    std::copy(src.begin(), src.end(), reinterpret_cast<Item*>(dst.begin()));
  }
  static void thawTo(folly::Range<const Item*> src, T& dst) {
    dst.assign(src.begin(), src.end());
  }
};

template <>
struct BufferHelpers<std::unique_ptr<folly::IOBuf>> {
  typedef uint8_t Item;
  static size_t size(const std::unique_ptr<folly::IOBuf>& src);

  static void copyTo(
      const std::unique_ptr<folly::IOBuf>& src, folly::MutableByteRange dst);
  static void thawTo(folly::ByteRange src, std::unique_ptr<folly::IOBuf>& dst);
};

template <>
struct BufferHelpers<folly::IOBuf> {
  typedef uint8_t Item;
  static size_t size(const folly::IOBuf& src);

  static void copyTo(const folly::IOBuf& src, folly::MutableByteRange dst);
  static void thawTo(folly::ByteRange src, folly::IOBuf& dst);
};

/**
 * for contiguous, blittable ranges
 */
template <class T>
struct StringLayout : public LayoutBase {
  typedef LayoutBase Base;
  typedef BufferHelpers<T> Helper;
  typedef typename Helper::Item Item;
  Field<size_t> distanceField;
  Field<size_t> countField;

  StringLayout()
      : LayoutBase(typeid(T)),
        distanceField(1, "distance"),
        countField(2, "count") {}

  FieldPosition maximize() {
    FieldPosition pos = startFieldPosition();
    FROZEN_MAXIMIZE_FIELD(distance);
    FROZEN_MAXIMIZE_FIELD(count);
    return pos;
  }

  FieldPosition layout(LayoutRoot& root, const T& o, LayoutPosition self) {
    FieldPosition pos = startFieldPosition();
    size_t n = Helper::size(o);
    if (!n) {
      return pos;
    }
    size_t dist =
        root.layoutBytesDistance(self.start, n * sizeof(Item), alignof(Item));
    pos = root.layoutField(self, pos, distanceField, dist);
    pos = root.layoutField(self, pos, countField, n);
    return pos;
  }

  void freeze(FreezeRoot& root, const T& o, FreezePosition self) const {
    size_t n = Helper::size(o);
    folly::MutableByteRange range;
    size_t dist;
    root.appendBytes(self.start, n * sizeof(Item), range, dist, alignof(Item));
    root.freezeField(self, distanceField, dist);
    root.freezeField(self, countField, n);
    folly::Range<Item*> target(reinterpret_cast<Item*>(range.begin()), n);
    Helper::copyTo(o, target);
  }

  void thaw(ViewPosition self, T& out) const {
    Helper::thawTo(view(self), out);
  }

  typedef folly::Range<const Item*> View;

  View view(ViewPosition self) const {
    View range;
    size_t dist, n;
    thawField(self, countField, n);
    if (n) {
      thawField(self, distanceField, dist);
      const byte* read = self.start + dist;
      range.reset(reinterpret_cast<const Item*>(read), n);
    }
    return range;
  }

  void print(std::ostream& os, int level) const override {
    LayoutBase::print(os, level);
    os << "string of " << folly::demangle(type.name());
    distanceField.print(os, level + 1);
    countField.print(os, level + 1);
  }

  void clear() final {
    LayoutBase::clear();
    distanceField.clear();
    countField.clear();
  }

  FROZEN_SAVE_INLINE(FROZEN_SAVE_FIELD(distance) FROZEN_SAVE_FIELD(count))

  FROZEN_LOAD_INLINE(FROZEN_LOAD_FIELD(distance, 1) FROZEN_LOAD_FIELD(count, 2))

  static size_t hash(const View& v) {
    return folly::hash::fnv64_buf_BROKEN(v.begin(), sizeof(Item) * v.size());
  }
};

} // namespace detail

template <class T>
struct Layout<T, typename std::enable_if<IsString<T>::value>::type>
    : apache::thrift::frozen::detail::StringLayout<
          typename std::decay<T>::type> {};

} // namespace frozen
} // namespace thrift
} // namespace apache

THRIFT_DECLARE_TRAIT(IsString, std::string)
THRIFT_DECLARE_TRAIT(IsString, folly::fbstring)
THRIFT_DECLARE_TRAIT(IsString, std::unique_ptr<folly::IOBuf>)
THRIFT_DECLARE_TRAIT(IsString, folly::IOBuf)
