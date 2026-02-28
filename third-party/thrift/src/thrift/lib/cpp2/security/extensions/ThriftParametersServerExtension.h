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

#include <fizz/server/ServerExtensions.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>
#include <thrift/lib/cpp2/security/extensions/Types.h>

namespace apache::thrift {

class ThriftParametersServerExtension : public fizz::ServerExtensions {
 public:
  explicit ThriftParametersServerExtension(
      const std::shared_ptr<ThriftParametersContext>& context)
      : context_(context) {}

  std::vector<fizz::Extension> getExtensions(
      const fizz::ClientHello& chlo) override {
    std::vector<fizz::Extension> serverExtensions;
    // get the client extensions from ClientHello
    auto params = getThriftExtension(chlo.extensions);
    if (!params) {
      VLOG(6) << "Client did not negotiate thrift parameters";
      return serverExtensions;
    }
    clientExtensions_ = std::move(params);

    NegotiationParameters negotiatedParams =
        negotiate(*clientExtensions_, *context_);
    // We need to save the result of `negotiatedPSP_` since determining which
    // value to send to the client involves a side effect (invoking the
    // PSP upgrade policy function). We need to ensure that the server is
    // consistent with its advertised behavior in case the policy function
    // changes in between invocations.
    negotiatedPSP_ = *negotiatedParams.pspUpgradeProtocol();

    ThriftParametersExt paramsExt;
    paramsExt.params = negotiatedParams;
    serverExtensions.push_back(encodeThriftExtension(paramsExt));
    return serverExtensions;
  }

  /**
   * Computes the NegotiationParameters that we will send back to the client.
   *
   * `clientParameters` is data sent by the Thrift client.
   *
   * `serverConfiguration` represents the server negotiation configuration
   *  parameters.
   *
   * This function is static to allow for unit testing.
   */
  static NegotiationParameters negotiate(
      const ThriftParametersExt& clientParameters,
      const ThriftParametersContext& serverConfiguration);

  /**
   * Return the negotiated compression algorithm that the server will use to
   * compress requests.
   */
  folly::Optional<CompressionAlgorithm> getThriftCompressionAlgorithm() const {
    if (!clientExtensions_.has_value()) {
      return folly::none;
    }
    auto compressionAlgos =
        clientExtensions_->params.compressionAlgos().value_or(0);
    for (const auto& comp : context_->getSupportedCompressionAlgorithms()) {
      assert(comp != CompressionAlgorithm::NONE);
      if (compressionAlgos & 1ull << (int(comp) - 1)) {
        return comp;
      }
    }
    return folly::none;
  }

  bool getNegotiatedStopTLS() const {
    return context_->getUseStopTLS() && clientExtensions_.has_value() &&
        clientExtensions_->params.useStopTLS().value_or(false);
  }

  bool getNegotiatedStopTLSV2() const {
    return context_->getUseStopTLSV2() && clientExtensions_.has_value() &&
        clientExtensions_->params.useStopTLSV2().value_or(false);
  }

  bool getNegotiatedStopTLSForTTLSTunnel() const {
    return context_->getUseStopTLSForTTLSTunnel() &&
        clientExtensions_.has_value() &&
        clientExtensions_->params.useStopTLSForTTLSTunnel().value_or(false);
  }

  uint64_t getNegotiatedPSPUpgrade() const { return negotiatedPSP_; }

 private:
  folly::Optional<ThriftParametersExt> clientExtensions_;
  std::shared_ptr<ThriftParametersContext> context_;
  uint64_t negotiatedPSP_{};
};

} // namespace apache::thrift
