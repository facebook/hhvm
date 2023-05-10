/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <fstream>
#include <string>
#include <utility>

#include <folly/Conv.h>
#include <folly/Range.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/HashUtil.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/WeightedCh3HashFunc.h"
#include "mcrouter/lib/routes/NullRoute.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
class McRefillRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  std::string routeName() const {
    return "McRefillRoute";
  }

  /**
   * Constructs McRefillRoute.
   */
  McRefillRoute(RouteHandlePtr primary, RouteHandlePtr refill)
      : primary_(primary), refill_(refill) {
    assert(primary_ != nullptr);
    assert(refill_ != nullptr);
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    if constexpr (!carbon::ListContains<
                      typename memcache::MemcacheRouterInfo::AllRequests,
                      Request>::value) {
      return false;
    }
    if (t(*primary_, req)) {
      return true;
    }
    return t(*refill_, req);
  }

  McLeaseGetReply route(const McLeaseGetRequest& req) const {
    constexpr size_t kLeaseHotMissToken = 1;
    auto reply = primary_->route(req);
    if (isMissResult(*reply.result_ref()) &&
        *reply.leaseToken_ref() != kLeaseHotMissToken) {
      // send request to prod
      auto refillReply = refill_->route(genGetRequest(req));
      if (isHitResult(*refillReply.result_ref())) {
        McMetagetRequest metaGet(req.key_ref()->fullKey());
        auto metaReply = refill_->route(metaGet);
        if (isHitResult(*metaReply.result_ref())) {
          folly::fibers::addTask(
              [this,
               setReq = genSetRequest(req, reply, refillReply, metaReply)]() {
                primary_->route(setReq);
              });
        }
      }
    }
    return reply;
  }

  template <class Request>
  ReplyT<Request> route(const Request& req, carbon::GetLikeT<Request> = 0)
      const {
    auto reply = primary_->route(req);
    if (isMissResult(*reply.result_ref())) {
      // send request to prod
      auto refillReply = refill_->route(genGetRequest(req));
      if (isHitResult(*refillReply.result_ref())) {
        McMetagetRequest metaGet(req.key_ref()->fullKey());
        auto metaReply = refill_->route(metaGet);
        if (isHitResult(*metaReply.result_ref())) {
          folly::fibers::addTask(
              [this,
               setReq = genSetRequest(req, reply, refillReply, metaReply)]() {
                primary_->route(setReq);
              });
        }
      }
    }
    return reply;
  }

  template <class Request>
  ReplyT<Request> route(
      const Request& req,
      carbon::OtherThanT<Request, carbon::GetLike<>> = 0) const {
    return primary_->route(req);
  }

 private:
  const std::shared_ptr<RouteHandleIf> primary_;
  const std::shared_ptr<RouteHandleIf> refill_;

  McGetRequest genGetRequest(const McGetRequest& req) const {
    return req;
  }

  template <class Request>
  McGetRequest genGetRequest(const Request& req, carbon::GetLikeT<Request> = 0)
      const {
    McGetRequest getReq(req.key_ref()->fullKey());
    return getReq;
  }

  McLeaseSetRequest genSetRequest(
      const McLeaseGetRequest& primaryReq,
      const McLeaseGetReply& primaryReply,
      const McGetReply& refillReply,
      const McMetagetReply& metaReply) const {
    McLeaseSetRequest sreq(primaryReq.key_ref()->fullKey());
    sreq.value_ref() = *refillReply.value_ref();
    sreq.flags_ref() = *refillReply.flags_ref();
    sreq.exptime_ref() = *metaReply.exptime_ref();
    sreq.leaseToken_ref() = *primaryReply.leaseToken_ref();
    return sreq;
  }

  template <class Request>
  McSetRequest genSetRequest(
      const Request& primaryReq,
      const ReplyT<Request>& /* unused */,
      const McGetReply& refillReply,
      const McMetagetReply& metaReply,
      carbon::GetLikeT<Request> = 0) const {
    McSetRequest sreq(primaryReq.key_ref()->fullKey());
    sreq.value_ref() = *refillReply.value_ref();
    sreq.flags_ref() = *refillReply.flags_ref();
    sreq.exptime_ref() = *metaReply.exptime_ref();
    return sreq;
  }
};

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeMcRefillRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "McRefillRoute is not an object");
  checkLogic(json.count("primary"), "McRefillRoute: no primary route");
  checkLogic(json.count("refill"), "McRefillRoute: no refill route");

  return makeRouteHandleWithInfo<RouterInfo, McRefillRoute>(
      factory.create(json["primary"]), factory.create(json["refill"]));
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
