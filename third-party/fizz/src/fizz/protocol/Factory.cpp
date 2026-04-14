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

Status Factory::makeKeyScheduler(
    std::unique_ptr<KeyScheduler>& ret,
    Error& err,
    CipherSuite cipher) const {
  std::unique_ptr<KeyDerivation> keyDer;
  FIZZ_RETURN_ON_ERROR(makeKeyDeriver(keyDer, err, cipher));
  ret = std::make_unique<KeyScheduler>(std::move(keyDer));
  return Status::Success;
}

Status Factory::makeHandshakeContext(
    std::unique_ptr<HandshakeContext>& ret,
    Error& err,
    CipherSuite cipher) const {
  HashFunction hash;
  const HasherFactoryWithMetadata* hasherFactory = nullptr;
  FIZZ_RETURN_ON_ERROR(getHashFunction(hash, err, cipher));
  FIZZ_RETURN_ON_ERROR(makeHasherFactory(hasherFactory, err, hash));
  ret = std::make_unique<HandshakeContextImpl>(hasherFactory);
  return Status::Success;
}

Status Factory::makeKeyDeriver(
    std::unique_ptr<KeyDerivation>& ret,
    Error& err,
    CipherSuite cipher) const {
  HashFunction hash;
  const HasherFactoryWithMetadata* hasherFactory = nullptr;
  FIZZ_RETURN_ON_ERROR(getHashFunction(hash, err, cipher));
  FIZZ_RETURN_ON_ERROR(makeHasherFactory(hasherFactory, err, hash));
  ret = std::make_unique<KeyDerivationImpl>(hasherFactory);
  return Status::Success;
}

std::shared_ptr<Cert> Factory::makeIdentityOnlyCert(std::string ident) const {
  return std::make_shared<IdentityCert>(std::move(ident));
}

Buf Factory::makeRandomIOBuf(size_t size) const {
  auto buf = folly::IOBuf::create(size);
  if (size > 0) {
    makeRandomBytes(buf->writableData(), size);
    buf->append(size);
  }
  return buf;
}
} // namespace fizz
