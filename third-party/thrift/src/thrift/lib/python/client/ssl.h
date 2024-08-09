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

#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/SSLContext.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace thrift {
namespace python {
namespace client {

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

  folly::Future<apache::thrift::RequestChannel::Ptr> connect();
  void setSupportedApplicationProtocols(
      const std::vector<std::string>& protocols);
  void connectSuccess() noexcept override;
  void connectErr(const folly::AsyncSocketException& ex) noexcept override;

 private:
  folly::Promise<apache::thrift::RequestChannel::Ptr> promise_;
  folly::AsyncSSLSocket::UniquePtr socket_;
  std::string host_;
  const uint16_t port_;
  const uint32_t connect_timeout_;
  const uint32_t ssl_timeout_;
  CLIENT_TYPE client_t_;
  apache::thrift::protocol::PROTOCOL_TYPES proto_;
  std::string endpoint_;
};

apache::thrift::RequestChannel::Ptr createHeaderChannel(
    folly::AsyncTransport::UniquePtr sock,
    CLIENT_TYPE client,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    folly::Optional<std::string> host = folly::none,
    folly::Optional<std::string> endpoint = folly::none);

} // namespace client
} // namespace python
} // namespace thrift
