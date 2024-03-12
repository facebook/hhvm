/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/ProxyDestination.h"

#include <limits>
#include <random>

#include "mcrouter/OptionsUtil.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyDestinationMap.h"
#include "mcrouter/config-impl.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/Clocks.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/mc/protocol.h"
#include "mcrouter/lib/network/ConnectionDownReason.h"
#include "mcrouter/lib/network/ConnectionOptions.h"
#include "mcrouter/lib/network/RpcStatsContext.h"

namespace folly {
class AsyncTransport;
} // namespace folly

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class Transport>
template <class Request>
ReplyT<Request> ProxyDestination<Transport>::send(
    const Request& request,
    DestinationRequestCtx& requestContext,
    std::chrono::milliseconds timeout,
    RpcStatsContext& rpcStatsContext) {
  markAsActive();
  auto reply = getTransport().sendSync(request, timeout, &rpcStatsContext);
  onReply(
      *reply.result_ref(),
      requestContext,
      rpcStatsContext,
      request.isBufferDirty());
  return reply;
}

template <class Transport>
bool ProxyDestination<Transport>::latencyAboveThreshold(uint64_t latency) {
  const auto rxmitDeviation =
      proxy().router().opts().rxmit_latency_deviation_us;
  if (!rxmitDeviation) {
    return false;
  }
  return (
      static_cast<double>(latency) - stats().avgLatency.value() >
      static_cast<double>(rxmitDeviation));
}

template <class Transport>
void ProxyDestination<Transport>::handleRxmittingConnection(
    const carbon::Result result,
    uint64_t latency) {
  constexpr uint32_t kReconnectionHoldoffFactor = 25;
  if (!transport_) {
    return;
  }
  const auto retransCycles =
      proxy().router().opts().collect_rxmit_stats_every_hz;
  if (retransCycles > 0 &&
      (isDataTimeoutResult(result) || latencyAboveThreshold(latency))) {
    const auto curCycles = cycles::getCpuCycles();
    if (curCycles > lastRetransCycles_ + retransCycles) {
      lastRetransCycles_ = curCycles;
      const auto currRetransPerKByte = transport_->getRetransmitsPerKb();
      if (currRetransPerKByte >= 0.0) {
        stats().retransPerKByte = currRetransPerKByte;
        proxy().stats().setValue(
            retrans_per_kbyte_max_stat,
            std::max(
                proxy().stats().getValue(retrans_per_kbyte_max_stat),
                static_cast<uint64_t>(currRetransPerKByte)));
        proxy().stats().increment(
            retrans_per_kbyte_sum_stat,
            static_cast<int64_t>(currRetransPerKByte));
        proxy().stats().increment(retrans_num_total_stat);
      }

      if (proxy().router().isRxmitReconnectionDisabled()) {
        return;
      }

      if (rxmitsToCloseConnection_ > 0 &&
          currRetransPerKByte >= rxmitsToCloseConnection_) {
        std::uniform_int_distribution<uint64_t> dist(
            1, kReconnectionHoldoffFactor);
        const uint64_t reconnectionJitters =
            retransCycles * dist(proxy().randomGenerator());
        if (lastConnCloseCycles_ + reconnectionJitters > curCycles) {
          return;
        }
        transport_->closeNow();
        proxy().stats().increment(retrans_closed_connections_stat);
        lastConnCloseCycles_ = curCycles;

        const auto maxThreshold =
            proxy().router().opts().max_rxmit_reconnect_threshold;
        const uint64_t maxRxmitReconnThreshold = maxThreshold == 0
            ? std::numeric_limits<uint64_t>::max()
            : maxThreshold;
        rxmitsToCloseConnection_ =
            std::min(maxRxmitReconnThreshold, 2 * rxmitsToCloseConnection_);
      } else if (3 * currRetransPerKByte < rxmitsToCloseConnection_) {
        const auto minThreshold =
            proxy().router().opts().min_rxmit_reconnect_threshold;
        rxmitsToCloseConnection_ =
            std::max(minThreshold, rxmitsToCloseConnection_ / 2);
      }
    }
  }
}

template <class Transport>
void ProxyDestination<Transport>::updateTransportTimeoutsIfShorter(
    std::chrono::milliseconds shortestConnectTimeout,
    std::chrono::milliseconds shortestWriteTimeout) {
  std::unique_lock g(transportLock_);
  if (transport_) {
    transport_->updateTimeoutsIfShorter(
        shortestConnectTimeout, shortestWriteTimeout);
  }
}

template <class Transport>
carbon::Result ProxyDestination<Transport>::sendProbe() {
  // Will reconnect if connection was closed
  // Version commands shouldn't take much longer than stablishing a
  // connection, so just using shortestConnectTimeout() here.
  return getTransport()
      .sendSync(McVersionRequest(), shortestWriteTimeout())
      .result_ref()
      .value();
}

template <class Transport>
void ProxyDestination<Transport>::onReply(
    const carbon::Result result,
    DestinationRequestCtx& destreqCtx,
    const RpcStatsContext& rpcStatsContext,
    bool isRequestBufferDirty) {
  handleTko(result, /* isProbeRequest */ false);

  if (!stats().results) {
    stats().results = std::make_unique<std::array<
        uint64_t,
        static_cast<size_t>(carbon::Result::NUM_RESULTS)>>();
  }
  ++(*stats().results)[static_cast<size_t>(result)];
  destreqCtx.endTime = nowUs();

  int64_t latency = destreqCtx.endTime - destreqCtx.startTime;
  stats().avgLatency.insertSample(latency);

  if (accessPoint()->compressed()) {
    if (rpcStatsContext.usedCodecId > 0) {
      proxy().stats().increment(replies_compressed_stat);
    } else {
      proxy().stats().increment(replies_not_compressed_stat);
    }
    proxy().stats().increment(
        reply_traffic_before_compression_stat,
        rpcStatsContext.replySizeBeforeCompression);
    proxy().stats().increment(
        reply_traffic_after_compression_stat,
        rpcStatsContext.replySizeAfterCompression);
  }

  proxy().stats().increment(destination_reqs_total_sum_stat);
  if (isRequestBufferDirty) {
    proxy().stats().increment(destination_reqs_dirty_buffer_sum_stat);
  }

  handleRxmittingConnection(result, latency);
}

template <class Transport>
typename Transport::RequestQueueStats
ProxyDestination<Transport>::getRequestStats() const {
  RequestQueueStats stats{0, 0};
  {
    std::unique_lock g(transportLock_);
    if (transport_) {
      stats = transport_->getRequestQueueStats();
    }
  }
  return stats;
}

template <class Transport>
std::shared_ptr<ProxyDestination<Transport>>
ProxyDestination<Transport>::create(
    ProxyBase& proxy,
    std::shared_ptr<AccessPoint> ap,
    std::chrono::milliseconds timeout,
    uint32_t qosClass,
    uint32_t qosPath,
    uint32_t idx) {
  checkLogic(
      Transport::isCompatible(ap->getProtocol()),
      "Transport {} not compatible with {} protocol.",
      Transport::name(),
      mc_protocol_to_string(ap->getProtocol()));
  std::shared_ptr<ProxyDestination<Transport>> ptr(
      new ProxyDestination<Transport>(
          proxy, std::move(ap), timeout, qosClass, qosPath, idx));
  ptr->selfPtr_ = ptr;
  return ptr;
}

template <class Transport>
ProxyDestination<Transport>::~ProxyDestination() {
  if (proxy().destinationMap()) {
    // Only remove if we are not shutting down Proxy.
    proxy().destinationMap()->removeDestination(*this);
  }

  if (transport_) {
    transport_->setConnectionStatusCallbacks(
        ConnectionStatusCallbacks{nullptr, nullptr});
    transport_->closeNow();
  }

  onTransitionFromState(stats().state);
  proxy().stats().decrement(num_servers_stat);
  if (accessPoint()->useSsl()) {
    proxy().stats().decrement(num_ssl_servers_stat);
  }
}

template <class Transport>
ProxyDestination<Transport>::ProxyDestination(
    ProxyBase& proxy,
    std::shared_ptr<AccessPoint> ap,
    std::chrono::milliseconds timeout,
    uint32_t qosClass,
    uint32_t qosPath,
    uint32_t idx)
    : ProxyDestinationBase(
          proxy,
          std::move(ap),
          timeout,
          qosClass,
          qosPath,
          idx),
      rxmitsToCloseConnection_(
          proxy.router().opts().min_rxmit_reconnect_threshold) {}

template <class Transport>
void ProxyDestination<Transport>::resetInactive() {
  // No need to reset non-existing client.
  if (transport_) {
    std::unique_ptr<Transport, typename Transport::Destructor> client;
    {
      std::unique_lock g(transportLock_);
      client = std::move(transport_);
    }
    client->closeNow();
    stats().inactiveConnectionClosedTimestampUs = nowUs();
  }
}

namespace detail {
void incrementProxyTlsKtlsSslStats(
    const folly::AsyncTransport& socket,
    SecurityMech mech,
    ProxyBase& proxy);
} // namespace detail

template <class Transport>
void ProxyDestination<Transport>::initializeTransport() {
  assert(!transport_);

  ConnectionOptions options(accessPoint());
  auto& opts = proxy().router().opts();
  options.tcpKeepAliveCount = opts.keepalive_cnt;
  options.tcpKeepAliveIdle = opts.keepalive_idle_s;
  options.tcpKeepAliveInterval = opts.keepalive_interval_s;
  options.numConnectTimeoutRetries = opts.connect_timeout_retries;
  options.connectTimeout = shortestConnectTimeout();
  options.writeTimeout = shortestWriteTimeout();
  options.routerInfoName = proxy().router().routerInfoName();
  if (!opts.debug_fifo_root.empty()) {
    options.debugFifoPath = getClientDebugFifoFullPath(opts);
  }
  if (opts.enable_qos) {
    options.enableQoS = true;
    options.qosClass = qosClass();
    options.qosPath = qosPath();
  }
  options.useJemallocNodumpAllocator = opts.jemalloc_nodump_buffers;
  if (accessPoint()->compressed()) {
    if (auto codecManager = proxy().router().getCodecManager()) {
      options.compressionCodecMap =
          codecManager->getCodecMap(proxy().eventBase().getEventBase());
      options.thriftCompression = true;
      options.thriftCompressionThreshold = opts.thrift_compression_threshold;
    }
  }

  if (accessPoint()->useSsl()) {
    options.securityOpts.sslPemCertPath = opts.pem_cert_path;
    options.securityOpts.sslPemKeyPath = opts.pem_key_path;
    if (opts.ssl_verify_peers) {
      options.securityOpts.sslPemCaPath = opts.pem_ca_path;
    }
    options.securityOpts.sessionCachingEnabled = opts.ssl_connection_cache;
    options.securityOpts.sslHandshakeOffload = opts.ssl_handshake_offload;
    options.securityOpts.sslServiceIdentity = opts.ssl_service_identity;
    options.securityOpts.sslAuthorizationEnforce =
        opts.ssl_service_identity_authorization_enforce;
    options.securityOpts.tfoEnabledForSsl = opts.enable_ssl_tfo;
    options.securityOpts.tlsPreferOcbCipher = opts.tls_prefer_ocb_cipher;
  }

  auto client = std::unique_ptr<Transport, typename Transport::Destructor>(
      new Transport(proxy().eventBase(), std::move(options)),
      typename Transport::Destructor());
  {
    std::unique_lock g(transportLock_);
    transport_ = std::move(client);
  }

  transport_->setFlushList(&proxy().flushList());

  transport_->setRequestStatusCallbacks(RequestStatusCallbacks{
      [this](int pending, int inflight) { // onStateChange
        if (pending != 0) {
          proxy().stats().increment(destination_pending_reqs_stat, pending);
          proxy().stats().setValue(
              destination_max_pending_reqs_stat,
              std::max(
                  proxy().stats().getValue(destination_max_pending_reqs_stat),
                  proxy().stats().getValue(destination_pending_reqs_stat)));
        }
        if (inflight != 0) {
          proxy().stats().increment(destination_inflight_reqs_stat, inflight);
          proxy().stats().setValue(
              destination_max_inflight_reqs_stat,
              std::max(
                  proxy().stats().getValue(destination_max_inflight_reqs_stat),
                  proxy().stats().getValue(destination_inflight_reqs_stat)));
        }
      },
      [this](size_t numToSend) { // onWrite
        proxy().stats().increment(num_socket_writes_stat);
        proxy().stats().increment(destination_batches_sum_stat);
        proxy().stats().increment(destination_requests_sum_stat, numToSend);
      },
      [this]() { // onPartialWrite
        proxy().stats().increment(num_socket_partial_writes_stat);
      }});

  transport_->setConnectionStatusCallbacks(ConnectionStatusCallbacks{
      [this](
          const folly::AsyncTransport& socket,
          int64_t numConnectRetries) mutable {
        setState(State::Up);
        proxy().stats().increment(num_connections_opened_stat);

        updatePoolStatConnections(true);
        detail::incrementProxyTlsKtlsSslStats(
            socket, accessPoint()->getSecurityMech(), proxy());
        if (numConnectRetries > 0) {
          proxy().stats().increment(num_connect_success_after_retrying_stat);
          proxy().stats().increment(
              num_connect_retries_stat, numConnectRetries);
        }
        updateConnectionClosedInternalStat();
      },
      [pdstnPtr = selfPtr_](
          ConnectionDownReason reason, int64_t numConnectRetries) {
        auto pdstn = pdstnPtr.lock();
        if (!pdstn) {
          LOG(WARNING) << "Proxy destination is already destroyed. "
                          "Stats will not be bumped.";
          return;
        }

        pdstn->proxy().stats().increment(num_connections_closed_stat);
        if (pdstn->accessPoint()->useSsl()) {
          auto mech = pdstn->accessPoint()->getSecurityMech();
          if (mech == SecurityMech::TLS_TO_PLAINTEXT) {
            pdstn->proxy().stats().increment(
                num_tls_to_plain_connections_closed_stat);
          } else if (mech == SecurityMech::KTLS12) {
            pdstn->proxy().stats().increment(num_ktls_connections_closed_stat);
          } else {
            pdstn->proxy().stats().increment(num_ssl_connections_closed_stat);
          }
        }

        pdstn->updatePoolStatConnections(false);

        if (reason == ConnectionDownReason::ABORTED) {
          pdstn->setState(State::Closed);
        } else {
          // In case of server going away, we should gracefully close the
          // connection (i.e. allow remaining outstanding requests to drain).
          if (reason == ConnectionDownReason::SERVER_GONE_AWAY) {
            pdstn->closeGracefully();
          }
          pdstn->setState(State::Down);
          pdstn->handleTko(
              reason == ConnectionDownReason::CONNECT_TIMEOUT
                  ? carbon::Result::CONNECT_TIMEOUT
                  : carbon::Result::CONNECT_ERROR,
              /* isProbeRequest= */ false);
        }

        pdstn->proxy().stats().increment(
            num_connect_retries_stat, numConnectRetries);
      }});

  transport_->setAuthorizationCallbacks(AuthorizationCallbacks{
      [this](
          const folly::AsyncTransport& socket,
          const ConnectionOptions& connectionOptions) mutable {
        if (auto& callback = proxy().router().svcIdentAuthCallbackFunc()) {
          if (!callback(socket, connectionOptions)) {
            proxy().stats().increment(num_authorization_failures_stat);
            return false;
          } else {
            proxy().stats().increment(num_authorization_successes_stat);
          }
        }
        return true;
      }});

  if (opts.target_max_inflight_requests > 0) {
    transport_->setThrottle(
        opts.target_max_inflight_requests, opts.target_max_pending_requests);
  }
}

template <class Transport>
void ProxyDestination<Transport>::closeGracefully() {
  if (transport_) {
    // In case we have outstanding probe, we should close now, to get it
    // properly cleared.
    if (probeInflight()) {
      transport_->closeNow();
    }
    // Check again, in case we reset it in closeNow()
    if (transport_) {
      transport_->setConnectionStatusCallbacks(
          ConnectionStatusCallbacks{nullptr, nullptr});
      std::unique_ptr<Transport, typename Transport::Destructor> client;
      {
        std::unique_lock g(transportLock_);
        client = std::move(transport_);
      }
      client.reset();
    }
  }
}

template <class Transport>
Transport& ProxyDestination<Transport>::getTransport() {
  if (!transport_) {
    initializeTransport();
  }
  return *transport_;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
