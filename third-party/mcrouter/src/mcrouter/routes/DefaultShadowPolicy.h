/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <folly/Function.h>

#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Default shadow policy: send exactly the same request to shadow
 * as the original; send out shadow requests right away.
 */
class DefaultShadowPolicy {
 public:
  DefaultShadowPolicy() = default;

  explicit DefaultShadowPolicy(CarbonRouterInstanceBase& router)
      : router_(&router) {}

  template <class Request>
  static constexpr bool supports() {
    return true;
  }

  template <class Request>
  std::shared_ptr<const Request> makeAdjustedNormalRequest(
      const Request& req) const {
    return std::make_shared<Request>(req);
  }

  template <class Request>
  std::shared_ptr<const Request> makeShadowRequest(
      const std::shared_ptr<const Request>& normalReq) const {
    return normalReq;
  }

  std::shared_ptr<const McLeaseSetRequest> makeShadowRequest(
      const std::shared_ptr<const McLeaseSetRequest>& normalReq) const {
    if (!router_) {
      return normalReq;
    }

    auto shadowReq = std::make_shared<McLeaseSetRequest>(*normalReq);
    shadowReq->leaseToken_ref() = router_->shadowLeaseTokenMap().withLock(
        [normalToken = *normalReq->leaseToken_ref()](const auto& tokenMap) {
          // For now, we assume at most one shadow request per normal request,
          // i.e., there is a mapping from normal lease token -> shadow lease
          // token. This can be made more robust by reworking the mapping to be
          // (normal token, shadow host) -> shadow token.
          auto it = tokenMap.findWithoutPromotion(normalToken);
          return it != tokenMap.end() ? it->second : 0 /* invalid token */;
        });
    return shadowReq;
  }

  template <class Reply>
  folly::Function<void(const Reply&)> makePostShadowReplyFn(
      const Reply& /* normalReply */) const {
    return {};
  }

  template <class Request>
  constexpr bool shouldDelayShadow() const {
    return false;
  }

 private:
  CarbonRouterInstanceBase* const router_{nullptr};
};

template <>
inline folly::Function<void(const McLeaseGetReply&)>
DefaultShadowPolicy::makePostShadowReplyFn(
    const McLeaseGetReply& normalReply) const {
  constexpr uint64_t kHotMissLeaseToken = 1;

  if (!router_ ||
      !(isMissResult(*normalReply.result_ref()) &&
        static_cast<uint64_t>(*normalReply.leaseToken_ref()) >
            kHotMissLeaseToken)) {
    return {};
  }

  return [&shadowTokenMap = router_->shadowLeaseTokenMap(),
          normalToken = *normalReply.leaseToken_ref()](
             const McLeaseGetReply& shadowReply) {
    if (!(isMissResult(*shadowReply.result_ref()) &&
          static_cast<uint64_t>(*shadowReply.leaseToken_ref()) >
              kHotMissLeaseToken)) {
      return;
    }

    shadowTokenMap.withLock(
        [normalToken, shadowToken = *shadowReply.leaseToken_ref()](
            auto& tokenMap) { tokenMap.set(normalToken, shadowToken); });
  };
}

template <>
constexpr bool DefaultShadowPolicy::shouldDelayShadow<McLeaseGetRequest>()
    const {
  // In order to insert the right normalToken -> shadowToken pair into
  // router_->shadowLeaseTokenMap(), we need the normal lease-get request to
  // complete before the shadow request.
  return true;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
