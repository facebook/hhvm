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

#include <cstdint>
#include <folly/Function.h>
#include <folly/SocketAddress.h>
#include <thrift/lib/cpp2/security/PSP.h>
#include <thrift/lib/cpp2/security/extensions/Types.h>

namespace apache::thrift {

/**
 * ThriftParametersContext conveys per-connection properties that may be used
 * during TLS negotiation for a secure Thrift connection.
 */
class ThriftParametersContext {
 public:
  folly::Range<const CompressionAlgorithm*> getSupportedCompressionAlgorithms()
      const {
    return supportedCompressionAlgos_;
  }

  void setPeerAddress(const folly::SocketAddress& addr) { peerAddr_ = addr; }
  void setUseStopTLS(bool useStopTLS) { useStopTLS_ = useStopTLS; }

  bool getUseStopTLS() const { return useStopTLS_; }

  void setUseStopTLSV2(bool useStopTLSV2) { useStopTLSV2_ = useStopTLSV2; }

  bool getUseStopTLSV2() const { return useStopTLSV2_; }

  void setUseStopTLSForTTLSTunnel(bool useStopTLSForTTLSTunnel) {
    useStopTLSForTTLSTunnel_ = useStopTLSForTTLSTunnel;
  }

  bool getUseStopTLSForTTLSTunnel() const { return useStopTLSForTTLSTunnel_; }

  // A policy function that determines which PSP negotiation versions are
  // supported for a given peer at `peerAddr`.
  using PSPNegotiationPolicy =
      std::uint64_t(const folly::SocketAddress& peerAddr) const;
  void setSupportedPSPVersionsPolicy(
      folly::Function<PSPNegotiationPolicy> pspPolicy) {
    if (pspPolicy) {
      pspNegotiationPolicy_ = std::move(pspPolicy);
    }
  }

  uint64_t getSupportedPSPNegotiations() const {
    return pspNegotiationPolicy_(peerAddr_);
  }

 private:
  static constexpr std::array<CompressionAlgorithm, 2>
      supportedCompressionAlgos_{{
          CompressionAlgorithm::ZSTD,
          CompressionAlgorithm::ZLIB,
      }};
  bool useStopTLS_{false};
  bool useStopTLSV2_{false};
  bool useStopTLSForTTLSTunnel_{false};
  folly::SocketAddress peerAddr_;
  folly::Function<PSPNegotiationPolicy> pspNegotiationPolicy_{
      [](const folly::SocketAddress&) { return THRIFT_PSP_NONE; }};
};
} // namespace apache::thrift
