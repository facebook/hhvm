/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/dynamic.h>

#include "mcrouter/lib/fbi/cpp/ParsingUtil.h"

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
template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeMcBucketRoute(
    typename RouterInfo::RouteHandlePtr rh,
    const folly::dynamic& json) {
  auto settings = parseMcBucketRouteSettings(json);
  return makeRouteHandleWithInfo<RouterInfo, McBucketRoute>(
      std::move(rh), settings);
}
} // namespace facebook::memcache::mcrouter
