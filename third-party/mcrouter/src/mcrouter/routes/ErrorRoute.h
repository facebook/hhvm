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

#include <folly/Conv.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyRequestContextTyped.h"
#include "mcrouter/config-impl.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/AccessPoint.h"
#include "mcrouter/lib/network/RpcStatsContext.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Returns the error reply for each request right away
 */
template <class RouterInfo>
class ErrorRoute {
 public:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;

  std::string routeName() const {
    const std::string name = "error";
    const std::string log = enableLogging_ ? "log" : "no-log";
    const std::string res = carbon::resultToString(result_);
    const auto nameWithProps = folly::to<std::string>(name, "|", log, "|", res);
    if (valueToSet_.empty()) {
      return nameWithProps;
    }
    return folly::to<std::string>(nameWithProps, "|", valueToSet_);
  }

  template <class Request>
  bool traverse(const Request&, const RouteHandleTraverser<RouteHandleIf>&)
      const {
    return false;
  }

  explicit ErrorRoute(
      std::string valueToSet = "",
      bool enableLogging = true,
      carbon::Result result = carbon::Result::LOCAL_ERROR)
      : valueToSet_(std::move(valueToSet)),
        enableLogging_(enableLogging),
        result_(result) {}

  template <class Request>
  ReplyT<Request> route(const Request& req) {
    auto reply = createReply<Request>(ErrorReply, result_, valueToSet_);
    if (enableLogging_) {
      if (auto& ctx = fiber_local<RouterInfo>::getSharedCtx()) {
        auto now = nowUs();
        AccessPoint ap;
        RpcStatsContext rpcContext;
        ctx->onBeforeRequestSent(
            routeName() /* poolName */,
            ap,
            folly::StringPiece(),
            req,
            fiber_local<RouterInfo>::getRequestClass(),
            now);
        ctx->onReplyReceived(
            routeName() /* poolName */,
            std::nullopt,
            ap,
            folly::StringPiece(),
            req,
            reply,
            fiber_local<RouterInfo>::getRequestClass(),
            now,
            now,
            -1,
            rpcContext,
            fiber_local<RouterInfo>::getNetworkTransportTimeUs(),
            fiber_local<RouterInfo>::getExtraDataCallbacks());
      }
    }
    return reply;
  }

 private:
  const std::string valueToSet_;
  const bool enableLogging_;
  const carbon::Result result_;
};

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr createErrorRoute(
    std::string valueToSet,
    bool enableLogging = true,
    carbon::Result result = carbon::Result::LOCAL_ERROR) {
  return makeRouteHandleWithInfo<RouterInfo, ErrorRoute>(
      std::move(valueToSet), enableLogging, result);
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeErrorRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>&,
    const folly::dynamic& json) {
  checkLogic(
      json.isObject() || json.isString() || json.isNull(),
      "ErrorRoute: should be string or object");
  std::string response;
  bool enableLogging{true};
  carbon::Result result = carbon::Result::LOCAL_ERROR;

  if (json.isString()) {
    response = json.getString();
  } else if (json.isObject()) {
    if (auto jResponse = json.get_ptr("response")) {
      checkLogic(jResponse->isString(), "ErrorRoute: response is not a string");
      response = jResponse->getString();
    }

    if (auto jEnableLog = json.get_ptr("enable_logging")) {
      checkLogic(
          jEnableLog->isBool(), "ErrorRoute: enable_logginng is not boolean");
      enableLogging = jEnableLog->getBool();
    }

    if (auto jReplyResult = json.get_ptr("result")) {
      checkLogic(
          jReplyResult->isString(), "ErrorRoute: result is not a string");
      result = carbon::resultFromString(jReplyResult->getString().c_str());
      checkLogic(
          isErrorResult(result),
          "ErrorRoute: result {} (code {}) is not a valid error result.",
          jReplyResult->getString(),
          static_cast<int>(result));
    }
  }
  return createErrorRoute<RouterInfo>(
      std::move(response), enableLogging, result);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
