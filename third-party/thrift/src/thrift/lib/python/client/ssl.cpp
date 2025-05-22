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

#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>

namespace apache::thrift::python::client {

apache::thrift::RequestChannel::Ptr createHeaderChannel(
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
    : socket_{folly::AsyncSSLSocket::newSocket(ctx, evb)},
      host_(host),
      port_(port),
      connect_timeout_(connect_timeout),
      ssl_timeout_(ssl_timeout),
      client_t_(client_t),
      proto_(proto),
      endpoint_(endpoint) {}

folly::Future<apache::thrift::RequestChannel::Ptr> ConnectHandler::connect() {
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
  promise_.setValue([this]() mutable -> apache::thrift::RequestChannel::Ptr {
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

} // namespace apache::thrift::python::client
