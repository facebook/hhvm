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
  virtual Status makeKeyScheduler(
      std::unique_ptr<KeyScheduler>& ret,
      Error& err,
      CipherSuite cipher) const;

  virtual Status makeKeyDeriver(
      std::unique_ptr<KeyDerivation>& ret,
      Error& err,
      CipherSuite cipher) const;

  virtual Status makeHandshakeContext(
      std::unique_ptr<HandshakeContext>& ret,
      Error& err,
      CipherSuite cipher) const;

  virtual Status makeKeyExchange(
      std::unique_ptr<KeyExchange>& ret,
      Error& err,
      NamedGroup group,
      KeyExchangeRole role) const = 0;

  virtual Status makeHasherFactory(
      const HasherFactoryWithMetadata*& ret,
      Error& err,
      HashFunction digest) const = 0;

  virtual Status makeAead(
      std::unique_ptr<Aead>& ret,
      Error& err,
      CipherSuite cipher) const = 0;

  virtual void makeRandomBytes(unsigned char* out, size_t count) const = 0;

  virtual Status makePeerCert(
      std::unique_ptr<PeerCert>& ret,
      Error& err,
      CertificateEntry certEntry,
      bool leaf) const = 0;

  virtual Status makePeerCertFromTicket(
      std::unique_ptr<Cert>& ret,
      Error& err,
      CertificateEntry certEntry) const {
    std::unique_ptr<PeerCert> peerCert;
    FIZZ_RETURN_ON_ERROR(
        makePeerCert(peerCert, err, std::move(certEntry), true));
    ret = std::move(peerCert);
    return Status::Success;
  }

  /**
   * Should not be overridden *unless* for testing.
   */
  virtual std::shared_ptr<Cert> makeIdentityOnlyCert(std::string ident) const;

  Buf makeRandomIOBuf(size_t size) const;
};
} // namespace fizz
