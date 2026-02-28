/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Factory.h>

namespace fizz {
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

  void makeRandomBytes(unsigned char* out, size_t count) const override;
};
} // namespace fizz
