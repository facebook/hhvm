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
#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersClientExtension.h>

namespace apache::thrift::stress {

class FizzStopTLSConnector
    : public fizz::client::AsyncFizzClient::HandshakeCallback,
      public fizz::AsyncFizzBase::EndOfTLSCallback {
 public:
  ~FizzStopTLSConnector() {
    // Ensure client is destroyed on the correct thread if it still exists
    if (client_ && connectEvb_) {
      auto clientToDestroy = std::move(client_);
      if (connectEvb_->isInEventBaseThread()) {
        // Already on correct thread, client will be destroyed when this goes out of scope
      } else {
        // Schedule destruction on the correct thread
        connectEvb_->runInEventBaseThread(
            [clientToDestroy = std::move(clientToDestroy)]() mutable {
              // The client will be destroyed when this lambda goes out of scope
              // This ensures it happens on the connectEvb_ thread
            });
      }
    }
  }
  folly::SemiFuture<folly::AsyncSocket::UniquePtr> connect(
      const folly::SocketAddress& address,
      folly::EventBase* connectEvb,
      folly::EventBase* targetEvb) {
    connectEvb_ = connectEvb;
    targetEvb_ = targetEvb;

    auto initConnection = [this, address]() {
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
    };

    // Check if we're already on the connectEvb thread to avoid deadlock
    if (connectEvb_->isInEventBaseThread()) {
      initConnection();
    } else {
      connectEvb_->runInEventBaseThreadAndWait(initConnection);
    }

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
    // Clean up the client on the correct thread before setting exception
    auto clientToDestroy = std::move(client_);
    connectEvb_->runInEventBaseThread(
        [clientToDestroy = std::move(clientToDestroy)]() mutable {
          // The client will be destroyed when this lambda goes out of scope
          // This ensures it happens on the connectEvb_ thread
        });

    promise_.setException(ex);
  }

  void endOfTLS(
      fizz::AsyncFizzBase* transport, std::unique_ptr<folly::IOBuf>) override {
    auto sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
    DCHECK(sock);

    // Transfer raw FD and ZeroCopyBufId to new socket bound to `targetEvb_`
    auto fd = sock->detachNetworkSocket();
    auto zcId = sock->getZeroCopyBufId();

    // Store the FD and ZeroCopyBufId for later socket creation
    // We can't create the AsyncSocket here due to threading constraints
    fd_ = fd;
    zeroCopyBufId_ = zcId;

    // Clean up the Fizz client on the correct thread to avoid threading issues
    // We need to move the client_ to avoid destructor being called on wrong
    // thread
    auto clientToDestroy = std::move(client_);
    connectEvb_->runInEventBaseThread(
        [clientToDestroy = std::move(clientToDestroy)]() mutable {
          // The client will be destroyed when this lambda goes out of scope
          // This ensures it happens on the connectEvb_ thread
        });

    // Signal that StopTLS is complete
    promise_.setValue(nullptr); // We'll create the actual socket later
  }

  // Helper method to create the socket once we're off the connection thread
  // This method returns the raw FD and ZeroCopyBufId for socket creation
  std::pair<folly::NetworkSocket, uint32_t> getSocketParams() {
    return {fd_, zeroCopyBufId_};
  }

 private:
  folly::EventBase* connectEvb_{};
  folly::EventBase* targetEvb_{};
  folly::Promise<folly::AsyncSocket::UniquePtr> promise_;
  fizz::client::AsyncFizzClient::UniquePtr client_;
  folly::NetworkSocket fd_{};
  uint32_t zeroCopyBufId_{0};
};

} // namespace apache::thrift::stress
