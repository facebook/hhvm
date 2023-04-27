/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>

#include <folly/Random.h>
#include <folly/fibers/EventBaseLoopController.h>

#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyDestinationMap.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
ProxyBase::ProxyBase(
    CarbonRouterInstanceBase& rtr,
    size_t id,
    folly::VirtualEventBase& evb,
    RouterInfo /* tag */)
    : router_(rtr),
      id_(id),
      eventBase_(evb),
      fiberManager_(
          typename fiber_local<RouterInfo>::ContextTypeTag(),
          std::make_unique<folly::fibers::EventBaseLoopController>(),
          getFiberManagerOptions(router_.opts())),
      asyncLog_(router_.opts()),
      stats_(router_.getStatsEnabledPools()),
      flushCallback_(*this),
      destinationMap_(std::make_unique<ProxyDestinationMap>(this)) {
  eventBase_.runInEventBaseThread([]() { isProxyThread_ = true; });
  // Setup a full random seed sequence
  folly::Random::seed(randomGenerator_);

  statsContainer_ = std::make_unique<ProxyStatsContainer>(*this);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
