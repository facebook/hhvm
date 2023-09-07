/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <utility>

#include <folly/fibers/FiberManager.h>

#include "mcrouter/routes/McBucketRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo, class ShadowPolicy>
template <class Request>
void ShadowRoute<RouterInfo, ShadowPolicy>::dispatchShadowRequest(
    std::shared_ptr<typename RouterInfo::RouteHandleIf> shadow,
    std::shared_ptr<Request> adjustedReq,
    folly::Function<void(const ReplyT<Request>&)> postShadowReplyFn) const {
  folly::fibers::addTask(
      [shadow = std::move(shadow),
       adjustedReq = std::move(adjustedReq),
       postShadowReplyFn = std::move(postShadowReplyFn)]() mutable {
        // we don't want to spool shadow requests
        fiber_local<RouterInfo>::clearAsynclogName();
        fiber_local<RouterInfo>::addRequestClass(RequestClass::kShadow);
        const auto shadowReply = shadow->route(*adjustedReq);
        if (postShadowReplyFn) {
          postShadowReplyFn(shadowReply);
        }
      });
}

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf> makeShadowRouteDefault(
    std::shared_ptr<typename RouterInfo::RouteHandleIf> normalRoute,
    ShadowData<RouterInfo> shadowData,
    DefaultShadowPolicy shadowPolicy) {
  return makeRouteHandleWithInfo<RouterInfo, ShadowRoute, DefaultShadowPolicy>(
      std::move(normalRoute), std::move(shadowData), std::move(shadowPolicy));
}

template <class RouterInfo>
std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>
makeShadowRoutes(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>> children,
    ProxyBase& proxy,
    ExtraRouteHandleProviderIf<RouterInfo>& extraProvider) {
  folly::StringPiece shadowPolicy = "default";
  if (auto jshadow_policy = json.get_ptr("shadow_policy")) {
    checkLogic(
        jshadow_policy->isString(),
        "ShadowRoute: shadow_policy is not a string");
    shadowPolicy = jshadow_policy->stringPiece();
  }

  auto jshadows = json.get_ptr("shadows");
  checkLogic(jshadows, "ShadowRoute: route doesn't contain shadows field");

  if (!jshadows->isArray()) {
    MC_LOG_FAILURE(
        proxy.router().opts(),
        failure::Category::kInvalidConfig,
        "ShadowRoute: shadows specified in route is not an array");
    return children;
  }

  size_t totalBucket = 0;
  if (auto* jNeedBucketization = json.get_ptr("bucketize")) {
    if (parseBool(*jNeedBucketization, "bucketize")) {
      auto settings = parseMcBucketRouteSettings(json);
      totalBucket = settings.totalBuckets;
    }
  }

  ShadowData<RouterInfo> data;
  data.reserve(jshadows->size());
  for (auto& shadow : *jshadows) {
    if (!shadow.isObject()) {
      MC_LOG_FAILURE(
          proxy.router().opts(),
          failure::Category::kInvalidConfig,
          "ShadowRoute: shadow is not an object");
      continue;
    }
    auto jtarget = shadow.get_ptr("target");
    if (!jtarget) {
      MC_LOG_FAILURE(
          proxy.router().opts(),
          failure::Category::kInvalidConfig,
          "ShadowRoute shadows: no target for shadow");
      continue;
    }
    try {
      auto s = ShadowSettings::create(shadow, proxy.router(), totalBucket);
      if (s) {
        data.emplace_back(factory.create(*jtarget), std::move(s));
      }
    } catch (const std::exception& e) {
      MC_LOG_FAILURE(
          proxy.router().opts(),
          failure::Category::kInvalidConfig,
          "Can not create shadow for ShadowRoute: {}",
          e.what());
    }
  }
  size_t i = 0;
  for (auto& child : children) {
    ShadowData<RouterInfo> childrenShadows;
    for (const auto& shadowData : data) {
      if ((shadowData.second->startIndex() <= i &&
           i < shadowData.second->endIndex()) ||
          !shadowData.second->keysToShadow().empty()) {
        childrenShadows.push_back(shadowData);
      }
    }
    if (!childrenShadows.empty()) {
      childrenShadows.shrink_to_fit();
      child = extraProvider.makeShadow(
          proxy, std::move(child), std::move(childrenShadows), shadowPolicy);
    }
    ++i;
  }
  return children;
}

template <class RouterInfo>
std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>
makeShadowRoutes(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    ProxyBase& proxy,
    ExtraRouteHandleProviderIf<RouterInfo>& extraProvider) {
  checkLogic(json.isObject(), "ShadowRoute should be an object");
  const auto jchildren = json.get_ptr("children");
  checkLogic(jchildren, "ShadowRoute: children not found");
  auto children = factory.createList(*jchildren);
  if (json.count("shadows")) {
    children = makeShadowRoutes(
        factory, json, std::move(children), proxy, extraProvider);
  }
  return children;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
