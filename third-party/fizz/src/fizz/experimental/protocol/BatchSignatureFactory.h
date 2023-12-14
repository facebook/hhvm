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

  std::unique_ptr<KeyScheduler> makeKeyScheduler(
      CipherSuite cipher) const override {
    return original_->makeKeyScheduler(cipher);
  }

  std::unique_ptr<KeyDerivation> makeKeyDeriver(
      CipherSuite cipher) const override {
    return original_->makeKeyDeriver(cipher);
  }

  std::unique_ptr<HandshakeContext> makeHandshakeContext(
      CipherSuite cipher) const override {
    return original_->makeHandshakeContext(cipher);
  }

  std::unique_ptr<KeyExchange> makeKeyExchange(
      NamedGroup group,
      KeyExchangeMode mode) const override {
    return original_->makeKeyExchange(group, mode);
  }

  std::unique_ptr<Aead> makeAead(CipherSuite cipher) const override {
    return original_->makeAead(cipher);
  }

  Random makeRandom() const override {
    return original_->makeRandom();
  }

  uint32_t makeTicketAgeAdd() const override {
    return original_->makeTicketAgeAdd();
  }

  std::unique_ptr<folly::IOBuf> makeRandomBytes(size_t count) const override {
    return original_->makeRandomBytes(count);
  }

  /**
   * Make BatchSigPeerCert instead of PeerCert.
   *
   * Since batch signature is only for verifying the leaf of the certificate
   * chain, so BatchSignaturePeerCert is turned only when @param leaf is true.
   */
  std::shared_ptr<PeerCert> makePeerCert(CertificateEntry certEntry, bool leaf)
      const override {
    if (leaf) {
      return std::make_shared<BatchSignaturePeerCert>(
          original_->makePeerCert(std::move(certEntry), leaf));
    }
    return original_->makePeerCert(std::move(certEntry), leaf);
  }

  std::shared_ptr<Cert> makeIdentityOnlyCert(std::string ident) const override {
    return original_->makeIdentityOnlyCert(std::move(ident));
  }

  std::string getHkdfPrefix() const override {
    return original_->getHkdfPrefix();
  }

 private:
  std::shared_ptr<Factory> original_;
};
} // namespace fizz
