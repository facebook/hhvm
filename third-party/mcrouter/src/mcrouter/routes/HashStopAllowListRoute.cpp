/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/HashStopAllowListRoute.h"

#include <folly/Format.h>
#include <folly/fibers/WhenN.h>

namespace facebook::memcache::mcrouter {

HashStopAllowListRouteSettings parseHashStopAllowListRouteSettings(
    const folly::dynamic& json) {
  HashStopAllowListRouteSettings settings;
  auto jPrefixList = json.get_ptr("prefixes");
  if (jPrefixList && jPrefixList->isArray() && jPrefixList->size() > 0) {
    std::vector<std::pair<std::string, bool>> prefixes;
    for (size_t i = 0; i < jPrefixList->size(); ++i) {
      const auto& prefix = jPrefixList->at(i);
      checkLogic(
          prefix.isString(),
          "{} expected string, found {}",
          prefix,
          prefix.typeName());
      prefixes.emplace_back(prefix.asString(), true);
    }
    settings.prefixes = std::move(prefixes);
  }
  return settings;
}

} // namespace facebook::memcache::mcrouter
