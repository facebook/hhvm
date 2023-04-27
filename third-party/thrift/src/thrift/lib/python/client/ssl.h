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
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace thrift {
namespace python {
namespace client {

using RequestChannel_ptr = std::unique_ptr<
    apache::thrift::RequestChannel,
    folly::DelayedDestruction::Destructor>;

class ConnectHandler : public folly::AsyncSocket::ConnectCallback,
                       public folly::DelayedDestruction {
 protected:
  ~ConnectHandler() override = default;

 public:
  using UniquePtr =
      std::unique_ptr<ConnectHandler, folly::DelayedDestruction::Destructor>;

  explicit ConnectHandler(
      const std::shared_ptr<folly::SSLContext>& ctx,
      folly::EventBase* evb,
      const std::string& host,
      const uint16_t port,
      const uint32_t connect_timeout,
      const uint32_t ssl_timeout,
      CLIENT_TYPE client_t,
      apache::thrift::protocol::PROTOCOL_TYPES proto,
      const std::string& endpoint);

  folly::Future<RequestChannel_ptr> connect();
  void setSupportedApplicationProtocols(
      const std::vector<std::string>& protocols);
  void connectSuccess() noexcept override;
  void connectErr(const folly::AsyncSocketException& ex) noexcept override;

 private:
  folly::Promise<RequestChannel_ptr> promise_;
  apache::thrift::async::TAsyncSSLSocket::UniquePtr socket_;
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
folly::Future<RequestChannel_ptr> createThriftChannelTCP(
    const std::shared_ptr<folly::SSLContext>& ctx,
    const std::string& host,
    const uint16_t port,
    const uint32_t connect_timeout,
    const uint32_t ssl_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    const std::string& endpoint);

RequestChannel_ptr sync_createThriftChannelTCP(
    const std::shared_ptr<folly::SSLContext>& ctx,
    const std::string& host,
    const uint16_t port,
    const uint32_t connect_timeout,
    const uint32_t ssl_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    const std::string& endpoint);

} // namespace client
} // namespace python
} // namespace thrift
