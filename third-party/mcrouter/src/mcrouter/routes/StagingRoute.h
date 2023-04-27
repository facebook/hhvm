/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <array>

#include <folly/Optional.h>
#include <folly/fibers/AddTasks.h>
#include <folly/fibers/FiberManager.h>
#include <folly/fibers/ForEach.h>

#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/fbi/cpp/FuncGenerator.h"
#include "mcrouter/lib/fbi/cpp/globals.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheRouteHandleIf.h"
#include "mcrouter/routes/McrouterRouteHandle.h"
#include "mcrouter/routes/RoutingUtils.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * This route handle is similar to WarmUpRoute that will allow a new pool to be
 * staged before moving into production. The difference in this route is that
 * it will always return the response from the warm route side and as quickly
 * as possible.  It will also attempt to synchronize the access pattern between
 * the 2 routes.
 *
 * Operations supported:
 *
 * "get" / "lease get":
 * Route the get request to the warm route. If it is a hit,
 * then send a metaget to the staging side.
 * This has 2 effects: it allows us to know if the object is already there
 * and also allows the synchronize the LRU position of the object for the
 * staging host.
 * In the case of carbon::Result::BUSY, we will bail and will not try
 * to add to the staging host to support partial staging.
 * Otherwise: on a hit, we will be done and no action further.
 *            on a miss, we will do an add operation to the staging host.
 *
 * "gets":
 * Routes "gets" operations to the warm route and translate to a
 * metaget on a staging side to synchronize the LRU.
 *
 * "set" / "lease set" / "cas":
 * This routes all sets to the warm route and if it is stored,
 * we will route to the staging route side asynchroniously as a normal set.
 *
 * "delete":
 * Route to both warm and staging routes simutaneously and wait for both
 * replies. We then return the worse reply from both. For deletes, we want to
 * ensure both copies are deleted.
 *
 * For all other operations:
 * We wil send to warm route and we async send the same operation to staging
 * route.
 *
 * Expiration time (TTL) for objects from warm -> staging update requests is
 * calculated based on the object's metadata retreived from warm route async.
 */
class StagingRoute {
 public:
  static std::string routeName() {
    return "staging";
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<MemcacheRouteHandleIf>& t) const {
    if (t(*warm_, req)) {
      return true;
    }
    return t(*staging_, req);
  }

  StagingRoute(
      std::shared_ptr<MemcacheRouteHandleIf> warm,
      std::shared_ptr<MemcacheRouteHandleIf> staging)
      : warm_(std::move(warm)), staging_(std::move(staging)) {
    assert(warm_ != nullptr);
    assert(staging_ != nullptr);
  }

  /**
   * Get operations
   * Route the get request to the warm route. If it is a hit,
   * then send a metaget to the staging side.
   * This has 2 effects: it allows us to know if the object is already there
   * and also attempts to synchronize the LRU position of the object for the
   * staging host.
   * In the case of carbon::Result::BUSY, we will bail and will not try
   * to add to the staging host to support partial staging.
   * Otherwise: on a hit, we will be done and no action further.
   *            on a miss, we will do an add operation to the staging host.
   * Always return the warm route's reply.
   */
  template <class Request>
  typename std::enable_if<
      folly::IsOneOf<Request, McGetRequest, McLeaseGetRequest>::value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    auto reply = warm_->route(req);

    if (isHitResult(*reply.result_ref())) {
      folly::fibers::addTask([req, reply, warm = warm_, staging = staging_]() {
        McMetagetRequest reqMetaget(req.key_ref()->fullKey());
        auto metaReply = staging->route(reqMetaget);
        if (*metaReply.result_ref() != carbon::Result::BUSY &&
            !isHitResult(*metaReply.result_ref())) {
          // Add to the staging side if we don't get a busy or a miss.
          // We will have to retrieve the exptime from the warm side through
          // a metaget and use the reply from the warm side for the value.
          uint32_t calculatedExptime = 0;
          if (getExptimeFromRoute(
                  warm, req.key_ref()->fullKey(), calculatedExptime)) {
            auto addReq = createRequestFromMessage<McAddRequest>(
                req.key_ref()->fullKey(), reply, calculatedExptime);
            staging->route(addReq);
          }
        }
      });
    }

    // always return warm reply
    return reply;
  }

  /**
   * "Gets" operation
   * This routes all "gets" operations to the warm route and translate to a
   * metaget on a staging side to synchronize the LRU.
   * Always return the warm reply.
   */
  template <class Request>
  typename std::enable_if<
      folly::IsOneOf<Request, McGetsRequest>::value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    // start with routing to warm
    auto reply = warm_->route(req);

    // route a metaget to staging side if we have a hit
    if (isHitResult(*reply.result_ref())) {
      folly::fibers::addTask(
          [staging = staging_,
           metaReq = McMetagetRequest(req.key_ref()->fullKey())] {
            staging->route(metaReq);
          });
    }

    // always return warm reply
    return reply;
  }

  /**
   * Set operations
   * This routes all sets to the warm route and if it is stored,
   * we will route to the staging route side asynchroniously as a normal set.
   * We will always reply with the warm route's reply.
   */
  template <class Request>
  typename std::enable_if<
      folly::IsOneOf<Request, McSetRequest, McLeaseSetRequest, McCasRequest>::
          value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    // start with routing to warm
    auto reply = warm_->route(req);

    // Set the data to the staging side as a normal set using the
    // original request value.
    int32_t exptime = *req.exptime_ref();
    if (isStoredResult(*reply.result_ref())) {
      folly::fibers::addTask(
          [staging = staging_,
           setReq = createRequestFromMessage<McSetRequest, Request>(
               req.key_ref()->fullKey(), req, exptime)]() {
            staging->route(setReq);
          });
    }

    // always return warm reply
    return reply;
  }

  /**
   * All sync-like (delete) operations:
   * This handles the cases where we want to send to both routes
   * simutaneously and wait for both replies. We then return
   * the worse reply from both. For deletes, we want to ensure both
   * copies are deleted.
   */
  template <class Request>
  typename std::enable_if<
      folly::IsOneOf<Request, McDeleteRequest>::value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    using Reply = ReplyT<Request>;

    // send to both routes simutanously and return the worse reply.
    std::array<std::function<Reply()>, 2> fs;
    fs[0] = [warm = warm_, &req]() { return warm->route(req); };
    fs[1] = [staging = staging_, &req]() { return staging->route(req); };

    Reply reply;
    bool bFirstReply = true;
    folly::fibers::forEach(
        fs.begin(),
        fs.end(),
        [&reply, &bFirstReply](size_t /* id */, Reply newReply) {
          if (bFirstReply ||
              worseThan(*newReply.result_ref(), *reply.result_ref())) {
            reply = std::move(newReply);
            bFirstReply = false;
          }
        });
    return reply;
  }

  /**
   * For all other operations:
   * We wil send to warm route and we async send the same operation to staging
   * route.
   */
  template <class Request>
  typename std::enable_if<
      !folly::IsOneOf<
          Request,
          McGetRequest,
          McGetsRequest,
          McLeaseGetRequest,
          McSetRequest,
          McLeaseSetRequest,
          McCasRequest,
          McDeleteRequest>::value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    auto reply = warm_->route(req);
    if (!isErrorResult(*reply.result_ref())) {
      folly::fibers::addTask(
          [staging = staging_, asyncReq = req]() { staging->route(asyncReq); });
    }

    return reply;
  }

 private:
  // route configuration
  const std::shared_ptr<MemcacheRouteHandleIf> warm_;
  const std::shared_ptr<MemcacheRouteHandleIf> staging_;
};

McrouterRouteHandlePtr makeStagingRoute(
    RouteHandleFactory<MemcacheRouteHandleIf>& factory,
    const folly::dynamic& json);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
