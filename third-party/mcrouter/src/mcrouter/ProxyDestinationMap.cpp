/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProxyDestinationMap.h"

#include <memory>

#include <folly/Format.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/EventBase.h>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyDestination.h"
#include "mcrouter/ProxyDestinationBase.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/network/AccessPoint.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

size_t ProxyDestinationMap::DestHasher::operator()(
    const ProxyDestinationBase* dest) const {
  return ProxyDestinationKey(*dest).hash();
}

size_t ProxyDestinationMap::DestHasher::operator()(
    const ProxyDestinationKey& key) const {
  return key.hash();
}

bool ProxyDestinationMap::DestEq::operator()(
    const ProxyDestinationBase* x,
    const ProxyDestinationBase* y) const {
  return ProxyDestinationKey(*x) == ProxyDestinationKey(*y);
}
bool ProxyDestinationMap::DestEq::operator()(
    const ProxyDestinationKey& x,
    const ProxyDestinationBase* y) const {
  return x == ProxyDestinationKey(*y);
}

std::shared_ptr<PoolTkoTracker> ProxyDestinationMap::createPoolTkoTracker(
    std::string poolName,
    uint32_t numTkosFailOpenEnter,
    uint32_t numTkosFailOpenExit) {
  return proxy_->router().tkoTrackerMap().createPoolTkoTracker(
      poolName, numTkosFailOpenEnter, numTkosFailOpenExit);
}

struct ProxyDestinationMap::StateList {
  using List = folly::IntrusiveList<
      ProxyDestinationBase,
      &ProxyDestinationBase::stateListHook_>;
  List list;
};

ProxyDestinationMap::ProxyDestinationMap(ProxyBase* proxy)
    : proxy_(proxy),
      active_(std::make_unique<StateList>()),
      inactive_(std::make_unique<StateList>()),
      inactivityTimeout_(0) {}

void ProxyDestinationMap::removeDestination(ProxyDestinationBase& destination) {
  if (destination.stateList_ == active_.get()) {
    active_->list.erase(StateList::List::s_iterator_to(destination));
  } else if (destination.stateList_ == inactive_.get()) {
    inactive_->list.erase(StateList::List::s_iterator_to(destination));
  }
  {
    std::lock_guard<std::mutex> lck(destinationsLock_);
    destinations_.erase(&destination);
  }
}

void ProxyDestinationMap::markAsActive(ProxyDestinationBase& destination) {
  if (destination.stateList_ == active_.get()) {
    return;
  }
  if (destination.stateList_ == inactive_.get()) {
    inactive_->list.erase(StateList::List::s_iterator_to(destination));
  }
  active_->list.push_back(destination);
  destination.stateList_ = active_.get();
}

void ProxyDestinationMap::resetAllInactive() {
  for (auto& it : inactive_->list) {
    it.resetInactive();
    it.stateList_ = nullptr;
  }
  inactive_->list.clear();
  active_.swap(inactive_);
}

void ProxyDestinationMap::setResetTimer(std::chrono::milliseconds interval) {
  folly::RequestContextScopeGuard rctxGuard{
      std::shared_ptr<folly::RequestContext>{}};
  assert(interval.count() > 0);
  inactivityTimeout_ = static_cast<uint32_t>(interval.count());
  resetTimer_ =
      folly::AsyncTimeout::make(proxy_->eventBase(), [this]() noexcept {
        resetAllInactive();
        scheduleTimer(false /* initialAttempt */);
      });
  scheduleTimer(true /* initialAttempt */);
}

void ProxyDestinationMap::scheduleTimer(bool initialAttempt) {
  if (!resetTimer_->scheduleTimeout(inactivityTimeout_)) {
    MC_LOG_FAILURE(
        proxy_->router().opts(),
        memcache::failure::Category::kSystemError,
        "failed to {}schedule inactivity timer",
        initialAttempt ? "" : "re-");
  }
}

ProxyDestinationMap::~ProxyDestinationMap() {}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
