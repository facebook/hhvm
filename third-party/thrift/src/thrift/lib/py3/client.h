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
#include <cstdint>
#include <functional>

#include <folly/Range.h>
#include <folly/SocketAddress.h>
#include <folly/Try.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/py3/client_wrapper.h>

namespace thrift {
namespace py3 {
typedef std::unique_ptr<
    apache::thrift::RequestChannel,
    folly::DelayedDestruction::Destructor>
    RequestChannel_ptr;

typedef std::unique_ptr<
    apache::thrift::HeaderClientChannel,
    folly::DelayedDestruction::Destructor>
    HeaderChannel_ptr;

/*
 * T is the cpp2 async client class
 * U is the py3 clientwraper class
 */
template <class T, class U>
std::unique_ptr<ClientWrapper> makeClientWrapper(RequestChannel_ptr&& channel) {
  std::shared_ptr<apache::thrift::RequestChannel> channel_ = std::move(channel);
  auto client = std::make_unique<T>(channel_);
  return std::make_unique<U>(std::move(client), std::move(channel_));
}

void destroyInEventBaseThread(RequestChannel_ptr&& ptr) {
  auto eb = ptr->getEventBase();
  eb->runInEventBaseThread([ptr = std::move(ptr)] {});
}

RequestChannel_ptr createHeaderChannel(
    folly::AsyncTransport::UniquePtr sock,
    CLIENT_TYPE client,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    folly::Optional<std::string> host = folly::none,
    folly::Optional<std::string> endpoint = folly::none) {
  apache::thrift::HeaderClientChannel::Options options;
  if (client == THRIFT_HTTP_CLIENT_TYPE) {
    options.useAsHttpClient(*host, *endpoint);
  } else {
    options.setClientType(client);
  }
  options.setProtocolId(proto);
  if (client == THRIFT_FRAMED_DEPRECATED) {
    options.setProtocolId(
        apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL);
  } else if (client == THRIFT_FRAMED_COMPACT) {
    options.setProtocolId(
        apache::thrift::protocol::PROTOCOL_TYPES::T_COMPACT_PROTOCOL);
  }
  return apache::thrift::HeaderClientChannel::newChannel(
      std::move(sock), std::move(options));
}

struct FutureConnectCallback : folly::AsyncSocket::ConnectCallback {
  explicit FutureConnectCallback(folly::AsyncSocket::UniquePtr s)
      : socket{std::move(s)} {}

  void connectSuccess() noexcept override {
    auto deleteMe = std::unique_ptr<FutureConnectCallback>{this};
    promise.setValue(std::move(socket));
  }

  void connectErr(const folly::AsyncSocketException& ex) noexcept override {
    using apache::thrift::transport::TTransportException;
    auto deleteMe = std::unique_ptr<FutureConnectCallback>{this};
    promise.setException(TTransportException{ex});
  }

  folly::AsyncSocket::UniquePtr socket;
  folly::Promise<folly::AsyncSocket::UniquePtr> promise;
};

/**
 * Asynchronously connect to `address` with a new AsyncSocket. The Future will
 * be completed on the given EventBase.
 */
folly::Future<folly::AsyncSocket::UniquePtr> asyncSocketConnect(
    folly::EventBase* eb,
    const folly::SocketAddress& address,
    uint32_t connect_timeout) {
  auto* callback = new FutureConnectCallback{folly::AsyncSocket::newSocket(eb)};
  auto future = callback->promise.getFuture();
  callback->socket->connect(callback, address, connect_timeout);
  return future;
}

/**
 * Create a thrift channel by connecting to a host:port over TCP.
 */
folly::Future<RequestChannel_ptr> createThriftChannelTCP(
    std::string&& host,
    uint16_t port,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    std::string&& endpoint) {
  auto eb = folly::getEventBase();
  return folly::via(
      eb,
      [=,
       host{std::move(host)},
       endpoint{std::move(endpoint)}]() -> RequestChannel_ptr {
        auto socket =
            folly::AsyncSocket::newSocket(eb, host, port, connect_timeout);
        if (client_t == THRIFT_ROCKET_CLIENT_TYPE) {
          auto chan = apache::thrift::RocketClientChannel::newChannel(
              std::move(socket));
          chan->setProtocolId(proto);
          return chan;
        }
        return createHeaderChannel(
            std::move(socket), client_t, proto, host, endpoint);
      });
}

/**
 * Create a thrift channel by connecting to a Unix domain socket.
 */
folly::Future<RequestChannel_ptr> createThriftChannelUnix(
    std::string&& path,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto) {
  auto eb = folly::getEventBase();
  return folly::via(
             eb,
             [=, path{std::move(path)}]() {
               return asyncSocketConnect(
                   eb,
                   folly::SocketAddress::makeFromPath(path),
                   connect_timeout);
             })
      .thenValue(
          [=](folly::AsyncSocket::UniquePtr socket) -> RequestChannel_ptr {
            if (client_t == THRIFT_ROCKET_CLIENT_TYPE) {
              auto chan = apache::thrift::RocketClientChannel::newChannel(
                  std::move(socket));
              chan->setProtocolId(proto);
              return chan;
            }
            return createHeaderChannel(std::move(socket), client_t, proto);
          });
}

} // namespace py3
} // namespace thrift
