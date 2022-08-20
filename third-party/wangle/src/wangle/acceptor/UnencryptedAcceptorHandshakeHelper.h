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

#include <folly/io/async/AsyncSSLSocket.h>

namespace wangle {

/**
 * This is a dummy handshake helper that immediately returns the socket to the
 * acceptor. This can be used with the peeking acceptor if no handshake is
 * needed.
 */
class UnencryptedAcceptorHandshakeHelper : public AcceptorHandshakeHelper {
 public:
  UnencryptedAcceptorHandshakeHelper() = default;

  void start(
      folly::AsyncSSLSocket::UniquePtr sock,
      AcceptorHandshakeHelper::Callback* callback) noexcept override {
    callback->connectionReady(
        std::move(sock), "", SecureTransportType::NONE, folly::none);
  }

  void dropConnection(
      SSLErrorEnum /* reason */ = SSLErrorEnum::NO_ERROR) override {
    CHECK(false) << "Nothing to drop";
  }
};

} // namespace wangle
