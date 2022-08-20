/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef THRIFT_LIB_CPP_FROZEN_H_
#define THRIFT_LIB_CPP_FROZEN_H_

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <glog/logging.h>
#include <folly/FBString.h>
#include <folly/Range.h>
#include <folly/lang/Bits.h>
#include <thrift/lib/cpp/RelativePtr.h>

/**
 * Frozen Type support
 *
 * @author Tom Jackson (tjackson@fb.com)
 *
 * This library implements support for frozen Thrift structures. These
 * structures allow a mutable value of a given type to be frozen into a
 * contiguous, relocatable section of memory. This section of memory may then
 * be written to disk, mmap'ed back, and used in-place without any
 * deserialization.
 *
 * These types require that the the structure does not change between freeze and
 * thaw. Any versioning mismatches will lead to undefined behavior.
 *
 * To enable the necessary code generation for these types, enable the 'frozen'
 * option when invoking the Thrift compiler for C++. Note that any custom
 * cpp.type overrides must be accompanied with a specialization of Freezer<T>
 * to enable Freezing values.
 *
 * This library works by recursively flattening data structures into fixed size
 * structures with buffers allocated immediately after each structure. These
 * buffers are addressed by relative pointers, enabling relocatability.
 *
 * For each struct type, T, the Thrift compiler will generate specializations of
 * the following template types:
 *
 *   apache::thrift::Frozen<T>  - The frozen counterpart to the mutable type T.
 *   apache::thrift::Freezer<T> - The freezer for T, a helper for freezing,
 *                                thawing, and sizing instances of T.
 *
 * See below for details on Frozen<T> and Freezer<T>, and for more information
 * see 'Thrift/Frozen' on the wiki.
 */
namespace apache {
namespace thrift {

/**
 * Frozen<T> - This is simply a canonical type name for naming the frozen
 * counterpart for a given mutable type.
 */
template <class T, class = void>
struct Frozen;

/**
 * Freezer<T> - The helper class which encapsulates the types and methods for
 * mapping from T and its instances to its frozen counterparts and its
 * instances.
 */
template <class T, class = void>
struct Freezer {
  // The type which a frozen value thaws back into, usually just T.
  typedef T ThawedType;

  // The type which a mutable value is frozen into. Here, it is simply T. For
  // complex data types, this class will be specialized and a different type
  // will be provided, usually Frozen<T>.
  typedef Frozen<T> FrozenType;

  // Compute the amount of out-of-struct space needed to represent the given
  // object. Specializations of this class will recurse down to children,
  // summing up their extraSize results recursively.
  static size_t extraSizeImpl(const ThawedType& src);

  // Freeze 'src' into 'dst' recursively, using 'buffer' for additional storage.
  // 'buffer' must be advanced by specializations of this class if the spare
  // space is used.
  static void freezeImpl(const ThawedType& src, FrozenType& dst, byte*& buffer);

  // Thaw 'src' into 'dst' recursively. Specializations of this class wil
  // recurse to each field, thawing each of their fields recursively.
  static void thawImpl(const FrozenType& src, ThawedType& dst);
};

/**
 * TrivialFreezer<T> - Freezer for POD types, which are simply copied.
 */
template <class T>
struct TrivialFreezer {
  // The type which a frozen value thaws back into, usually just T.
  typedef T ThawedType;

  // The type which a mutable value is frozen into. Here, it is simply T. For
  // complex data types, this class will be specialized and a different type
  // will be provided, usually Frozen<T>.
  typedef T FrozenType;

  // Compute the amount of out-of-struct space needed to represent the given
  // object. Specializations of this class will recurse down to children,
  // summing up their extraSize results recursively.
  static constexpr size_t extraSizeImpl(const ThawedType& /* src */) {
    return 0;
  }

  // Freeze 'src' into 'dst' recursively, using 'buffer' for additional storage.
  // 'buffer' must be advanced by specializations of this class if the spare
  // space is used.
  static void freezeImpl(
      const ThawedType& src, FrozenType& dst, byte*& /* buffer */) {
    dst = src;
  }

  // Thaw 'src' into 'dst' recursively. Specializations of this class wil
  // recurse to each field, thawing each of their fields recursively.
  static void thawImpl(const FrozenType& src, ThawedType& dst) { dst = src; }
};

/**
 * Freezer<POD> - Use trivial Freezer.
 */
template <class T>
struct Freezer<
    T,
    typename std::enable_if<
        std::is_pod<T>::value && !std::is_const<T>::value>::type>
    : TrivialFreezer<T> {};

/**
 * Freezer specialization for const T. Primarily a pass-through to Freezer<T>,
 * but const-ness removed to enable thawing.
 */
template <class T>
struct Freezer<const T, void> : Freezer<T, void> {};

/**
 * extraSize - Dispatch to Freezer<T>'s extraSizeImpl to calculate addition
 * space needed for 'src'.
 */
template <class T>
size_t extraSize(const T& src) {
  return Freezer<T>::extraSizeImpl(src);
}

template <class T>
T unaligned_ptr_cast(void* ptr) {
  return static_cast<T>(ptr);
}

template <class T>
const T unaligned_ptr_cast(const void* ptr) {
  return static_cast<const T>(ptr);
}

namespace frzn_dtl {

// frozen size of implementation //
template <typename T>
struct z {
  static constexpr inline std::size_t size() { return sizeof(T); }
  static constexpr inline std::size_t alignment() { return alignof(T); }
};

} // namespace frzn_dtl

template <typename T>
using FrozenSizeOf = frzn_dtl::z<typename std::decay<T>::type>;

/**
 * frozenSize - Total space needed to store a frozen representation of 'src'.
 */
template <class T>
size_t frozenSize(const T& src) {
  return FrozenSizeOf<typename Freezer<T>::FrozenType>::size() + extraSize(src);
}

template <typename T>
const Frozen<T>& frozenView(const void* blob) {
  return *reinterpret_cast<const Frozen<T>*>(blob);
}

/**
 * freeze(...) - Freeze 'src' into 'dst', consuming 'buffer' for extra space.
 */
template <class T, class FrozenType = typename Freezer<T>::FrozenType>
void freeze(const T& src, FrozenType& dst, byte*& buffer) {
  Freezer<T>::freezeImpl(src, dst, buffer);
}

/**
 * freeze(...) - Freeze 'src' by consuming memory pointed to by 'buffer'.
 */
template <class T, class FrozenType = typename Freezer<T>::FrozenType>
const FrozenType* freeze(const T& src, byte*& buffer) {
  // NOTE(tjackson): This pointer will not necessarily be aligned with
  //                 alignof(FrozenType).
  FrozenType* frozen = unaligned_ptr_cast<FrozenType*>(buffer);
  buffer += FrozenSizeOf<FrozenType>::size();
  Freezer<T>::freezeImpl(src, *frozen, buffer);
  return frozen;
}

struct FrozenTypeDeleter {
  void operator()(const void* ptr) const { free(const_cast<void*>(ptr)); }
};

template <class T, class FrozenType = typename Freezer<T>::FrozenType>
using FrozenTypeUPtr = std::unique_ptr<const FrozenType, FrozenTypeDeleter>;

// Enables disambiguated calls to freeze(), which also exists in frozen2
enum class Frozen1 { Marker };

/**
 * freeze(...) - Freeze 'src' into a newly-allocated buffer owned by a
 * unique_ptr.
 */
template <class T, class FrozenType = typename Freezer<T>::FrozenType>
FrozenTypeUPtr<T, FrozenType> freeze(const T& src, Frozen1 = Frozen1::Marker) {
  // find how much space we need
  size_t size = frozenSize(src);
  // allocate it
  byte* memory = static_cast<byte*>(malloc(size));
  byte* buffer = memory;
  byte* finish = buffer + size;

  // NOTE(tjackson): This pointer will not necessarily be aligned with
  //                 alignof(FrozenType).
  FrozenType* frozen = unaligned_ptr_cast<FrozenType*>(memory);
  FrozenTypeUPtr<T, FrozenType> ret(frozen);

  buffer += FrozenSizeOf<FrozenType>::size();
  // start populating the object graph, starting from 'src' with spare storage
  // allocated at 'buffer'.
  freeze(src, *frozen, buffer);
  DCHECK_EQ(buffer, finish);
  return ret;
}

/**
 * thaw(...) - Thaw a given frozen into its mutable counterpart.
 */
template <class ThawedType>
void thaw(
    const typename Freezer<ThawedType>::FrozenType& frozen,
    ThawedType& thawed) {
  Freezer<ThawedType>::thawImpl(frozen, thawed);
}

/**
 * thaw(...) - Thaw a given frozen into its mutable counterpart.
 */
template <class ThawedType>
ThawedType thaw(const Frozen<ThawedType>& frozen) {
  ThawedType thawed;
  Freezer<ThawedType>::thawImpl(frozen, thawed);
  return thawed;
}

/**
 * Freezer<std::pair<A, B>> - Freezer for pairs of values, frozen as
 * std::pair<Frozen<A>, Frozen<B>> with the help of Freezer<A> and Freezer<B>.
 */
template <class A, class B>
struct Freezer<std::pair<A, B>> {
  typedef typename Freezer<A>::ThawedType ThawedFirst;
  typedef typename Freezer<B>::ThawedType ThawedSecond;
  typedef std::pair<ThawedFirst, ThawedSecond> ThawedType;
  typedef typename Freezer<A>::FrozenType FrozenFirst;
  typedef typename Freezer<B>::FrozenType FrozenSecond;
  typedef std::pair<FrozenFirst, FrozenSecond> FrozenType;

  static constexpr size_t extraSizeImpl(const ThawedType& src) {
    return Freezer<A>::extraSizeImpl(src.first) +
        Freezer<B>::extraSizeImpl(src.second);
  }

  static void freezeImpl(
      const ThawedType& src, FrozenType& dst, byte*& buffer) {
    Freezer<A>::freezeImpl(src.first, dst.first, buffer);
    Freezer<B>::freezeImpl(src.second, dst.second, buffer);
  }

  static void thawImpl(const FrozenType& src, ThawedType& dst) {
    Freezer<A>::thawImpl(src.first, dst.first);
    Freezer<B>::thawImpl(src.second, dst.second);
  }
};

namespace frzn_dtl {

// implementation of `FrozenIterator` //
template <typename FrozenItem>
struct t {
  using type = const FrozenItem*;
};

} // namespace frzn_dtl

template <typename FrozenItem>
using FrozenIterator = typename frzn_dtl::t<FrozenItem>::type;

/**
 * FrozenRange<...> - Represents a range of contiguous, frozen values pointed to
 * by relative pointers. Assists in the freezing of vectors, sets, strings, and
 * maps.
 *
 * The memory pointed to by FrozenRange is usually located at addresses just
 * higher than the address of the FrozenRange itself.
 */
template <
    class ThawedItem,
    class FrozenItem = typename Freezer<ThawedItem>::FrozenType>
struct FrozenRange {
  typedef const FrozenItem value_type;
  typedef FrozenIterator<FrozenItem> iterator;
  typedef FrozenIterator<FrozenItem> const_iterator;

  FrozenRange() : begin_(nullptr), end_(nullptr) {}

  FrozenRange(const_iterator begin, const_iterator end)
      : begin_(begin), end_(end) {}

  const_iterator begin() const { return const_iterator(begin_.get()); }
  const_iterator end() const { return const_iterator(end_.get()); }

  const value_type& front() const {
    if (size() == 0) {
      throw std::out_of_range("range is empty");
    }
    return this->begin()[0];
  }

  const value_type& back() const {
    if (size() == 0) {
      throw std::out_of_range("range is empty");
    }
    return this->end()[-1];
  }

  size_t size() const { return end() - begin(); }
  bool empty() const { return end_.get() == begin_.get(); }

  const value_type& operator[](int i) const { return *(begin() + i); }

  template <class T>
  bool operator<(const FrozenRange<T>& other) const {
    return range() < other.range();
  }

  template <class T>
  bool operator>(const FrozenRange<T>& other) const {
    return range() > other.range();
  }

  template <class T>
  bool operator==(const FrozenRange<T>& other) const {
    return range() == other.range();
  }

  bool operator<(folly::StringPiece other) const { return range() < other; }

  bool operator>(folly::StringPiece other) const { return range() > other; }

  bool operator==(folly::StringPiece other) const { return range() == other; }

  folly::Range<const_iterator> range() const {
    return folly::Range<const_iterator>(this->begin(), this->end());
  }

  void reset(const_iterator begin, const_iterator end) {
    begin_.reset(begin);
    end_.reset(end);
  }

  void clear() {
    begin_.reset();
    end_.reset();
  }

 private:
  RelativePtr<const value_type> begin_, end_;
};

namespace detail {

template <class T>
struct IsFrozenRange : std::false_type {};

template <class ThawedItem, class FrozenItem>
struct IsFrozenRange<FrozenRange<ThawedItem, FrozenItem>> : std::true_type {};

} // namespace detail

/**
 * Operator overloads to facilitate comparison and sorting of FrozenRange's.
 * Primarily needed for inclusion in std::map's.
 */
template <class ThawedString>
bool operator==(const FrozenRange<char, ThawedString>& str, const char* cstr) {
  return str == folly::StringPiece(cstr);
}

template <class ThawedString>
bool operator==(const char* cstr, FrozenRange<char, ThawedString>& str) {
  return str == folly::StringPiece(cstr);
}

template <class ThawedString>
bool operator<(const FrozenRange<char, ThawedString>& str, const char* cstr) {
  return str < folly::StringPiece(cstr);
}

template <class ThawedString>
bool operator<(const char* cstr, const FrozenRange<char, ThawedString>& str) {
  return folly::StringPiece(cstr) < str;
}

template <
    class ThawedItem,
    class FrozenItem,
    class Range,
    class = decltype(std::declval<Range>().begin())>
typename std::
    enable_if<!apache::thrift::detail::IsFrozenRange<Range>::value, bool>::type
    operator<(
        const Range& range, const FrozenRange<ThawedItem, FrozenItem>& frozen) {
  return frozen > range;
}

template <
    class ThawedItem,
    class FrozenItem,
    class Range,
    class = decltype(std::declval<Range>().begin())>
typename std::
    enable_if<!apache::thrift::detail::IsFrozenRange<Range>::value, bool>::type
    operator==(
        const Range& range, const FrozenRange<ThawedItem, FrozenItem>& frozen) {
  return range.size() == frozen.size() &&
      std::equal(range.begin(), range.end(), frozen.begin());
}

template <class Ostream, class ThawedItem, class FrozenItem>
Ostream& operator<<(
    Ostream& os, const FrozenRange<ThawedItem, FrozenItem>& frozen) {
  for (auto& item : frozen) {
    os << item;
  }
  return os;
}

/**
 * RangeFreezer - Helper type for freezing range-like types.
 */
template <
    class ThawedItem,
    class Container,
    class FrozenItem = typename Freezer<ThawedItem>::FrozenType>
struct RangeFreezer {
  typedef Container ThawedType;
  typedef FrozenRange<ThawedItem> FrozenType;

  static size_t extraSizeImpl(const ThawedType& src) {
    size_t size = 0;
    // Extra space needed is the sum of the extra space needed for each of the
    // items in the source range.
    for (auto& item : src) {
      size += frozenSize(item);
    }
    return size;
  }

  static void freezeImpl(
      const ThawedType& src, FrozenType& dst, byte*& buffer) {
    size_t size = src.size();
    if (!size) {
      // point to [nullptr, nullptr) if the range is empty.
      dst.reset(nullptr, nullptr);
      return;
    }
    FrozenItem* begin = unaligned_ptr_cast<FrozenItem*>(buffer);
    FrozenItem* end = begin + size;
    buffer = unaligned_ptr_cast<byte*>(end);
    dst.reset(begin, end);
    // freeze each of the items into their pre-allocated slot, using bytes
    // pointed to by 'buffer' for storing variable-sized content.
    for (auto& item : src) {
      freeze(item, *begin++, buffer);
    }
  }

  static void thawImpl(const FrozenType& src, ThawedType& dst) {
    dst.resize(src.size());
    uint32_t i = 0;
    // Thaw each of the items in-place into elements of the output type.
    for (auto& item : src) {
      thaw(item, dst[i++]);
    }
  }
};

/**
 * FrozenMap<...> - Wraps a sorted FrozenRange<pair<K, V>> with the expected
 * interface for a map. find() and others are templetized to allow searching for
 * values of any type which may be compared to the key, though may not be of
 * type K.
 */
template <class K, class V>
struct FrozenMap : FrozenRange<std::pair<const K, V>> {
  typedef typename Freezer<K>::FrozenType key_type;
  typedef typename Freezer<V>::FrozenType mapped_type;
  typedef std::pair<key_type, mapped_type> value_type;
  typedef FrozenIterator<value_type> iterator;
  typedef FrozenIterator<value_type> const_iterator;

  template <class Key>
  const mapped_type& at(const Key& key) const {
    auto found = find(key);
    if (found == this->end()) {
      throw std::out_of_range("key not found");
    }
    return found->second;
  }

  template <class Key>
  const_iterator find(const Key& key) const {
    auto found = lower_bound(key);
    if (found != this->end() && found->first == key) {
      return found;
    } else {
      return this->end();
    }
  }

  template <class Key>
  const_iterator lower_bound(const Key& key) const {
    return std::lower_bound(
        this->begin(), this->end(), key, KeyComparator<Key>());
  }

  template <class Key>
  const_iterator upper_bound(const Key& key) const {
    return std::upper_bound(
        this->begin(), this->end(), key, KeyComparator<Key>());
  }

  template <class Key>
  std::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
    return std::equal_range(
        this->begin(), this->end(), key, KeyComparator<Key>());
  }

  template <class Key>
  size_t count(const Key& key) const {
    return find(key) != this->end() ? 1 : 0;
  }

 private:
  template <class Key>
  struct KeyComparator {
    bool operator()(const value_type& a, const Key& b) const {
      return a.first < b;
    }
    bool operator()(const Key& a, const value_type& b) const {
      return a < b.first;
    }
  };
};

/**
 * Freezer<Map<K, V>> - Freezes map<K, V> into the above Frozen<map<K, V>>.
 */
template <class K, class V, class MapType>
struct MapFreezer : public RangeFreezer<std::pair<const K, V>, MapType> {
  typedef MapType ThawedType;
  typedef FrozenMap<K, V> FrozenType;

  static void thawImpl(const FrozenType& src, ThawedType& dst) {
    dst.clear();
    // thaw the range into the output map, entry-by-entry
    for (auto& item : src) {
      std::pair<K, V> pair;
      thaw(item, pair);
      dst.insert(std::move(pair));
    }
  }
};

namespace detail {

struct BlockIndex {
  BlockIndex() : offset(0), mask(0) {}

  uint64_t offset;
  uint64_t mask;

  static constexpr size_t kSize = sizeof(uint64_t) * 8;
};

// Do NOT use this hash function anywhere else (hint use folly::Hash instead).
// Used here because the result of this function dictates frozen layout.
struct DeprecatedStringPieceHash {
  std::size_t operator()(const folly::StringPiece str) const {
    const auto size = str.size();
    const auto data = str.data();
    // Taken from fbi/nstring.h:
    //    Quick and dirty bernstein hash...fine for short ascii strings
    uint32_t hash = 5381;
    for (size_t ix = 0; ix < size; ix++) {
      hash = ((hash << 5) + hash) + data[ix];
    }
    return static_cast<std::size_t>(hash);
  }
};

} // namespace detail

inline size_t frozenHash(folly::StringPiece sp) {
  return apache::thrift::detail::DeprecatedStringPieceHash()(sp);
}

inline size_t frozenHash(const FrozenRange<char>& fr) {
  return apache::thrift::detail::DeprecatedStringPieceHash()(fr.range());
}

inline size_t frozenHash(size_t i) {
  return std::hash<size_t>()(i) * 3; // avoid contiguous hash values
}

template <>
struct Freezer<apache::thrift::detail::BlockIndex, void>
    : TrivialFreezer<apache::thrift::detail::BlockIndex> {};

/**
 * FrozenHashMap<...> - A sparsehash-based hashtable for frozen HashMaps.
 */
template <class K, class V>
struct FrozenHashMap : public FrozenRange<std::pair<const K, V>> {
 private:
  typedef FrozenRange<std::pair<const K, V>> Base;

 public:
  typedef typename Freezer<K>::FrozenType key_type;
  typedef typename Freezer<V>::FrozenType mapped_type;
  typedef std::pair<key_type, mapped_type> value_type;
  typedef FrozenIterator<value_type> iterator;
  typedef FrozenIterator<value_type> const_iterator;

  template <class Key>
  const_iterator find(const Key& key) const {
    auto h = frozenHash(key);
    auto chunks = blockIndex.size();
    auto bits = apache::thrift::detail::BlockIndex::kSize;
    auto buckets = chunks * bits;
    for (size_t p = 0; p < buckets; h += ++p) { // quadratic probing
      auto bucket = h % buckets;
      auto major = bucket / bits;
      auto minor = bucket % bits;
      const apache::thrift::detail::BlockIndex* block = &blockIndex[major];
      for (;;) {
        if (0 == (1 & (block->mask >> minor))) {
          return this->end();
        }
        size_t subOffset = folly::popcount(block->mask & ((1ULL << minor) - 1));
        auto index = block->offset + subOffset;
        auto found = this->begin() + index;
        if (found->first == key) {
          return found;
        }
        minor += ++p;
        if (LIKELY(minor < bits)) {
          h += p; // same block shortcut
        } else {
          --p; // undo
          break;
        }
      }
    }
    return this->end();
  }

  template <class Key>
  const mapped_type& at(const Key& key) const {
    auto found = find(key);
    if (found == this->end()) {
      throw std::out_of_range("key not found");
    }
    return found->second;
  }

  template <class Key>
  std::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
    const_iterator found = find(key);
    if (found == this->end()) {
      return std::make_pair(nullptr, nullptr);
    } else {
      return std::make_pair(found, found + 1);
    }
  }

  template <class Key>
  size_t count(const Key& key) const {
    return find(key) != this->end() ? 1 : 0;
  }

  void clear() {
    Base::clear();
    blockIndex.clear();
  }

  FrozenRange<apache::thrift::detail::BlockIndex> blockIndex;

 private:
  template <class Key>
  struct KeyComparator {
    bool operator()(const value_type& a, const Key& b) const {
      return a.first < b;
    }
    bool operator()(const Key& a, const value_type& b) const {
      return a < b.first;
    }
  };
};

/**
 * Freezer<Map<K, V>> - Freezes map<K, V> into the above Frozen<map<K, V>>.
 */
template <class K, class V, class HashMapType>
struct HashMapFreezer {
  typedef HashMapType ThawedType;
  typedef FrozenHashMap<K, V> FrozenType;
  typedef typename ThawedType::value_type ThawedItem;
  typedef typename Freezer<ThawedItem>::FrozenType FrozenItem;

  static size_t chunkCount(size_t size) {
    auto bits = apache::thrift::detail::BlockIndex::kSize;
    // 1.5 => 66% load factor => 3 bits/entry overhead
    // 2.0 => 50% load factor => 4 bits/entry overhead
    return size_t(size * 2.0 + bits - 1) / bits;
  }

  static void freezeImpl(
      const ThawedType& src, FrozenType& dst, byte*& buffer) {
    size_t size = src.size();
    if (!size) {
      // point to [nullptr, nullptr) if the range is empty.
      dst.clear();
      return;
    }

    size_t chunks = chunkCount(size);
    auto bits = apache::thrift::detail::BlockIndex::kSize;
    size_t buckets = chunks * bits;
    std::unique_ptr<const ThawedItem*[]> index(new const ThawedItem*[buckets]);
    for (size_t b = 0; b < buckets; ++b) {
      index[b] = nullptr;
    }

    for (auto& item : src) {
      size_t h = frozenHash(item.first);
      for (size_t p = 0;; h += ++p) { // quadratic probing
        const ThawedItem** bucket = &index[h % buckets];
        if (*bucket) {
          if (p == buckets) {
            throw std::out_of_range("buckets!");
          }
          continue;
        } else {
          *bucket = &item;
          break;
        }
      }
    }
    auto* itemsBegin = unaligned_ptr_cast<FrozenItem*>(buffer);
    auto* itemsEnd = itemsBegin + size;
    dst.reset(itemsBegin, itemsEnd);
    buffer = unaligned_ptr_cast<byte*>(itemsEnd);

    auto* indexBegin =
        unaligned_ptr_cast<apache::thrift::detail::BlockIndex*>(buffer);
    auto* indexEnd = indexBegin + chunks;
    dst.blockIndex.reset(indexBegin, indexEnd);
    buffer = unaligned_ptr_cast<byte*>(indexEnd);

    size_t count = 0;
    size_t b = 0;
    for (size_t c = 0; c < chunks; ++c) {
      apache::thrift::detail::BlockIndex chunk;
      chunk.offset = count;
      for (size_t offset = 0; offset < bits; ++offset) {
        if (const ThawedItem* bucket = index[b++]) {
          chunk.mask |= uint64_t(1) << offset;
          freeze(*bucket, *itemsBegin++, buffer);
          ++count;
        }
      }
      *indexBegin++ = chunk;
    }
  }

  static void thawImpl(const FrozenType& src, ThawedType& dst) {
    dst.reserve(src.size());
    dst.clear();
    for (auto& item : src) {
      std::pair<K, V> pair;
      thaw(item, pair);
      dst.insert(std::move(pair));
    }
  }

  static size_t extraSizeImpl(const ThawedType& src) {
    size_t size = 0;
    // Extra space needed is the sum of the extra space needed for each of the
    // items in the source range.
    for (auto& item : src) {
      size += frozenSize(item);
    }
    size += sizeof(apache::thrift::detail::BlockIndex) * chunkCount(src.size());
    return size;
  }
};

/**
 * Frozen<std::set<T>> - Wraps a sorted FrozenRange<T> with the expected
 * interface for a set. find() and others are templetized to allow searching for
 * values of any type which may be compared to the item, though may not be of
 * type T.
 */
template <
    class ThawedItem,
    class FrozenItem = typename Freezer<ThawedItem>::FrozenType>
struct FrozenSet : public FrozenRange<ThawedItem, FrozenItem> {
  typedef const FrozenItem value_type;
  typedef value_type* iterator;
  typedef value_type* const_iterator;

  template <class Key>
  const_iterator find(const Key& key) const {
    auto found = lower_bound(key);
    if (found != this->end() && *found == key) {
      return found;
    } else {
      return this->end();
    }
  }

  template <class Key>
  const_iterator lower_bound(const Key& key) const {
    return std::lower_bound(this->begin(), this->end(), key);
  }

  template <class Key>
  const_iterator upper_bound(const Key& key) const {
    return std::upper_bound(this->begin(), this->end(), key);
  }

  template <class Key>
  std::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
    return std::equal_range(this->begin(), this->end(), key);
  }

  template <class Key>
  size_t count(const Key& key) const {
    return find(key) != this->end() ? 1 : 0;
  }
};

/**
 * Freezer<std::set<T>> - Freezes set<T> into the above Frozen<set<T>>.
 */
template <class ThawedItem, class SetType>
struct SetFreezer : public RangeFreezer<ThawedItem, SetType> {
  typedef SetType ThawedType;
  typedef FrozenSet<ThawedItem> FrozenType;

  static void thawImpl(const FrozenType& src, ThawedType& dst) {
    dst.clear();
    for (auto& frozen : src) {
      ThawedItem item;
      thaw(frozen, item);
      dst.insert(std::move(item));
    }
  }
};

template <>
struct Freezer<std::string, void> : RangeFreezer<char, std::string> {};

template <>
struct Freezer<folly::fbstring, void> : RangeFreezer<char, folly::fbstring> {};

template <>
struct Freezer<folly::StringPiece, void>
    : public RangeFreezer<char, folly::StringPiece> {};

template <class K, class V>
struct Freezer<std::map<K, V>, void> : MapFreezer<K, V, std::map<K, V>> {};

template <class K, class V>
struct Freezer<std::unordered_map<K, V>, void>
    : HashMapFreezer<K, V, std::unordered_map<K, V>> {};

template <class T>
struct Freezer<std::vector<T>, void> : RangeFreezer<T, std::vector<T>> {};

template <class T>
struct Freezer<std::set<T>, void> : SetFreezer<T, std::set<T>> {};

std::unique_ptr<const FrozenRange<char>, FrozenTypeDeleter> inline freezeStr(
    folly::StringPiece str) {
  return freeze(str);
}

} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT_LIB_CPP_FROZEN_H_
