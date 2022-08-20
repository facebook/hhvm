/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/LatencyInjectionRoute.h"

#include <folly/Singleton.h>

namespace facebook::memcache::mcrouter {

namespace {
folly::Singleton<folly::ThreadWheelTimekeeperHighRes>
    timekeeperHighResSingleton_;
}

namespace detail {
folly::ReadMostlySharedPtr<folly::Timekeeper> getTimekeeperHighResSingleton() {
  return timekeeperHighResSingleton_.try_get_fast();
}
} // namespace detail

} // namespace facebook::memcache::mcrouter
