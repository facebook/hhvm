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

#include <thrift/lib/python/client/RequestChannel.h>

#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/HTTPClientChannel.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/python/client/ssl.h>

namespace thrift {
namespace python {
namespace client {

using namespace apache::thrift;

folly::Future<RequestChannel::Ptr>
DefaultChannelFactory::createThriftChannelTCP(
    const std::string& host,
    uint16_t port,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    const std::string& endpoint) {
  auto eb = folly::getGlobalIOExecutor()->getEventBase();
  auto future = folly::via(eb, [=]() -> RequestChannel::Ptr {
    auto socket =
        folly::AsyncSocket::newSocket(eb, host, port, connect_timeout);
    if (client_t == THRIFT_HEADER_CLIENT_TYPE ||
        client_t == THRIFT_HTTP_CLIENT_TYPE) {
      return createHeaderChannel(
          std::move(socket), client_t, proto, host, endpoint);
    } else if (client_t == THRIFT_ROCKET_CLIENT_TYPE) {
      auto chan = RocketClientChannel::newChannel(std::move(socket));
      chan->setProtocolId(proto);
      return chan;
    } else if (client_t == THRIFT_HTTP2_CLIENT_TYPE) {
      auto chan = HTTPClientChannel::newHTTP2Channel(std::move(socket));
      chan->setHTTPHost(host);
      chan->setHTTPUrl(endpoint);
      chan->setProtocolId(proto);
      return chan;
    } else {
      throw std::runtime_error("Unsupported client type");
    }
  });
  return future;
}

RequestChannel::Ptr ChannelFactory::sync_createThriftChannelTCP(
    const std::string& host,
    uint16_t port,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    const std::string& endpoint) {
  auto future = createThriftChannelTCP(
      host, port, connect_timeout, client_t, proto, endpoint);
  return std::move(future.wait().value());
}

folly::Future<RequestChannel::Ptr>
DefaultChannelFactory::createThriftChannelUnix(
    const std::string& path,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto) {
  auto eb = folly::getGlobalIOExecutor()->getEventBase();
  auto future = folly::via(eb, [=]() -> RequestChannel::Ptr {
    auto socket = folly::AsyncSocket::newSocket(
        eb, folly::SocketAddress::makeFromPath(path), connect_timeout);
    if (client_t == THRIFT_HEADER_CLIENT_TYPE) {
      return createHeaderChannel(std::move(socket), client_t, proto);
    } else if (client_t == THRIFT_ROCKET_CLIENT_TYPE) {
      auto chan = RocketClientChannel::newChannel(std::move(socket));
      chan->setProtocolId(proto);
      return chan;
    } else {
      throw std::runtime_error("Unsupported client type");
    }
  });
  return future;
}

RequestChannel::Ptr ChannelFactory::sync_createThriftChannelUnix(
    const std::string& path,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto) {
  auto future = createThriftChannelUnix(path, connect_timeout, client_t, proto);
  return std::move(future.wait().value());
}

folly::Future<apache::thrift::RequestChannel::Ptr>
DefaultChannelFactory::createThriftChannelSSL(
    const std::shared_ptr<folly::SSLContext>& ctx,
    const std::string& host,
    const uint16_t port,
    const uint32_t connect_timeout,
    const uint32_t ssl_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    const std::string& endpoint) {
  auto eb = folly::getGlobalIOExecutor()->getEventBase();
  return folly::via(
      eb,
      [=,
       ctx = ctx,
       host = host,
       port = port,
       connect_timeout = connect_timeout,
       ssl_timeout = ssl_timeout,
       endpoint = endpoint]() mutable {
        ConnectHandler::UniquePtr handler{new ConnectHandler(
            ctx,
            eb,
            host,
            port,
            connect_timeout,
            ssl_timeout,
            client_t,
            proto,
            endpoint)};

        if (client_t == CLIENT_TYPE::THRIFT_ROCKET_CLIENT_TYPE) {
          handler->setSupportedApplicationProtocols({"rs"});
        } else if (client_t == CLIENT_TYPE::THRIFT_HEADER_CLIENT_TYPE) {
          handler->setSupportedApplicationProtocols({"thrift"});
        }
        auto future = handler->connect();
        handler.release();
        return future;
      });
}

apache::thrift::RequestChannel::Ptr ChannelFactory::sync_createThriftChannelSSL(
    const std::shared_ptr<folly::SSLContext>& ctx,
    const std::string& host,
    const uint16_t port,
    const uint32_t connect_timeout,
    const uint32_t ssl_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    const std::string& endpoint) {
  auto future = createThriftChannelSSL(
      ctx, host, port, connect_timeout, ssl_timeout, client_t, proto, endpoint);
  return std::move(future.wait().value());
}

} // namespace client
} // namespace python
} // namespace thrift
