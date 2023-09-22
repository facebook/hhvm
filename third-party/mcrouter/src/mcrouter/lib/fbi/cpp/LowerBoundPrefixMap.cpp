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
#include <span>
#include <stdexcept>

#include <folly/CPortability.h>
#include <folly/Likely.h>

namespace facebook::memcache::detail {

std::ostream& operator<<(std::ostream& os, const SmallPrefix& self) {
  std::uint64_t correctOrder = folly::Endian::swap(self.data_);
  std::string_view s(reinterpret_cast<const char*>(correctOrder), 8u);
  return os << s;
}

LowerBoundPrefixMapCommon::LowerBoundPrefixMapCommon(
    const std::vector<std::string_view>& sortedUniquePrefixes) {
  smallPrefixes_.reserve(sortedUniquePrefixes.size() + 1);
  markers_.reserve(sortedUniquePrefixes.size() + 1);
  previousPrefix_.reserve(sortedUniquePrefixes.size());

  // total size
  {
    std::size_t size = std::transform_reduce(
        sortedUniquePrefixes.begin(),
        sortedUniquePrefixes.end(),
        std::size_t{0},
        std::plus<>{},
        [](const auto& x) { return x.size(); });
    if (size >= std::numeric_limits<std::uint32_t>::max()) {
      throw std::runtime_error(
          "too many chars for LowerBoundPrefixMap: " + std::to_string(size));
    }
    chars_.resize(size);
  }

  char* cur = chars_.data();
  markers_.push_back(0);

  // Adding an empty string with no matches.
  // This acts as a sentinel so we are always guranteed to find an
  // upper bound. If the user gives us an empty string, this still works.
  //
  // NOTE: this single element emplace is reasonably fast because the set is
  //       empty.
  smallPrefixes_.emplace(SmallPrefix(), IndexPair{0, 0});

  for (const auto& prefix : sortedUniquePrefixes) {
    // Prefixes always come before lexicographically, so if there is one
    // we will find it.
    previousPrefix_.push_back(findPrefix(prefix));

    // Add small prefix
    {
      std::uint32_t curPos = static_cast<std::uint32_t>(markers_.size() - 1);

      // NOTE: this single element emplace is reasonably fast,
      //       because it's in the end. Unfortunately, if we were
      //       to use a hint, we'd loose 'inserted' bool, which
      //       makes the code clumsier. If it becomes necessary this
      //       code can be written with vectors for better performance.
      auto [it, inserted] = smallPrefixes_.insert(
          {SmallPrefix{prefix}, IndexPair{curPos, curPos + 1}});

      if (!inserted) {
        // Prefixes match with the last one, so we need to
        // update the range.
        ++(it->second.second);
      }
    }

    cur = std::copy(prefix.begin(), prefix.end(), cur);
    markers_.push_back(static_cast<std::uint32_t>(cur - chars_.data()));
  }
  chars_.resize(cur - chars_.data());
}

std::uint32_t LowerBoundPrefixMapCommon::findPrefix(
    std::string_view query) const noexcept {
  // Due to a sentinel - guaranteed to not be .begin()
  auto lb = smallPrefixes_.upper_bound(SmallPrefix{query});
  auto [roughFrom, roughTo] = std::prev(lb)->second;

  // Binary search complete strings between rough boundaries.
  // NOTE: which array we search - doesn't matter -
  //       we just want indexes.
  auto cur = std::upper_bound(
                 markers_.begin() + roughFrom,
                 markers_.begin() + roughTo,
                 query,
                 [&](std::string_view q, const auto& m) {
                   auto i = static_cast<std::uint32_t>(&m - markers_.data());
                   return q < str(i);
                 }) -
      markers_.begin();

  while (cur != 0 && !query.starts_with(str(cur - 1))) {
    cur = previousPrefix_[cur - 1];
  }

  return cur;
}

} // namespace facebook::memcache::detail
