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
#include <fizz/protocol/Factory.h>

namespace fizz {

class OpenSSLFactory : public Factory {
 public:
  std::unique_ptr<KeyDerivation> makeKeyDeriver(
      CipherSuite cipher) const override {
    switch (cipher) {
      case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
      case CipherSuite::TLS_AES_128_GCM_SHA256:
      case CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL:
      case CipherSuite::TLS_AEGIS_128L_SHA256:
        return KeyDerivationImpl::make<Sha256>(getHkdfPrefix());
      case CipherSuite::TLS_AES_256_GCM_SHA384:
      case CipherSuite::TLS_AEGIS_256_SHA384:
        return KeyDerivationImpl::make<Sha384>(getHkdfPrefix());
      default:
        throw std::runtime_error("ks: not implemented");
    }
  }

  std::unique_ptr<HandshakeContext> makeHandshakeContext(
      CipherSuite cipher) const override {
    switch (cipher) {
      case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
      case CipherSuite::TLS_AES_128_GCM_SHA256:
      case CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL:
      case CipherSuite::TLS_AEGIS_128L_SHA256:
        return std::make_unique<HandshakeContextImpl<Sha256>>(getHkdfPrefix());
      case CipherSuite::TLS_AES_256_GCM_SHA384:
      case CipherSuite::TLS_AEGIS_256_SHA384:
        return std::make_unique<HandshakeContextImpl<Sha384>>(getHkdfPrefix());
      default:
        throw std::runtime_error("hs: not implemented");
    }
  }
};
} // namespace fizz
