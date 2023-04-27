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

#include <wangle/acceptor/PeekingAcceptorHandshakeHelper.h>

namespace wangle {

/**
 * This class holds different peekers that will be used to get the appropriate
 * AcceptorHandshakeHelper to handle the security protocol negotiation.
 */
class SecurityProtocolContextManager {
 public:
  /**
   * Adds a peeker to be used when accepting connections on a secure port.
   * Peekers will be used in the order they are added.
   */
  void addPeeker(PeekingCallbackPtr peekingCallback) {
    if (peekingCallback->getBytesRequired() > numBytes_) {
      numBytes_ = peekingCallback->getBytesRequired();
    }
    peekingCallbacks_.push_back(std::move(peekingCallback));
  }

  AcceptorHandshakeManager* getHandshakeManager(
      Acceptor* acceptor,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      TransportInfo& tinfo) noexcept {
    return new PeekingAcceptorHandshakeManager(
        acceptor, clientAddr, acceptTime, tinfo, peekingCallbacks_, numBytes_);
  }

  size_t getPeekBytes() const {
    return numBytes_;
  }

 private:
  /**
   * Peeking callbacks for each handshake protocol.
   */
  std::vector<PeekingCallbackPtr> peekingCallbacks_;

  /**
   * Highest number of bytes required by a peeking callback.
   */
  size_t numBytes_{0};
};

} // namespace wangle
