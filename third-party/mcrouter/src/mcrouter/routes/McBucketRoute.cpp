/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/McBucketRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {
McBucketRouteSettings parseMcBucketRouteSettings(const folly::dynamic& json) {
  McBucketRouteSettings settings;
  checkLogic(
      json.count("total_buckets"), "McBucketRoute: no total number of buckets");
  auto totalBuckets = parseInt(
      *json.get_ptr("total_buckets"),
      "total_buckets",
      1,
      std::numeric_limits<int64_t>::max());

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
