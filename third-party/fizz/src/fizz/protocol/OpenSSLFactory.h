/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Sha256.h>
#include <fizz/crypto/Sha384.h>
#include <fizz/crypto/aead/AEGISCipher.h>
#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/AESGCM256.h>
#include <fizz/crypto/aead/AESOCB128.h>
#include <fizz/crypto/aead/ChaCha20Poly1305.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/crypto/exchange/ECCurveKeyExchange.h>
#include <fizz/crypto/exchange/X25519.h>
#include <fizz/protocol/DefaultFactory.h>

namespace fizz {

class OpenSSLFactory : public DefaultFactory {
 public:
  [[nodiscard]] std::unique_ptr<KeyExchange> makeKeyExchange(
      NamedGroup group,
      KeyExchangeMode mode) const override;

  [[nodiscard]] std::unique_ptr<Aead> makeAead(
      CipherSuite cipher) const override;

  std::unique_ptr<KeyDerivation> makeKeyDeriver(
      CipherSuite cipher) const override;

  std::unique_ptr<HandshakeContext> makeHandshakeContext(
      CipherSuite cipher) const override;

  [[nodiscard]] std::shared_ptr<PeerCert> makePeerCert(
      CertificateEntry certEntry,
      bool /*leaf*/) const override;
};
} // namespace fizz
