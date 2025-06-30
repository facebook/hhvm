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

#include <fizz/client/AsyncFizzClient.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersClientExtension.h>

namespace apache::thrift::stress {

class FizzStopTLSConnector
    : public fizz::client::AsyncFizzClient::HandshakeCallback,
      public fizz::AsyncFizzBase::EndOfTLSCallback {
 public:
  folly::SemiFuture<folly::AsyncSocket::UniquePtr> connect(
      const folly::SocketAddress& address,
      folly::EventBase* connectEvb,
      folly::EventBase* targetEvb) {
    connectEvb_ = connectEvb;
    targetEvb_ = targetEvb;

    auto sock = folly::AsyncSocket::newSocket(connectEvb_, address);
    auto ctx = std::make_shared<fizz::client::FizzClientContext>();
    ctx->setSupportedAlpns({"rs"});

    auto thriftParams =
        std::make_shared<apache::thrift::ThriftParametersContext>();
    thriftParams->setUseStopTLS(true);
    auto extension =
        std::make_shared<apache::thrift::ThriftParametersClientExtension>(
            thriftParams);

    client_.reset(new fizz::client::AsyncFizzClient(
        std::move(sock), std::move(ctx), std::move(extension)));

    client_->connect(
        this,
        nullptr,
        folly::none,
        folly::none,
        folly::Optional<std::vector<fizz::ech::ParsedECHConfig>>(folly::none),
        std::chrono::milliseconds(3000));

    return promise_.getSemiFuture();
  }

  // Handshake success: now wait for StopTLS to complete
  void fizzHandshakeSuccess(
      fizz::client::AsyncFizzClient* client) noexcept override {
    client->setEndOfTLSCallback(this);
  }

  void fizzHandshakeError(
      fizz::client::AsyncFizzClient* /*unused*/,
      folly::exception_wrapper ex) noexcept override {
    promise_.setException(ex);
  }

  void endOfTLS(
      fizz::AsyncFizzBase* transport, std::unique_ptr<folly::IOBuf>) override {
    auto sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
    DCHECK(sock);

    // Transfer raw FD and ZeroCopyBufId to new socket bound to `targetEvb_`
    auto fd = sock->detachNetworkSocket();
    auto zcId = sock->getZeroCopyBufId();
    auto newSock = folly::AsyncSocket::UniquePtr(
        new folly::AsyncSocket(targetEvb_, fd, zcId));

    promise_.setValue(std::move(newSock));
  }

 private:
  folly::EventBase* connectEvb_{};
  folly::EventBase* targetEvb_{};
  folly::Promise<folly::AsyncSocket::UniquePtr> promise_;
  fizz::client::AsyncFizzClient::UniquePtr client_;
};

} // namespace apache::thrift::stress
