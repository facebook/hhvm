/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/extensions/delegatedcred/DelegatedCredentialClientExtension.h>
#include <fizz/extensions/delegatedcred/Types.h>

namespace fizz {
namespace extensions {

Status DelegatedCredentialClientExtension::getClientHelloExtensions(
    std::vector<Extension>& ret,
    Error& err) const {
  ret.clear();
  DelegatedCredentialSupport supp;
  supp.supported_signature_algorithms = supportedSchemes_;
  Extension ext;
  FIZZ_RETURN_ON_ERROR(encodeExtension(ext, err, supp));
  ret.push_back(std::move(ext));
  return Status::Success;
}

Status DelegatedCredentialClientExtension::onEncryptedExtensions(
    Error& /* err */,
    const std::vector<Extension>& /* extensions */) {
  return Status::Success;
}
} // namespace extensions
} // namespace fizz
