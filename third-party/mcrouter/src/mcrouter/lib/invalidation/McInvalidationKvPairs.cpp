/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/invalidation/McInvalidationKvPairs.h"

#include <cassert>
#include <optional>

#include <folly/Conv.h>
#include <folly/logging/xlog.h>

namespace facebook::memcache::invalidation {

using KeyValuePairs = McInvalidationKvPairs::KeyValuePairs;

KeyValuePairs McInvalidationKvPairs::createAxonKvPairs(
    std::string serialized,
    std::optional<std::string> region,
    std::optional<std::string> pool,
    std::optional<std::string> message,
    DistributionType type,
    DistributionOperation operation,
    std::optional<std::string> srcRegion) {
  XCHECK(!serialized.empty());

  KeyValuePairs kvPairs;
  kvPairs.emplace(kSerialized, serialized);
  kvPairs.emplace(kVersion, folly::to<std::string>(getMcInvalidationVersion()));
  if (region.has_value()) {
    kvPairs.emplace(kRegion, *region);
  }
  if (message.has_value()) {
    kvPairs.emplace(kMessage, *message);
  }
  if (pool.has_value()) {
    kvPairs.emplace(kPool, *pool);
  }

  kvPairs.emplace(kType, fmt::to_string(static_cast<uint8_t>(type)));

  kvPairs.emplace(kOperation, fmt::to_string(static_cast<uint8_t>(operation)));

  if (srcRegion.has_value()) {
    kvPairs.emplace(kSourceRegion, *srcRegion);
  }
  return kvPairs;
}

bool McInvalidationKvPairs::validateAxonKvPairs(
    const KeyValuePairs& keyValues) {
  auto serializedIt = keyValues.find(kSerialized);
  if (serializedIt == keyValues.end() || serializedIt->second.empty()) {
    XLOG_EVERY_N(WARNING, 1000) << "Missing key [serialized]";
    return false;
  }

  // compare versions, if lower than 1 version - return false
  auto versionIt = keyValues.find(kVersion);
  if (versionIt == keyValues.end()) {
    XLOG_EVERY_N(WARNING, 1000) << "Missing key [version]";
    return false;
  }
  if (folly::to<uint32_t>(versionIt->second) + 1 < getMcInvalidationVersion()) {
    XLOG_EVERY_N(WARNING, 1000) << folly::to<std::string>(
        "Version mismatch! expected: ",
        versionIt->second,
        ", actual: ",
        getMcInvalidationVersion());
    return false;
  }
  return true;
}
} // namespace facebook::memcache::invalidation
