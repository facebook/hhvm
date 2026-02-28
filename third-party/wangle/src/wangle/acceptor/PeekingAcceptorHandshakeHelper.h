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

#include <wangle/acceptor/AcceptorHandshakeManager.h>
#include <wangle/acceptor/SocketPeeker.h>

namespace wangle {

/**
 * A hanshake helper which helpes switching between
 * SSL and other protocols, so that we can run both
 * SSL and other protocols over the same port at the
 * same time.
 * The mechanism used by this is to peek the first numBytes
 * bytes of the socket and send it to the peek helper
 * to decide which protocol it is.
 */
class PeekingAcceptorHandshakeHelper : public AcceptorHandshakeHelper,
                                       public SocketPeeker::Callback {
 public:
  class PeekCallback {
   public:
    explicit PeekCallback(size_t bytesRequired)
        : bytesRequired_(bytesRequired) {}
    virtual ~PeekCallback() = default;

    size_t getBytesRequired() const {
      return bytesRequired_;
    }

    virtual AcceptorHandshakeHelper::UniquePtr getHelper(
        const std::vector<uint8_t>& peekedBytes,
        const folly::SocketAddress& clientAddr,
        std::chrono::steady_clock::time_point acceptTime,
        TransportInfo& tinfo) = 0;

   private:
    const size_t bytesRequired_;
  };

  PeekingAcceptorHandshakeHelper(
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      TransportInfo& tinfo,
      const std::vector<PeekCallback*>& peekCallbacks,
      size_t numBytes)
      : clientAddr_(clientAddr),
        acceptTime_(acceptTime),
        tinfo_(tinfo),
        peekCallbacks_(peekCallbacks),
        numBytes_(numBytes) {}

  // From AcceptorHandshakeHelper
  void start(
      folly::AsyncSSLSocket::UniquePtr sock,
      AcceptorHandshakeHelper::Callback* callback) noexcept override {
    socket_ = std::move(sock);
    callback_ = callback;
    CHECK_EQ(
        socket_->getSSLState(),
        folly::AsyncSSLSocket::SSLStateEnum::STATE_UNENCRYPTED);
    peeker_.reset(new SocketPeeker(*socket_, this, numBytes_));
    peeker_->start();
  }

  void dropConnection(SSLErrorEnum reason = SSLErrorEnum::NO_ERROR) override {
    CHECK_NE(socket_.get() == nullptr, helper_.get() == nullptr);
    if (socket_) {
      socket_->closeNow();
    } else if (helper_) {
      helper_->dropConnection(reason);
    }
  }

  void peekSuccess(std::vector<uint8_t> peekBytes) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    peeker_ = nullptr;

    for (auto& peekCallback : peekCallbacks_) {
      helper_ =
          peekCallback->getHelper(peekBytes, clientAddr_, acceptTime_, tinfo_);
      if (helper_) {
        break;
      }
    }

    if (!helper_) {
      // could not get a helper, report error.
      auto type =
          folly::AsyncSocketException::AsyncSocketExceptionType::CORRUPTED_DATA;
      return peekError(
          folly::AsyncSocketException(type, "Unrecognized protocol"));
    }

    auto callback = callback_;
    callback_ = nullptr;
    helper_->start(std::move(socket_), callback);
    CHECK(!socket_);
  }

  void peekError(const folly::AsyncSocketException& ex) noexcept override {
    peeker_ = nullptr;
    auto callback = callback_;
    callback_ = nullptr;
    callback->connectionError(
        socket_.get(), folly::exception_wrapper(ex), folly::none);
  }

 private:
  ~PeekingAcceptorHandshakeHelper() override = default;

  folly::AsyncSSLSocket::UniquePtr socket_;
  AcceptorHandshakeHelper::UniquePtr helper_;
  SocketPeeker::UniquePtr peeker_;

  AcceptorHandshakeHelper::Callback* callback_;
  const folly::SocketAddress& clientAddr_;
  std::chrono::steady_clock::time_point acceptTime_;
  TransportInfo& tinfo_;
  const std::vector<PeekCallback*>& peekCallbacks_;
  size_t numBytes_;
};

using PeekingCallbackPtr = PeekingAcceptorHandshakeHelper::PeekCallback*;

class PeekingAcceptorHandshakeManager : public AcceptorHandshakeManager {
 public:
  PeekingAcceptorHandshakeManager(
      Acceptor* acceptor,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      TransportInfo tinfo,
      const std::vector<PeekingCallbackPtr>& peekCallbacks,
      size_t numBytes)
      : AcceptorHandshakeManager(
            acceptor,
            clientAddr,
            acceptTime,
            std::move(tinfo)),
        peekCallbacks_(peekCallbacks),
        numBytes_(numBytes) {}

 protected:
  void startHelper(folly::AsyncSSLSocket::UniquePtr sock) override {
    helper_.reset(new PeekingAcceptorHandshakeHelper(
        clientAddr_, acceptTime_, tinfo_, peekCallbacks_, numBytes_));
    helper_->start(std::move(sock), this);
  }

  const std::vector<PeekingCallbackPtr>& peekCallbacks_;
  size_t numBytes_;
};

} // namespace wangle
