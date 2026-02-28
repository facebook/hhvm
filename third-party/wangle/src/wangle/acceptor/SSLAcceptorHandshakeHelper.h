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

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <wangle/acceptor/AcceptorHandshakeManager.h>
#include <wangle/acceptor/ManagedConnection.h>
#include <wangle/acceptor/PeekingAcceptorHandshakeHelper.h>
#include <wangle/acceptor/TransportInfo.h>
#include <chrono>

namespace wangle {

class SSLAcceptorHandshakeHelper : public AcceptorHandshakeHelper,
                                   public folly::AsyncSSLSocket::HandshakeCB {
 public:
  SSLAcceptorHandshakeHelper(
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      TransportInfo& tinfo)
      : clientAddr_(clientAddr), acceptTime_(acceptTime), tinfo_(tinfo) {}

  void start(
      folly::AsyncSSLSocket::UniquePtr sock,
      AcceptorHandshakeHelper::Callback* callback) noexcept override;

  void dropConnection(SSLErrorEnum reason = SSLErrorEnum::NO_ERROR) override {
    sslError_ = reason;
    if (socket_) {
      socket_->closeNow();
    }
  }

  static void fillSSLTransportInfoFields(
      folly::AsyncSSLSocket* sock,
      TransportInfo& tinfo);

 protected:
  // AsyncSSLSocket::HandshakeCallback API
  void handshakeSuc(folly::AsyncSSLSocket* sock) noexcept override;
  void handshakeErr(
      folly::AsyncSSLSocket* sock,
      const folly::AsyncSocketException& ex) noexcept override;

  folly::AsyncSSLSocket::UniquePtr socket_;
  AcceptorHandshakeHelper::Callback* callback_;
  const folly::SocketAddress& clientAddr_;
  std::chrono::steady_clock::time_point acceptTime_;
  TransportInfo& tinfo_;
  SSLErrorEnum sslError_{SSLErrorEnum::NO_ERROR};
};

class DefaultToSSLPeekingCallback
    : public PeekingAcceptorHandshakeHelper::PeekCallback {
 public:
  DefaultToSSLPeekingCallback()
      : PeekingAcceptorHandshakeHelper::PeekCallback(0) {}

  AcceptorHandshakeHelper::UniquePtr getHelper(
      const std::vector<uint8_t>& /* bytes */,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      TransportInfo& tinfo) override {
    return AcceptorHandshakeHelper::UniquePtr(
        new SSLAcceptorHandshakeHelper(clientAddr, acceptTime, tinfo));
  }
};

} // namespace wangle
