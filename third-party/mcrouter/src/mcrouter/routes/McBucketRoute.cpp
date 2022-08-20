/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/McBucketRoute.h"

#include <folly/dynamic.h>

#include "mcrouter/lib/fbi/cpp/ParsingUtil.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"

namespace facebook::memcache::mcrouter {

/*
 *   McBucketRoute Config:
 * - bucketize(bool)        - enable the bucketization
 * - total_buckets(int)     - total number of buckets
 * - bucketize_until(int)   - enable the handle for buckets until (exclusive)
 *                            this number. Must be less than total_buckets.
 *                            Needed for gradual migration.
 * - bucketization_keyspace - can be used to separate into multiple
 *                            bucketization domains decoupled from each other,
 *                            e.g. one keyspace for each Memache pool.
 */
std::shared_ptr<MemcacheRouteHandleIf> makeMcBucketRoute(
    std::shared_ptr<MemcacheRouteHandleIf> rh,
    const folly::dynamic& json) {
  McBucketRouteSettings settings;
  checkLogic(
      json.count("total_buckets"), "McBucketRoute: no total number of buckets");
  auto totalBuckets = parseInt(
      *json.get_ptr("total_buckets"),
      "total_buckets",
      1,
      std::numeric_limits<int64_t>::max());

  auto bucketizeUntil = 0;
  if (json.count("bucketize_until")) {
    bucketizeUntil = parseInt(
        *json.get_ptr("bucketize_until"), "bucketize_until", 0, totalBuckets);
  }

  auto* bucketizationKeyspacePtr = json.get_ptr("bucketization_keyspace");
  checkLogic(
      bucketizationKeyspacePtr && bucketizationKeyspacePtr->isString(),
      "McBucketRoute: must have bucketization_keyspace");

  if (auto jSalt = json.get_ptr("salt")) {
    checkLogic(jSalt->isString(), "McBucketRoute: salt is not a string");
    settings.salt = jSalt->getString();
  }

  settings.totalBuckets = totalBuckets;
  settings.bucketizeUntil = bucketizeUntil;
  return std::make_shared<MemcacheRouteHandle<McBucketRoute>>(
      std::move(rh), settings);
}
} // namespace facebook::memcache::mcrouter
