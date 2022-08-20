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

#include <wangle/acceptor/TLSPlaintextPeekingCallback.h>

namespace wangle {

bool TLSPlaintextPeekingCallback::looksLikeTLS(
    const std::vector<uint8_t>& bytes) {
  CHECK_GE(bytes.size(), kPeekCount);
  // TLS starts with
  // 0: 0x16 - handshake magic
  // 1: 0x03 - SSL major version
  // 2: 0x00 to 0x03 - minor version
  // 3-4: Length
  // 5: 0x01 - Handshake type (Client Hello)
  if (bytes[0] != 0x16 || bytes[1] != 0x03 || bytes[5] != 0x01) {
    return false;
  }
  return true;
}

AcceptorHandshakeHelper::UniquePtr TLSPlaintextPeekingCallback::getHelper(
    const std::vector<uint8_t>& bytes,
    const folly::SocketAddress& /* clientAddr */,
    std::chrono::steady_clock::time_point /* acceptTime */,
    TransportInfo&) {
  if (!TLSPlaintextPeekingCallback::looksLikeTLS(bytes)) {
    return AcceptorHandshakeHelper::UniquePtr(
        new UnencryptedAcceptorHandshakeHelper());
  }

  return nullptr;
}

} // namespace wangle
