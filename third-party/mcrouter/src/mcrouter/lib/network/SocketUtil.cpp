/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/network/SocketUtil.h"

#include <type_traits>

#include <folly/Expected.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>

#include "mcrouter/lib/fbi/cpp/LogFailure.h"
#include "mcrouter/lib/network/AsyncTlsToPlaintextSocket.h"
#include "mcrouter/lib/network/ConnectionOptions.h"
#include "mcrouter/lib/network/McFizzClient.h"
#include "mcrouter/lib/network/Qos.h"
#include "mcrouter/lib/network/ThreadLocalSSLContextProvider.h"

namespace facebook {
namespace memcache {

namespace {

std::string getSessionKey(const ConnectionOptions& opts) {
  std::string service;
  if (opts.accessPoint->getServiceIdOverride().has_value()) {
    service = opts.accessPoint->getServiceIdOverride().value();
  } else {
    const auto& svcIdentity = opts.securityOpts.sslServiceIdentity;
    service = svcIdentity.empty() ? opts.accessPoint->toHostPortString()
                                  : svcIdentity;
  }
  return fmt::format(
      "{}:{}:{}",
      service,
      mc_protocol_to_string(opts.accessPoint->getProtocol()),
      securityMechToString(opts.accessPoint->getSecurityMech()));
}

void createTCPKeepAliveOptions(
    folly::SocketOptionMap& options,
    int cnt,
    int idle,
    int interval) {
  // 0 means KeepAlive is disabled.
  if (cnt != 0) {
#ifdef SO_KEEPALIVE
    folly::SocketOptionMap::key_type key;
    key.level = SOL_SOCKET;
    key.optname = SO_KEEPALIVE;
    options[key] = 1;

    key.level = IPPROTO_TCP;

#ifdef TCP_KEEPCNT
    key.optname = TCP_KEEPCNT;
    options[key] = cnt;
#endif // TCP_KEEPCNT

#ifdef TCP_KEEPIDLE
    key.optname = TCP_KEEPIDLE;
    options[key] = idle;
#endif // TCP_KEEPIDLE

#ifdef TCP_KEEPINTVL
    key.optname = TCP_KEEPINTVL;
    options[key] = interval;
#endif // TCP_KEEPINTVL

#endif // SO_KEEPALIVE
  }
}

const folly::SocketOptionKey getQoSOptionKey(sa_family_t addressFamily) {
  static const folly::SocketOptionKey kIpv4OptKey = {IPPROTO_IP, IP_TOS};
  static const folly::SocketOptionKey kIpv6OptKey = {IPPROTO_IPV6, IPV6_TCLASS};
  return (addressFamily == AF_INET) ? kIpv4OptKey : kIpv6OptKey;
}

uint64_t getClientQoS(uint64_t qosClassLvl, uint64_t qosPathLvl) {
  uint64_t qos = 0;
  if (!getQoS(qosClassLvl, qosPathLvl, qos)) {
    LOG_FAILURE(
        "AsyncMcClient",
        failure::Category::kSystemError,
        "Invalid QoS class/path value in AsyncMcClient");
  }
  return qos;
}

void createQoSClassOption(
    folly::SocketOptionMap& options,
    const sa_family_t addressFamily,
    uint64_t qosClass,
    uint64_t qosPath) {
  const auto& optkey = getQoSOptionKey(addressFamily);
  options[optkey] = getClientQoS(qosClass, qosPath);
}

} // namespace

template <bool CreateThriftFriendlySocket>
folly::Expected<
    folly::AsyncTransportWrapper::UniquePtr,
    folly::AsyncSocketException>
createSocketCommon(
    folly::EventBase& eventBase,
    const ConnectionOptions& connectionOptions) {
  using AsyncSocketT = std::conditional_t<
      CreateThriftFriendlySocket,
      folly::AsyncSocket,
      folly::AsyncSocket>;
  using AsyncSSLSocketT = std::conditional_t<
      CreateThriftFriendlySocket,
      apache::thrift::async::TAsyncSSLSocket,
      folly::AsyncSSLSocket>;
  using Ptr = folly::AsyncTransportWrapper::UniquePtr;

  Ptr socket;

  const auto mech = connectionOptions.accessPoint->getSecurityMech();
  if (mech == SecurityMech::NONE) {
    socket.reset(new AsyncSocketT(&eventBase));
    return socket;
  }

  // Creating a secure transport - make sure it isn't over a unix domain sock
  if (connectionOptions.accessPoint->isUnixDomainSocket()) {
    return folly::makeUnexpected(folly::AsyncSocketException(
        folly::AsyncSocketException::BAD_ARGS,
        "SSL protocol is not applicable for Unix Domain Sockets"));
  }
  const auto& securityOpts = connectionOptions.securityOpts;
  const auto& sessionKey = getSessionKey(connectionOptions);
  if (isAsyncSSLSocketMech(mech)) {
    // openssl based tls
    auto sslContext = getClientContext(eventBase, securityOpts, mech);
    if (!sslContext) {
      return folly::makeUnexpected(folly::AsyncSocketException(
          folly::AsyncSocketException::SSL_ERROR,
          "SSLContext provider returned nullptr, "
          "check SSL certificates"));
    }

    typename AsyncSSLSocketT::UniquePtr sslSocket(
        new AsyncSSLSocketT(sslContext, &eventBase));
    if (securityOpts.sessionCachingEnabled) {
      if (auto clientCtx =
              std::dynamic_pointer_cast<ClientSSLContext>(sslContext)) {
        sslSocket->setSessionKey(sessionKey);
        auto session = clientCtx->getCache().getSSLSession(sessionKey);
        if (session) {
          sslSocket->setRawSSLSession(std::move(session));
        }
      }
    }
    if (securityOpts.tfoEnabledForSsl) {
      sslSocket->enableTFO();
    }
    sslSocket->forceCacheAddrOnFailure(true);
    socket.reset(sslSocket.release());
    return socket;
  }

  // tls 13 fizz
  auto fizzContextAndVerifier = getFizzClientConfig(eventBase, securityOpts);
  if (!fizzContextAndVerifier.first) {
    return folly::makeUnexpected(folly::AsyncSocketException(
        folly::AsyncSocketException::SSL_ERROR,
        "Fizz context provider returned nullptr, "
        "check SSL certificates"));
  }
  auto fizzClient = new McFizzClient(
      &eventBase,
      std::move(fizzContextAndVerifier.first),
      std::move(fizzContextAndVerifier.second));
  fizzClient->setSessionKey(sessionKey);
  if (securityOpts.tfoEnabledForSsl) {
    if (auto underlyingSocket =
            fizzClient->getUnderlyingTransport<folly::AsyncSocket>()) {
      underlyingSocket->enableTFO();
    }
  }
  socket.reset(fizzClient);
  return socket;
}

folly::Expected<
    folly::AsyncTransportWrapper::UniquePtr,
    folly::AsyncSocketException>
createSocket(
    folly::EventBase& eventBase,
    const ConnectionOptions& connectionOptions) {
  folly::AsyncTransportWrapper::UniquePtr socket;

  return createSocketCommon<false>(eventBase, connectionOptions);
}

folly::Expected<
    folly::AsyncTransportWrapper::UniquePtr,
    folly::AsyncSocketException>
createAsyncSocket(
    folly::EventBase& eventBase,
    const ConnectionOptions& connectionOptions) {
  auto socket = createSocketCommon<true>(eventBase, connectionOptions);

  const auto mech = connectionOptions.accessPoint->getSecurityMech();
  if (mech == SecurityMech::NONE || mech == SecurityMech::TLS ||
      mech == SecurityMech::TLS13_FIZZ || socket.hasError()) {
    return socket;
  }
  DCHECK(mech == SecurityMech::TLS_TO_PLAINTEXT);
  return AsyncTlsToPlaintextSocket::create(std::move(socket).value());
}

folly::Expected<folly::SocketAddress, folly::AsyncSocketException>
getSocketAddress(const ConnectionOptions& connectionOptions) {
  try {
    folly::SocketAddress address;
    if (connectionOptions.accessPoint->isUnixDomainSocket()) {
      address.setFromPath(connectionOptions.accessPoint->getHost());
    } else {
      address = folly::SocketAddress(
          connectionOptions.accessPoint->getHost(),
          connectionOptions.accessPoint->getPort(),
          /* allowNameLookup */ true);
    }
    return address;
  } catch (const std::system_error& e) {
    return folly::makeUnexpected(folly::AsyncSocketException(
        folly::AsyncSocketException::NOT_OPEN,
        folly::sformat(
            "AsyncMcClient",
            failure::Category::kBadEnvironment,
            "{}",
            e.what())));
  }
}

folly::SocketOptionMap createSocketOptions(
    const folly::SocketAddress& address,
    const ConnectionOptions& connectionOptions) {
  folly::SocketOptionMap options;

  if (connectionOptions.accessPoint->isUnixDomainSocket()) {
    // TCP socket options are not supported by a Unix domain socket transport.
    return options;
  }

  createTCPKeepAliveOptions(
      options,
      connectionOptions.tcpKeepAliveCount,
      connectionOptions.tcpKeepAliveIdle,
      connectionOptions.tcpKeepAliveInterval);
  if (connectionOptions.enableQoS) {
    createQoSClassOption(
        options,
        address.getFamily(),
        connectionOptions.qosClass,
        connectionOptions.qosPath);
  }

  return options;
}

void checkWhetherQoSIsApplied(
    int socketFd,
    const folly::SocketAddress& address,
    const ConnectionOptions& connectionOptions,
    folly::StringPiece transportName) {
  const auto& optkey = getQoSOptionKey(address.getFamily());

  const uint64_t expectedValue =
      getClientQoS(connectionOptions.qosClass, connectionOptions.qosPath);

  uint64_t val = 0;
  socklen_t len = sizeof(expectedValue);
  int rv = getsockopt(socketFd, optkey.level, optkey.optname, &val, &len);
  // Zero out last 2 bits as they are not used for the QOS value
  constexpr uint64_t kMaskTwoLeastSignificantBits = 0xFFFFFFFc;
  val = val & kMaskTwoLeastSignificantBits;
  if (rv != 0 || val != expectedValue) {
    LOG_FAILURE(
        transportName,
        failure::Category::kSystemError,
        "Failed to apply QoS! "
        "Return Value: {} (expected: {}). "
        "QoS Value: {} (expected: {}).",
        rv,
        0,
        val,
        expectedValue);
  }
}

} // namespace memcache
} // namespace facebook
