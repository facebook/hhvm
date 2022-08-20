/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/hash/Hash.h>
#include <memory>
#include <string>

#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyRequestContext.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/CarbonMessageConversionUtils.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/routes/ErrorRoute.h"

namespace folly {
struct dynamic;
}

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Use userIpAddress to hash request among children
 * Optional offset param to pick different child
 * Requires --retain_source_ip option to be true
 */
template <class RouterInfo>
class OriginalClientHashRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;

 public:
  static std::string routeName() {
    return "OriginalClientHashRoute";
  }

  explicit OriginalClientHashRoute(
      std::vector<std::shared_ptr<RouteHandleIf>> children,
      uint64_t offset)
      : children_(std::move(children)), offset_(offset) {}

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    if (children_.empty()) {
      return false;
    }

    auto& ctx = mcrouter::fiber_local<RouterInfo>::getSharedCtx();
    auto& ip = ctx->userIpAddress();
    if (!ip.empty()) {
      return t(
          *children_[(folly::Hash()(ip) + offset_) % children_.size()], req);
    }
    return false;
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) {
    ReplyT<Request> reply;
    assert(children_.size() > 1);

    auto& ctx = mcrouter::fiber_local<RouterInfo>::getSharedCtx();
    auto& ip = ctx->userIpAddress();
    if (!ip.empty()) {
      return children_[(folly::Hash()(ip) + offset_) % children_.size()]->route(
          req);
    } else {
      return createReply<Request>(
          ErrorReply, carbon::Result::LOCAL_ERROR, "Missing IP");
    }
  }

 private:
  const std::vector<std::shared_ptr<RouteHandleIf>> children_;
  uint64_t offset_{0};
};

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf>
createOriginalClientHashRoute(
    std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>> children,
    uint64_t offset) {
  return makeRouteHandleWithInfo<RouterInfo, OriginalClientHashRoute>(
      std::move(children), offset);
}

/**
 *
 * Config example:
 *  {
 *    "children": [
 *      "child1",
 *      "child2"
 *    ],
 *    "offset": 0
 *  }
 */
template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf> makeOriginalClientHashRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  auto jChildren = json.get_ptr("children");
  checkLogic(
      jChildren != nullptr && jChildren->isArray(),
      "OriginalClientHashRoute: The 'children' must be specified.");
  auto children = factory.createList(*jChildren);
  checkLogic(
      children.size() > 0,
      "OriginalClientHashRoute: At least one 'children' is required");

  if (children.size() == 1) {
    return std::move(children[0]);
  }

  auto jOffset = json.get_ptr("offset");
  uint64_t offset = 0;
  if (jOffset) {
    checkLogic(
        jOffset->isInt() && jOffset->getInt() >= 0 &&
            jOffset->getInt() < static_cast<int64_t>(children.size()),
        "OriginalClientHashRoute: The 'offset' must be non-negative integer and less than size of 'children'.");
    offset = jOffset->getInt();
  }

  return createOriginalClientHashRoute<RouterInfo>(std::move(children), offset);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
