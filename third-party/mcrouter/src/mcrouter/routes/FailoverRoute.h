/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/LeaseTokenMap.h"
#include "mcrouter/McReqUtil.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyRequestContextTyped.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/FailoverContext.h"
#include "mcrouter/lib/FailoverErrorsSettings.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/routes/FailoverPolicy.h"
#include "mcrouter/routes/FailoverRateLimiter.h"

namespace folly {
struct dynamic;
}

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
class RouteHandleFactory;

namespace mcrouter {

namespace detail {

// Append number of destinations to the name of the route handle.
// That's useful for us to just failover LeaseSets when then number of
// failover targets didn't change.
inline std::string getFailoverRouteName(std::string name, size_t numTargets) {
  if (name.empty()) {
    return "";
  }
  return folly::to<std::string>(
      std::move(name), ":failover_targets=", numTargets);
}

} // namespace detail

/**
 * Sends the same request sequentially to each destination in the list in order,
 * until the first non-error reply.  If all replies result in errors, returns
 * the last destination's reply.
 */
template <
    class RouterInfo,
    typename FailoverPolicyT,
    typename FailoverErrorsSettingsT = FailoverErrorsSettings>
class FailoverRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;

 public:
  std::string routeName() const {
    if (name_.empty()) {
      return "failover";
    }
    return "failover:" + name_;
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    if (fiber_local<RouterInfo>::getFailoverDisabled()) {
      return t(*targets_[0], req); // normal route
    }
    auto policyCtx = failoverPolicy_.context(req);
    auto iter = failoverPolicy_.cbegin(req, policyCtx);
    // normal route
    // This must be called here so that selectedIndex is set and the failover
    // iterator below does not select the index from normal route.
    if (t(*targets_[0], req)) {
      return true;
    }
    std::vector<std::shared_ptr<RouteHandleIf>> failovers;
    ++iter;
    while (iter != failoverPolicy_.cend(req, policyCtx)) {
      std::shared_ptr<RouteHandleIf> rh = targets_[iter.getTrueIndex()];
      failovers.push_back(rh);
      ++iter;
    }
    return t(failovers, req);
  }

  FailoverRoute(
      std::vector<std::shared_ptr<RouteHandleIf>> targets,
      FailoverErrorsSettingsT failoverErrors,
      std::unique_ptr<FailoverRateLimiter> rateLimiter,
      bool failoverTagging,
      bool enableLeasePairing,
      std::string name,
      const folly::dynamic& policyConfig)
      : name_(detail::getFailoverRouteName(std::move(name), targets.size())),
        targets_(std::move(targets)),
        failoverErrors_(std::move(failoverErrors)),
        rateLimiter_(std::move(rateLimiter)),
        failoverTagging_(failoverTagging),
        failoverPolicy_(targets_, policyConfig),
        enableLeasePairing_(enableLeasePairing) {
    assert(!targets_.empty());
    assert(!enableLeasePairing_ || !name_.empty());
  }

  virtual ~FailoverRoute() {}

  template <class Request>
  ReplyT<Request> route(const Request& req) {
    return doRoute(req);
  }

  McLeaseSetReply route(const McLeaseSetRequest& req) {
    if (!enableLeasePairing_) {
      return doRoute(req);
    }

    // Look into LeaseTokenMap
    auto& proxy = fiber_local<RouterInfo>::getSharedCtx()->proxy();
    auto& map = proxy.router().leaseTokenMap();
    if (auto item = map.query(name_, *req.leaseToken_ref())) {
      auto mutReq = req;
      mutReq.leaseToken_ref() = item->originalToken;
      proxy.stats().increment(redirected_lease_set_count_stat);
      if (targets_.size() <= item->routeHandleChildIndex) {
        McLeaseSetReply errorReply(carbon::Result::LOCAL_ERROR);
        errorReply.message_ref() =
            "LeaseSet failover destination out-of-range.";
        return errorReply;
      }
      return targets_[item->routeHandleChildIndex]->route(mutReq);
    }

    // If not found in the map, send to normal destiantion (don't failover)
    return targets_[0]->route(req);
  }

  McLeaseGetReply route(const McLeaseGetRequest& req) {
    size_t childIndex = 0;
    auto reply = doRoute(req, childIndex);
    const auto leaseToken = *reply.leaseToken_ref();

    if (!enableLeasePairing_ || leaseToken <= 1) {
      return reply;
    }

    auto& map = fiber_local<RouterInfo>::getSharedCtx()
                    ->proxy()
                    .router()
                    .leaseTokenMap();

    // If the lease token returned by the underlying route handle conflicts
    // with special tokens space, we need to store it in the map even if we
    // didn't failover.
    if (childIndex > 0 || map.conflicts(leaseToken)) {
      auto specialToken =
          map.insert(name_, {static_cast<uint64_t>(leaseToken), childIndex});
      reply.leaseToken_ref() = specialToken;
    }

    return reply;
  }

 protected:
  const std::string name_;

 private:
  const std::vector<std::shared_ptr<RouteHandleIf>> targets_;
  const FailoverErrorsSettingsT failoverErrors_;
  std::unique_ptr<FailoverRateLimiter> rateLimiter_;
  const bool failoverTagging_{false};
  FailoverPolicyT failoverPolicy_;
  const bool enableLeasePairing_{false};

  template <class Request>
  inline ReplyT<Request> doRoute(const Request& req) {
    size_t tmp;
    return doRoute(req, tmp);
  }

  template <class Request, class Iterator>
  bool processReply(
      const ReplyT<Request>& reply,
      const Request& req,
      bool& conditionalFailover,
      Iterator& iter,
      FailoverPolicyContext& ctx) {
    auto res = shouldFailover(reply, req);
    if (FOLLY_LIKELY(res == FailoverErrorsSettingsBase::FailoverType::NONE)) {
      return true;
    }
    if (res == FailoverErrorsSettingsBase::FailoverType::CONDITIONAL) {
      conditionalFailover = true;
    }

    auto& proxy = fiber_local<RouterInfo>::getSharedCtx()->proxy();
    if (isErrorResult(*reply.result_ref())) {
      if (!isTkoOrHardTkoResult(*reply.result_ref())) {
        proxy.stats().increment(failover_policy_result_error_stat);
      } else {
        proxy.stats().increment(failover_policy_tko_error_stat);
        // If it is Tko or Hard Tko set the failure domain so that
        // we do not pick next failover target from the same failure domain
        if (reply.destination()) {
          iter.setFailedDomain(reply.destination()->getFailureDomain());
        }
      }
    }
    if (!isTkoOrHardTkoResult(*reply.result_ref())) {
      if (rateLimiter_ && !rateLimiter_->failoverAllowed()) {
        proxy.stats().increment(failover_rate_limited_stat);
        return true;
      }
      // We didn't do any work for TKO or hard TKO. Don't count it as a try.
      if (!failoverPolicy_.excludeError(*reply.result_ref())) {
        ++ctx.numTries_;
      }
    }

    return false;
  }

  template <class Request>
  ReplyT<Request> doRoute(const Request& req, size_t& childIndex) {
    auto& proxy = fiber_local<RouterInfo>::getSharedCtx()->proxy();

    bool conditionalFailover = false;
    bool allFailed = false;
    SCOPE_EXIT {
      if (conditionalFailover) {
        proxy.stats().increment(failover_conditional_stat);
        proxy.stats().increment(failover_conditional_count_stat);
      }
      if (allFailed) {
        proxy.stats().increment(failover_all_failed_stat);
        proxy.stats().increment(failover_all_failed_count_stat);
        proxy.stats().increment(failoverPolicy_.getFailoverFailedStat());
      }
    };

    auto policyCtx = failoverPolicy_.context(req);
    auto iter = failoverPolicy_.begin(req, policyCtx);
    auto normalReply = iter->route(req);
    if (rateLimiter_) {
      rateLimiter_->bumpTotalReqs();
    }
    if (fiber_local<RouterInfo>::getSharedCtx()->failoverDisabled()) {
      return normalReply;
    }
    if (failoverTagging_) {
      setFailoverHopCount(normalReply, getFailoverHopCount(req));
    }
    if (FOLLY_LIKELY(processReply(
            normalReply, req, conditionalFailover, iter, policyCtx))) {
      return normalReply;
    }
    if (++iter == failoverPolicy_.end(req, policyCtx)) {
      if (isErrorResult(*normalReply.result_ref())) {
        allFailed = true;
      }
      return normalReply;
    }
    proxy.stats().increment(failover_all_stat);
    proxy.stats().increment(failoverPolicy_.getFailoverStat());

    childIndex = 0;
    // Failover
    return fiber_local<RouterInfo>::runWithLocals([this,
                                                   iter,
                                                   &req,
                                                   &proxy,
                                                   &normalReply,
                                                   &policyCtx,
                                                   &childIndex,
                                                   &allFailed,
                                                   &conditionalFailover]() {
      fiber_local<RouterInfo>::addRequestClass(RequestClass::kFailover);
      auto doFailover = [this, &req, &proxy, &normalReply](auto& child) {
        uint32_t cnt =
            failoverTagging_ ? fiber_local<RouterInfo>::incFailoverCount() : 0;
        auto failoverReply = child->route(req);
        if (failoverTagging_) {
          setFailoverHopCount(failoverReply, getFailoverHopCount(req) + cnt);
        }
        FailoverContext failoverContext(
            child.getTrueIndex(),
            targets_.size() - 1,
            req,
            failoverPolicy_.getFailureDomainsEnabled(),
            normalReply,
            failoverReply);
        logFailover(proxy, failoverContext);
        carbon::setIsFailoverIfPresent(failoverReply, true);
        return failoverReply;
      };

      auto cur = iter;
      // set the index of the child that generated the reply.
      SCOPE_EXIT {
        childIndex = cur.getTrueIndex();
      };
      auto nx = cur;
      // Passive iterator does not do routing
      nx.setPassive();

      auto incFailureDomainStat =
          [&proxy](ReplyT<Request>& nReply, ReplyT<Request>& fReply) {
            if (nReply.destination() && fReply.destination() &&
                (nReply.destination()->getFailureDomain() ==
                 fReply.destination()->getFailureDomain())) {
              proxy.stats().increment(failover_same_failure_domain_stat, 1);
            }
          };

      ReplyT<Request> failoverReply = normalReply;
      for (++nx; nx != failoverPolicy_.end(req, policyCtx) &&
           policyCtx.numTries_ < failoverPolicy_.maxErrorTries();
           ++cur, ++nx) {
        failoverReply = doFailover(cur);
        incFailureDomainStat(normalReply, failoverReply);
        if (FOLLY_LIKELY(processReply(
                failoverReply, req, conditionalFailover, cur, policyCtx))) {
          return failoverReply;
        }
      }
      if (policyCtx.numTries_ < failoverPolicy_.maxErrorTries()) {
        failoverReply = doFailover(cur);
        incFailureDomainStat(normalReply, failoverReply);
        if (isErrorResult(*failoverReply.result_ref())) {
          allFailed = true;
        }
      }
      proxy.stats().increment(
          failover_num_collisions_stat, cur.getStats().num_collisions);
      proxy.stats().increment(
          failover_num_failed_domain_collisions_stat,
          cur.getStats().num_failed_domain_collisions);

      return failoverReply;
    });
  }

  template <class Request>
  FailoverErrorsSettingsBase::FailoverType shouldFailover(
      const ReplyT<Request>& reply,
      const Request& req) const {
    return failoverErrors_.shouldFailover(reply, req);
  }
};

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf> makeFailoverRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    ExtraRouteHandleProviderIf<RouterInfo>& extraProvider);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "mcrouter/routes/FailoverRoute-inl.h"
