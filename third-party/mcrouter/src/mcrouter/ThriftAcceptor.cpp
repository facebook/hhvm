/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <string>
#include <utility>

#include <fmt/core.h>

#include <folly/GLog.h>
#include <folly/io/async/AsyncSSLSocket.h>

#include <mcrouter/ThriftAcceptor.h>
#include <mcrouter/lib/network/McSSLUtil.h>
#include <mcrouter/lib/network/SecurityOptions.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <wangle/acceptor/SecureTransportType.h>

namespace folly {
class SocketAddress;
} // namespace folly

namespace facebook {
namespace memcache {

std::shared_ptr<wangle::Acceptor> ThriftAcceptorFactory::newAcceptor(
    folly::EventBase* evb) {
  class ThriftAcceptor : public apache::thrift::Cpp2Worker {
    using ThriftAclCheckerFunc = ThriftAcceptorFactory::ThriftAclCheckerFunc;

   protected:
    struct Tag {};

   public:
    ThriftAcceptor(apache::thrift::ThriftServer& server)
        : apache::thrift::Cpp2Worker(&server, {}) {}

    static std::shared_ptr<ThriftAcceptor> create(
        apache::thrift::ThriftServer& server,
        folly::EventBase* evb) {
      auto self = std::make_shared<ThriftAcceptor>(server);
      self->construct(&server, evb, nullptr);
      return self;
    }

    void onNewConnection(
        folly::AsyncTransportWrapper::UniquePtr socket,
        const folly::SocketAddress* address,
        const std::string& nextProtocolName,
        wangle::SecureTransportType secureTransportType,
        const wangle::TransportInfo& transportInfo) final {
      if (!nextProtocolName.empty() ||
          secureTransportType != wangle::SecureTransportType::NONE) {
        // We can only handle plaintext connections.
        FB_LOG_EVERY_MS(ERROR, 5000) << fmt::format(
            "Dropping new connection with SecureTransportType '{}' and"
            " nextProtocolName '{}'",
            wangle::getSecureTransportName(secureTransportType),
            nextProtocolName);
        return;
      }

      // Ensure socket has same options that would be applied in AsyncMcServer
      auto* asyncSocket =
          socket->getUnderlyingTransport<folly::AsyncSocketTransport>();
      asyncSocket->setNoDelay(true);
      asyncSocket->setSendTimeout(0);
      apache::thrift::Cpp2Worker::onNewConnection(
          std::move(socket), address, "", secureTransportType, transportInfo);
    }
  };
  return ThriftAcceptor::create(server_, evb);
}

} // namespace memcache
} // namespace facebook
