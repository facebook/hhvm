/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Sha256.h>
#include <fizz/record/Types.h>

namespace fizz {

/**
 * Batch signature schemes based on Hash.
 *
 * Leave undefined intentionally. Those that are not defined explicitly defined
 * will result in compilation error.
 */
template <class Hash>
struct BatchSignatureSchemes;

/**
 * Batch signature schemes based on Sha256.
 */
template <>
struct BatchSignatureSchemes<Sha256> {
  constexpr static std::array<SignatureScheme, 2> schemes = {
      SignatureScheme::ecdsa_secp256r1_sha256_batch,
      SignatureScheme::rsa_pss_sha256_batch,
  };

  static folly::Optional<SignatureScheme> getFromBaseScheme(
      SignatureScheme baseScheme) {
    switch (baseScheme) {
      case SignatureScheme::ecdsa_secp256r1_sha256:
        return SignatureScheme::ecdsa_secp256r1_sha256_batch;
      case SignatureScheme::rsa_pss_sha256:
        return SignatureScheme::rsa_pss_sha256_batch;
      case SignatureScheme::ed25519:
      case SignatureScheme::ed448:
      case SignatureScheme::ecdsa_secp384r1_sha384:
      case SignatureScheme::ecdsa_secp521r1_sha512:
      case SignatureScheme::rsa_pss_sha384:
      case SignatureScheme::rsa_pss_sha512:
      case SignatureScheme::ecdsa_secp256r1_sha256_batch:
      case SignatureScheme::ecdsa_secp384r1_sha384_batch:
      case SignatureScheme::ecdsa_secp521r1_sha512_batch:
      case SignatureScheme::rsa_pss_sha256_batch:
      case SignatureScheme::ed25519_batch:
      case SignatureScheme::ed448_batch:
        break;
    }
    return folly::none;
  }
};

// TODO: Extend the BatchSchemeInfo to be:
//       struct BatchSchemeInfo {
//         SignatureScheme baseScheme;
//         Hasher hasher;
//       };
struct BatchSchemeInfo {
  SignatureScheme baseScheme;
};

/**
 * Try to get information of a batch signature scheme.
 *
 * If @param supportedBaseSchemes is empty, the applicable base signature scheme
 * will be returned directly.
 *
 * @return folly::none if @param batchScheme is not a batch signature scheme or
 *         the base signature scheme is not in @param supportedBaseSchemes.
 */
folly::Optional<BatchSchemeInfo> getBatchSchemeInfo(
    SignatureScheme batchScheme,
    const std::vector<SignatureScheme>& supportedBaseSchemes = {});

} // namespace fizz
