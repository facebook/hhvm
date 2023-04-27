/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#pragma once

#include <fizz/client/ClientExtensions.h>
#include <fizz/extensions/delegatedcred/Types.h>

namespace fizz {
namespace extensions {

class DelegatedCredentialClientExtension : public ClientExtensions {
 public:
  explicit DelegatedCredentialClientExtension(
      std::vector<SignatureScheme> schemes)
      : supportedSchemes_(std::move(schemes)) {}

  std::vector<Extension> getClientHelloExtensions() const override;

  void onEncryptedExtensions(const std::vector<Extension>& extensions) override;
  std::vector<SignatureScheme> supportedSchemes_;
};
} // namespace extensions
} // namespace fizz
