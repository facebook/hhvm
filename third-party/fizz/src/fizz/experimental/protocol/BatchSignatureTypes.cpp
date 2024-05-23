/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/experimental/protocol/BatchSignatureTypes.h>

namespace fizz {

constexpr std::array<SignatureScheme, 2> BatchSignatureSchemes<Sha256>::schemes;

folly::Optional<BatchSchemeInfo> getBatchSchemeInfo(
    SignatureScheme batchScheme,
    const std::vector<SignatureScheme>& supportedBaseSchemes) {
  BatchSchemeInfo info{};
  switch (batchScheme) {
    case SignatureScheme::ecdsa_secp256r1_sha256_batch:
      info.baseScheme = SignatureScheme::ecdsa_secp256r1_sha256;
      break;
    case SignatureScheme::ecdsa_secp384r1_sha384_batch:
      info.baseScheme = SignatureScheme::ecdsa_secp384r1_sha384;
      break;
    case SignatureScheme::ecdsa_secp521r1_sha512_batch:
      info.baseScheme = SignatureScheme::ecdsa_secp521r1_sha512;
      break;
    case SignatureScheme::ed25519_batch:
      info.baseScheme = SignatureScheme::ed25519;
      break;
    case SignatureScheme::ed448_batch:
      info.baseScheme = SignatureScheme::ed448;
      break;
    case SignatureScheme::rsa_pss_sha256_batch:
      info.baseScheme = SignatureScheme::rsa_pss_sha256;
      break;
    case SignatureScheme::ecdsa_secp256r1_sha256:
    case SignatureScheme::ecdsa_secp384r1_sha384:
    case SignatureScheme::ecdsa_secp521r1_sha512:
    case SignatureScheme::rsa_pss_sha256:
    case SignatureScheme::rsa_pss_sha384:
    case SignatureScheme::rsa_pss_sha512:
    case SignatureScheme::ed25519:
    case SignatureScheme::ed448:
      return folly::none;
  }
  if (!supportedBaseSchemes.empty() &&
      std::find(
          supportedBaseSchemes.begin(),
          supportedBaseSchemes.end(),
          info.baseScheme) == supportedBaseSchemes.end()) {
    return folly::none;
  }
  return info;
}

} // namespace fizz
