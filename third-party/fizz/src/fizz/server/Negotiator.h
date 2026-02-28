/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <vector>

#include <folly/Optional.h>

namespace fizz {
namespace server {

/**
 * Negotiate a parameter given a list of server preference tiers and a list of
 * client preferences. Within a server preference tier the client preference
 * will be respected.
 */
template <typename T>
folly::Optional<T> negotiate(
    const std::vector<std::vector<T>>& serverPref,
    const std::vector<T>& clientPref) {
  for (const auto& prefTier : serverPref) {
    if (prefTier.size() == 1) {
      if (std::find(clientPref.begin(), clientPref.end(), prefTier.front()) !=
          clientPref.end()) {
        return prefTier.front();
      }
    } else {
      for (const auto& pref : clientPref) {
        if (std::find(prefTier.begin(), prefTier.end(), pref) !=
            prefTier.end()) {
          return pref;
        }
      }
    }
  }
  return folly::none;
}

/**
 * Negotatiate a parameter given the server's preference. Client preference is
 * ignored.
 */
template <typename T>
folly::Optional<T> negotiate(
    const std::vector<T>& serverPref,
    const std::vector<T>& clientPref) {
  for (const auto& pref : serverPref) {
    if (std::find(clientPref.begin(), clientPref.end(), pref) !=
        clientPref.end()) {
      return pref;
    }
  }
  return folly::none;
}
} // namespace server
} // namespace fizz
