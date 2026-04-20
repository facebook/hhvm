/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/extensions/tokenbinding/TokenBindingClientExtension.h>
#include <fizz/util/Logging.h>

namespace fizz {
namespace extensions {

Status TokenBindingClientExtension::getClientHelloExtensions(
    std::vector<Extension>& ret,
    Error& err) const {
  ret.clear();
  if (context_->getSupportedVersions().empty() ||
      context_->getKeyParams().empty()) {
    return Status::Success;
  }
  TokenBindingParameters clientParams;
  clientParams.version = context_->getSupportedVersions().front();
  clientParams.key_parameters_list = context_->getKeyParams();
  Extension ext;
  FIZZ_RETURN_ON_ERROR(encodeExtension(ext, err, clientParams));
  ret.push_back(std::move(ext));
  return Status::Success;
}

Status TokenBindingClientExtension::onEncryptedExtensions(
    Error& err,
    const std::vector<Extension>& extensions) {
  folly::Optional<TokenBindingParameters> serverParams;
  FIZZ_RETURN_ON_ERROR(
      getExtension<TokenBindingParameters>(serverParams, err, extensions));
  if (!serverParams.has_value()) {
    FIZZ_VLOG(6) << "Server did not negotiate token binding";
    return Status::Success;
  }
  if (serverParams->key_parameters_list.size() != 1) {
    return err.error(
        "Incorrect number of key_parameters sent by server",
        AlertDescription::unsupported_extension);
  }
  if (serverParams->version > context_->getSupportedVersions().front()) {
    return err.error(
        "Server sent higher tokbind version",
        AlertDescription::unsupported_extension);
  }

  auto keyParam = std::find(
      context_->getKeyParams().begin(),
      context_->getKeyParams().end(),
      serverParams->key_parameters_list.front());
  if (keyParam == context_->getKeyParams().end()) {
    return err.error(
        "Unsupported key parameter sent by server",
        AlertDescription::unsupported_extension);
  }

  auto version = std::find(
      context_->getSupportedVersions().begin(),
      context_->getSupportedVersions().end(),
      serverParams->version);
  if (version == context_->getSupportedVersions().end()) {
    FIZZ_VLOG(6) << "Server sent lower, unsupported, token binding version";
    return Status::Success;
  }
  negotiatedVersion_ = *version;
  negotiatedKeyParam_ = *keyParam;
  return Status::Success;
}
} // namespace extensions
} // namespace fizz
