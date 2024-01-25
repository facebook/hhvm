/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Conv.h>
#include <folly/Function.h>
#include <folly/Synchronized.h>
#include <folly/dynamic.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

/* Create struct of external stats */
#define STAT(name, ...)
#define STUI STAT
#define STSI STAT
#define STSS STAT
#define EXTERNAL_STAT(name) uint64_t name{0};
struct ExternalStatsData {
#include "stat_list.h"
  operator std::unordered_map<std::string, uint64_t>() const;
  folly::dynamic dump(const bool filterZeroes) const;
};
#undef STAT
#undef STUI
#undef STSI
#undef STSS
#undef EXTERNAL_STAT

class ExternalStatsHandler final {
 public:
  ExternalStatsHandler() = default;

  using ExternalStatsCallback = folly::Function<void(ExternalStatsData&) const>;

  void registerExternalStats(
      const std::string& category,
      ExternalStatsCallback cb);

  std::unordered_map<std::string, uint64_t> getStats() const;
  folly::dynamic dumpStats(const bool filterZeros = false) const;

  ExternalStatsHandler& operator=(const ExternalStatsHandler&) = delete;
  ExternalStatsHandler(const ExternalStatsHandler&) = delete;

 private:
  folly::Synchronized<std::unordered_map<std::string, ExternalStatsCallback>>
      externalStatsCallback_;

  void visit(ExternalStatsData& data) const;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
