/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/fibers/AddTasks.h>
#include <folly/fibers/FiberManager.h>

#include "mcrouter/lib/McKey.h"
#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/fbi/cpp/FuncGenerator.h"
#include "mcrouter/lib/fbi/cpp/globals.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
class RouteHandleFactory;

namespace mcrouter {

/**
 * This route handle will allow a particular key to live on more than one
 * host in a destination pool. This is to primarily mitigate hot keys
 * overwhelming a single host.
 *
 * This is done by rehashing the key with a value and routing based on the
 * new key.
 *
 * @param   child     Child route handle to route request to
 * @param   replicas  Number of ways to split keys
 * @param   allSync   Sync sets and deletes amongst all children
 * @param   firstHit  returns the result of the first hit. NOTE: This should NOT
 *                    be used for hot key routing
 */
template <class RouterInfo>
class KeySplitRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  KeySplitRoute(
      RouteHandlePtr child,
      size_t replicas,
      bool allSync,
      bool firstHit = false)
      : child_(std::move(child)),
        replicas_(replicas),
        allSync_(allSync),
        firstHit_(firstHit) {
    assert(child_ != nullptr);
    assert(replicas_ >= kMinReplicaCount);
    assert(replicas_ <= kMaxReplicaCount);
  }

  std::string routeName() const {
    uint64_t replicaId = getReplicaId();
    return folly::sformat(
        "keysplit|replicas={}|all-sync={}|first-hit={}|replicaId={}",
        replicas_,
        allSync_,
        firstHit_,
        replicaId);
  }

  static constexpr size_t kMinReplicaCount = 2;
  static constexpr size_t kMaxReplicaCount = 1000;

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    uint64_t replicaId = getReplicaId();
    if (shouldAugmentRequest(replicaId)) {
      return t(*child_, copyAndAugment(req, replicaId));
    }
    return t(*child_, req);
  }

  // Route only to 1 replica if first_hit is not set, otherwise return first
  // result that is not an error or miss or the last reply.
  template <class Request>
  typename std::enable_if<
      folly::
          IsOneOf<Request, McGetRequest, McLeaseGetRequest, McLeaseSetRequest>::
              value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    if (!canAugmentRequest(req)) {
      return child_->route(req);
    }

    if (firstHit_) {
      return routeAllFastest(req);
    }
    // always retrieve from 1 replica
    uint64_t replicaId = getReplicaId();
    return routeOne(req, replicaId);
  }

  // Route only if all sync is enabled. Otherwise, route normally
  template <class Request>
  typename std::enable_if<
      folly::IsOneOf<Request, McSetRequest>::value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    if (!canAugmentRequest(req)) {
      return child_->route(req);
    }

    uint64_t replicaId = getReplicaId();

    // Only set to all key replicas if we have it enabled
    if (allSync_) {
      return routeAll(req, replicaId);
    }
    return routeOne(req, replicaId);
  }

  // Unconditionally route to all replicas.
  template <class Request>
  typename std::enable_if<
      folly::IsOneOf<Request, McDeleteRequest>::value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    if (!canAugmentRequest(req)) {
      return child_->route(req);
    }

    // always delete all key replicas
    uint64_t replicaId = getReplicaId();
    return routeAll(req, replicaId);
  }

  // fallback to just normal routing through one of the replicas
  template <class Request>
  typename std::enable_if<
      !folly::IsOneOf<
          Request,
          McGetRequest,
          McLeaseGetRequest,
          McLeaseSetRequest,
          McSetRequest,
          McDeleteRequest>::value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    uint64_t replicaId = getReplicaId();
    return routeOne(req, replicaId);
  }

 private:
  static constexpr folly::StringPiece kMemcacheReplicaSeparator = "::";
  static constexpr size_t kMaxMcKeyLength = 255;
  static constexpr size_t kExtraKeySpaceNeeded =
      kMemcacheReplicaSeparator.size() +
      detail::numDigitsBase10(kMaxReplicaCount - 1);

  // route configuration
  const std::shared_ptr<RouteHandleIf> child_;
  const size_t replicas_{2};
  const bool allSync_{false};
  const bool firstHit_{false};

  template <class Request>
  bool canAugmentRequest(const Request& req) const {
    // don't augment if length of key is too long
    return req.key_ref()->fullKey().size() + kExtraKeySpaceNeeded <=
        kMaxMcKeyLength;
  }

  bool shouldAugmentRequest(size_t replicaId) const {
    // first replica will route normally without key change.
    return replicaId > 0;
  }

  template <class Request>
  Request copyAndAugment(Request& originalReq, uint64_t replicaId) const {
    auto req = originalReq;
    if (req.key_ref()->hasHashStop()) {
      req.key_ref() = folly::to<std::string>(
          req.key_ref()->routingKey(),
          kMemcacheReplicaSeparator,
          replicaId,
          req.key_ref()->afterRoutingKey());
    } else {
      req.key_ref() = folly::to<std::string>(
          req.key_ref()->fullKey(), kMemcacheReplicaSeparator, replicaId);
    }
    return req;
  }

  template <class Request>
  ReplyT<Request> routeOne(const Request& req, uint64_t replicaId) const {
    if (shouldAugmentRequest(replicaId)) {
      return child_->route(copyAndAugment(req, replicaId));
    } else {
      return child_->route(req);
    }
  }

  template <class Request>
  ReplyT<Request> routeAll(const Request& req, uint64_t replicaId) const {
    // send off to all replicas async except the one we are assigned to
    for (size_t id = 0; id < replicas_; ++id) {
      if (id == replicaId) {
        continue;
      }

      // we need to make a copy to send to the fiber
      auto reqCopy = shouldAugmentRequest(id) ? copyAndAugment(req, id) : req;
      folly::fibers::addTask(
          [child = child_, reqReplica = std::move(reqCopy)]() {
            return child->route(reqReplica);
          });
    }

    return routeOne(req, replicaId);
  }

  template <class Request>
  typename std::enable_if<
      folly::
          IsOneOf<Request, McGetRequest, McLeaseGetRequest, McLeaseSetRequest>::
              value,
      ReplyT<Request>>::type
  routeAllFastest(const Request& req) const {
    using Reply = ReplyT<Request>;

    std::vector<std::function<Reply()>> funcs;
    funcs.reserve(replicas_);
    for (size_t id = 0; id < replicas_; ++id) {
      auto reqCopy = shouldAugmentRequest(id)
          ? std::make_shared<Request>(copyAndAugment(req, id))
          : std::make_shared<Request>(req);
      funcs.push_back(
          [reqCopy, child = child_]() { return child->route(*reqCopy); });
    }
    auto taskIt = folly::fibers::addTasks(funcs.begin(), funcs.end());
    while (true) {
      auto reply = taskIt.awaitNext();
      if (folly::IsOneOf<Request, McGetRequest, McLeaseGetRequest>::value) {
        if (isHitResult(*reply.result_ref()) || !taskIt.hasNext()) {
          return reply;
        }
      } else {
        if (isStoredResult(*reply.result_ref()) || !taskIt.hasNext()) {
          return reply;
        }
      }
    }
  }

  // if its not a get  or leaseset request then just do a normal route all
  // Note: This will not be called.
  template <class Request>
  typename std::enable_if<
      !folly::
          IsOneOf<Request, McGetRequest, McLeaseGetRequest, McLeaseSetRequest>::
              value,
      ReplyT<Request>>::type
  routeAllFastest(const Request& req) const {
    uint64_t replicaId = getReplicaId();
    return routeAll(req, replicaId);
  }

  uint64_t getReplicaId() const {
    return globals::hostid() % replicas_;
  }
};

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeKeySplitRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "mcrouter/routes/KeySplitRoute-inl.h"
