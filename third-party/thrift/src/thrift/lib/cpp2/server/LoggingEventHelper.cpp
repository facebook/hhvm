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
#include <thrift/lib/cpp2/server/peeking/PeekingManager.h>

using fizz::server::AsyncFizzServer;

namespace apache::thrift {

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

void maybeLogTlsPeerCertEvent(
    const ConnectionLoggingContext& context,
    const folly::AsyncTransportCertificate* cert) {
  DCHECK(context.getWorker() && context.getWorker()->getServer());
  auto ret = detail::isCertIPMismatch(context, cert);
  switch (ret) {
    case CertIPResult::SKIPPED_OTHER:
      THRIFT_CONNECTION_EVENT(tls.cert_ip_skipped_other).log(context);
      return;
    case CertIPResult::SKIPPED_TLS_TUNNEL:
      THRIFT_CONNECTION_EVENT(tls.cert_ip_skipped_tls_tunnel).log(context);
      return;
    case CertIPResult::SKIPPED_EXTENSION_NOT_PRESENT:
      THRIFT_CONNECTION_EVENT(tls.cert_ip_skipped_extension_not_present)
          .log(context);
      return;
    case CertIPResult::SKIPPED_LOCALHOST:
      THRIFT_CONNECTION_EVENT(tls.cert_ip_skipped_localhost).log(context);
      return;
    case CertIPResult::SKIPPED_LOCALHOST_IPONLY:
      THRIFT_CONNECTION_EVENT(tls.cert_ip_skipped_localhost_iponly)
          .log(context);
      return;
    case CertIPResult::MATCHED_ENFORCED:
      THRIFT_CONNECTION_EVENT(tls.cert_ip_match_enforced).log(context);
      return;
    case CertIPResult::MATCHED_UNENFORCED:
      THRIFT_CONNECTION_EVENT(tls.cert_ip_match_unenforced).log(context);
      return;
    case CertIPResult::MISMATCHED:
      THRIFT_CONNECTION_EVENT(tls.cert_ip_mismatch).log(context);
      return;
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
          /*
           * We have to deal with two cases here
           * 1. no peer cert received at all
           * 2. If a peer cert was received we can additionally log data from
           * said cert but only if it hasn't been dropped from
           * the context/transport, which may have happened to conserve memory
           */
          if (!context.peerCertReceived()) {
            logTlsNoPeerCertEvent(context);
          } else if (transport->getPeerCertificate() != nullptr) {
            maybeLogTlsPeerCertEvent(context, transport->getPeerCertificate());
          }
        } else {
          logNonTLSEvent(context);
        }
      }
    } catch (...) {
      LOG(ERROR)
          << "Exception thrown during Thrift server connection events logging: "
          << folly::exceptionStr(folly::current_exception());
    }
    return true;
  }));
}

} // namespace apache::thrift
