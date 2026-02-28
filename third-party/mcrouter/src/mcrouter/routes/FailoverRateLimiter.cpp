/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FailoverRateLimiter.h"

#include <folly/json/dynamic.h>

#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

const double kDefaultBurst = 1000;

folly::TokenBucket tbFromJson(const folly::dynamic& json) {
  checkLogic(
      json.isObject(), "FailoverRateLimiter: failover limit is not an object");
  auto jRate = json.get_ptr("rate");
  checkLogic(jRate != nullptr, "FailoverRateLimiter: rate not found");
  checkLogic(jRate->isNumber(), "FailoverRateLimiter: rate is not a number");
  double rate = jRate->asDouble();
  rate = std::min(std::max(rate, 0.0), 1.0);
  double burst = kDefaultBurst;
  if (auto jBurst = json.get_ptr("burst")) {
    checkLogic(
        jBurst->isNumber(), "FailoverRateLimiter: burst is not a number");
    burst = jBurst->asDouble();
    burst = std::max(burst, 1.0);
  }
  return {rate, burst, /* allow `burst` requests at time 0 */ -1e6};
}

} // namespace

FailoverRateLimiter::FailoverRateLimiter(const folly::dynamic& json)
    : tb_(tbFromJson(json)) {}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
