/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cassert>
#include <random>
#include <utility>

#include <folly/Conv.h>
#include <folly/Optional.h>
#include <folly/fibers/FiberManager.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/routes/DefaultShadowSelectorPolicy.h"

namespace facebook {
namespace memcache {

namespace detail {
/**
 *  * @return monotonic time suitable for measuring intervals in microseconds.
 *   */
inline int64_t nowUs() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}
} // namespace detail

/**
 * Route handle that can select a child from a list of children.
 *
 * @tparam RouterInfo   The router.
 * @tparam Selector     The selector, that has to implement
 *                      "type()" and "select()"
 * @tparam ShadowSelectorPolicy The shadow policy that has to
 *                      implement "makePostShadowReplyFn()"
 *
 */
template <class RouterInfo, typename Selector>
class SelectionRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  std::string routeName() const {
    return folly::to<std::string>("selection|", selector_.type());
  }

  /**
   * Constructs SelectionRoute.
   *
   * @param children                List of children route handles.
   * @param selector                Selector responsible for choosing to which
   *                                of the children the request should be sent
   *                                to.
   * @param outOfRangeDestination   The destination to which the request will be
   *                                routed if selector.select() returns a value
   *                                that is >= than children.size().
   */
  SelectionRoute(
      std::vector<RouteHandlePtr> children,
      Selector selector,
      RouteHandlePtr outOfRangeDestination)
      : children_(std::move(children)),
        selector_(std::move(selector)),
        outOfRangeDestination_(std::move(outOfRangeDestination)) {
    assert(outOfRangeDestination_);
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(select(req), req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) {
    return select(req).route(req);
  }

 protected:
  const std::vector<RouteHandlePtr> children_;
  const Selector selector_;
  const RouteHandlePtr outOfRangeDestination_;

  template <class Request>
  RouteHandleIf& select(const Request& req) const {
    size_t idx = selector_.select(req, children_.size());
    if (idx >= children_.size()) {
      return *outOfRangeDestination_;
    }
    mcrouter::fiber_local<RouterInfo>::setSelectedIndex(idx);
    return *children_[idx];
  }
};

template <
    class RouterInfo,
    typename Selector,
    typename ShadowSelectorPolicy = DefaultShadowSelectorPolicy>
class SelectionRouteWithShadow : public SelectionRoute<RouterInfo, Selector> {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  std::string routeName() const {
    return folly::to<std::string>(
        "selection|", this->selector_.type(), "|shadow_enabled");
  }

  /**
   * Constructs SelectionRoute with shadows.
   *
   * @param children                See SelectionRoute
   * @param selector                See SelectionRoute
   * @param outOfRangeDestination   See SelectionRoute
   * @param shadowChildren          Optional list of shadow children route
   *                                handles.
   * @param shadowSelector          Optional selector responsible for choosing
   *                                to which of the children the request should
   *                                be sent to.
   * @param shadowProbabilities     Vector of uint16_t representing percentages
   *                                from 0 - 100 indicating percetage of the
   *                                time to shadow to this destination when
   *                                selected.
   */
  SelectionRouteWithShadow(
      std::vector<RouteHandlePtr> children,
      Selector selector,
      RouteHandlePtr outOfRangeDestination,
      std::vector<RouteHandlePtr> shadowChildren = {},
      folly::Optional<Selector> shadowSelector = folly::none,
      std::vector<uint16_t> shadowProbabilities = {},
      folly::Optional<ShadowSelectorPolicy> shadowSelectorPolicy = folly::none,
      uint32_t seed = detail::nowUs())
      : SelectionRoute<RouterInfo, Selector>(
            std::move(children),
            std::move(selector),
            std::move(outOfRangeDestination)),
        shadowChildren_(std::move(shadowChildren)),
        shadowSelector_(std::move(shadowSelector)),
        shadowProbabilities_(shadowProbabilities),
        shadowSelectorPolicy_(shadowSelectorPolicy),
        gen_(seed) {
    assert(!shadowChildren_.empty());
    assert(shadowSelector_.hasValue() && shadowProbabilities_.size() > 0);
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    if (t(this->select(req), req)) {
      return true;
    }
    return mcrouter::fiber_local<RouterInfo>::runWithLocals(
        [this, &req, &t]() mutable {
          mcrouter::fiber_local<RouterInfo>::addRequestClass(
              mcrouter::RequestClass::kShadow);
          return (t(*shadowSelect<Request>(shadowSelectIdx(req)), req));
        });
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) {
    auto idx = shadowSelectIdx(req);
    if (shouldShadow(idx)) {
      auto adjustedNormalReq = std::make_shared<Request>(req);
      assert(adjustedNormalReq);
      dispatchShadowRequest(
          idx,
          adjustedNormalReq,
          shadowSelectorPolicy_.hasValue()
              ? shadowSelectorPolicy_
                    ->template makePostShadowReplyFn<ReplyT<Request>>()
              : nullptr);
      return this->select(*adjustedNormalReq).route(*adjustedNormalReq);
    }
    return this->select(req).route(req);
  }

 private:
  const std::vector<RouteHandlePtr> shadowChildren_;
  const folly::Optional<Selector> shadowSelector_;
  // Probability vector to shadow request to rh
  const std::vector<uint16_t> shadowProbabilities_;
  // Optional callback to be invoked on shadow request
  // response.
  folly::Optional<ShadowSelectorPolicy> shadowSelectorPolicy_;
  // Random Number generator used for Shadow functionality
  std::ranlux24_base gen_;

  template <class Request>
  size_t shadowSelectIdx(const Request& req) const {
    return shadowSelector_->select(req, shadowChildren_.size());
  }

  template <class Request>
  RouteHandlePtr shadowSelect(size_t idx) const {
    if (idx >= shadowChildren_.size()) {
      return this->outOfRangeDestination_;
    }
    return shadowChildren_[idx];
  }

  // Dispatch shadow request on another fiber without witing on response.
  template <class Request>
  void dispatchShadowRequest(
      size_t idx,
      std::shared_ptr<Request> adjustedReq,
      folly::Function<void(const ReplyT<Request>&)> postShadowReplyFn) {
    folly::fibers::addTask([shadow = shadowSelect<Request>(idx),
                            postShadowReplyFn = std::move(postShadowReplyFn),
                            adjustedReq = std::move(adjustedReq)]() mutable {
      // we don't want to spool shadow requests
      mcrouter::fiber_local<RouterInfo>::clearAsynclogName();
      mcrouter::fiber_local<RouterInfo>::addRequestClass(
          mcrouter::RequestClass::kShadow);
      const auto shadowReply = shadow->route(*adjustedReq);
      if (postShadowReplyFn) {
        postShadowReplyFn(shadowReply);
      }
    });
  }

  /*
   * Will shadow to this destination if generated random number is lower
   * than the entry in the probability vector.
   */
  bool shouldShadow(size_t idx) {
    if (idx >= shadowChildren_.size()) {
      return false;
    }
    return ((gen_() % 100) < shadowProbabilities_[idx]);
  }
};

} // namespace memcache
} // namespace facebook
