/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/extensions/tokenbinding/TokenBindingClientExtension.h>

namespace fizz {
namespace extensions {

std::vector<Extension> TokenBindingClientExtension::getClientHelloExtensions()
    const {
  std::vector<Extension> clientExtensions;
  if (context_->getSupportedVersions().empty() ||
      context_->getKeyParams().empty()) {
    return clientExtensions;
  }
  TokenBindingParameters clientParams;
  clientParams.version = context_->getSupportedVersions().front();
  clientParams.key_parameters_list = context_->getKeyParams();
  clientExtensions.push_back(encodeExtension(clientParams));
  return clientExtensions;
}

void TokenBindingClientExtension::onEncryptedExtensions(
    const std::vector<Extension>& extensions) {
  auto serverParams = getExtension<TokenBindingParameters>(extensions);
  if (!serverParams.has_value()) {
    VLOG(6) << "Server did not negotiate token binding";
    return;
  }
  if (serverParams->key_parameters_list.size() != 1) {
    throw FizzException(
        "Incorrect number of key_parameters sent by server",
        AlertDescription::unsupported_extension);
  }
  if (serverParams->version > context_->getSupportedVersions().front()) {
    throw FizzException(
        "Server sent higher tokbind version",
        AlertDescription::unsupported_extension);
  }

  auto keyParam = std::find(
      context_->getKeyParams().begin(),
      context_->getKeyParams().end(),
      serverParams->key_parameters_list.front());
  if (keyParam == context_->getKeyParams().end()) {
    throw FizzException(
        "Unsupported key parameter sent by server",
        AlertDescription::unsupported_extension);
  }

  auto version = std::find(
      context_->getSupportedVersions().begin(),
      context_->getSupportedVersions().end(),
      serverParams->version);
  if (version == context_->getSupportedVersions().end()) {
    VLOG(6) << "Server sent lower, unsupported, token binding version";
    return;
  }
  negotiatedVersion_ = *version;
  negotiatedKeyParam_ = *keyParam;
}
} // namespace extensions
} // namespace fizz
