/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <limits>
#include <memory>
#include <string>
#include <utility>

#include <folly/Range.h>
#include <folly/fibers/FiberManager.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/mc/msg.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
McGetReply L1L2SizeSplitRoute<RouterInfo>::route(
    const McGetRequest& req) const {
  // Set flag on the read path. Server will only return back sentinel values
  // when this flag is present in order to accommodate old clients.
  // TODO It's probably fine to const_cast and avoid the copy here
  const auto l1ReqWithFlag = [&req]() {
    auto r = req;
    r.flags_ref() = *r.flags_ref() | MC_MSG_FLAG_SIZE_SPLIT;
    return r;
  }();
  auto reply = l1_->route(l1ReqWithFlag);
  if (isHitResult(*reply.result_ref()) &&
      (*reply.flags_ref() & MC_MSG_FLAG_SIZE_SPLIT)) {
    const auto l1Value = coalesceAndGetRange(reply.value_ref());
    // Real value lives in the L2 pool. If L1 reply has a non-empty value, then
    // we need to route to L2 using key suffixed with |==|<rand>
    if (!l1Value.empty()) {
      const auto l2ReqWithKeyAndSuffix =
          McGetRequest(makeL2Key(req.key_ref()->fullKey(), l1Value));
      auto l2Reply = l2_->route(l2ReqWithKeyAndSuffix);

      if (isHitResult(*l2Reply.result_ref())) {
        McGetReply l2ReplyWithHitResult(carbon::Result::FOUND);
        l2ReplyWithHitResult.flags_ref() = *l2Reply.flags_ref();
        l2ReplyWithHitResult.value_ref().move_from(l2Reply.value_ref());
        return l2ReplyWithHitResult;
      }

      if (isErrorResult(*l2Reply.result_ref())) {
        if (isFailoverErrorResult(*l2Reply.result_ref())) {
          deleteSentinel(req, l1Value);
        }
        return McGetReply(*l2Reply.result_ref());
      } else {
        deleteSentinel(req, l1Value);
        return McGetReply(carbon::Result::LOCAL_ERROR);
      }
    }
    reply = l2_->route(req);
  }
  // If we didn't get a sentinel value, pass along
  return reply;
}

template <class RouterInfo>
McSetReply L1L2SizeSplitRoute<RouterInfo>::route(
    const McSetRequest& req) const {
  if (fullSetShouldGoToL1(req)) {
    return l1_->route(req);
  }

  // Value is large enough to warrant split sets
  if (bothFullSet_) {
    auto l1Reply = l1_->route(req);
    if (isStoredResult(*l1Reply.result_ref())) {
      folly::fibers::addTask([l2 = l2_, req]() { l2->route(req); });
    }
    return l1Reply;
  } else {
    // Store to L2 first to avoid race
    auto reply = l2_->route(req);
    if (isStoredResult(*reply.result_ref())) {
      // Set key to L1 with empty value and special sentinel flag
      const auto l1Sentinel = [&req]() {
        auto r = req;
        r.value_ref() = folly::IOBuf();
        r.flags_ref() = *r.flags_ref() | MC_MSG_FLAG_SIZE_SPLIT;
        return r;
      }();
      return l1_->route(l1Sentinel);
    }
    return reply;
  }
}

template <class RouterInfo>
folly::Optional<McLeaseGetReply> L1L2SizeSplitRoute<RouterInfo>::doFilter(
    const McLeaseGetReply& reply) const {
  folly::Optional<McLeaseGetReply> ret;
  constexpr uint64_t kHotMissToken = 1;
  // We got an L1 sentinel, but a nonzero lease token. Note that we may convert
  // a stale hit on a sentinel to a regular lease miss or hot miss.
  if (static_cast<uint64_t>(*reply.leaseToken_ref()) >= kHotMissToken) {
    McLeaseGetReply r(carbon::Result::NOTFOUND);
    r.leaseToken_ref() = *reply.leaseToken_ref();
    ret.assign(std::move(r));
  }
  return ret;
}

template <class RouterInfo>
template <class Reply>
folly::Optional<Reply> L1L2SizeSplitRoute<RouterInfo>::doFilter(
    const Reply& /* unused */) const {
  return folly::none;
}

template <class RouterInfo>
template <class Request>
typename std::enable_if<
    folly::IsOneOf<Request, McLeaseGetRequest, McGetsRequest>::value,
    ReplyT<Request>>::type
L1L2SizeSplitRoute<RouterInfo>::doRoute(const Request& req, size_t retriesLeft)
    const {
  // Set flag on the read path. Server will only return back sentinel values
  // when this flag is present in order to accommodate old clients.
  const auto l1ReqWithFlag = [&req]() {
    auto r = req;
    r.flags_ref() = *r.flags_ref() | MC_MSG_FLAG_SIZE_SPLIT;
    return r;
  }();
  auto l1Reply = l1_->route(l1ReqWithFlag);

  // If we didn't receive a sentinel value, return immediately.
  if (!(*l1Reply.flags_ref() & MC_MSG_FLAG_SIZE_SPLIT)) {
    return l1Reply;
  }

  // Operation specific filtering on reply from L1
  if (auto optReply = doFilter(l1Reply)) {
    return optReply.value();
  }

  // At this point, we got a non-stale sentinel hit from L1.
  const auto l1Value = coalesceAndGetRange(l1Reply.value_ref());
  const auto l2Req = l1Value.empty()
      ? McGetRequest(req.key_ref()->fullKey())
      : McGetRequest(makeL2Key(req.key_ref()->fullKey(), l1Value));

  auto l2Reply = l2_->route(l2Req);
  if (isHitResult(*l2Reply.result_ref())) {
    ReplyT<Request> reply(carbon::Result::FOUND);
    reply.flags_ref() = *l2Reply.flags_ref();
    reply.value_ref().move_from(l2Reply.value_ref());
    augmentReply(reply, l1Reply);
    return reply;
  }

  if (isErrorResult(*l2Reply.result_ref())) {
    if (isFailoverErrorResult(*l2Reply.result_ref())) {
      deleteSentinel(req, l1Value);
    }
    return ReplyT<Request>(*l2Reply.result_ref());
  } else if (retriesLeft != 0) {
    deleteSentinel(req, l1Value);
    return doRoute(req, retriesLeft - 1);
  } else {
    return ReplyT<Request>(carbon::Result::LOCAL_ERROR);
  }
}

template <class RouterInfo>
McLeaseSetReply L1L2SizeSplitRoute<RouterInfo>::route(
    const McLeaseSetRequest& req) const {
  if (fullSetShouldGoToL1(req)) {
    return l1_->route(req);
  }

  if (bothFullSet_) {
    auto adjustedReq = [&]() {
      auto r = req;
      r.key_ref() = makeL2Key(
          req.key_ref()->fullKey(), folly::to<std::string>(randomGenerator_()));
      return r;
    }();
    auto l1Reply = l1_->route(adjustedReq);

    if (isStoredResult(*l1Reply.result_ref())) {
      auto makeL2Req = [&]() {
        McSetRequest r(adjustedReq.key_ref()->fullKey());
        r.flags_ref() = *req.flags_ref();
        r.exptime_ref() = *req.exptime_ref();
        r.value_ref() = std::move(*adjustedReq.value_ref());
        return r;
      };
      folly::fibers::addTask(
          [l2 = l2_, l2Req = makeL2Req()] { l2->route(l2Req); });
    }
    return l1Reply;
  } else {
    // Generate a random integer that will be used as the value for the L1 item
    // and will be mixed into the key for L2.
    const auto randInt = randomGenerator_();
    auto l2SetReq = [&]() {
      McSetRequest r(makeL2Key(req.key_ref()->fullKey(), randInt));
      r.value_ref() = *req.value_ref();
      r.flags_ref() = *req.flags_ref();
      r.exptime_ref() = *req.exptime_ref();
      return r;
    }();
    auto l2Reply = l2_->route(l2SetReq);

    if (isStoredResult(*l2Reply.result_ref())) {
      const auto l1SentinelReq = [&]() {
        auto r = req;
        r.flags_ref() = *r.flags_ref() | MC_MSG_FLAG_SIZE_SPLIT;
        r.value_ref() = folly::IOBuf(
            folly::IOBuf::CopyBufferOp(), folly::to<std::string>(randInt));
        return r;
      }();

      return l1_->route(l1SentinelReq);
    } else {
      // If L2 is failing, cut the exptime down and store full value to L1.
      const auto l1FallbackReq = [&]() {
        auto r = req;
        if (*r.exptime_ref() > failureTtl_ || *r.exptime_ref() == 0) {
          r.exptime_ref() = failureTtl_;
        }
        return r;
      }();
      return l1_->route(l1FallbackReq);
    }
  }
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeL1L2SizeSplitRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "L1L2SizeSplitRoute should be an object");
  checkLogic(json.count("l1"), "L1L2SizeSplitRoute: no l1 route");
  checkLogic(json.count("l2"), "L1L2SizeSplitRoute: no l2 route");
  checkLogic(json.count("threshold"), "L1L2SizeSplitRoute: no threshold");
  checkLogic(
      json["threshold"].isInt(),
      "L1L2SizeSplitRoute: threshold is not an integer");
  size_t threshold = json["threshold"].getInt();

  int32_t ttlThreshold = 0;
  if (json.count("ttl_threshold")) {
    checkLogic(
        json["ttl_threshold"].isInt(),
        "L1L2SizeSplitRoute: ttl_threshold is not an integer");
    ttlThreshold = json["ttl_threshold"].getInt();
    checkLogic(
        ttlThreshold >= 0,
        "L1L2SizeSplitRoute: ttl_threshold must be nonnegative");
  }

  int32_t failureTtl = 60;
  if (json.count("failure_ttl")) {
    checkLogic(
        json["failure_ttl"].isInt(),
        "L1L2SizeSplitRoute: failure_ttl is not an integer");
    failureTtl = json["failure_ttl"].getInt();
    checkLogic(
        failureTtl >= 0, "L1L2SizeSplitRoute: failure_ttl must be nonnegative");
    checkLogic(
        failureTtl != 0, "L1L2SizeSplitRoute: failure_ttl must not be zero");
  }

  bool bothFullSet = false;
  if (json.count("both_full_set")) {
    checkLogic(
        json["both_full_set"].isBool(),
        "L1L2SizeSplitRoute: both_full_set is not a boolean");
    bothFullSet = json["both_full_set"].getBool();
  }

  uint32_t numRetries = L1L2SizeSplitRoute<RouterInfo>::kDefaultNumRetries;
  if (json.count("retries")) {
    checkLogic(
        json["retries"].isInt(),
        "L1L2SizeSplitRoute: number of retries is not an integer");
    numRetries = json["retries"].getInt();
    checkLogic(
        numRetries > 0, "L1L2SizeSplitRoute: number of retries must be > 0");
    checkLogic(
        numRetries <= L1L2SizeSplitRoute<RouterInfo>::kMaxNumRetries,
        "L1L2SizeSplitRoute: maximum number of retries is " +
            std::to_string(L1L2SizeSplitRoute<RouterInfo>::kMaxNumRetries));
  }

  return makeRouteHandleWithInfo<RouterInfo, L1L2SizeSplitRoute>(
      factory.create(json["l1"]),
      factory.create(json["l2"]),
      threshold,
      ttlThreshold,
      failureTtl,
      bothFullSet,
      numRetries);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
