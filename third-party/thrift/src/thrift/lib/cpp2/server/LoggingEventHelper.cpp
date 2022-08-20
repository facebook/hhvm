/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/lang/Assume.h>

#include <fizz/server/AsyncFizzServer.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/LoggingEventHelper.h>

using fizz::server::AsyncFizzServer;

namespace apache {
namespace thrift {

namespace {
void logTlsNoPeerCertEvent(const ConnectionLoggingContext& context) {
  DCHECK(context.getWorker() && context.getWorker()->getServer());
  const auto& configSource =
      context.getWorker()->getServer()->metadata().tlsConfigSource;
  if (configSource && *configSource == kDefaultTLSConfigSource) {
    THRIFT_CONNECTION_EVENT(tls.no_peer_cert.config_default).log(context);
  } else {
    THRIFT_CONNECTION_EVENT(tls.no_peer_cert.config_manual).log(context);
  }
}

void logNonTLSEvent(const ConnectionLoggingContext& context) {
  DCHECK(context.getWorker() && context.getWorker()->getServer());
  auto server = context.getWorker()->getServer();
  // There's actually no point in logging these at all as wangle will not set
  // up tls if there's no config.
  if (!server->getSSLConfig() ||
      server->getSSLPolicy() != SSLPolicy::REQUIRED ||
      (server->isPlaintextAllowedOnLoopback() &&
       context.getPeerAddress()->isLoopbackAddress())) {
    THRIFT_CONNECTION_EVENT(non_tls.manual_policy).log(context);
  } else {
    THRIFT_CONNECTION_EVENT(non_tls).log(context);
  }
}

void logIfAlpnMismatch(
    const ConnectionLoggingContext& context,
    const folly::AsyncTransport* transport) {
  auto sock = transport->getUnderlyingTransport<folly::AsyncSSLSocket>();
  if (sock) {
    if (sock->getApplicationProtocol().empty() &&
        !sock->getClientAlpns().empty()) {
      THRIFT_CONNECTION_EVENT(alpn.mismatch.ssl).log(context);
    }
  } else if (auto fizz = transport->getUnderlyingTransport<AsyncFizzServer>()) {
    auto& state = fizz->getState();
    if (!state.alpn() && !state.handshakeLogging()->clientAlpns.empty()) {
      THRIFT_CONNECTION_EVENT(alpn.mismatch.fizz).log(context);
    }
  }
}
} // namespace

void logSetupConnectionEventsOnce(
    folly::once_flag& flag, const ConnectionLoggingContext& context) {
  static_cast<void>(folly::try_call_once(flag, [&]() noexcept {
    try {
      THRIFT_CONNECTION_EVENT(new_connection).log(context);
      if (auto transportType = context.getTransportType()) {
        switch (*transportType) {
          case Cpp2ConnContext::TransportType::HEADER:
            THRIFT_CONNECTION_EVENT(new_connection.header).log(context);
            break;
          case Cpp2ConnContext::TransportType::ROCKET:
            THRIFT_CONNECTION_EVENT(new_connection.rocket).log(context);
            break;
          case Cpp2ConnContext::TransportType::HTTP2:
            THRIFT_CONNECTION_EVENT(new_connection.http2).log(context);
            break;
          default:
            folly::assume_unreachable();
        }
      }
      if (auto transport = context.getTransport()) {
        const auto& protocol = context.getSecurityProtocol();
        if (protocol == "TLS" || protocol == "Fizz" || protocol == "stopTLS" ||
            protocol == "Fizz/KTLS") {
          if (!transport->getPeerCertificate()) {
            logTlsNoPeerCertEvent(context);
          }
          logIfAlpnMismatch(context, transport);
        } else {
          logNonTLSEvent(context);
        }
      }
    } catch (...) {
      LOG(ERROR)
          << "Exception thrown during Thrift server connection events logging: "
          << folly::exceptionStr(std::current_exception());
    }
    return true;
  }));
}

} // namespace thrift
} // namespace apache
