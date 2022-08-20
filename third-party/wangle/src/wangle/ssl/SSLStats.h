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

#include <stdint.h>

namespace wangle {

class SSLStats {
 public:
  virtual ~SSLStats() noexcept {}

  // downstream
  virtual void recordSSLAcceptLatency(int64_t latency) noexcept = 0;
  virtual void recordTLSTicket(bool ticketNew, bool ticketHit) noexcept = 0;
  virtual void
  recordSSLSession(bool sessionNew, bool sessionHit, bool foreign) noexcept = 0;
  virtual void recordSSLSessionRemove() noexcept = 0;
  virtual void recordSSLSessionFree(uint32_t freed) noexcept = 0;
  virtual void recordSSLSessionSetError(uint32_t err) noexcept = 0;
  virtual void recordSSLSessionGetError(uint32_t err) noexcept = 0;
  virtual void recordClientRenegotiation() noexcept = 0;
  virtual void recordSSLClientCertificateMismatch() noexcept = 0;
  virtual void recordTLSTicketRotation(bool valid) noexcept = 0;

  // upstream
  virtual void recordSSLUpstreamConnection(bool handshake) noexcept = 0;
  virtual void recordSSLUpstreamConnectionError(bool verifyError) noexcept = 0;
};

} // namespace wangle
