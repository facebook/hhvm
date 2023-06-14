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

#include <thrift/lib/python/client/ssl.h>

#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/python/client/RequestChannel.h>

namespace thrift {
namespace python {
namespace client {

ConnectHandler::ConnectHandler(
    const std::shared_ptr<folly::SSLContext>& ctx,
    folly::EventBase* evb,
    const std::string& host,
    const uint16_t port,
    const uint32_t connect_timeout,
    const uint32_t ssl_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    const std::string& endpoint)
    : socket_{new apache::thrift::async::TAsyncSSLSocket(ctx, evb)},
      host_(host),
      port_(port),
      connect_timeout_(connect_timeout),
      ssl_timeout_(ssl_timeout),
      client_t_(client_t),
      proto_(proto),
      endpoint_(endpoint) {}

folly::Future<RequestChannel_ptr> ConnectHandler::connect() {
  folly::DelayedDestruction::DestructorGuard dg(this);
  socket_->connect(
      this,
      folly::SocketAddress(host_, port_),
      std::chrono::milliseconds(connect_timeout_),
      std::chrono::milliseconds(connect_timeout_ + ssl_timeout_));
  return promise_.getFuture();
}

void ConnectHandler::setSupportedApplicationProtocols(
    const std::vector<std::string>& protocols) {
  socket_->setSupportedApplicationProtocols(protocols);
}

void ConnectHandler::connectSuccess() noexcept {
  UniquePtr p(this);
  promise_.setValue([this]() mutable -> RequestChannel_ptr {
    if (client_t_ == CLIENT_TYPE::THRIFT_ROCKET_CLIENT_TYPE) {
      auto chan =
          apache::thrift::RocketClientChannel::newChannel(std::move(socket_));
      chan->setProtocolId(proto_);
      return chan;
    }
    return createHeaderChannel(
        std::move(socket_), client_t_, proto_, host_, endpoint_);
  }());
}

void ConnectHandler::connectErr(
    const folly::AsyncSocketException& ex) noexcept {
  using apache::thrift::transport::TTransportException;
  UniquePtr p(this);
  promise_.setException(TTransportException(ex));
}

/**
 * Create a thrift channel by connecting to a host:port over TCP then SSL.
 */
folly::Future<RequestChannel_ptr> createThriftChannelTCP(
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

RequestChannel_ptr sync_createThriftChannelTCP(
    const std::shared_ptr<folly::SSLContext>& ctx,
    const std::string& host,
    const uint16_t port,
    const uint32_t connect_timeout,
    const uint32_t ssl_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    const std::string& endpoint) {
  auto future = createThriftChannelTCP(
      ctx, host, port, connect_timeout, ssl_timeout, client_t, proto, endpoint);
  return std::move(future.wait().value());
}

} // namespace client
} // namespace python
} // namespace thrift
