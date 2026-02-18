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

#include <folly/lang/Bits.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersServerExtension.h>

namespace apache::thrift {
NegotiationParameters ThriftParametersServerExtension::negotiate(
    const ThriftParametersExt& clientParameters,
    const ThriftParametersContext& serverConfiguration) {
  std::uint64_t compressionAlgorithms = 0;
  for (const auto& comp :
       serverConfiguration.getSupportedCompressionAlgorithms()) {
    assert(comp != CompressionAlgorithm::NONE);
    compressionAlgorithms |= 1ull << (int(comp) - 1);
  }

  uint64_t serverPSP = [&] {
    const auto& clientExtension = clientParameters.params;
    uint64_t clientSupports = 0;
    if (auto clientPSP = clientExtension.get_pspUpgradeProtocol()) {
      clientSupports |= *clientPSP;
    }

    uint64_t serverSupports = serverConfiguration.getSupportedPSPNegotiations();
    uint64_t common = clientSupports & serverSupports;
    auto highestBit = folly::findLastSet(common);
    if (highestBit > 0) {
      return uint64_t(1) << (highestBit - 1);
    } else {
      return uint64_t(0);
    }
  }();

  NegotiationParameters negotiatedParams;
  negotiatedParams.compressionAlgos() = compressionAlgorithms;
  negotiatedParams.useStopTLS() = serverConfiguration.getUseStopTLS();
  negotiatedParams.useStopTLSV2() = serverConfiguration.getUseStopTLSV2();
  negotiatedParams.useStopTLSForTTLSTunnel() =
      serverConfiguration.getUseStopTLSForTTLSTunnel();
  negotiatedParams.pspUpgradeProtocol() = serverPSP;
  return negotiatedParams;
}
} // namespace apache::thrift
