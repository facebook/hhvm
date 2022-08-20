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

#include <folly/portability/GMock.h>
#include <wangle/ssl/SSLStats.h>

namespace wangle {

class MockSSLStats : public wangle::SSLStats {
 public:
  MOCK_METHOD(void, recordTLSTicketRotation, (bool valid), (noexcept));

  // downstream
  void recordSSLAcceptLatency(int64_t /* unused */) noexcept override {}
  void recordTLSTicket(bool /* unused */, bool /* unused */) noexcept override {
  }
  void recordSSLSession(
      bool /* unused */,
      bool /* unused */,
      bool /* unused */) noexcept override {}
  void recordSSLSessionRemove() noexcept override {}
  void recordSSLSessionFree(uint32_t /* unused */) noexcept override {}
  void recordSSLSessionSetError(uint32_t /* unused */) noexcept override {}
  void recordSSLSessionGetError(uint32_t /* unused */) noexcept override {}
  void recordClientRenegotiation() noexcept override {}
  void recordSSLClientCertificateMismatch() noexcept override {}

  // upstream
  MOCK_METHOD(void, recordSSLUpstreamConnection, (bool handshake), (noexcept));
  void recordSSLUpstreamConnectionError(bool /* unused */) noexcept override {}
};

} // namespace wangle
