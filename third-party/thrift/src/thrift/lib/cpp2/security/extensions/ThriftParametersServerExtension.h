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

namespace apache {
namespace thrift {

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

    // send the server accept list
    std::uint64_t compressionAlgorithms = 0;
    for (const auto& comp : context_->getSupportedCompressionAlgorithms()) {
      assert(comp != CompressionAlgorithm::NONE);
      compressionAlgorithms |= 1ull << (int(comp) - 1);
    }
    NegotiationParameters negotiatedParams;
    negotiatedParams.compressionAlgos_ref() = compressionAlgorithms;
    negotiatedParams.useStopTLS_ref() = context_->getUseStopTLS();
    ThriftParametersExt paramsExt;
    paramsExt.params = negotiatedParams;
    serverExtensions.push_back(encodeThriftExtension(paramsExt));
    return serverExtensions;
  }

  /**
   * Return the negotiated compression algorithm that the server will use to
   * compress requests.
   */
  folly::Optional<CompressionAlgorithm> getThriftCompressionAlgorithm() {
    if (!clientExtensions_.has_value()) {
      return folly::none;
    }
    auto compressionAlgos =
        clientExtensions_->params.compressionAlgos_ref().value_or(0);
    for (const auto& comp : context_->getSupportedCompressionAlgorithms()) {
      assert(comp != CompressionAlgorithm::NONE);
      if (compressionAlgos & 1ull << (int(comp) - 1)) {
        return comp;
      }
    }
    return folly::none;
  }

  bool getNegotiatedStopTLS() {
    return context_->getUseStopTLS() && clientExtensions_.has_value() &&
        clientExtensions_->params.useStopTLS_ref().value_or(false);
  }

 private:
  folly::Optional<ThriftParametersExt> clientExtensions_;
  std::shared_ptr<ThriftParametersContext> context_;
};

} // namespace thrift
} // namespace apache
