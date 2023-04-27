/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/extensions/delegatedcred/DelegatedCredentialClientExtension.h>

namespace fizz {
namespace extensions {

std::vector<Extension>
DelegatedCredentialClientExtension::getClientHelloExtensions() const {
  std::vector<Extension> clientExtensions;
  DelegatedCredentialSupport supp;
  supp.supported_signature_algorithms = supportedSchemes_;
  clientExtensions.push_back(encodeExtension(std::move(supp)));
  return clientExtensions;
}

void DelegatedCredentialClientExtension::onEncryptedExtensions(
    const std::vector<Extension>& extensions) {}
} // namespace extensions
} // namespace fizz
