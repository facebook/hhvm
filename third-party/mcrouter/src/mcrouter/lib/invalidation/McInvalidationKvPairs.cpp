/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/invalidation/McInvalidationKvPairs.h"

#include <cassert>

#include <folly/Conv.h>
#include <folly/logging/xlog.h>

namespace facebook::memcache::invalidation {

using KeyValuePairs = McInvalidationKvPairs::KeyValuePairs;

KeyValuePairs McInvalidationKvPairs::createAxonKvPairs(
    const std::string& serialized,
    std::optional<std::string> regionOpt,
    std::optional<std::string> poolOpt,
    std::optional<std::string> messageOpt) {
  XCHECK(!serialized.empty());

  KeyValuePairs kvPairs;
  kvPairs.emplace(kSerialized, serialized);
  kvPairs.emplace(kVersion, folly::to<std::string>(getMcInvalidationVersion()));
  if (regionOpt.has_value()) {
    kvPairs.emplace(kRegion, *regionOpt);
  }
  if (messageOpt.has_value()) {
    kvPairs.emplace(kMessage, *messageOpt);
  }
  if (poolOpt.has_value()) {
    kvPairs.emplace(kPool, *poolOpt);
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
