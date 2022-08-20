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
  std::unique_ptr<KeyExchange> makeKeyExchange(
      NamedGroup group,
      KeyExchangeMode mode) const override {
    switch (group) {
      case NamedGroup::secp521r1_x25519:
        return std::make_unique<HybridKeyExchange>(
            std::make_unique<OpenSSLECKeyExchange<P521>>(),
            std::make_unique<X25519KeyExchange>());
      case NamedGroup::secp384r1_bikel3:
        return std::make_unique<HybridKeyExchange>(
            std::make_unique<OpenSSLECKeyExchange<P384>>(),
            OQSKeyExchange::createOQSKeyExchange(mode, OQS_KEM_alg_bike_l3));
      default:
        throw std::runtime_error("ke: not implemented");
    }
  }
};
} // namespace fizz
