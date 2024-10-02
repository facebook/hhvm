/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/fizz-config.h>

#include <fizz/protocol/MultiBackendFactory.h>

#include <fizz/backend/libaegis/LibAEGIS.h>
#include <fizz/backend/liboqs/LibOQS.h>
#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/crypto/Hkdf.h>
#include <fizz/crypto/exchange/HybridKeyExchange.h>

#include <sodium.h>

namespace fizz {

std::unique_ptr<KeyExchange> MultiBackendFactory::makeKeyExchange(
    NamedGroup group,
    KeyExchangeRole role) const {
  (void)role;
  switch (group) {
    case NamedGroup::secp256r1:
      return fizz::openssl::makeKeyExchange<fizz::P256>();
    case NamedGroup::secp384r1:
      return fizz::openssl::makeKeyExchange<fizz::P384>();
    case NamedGroup::secp521r1:
      return fizz::openssl::makeKeyExchange<fizz::P521>();
    case NamedGroup::x25519:
      return std::make_unique<X25519KeyExchange>();
#if FIZZ_HAVE_OQS
    case NamedGroup::x25519_kyber512:
    case NamedGroup::x25519_kyber512_experimental:
      return std::make_unique<HybridKeyExchange>(
          std::make_unique<X25519KeyExchange>(),
          fizz::liboqs::makeKeyExchange<Kyber512>(role));
    case NamedGroup::secp256r1_kyber512:
      return std::make_unique<HybridKeyExchange>(
          fizz::openssl::makeKeyExchange<fizz::P256>(),
          fizz::liboqs::makeKeyExchange<Kyber512>(role));
    case NamedGroup::kyber512:
      return fizz::liboqs::makeKeyExchange<Kyber512>(role);
    case NamedGroup::x25519_kyber768_draft00:
    case NamedGroup::x25519_kyber768_experimental:
      return std::make_unique<HybridKeyExchange>(
          std::make_unique<X25519KeyExchange>(),
          fizz::liboqs::makeKeyExchange<Kyber768>(role));
    case NamedGroup::secp256r1_kyber768_draft00:
      return std::make_unique<HybridKeyExchange>(
          fizz::openssl::makeKeyExchange<fizz::P256>(),
          fizz::liboqs::makeKeyExchange<Kyber768>(role));
    case NamedGroup::secp384r1_kyber768:
      return std::make_unique<HybridKeyExchange>(
          fizz::openssl::makeKeyExchange<fizz::P384>(),
          fizz::liboqs::makeKeyExchange<Kyber768>(role));
#endif
    default:
      throw std::runtime_error("ke: not implemented");
  }
}

std::unique_ptr<Aead> MultiBackendFactory::makeAead(CipherSuite cipher) const {
  switch (cipher) {
    case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
      return openssl::OpenSSLEVPCipher::makeCipher<fizz::ChaCha20Poly1305>();
    case CipherSuite::TLS_AES_128_GCM_SHA256:
      return openssl::OpenSSLEVPCipher::makeCipher<fizz::AESGCM128>();
    case CipherSuite::TLS_AES_256_GCM_SHA384:
      return openssl::OpenSSLEVPCipher::makeCipher<fizz::AESGCM256>();
    case CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL:
      return openssl::OpenSSLEVPCipher::makeCipher<fizz::AESOCB128>();
#if FIZZ_HAVE_LIBAEGIS
    case CipherSuite::TLS_AEGIS_256_SHA512:
      return libaegis::makeCipher<fizz::AEGIS256>();
    case CipherSuite::TLS_AEGIS_128L_SHA256:
      return libaegis::makeCipher<fizz::AEGIS128L>();
#endif
    default:
      throw std::runtime_error("aead: not implemented");
  }
}

const HasherFactoryWithMetadata* MultiBackendFactory::makeHasherFactory(
    HashFunction digest) const {
  switch (digest) {
    case HashFunction::Sha256:
      return openssl::hasherFactory<fizz::Sha256>();
    case HashFunction::Sha384:
      return openssl::hasherFactory<fizz::Sha384>();
    case HashFunction::Sha512:
      return openssl::hasherFactory<fizz::Sha512>();
    default:
      throw std::runtime_error("makeHasher: not implemented");
  }
}

void MultiBackendFactory::makeRandomBytes(unsigned char* out, size_t count)
    const {
  randombytes_buf(out, count);
}

std::unique_ptr<PeerCert> MultiBackendFactory::makePeerCert(
    CertificateEntry certEntry,
    bool /*leaf*/) const {
  return openssl::CertUtils::makePeerCert(std::move(certEntry.cert_data));
}

} // namespace fizz
