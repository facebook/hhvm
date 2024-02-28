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

/**
 * LoadBalancerRoute looks at the server loads of all the servers and chooses
 * the best one to send request to. Complement of the server loads of the each
 * of the servers is used as 'weight' to the WeightedCh3Hash function to
 * determine the next destination server.
 *
 * @tparam RouteHandleInfo   The Router
 */
template <class RouterInfo>
class LoadBalancerRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  enum class AlgorithmType : std::uint8_t {
    WEIGHTED_HASHING = 1,
    TWO_RANDOM_CHOICES = 2,
  };

  static constexpr folly::StringPiece kWeightedHashing = "weighted-hashing";
  static constexpr folly::StringPiece kTwoRandomChoices = "two-random-choices";

  std::string routeName() const {
    return folly::to<std::string>(
        "loadbalancer|",
        algorithm_ == AlgorithmType::WEIGHTED_HASHING ? kWeightedHashing
                                                      : kTwoRandomChoices);
  }

  /**
   * Constructs LoadBalancerRoute.
   * Initializes the weighted load to 1.0. This means serverLoad is 0.
   *
   * @param children                List of children route handles.
   * @param salt                    salt
   * @param loadTtl                 TTL for load in micro seconds
   * @param failoverCount           Number of times to route the request.
   *                                1 means no failover (just route once).
   *                                The value will be capped to
   *                                std::min(failoverCount, children.size())
   * @param algorithm               Load balancing algorithm to use.
   * @param seed                    seed for random number generator used in
   *                                the two random choices algorithm.
   */
  LoadBalancerRoute(
      std::vector<std::shared_ptr<RouteHandleIf>> children,
      std::string salt,
      std::chrono::microseconds loadTtl,
      size_t failoverCount,
      AlgorithmType algorithm = AlgorithmType::WEIGHTED_HASHING,
      uint32_t seed = nowUs(),
      bool enableThriftServerLoad = false)
      : children_(std::move(children)),
        salt_(std::move(salt)),
        loadTtl_(loadTtl),
        failoverCount_(std::min(failoverCount, children_.size())),
        loadComplements_(children_.size(), 1.0),
        medianLoadScratch_(children_.size()),
        expTimes_(children_.size(), std::chrono::microseconds(0)),
        gen_(seed),
        algorithm_(algorithm),
        enableThriftServerLoad_(enableThriftServerLoad) {
    assert(children_.size() >= 2);
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    // Walk all children
    return t(children_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) {
    if (algorithm_ == AlgorithmType::TWO_RANDOM_CHOICES) {
      return routeTwoRandomChoices(req);
    }
    // first try
    size_t idx = selectWeightedHashing(req, loadComplements_, salt_);
    auto reply = doRoute(req, idx);

    // retry in case of error
    if (failoverCount_ > 1 && shouldFailover(reply)) {
      std::vector<double> weights = loadComplements_;
      std::vector<size_t> indexMap(children_.size(), 0);
      std::iota(indexMap.begin(), indexMap.end(), 0);

      int64_t retries = failoverCount_;
      while (--retries > 0 && shouldFailover(reply)) {
        std::swap(weights[idx], weights.back());
        std::swap(indexMap[idx], indexMap.back());
        weights.pop_back();
        indexMap.pop_back();

        idx = selectWeightedHashing(
            req, weights, folly::to<std::string>(salt_, retries));
        reply = doRoute(req, indexMap[idx], /* isFailover */ true);
      }
    }

    // Reset expried loads.
    const int64_t now = nowUs();
    auto loadMedian = getLoadComplementsMedian();
    size_t i = 0;
    for ([[maybe_unused]] const auto& v : children_) {
      if (expTimes_[i].count() != 0 && expTimes_[i].count() <= now) {
        expTimes_[i] = std::chrono::microseconds(0);
        loadComplements_[i] = loadMedian;
        if (auto& ctx = mcrouter::fiber_local<RouterInfo>::getSharedCtx()) {
          ctx->proxy().stats().increment(load_balancer_load_reset_count_stat);
        }
      }
      ++i;
    }

    return reply;
  }

 private:
  const std::vector<std::shared_ptr<RouteHandleIf>> children_;
  const std::string salt_;
  // Default TTL of a serer load. Indicates the time after which a server load
  // value is reset to the getLoadComplementsMedian() value.
  const std::chrono::microseconds loadTtl_{100000};
  // Number of times to retry on error.
  const size_t failoverCount_{1};

  // 1-complement of the server load (i.e. 1 - serverLoad) of each of the
  // children.
  std::vector<double> loadComplements_;
  // Scratch space for computing the median of loadComplements_.
  std::vector<double> medianLoadScratch_;
  // Point in time when the loadComplement becomes too old to be trusted.
  std::vector<std::chrono::microseconds> expTimes_;
  // Random Number generator used for TwoRandomChoices algorithm
  std::ranlux24_base gen_;
  // Load balancing algorithm
  AlgorithmType algorithm_;
  // When enabled, thrift request would ask server for load stat
  bool enableThriftServerLoad_;

  // route the request and update server load.
  template <class Request>
  ReplyT<Request>
  doRoute(const Request& req, const size_t idx, bool isFailover = false) {
    assert(idx < children_.size());
    return mcrouter::fiber_local<RouterInfo>::runWithLocals(
        [this, &req, idx, isFailover]() {
          if (isFailover) {
            fiber_local<RouterInfo>::addRequestClass(RequestClass::kFailover);
          }
          if (enableThriftServerLoad_) {
            fiber_local<RouterInfo>::setThriftServerLoadEnabled(true);
          }
          auto reply = children_[idx]->route(req);
          auto load = mcrouter::fiber_local<RouterInfo>::getServerLoad();
          if (!load.isZero()) {
            loadComplements_[idx] = load.complement().percentLoad() / 100;

            // For consistency with the two random choice algorithm, we set
            // expiration timers in all cases. We should be able to avoid
            // extreme server load flucatuations on expiration timers now that
            // we are using a median.
            expTimes_[idx] =
                std::chrono::microseconds(nowUs() + loadTtl_.count());
          }
          carbon::setIsFailoverIfPresent(reply, isFailover);
          return reply;
        });
  }

  template <class Reply>
  bool shouldFailover(const Reply& reply) {
    return isErrorResult(*reply.result_ref());
  }

  template <class Request>
  ReplyT<Request> routeTwoRandomChoices(const Request& req) {
    std::pair<size_t, size_t> idxs = selectTwoRandomChoices();
    if (enableThriftServerLoad_) {
      mcrouter::fiber_local<RouterInfo>::setThriftServerLoadEnabled(true);
    }
    auto rep = children_[idxs.first]->route(req);
    auto load = mcrouter::fiber_local<RouterInfo>::getServerLoad();
    loadComplements_[idxs.first] = load.complement().percentLoad() / 100;

    auto now = nowUs();

    // In the two random choice algorithm the highest load server will never
    // have a request routed to it regardless of which two random pairs are
    // chosen unless the primary choice fails. Because of this, we introduce an
    // expiration timer which resets the load to a reasonable default (median
    // load) to bring this previously high load server which is no longer
    // receiving requests back into the viable candidate pool after having some
    // time to cool off. Previously we were only setting the expiration timer
    // only if load > default. However, this made us vulnerable to edge cases
    // where we could have a server A such that load(A) < Median-Load so it's
    // expiration timer would not get set. However, the median could change as
    // load on other servers changes such that load(A) > Media-Load in the
    // future. Server A could become the highest loaded server and be
    // permanently excluded from the viable candidate set. To avoid situations
    // like this we set the expiration timer in all cases. Notice that servers
    // with low load (below the median) are unlikely to have their load
    // inadvertently bumped up because the algorithm will favor routes to low
    // load servers and continuously update their load based on replies. This
    // means we mostly only need to worry how the expiration timer affects
    // servers with a high load which we are not routing many/any requests to.
    expTimes_[idxs.first] = std::chrono::microseconds(now + loadTtl_.count());

    // If the second entry, which had higher load that the first entry, has
    // already expired, reset the expiry time to zero and set the load to
    // getLoadComplementsMedian().
    if (expTimes_[idxs.second].count() != 0 &&
        expTimes_[idxs.second].count() <= now) {
      expTimes_[idxs.second] = std::chrono::microseconds(0);
      loadComplements_[idxs.second] = getLoadComplementsMedian();
      if (auto& ctx = mcrouter::fiber_local<RouterInfo>::getSharedCtx()) {
        ctx->proxy().stats().increment(load_balancer_load_reset_count_stat);
      }
    }

    return rep;
  }

  template <class Request>
  size_t selectWeightedHashingInternal(
      const Request& req,
      const std::vector<double>& weights,
      folly::StringPiece salt) const {
    size_t n = 0;
    if (salt.empty()) {
      n = WeightedCh3HashFunc::hash(req.key_ref()->routingKey(), weights);
    } else {
      n = hashWithSalt(
          req.key_ref()->routingKey(),
          salt,
          [&weights](const folly::StringPiece sp) {
            return WeightedCh3HashFunc::hash(sp, weights);
          });
    }
    if (FOLLY_UNLIKELY(n >= children_.size())) {
      throw std::runtime_error("index out of range");
    }
    return n;
  }

  /**
   * Implements Two Random Choices algorithm
   *
   * @return       pair of randomly selected idxs. First idx is of the child
   *               with lowest load. The second idx is of the child with the
   *               highest load.
   *
   */
  std::pair<size_t, size_t> selectTwoRandomChoices() {
    uint32_t x = 0;
    uint32_t y = 1;
    if (children_.size() > 2) {
      x = gen_() % children_.size();
      y = gen_() % (children_.size() - 1);
      if (x == y) {
        y = children_.size() - 1;
      }
    }

    if (loadComplements_[x] > loadComplements_[y]) {
      return std::make_pair<size_t, size_t>(x, y);
    }
    return std::make_pair<size_t, size_t>(y, x);
  }

  template <class Request>
  size_t selectWeightedHashing(
      const Request& req,
      const std::vector<double>& weights,
      folly::StringPiece salt) const {
    // Hash functions can be stack-intensive so jump back to the main context
    return folly::fibers::runInMainContext([this, &req, &weights, &salt]() {
      return selectWeightedHashingInternal(req, weights, salt);
    });
  }

  double getLoadComplementsMedian() {
    // There are usually < 16 servers so the entire load complements vector
    // fits inside at MOST three cache lines. This is likely better than using
    // ordered sets to track a median.
    std::copy(
        loadComplements_.begin(),
        loadComplements_.end(),
        medianLoadScratch_.begin());
    auto begin = medianLoadScratch_.begin();
    auto median = begin + medianLoadScratch_.size() / 2;
    std::nth_element(begin, median, medianLoadScratch_.end());
    return *median;
  }
};

template <class RouterInfo>
constexpr folly::StringPiece LoadBalancerRoute<RouterInfo>::kWeightedHashing;
template <class RouterInfo>
constexpr folly::StringPiece LoadBalancerRoute<RouterInfo>::kTwoRandomChoices;

template <class RouterInfo>
struct LoadBalancerRouteOptions {
  folly::StringPiece salt;
  std::chrono::microseconds loadTtl{100 * 1000}; // 100 milliseconds
  size_t failoverCount{1};
  typename LoadBalancerRoute<RouterInfo>::AlgorithmType algorithm{
      LoadBalancerRoute<RouterInfo>::AlgorithmType::WEIGHTED_HASHING};
  bool enableThriftServerLoad{false};
};

template <class RouterInfo>
LoadBalancerRouteOptions<RouterInfo> parseLoadBalancerRouteJson(
    const folly::dynamic& json) {
  LoadBalancerRouteOptions<RouterInfo> options;

  if (auto jSalt = json.get_ptr("salt")) {
    checkLogic(jSalt->isString(), "LoadBalancerRoute: salt is not a string");
    options.salt = jSalt->stringPiece();
  }

  if (auto jLoadTtl = json.get_ptr("load_ttl_ms")) {
    checkLogic(
        jLoadTtl->isInt(), "LoadBalancerRoute: load_ttl_ms is not an integer");
    options.loadTtl = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::milliseconds(jLoadTtl->getInt()));
  }

  if (auto jFailoverCount = json.get_ptr("failover_count")) {
    checkLogic(
        jFailoverCount->isInt(),
        "LoadBalancerRoute: failover_count is not an integer");
    options.failoverCount = jFailoverCount->getInt();
  }

  if (auto jAlgorithm = json.get_ptr("algorithm")) {
    checkLogic(
        jAlgorithm->isString(), "LoadBalancerRoute: algorithm is not a string");
    std::string algorithmStr = jAlgorithm->getString();
    if (algorithmStr == LoadBalancerRoute<RouterInfo>::kWeightedHashing) {
      options.algorithm =
          LoadBalancerRoute<RouterInfo>::AlgorithmType::WEIGHTED_HASHING;
    } else if (
        algorithmStr == LoadBalancerRoute<RouterInfo>::kTwoRandomChoices) {
      options.algorithm =
          LoadBalancerRoute<RouterInfo>::AlgorithmType::TWO_RANDOM_CHOICES;
    } else {
      throwLogic("Unknown algorithm: {}", algorithmStr);
    }
  }

  if (auto jEnableThriftServerLoad =
          json.get_ptr("enable_thrift_server_load")) {
    checkLogic(
        jEnableThriftServerLoad->isBool(),
        "LoadBalancerRoute: enable_thrift_server_load is not a bool");
    options.enableThriftServerLoad = jEnableThriftServerLoad->getBool();
  }

  return options;
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr createLoadBalancerRoute(
    std::vector<typename RouterInfo::RouteHandlePtr> children,
    const LoadBalancerRouteOptions<RouterInfo>& options) {
  if (children.size() == 0) {
    return createNullRoute<typename RouterInfo::RouteHandleIf>();
  }
  if (children.size() == 1) {
    return std::move(children[0]);
  }

  return makeRouteHandleWithInfo<RouterInfo, LoadBalancerRoute>(
      std::move(children),
      options.salt.str(),
      options.loadTtl,
      options.failoverCount,
      options.algorithm,
      nowUs(),
      options.enableThriftServerLoad);
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr createLoadBalancerRoute(
    const folly::dynamic& json,
    std::vector<typename RouterInfo::RouteHandlePtr> children) {
  assert(json.isObject());

  if (children.size() == 0) {
    return createNullRoute<typename RouterInfo::RouteHandleIf>();
  }
  if (children.size() == 1) {
    return std::move(children[0]);
  }

  return createLoadBalancerRoute<RouterInfo>(
      std::move(children), parseLoadBalancerRouteJson<RouterInfo>(json));
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr createLoadBalancerRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& /* factory */,
    const folly::dynamic& json,
    std::vector<typename RouterInfo::RouteHandlePtr> rh) {
  return createLoadBalancerRoute<RouterInfo>(json, std::move(rh));
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeLoadBalancerRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "LoadBalancerRoute is not an object");

  auto jChildren = json.get_ptr("children");
  checkLogic(
      jChildren != nullptr,
      "LoadBalancerRoute: 'children' property is missing");

  auto children = factory.createList(*jChildren);
  return createLoadBalancerRoute<RouterInfo>(json, std::move(children));
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
