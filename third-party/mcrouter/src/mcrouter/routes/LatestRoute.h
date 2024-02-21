/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include <folly/json/dynamic.h>

#include "mcrouter/lib/FailoverErrorsSettings.h"
#include "mcrouter/lib/WeightedCh3HashFunc.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/globals.h"
#include "mcrouter/routes/FailoverRoute.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

template <class RouteHandleIf>
std::vector<std::shared_ptr<RouteHandleIf>> getTargets(
    std::vector<std::shared_ptr<RouteHandleIf>> targets,
    size_t failoverCount,
    size_t threadId,
    std::vector<double> weights,
    folly::StringPiece salt) {
  if (targets.size() <= 1) {
    return std::move(targets);
  }

  std::vector<std::shared_ptr<RouteHandleIf>> failovers;
  failoverCount = std::min(failoverCount, targets.size());
  size_t hashKey = folly::hash::hash_combine(0, globals::hostid());
  if (threadId != 0) {
    hashKey = folly::hash::hash_combine(hashKey, threadId);
  }
  if (!salt.empty()) {
    hashKey = folly::Hash()(hashKey, salt);
  }
  failovers.reserve(failoverCount);
  for (size_t i = 0; i < failoverCount; ++i) {
    auto id =
        WeightedCh3HashFunc::hash(folly::to<std::string>(hashKey), weights);
    failovers.push_back(std::move(targets[id]));
    std::swap(targets[id], targets.back());
    targets.pop_back();
    std::swap(weights[id], weights.back());
    weights.pop_back();
    hashKey = folly::hash::hash_combine(hashKey, i);
  }
  return failovers;
}

} // namespace detail

struct LatestRouteOptions {
  FailoverErrorsSettings errorsSettings;
  size_t failoverCount = 5;
  size_t failoverThreadId = 0;
  folly::StringPiece salt;
  const folly::dynamic* jFailoverPolicy{nullptr};
};

inline LatestRouteOptions parseLatestRouteJson(
    const folly::dynamic& json,
    size_t threadId) {
  LatestRouteOptions options;

  if (json.isObject()) {
    options.errorsSettings = parseFailoverErrorsSettings(json);

    if (auto jfailoverCount = json.get_ptr("failover_count")) {
      checkLogic(
          jfailoverCount->isInt(),
          "LatestRoute: failover_count is not an integer");
      options.failoverCount = jfailoverCount->getInt();
    }

    if (auto jsalt = json.get_ptr("salt")) {
      checkLogic(jsalt->isString(), "LatestRoute: salt is not a string");
      options.salt = jsalt->stringPiece();
    }

    if (auto jthreadLocalFailover = json.get_ptr("thread_local_failover")) {
      checkLogic(
          jthreadLocalFailover->isBool(),
          "LatestRoute: thread_local_failover is not a boolean");
      if (jthreadLocalFailover->getBool()) {
        options.failoverThreadId = threadId;
      }
    }

    options.jFailoverPolicy = parseFailoverPolicy(json);
  }

  return options;
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr createLatestRoute(
    const folly::dynamic& json,
    std::vector<typename RouterInfo::RouteHandlePtr> targets,
    LatestRouteOptions options,
    std::vector<double> weights) {
  return makeFailoverRouteWithFailoverErrorSettings<
      RouterInfo,
      FailoverRoute,
      FailoverErrorsSettings>(
      json,
      detail::getTargets(
          std::move(targets),
          options.failoverCount,
          options.failoverThreadId,
          std::move(weights),
          options.salt),
      std::move(options.errorsSettings),
      options.jFailoverPolicy);
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr createLatestRoute(
    const folly::dynamic& json,
    std::vector<typename RouterInfo::RouteHandlePtr> targets,
    size_t threadId) {
  LatestRouteOptions options = parseLatestRouteJson(json, threadId);

  std::vector<double> weights;
  if (!json.isObject() || !json.count("weights")) {
    weights.resize(targets.size(), 1.0);
  } else {
    weights = WeightedCh3HashFunc::parseWeights(json, targets.size());
  }

  return createLatestRoute<RouterInfo>(
      json, std::move(targets), std::move(options), std::move(weights));
}

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf> makeLatestRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>> children;
  if (json.isObject()) {
    if (auto jchildren = json.get_ptr("children")) {
      children = factory.createList(*jchildren);
    }
  } else {
    children = factory.createList(json);
  }
  return createLatestRoute<RouterInfo>(
      json, std::move(children), factory.getThreadId());
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr createLatestRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    std::vector<typename RouterInfo::RouteHandlePtr> children) {
  return createLatestRoute<RouterInfo>(
      json, std::move(children), factory.getThreadId());
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
