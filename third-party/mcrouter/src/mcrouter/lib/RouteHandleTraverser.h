/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Likely.h>
#include <functional>
#include <memory>
#include <vector>

#include "mcrouter/HostWithShard-fwd.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/PoolContext.h"
#include "mcrouter/lib/network/AccessPoint.h"

namespace facebook {
namespace memcache {

/**
 * DFS over RouteHandle tree. Calls StartFunc before entering a node and
 * EndFunc after traversing over children of a node.
 */
template <class RouteHandleIf>
class RouteHandleTraverser {
 public:
  /**
   * Callback that is be called before traversing each route handle
   *
   * @param r   The route handle we are about to traverse.
   */
  using StartFunc = std::function<void(const RouteHandleIf& r)>;

  /**
   * Called right after traversing a route handle.
   */
  using EndFunc = std::function<void()>;

  /**
   * Called when traversing a leaf node that knows how to talk to a server
   * (i.e. DestinationRoute).
   *
   * @param accessPoint   The destination server that the current RouteHandle
   *                      talks to.
   * @param pollContext   Information about the pool that the destination
   *                      server belogs to.
   *
   * @return              True iff we should stop the traversal.
   *                      Note: The traversal will only be stopped iff the
   *                      destination server is not TKO'd.
   */
  using AccessPointFunc =
      std::function<bool(const AccessPoint&, const PoolContext&)>;

  using RequestClass = mcrouter::RequestClass;
  using SRHostWithShardFunc =
      std::function<bool(const HostWithShard&, const RequestClass&)>;
  using SRHostFunc =
      std::function<bool(const AccessPoint&, const RequestClass&)>;

  /**
   * Creates a route handle traverser.
   */
  explicit RouteHandleTraverser(
      StartFunc start = nullptr,
      EndFunc end = nullptr,
      AccessPointFunc accessPointFn = nullptr,
      SRHostWithShardFunc srHostwithShardFn = nullptr,
      SRHostFunc srHostFn = nullptr)
      : start_(std::move(start)),
        end_(std::move(end)),
        accessPointFn_(std::move(accessPointFn)),
        srHostwithShardFn_(std::move(srHostwithShardFn)),
        srHostFn_(std::move(srHostFn)) {}

  template <class Request>
  bool operator()(const RouteHandleIf& r, const Request& req) const {
    if (FOLLY_UNLIKELY(start_ != nullptr)) {
      start_(r);
    }
    auto stopTraversal = r.traverse(req, *this);
    if (FOLLY_UNLIKELY(end_ != nullptr)) {
      end_();
    }
    return stopTraversal;
  }

  template <class Request>
  bool operator()(
      const AccessPoint& accessPoint,
      const PoolContext& poolContext,
      const Request&) const {
    bool stopTraversal = false;
    if (accessPointFn_) {
      stopTraversal = accessPointFn_(accessPoint, poolContext);
    }
    return stopTraversal;
  }

  template <class Request>
  bool operator()(
      const HostWithShard& srHostWithShard,
      const RequestClass& requestClass,
      const Request&) const {
    bool stopTraversal = false;
    if (srHostwithShardFn_) {
      stopTraversal = srHostwithShardFn_(srHostWithShard, requestClass);
    }
    return stopTraversal;
  }

  template <class Request>
  bool operator()(
      const AccessPoint& srHost,
      const RequestClass& requestClass,
      const Request&) const {
    bool stopTraversal = false;
    if (srHostFn_) {
      stopTraversal = srHostFn_(srHost, requestClass);
    }
    return stopTraversal;
  }

  /*
   * If a route handle traverse returns true, the RouteHandleTraverser
   * will exit early from the traversal.
   */
  template <class Request>
  bool operator()(
      const std::vector<std::shared_ptr<RouteHandleIf>>& v,
      const Request& req) const {
    for (const auto& child : v) {
      if (operator()(*child, req)) {
        return true;
      }
    }
    return false;
  }

  // options that may be used to control traversal action
  class Options {
   private:
    /**
     * Split size to use for shard split routing.
     * It is not specified by default, in which case split size in routing
     * config will be used.
     */
    size_t splitSize_{0};

   public:
    /**
     * Set splitSize
     */
    void setSplitSize(size_t value) {
      splitSize_ = value;
    }

    /**
     * Get splitSize option
     */
    size_t getSplitSize() const {
      return splitSize_;
    }
  };

 private:
  StartFunc start_;
  EndFunc end_;
  AccessPointFunc accessPointFn_;
  SRHostWithShardFunc srHostwithShardFn_;
  SRHostFunc srHostFn_;
  Options options_;

 public:
  const Options& options() const {
    return options_;
  }

  Options& options() {
    return options_;
  }
};
} // namespace memcache
} // namespace facebook
