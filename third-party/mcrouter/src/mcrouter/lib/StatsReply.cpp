/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "StatsReply.h"

#include <folly/io/IOBuf.h>

#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {

McStatsReply StatsReply::getReply() {
  /**
   * In the 'stats' IOBuf, we store the string representation returned to
   * clients, e.g.,
   * "STAT stat1 value1\r\nSTAT stat2 value2\r\n..."
   */

  McStatsReply reply(carbon::Result::OK);
  std::vector<std::string> statsList;

  for (const auto& s : stats_) {
    statsList.emplace_back(
        folly::to<std::string>("STAT ", s.first, ' ', s.second));
  }

  reply.stats_ref() = std::move(statsList);

  return reply;
}
} // namespace memcache
} // namespace facebook
