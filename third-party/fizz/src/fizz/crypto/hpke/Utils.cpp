/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/hpke/Utils.h>

namespace fizz {
namespace hpke {

HpkeSuiteId
generateHpkeSuiteId(NamedGroup group, HashFunction hash, CipherSuite suite) {
  KEMId kemId;
  Error err;
  FIZZ_THROW_ON_ERROR(getKEMId(kemId, err, group), err);
  KDFId kdfId;
  FIZZ_THROW_ON_ERROR(getKDFId(kdfId, err, hash), err);
  AeadId aeadId;
  FIZZ_THROW_ON_ERROR(getAeadId(aeadId, err, suite), err);
  return generateHpkeSuiteId(kemId, kdfId, aeadId);
}

HpkeSuiteId generateHpkeSuiteId(KEMId kem, KDFId kdf, AeadId aead) {
  std::unique_ptr<folly::IOBuf> suiteId = folly::IOBuf::copyBuffer("HPKE");
  folly::io::Appender appender(suiteId.get(), 6);
  Error err;
  FIZZ_THROW_ON_ERROR(detail::write(err, kem, appender), err);
  FIZZ_THROW_ON_ERROR(detail::write(err, kdf, appender), err);
  FIZZ_THROW_ON_ERROR(detail::write(err, aead, appender), err);
  return suiteId;
}

folly::Optional<KEMId> tryGetKEMId(NamedGroup group) {
  switch (group) {
    case NamedGroup::secp256r1:
      return KEMId::secp256r1;
    case NamedGroup::secp384r1:
      return KEMId::secp384r1;
    case NamedGroup::secp521r1:
      return KEMId::secp521r1;
    case NamedGroup::x25519:
      return KEMId::x25519;
    default:
      return folly::none;
  }
}

Status getKEMId(KEMId& ret, Error& err, NamedGroup group) {
  const auto kemId = tryGetKEMId(group);
  if (!kemId.has_value()) {
    return err.error("ke: not implemented");
  }
  ret = *kemId;
  return Status::Success;
}

Status getKDFId(KDFId& ret, Error& err, HashFunction hash) {
  switch (hash) {
    case HashFunction::Sha256:
      ret = KDFId::Sha256;
      return Status::Success;
    case HashFunction::Sha384:
      ret = KDFId::Sha384;
      return Status::Success;
    case HashFunction::Sha512:
      ret = KDFId::Sha512;
      return Status::Success;
    default:
      return err.error("kdf: not implemented");
  }
}

folly::Optional<AeadId> tryGetAeadId(CipherSuite suite) {
  switch (suite) {
    case CipherSuite::TLS_AES_128_GCM_SHA256:
      return AeadId::TLS_AES_128_GCM_SHA256;
    case CipherSuite::TLS_AES_256_GCM_SHA384:
      return AeadId::TLS_AES_256_GCM_SHA384;
    case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
      return AeadId::TLS_CHACHA20_POLY1305_SHA256;
    default:
      return folly::none;
  }
}

Status getAeadId(AeadId& ret, Error& err, CipherSuite suite) {
  const auto aeadId = tryGetAeadId(suite);
  if (!aeadId.has_value()) {
    return err.error("ciphersuite: not implemented");
  }
  ret = *aeadId;
  return Status::Success;
}

Status getKexGroup(NamedGroup& ret, Error& err, KEMId kemId) {
  switch (kemId) {
    case KEMId::secp256r1:
      ret = NamedGroup::secp256r1;
      return Status::Success;
    case KEMId::secp384r1:
      ret = NamedGroup::secp384r1;
      return Status::Success;
    case KEMId::secp521r1:
      ret = NamedGroup::secp521r1;
      return Status::Success;
    case KEMId::x25519:
      ret = NamedGroup::x25519;
      return Status::Success;
    default:
      return err.error("can't make key exchange: not implemented");
  }
}

Status getHashFunctionForKEM(HashFunction& ret, Error& err, KEMId kemId) {
  switch (kemId) {
    case KEMId::secp256r1:
      ret = HashFunction::Sha256;
      return Status::Success;
    case KEMId::secp384r1:
      ret = HashFunction::Sha384;
      return Status::Success;
    case KEMId::secp521r1:
      ret = HashFunction::Sha512;
      return Status::Success;
    case KEMId::x25519:
      ret = HashFunction::Sha256;
      return Status::Success;
    default:
      return err.error("can't make KEM hash function: not implemented");
  }
}

Status getHashFunction(HashFunction& ret, Error& err, KDFId kdfId) {
  switch (kdfId) {
    case KDFId::Sha256:
      ret = HashFunction::Sha256;
      return Status::Success;
    case KDFId::Sha384:
      ret = HashFunction::Sha384;
      return Status::Success;
    case KDFId::Sha512:
      ret = HashFunction::Sha512;
      return Status::Success;
    default:
      return err.error("kdf: not implemented");
  }
}

Status getCipherSuite(CipherSuite& ret, Error& err, AeadId aeadId) {
  switch (aeadId) {
    case AeadId::TLS_AES_128_GCM_SHA256:
      ret = CipherSuite::TLS_AES_128_GCM_SHA256;
      return Status::Success;
    case AeadId::TLS_AES_256_GCM_SHA384:
      ret = CipherSuite::TLS_AES_256_GCM_SHA384;
      return Status::Success;
    case AeadId::TLS_CHACHA20_POLY1305_SHA256:
      ret = CipherSuite::TLS_CHACHA20_POLY1305_SHA256;
      return Status::Success;
    default:
      return err.error("ciphersuite: not implemented");
  }
}

Status nenc(size_t& ret, Error& err, KEMId kemId) {
  // Refer to Table 2 in 7.1.  Key Encapsulation Mechanisms (KEMs)
  switch (kemId) {
    case KEMId::secp256r1:
      ret = 65;
      return Status::Success;
    case KEMId::secp384r1:
      ret = 97;
      return Status::Success;
    case KEMId::secp521r1:
      ret = 133;
      return Status::Success;
    case KEMId::x25519:
      ret = 32;
      return Status::Success;
    default:
      return err.error("unknown or invalid kem");
  }
}
} // namespace hpke
} // namespace fizz
