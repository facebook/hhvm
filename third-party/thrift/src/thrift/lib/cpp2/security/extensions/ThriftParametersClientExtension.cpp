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

#include <thrift/lib/cpp2/security/extensions/ThriftParametersClientExtension.h>

namespace apache::thrift {

ThriftParametersClientExtension::ThriftParametersClientExtension(
    const std::shared_ptr<ThriftParametersContext>& context)
    : context_(context) {
  // context_->getSupportedPSPNegotiations() invokes an arbitrary policy
  // function, so we evaluate it once and use the result consistently
  // throughout the lifetime of the connection.
  offerredPSP_ = context_->getSupportedPSPNegotiations();
}
std::vector<fizz::Extension>
ThriftParametersClientExtension::getClientHelloExtensions() const {
  std::vector<fizz::Extension> clientExtensions;
  NegotiationParameters params;
  uint64_t compressionAlgorithms = 0;
  for (const auto& comp : context_->getSupportedCompressionAlgorithms()) {
    assert(comp != CompressionAlgorithm::NONE);
    compressionAlgorithms |= 1ull << (int(comp) - 1);
  }

  params.compressionAlgos() = compressionAlgorithms;
  params.useStopTLS() = context_->getUseStopTLS();
  params.useStopTLSV2() = context_->getUseStopTLSV2(); // Added for StopTLS V2
  params.useStopTLSForTTLSTunnel() = context_->getUseStopTLSForTTLSTunnel();
  params.pspUpgradeProtocol() = offerredPSP_;

  ThriftParametersExt paramsExt;
  paramsExt.params = params;
  clientExtensions.push_back(encodeThriftExtension(paramsExt));
  return clientExtensions;
}

void ThriftParametersClientExtension::onEncryptedExtensions(
    const std::vector<fizz::Extension>& extensions) {
  folly::Optional<ThriftParametersExt> serverParams =
      getThriftExtension(extensions);

  if (!serverParams.has_value()) {
    VLOG(6) << "Server did not negotiate thrift parameters";
    return;
  }
  const auto& negotiatedParams = serverParams->params;
  if (auto serverCompressions = negotiatedParams.compressionAlgos()) {
    for (const auto& comp : context_->getSupportedCompressionAlgorithms()) {
      assert(comp != CompressionAlgorithm::NONE);
      if (*serverCompressions & 1ull << (int(comp) - 1)) {
        negotiatedThriftCompressionAlgo_ = comp;
        break;
      }
    }
  } else {
    VLOG(6) << "Server did not negotiate thrift compression algorithms";
  }

  negotiatedStopTLS_ = context_->getUseStopTLS() &&
      negotiatedParams.useStopTLS().value_or(false);
  negotiatedStopTLSV2_ = context_->getUseStopTLSV2() &&
      negotiatedParams.useStopTLSV2().value_or(false);
  negotiatedStopTLSForTTLSTunnel_ = context_->getUseStopTLSForTTLSTunnel() &&
      negotiatedParams.useStopTLSForTTLSTunnel().value_or(false);

  // The server must select a bit from our offerred bitmask and the server
  // must select only one version.
  auto serverPSPSelection = negotiatedParams.pspUpgradeProtocol().value_or(0);
  if (folly::popcount((serverPSPSelection & offerredPSP_)) == 1) {
    negotiatedPSP_ = serverPSPSelection;
  }
}

} // namespace apache::thrift
