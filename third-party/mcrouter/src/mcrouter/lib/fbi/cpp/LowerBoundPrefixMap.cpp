/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/fbi/cpp/LowerBoundPrefixMap.h"

#include <cassert>
#include <cstring>
#include <ostream>

#include <folly/lang/Bits.h>

namespace facebook::memcache::detail {
namespace {

// .starts_with is not available before C++20 and this needs to compile with an
// older standard
bool std_string_view_starts_with(std::string_view what, std::string_view with) {
  return what.substr(0, with.size()) == with;
}

#ifdef __aarch64__
// Branchless upper_bound on ARM. Conditional index updates compile to CSEL
// instructions, avoiding branch mispredictions that hurt
// sorted_vector_map::upper_bound() through iterator/comparator wrappers.
// On x86, the hardware branch predictor handles upper_bound well so this
// is not beneficial there.
template <typename I, typename T, typename Compare>
I branchlessUpperBound(I f, I l, const T& x, Compare comp) {
  auto len = static_cast<std::size_t>(l - f);
  if (len == 0) {
    return f;
  }

  while (len > 1) {
    auto half = len / 2;
    f += comp(x, f[half]) ? 0 : half;
    len -= half;
  }
  f += comp(x, *f) ? 0 : 1;

  return f;
}

#endif

} // namespace

FOLLY_ALWAYS_INLINE
bool checkSmallPrefixStartsWith(
    SmallPrefix full,
    SmallPrefix start,
    std::uint64_t lengthMask) {
  return (full.data_ & lengthMask) == start.data_;
}

std::ostream& operator<<(std::ostream& os, const SmallPrefix& self) {
  std::uint64_t correctOrder = folly::Endian::swap(self.data_);
  std::string_view s(reinterpret_cast<const char*>(correctOrder), 8u);
  return os << s;
}

LowerBoundPrefixMapCommon::LowerBoundPrefixMapCommon(
    folly::string_tape sortedUniquePrefixes)
    : fullPrefixes_(std::move(sortedUniquePrefixes)) {
  smallPrefixes_.reserve(fullPrefixes_.size() + 1);
  previousPrefix_.reserve(fullPrefixes_.size());
  smallPrefixExtraInfo_.reserve(fullPrefixes_.size());

  // Adding an empty string with no matches.
  // This acts as a sentinel so we are always guranteed to find an
  // upper bound. If the user gives us an empty string, this still works.
  //
  // NOTE: this single element emplace is reasonably fast because the set is
  //       empty.
  smallPrefixes_.emplace(SmallPrefix(), IndexPair{0, 0});

  for (std::size_t i = 0; i != fullPrefixes_.size(); ++i) {
    const auto& prefix = fullPrefixes_[i];
    // Null bytes are not expected. If present it would break the
    // findPrefix fast SmallPrefix path (e.g. SmallPrefix "ab\0", query "ab").
    assert(prefix.find('\0') == std::string_view::npos);

    // Prefixes always come before lexicographically, so if there is one
    // we will find it.
    previousPrefix_.push_back(findPrefix(prefix));

    auto len = std::min<size_t>(prefix.size(), (size_t)8);
    auto mask = folly::n_most_significant_bits<std::uint64_t>(len * 8);
    smallPrefixExtraInfo_.push_back({SmallPrefix{prefix}, mask});

    // Add small prefix ---
    // if it is there already - update existing, otherwise insert [i, i+1]
    //
    // NOTE: this single element emplace in a sorted_vector is reasonably fast,
    //       because it's in the end. Unfortunately, we can't use hint for api
    //       reasons.
    auto [it, inserted] =
        smallPrefixes_.insert({SmallPrefix{prefix}, IndexPair{i, i + 1}});

    if (!inserted) {
      ++(it->second.second);
    }
  }
}

std::uint32_t LowerBoundPrefixMapCommon::findPrefix(
    std::string_view query) const noexcept {
  SmallPrefix qSmall{query};
#ifdef __aarch64__
  auto afterPrefix = branchlessUpperBound(
      smallPrefixes_.begin(),
      smallPrefixes_.end(),
      qSmall,
      [](const SmallPrefix& v, const auto& elem) { return v < elem.first; });
#else
  // Due to a sentinel - guaranteed to not be .begin()
  auto afterPrefix = smallPrefixes_.upper_bound(qSmall);
#endif
  const auto bucketIt = std::prev(afterPrefix);
  auto [roughFrom, roughTo] = bucketIt->second;

  if (bucketIt->first < qSmall || query.size() < 8) {
    std::uint32_t cur = roughTo;
    while (cur != 0 &&
           !checkSmallPrefixStartsWith(
               qSmall,
               smallPrefixExtraInfo_[cur - 1].smallPrefix,
               smallPrefixExtraInfo_[cur - 1].lengthMask)) {
      cur = previousPrefix_[cur - 1];
    }
    return cur;
  }

  std::uint32_t cur = static_cast<std::uint32_t>(
      std::upper_bound(
          fullPrefixes_.begin() + roughFrom,
          fullPrefixes_.begin() + roughTo,
          query) -
      fullPrefixes_.begin());

  while (cur != 0 &&
         !std_string_view_starts_with(query, fullPrefixes_[cur - 1])) {
    cur = previousPrefix_[cur - 1];
  }

  return cur;
}

} // namespace facebook::memcache::detail
