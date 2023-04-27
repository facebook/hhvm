/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>

#include <folly/fibers/FiberManager.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/routes/McrouterRouteHandle.h"
#include "mcrouter/routes/RoutingUtils.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * This route handle allows for substantial changes in the number of boxes in
 * a pool without increasing the miss rate and, subsequently, the load on the
 * underlying storage or service.
 *
 * get: send the request to "cold" route handle and in case of a miss,
 *     fetch data from the "warm" route handle. If "warm" returns a hit,
 *     the response is then forwarded to the client and an asynchronous 'add'
 *     request updates the value in the "cold" route handle.
 * gets: send the request to "cold" route handle and in case of a miss,
 *     fetch data from the "warm" route handle with simple 'get' request.
 *     If "warm" returns a hit, synchronously try to add value to "cold"
 *     using 'add' operation and send original 'gets' request to "cold" one
 *     more time.
 * lease get: send the request to "cold" route and in case of a miss
 *     (not hot miss!) fetch data from the "warm" using simple 'get' request.
 *     If "warm" returns a hit, the response is forwarded to the client and
 *     an asynchronous lease set updates the value in the cold route handle.
 * metaget: send the request to "cold" route and in case of a miss, send
 *     request to "warm".
 * set/delete/incr/decr/etc.: send to the "cold" route, do not modify "warm".
 *     Client is responsible for "warm" consistency.
 * gat/gats: send the request to "cold" route handle and in case of a miss,
 *     fetch data from the "warm" route handle with simple 'get' request.
 *     If "warm" returns a hit, synchronously try to add value to "cold"
 *     using 'add' operation and send original 'gat' or 'gats' request
 *     to "cold" one more time.
 *
 * Expiration time (TTL) for automatic warm -> cold update requests is
 * configured with "exptime" field. If the field is not present and
 * "enable_metaget" is true, exptime is fetched from "warm" on every update
 * operation with additional 'metaget' request.
 */
template <class RouteHandleIf>
class WarmUpRoute {
 public:
  static std::string routeName() {
    return "warm-up";
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    if (t(*cold_, req)) {
      return true;
    }
    return t(*warm_, req);
  }

  WarmUpRoute(
      std::shared_ptr<RouteHandleIf> warm,
      std::shared_ptr<RouteHandleIf> cold,
      folly::Optional<uint32_t> exptime)
      : warm_(std::move(warm)),
        cold_(std::move(cold)),
        exptime_(std::move(exptime)) {
    assert(warm_ != nullptr);
    assert(cold_ != nullptr);
  }

  //////////////////////////////// get /////////////////////////////////////
  McGetReply route(const McGetRequest& req) {
    auto coldReply = cold_->route(req);
    if (isHitResult(*coldReply.result_ref())) {
      return coldReply;
    }

    /* else */
    auto warmReply = warm_->route(req);
    uint32_t exptime = 0;
    if (isHitResult(*warmReply.result_ref()) &&
        getExptimeForCold(req, exptime)) {
      folly::fibers::addTask(
          [cold = cold_,
           addReq = createRequestFromMessage<McAddRequest>(
               req.key_ref()->fullKey(), warmReply, exptime)]() {
            cold->route(addReq);
          });
    }
    return warmReply;
  }

  ///////////////////////////////metaget//////////////////////////////////
  McMetagetReply route(const McMetagetRequest& req) {
    auto coldReply = cold_->route(req);
    if (isHitResult(*coldReply.result_ref())) {
      return coldReply;
    }
    return warm_->route(req);
  }

  /////////////////////////////lease_get//////////////////////////////////
  McLeaseGetReply route(const McLeaseGetRequest& req) {
    auto coldReply = cold_->route(req);
    if (isHitResult(*coldReply.result_ref()) ||
        isHotMissResult(*coldReply.result_ref())) {
      // in case of a hot miss somebody else will set the value
      return coldReply;
    }

    // miss with lease token from cold route: send simple get to warm route
    McGetRequest reqOpGet(req.key_ref()->fullKey());
    auto warmReply = warm_->route(reqOpGet);
    uint32_t exptime = 0;
    if (isHitResult(*warmReply.result_ref()) &&
        getExptimeForCold(reqOpGet, exptime)) {
      // update cold route with lease set
      auto setReq = createRequestFromMessage<McLeaseSetRequest>(
          reqOpGet.key_ref()->fullKey(), warmReply, exptime);
      setReq.leaseToken_ref() = *coldReply.leaseToken_ref();

      folly::fibers::addTask(
          [cold = cold_, req = std::move(setReq)]() { cold->route(req); });
      // On hit, no need to copy appSpecificErrorCode or message
      McLeaseGetReply reply(*warmReply.result_ref());
      reply.flags_ref() = *warmReply.flags_ref();
      reply.value_ref().move_from(warmReply.value_ref());
      return reply;
    }
    return coldReply;
  }

  ////////////////////////////////gets////////////////////////////////////
  McGetsReply route(const McGetsRequest& req) {
    auto coldReply = cold_->route(req);
    if (isHitResult(*coldReply.result_ref())) {
      return coldReply;
    }

    // miss: send simple get to warm route
    McGetRequest reqGet(req.key_ref()->fullKey());
    auto warmReply = warm_->route(reqGet);
    uint32_t exptime = 0;
    if (isHitResult(*warmReply.result_ref()) &&
        getExptimeForCold(req, exptime)) {
      // update cold route if we have the value
      auto addReq = createRequestFromMessage<McAddRequest>(
          req.key_ref()->fullKey(), warmReply, exptime);
      cold_->route(addReq);
      // and grab cas token again
      return cold_->route(req);
    }
    return coldReply;
  }

  //////////////////////////////// gat /////////////////////////////////////
  McGatReply route(const McGatRequest& req) {
    auto coldReply = cold_->route(req);
    if (isHitResult(*coldReply.result_ref())) {
      return coldReply;
    }

    // miss: send simple get to warm route
    McGetRequest reqGet(req.key_ref()->fullKey());
    auto warmReply = warm_->route(reqGet);
    if (isHitResult(*warmReply.result_ref())) {
      // update cold route if we have the value
      auto addReq = createRequestFromMessage<McAddRequest>(
          req.key_ref()->fullKey(), warmReply, *req.exptime_ref());
      cold_->route(addReq);
      return cold_->route(req);
    }
    return coldReply;
  }

  ////////////////////////////////gats////////////////////////////////////
  McGatsReply route(const McGatsRequest& req) {
    auto coldReply = cold_->route(req);
    if (isHitResult(*coldReply.result_ref())) {
      return coldReply;
    }

    // miss: send simple get to warm route
    McGetRequest reqGet(req.key_ref()->fullKey());
    auto warmReply = warm_->route(reqGet);
    if (isHitResult(*warmReply.result_ref())) {
      // update cold route if we have the value
      auto addReq = createRequestFromMessage<McAddRequest>(
          req.key_ref()->fullKey(), warmReply, *req.exptime_ref());
      cold_->route(addReq);
      // and grab cas token again
      return cold_->route(req);
    }
    return coldReply;
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    // client is responsible for consistency of warm route, do not replicate
    // any update/delete operations
    return cold_->route(req);
  }

 private:
  const std::shared_ptr<RouteHandleIf> warm_;
  const std::shared_ptr<RouteHandleIf> cold_;
  const folly::Optional<uint32_t> exptime_;

  template <class Request>
  bool getExptimeForCold(const Request& req, uint32_t& exptime) {
    // if an exptime is defined by the configs, we will just use
    // that value. otherwise, we will get it from the warm side.
    if (exptime_.hasValue()) {
      exptime = *exptime_;
      return true;
    }
    return getExptimeFromRoute<RouteHandleIf>(
        warm_, req.key_ref()->fullKey(), exptime);
  }
};

McrouterRouteHandlePtr makeWarmUpRoute(
    RouteHandleFactory<McrouterRouteHandleIf>& factory,
    const folly::dynamic& json);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
