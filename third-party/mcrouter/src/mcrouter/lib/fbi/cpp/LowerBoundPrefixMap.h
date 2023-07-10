/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/CPortability.h>
#include <folly/lang/Bits.h>
#include <folly/sorted_vector_types.h>

#include <algorithm>
#include <compare>
#include <iosfwd>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace facebook::memcache {
namespace detail {

// Comparing strings is expensive.
// We can compare 8 byte integers instead.
// Especially since we can prebuild the prefixes
// this becomes a lot faster.
class SmallPrefix {
 public:
  SmallPrefix() = default;

  FOLLY_ALWAYS_INLINE
  explicit SmallPrefix(std::string_view s) {
    if (FOLLY_LIKELY(s.size() >= 8)) {
      std::memcpy(&data_, s.data(), 8);
      data_ = folly::Endian::swap(data_);
      return;
    }
    std::memcpy(&data_, s.data(), s.size());
    data_ = folly::Endian::swap(data_);
  }

  bool operator==(const SmallPrefix&) const = default;
  auto operator<=>(const SmallPrefix&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const SmallPrefix& self);

 private:
  std::uint64_t data_ = 0;
};

// What is the idea
//
// If a string (query) has a prefix in a sorted lexicographically array
// then when you binary search for it, you will either:
// a) find the longest prefix in the array
// b) find a string that has that longest prefix with the query in common
//
// Proof:
//   Without losing generality: longest prefix abc, query abcfz
//   Let's say we find
//      [abc,  xxxxx, bcd, ...]
//                   ^
//   Assume that xxxxx doesn't start with abc: then abcfz would be between
//   abc and xxxxx which is against the assumption.
//
struct LowerBoundPrefixMapCommon {
  LowerBoundPrefixMapCommon() = default;
  explicit LowerBoundPrefixMapCommon(
      std::span<const std::string_view> sortedUniquePrefixes);

  // returns 1 based indexes, 0 if not found.
  std::uint32_t findPrefix(std::string_view query) const noexcept;

  using IndexPair = std::pair<std::uint32_t, std::uint32_t>;

  // A map by a smaller prefix to speed up the search.
  // Index pair indicates all strings starting with a given smaller prefix.
  //
  // NOTE: in theory folly::heap_vector_map should be better here
  //       but benchmarks do not support that idea.
  folly::sorted_vector_map<SmallPrefix, IndexPair> smallPrefixes_;

  // All strings are stored contiguously in this buffer of chars, separated at
  // markers_.
  // markers_[0] == 0, markers_.back() == chars.size().
  // This is sometimes known as a "tape"
  // This is faster in the benchmarks and more cache local.
  std::vector<char> chars_;
  std::vector<std::uint32_t> markers_;

  // Each string might have a prefix also in the array.
  // a b ba baa bab
  //   ^_|   |    |
  //     ^___|    |
  //     ^________|
  // This is the index of that prefix, base 1 (0 means absence).
  std::vector<std::uint32_t> previousPrefix_;

  std::string_view str(std::uint32_t i) const {
    const char* f = chars_.data() + markers_[i];
    const char* l = chars_.data() + markers_[i + 1];
    return std::string_view{f, l};
  }
};

} // namespace detail

/*
 * A map that for a given query finds an item corresponding
 * to the longest prefix in the map.
 *
 * Map is constructed once and then does not change.
 *
 * Example:
 *   {
 *     "b"  : 1,
 *     "bd" : 2
 *     "c"  : 3,
 *   }
 *
 *  For "ba" result is 1
 *  For "bd" result is 2
 *  For "d"  result is null
 *
 * NOTE:
 *  If the input prefix : values mapping contains
 *  duplicated prefixes, the last one is chosen.
 */
template <typename T>
class LowerBoundPrefixMap {
 public:
  class Builder {
   public:
    void reserve(std::size_t n) {
      prefix2value_.reserve(n);
    }

    void insert(std::pair<std::string, T> x) {
      prefix2value_.push_back(std::move(x));
    }

    LowerBoundPrefixMap build() && {
      return LowerBoundPrefixMap{std::move(prefix2value_)};
    }

   private:
    std::vector<std::pair<std::string, T>> prefix2value_;
  };

  LowerBoundPrefixMap() = default;

  explicit LowerBoundPrefixMap(
      std::vector<std::pair<std::string, T>> prefix2value);

  const T* findPrefix(std::string_view query) const noexcept {
    std::uint32_t pos = searchLogic_.findPrefix(query);
    return pos == 0 ? nullptr : &values_[pos - 1];
  }

 private:
  detail::LowerBoundPrefixMapCommon searchLogic_;
  std::vector<T> values_;
};

template <typename T>
LowerBoundPrefixMap<T>::LowerBoundPrefixMap(
    std::vector<std::pair<std::string, T>> prefix2value) {
  std::stable_sort(
      prefix2value.begin(),
      prefix2value.end(),
      [](const auto& x, const auto& y) { return x.first < y.first; });

  // preferring the last duplicate
  auto rend = std::unique(
      prefix2value.rbegin(),
      prefix2value.rend(),
      [](const auto& x, const auto& y) { return x.first == y.first; });

  std::span<std::pair<std::string, T>> sortedUnique{
      rend.base(), prefix2value.end()};

  std::vector<std::string_view> sortedPrefixes;
  sortedPrefixes.reserve(sortedUnique.size());
  values_.reserve(sortedUnique.size());

  for (auto& [prefix, value] : sortedUnique) {
    sortedPrefixes.emplace_back(prefix);
    values_.emplace_back(std::move(value));
  }

  searchLogic_ = detail::LowerBoundPrefixMapCommon(sortedPrefixes);
}

} // namespace facebook::memcache
