/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/signature/Signature.h>
#include <fizz/extensions/tokenbinding/Types.h>
#include <fizz/record/Types.h>

namespace fizz {
namespace extensions {

/*
 * validateTokenBinding verifies the signature passed in with tokenBinding
 * If verification succeeds, the TokenBindingID associated with the signature is
 * returned. On verification failure, folly::none is returned
 */

class Validator {
 public:
  static folly::Optional<TokenBindingID> validateTokenBinding(
      TokenBinding tokenBinding,
      const Buf& ekm,
      const TokenBindingKeyParameters& negotiatedParameters);

  static folly::Optional<TokenBindingID> validateTokenBinding(
      TokenBinding tokenBinding,
      const Buf& ekm,
      // vector is used instead of set because `supportedParameters` is expected
      // to be small in size
      const std::vector<TokenBindingKeyParameters>& supportedParameters);

 private:
  static folly::Optional<TokenBindingID> constructAndVerifyMessage(
      TokenBinding tokenBinding,
      const Buf& ekm);

  static void verify(
      const TokenBindingKeyParameters& keyParams,
      const Buf& key,
      const Buf& signature,
      const Buf& message);

  static folly::ssl::EcdsaSigUniquePtr constructECDSASig(const Buf& signature);

  static folly::ssl::EcKeyUniquePtr constructEcKeyFromBuf(const Buf& key);
};
} // namespace extensions
} // namespace fizz
