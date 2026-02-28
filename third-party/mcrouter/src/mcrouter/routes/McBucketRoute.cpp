/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/McBucketRoute.h"
#include <cstdint>

namespace facebook {
namespace memcache {
namespace mcrouter {
namespace detail {
std::vector<std::pair<std::string, Ch3HashFunc>> buildPrefixMap(
    const std::unordered_map<std::string, uint64_t>& map) {
  std::vector<std::pair<std::string, Ch3HashFunc>> result;
  for (const auto& [prefix, bucket] : map) {
    result.emplace_back(prefix, Ch3HashFunc(bucket));
  }
  return result;
}
} // namespace detail

McBucketRouteSettings parseMcBucketRouteSettings(const folly::dynamic& json) {
  McBucketRouteSettings settings;
  checkLogic(
      json.count("total_buckets"), "McBucketRoute: no total number of buckets");
  auto totalBuckets = parseInt(
      *json.get_ptr("total_buckets"),
      "total_buckets",
      1,
      std::numeric_limits<int64_t>::max());

  auto jBucketsByPrefix = json.get_ptr("total_buckets_by_prefix");
  if (jBucketsByPrefix && jBucketsByPrefix->isObject()) {
    std::unordered_map<std::string, uint64_t> prefixToBuckets;
    for (const auto& it : jBucketsByPrefix->items()) {
      checkLogic(
          it.first.isString(),
          "{} expected string, found {}",
          it.first,
          it.first.typeName());
      checkLogic(
          it.second.isInt(),
          "{} expected int, found {}",
          it.second,
          it.second.typeName());
      auto buckets = it.second.asInt();
      checkLogic(
          0 < buckets && buckets <= totalBuckets,
          "Bucket count should be in range (0, {}], got {}",
          totalBuckets,
          buckets);
      prefixToBuckets[it.first.asString()] = it.second.asInt();
    }
    settings.prefixToBuckets = std::move(prefixToBuckets);
  }

  auto* bucketizationKeyspacePtr = json.get_ptr("bucketization_keyspace");
  checkLogic(
      bucketizationKeyspacePtr && bucketizationKeyspacePtr->isString(),
      "McBucketRoute: must have bucketization_keyspace");
  settings.bucketizationKeyspace = bucketizationKeyspacePtr->getString();

  if (auto jSalt = json.get_ptr("salt")) {
    checkLogic(jSalt->isString(), "McBucketRoute: salt is not a string");
    settings.salt = jSalt->getString();
  }

  settings.totalBuckets = totalBuckets;
  return settings;
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
