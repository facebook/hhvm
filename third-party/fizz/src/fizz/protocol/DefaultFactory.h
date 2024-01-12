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
 * This class instantiates various objects to facilitate testing.
 */
class DefaultFactory : public Factory {
 public:
  [[nodiscard]] std::unique_ptr<PlaintextReadRecordLayer>
  makePlaintextReadRecordLayer() const override {
    return std::make_unique<PlaintextReadRecordLayer>();
  }

  [[nodiscard]] std::unique_ptr<PlaintextWriteRecordLayer>
  makePlaintextWriteRecordLayer() const override {
    return std::make_unique<PlaintextWriteRecordLayer>();
  }

  [[nodiscard]] std::unique_ptr<EncryptedReadRecordLayer>
  makeEncryptedReadRecordLayer(EncryptionLevel encryptionLevel) const override {
    return std::make_unique<EncryptedReadRecordLayer>(encryptionLevel);
  }

  [[nodiscard]] std::unique_ptr<EncryptedWriteRecordLayer>
  makeEncryptedWriteRecordLayer(
      EncryptionLevel encryptionLevel) const override {
    return std::make_unique<EncryptedWriteRecordLayer>(encryptionLevel);
  }

  [[nodiscard]] std::unique_ptr<KeyScheduler> makeKeyScheduler(
      CipherSuite cipher) const override {
    auto keyDer = makeKeyDeriver(cipher);
    return std::make_unique<KeyScheduler>(std::move(keyDer));
  }

  [[nodiscard]] Random makeRandom() const override {
    return RandomGenerator<Random().size()>().generateRandom();
  }

  [[nodiscard]] uint32_t makeTicketAgeAdd() const override {
    return RandomNumGenerator<uint32_t>().generateRandom();
  }

  [[nodiscard]] std::unique_ptr<folly::IOBuf> makeRandomBytes(
      size_t count) const override {
    return RandomBufGenerator(count).generateRandom();
  }

  [[nodiscard]] std::shared_ptr<Cert> makeIdentityOnlyCert(
      std::string ident) const override {
    return std::make_shared<IdentityCert>(std::move(ident));
  }

  [[nodiscard]] std::string getHkdfPrefix() const override {
    return kHkdfLabelPrefix.str();
  }
};
} // namespace fizz
