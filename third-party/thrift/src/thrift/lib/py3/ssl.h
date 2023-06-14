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

#pragma once

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/SSLContext.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>
#include <thrift/lib/py3/client.h>

namespace thrift {
namespace py3 {

using apache::thrift::async::TAsyncSSLSocket;

class ConnectHandler : public folly::AsyncSocket::ConnectCallback,
                       public folly::DelayedDestruction {
 protected:
  ~ConnectHandler() = default;

 public:
  using UniquePtr =
      std::unique_ptr<ConnectHandler, folly::DelayedDestruction::Destructor>;
  explicit ConnectHandler(
      const std::shared_ptr<folly::SSLContext>& ctx,
      folly::EventBase* evb,
      std::string&& host,
      const uint16_t port,
      const uint32_t connect_timeout,
      const uint32_t ssl_timeout,
      CLIENT_TYPE client_t,
      apache::thrift::protocol::PROTOCOL_TYPES proto,
      std::string&& endpoint)
      : socket_{new TAsyncSSLSocket(ctx, evb)},
        host_(std::move(host)),
        port_(port),
        connect_timeout_(connect_timeout),
        ssl_timeout_(ssl_timeout),
        client_t_(client_t),
        proto_(proto),
        endpoint_(std::move(endpoint)) {}

  folly::Future<RequestChannel_ptr> connect() {
    folly::DelayedDestruction::DestructorGuard dg(this);
    socket_->connect(
        this,
        folly::SocketAddress(host_, port_),
        std::chrono::milliseconds(connect_timeout_),
        std::chrono::milliseconds(connect_timeout_ + ssl_timeout_));
    return promise_.getFuture();
  }

  void setSupportedApplicationProtocols(
      const std::vector<std::string>& protocols) {
    socket_->setSupportedApplicationProtocols(protocols);
  }

  void connectSuccess() noexcept override {
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

  void connectErr(const folly::AsyncSocketException& ex) noexcept override {
    using apache::thrift::transport::TTransportException;
    UniquePtr p(this);
    promise_.setException(TTransportException(ex));
  }

 private:
  folly::Promise<RequestChannel_ptr> promise_;
  TAsyncSSLSocket::UniquePtr socket_;
  std::string host_;
  const uint16_t port_;
  const uint32_t connect_timeout_;
  const uint32_t ssl_timeout_;
  CLIENT_TYPE client_t_;
  apache::thrift::protocol::PROTOCOL_TYPES proto_;
  std::string endpoint_;
};

/**
 * Create a thrift channel by connecting to a host:port over TCP then SSL.
 */
inline folly::Future<RequestChannel_ptr> createThriftChannelTCP(
    const std::shared_ptr<folly::SSLContext>& ctx,
    std::string&& host,
    const uint16_t port,
    const uint32_t connect_timeout,
    const uint32_t ssl_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    std::string&& endpoint) {
  auto eb = folly::getEventBase();
  return folly::via(
      eb, [=, host{std::move(host)}, endpoint{std::move(endpoint)}]() mutable {
        ConnectHandler::UniquePtr handler{new ConnectHandler(
            ctx,
            eb,
            std::move(host),
            port,
            connect_timeout,
            ssl_timeout,
            client_t,
            proto,
            std::move(endpoint))};

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
} // namespace py3
} // namespace thrift
