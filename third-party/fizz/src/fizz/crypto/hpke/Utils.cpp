/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/hpke/Utils.h>

#include <fizz/crypto/Sha256.h>
#include <fizz/crypto/Sha384.h>
#include <fizz/crypto/Sha512.h>
#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/AESGCM256.h>
#include <fizz/crypto/aead/ChaCha20Poly1305.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/crypto/exchange/ECCurveKeyExchange.h>
#include <fizz/crypto/exchange/X25519.h>

namespace fizz {
namespace hpke {

HpkeSuiteId
generateHpkeSuiteId(NamedGroup group, HashFunction hash, CipherSuite suite) {
  return generateHpkeSuiteId(getKEMId(group), getKDFId(hash), getAeadId(suite));
}

HpkeSuiteId generateHpkeSuiteId(KEMId kem, KDFId kdf, AeadId aead) {
  std::unique_ptr<folly::IOBuf> suiteId = folly::IOBuf::copyBuffer("HPKE");
  folly::io::Appender appender(suiteId.get(), 6);
  detail::write(kem, appender);
  detail::write(kdf, appender);
  detail::write(aead, appender);
  return suiteId;
}

KEMId getKEMId(NamedGroup group) {
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
      throw std::runtime_error("ke: not implemented");
  }
}

KDFId getKDFId(HashFunction hash) {
  switch (hash) {
    case HashFunction::Sha256:
      return KDFId::Sha256;
    case HashFunction::Sha384:
      return KDFId::Sha384;
    case HashFunction::Sha512:
      return KDFId::Sha512;
    default:
      throw std::runtime_error("kdf: not implemented");
  }
}

AeadId getAeadId(CipherSuite suite) {
  switch (suite) {
    case CipherSuite::TLS_AES_128_GCM_SHA256:
      return AeadId::TLS_AES_128_GCM_SHA256;
    case CipherSuite::TLS_AES_256_GCM_SHA384:
      return AeadId::TLS_AES_256_GCM_SHA384;
    case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
      return AeadId::TLS_CHACHA20_POLY1305_SHA256;
    default:
      throw std::runtime_error("ciphersuite: not implemented");
  }
}

NamedGroup getKexGroup(KEMId kemId) {
  switch (kemId) {
    case KEMId::secp256r1:
      return NamedGroup::secp256r1;
    case KEMId::secp384r1:
      return NamedGroup::secp384r1;
    case KEMId::secp521r1:
      return NamedGroup::secp521r1;
    case KEMId::x25519:
      return NamedGroup::x25519;
    default:
      throw std::runtime_error("can't make key exchange: not implemented");
  }
}

HashFunction getHashFunctionForKEM(KEMId kemId) {
  switch (kemId) {
    case KEMId::secp256r1:
      return HashFunction::Sha256;
    case KEMId::secp384r1:
      return HashFunction::Sha384;
    case KEMId::secp521r1:
      return HashFunction::Sha512;
    case KEMId::x25519:
      return HashFunction::Sha256;
    default:
      throw std::runtime_error("can't make KEM hash function: not implemented");
  }
}

HashFunction getHashFunction(KDFId kdfId) {
  switch (kdfId) {
    case KDFId::Sha256:
      return HashFunction::Sha256;
    case KDFId::Sha384:
      return HashFunction::Sha384;
    case KDFId::Sha512:
      return HashFunction::Sha512;
    default:
      throw std::runtime_error("kdf: not implemented");
  }
}

CipherSuite getCipherSuite(AeadId aeadId) {
  switch (aeadId) {
    case AeadId::TLS_AES_128_GCM_SHA256:
      return CipherSuite::TLS_AES_128_GCM_SHA256;
    case AeadId::TLS_AES_256_GCM_SHA384:
      return CipherSuite::TLS_AES_256_GCM_SHA384;
    case AeadId::TLS_CHACHA20_POLY1305_SHA256:
      return CipherSuite::TLS_CHACHA20_POLY1305_SHA256;
    default:
      throw std::runtime_error("ciphersuite: not implemented");
  }
}

std::unique_ptr<Hkdf> makeHpkeHkdf(
    std::unique_ptr<folly::IOBuf> prefix,
    KDFId kdfId) {
  switch (kdfId) {
    case KDFId::Sha256:
      return std::make_unique<Hkdf>(
          std::move(prefix),
          std::make_unique<HkdfImpl>(HkdfImpl::create<Sha256>()));
    case KDFId::Sha384:
      return std::make_unique<Hkdf>(
          std::move(prefix),
          std::make_unique<HkdfImpl>(HkdfImpl::create<Sha384>()));
    case KDFId::Sha512:
      return std::make_unique<Hkdf>(
          std::move(prefix),
          std::make_unique<HkdfImpl>(HkdfImpl::create<Sha512>()));
    default:
      throw std::runtime_error("hkdf: not implemented");
  }
}

std::unique_ptr<KeyExchange> makeKeyExchange(KEMId kemId) {
  switch (kemId) {
    case KEMId::secp256r1:
      return std::make_unique<OpenSSLECKeyExchange<P256>>();
    case KEMId::secp384r1:
      return std::make_unique<OpenSSLECKeyExchange<P384>>();
    case KEMId::secp521r1:
      return std::make_unique<OpenSSLECKeyExchange<P521>>();
    case KEMId::x25519:
      return std::make_unique<X25519KeyExchange>();
    default:
      throw std::runtime_error("can't make key exchange: not implemented");
  }
}

size_t nenc(KEMId kemId) {
  // Refer to Table 2 in 7.1.  Key Encapsulation Mechanisms (KEMs)
  switch (kemId) {
    case KEMId::secp256r1:
      return 65;
    case KEMId::secp384r1:
      return 97;
    case KEMId::secp521r1:
      return 133;
    case KEMId::x25519:
      return 32;
    default:
      throw std::runtime_error("unknown or invalid kem");
  }
}

std::unique_ptr<Aead> makeCipher(AeadId aeadId) {
  switch (aeadId) {
    case AeadId::TLS_CHACHA20_POLY1305_SHA256:
      return OpenSSLEVPCipher::makeCipher<ChaCha20Poly1305>();
    case AeadId::TLS_AES_128_GCM_SHA256:
      return OpenSSLEVPCipher::makeCipher<AESGCM128>();
    case AeadId::TLS_AES_256_GCM_SHA384:
      return OpenSSLEVPCipher::makeCipher<AESGCM256>();
    default:
      throw std::runtime_error("can't make aead: not implemented");
  }
}

} // namespace hpke
} // namespace fizz
