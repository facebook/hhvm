/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <vector>

#include <folly/Conv.h>

namespace facebook {
namespace memcache {

class McStatsReply;

class StatsReply {
 public:
  template <typename V>
  void addStat(folly::StringPiece name, V&& value) {
    stats_.emplace_back(
        name.str(), folly::to<std::string>(std::forward<V>(value)));
  }

  McStatsReply getReply();

 private:
  std::vector<std::pair<std::string, std::string>> stats_;
};
} // namespace memcache
} // namespace facebook
