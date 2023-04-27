/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <array>
#include <memory>
#include <string>

#include <folly/Range.h>
#include <folly/SpinLock.h>

#include "mcrouter/ProxyDestinationBase.h"
#include "mcrouter/TkoLog.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/Transport.h"

namespace facebook {
namespace memcache {

struct AccessPoint;
struct RpcStatsContext;

namespace mcrouter {

class ProxyBase;
class ProxyDestinationMap;
class TkoTracker;

struct DestinationRequestCtx {
  int64_t startTime{0};
  int64_t endTime{0};

  explicit DestinationRequestCtx(int64_t now) : startTime(now) {}
};

template <class Transport>
class ProxyDestination : public ProxyDestinationBase {
 public:
  using ConnectionStatusCallbacks =
      typename Transport::ConnectionStatusCallbacks;
  using RequestStatusCallbacks = typename Transport::RequestStatusCallbacks;
  using AuthorizationCallbacks = typename Transport::AuthorizationCallbacks;
  using RequestQueueStats = typename Transport::RequestQueueStats;

  ~ProxyDestination();

  /**
   * Sends a request to this destination.
   * NOTE: This is a blocking call that will return reply, once it's ready.
   *
   * @param request             The request to send.
   * @param requestContext      Context about this request.
   * @param timeout             The timeout of this call.
   * @param rpcStatsContext     Output argument with stats about the RPC
   */
  template <class Request>
  ReplyT<Request> send(
      const Request& request,
      DestinationRequestCtx& requestContext,
      std::chrono::milliseconds timeout,
      RpcStatsContext& rpcStatsContext);

  void resetInactive() override final;

  /**
   * Gracefully closes the connection, allowing it to properly drain if
   * possible.
   */
  void closeGracefully();

  RequestQueueStats getRequestStats() const override final;

 protected:
  void updateTransportTimeoutsIfShorter(
      std::chrono::milliseconds shortestConnectTimeout,
      std::chrono::milliseconds shortestWriteTimeout) override final;
  carbon::Result sendProbe() override final;
  std::weak_ptr<ProxyDestinationBase> selfPtr() override final {
    return selfPtr_;
  }

  std::shared_ptr<const AccessPoint> replaceAP(
      std::shared_ptr<const AccessPoint> newAccessPoint) {
    auto ret = ProxyDestinationBase::replaceAP(newAccessPoint);
    closeGracefully();
    return ret;
  }

 private:
  std::unique_ptr<Transport, typename Transport::Destructor> transport_;
  // Ensure proxy thread doesn't reset the Transport
  // while config and stats threads may be accessing it
  mutable folly::SpinLock transportLock_;

  // Retransmits control information
  uint64_t lastRetransCycles_{0}; // Cycles when restransmits were last fetched
  uint64_t rxmitsToCloseConnection_{0};
  uint64_t lastConnCloseCycles_{0}; // Cycles when connection was last closed

  /**
   * Creates a new ProxyDestination.
   *
   * @throws std::logic_error If Transport is not compatible with
   *                          AccessPoint::getProtocol().
   */
  static std::shared_ptr<ProxyDestination> create(
      ProxyBase& proxy,
      std::shared_ptr<AccessPoint> ap,
      std::chrono::milliseconds timeout,
      uint32_t qosClass,
      uint32_t qosPath,
      uint32_t idx = 0);

  Transport& getTransport();
  void initializeTransport();

  ProxyDestination(
      ProxyBase& proxy,
      std::shared_ptr<AccessPoint> ap,
      std::chrono::milliseconds timeout,
      uint32_t qosClass,
      uint32_t qosPath,
      uint32_t idx);

  // Process tko, stats and duration timer.
  void onReply(
      const carbon::Result result,
      DestinationRequestCtx& destreqCtx,
      const RpcStatsContext& rpcStatsContext,
      bool isRequestBufferDirty);

  void handleRxmittingConnection(const carbon::Result result, uint64_t latency);
  bool latencyAboveThreshold(uint64_t latency);

  std::weak_ptr<ProxyDestination> selfPtr_;

  friend class ProxyDestinationMap;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "ProxyDestination-inl.h"
