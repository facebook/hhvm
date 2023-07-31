/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Format.h>
#include <folly/Range.h>

#include "mcrouter/config.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

constexpr folly::StringPiece kRouteName = "ClientCompatibilityChecker";

template <typename Request, typename = std::enable_if_t<true>>
class HasClientVersionTrait : public std::false_type {};
template <typename Request>
class HasClientVersionTrait<
    Request,
    std::void_t<
        decltype(std::declval<std::decay_t<Request>&>().clientVersion_ref())>>
    : public std::true_type {};

/**
 * Compares the client version on the request to the maximum version in
 * the protocol. If the client version is newer, then the request is
 * rejected due to likely risk of data loss from protocol incompatibility.
 */
template <class RouterInfo>
class ClientCompatibilityCheckerRoute {
 public:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

  ClientCompatibilityCheckerRoute(
      RouteHandlePtr rh,
      uint32_t maxSupportedClientVersion)
      : rh_(std::move(rh)),
        maxSupportedClientVersion_(maxSupportedClientVersion) {}

  std::string routeName() const {
    return fmt::format(
        "{}|maxSupportedClientVersion={}",
        kRouteName.str(),
        maxSupportedClientVersion_);
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(*rh_, req);
  }

  template <class Request>
  std::enable_if_t<HasClientVersionTrait<Request>::value, ReplyT<Request>>
  route(const Request& req) const {
    if (static_cast<uint32_t>(*req.clientVersion_ref()) >=
        maxSupportedClientVersion_) {
      return createReply<Request>(
          ErrorReply,
          carbon::Result::CLIENT_ERROR,
          folly::sformat(
              "Requests from newer client with version {} are not supported. "
              "Maximum supported client version is {}",
              static_cast<uint32_t>(*req.clientVersion_ref()),
              maxSupportedClientVersion_ - 1));
    }
    return rh_->route(req);
  }

  template <class Request>
  std::enable_if_t<!HasClientVersionTrait<Request>::value, ReplyT<Request>>
  route(const Request& req) const {
    return rh_->route(req);
  }

 private:
  const RouteHandlePtr rh_;
  const uint32_t maxSupportedClientVersion_;
};

/**
 * Creates a ClientCompatibilityCheckerRoute from a json config.
 *
 * Sample json:
 * {
 *   "type": "ClientCompatibilityCheckerRoute",
 *   "child": "PoolRoute|pool_name",
 *   "max_supported_client_version": 10,
 * }
 */
template <class RouterInfo>
inline typename RouterInfo::RouteHandlePtr makeClientCompatibilityCheckerRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    const std::optional<uint32_t> maxSupportedClientVersion = std::nullopt) {
  checkLogic(
      json.isObject(),
      "ClientCompatibilityCheckerRoute: config is not an object.");

  auto jChild = json.get_ptr("child");
  checkLogic(
      jChild != nullptr,
      "ClientCompatibilityCheckerRoute: 'child' property is missing.");
  auto child = factory.create(*jChild);

  uint32_t maxClientVersion = 0;
  if (maxSupportedClientVersion) {
    maxClientVersion = *maxSupportedClientVersion;
  } else {
    auto jMaxSupportedClientVersion =
        json.get_ptr("max_supported_client_version");
    checkLogic(
        jMaxSupportedClientVersion != nullptr,
        "ClientCompatibilityCheckerRoute: 'max_supported_client_version' property is missing");
    maxClientVersion = jMaxSupportedClientVersion->asInt();
  }
  return makeRouteHandleWithInfo<RouterInfo, ClientCompatibilityCheckerRoute>(
      std::move(child), maxClientVersion);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
