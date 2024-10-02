/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>

#include <fizz/crypto/Crypto.h>
#include <fizz/crypto/Hasher.h>
#include <fizz/crypto/KeyDerivation.h>
#include <fizz/crypto/RandomGenerator.h>
#include <fizz/crypto/aead/Aead.h>
#include <fizz/crypto/exchange/KeyExchange.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/HandshakeContext.h>
#include <fizz/protocol/KeyScheduler.h>
#include <fizz/protocol/Types.h>
#include <fizz/record/EncryptedRecordLayer.h>
#include <fizz/record/PlaintextRecordLayer.h>
#include <fizz/record/Types.h>

namespace fizz {

class PeerCert;

/**
 * `fizz::Factory` creates objects that are used by other components of Fizz.
 *
 * Fizz does not directly handle certain parts of TLS (such as cryptographic
 * algorithms, X509, etc.). These portions are considered "primitives".
 * Primitives are provided by various backends (which are typically third
 * party libraries, such as OpenSSL).
 *
 * The role of the factory is to compose primitives from one or more backends
 * in order to implement other Fizz interfaces.
 */
class Factory {
 public:
  virtual ~Factory();
  /**
   * Should not be overridden *unless* for testing.
   */
  virtual std::unique_ptr<PlaintextReadRecordLayer>
  makePlaintextReadRecordLayer() const;

  /**
   * Should not be overridden *unless* for testing.
   */
  virtual std::unique_ptr<PlaintextWriteRecordLayer>
  makePlaintextWriteRecordLayer() const;

  /**
   * Should not be overridden *unless* for testing.
   */
  virtual std::unique_ptr<EncryptedReadRecordLayer>
  makeEncryptedReadRecordLayer(EncryptionLevel encryptionLevel) const;

  /**
   * Should not be overridden *unless* for testing.
   */
  virtual std::unique_ptr<EncryptedWriteRecordLayer>
  makeEncryptedWriteRecordLayer(EncryptionLevel encryptionLevel) const;

  /**
   * Should not be overridden *unless* for testing.
   */
  virtual std::unique_ptr<KeyScheduler> makeKeyScheduler(
      CipherSuite cipher) const;

  virtual std::unique_ptr<KeyDerivation> makeKeyDeriver(
      CipherSuite cipher) const;

  virtual std::unique_ptr<HandshakeContext> makeHandshakeContext(
      CipherSuite cipher) const;

  virtual std::unique_ptr<KeyExchange> makeKeyExchange(
      NamedGroup group,
      KeyExchangeRole role) const = 0;

  virtual const HasherFactoryWithMetadata* makeHasherFactory(
      HashFunction digest) const = 0;

  virtual std::unique_ptr<Aead> makeAead(CipherSuite cipher) const = 0;

  /**
   * Should not be overridden *unless* for testing.
   *
   * TODO: Deprecate this.
   */
  virtual Random makeRandom() const;

  /**
   * Should not be overridden *unless* for testing.
   *
   * TODO: Deprecate this.
   */
  virtual uint32_t makeTicketAgeAdd() const;

  /**
   * Should not be overridden *unless* for testing.
   *
   * TODO: Deprecate this.
   */
  virtual std::unique_ptr<folly::IOBuf> makeRandomBytes(size_t count) const;

  virtual void makeRandomBytes(unsigned char* out, size_t count) const = 0;

  virtual std::unique_ptr<PeerCert> makePeerCert(
      CertificateEntry certEntry,
      bool /*leaf*/) const = 0;

  /**
   * Should not be overridden *unless* for testing.
   */
  virtual std::shared_ptr<Cert> makeIdentityOnlyCert(std::string ident) const;
};
} // namespace fizz
