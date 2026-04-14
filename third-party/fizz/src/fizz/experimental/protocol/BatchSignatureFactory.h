/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/experimental/client/BatchSignaturePeerCert.h>
#include <fizz/protocol/Factory.h>

namespace fizz {

/**
 * A decorator class for an exisiting DefaultFactory to generate PeerCert that
 * supports batch signature schemes.
 */
class BatchSignatureFactory : public Factory {
 public:
  static std::unique_ptr<Factory> makeBatchSignatureFactory(
      std::shared_ptr<Factory> original) {
    return std::make_unique<BatchSignatureFactory>(original);
  }

  BatchSignatureFactory(std::shared_ptr<Factory> original)
      : original_(original) {}

  std::unique_ptr<PlaintextReadRecordLayer> makePlaintextReadRecordLayer()
      const override {
    return original_->makePlaintextReadRecordLayer();
  }

  std::unique_ptr<PlaintextWriteRecordLayer> makePlaintextWriteRecordLayer()
      const override {
    return original_->makePlaintextWriteRecordLayer();
  }

  std::unique_ptr<EncryptedReadRecordLayer> makeEncryptedReadRecordLayer(
      EncryptionLevel encryptionLevel) const override {
    return original_->makeEncryptedReadRecordLayer(encryptionLevel);
  }

  std::unique_ptr<EncryptedWriteRecordLayer> makeEncryptedWriteRecordLayer(
      EncryptionLevel encryptionLevel) const override {
    return original_->makeEncryptedWriteRecordLayer(encryptionLevel);
  }

  Status makeKeyScheduler(
      std::unique_ptr<KeyScheduler>& ret,
      Error& err,
      CipherSuite cipher) const override {
    return original_->makeKeyScheduler(ret, err, cipher);
  }

  Status makeKeyDeriver(
      std::unique_ptr<KeyDerivation>& ret,
      Error& err,
      CipherSuite cipher) const override {
    return original_->makeKeyDeriver(ret, err, cipher);
  }

  Status makeHasherFactory(
      const HasherFactoryWithMetadata*& ret,
      Error& err,
      HashFunction digest) const override {
    return original_->makeHasherFactory(ret, err, digest);
  }

  Status makeHandshakeContext(
      std::unique_ptr<HandshakeContext>& ret,
      Error& err,
      CipherSuite cipher) const override {
    return original_->makeHandshakeContext(ret, err, cipher);
  }

  Status makeKeyExchange(
      std::unique_ptr<KeyExchange>& ret,
      Error& err,
      NamedGroup group,
      KeyExchangeRole role) const override {
    return original_->makeKeyExchange(ret, err, group, role);
  }

  Status makeAead(std::unique_ptr<Aead>& ret, Error& err, CipherSuite cipher)
      const override {
    return original_->makeAead(ret, err, cipher);
  }

  void makeRandomBytes(unsigned char* out, size_t count) const override {
    return original_->makeRandomBytes(out, count);
  }

  /**
   * Make BatchSigPeerCert instead of PeerCert.
   *
   * Since batch signature is only for verifying the leaf of the certificate
   * chain, so BatchSignaturePeerCert is turned only when @param leaf is true.
   */
  std::unique_ptr<PeerCert> makePeerCert(CertificateEntry certEntry, bool leaf)
      const override {
    if (leaf) {
      return std::make_unique<BatchSignaturePeerCert>(
          original_->makePeerCert(std::move(certEntry), leaf));
    }
    return original_->makePeerCert(std::move(certEntry), leaf);
  }

  std::shared_ptr<Cert> makeIdentityOnlyCert(std::string ident) const override {
    return original_->makeIdentityOnlyCert(std::move(ident));
  }

 private:
  std::shared_ptr<Factory> original_;
};
} // namespace fizz
