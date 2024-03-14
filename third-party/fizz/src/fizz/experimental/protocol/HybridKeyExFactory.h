/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/exchange/HybridKeyExchange.h>
#include <fizz/experimental/crypto/exchange/OQSKeyExchange.h>
#include <fizz/protocol/OpenSSLFactory.h>

namespace fizz {
class HybridKeyExFactory : public OpenSSLFactory {
 public:
  std::unique_ptr<KeyExchange> makeKeyExchange(
      NamedGroup group,
      KeyExchangeMode mode) const override {
    switch (group) {
      case NamedGroup::secp521r1_x25519:
        return std::make_unique<HybridKeyExchange>(
            std::make_unique<OpenSSLECKeyExchange<P521>>(),
            std::make_unique<X25519KeyExchange>());
      case NamedGroup::x25519_kyber512:
        return std::make_unique<HybridKeyExchange>(
            std::make_unique<X25519KeyExchange>(),
            OQSKeyExchange::createOQSKeyExchange(mode, OQS_KEM_alg_kyber_512));
      case NamedGroup::secp256r1_kyber512:
        return std::make_unique<HybridKeyExchange>(
            std::make_unique<OpenSSLECKeyExchange<P256>>(),
            OQSKeyExchange::createOQSKeyExchange(mode, OQS_KEM_alg_kyber_512));
      case NamedGroup::kyber512:
        return OQSKeyExchange::createOQSKeyExchange(
            mode, OQS_KEM_alg_kyber_512);
      case NamedGroup::x25519_kyber768_draft00:
      case NamedGroup::x25519_kyber768_experimental:
        return std::make_unique<HybridKeyExchange>(
            std::make_unique<X25519KeyExchange>(),
            OQSKeyExchange::createOQSKeyExchange(mode, OQS_KEM_alg_kyber_768));
      case NamedGroup::secp256r1_kyber768_draft00:
        return std::make_unique<HybridKeyExchange>(
            std::make_unique<OpenSSLECKeyExchange<P256>>(),
            OQSKeyExchange::createOQSKeyExchange(mode, OQS_KEM_alg_kyber_768));
      case NamedGroup::secp384r1_kyber768:
        return std::make_unique<HybridKeyExchange>(
            std::make_unique<OpenSSLECKeyExchange<P384>>(),
            OQSKeyExchange::createOQSKeyExchange(mode, OQS_KEM_alg_kyber_768));
      default:
        return OpenSSLFactory::makeKeyExchange(group, mode);
    }
  }
};
} // namespace fizz
