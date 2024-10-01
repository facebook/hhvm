/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/ECCurve.h>
#include <fizz/backend/openssl/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/backend/openssl/crypto/exchange/OpenSSLKeyExchange.h>
#include <fizz/crypto/exchange/X25519.h>
#include <fizz/protocol/Factory.h>

namespace fizz {
class PeerCert;

/**
 * A fizz::Factory implementation composed of primitives from
 * multiple backends.
 */
class MultiBackendFactory : public Factory {
 public:
  [[nodiscard]] std::unique_ptr<KeyExchange> makeKeyExchange(
      NamedGroup group,
      KeyExchangeRole role) const override;

  [[nodiscard]] std::unique_ptr<Aead> makeAead(
      CipherSuite cipher) const override;

  const HasherFactoryWithMetadata* makeHasherFactory(
      HashFunction digest) const override;

  [[nodiscard]] std::unique_ptr<PeerCert> makePeerCert(
      CertificateEntry certEntry,
      bool /*leaf*/) const override;
};
} // namespace fizz
