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

#pragma once

#include <string>
#include <unordered_set>
#include <vector>

namespace wangle {

struct TLSTicketKeySeeds {
  std::vector<std::string> oldSeeds;
  std::vector<std::string> currentSeeds;
  std::vector<std::string> newSeeds;

  bool operator==(const TLSTicketKeySeeds& rhs) const {
    return (
        oldSeeds == rhs.oldSeeds && currentSeeds == rhs.currentSeeds &&
        newSeeds == rhs.newSeeds);
  }

  bool isValidRotation(const TLSTicketKeySeeds& next) const {
    // First branch corresponds to not having any ticket seeds, and then
    // adding ticket seeds for the first time. The second branch corresponds to
    // a compatible ticket seed update. The third branch is the case of setting
    // a subset of the ticket seeds a second time
    return (isEmpty() && next.isNotEmpty()) ||
        (areSeedsSubset(newSeeds, next.currentSeeds) &&
         areSeedsSubset(currentSeeds, next.oldSeeds)) ||
        (areSeedsSubset(oldSeeds, next.oldSeeds) &&
         areSeedsSubset(currentSeeds, next.currentSeeds) &&
         areSeedsSubset(newSeeds, next.newSeeds));
  }

  bool isEmpty() const {
    return oldSeeds.empty() && currentSeeds.empty() && newSeeds.empty();
  }

  bool isNotEmpty() const {
    return !oldSeeds.empty() && !currentSeeds.empty() && !newSeeds.empty();
  }

  // Verify the LHS is a subset of RHS
  static bool areSeedsSubset(
      const std::vector<std::string>& lhs,
      const std::vector<std::string>& rhs) {
    if (lhs.size() > rhs.size()) {
      return false;
    }
    std::unordered_set<std::string> a{rhs.cbegin(), rhs.cend()};
    for (const auto& v :
         std::unordered_set<std::string>{lhs.cbegin(), lhs.cend()}) {
      if (a.find(v) == a.end()) {
        return false;
      }
    }
    return true;
  }
};

} // namespace wangle
