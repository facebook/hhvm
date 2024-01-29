/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/ProxyDestination.h"

#include <folly/io/async/AsyncSSLSocket.h>

#include "mcrouter/lib/network/AsyncTlsToPlaintextSocket.h"
#include "mcrouter/lib/network/McFizzClient.h"
#include "mcrouter/lib/network/McSSLUtil.h"
#include "mcrouter/lib/network/TlsToPlainTransport.h"

namespace facebook::memcache::mcrouter::detail {

void incrementProxyTlsKtlsSslStats(
    const folly::AsyncTransport& socket,
    SecurityMech mech,
    ProxyBase& proxy) {
  if (mech == SecurityMech::TLS_TO_PLAINTEXT) {
    if (const auto* tlsToPlainSock =
            socket.getUnderlyingTransport<TlsToPlainTransport>()) {
      auto stats = tlsToPlainSock->getStats();
      proxy.stats().increment(num_tls_to_plain_connections_opened_stat);
      if (stats.sessionReuseAttempted) {
        proxy.stats().increment(num_tls_to_plain_resumption_attempts_stat);
      }
      if (stats.sessionReuseSuccess) {
        proxy.stats().increment(num_tls_to_plain_resumption_successes_stat);
      }
    } else if (
        const auto* thriftTlsToPlainSock =
            socket.getUnderlyingTransport<AsyncTlsToPlaintextSocket>()) {
      proxy.stats().increment(num_tls_to_plain_connections_opened_stat);

      using Status = AsyncTlsToPlaintextSocket::SessionResumptionStatus;
      switch (thriftTlsToPlainSock->getSessionResumptionStatus()) {
        case Status::RESUMPTION_NOT_ATTEMPTED:
          break;
        case Status::RESUMPTION_ATTEMPTED_AND_SUCCEEDED:
          proxy.stats().increment(num_tls_to_plain_resumption_successes_stat);
          [[fallthrough]];
        case Status::RESUMPTION_ATTEMPTED_AND_FAILED:
          proxy.stats().increment(num_tls_to_plain_resumption_attempts_stat);
      };
    } else {
      proxy.stats().increment(num_tls_to_plain_fallback_failures_stat);
    }
  }
  if (mech == SecurityMech::KTLS12) {
    auto stats = McSSLUtil::getKtlsStats(socket);
    if (stats) {
      proxy.stats().increment(num_ktls_connections_opened_stat);
      if (stats->sessionReuseAttempted) {
        proxy.stats().increment(num_ktls_resumption_attempts_stat);
      }
      if (stats->sessionReuseSuccess) {
        proxy.stats().increment(num_ktls_resumption_successes_stat);
      }
    } else {
      proxy.stats().increment(num_ktls_fallback_failures_stat);
    }
  }
  // no else if here in case the tls to plain didn't work - we can capture
  // ssl socket stats here
  if (const auto* sslSocket =
          socket.getUnderlyingTransport<folly::AsyncSSLSocket>()) {
    proxy.stats().increment(num_ssl_connections_opened_stat);
    if (sslSocket->sessionResumptionAttempted()) {
      proxy.stats().increment(num_ssl_resumption_attempts_stat);
    }
    if (sslSocket->getSSLSessionReused()) {
      proxy.stats().increment(num_ssl_resumption_successes_stat);
    }
  } else if (
      const auto* fizzSock = socket.getUnderlyingTransport<McFizzClient>()) {
    proxy.stats().increment(num_ssl_connections_opened_stat);
    if (fizzSock->pskResumed()) {
      proxy.stats().increment(num_ssl_resumption_successes_stat);
      proxy.stats().increment(num_ssl_resumption_attempts_stat);
    } else {
      auto pskState = fizzSock->getState().pskType();
      if (pskState && pskState.value() == fizz::PskType::Rejected) {
        // session resumption was attempted, but failed
        proxy.stats().increment(num_ssl_resumption_attempts_stat);
      }
    }
  }
}

} // namespace facebook::memcache::mcrouter::detail
