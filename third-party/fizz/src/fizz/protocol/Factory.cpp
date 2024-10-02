/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <fizz/protocol/Factory.h>

namespace fizz {
Factory::~Factory() = default;

std::unique_ptr<PlaintextReadRecordLayer>
Factory::makePlaintextReadRecordLayer() const {
  return std::make_unique<PlaintextReadRecordLayer>();
}
std::unique_ptr<PlaintextWriteRecordLayer>
Factory::makePlaintextWriteRecordLayer() const {
  return std::make_unique<PlaintextWriteRecordLayer>();
}

std::unique_ptr<EncryptedReadRecordLayer> Factory::makeEncryptedReadRecordLayer(
    EncryptionLevel encryptionLevel) const {
  return std::make_unique<EncryptedReadRecordLayer>(encryptionLevel);
}

std::unique_ptr<EncryptedWriteRecordLayer>
Factory::makeEncryptedWriteRecordLayer(EncryptionLevel encryptionLevel) const {
  return std::make_unique<EncryptedWriteRecordLayer>(encryptionLevel);
}

std::unique_ptr<KeyScheduler> Factory::makeKeyScheduler(
    CipherSuite cipher) const {
  auto keyDer = makeKeyDeriver(cipher);
  return std::make_unique<KeyScheduler>(std::move(keyDer));
}

// TODO: This should not belong as part of the base Factory; a concrete
// factory should provide the random primitive.
uint32_t Factory::makeTicketAgeAdd() const {
  return RandomNumGenerator<uint32_t>().generateRandom();
}

// TODO: This should not belong as part of the base Factory; a concrete
// factory should provide the random primitive.
std::unique_ptr<folly::IOBuf> Factory::makeRandomBytes(size_t count) const {
  return RandomBufGenerator(count).generateRandom();
}

std::unique_ptr<HandshakeContext> Factory::makeHandshakeContext(
    CipherSuite cipher) const {
  auto hasherFactory = makeHasherFactory(getHashFunction(cipher));
  return std::make_unique<HandshakeContextImpl>(hasherFactory);
}
//

std::unique_ptr<KeyDerivation> Factory::makeKeyDeriver(
    CipherSuite cipher) const {
  auto hasher = makeHasherFactory(getHashFunction(cipher));
  return std::make_unique<KeyDerivationImpl>(hasher);
}

std::shared_ptr<Cert> Factory::makeIdentityOnlyCert(std::string ident) const {
  return std::make_shared<IdentityCert>(std::move(ident));
}
} // namespace fizz
