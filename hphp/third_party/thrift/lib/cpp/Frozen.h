/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef HPHP_THRIFT_LIB_CPP_FROZEN_H
#define HPHP_THRIFT_LIB_CPP_FROZEN_H

#include "folly/Range.h"
#include <memory>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <type_traits>
#include "thrift/lib/cpp/RelativePtr.h"

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
namespace apache { namespace thrift {

/**
 * Frozen<T> - This is simply a canonical type name for naming the frozen
 * counterpart for a given mutable type.
 */
template<class T, class = void>
struct Frozen;

/**
 * Freezer<T> - The helper class which encapsulates the types and methods for
 * mapping from T and its instances to its frozen counterparts and its
 * instances.
 */
template<class T, class = void>
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
  static void freezeImpl(const ThawedType& src,
                         FrozenType& dst,
                         byte*& buffer);

  // Thaw 'src' into 'dst' recursively. Specializations of this class wil
  // recurse to each field, thawing each of their fields recursively.
  static void thawImpl(const FrozenType& src, ThawedType& dst);
};

/**
 * Freezer<T> - Freezer for POD types, which are simply copied.
 */
template<class T>
struct Freezer<T,
               typename std::enable_if<
                 std::is_pod<T>::value
                 && !std::is_const<T>::value
               >::type> {
  // The type which a frozen value thaws back into, usually just T.
  typedef T ThawedType;

  // The type which a mutable value is frozen into. Here, it is simply T. For
  // complex data types, this class will be specialized and a different type
  // will be provided, usually Frozen<T>.
  typedef T FrozenType;

  // Compute the amount of out-of-struct space needed to represent the given
  // object. Specializations of this class will recurse down to children,
  // summing up their extraSize results recursively.
  static constexpr size_t extraSizeImpl(const ThawedType& src) {
    return 0;
  }

  // Freeze 'src' into 'dst' recursively, using 'buffer' for additional storage.
  // 'buffer' must be advanced by specializations of this class if the spare
  // space is used.
  static void freezeImpl(const ThawedType& src,
                         FrozenType& dst,
                         byte*& buffer) {
    dst = src;
  }

  // Thaw 'src' into 'dst' recursively. Specializations of this class wil
  // recurse to each field, thawing each of their fields recursively.
  static void thawImpl(const FrozenType& src, ThawedType& dst) {
    dst = src;
  }
};

/**
 * Freezer specialization for const T. Primarily a pass-through to Freezer<T>,
 * but const-ness removed to enable thawing.
 */
template<class T>
struct Freezer<const T, void> : public Freezer<T, void> {};

/**
 * extraSize - Dispatch to Freezer<T>'s extraSizeImpl to calculate addition
 * space needed for 'src'.
 */
template<class T>
size_t extraSize(const T& src) {
  return Freezer<T>::extraSizeImpl(src);
}

template<class T>
T unaligned_ptr_cast(void* ptr) {
  return static_cast<T>(ptr);
}

template<class T>
const T unaligned_ptr_cast(const void* ptr) {
  return static_cast<const T>(ptr);
}

/**
 * frozenSize - Total space needed to store a frozen representation of 'src'.
 */
template<class T>
size_t frozenSize(const T& src) {
  return sizeof(typename Freezer<T>::FrozenType) + extraSize(src);
}

/**
 * freeze(...) - Freeze 'src' into 'dst', consuming 'buffer' for extra space.
 */
template<class T,
         class FrozenType = typename Freezer<T>::FrozenType>
void freeze(const T& src, FrozenType& dst, byte*& buffer) {
  Freezer<T>::freezeImpl(src, dst, buffer);
}

/**
 * freeze(...) - Freeze 'src' by consuming memory pointed to by 'buffer'.
 */
template<class T,
         class FrozenType = typename Freezer<T>::FrozenType>
const FrozenType*
freeze(const T& src, byte*& buffer) {
  // NOTE(tjackson): This pointer will not necessarily be aligned with
  //                 alignof(FrozenType).
  FrozenType* frozen = unaligned_ptr_cast<FrozenType*>(buffer);
  buffer += sizeof(FrozenType);
  Freezer<T>::freezeImpl(src, *frozen, buffer);
  return frozen;
}

struct FrozenTypeDeleter {
  void operator()(const void* ptr) const {
    free(const_cast<void*>(ptr));
  }
};

template<class T,
         class FrozenType = typename Freezer<T>::FrozenType>
using FrozenTypeUPtr = std::unique_ptr<const FrozenType, FrozenTypeDeleter>;

/**
 * freeze(...) - Freeze 'src' into a newly-allocated buffer owned by a
 * unique_ptr.
 */
template<class T,
         class FrozenType = typename Freezer<T>::FrozenType>
FrozenTypeUPtr<T, FrozenType>
freeze(const T& src) {
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

  buffer += sizeof(FrozenType);
  // start populating the object graph, starting from 'src' with spare storage
  // allocated at 'buffer'.
  freeze(src, *frozen, buffer);
  assert(buffer == finish);

  return ret;
}

/**
 * thaw(...) - Thaw a given frozen into its mutable counterpart.
 */
template<class ThawedType>
void thaw(const typename Freezer<ThawedType>::FrozenType& frozen,
          ThawedType& thawed) {
  Freezer<ThawedType>::thawImpl(frozen, thawed);
}

/**
 * thaw(...) - Thaw a given frozen into its mutable counterpart.
 */
template<class ThawedType>
ThawedType thaw(const Frozen<ThawedType>& frozen) {
  ThawedType thawed;
  Freezer<ThawedType>::thawImpl(frozen, thawed);
  return thawed;
}

/**
 * Freezer<std::pair<A, B>> - Freezer for pairs of values, frozen as
 * std::pair<Frozen<A>, Frozen<B>> with the help of Freezer<A> and Freezer<B>.
 */
template<class A, class B>
struct Freezer<std::pair<A, B>> {
  typedef std::pair<typename Freezer<A>::ThawedType,
                    typename Freezer<B>::ThawedType> ThawedType;
  typedef std::pair<typename Freezer<A>::FrozenType,
                    typename Freezer<B>::FrozenType> FrozenType;

  static constexpr size_t extraSizeImpl(const ThawedType& src) {
    return Freezer<A>::extraSizeImpl(src.first) +
           Freezer<B>::extraSizeImpl(src.second);
  }

  static void freezeImpl(const ThawedType& src,
                         FrozenType& dst,
                         byte*& buffer) {
    Freezer<A>::freezeImpl(src.first, dst.first, buffer);
    Freezer<B>::freezeImpl(src.second, dst.second, buffer);
  }

  static void thawImpl(const FrozenType& src, ThawedType& dst) {
    Freezer<A>::thawImpl(src.first, dst.first);
    Freezer<B>::thawImpl(src.second, dst.second);
  }
};

/**
 * FrozenRange<...> - Represents a range of contiguous, frozen values pointed to
 * by relative pointers. Assists in the freezing of vectors, sets, strings, and
 * maps.
 *
 * The memory pointed to by FrozenRange is usually located at addresses just
 * higher than the address of the FrozenRange itself.
 */
template<class ThawedItem,
         class FrozenItem = typename Freezer<ThawedItem>::FrozenType>
struct FrozenRange {
  typedef const FrozenItem value_type;
  typedef const value_type* iterator;
  typedef const value_type* const_iterator;

  FrozenRange()
    : begin_(nullptr), end_(nullptr) {}

  FrozenRange(const_iterator begin, const_iterator end)
    : begin_(begin), end_(end) {}

  const_iterator begin() const { return begin_.get(); }
  const_iterator end() const { return end_.get(); }

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

  size_t size() const { return end_.get() - begin_.get(); }
  bool empty() const { return end_.get() == begin_.get(); }

  const value_type& operator[](int i) const {
    return begin_.get()[i];
  }

  template<class Range,
           class = decltype(std::declval<Range>().begin())>
  bool operator<(const Range& range) const {
    return std::lexicographical_compare(this->begin(), this->end(),
                                        range.begin(), range.end());
  }

  template<class Range,
           class = decltype(std::declval<Range>().begin())>
  bool operator>(const Range& range) const {
    return std::lexicographical_compare(range.begin(), range.end(),
                                        this->begin(), this->end());
  }

  template<class Range,
           class = decltype(std::declval<Range>().begin())>
  bool operator==(const Range& range) const {
    return size() == range.size() && std::equal(this->begin(), this->end(),
                                                range.begin());
  }

  folly::Range<const_iterator> range() const {
    return folly::Range<const_iterator>(this->begin(), this->end());
  }

  void reset(const_iterator begin, const_iterator end) {
    begin_.reset(begin);
    end_.reset(end);
  }

 private:
  RelativePtr<const value_type> begin_, end_;
};

/**
 * operator overloads to facilitate comparison and sorting of FrozenRange's.
 * Primarily needed for inclusion in std::map's.
 */
template<class ThawedString>
bool operator==(const FrozenRange<char, ThawedString>& str,
                const char* cstr) {
  return str == folly::StringPiece(cstr);
}

template<class ThawedString>
bool operator==(const char* cstr,
                FrozenRange<char, ThawedString>& str) {
  return str == folly::StringPiece(cstr);
}

template<class ThawedString>
bool operator<(const FrozenRange<char, ThawedString>& str,
               const char* cstr) {
  return str < folly::StringPiece(cstr);
}

template<class ThawedString>
bool operator<(const char* cstr,
               const FrozenRange<char, ThawedString>& str) {
  return folly::StringPiece(cstr) < str;
}

template<class ThawedItem,
         class FrozenItem,
         class Range,
         class = decltype(std::declval<Range>().begin())>
bool operator<(const Range& range,
               const FrozenRange<ThawedItem, FrozenItem>& frozen) {
  return frozen > range;
}

template<class ThawedItem,
         class FrozenItem,
         class Range,
         class = decltype(std::declval<Range>().begin())>
bool operator==(const Range& range,
                const FrozenRange<ThawedItem, FrozenItem>& frozen) {
  return range.size() == frozen.size()
    && std::equal(range.begin(), range.end(), frozen.begin());
}

template<class Ostream,
         class ThawedItem,
         class FrozenItem>
Ostream& operator<<(Ostream& os,
                    const FrozenRange<ThawedItem, FrozenItem>& frozen) {
  for (auto& item : frozen) {
    os << item;
  }
  return os;
}

/**
 * RangeFreezer - Helper type for freezing range-like types.
 */
template<class ThawedItem,
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

  static void freezeImpl(const ThawedType& src,
                         FrozenType& dst,
                         byte*& buffer) {
    size_t size = src.size();
    if (!size) {
      //point to [nullptr, nullptr) if the range is empty.
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
 * FrozenMap<...> - Wraps a sorted FrozenRange<K, V> with the expected
 * interface for a map. find() and others are templetized to allow searching for
 * values of any type which may be compared to the key, though may not be of
 * type K.
 */
template<class K,
         class V>
struct FrozenMap : public FrozenRange<std::pair<const K, V>> {
  typedef typename Freezer<K>::FrozenType key_type;
  typedef typename Freezer<V>::FrozenType mapped_type;
  typedef std::pair<key_type, mapped_type> value_type;
  typedef const value_type* iterator;
  typedef const value_type* const_iterator;

  template<class Key>
  const mapped_type& at(const Key& key) const {
    auto found = find(key);
    if (found == this->end()) {
      throw std::out_of_range("key not found");
    }
    return found->second;
  }

  template<class Key>
  const_iterator find(const Key& key) const {
    auto found = lower_bound(key);
    if (found != this->end() &&
        found->first == key) {
      return found;
    } else {
      return this->end();
    }
  }

  template<class Key>
  const_iterator lower_bound(const Key& key) const {
    return std::lower_bound(this->begin(), this->end(),
                            key, KeyComparator<Key>());
  }

  template<class Key>
  const_iterator upper_bound(const Key& key) const {
    return std::upper_bound(this->begin(), this->end(),
                            key, KeyComparator<Key>());
  }

  template<class Key>
  std::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
    return std::equal_range(this->begin(), this->end(),
                            key, KeyComparator<Key>());
  }

  template<class Key>
  size_t count(const Key& key) const {
    auto equalRange = equal_range(key);
    return equalRange.second - equalRange.first;
  }

 private:
  template<class Key>
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
template<class K,
         class V,
         class MapType>
struct MapFreezer : public RangeFreezer<std::pair<const K, V>, MapType> {
  typedef MapType ThawedType;
  typedef FrozenMap<K, V> FrozenType;

  static void thawImpl(const FrozenType& src, ThawedType& dst) {
    dst.clear();
    // thaw the range into the output map, entry-by-entry
    for (auto& item : src) {
      std::pair<K, V> pair;
      thaw(item, pair);
      dst.insert(dst.end(), std::move(pair));
    }
  }
};

/**
 * Frozen<std::set<T>> - Wraps a sorted FrozenRange<T> with the expected
 * interface for a set. find() and others are templetized to allow searching for
 * values of any type which may be compared to the item, though may not be of
 * type T.
 */
template<class ThawedItem,
         class FrozenItem = typename Freezer<ThawedItem>::FrozenType>
struct FrozenSet : public FrozenRange<ThawedItem, FrozenItem> {
  typedef const FrozenItem value_type;
  typedef value_type* iterator;
  typedef value_type* const_iterator;

  template<class Key>
  const_iterator find(const Key& key) const {
    auto found = lower_bound(key);
    if (found != this->end() &&
        *found == key) {
      return found;
    } else {
      return this->end();
    }
  }

  template<class Key>
  const_iterator lower_bound(const Key& key) const {
    return std::lower_bound(this->begin(), this->end(), key);
  }

  template<class Key>
  const_iterator upper_bound(const Key& key) const {
    return std::upper_bound(this->begin(), this->end(), key);
  }

  template<class Key>
  std::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
    return std::equal_range(this->begin(), this->end(), key);
  }

  template<class Key>
  size_t count(const Key& key) const {
    auto equalRange = equal_range(key);
    return equalRange.second - equalRange.first;
  }
};

/**
 * Freezer<std::set<T>> - Freezes set<T> into the above Frozen<set<T>>.
 */
template<class ThawedItem,
         class SetType>
struct SetFreezer : public RangeFreezer<ThawedItem, SetType> {
  typedef SetType ThawedType;
  typedef FrozenSet<ThawedItem> FrozenType;

  static void thawImpl(const FrozenType& src, ThawedType& dst) {
    dst.clear();
    for (auto& frozen : src) {
      ThawedItem item;
      thaw(frozen, item);
      dst.insert(dst.end(), std::move(item));
    }
  }
};

template<>
struct Freezer<std::string, void>
  : public RangeFreezer<char, std::string> {};

template<>
struct Freezer<folly::fbstring, void>
  : public RangeFreezer<char, folly::fbstring> {};

template<class K, class V>
struct Freezer<std::map<K, V>, void>
  : public MapFreezer<K, V, std::map<K, V>> {};

template<class T>
struct Freezer<std::vector<T>, void>
  : public RangeFreezer<T, std::vector<T>> {};

template<class T>
struct Freezer<std::set<T>, void>
  : public SetFreezer<T, std::set<T>> {};

}} // apache::thrift
#endif // #ifndef HPHP_THRIFT_LIB_CPP_FROZEN_H
