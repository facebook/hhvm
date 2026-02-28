/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/TokenBucket.h>

#include "mcrouter/lib/carbon/RoutingGroups.h"

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * This is a container for TokenBucket rate limiters for different
 * operation types.
 */
class RateLimiter {
 public:
  /**
   * @param json  Rate limiting configuration; must be an object. Format:
   *
   *              { "gets_rate": GR, "gets_burst": GB,
   *                "sets_rate": SR, "sets_burst": GB,
   *                "deletes_rate": DR, "deletes_burst": DB }
   *
   *              Where rate and burst parameters are passed to
   *              the corresponding TokenBucket's constructor.
   *              If some *_rate key is missing, no rate limiting is
   *              performed for that operation.
   *              If some *_burst key is missing, burst is set
   *              equal to rate.
   */
  explicit RateLimiter(const folly::dynamic& json);

  template <class Request>
  bool canPassThrough(carbon::GetLikeT<Request> = 0) {
    return FOLLY_LIKELY(
        !getsTb_ ||
        getsTb_->consume(1.0, folly::TokenBucket::defaultClockNow()));
  }

  template <class Request>
  bool canPassThrough(carbon::UpdateLikeT<Request> = 0) {
    return FOLLY_LIKELY(
        !setsTb_ ||
        setsTb_->consume(1.0, folly::TokenBucket::defaultClockNow()));
  }

  template <class Request>
  bool canPassThrough(carbon::DeleteLikeT<Request> = 0) {
    return FOLLY_LIKELY(
        !deletesTb_ ||
        deletesTb_->consume(1.0, folly::TokenBucket::defaultClockNow()));
  }

  template <class Request>
  bool canPassThrough(
      carbon::OtherThanT<
          Request,
          carbon::GetLike<>,
          carbon::UpdateLike<>,
          carbon::DeleteLike<>> = 0) {
    return true;
  }

  /**
   * String representation useful for debugging
   */
  std::string toDebugStr() const;

 private:
  folly::Optional<folly::TokenBucket> getsTb_;
  folly::Optional<folly::TokenBucket> setsTb_;
  folly::Optional<folly::TokenBucket> deletesTb_;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
