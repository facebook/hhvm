/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/test/TestUtil.h>
#include <fizz/fizz-config.h>

#if FIZZ_BUILD_AEGIS
#include <fizz/crypto/aead/AEGISCipher.h>
#endif

#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/AESGCM256.h>
#include <fizz/crypto/aead/AESOCB128.h>
#include <fizz/crypto/aead/ChaCha20Poly1305.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <folly/String.h>
#include <folly/ssl/OpenSSLCertUtils.h>
#include <sodium/randombytes.h>

using namespace folly;
using namespace folly::ssl;

namespace fizz {
namespace test {

EvpPkeyUniquePtr getPrivateKey(StringPiece key) {
  BioUniquePtr bio(BIO_new(BIO_s_mem()));
  CHECK(bio);
  CHECK_EQ(BIO_write(bio.get(), key.data(), key.size()), key.size());
  EvpPkeyUniquePtr pkey(
      PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr));
  CHECK(pkey);
  return pkey;
}

EvpPkeyUniquePtr getPublicKey(StringPiece key) {
  BioUniquePtr bio(BIO_new(BIO_s_mem()));
  CHECK(bio);
  CHECK_EQ(BIO_write(bio.get(), key.data(), key.size()), key.size());
  EvpPkeyUniquePtr pkey(
      PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr));
  CHECK(pkey);
  return pkey;
}

// Converts the hex encoded string to an IOBuf.
std::unique_ptr<folly::IOBuf> toIOBuf(folly::StringPiece hexData) {
  std::string out;
  CHECK(folly::unhexlify(hexData, out));
  return folly::IOBuf::copyBuffer(out);
}

folly::ssl::X509UniquePtr getCert(folly::StringPiece cert) {
  BioUniquePtr bio(BIO_new(BIO_s_mem()));
  CHECK(bio);
  CHECK_EQ(BIO_write(bio.get(), cert.data(), cert.size()), cert.size());
  X509UniquePtr x509(PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr));
  CHECK(x509);
  return x509;
}

std::unique_ptr<folly::IOBuf> getCertData(folly::StringPiece cert) {
  return OpenSSLCertUtils::derEncode(*getCert(cert));
}

static struct randombytes_implementation mockRandom = {
    []() { return "test"; }, // implementation_name
    []() { return (uint32_t)0x44444444; }, // random
    nullptr, // stir
    nullptr, // uniform
    [](void* const buf, const size_t size) { memset(buf, 0x44, size); }, // buf
    nullptr}; // close

void useMockRandom() {
  randombytes_set_implementation(&mockRandom);
}

std::unique_ptr<Aead> getCipher(CipherSuite suite) {
  std::unique_ptr<Aead> cipher;
  switch (suite) {
    case CipherSuite::TLS_AES_128_GCM_SHA256:
      cipher = OpenSSLEVPCipher::makeCipher<AESGCM128>();
      break;
    case CipherSuite::TLS_AES_256_GCM_SHA384:
      cipher = OpenSSLEVPCipher::makeCipher<AESGCM256>();
      break;
    case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
      cipher = OpenSSLEVPCipher::makeCipher<ChaCha20Poly1305>();
      break;
    case CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL:
      cipher = OpenSSLEVPCipher::makeCipher<AESOCB128>();
      break;
#if FIZZ_BUILD_AEGIS
    case CipherSuite::TLS_AEGIS_128L_SHA256:
      cipher = AEGISCipher::make128L();
      break;
    case CipherSuite::TLS_AEGIS_256_SHA512:
      cipher = AEGISCipher::make256();
      break;
#endif
    default:
      throw std::runtime_error("Invalid cipher");
  }
  constexpr size_t kHeadroom = 10;
  cipher->setEncryptedBufferHeadroom(kHeadroom);
  return cipher;
}

} // namespace test
} // namespace fizz
