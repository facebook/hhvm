/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/extensions/tokenbinding/Types.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {
namespace extensions {

class TokenBindingConstructor {
 public:
  static Status createTokenBinding(
      TokenBinding& ret,
      Error& err,
      EVP_PKEY& keyPair,
      const Buf& ekm,
      TokenBindingKeyParameters negotiatedParameters,
      TokenBindingType type);

 private:
  static Status
  encodeEcKey(Buf& ret, Error& err, const folly::ssl::EcKeyUniquePtr& ecKey);

  static Status encodeEcdsaSignature(
      Buf& ret,
      Error& err,
      const folly::ssl::EcdsaSigUniquePtr& signature);

  static Status signWithEcKey(
      Buf& ret,
      Error& err,
      const folly::ssl::EcKeyUniquePtr& key,
      const Buf& message);

  static Status
  addBignumToSignature(Error& err, const Buf& signature, BIGNUM* bigNum);
};
} // namespace extensions
} // namespace fizz
