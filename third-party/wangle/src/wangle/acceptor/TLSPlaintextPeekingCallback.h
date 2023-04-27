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
#include <wangle/acceptor/SSLAcceptorHandshakeHelper.h>
#include <wangle/acceptor/UnencryptedAcceptorHandshakeHelper.h>

namespace wangle {

/**
 * A peeking callback that makes it convenient to create a server
 * that will accept both TLS and plaintext traffic.
 */
class TLSPlaintextPeekingCallback
    : public PeekingAcceptorHandshakeHelper::PeekCallback {
  enum { kPeekCount = 9 };

 public:
  TLSPlaintextPeekingCallback()
      : PeekingAcceptorHandshakeHelper::PeekCallback(kPeekCount) {}

  AcceptorHandshakeHelper::UniquePtr getHelper(
      const std::vector<uint8_t>& bytes,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      TransportInfo& tinfo) override;

 private:
  static bool looksLikeTLS(const std::vector<uint8_t>& peekBytes);
};

} // namespace wangle
