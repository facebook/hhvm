/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <unordered_map>

namespace facebook::common::mysql_client {

template <typename Key>
class StringStore {
 public:
  using ValueGenerator = std::function<std::string()>;
  const std::string& getString(Key&& key, const ValueGenerator& gen) {
    // See if we already have a string for the specified key
    if (auto it = map_.find(key); it != map_.end()) {
      // If so, return it.
      return it->second;
    }

    // If not, create a new string and insert it into the map.
    auto [it, _] = map_.emplace(std::move(key), gen());
    return it->second;
  }

 private:
  // Whatever map we use for this must guarantee reference stability as
  // `getString` returns a reference to values in the map.
  std::unordered_map<Key, std::string> map_;
};

} // namespace facebook::common::mysql_client
