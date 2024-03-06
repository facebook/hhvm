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

namespace thrift {
namespace python {
namespace client {

using namespace apache::thrift;

RequestChannel_ptr createHeaderChannel(
    folly::AsyncTransport::UniquePtr sock,
    CLIENT_TYPE client,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    folly::Optional<std::string> host,
    folly::Optional<std::string> endpoint) {
  apache::thrift::HeaderClientChannel::Options options;
  if (client == THRIFT_HTTP_CLIENT_TYPE) {
    options.useAsHttpClient(*host, *endpoint);
  } else {
    options.setClientType(client);
  }
  options.setProtocolId(proto);
  return apache::thrift::HeaderClientChannel::newChannel(
      std::move(sock), std::move(options));
}

folly::Future<RequestChannel_ptr> createThriftChannelTCP(
    const std::string& host,
    uint16_t port,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    const std::string& endpoint) {
  auto eb = folly::getGlobalIOExecutor()->getEventBase();
  auto future = folly::via(eb, [=]() -> RequestChannel_ptr {
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

RequestChannel_ptr sync_createThriftChannelTCP(
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

folly::Future<RequestChannel_ptr> createThriftChannelUnix(
    const std::string& path,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto) {
  auto eb = folly::getGlobalIOExecutor()->getEventBase();
  auto future = folly::via(eb, [=]() -> RequestChannel_ptr {
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

RequestChannel_ptr sync_createThriftChannelUnix(
    const std::string& path,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto) {
  auto future = createThriftChannelUnix(path, connect_timeout, client_t, proto);
  return std::move(future.wait().value());
}

} // namespace client
} // namespace python
} // namespace thrift
