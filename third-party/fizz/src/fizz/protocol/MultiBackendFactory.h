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
  Status makeKeyExchange(
      std::unique_ptr<KeyExchange>& ret,
      Error& err,
      NamedGroup group,
      KeyExchangeRole role) const override;

  Status makeAead(std::unique_ptr<Aead>& ret, Error& err, CipherSuite cipher)
      const override;

  Status makeHasherFactory(
      const HasherFactoryWithMetadata*& ret,
      Error& err,
      HashFunction digest) const override;

  Status makePeerCert(
      std::unique_ptr<PeerCert>& ret,
      Error& err,
      CertificateEntry certEntry,
      bool leaf) const override;

  void makeRandomBytes(unsigned char* out, size_t count) const override;
};
} // namespace fizz
