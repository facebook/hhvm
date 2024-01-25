/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/ExternalStatsHandler.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

#define STAT(name, ...)
#define STUI STAT
#define STSI STAT
#define STSS STAT
#define EXTERNAL_STAT(name) {#name, name},
ExternalStatsData::operator std::unordered_map<std::string, uint64_t>() const {
  return {
#include "stat_list.h"
  };
}
#undef STAT
#undef STUI
#undef STSI
#undef STSS
#undef EXTERNAL_STAT

folly::dynamic ExternalStatsData::dump(const bool filterZeroes) const {
  const std::unordered_map<std::string, uint64_t> statsResult = *this;
  folly::dynamic result(folly::dynamic::object());
  for (const auto& kv : statsResult) {
    const auto& value = kv.second;
    if (filterZeroes && value == 0) {
      continue;
    }
    result[kv.first] = value;
  }
  return result;
}

void ExternalStatsHandler::registerExternalStats(
    const std::string& category,
    ExternalStatsCallback cb) {
  auto locked = externalStatsCallback_.wlock();
  /*
   * Try inserting a new callback.
   * If it exists already, ignore it and move on
   */
  locked->emplace(category, std::move(cb));
}

void ExternalStatsHandler::visit(ExternalStatsData& data) const {
  const auto locked = externalStatsCallback_.rlock();
  for (const auto& categoryCb : *locked) {
    categoryCb.second(data);
  }
}

std::unordered_map<std::string, uint64_t> ExternalStatsHandler::getStats()
    const {
  ExternalStatsData data;
  visit(data);
  return data;
}

folly::dynamic ExternalStatsHandler::dumpStats(const bool filterZeroes) const {
  ExternalStatsData data;
  visit(data);
  return data.dump(filterZeroes);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
