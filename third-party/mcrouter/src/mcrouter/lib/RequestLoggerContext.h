/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <folly/Range.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/carbon/RequestReplyUtil.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/RpcStatsContext.h"

namespace folly {
class IOBuf;
} // namespace folly

namespace facebook {
namespace memcache {

struct AccessPoint;

namespace mcrouter {

enum class RequestLoggerContextFlags : uint8_t {
  NONE = 0x00,
  // Underlying routing is SR based
  USING_SR = 0x01,
};

/*
 * union operator
 */
constexpr RequestLoggerContextFlags operator|(
    RequestLoggerContextFlags a,
    RequestLoggerContextFlags b) {
  return static_cast<RequestLoggerContextFlags>(
      static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

/*
 * compound assignment union operator
 */
constexpr RequestLoggerContextFlags& operator|=(
    RequestLoggerContextFlags& a,
    RequestLoggerContextFlags b) {
  a = a | b;
  return a;
}

/*
 * intersection operator
 */
constexpr RequestLoggerContextFlags operator&(
    RequestLoggerContextFlags a,
    RequestLoggerContextFlags b) {
  return static_cast<RequestLoggerContextFlags>(
      static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

/*
 * compound assignment intersection operator
 */
constexpr RequestLoggerContextFlags& operator&=(
    RequestLoggerContextFlags& a,
    RequestLoggerContextFlags b) {
  a = a & b;
  return a;
}

/*
 * exclusion parameter
 */
constexpr RequestLoggerContextFlags operator~(RequestLoggerContextFlags a) {
  return static_cast<RequestLoggerContextFlags>(~static_cast<uint32_t>(a));
}

/*
 * unset operator
 */
constexpr RequestLoggerContextFlags unSet(
    RequestLoggerContextFlags a,
    RequestLoggerContextFlags b) {
  return a & ~b;
}

/*
 * inclusion operator
 */
constexpr bool isSet(RequestLoggerContextFlags a, RequestLoggerContextFlags b) {
  return (a & b) == b;
}

struct RequestLoggerContext {
  RequestLoggerContext(
      const folly::StringPiece poolName_,
      const AccessPoint& ap_,
      folly::StringPiece strippedRoutingPrefix_,
      RequestClass requestClass_,
      const int64_t startTimeUs_,
      const int64_t endTimeUs_,
      const carbon::Result replyResult_,
      const RpcStatsContext rpcStatsContext_,
      const int64_t networkTransportTimeUs_,
      const std::vector<ExtraDataCallbackT>& extraDataCallbacks_,
      const std::string_view bucketId_,
      const RequestLoggerContextFlags flags_ = RequestLoggerContextFlags::NONE,
      const uint32_t numFailovers_ = 0,
      int64_t beforeReqLatencyInjectedUs_ = 0,
      int64_t afterReqLatencyInjectedUs_ = 0,
      std::optional<size_t> poolIndex_ = std::nullopt,
      std::optional<int64_t> productId_ = std::nullopt)
      : strippedRoutingPrefix(strippedRoutingPrefix_),
        requestClass(requestClass_),
        poolName(poolName_),
        ap(ap_),
        startTimeUs(startTimeUs_),
        endTimeUs(endTimeUs_),
        replyResult(replyResult_),
        rpcStatsContext(rpcStatsContext_),
        networkTransportTimeUs(networkTransportTimeUs_),
        extraDataCallbacks(extraDataCallbacks_),
        bucketId(bucketId_),
        flags(flags_),
        numFailovers(numFailovers_),
        beforeReqLatencyInjectedUs(beforeReqLatencyInjectedUs_),
        afterReqLatencyInjectedUs(afterReqLatencyInjectedUs_),
        poolIndex(poolIndex_),
        productId(productId_) {}

  RequestLoggerContext(const RequestLoggerContext&) = delete;
  RequestLoggerContext& operator=(const RequestLoggerContext&) = delete;

  const folly::StringPiece strippedRoutingPrefix;
  const RequestClass requestClass;
  const folly::StringPiece poolName;
  const AccessPoint& ap;
  const int64_t startTimeUs;
  const int64_t endTimeUs;
  const carbon::Result replyResult;
  const RpcStatsContext rpcStatsContext;
  const int64_t networkTransportTimeUs;
  const std::vector<ExtraDataCallbackT>& extraDataCallbacks;
  const std::string_view bucketId;
  const RequestLoggerContextFlags flags;
  const uint32_t numFailovers;
  const int64_t beforeReqLatencyInjectedUs;
  const int64_t afterReqLatencyInjectedUs;
  std::optional<size_t> poolIndex;
  std::optional<int64_t> productId;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
