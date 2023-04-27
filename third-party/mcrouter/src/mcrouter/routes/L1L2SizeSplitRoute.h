/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <random>
#include <string>
#include <utility>

#include <folly/Conv.h>
#include <folly/Random.h>
#include <folly/Range.h>
#include <folly/fibers/FiberManager.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/McKey.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
class RouteHandleFactory;

namespace mcrouter {

/**
 * This route handle is intended to be used for asymmetric data storage.
 *
 * Values smaller than threshold are only written to L1 pool. Above, and
 * a small sentinel value is left in the L1 pool. Optionally, full data
 * can be written to both pools.
 *
 * Operations supported are set, get, lease-set, lease-get, gets and cas.
 * All other operations will go to L1.
 *
 */
template <class RouterInfo>
class L1L2SizeSplitRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  static constexpr uint32_t kDefaultNumRetries = 1;
  static constexpr uint32_t kMaxNumRetries = 10;
  static std::string routeName() {
    return "l1l2-sizesplit";
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    if constexpr (!carbon::ListContains<
                      typename memcache::MemcacheRouterInfo::AllRequests,
                      Request>::value) {
      return false;
    }
    if (t(*l1_, req)) {
      return true;
    }
    return t(*l2_, req);
  }

  L1L2SizeSplitRoute(
      RouteHandlePtr l1,
      RouteHandlePtr l2,
      size_t threshold,
      int32_t ttlThreshold,
      int32_t failureTtl,
      bool bothFullSet,
      uint32_t numRetries)
      : l1_(std::move(l1)),
        l2_(std::move(l2)),
        threshold_(threshold),
        ttlThreshold_(ttlThreshold),
        failureTtl_(failureTtl),
        bothFullSet_(bothFullSet),
        numRetries_(numRetries) {
    assert(l1_ != nullptr);
    assert(l2_ != nullptr);
    folly::Random::seed(randomGenerator_);
  }

  // Get will go to L1 and if it receives sentinel, to L2.
  McGetReply route(const McGetRequest& req) const;

  // If below threshold_ set to L1 without sentinel flag set.
  // If above threshold_ set to L2 and also set sentinel to L1.
  McSetReply route(const McSetRequest& req) const;

  // If below threshold_ set to L1 without sentinel flag set.
  // If above threshold_ set to L2 with a custom key using hash alias and also
  // set sentinel to L1.
  McLeaseSetReply route(const McLeaseSetRequest& req) const;

  // McLeaseGet: lease-get will go to L1 and if it receives sentinel, to L2 with
  // a custom key using the hash alias and the integer value in the sentinel.
  // Gets: Gets to L1, if sentinel then fetch value from L2.
  template <class Request>
  typename std::enable_if<
      folly::IsOneOf<Request, McLeaseGetRequest, McGetsRequest>::value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    return doRoute(req, numRetries_ /* retriesLeft */);
  }

  // All other operations should route to L1, including CAS which to guarantee
  // consistency, will go to L1 only.
  template <class Request>
  typename std::enable_if<
      !folly::IsOneOf<Request, McLeaseGetRequest, McGetsRequest>::value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    if constexpr (!carbon::ListContains<
                      typename memcache::MemcacheRouterInfo::AllRequests,
                      Request>::value) {
      return createReply<Request>(ErrorReply, "Unsupported request type");
    }
    return l1_->route(req);
  }

  template <class Request>
  typename std::enable_if<
      folly::IsOneOf<Request, McLeaseGetRequest, McGetsRequest>::value,
      ReplyT<Request>>::type
  doRoute(const Request& req, size_t retriesLeft) const;

 private:
  static constexpr folly::StringPiece kHashAlias = "|==|";
  static constexpr size_t kMaxMcKeyLength = 255;
  static constexpr size_t kExtraKeySpaceNeeded = kHashAlias.size() +
      detail::numDigitsBase10(std::numeric_limits<uint64_t>::max());

  const std::shared_ptr<RouteHandleIf> l1_;
  const std::shared_ptr<RouteHandleIf> l2_;
  const size_t threshold_{0};
  const int32_t ttlThreshold_{0};
  const int32_t failureTtl_{0};
  const bool bothFullSet_{false};
  const uint32_t numRetries_{kDefaultNumRetries};
  mutable std::mt19937 randomGenerator_;

  McLeaseGetReply doLeaseGetRoute(
      const McLeaseGetRequest& req,
      size_t retriesLeft) const;

  template <class Suffix>
  static std::string makeL2Key(folly::StringPiece key, Suffix randomSuffix) {
    return folly::to<std::string>(key, kHashAlias, randomSuffix);
  }

  template <class Request>
  inline void deleteSentinel(const Request& req, folly::StringPiece l1Value)
      const {
    McGetsRequest l1GetsRequest(req.key_ref()->fullKey());
    auto l1GetsReply = l1_->route(l1GetsRequest);
    if (isHitResult(*l1GetsReply.result_ref()) &&
        coalesceAndGetRange(*l1GetsReply.value_ref()) == l1Value) {
      McCasRequest l1CasRequest(req.key_ref()->fullKey());
      l1CasRequest.casToken_ref() = *l1GetsReply.casToken_ref();
      l1CasRequest.exptime_ref() = -1;
      l1_->route(l1CasRequest);
    }
    return;
  }

  template <class Request>
  bool fullSetShouldGoToL1(const Request& req) const {
    return threshold_ == 0 ||
        req.value_ref()->computeChainDataLength() < threshold_ ||
        (ttlThreshold_ != 0 && *req.exptime_ref() < ttlThreshold_) ||
        req.key_ref()->fullKey().size() + kExtraKeySpaceNeeded >
        kMaxMcKeyLength;
  }
  void augmentReply(McGetsReply& reply, const McGetsReply& l1Reply) const {
    reply.casToken_ref() = *l1Reply.casToken_ref();
  }
  void augmentReply(
      McLeaseGetReply& /* unused */,
      const McLeaseGetReply& /* unused */) const {}

  folly::Optional<McLeaseGetReply> doFilter(const McLeaseGetReply& reply) const;

  template <class Reply>
  folly::Optional<Reply> doFilter(const Reply& reply) const;
};

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeL1L2SizeSplitRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "mcrouter/routes/L1L2SizeSplitRoute-inl.h"
