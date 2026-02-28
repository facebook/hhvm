/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>

#include "mcrouter/lib/fbi/counting_sem.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

class CarbonRouterClientBase {
 public:
  CarbonRouterClientBase(
      size_t maximumOutstanding,
      bool maximumOutstandingError);

  virtual ~CarbonRouterClientBase() = default;

  /**
   * Unique client id. Ids are not re-used for the lifetime of the process.
   */
  uint64_t clientId() const {
    return clientId_;
  }

  /**
   * Maximum allowed requests in flight (unlimited if 0)
   */
  unsigned int maxOutstanding() const {
    return maxOutstanding_;
  }

  /**
   * If true, error is immediately returned when maxOutstanding_ limit is hit
   * If false, sender thread is blocked when maxOutstanding_ limit is hit
   */
  bool maxOutstandingError() const {
    return maxOutstandingError_;
  }

  counting_sem_t* outstandingReqsSem() {
    return &outstandingReqsSem_;
  }

 private:
  /**
   * Automatically-assigned client id, used for QOS for different clients
   * sharing the same connection.
   */
  uint64_t clientId_;

  /// Maximum allowed requests in flight (unlimited if 0)
  const unsigned int maxOutstanding_;
  /// If true, error is immediately returned when maxOutstanding_ limit is hit
  /// If false, sender thread is blocked when maxOutstanding_ limit is hit
  const bool maxOutstandingError_;
  counting_sem_t outstandingReqsSem_;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
