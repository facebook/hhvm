/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/fbi/cpp/LowerBoundPrefixMap.h"

#include <cstring>
#include <numeric>
#include <ostream>
#include <stdexcept>

#include <folly/CPortability.h>
#include <folly/Likely.h>

namespace facebook::memcache::detail {
namespace {

// .starts_with is not avaliable before C++20 and this needs to compile with an
// older standard
bool std_string_view_starts_with(std::string_view what, std::string_view with) {
  return what.substr(0, with.size()) == with;
}

} // namespace

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

  // Adding an empty string with no matches.
  // This acts as a sentinel so we are always guranteed to find an
  // upper bound. If the user gives us an empty string, this still works.
  //
  // NOTE: this single element emplace is reasonably fast because the set is
  //       empty.
  smallPrefixes_.emplace(SmallPrefix(), IndexPair{0, 0});

  for (std::size_t i = 0; i != fullPrefixes_.size(); ++i) {
    const auto& prefix = fullPrefixes_[i];

    // Prefixes always come before lexicographically, so if there is one
    // we will find it.
    previousPrefix_.push_back(findPrefix(prefix));

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
  // Due to a sentinel - guaranteed to not be .begin()
  auto afterPrefix = smallPrefixes_.upper_bound(SmallPrefix{query});
  auto [roughFrom, roughTo] = std::prev(afterPrefix)->second;

  // Binary search complete strings between rough boundaries.
  // NOTE: which array we search - doesn't matter -
  //       we just want indexes.
  auto cur = std::upper_bound(
                 fullPrefixes_.begin() + roughFrom,
                 fullPrefixes_.begin() + roughTo,
                 query) -
      fullPrefixes_.begin();

  while (cur != 0 &&
         !std_string_view_starts_with(query, fullPrefixes_[cur - 1])) {
    cur = previousPrefix_[cur - 1];
  }

  return cur;
}

} // namespace facebook::memcache::detail
