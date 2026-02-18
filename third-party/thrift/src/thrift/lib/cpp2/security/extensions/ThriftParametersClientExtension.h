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

#include <fizz/client/ClientExtensions.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>
#include <thrift/lib/cpp2/security/extensions/Types.h>

namespace apache::thrift {

class ThriftParametersClientExtension : public fizz::ClientExtensions {
 public:
  explicit ThriftParametersClientExtension(
      const std::shared_ptr<ThriftParametersContext>& context);

  std::vector<fizz::Extension> getClientHelloExtensions() const override;

  void onEncryptedExtensions(
      const std::vector<fizz::Extension>& extensions) override;

  /**
   * Return the negotiated compression algorithm that the client will use to
   * compress requests.
   */
  const folly::Optional<CompressionAlgorithm>& getThriftCompressionAlgorithm()
      const {
    return negotiatedThriftCompressionAlgo_;
  }

  bool getNegotiatedStopTLS() const { return negotiatedStopTLS_; }

  bool getNegotiatedStopTLSV2() const { return negotiatedStopTLSV2_; }

  bool getNegotiatedStopTLSForTTLSTunnel() const {
    return negotiatedStopTLSForTTLSTunnel_;
  }

  uint64_t getNegotiatedPSPUpgrade() const { return negotiatedPSP_; }

 private:
  folly::Optional<CompressionAlgorithm> negotiatedThriftCompressionAlgo_;
  bool negotiatedStopTLS_{false};
  bool negotiatedStopTLSV2_{false}; // Added for StopTLS V2 support
  bool negotiatedStopTLSForTTLSTunnel_{false};

  uint64_t offerredPSP_{};
  uint64_t negotiatedPSP_{};
  std::shared_ptr<ThriftParametersContext> context_;
};
} // namespace apache::thrift
