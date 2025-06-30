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

std::vector<fizz::Extension>
ThriftParametersClientExtension::getClientHelloExtensions() const {
  std::vector<fizz::Extension> clientExtensions;
  NegotiationParameters params;
  uint64_t compressionAlgorithms = 0;
  for (const auto& comp : context_->getSupportedCompressionAlgorithms()) {
    assert(comp != CompressionAlgorithm::NONE);
    compressionAlgorithms |= 1ull << (int(comp) - 1);
  }
  params.compressionAlgos_ref() = compressionAlgorithms;
  params.useStopTLS_ref() = context_->getUseStopTLS();
  params.useStopTLSV2_ref() =
      context_->getUseStopTLSV2(); // Added for StopTLS V2
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
  if (auto serverCompressions = serverParams->params.compressionAlgos_ref()) {
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
      serverParams->params.useStopTLS_ref().value_or(false);

  negotiatedStopTLSV2_ = context_->getUseStopTLSV2() &&
      serverParams->params.useStopTLSV2_ref().value_or(false);
}

} // namespace apache::thrift
