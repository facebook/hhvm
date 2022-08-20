/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <list>
#include <memory>
#include <vector>

#include <folly/Conv.h>
#include <folly/ScopeGuard.h>
#include <folly/fibers/Baton.h>

#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyRequestContext.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/options.h"

namespace folly {
struct dynamic;
}

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
class RouteHandleFactory;

namespace mcrouter {

/*
 * No more than N requests will be allowed to be concurrently processed by child
 * route. All blocked requests will be sent one request per sender id in
 * round-robin fashion to guarantee fairness.
 */
template <class RouterInfo>
class OutstandingLimitRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;

 public:
  std::string routeName() const {
    return folly::to<std::string>("outstanding-limit|limit=", maxOutstanding_);
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(*target_, req);
  }

  OutstandingLimitRoute(
      std::shared_ptr<RouteHandleIf> target,
      size_t maxOutstanding)
      : target_(std::move(target)), maxOutstanding_(maxOutstanding) {}

  template <class Request>
  ReplyT<Request> route(const Request& req) {
    if (outstanding_ == maxOutstanding_) {
      auto& ctx = fiber_local<RouterInfo>::getSharedCtx();
      auto senderId = ctx->senderId();
      auto& entry = [&]() -> QueueEntry& {
        auto entry_it = senderIdToEntry_.find(senderId);
        if (entry_it != senderIdToEntry_.end()) {
          return *entry_it->second;
        }
        blockedRequests_.push_back(std::make_unique<QueueEntry>(senderId));
        if (senderId) {
          senderIdToEntry_[senderId] = blockedRequests_.back().get();
        }
        return *blockedRequests_.back();
      }();

      auto& stats = ctx->proxy().stats();
      folly::fibers::Baton baton;
      int64_t waitingSince = 0;
      if (carbon::GetLike<Request>::value) {
        ++currentGetReqsWaiting_;
        waitingSince = nowUs();
      } else if (carbon::UpdateLike<Request>::value) {
        ++currentUpdateReqsWaiting_;
        waitingSince = nowUs();
      }
      entry.batons.push_back(&baton);
      baton.wait();
      if (waitingSince > 0) {
        if (carbon::GetLike<Request>::value) {
          stats.increment(
              outstanding_route_get_wait_time_sum_us_stat,
              static_cast<uint64_t>(nowUs() - waitingSince));
          stats.increment(
              outstanding_route_get_reqs_queued_helper_stat,
              currentGetReqsWaiting_);
          --currentGetReqsWaiting_;
          stats.increment(outstanding_route_get_reqs_queued_stat, 1);
        } else if (carbon::UpdateLike<Request>::value) {
          stats.increment(
              outstanding_route_update_wait_time_sum_us_stat,
              static_cast<uint64_t>(nowUs() - waitingSince));
          stats.increment(
              outstanding_route_update_reqs_queued_helper_stat,
              currentUpdateReqsWaiting_);
          --currentUpdateReqsWaiting_;
          stats.increment(outstanding_route_update_reqs_queued_stat, 1);
        }
      }
    } else {
      outstanding_++;
      assert(outstanding_ <= maxOutstanding_);
    }

    SCOPE_EXIT {
      if (!blockedRequests_.empty()) {
        auto entry = std::move(blockedRequests_.front());
        blockedRequests_.pop_front();

        assert(!entry->batons.empty());

        entry->batons.front()->post();
        entry->batons.pop_front();

        if (!entry->batons.empty()) {
          blockedRequests_.push_back(std::move(entry));
        } else {
          senderIdToEntry_.erase(entry->senderId);
        }
      } else {
        outstanding_--;
      }
    };

    return target_->route(req);
  }

 private:
  const std::shared_ptr<RouteHandleIf> target_;
  const size_t maxOutstanding_;
  size_t outstanding_{0};
  size_t currentGetReqsWaiting_{0};
  size_t currentUpdateReqsWaiting_{0};

  struct QueueEntry {
    QueueEntry(QueueEntry&&) = delete;
    QueueEntry& operator=(QueueEntry&&) = delete;

    explicit QueueEntry(size_t senderId_) : senderId(senderId_) {}
    size_t senderId;
    std::list<folly::fibers::Baton*> batons;
  };

  std::list<std::unique_ptr<QueueEntry>> blockedRequests_;
  std::unordered_map<size_t, QueueEntry*> senderIdToEntry_;
};

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf> makeOutstandingLimitRoute(
    std::shared_ptr<typename RouterInfo::RouteHandleIf> normalRoute,
    size_t maxOutstanding) {
  return makeRouteHandleWithInfo<RouterInfo, OutstandingLimitRoute>(
      std::move(normalRoute), maxOutstanding);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
