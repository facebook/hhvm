/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/json/dynamic.h>

#include "mcrouter/lib/Ch3HashFunc.h"
#include "mcrouter/lib/Crc32HashFunc.h"
#include "mcrouter/lib/HashSelector.h"
#include "mcrouter/lib/RendezvousHashFunc.h"
#include "mcrouter/lib/SelectionRouteFactory.h"
#include "mcrouter/lib/WeightedCh3HashFunc.h"
#include "mcrouter/lib/WeightedCh3RvHashFunc.h"
#include "mcrouter/lib/WeightedRendezvousHashFunc.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/routes/NullRoute.h"
#include "mcrouter/lib/routes/SelectionRoute.h"
#include "mcrouter/routes/LatestRoute.h"
#include "mcrouter/routes/LoadBalancerRoute.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"
#include "mcrouter/routes/RendezvousRouteHelpers.h"
#include "mcrouter/routes/ShardHashFunc.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class HashFunc>
HashSelector<HashFunc> createHashSelector(std::string salt, HashFunc func) {
  return HashSelector<HashFunc>(std::move(salt), std::move(func));
}

template <class HashFunc, class RouterInfo>
BucketHashSelector<HashFunc, RouterInfo> createBucketHashSelector(
    std::string salt,
    HashFunc func,
    bool clientFanout = false) {
  return BucketHashSelector<HashFunc, RouterInfo>(
      std::move(salt), std::move(func), clientFanout);
}

template <class RouterInfo, class HashFunc>
typename std::
    enable_if_t<!RouterInfo::bucketization, typename RouterInfo::RouteHandlePtr>
    createHashRoute(
        std::vector<typename RouterInfo::RouteHandlePtr> rh,
        std::string salt,
        HashFunc func,
        bool bucketized = false,
        bool clientFanout = false) {
  checkLogic(
      !bucketized,
      "Bucketization not implemented for router info: {}",
      RouterInfo::name);

  checkLogic(
      !clientFanout,
      "Fanout not implemented for router info: {}",
      RouterInfo::name);

  return createSelectionRoute<RouterInfo, HashSelector<HashFunc>>(
      std::move(rh),
      createHashSelector<HashFunc>(std::move(salt), std::move(func)));
}

template <class RouterInfo, class HashFunc>
typename std::
    enable_if_t<RouterInfo::bucketization, typename RouterInfo::RouteHandlePtr>
    createHashRoute(
        std::vector<typename RouterInfo::RouteHandlePtr> rh,
        std::string salt,
        HashFunc func,
        bool bucketized = false,
        bool clientFanout = false) {
  if (folly::IsOneOf<HashFunc, WeightedCh3HashFunc, WeightedCh3RvHashFunc>::
          value) {
    if (bucketized) {
      return createSelectionRoute<
          RouterInfo,
          BucketHashSelector<HashFunc, RouterInfo>>(
          std::move(rh),
          createBucketHashSelector<HashFunc, RouterInfo>(
              std::move(salt), std::move(func), clientFanout));
    }
  }
  checkLogic(
      !bucketized,
      "Bucketization not implemented for this ch type: {}",
      HashFunc::type());

  checkLogic(
      !clientFanout,
      "Fanout not implemented for this ch type: {}",
      HashFunc::type());

  return createSelectionRoute<RouterInfo, HashSelector<HashFunc>>(
      std::move(rh),
      createHashSelector<HashFunc>(std::move(salt), std::move(func)));
}

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf> createHashRoute(
    const folly::dynamic& json,
    std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>> rh,
    size_t threadId,
    ProxyBase& proxy) {
  std::string salt;
  folly::StringPiece funcType = Ch3HashFunc::type();
  auto bucketize = false;
  auto clientFanout = false;
  if (json.isObject()) {
    if (auto jsalt = json.get_ptr("salt")) {
      checkLogic(jsalt->isString(), "HashRoute: salt is not a string");
      salt = jsalt->getString();
    }
    if (auto jhashFunc = json.get_ptr("hash_func")) {
      checkLogic(jhashFunc->isString(), "HashRoute: hash_func is not a string");
      funcType = jhashFunc->stringPiece();
    }
    if (auto* jNeedBucketization = json.get_ptr("bucketize")) {
      bucketize = parseBool(*jNeedBucketization, "bucketize");
    }
    // client_fanout is only supported with bucketization
    if (bucketize) {
      if (auto* jClientFanout = json.get_ptr("client_fanout")) {
        clientFanout = parseBool(*jClientFanout, "client_fanout");
      }
    }
  }

  auto n = rh.size();
  if (n == 0) {
    auto errorOnEmpty = true;
    folly::StringPiece name{nullptr, nullptr};
    if (json.isObject()) {
      if (auto* jErrorOnEmpty = json.get_ptr("error_on_empty")) {
        errorOnEmpty = parseBool(*jErrorOnEmpty, "error_on_empty");
      }
      if (auto* jName = json.get_ptr("name")) {
        name = jName->stringPiece();
      }
    }
    return errorOnEmpty ? createErrorRoute<RouterInfo>(folly::sformat(
                              "HashRoute with empty children, name: {}", name))
                        : createNullRoute<typename RouterInfo::RouteHandleIf>();
  }
  if (n == 1) {
    return std::move(rh[0]);
  }

  if (funcType == Ch3HashFunc::type()) {
    return createHashRoute<RouterInfo, Ch3HashFunc>(
        std::move(rh), std::move(salt), Ch3HashFunc(n));
  } else if (funcType == Crc32HashFunc::type()) {
    return createHashRoute<RouterInfo, Crc32HashFunc>(
        std::move(rh), std::move(salt), Crc32HashFunc(n));
  } else if (funcType == WeightedCh3HashFunc::type()) {
    WeightedCh3HashFunc func{json, n};
    return createHashRoute<RouterInfo, WeightedCh3HashFunc>(
        std::move(rh),
        std::move(salt),
        std::move(func),
        bucketize,
        clientFanout);
  } else if (funcType == WeightedCh3RvHashFunc::type()) {
    WeightedCh3RvHashFunc func{json, n, proxy.router().rtVarsDataWeak().lock()};
    return createHashRoute<RouterInfo, WeightedCh3RvHashFunc>(
        std::move(rh),
        std::move(salt),
        std::move(func),
        bucketize,
        clientFanout);
  } else if (funcType == ConstShardHashFunc::type()) {
    return createHashRoute<RouterInfo, ConstShardHashFunc>(
        std::move(rh), std::move(salt), ConstShardHashFunc(n));
  } else if (
      funcType == RendezvousHashFunc::type() ||
      funcType == WeightedRendezvousHashFunc::type()) {
    auto endpoints = getTags(json, rh.size(), "HashRoute");
    if (funcType == RendezvousHashFunc::type()) {
      return createHashRoute<RouterInfo, RendezvousHashFunc>(
          std::move(rh),
          std::move(salt),
          RendezvousHashFunc(std::move(endpoints), json));
    } else {
      return createHashRoute<RouterInfo, WeightedRendezvousHashFunc>(
          std::move(rh),
          std::move(salt),
          WeightedRendezvousHashFunc(std::move(endpoints), json));
    }
  } else if (funcType == "Latest") {
    return createLatestRoute<RouterInfo>(json, std::move(rh), threadId);
  } else if (funcType == "LoadBalancer") {
    return createLoadBalancerRoute<RouterInfo>(json, std::move(rh));
  }
  throwLogic("Unknown hash function: {}", funcType);
}

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf> makeHashRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    ProxyBase& proxy) {
  std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>> children;
  if (json.isObject()) {
    if (auto jchildren = json.get_ptr("children")) {
      children = factory.createList(*jchildren);
    }
  } else {
    children = factory.createList(json);
  }
  return createHashRoute<RouterInfo>(
      json, std::move(children), factory.getThreadId(), proxy);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
