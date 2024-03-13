/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

#include <fizz/crypto/Utils.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialUtils.h>
#include <fizz/extensions/delegatedcred/SelfDelegatedCredential.h>
#include <fizz/protocol/OpenSSLSelfCertImpl.h>

using namespace folly;

using namespace testing;

namespace fizz {
namespace extensions {
namespace test {

StringPiece kDelegatedCert{
    "-----BEGIN CERTIFICATE-----\n"
    "MIICKzCCAdGgAwIBAgIJAPi2vMRfOVd0MAoGCCqGSM49BAMCMGIxCzAJBgNVBAYT\n"
    "AlhYMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29t\n"
    "cGFueSBMdGQxHjAcBgNVBAMMFXJldnByb3h5LWRlbGVnYXRlZC1lYzAgFw0xOTA5\n"
    "MjMwMjAyMzVaGA8yMTE5MDgzMDAyMDIzNVowYjELMAkGA1UEBhMCWFgxFTATBgNV\n"
    "BAcMDERlZmF1bHQgQ2l0eTEcMBoGA1UECgwTRGVmYXVsdCBDb21wYW55IEx0ZDEe\n"
    "MBwGA1UEAwwVcmV2cHJveHktZGVsZWdhdGVkLWVjMFkwEwYHKoZIzj0CAQYIKoZI\n"
    "zj0DAQcDQgAE7EbZMKds65EYciaSULFH4wZKt/OThiUL4uQW9cybr2HIzK68corO\n"
    "JCeHXOsV3lpYS46b39SBZr1GZprFHH5gHaNuMGwwHQYDVR0OBBYEFMLkRMB4SclK\n"
    "8K8uYMQBaYw0gNP7MB8GA1UdIwQYMBaAFMLkRMB4SclK8K8uYMQBaYw0gNP7MAwG\n"
    "A1UdEwQFMAMBAf8wCwYDVR0PBAQDAgHmMA8GCSsGAQQBgtpLLAQCBQAwCgYIKoZI\n"
    "zj0EAwIDSAAwRQIgB2EWbwWohYziQ2LmY8Qmn8y0WKR6Mbm5aad0rUBvtK4CIQCv\n"
    "0U6Z/gFrVr0Cb2kc7M37KD9z5eeTwkQuGqs5GXF8Ow==\n"
    "-----END CERTIFICATE-----"};

// This is a dummy private key for testing.
// @lint-ignore-every PRIVATEKEY
StringPiece kDelegatedKey{
    "-----BEGIN PRIVATE KEY-----\n"
    "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgOe/v6hxwTP9uA5dE\n"
    "se5CO6ARqeOYXEy1ede9eRCmDduhRANCAATsRtkwp2zrkRhyJpJQsUfjBkq385OG\n"
    "JQvi5Bb1zJuvYcjMrrxyis4kJ4dc6xXeWlhLjpvf1IFmvUZmmsUcfmAd\n"
    "-----END PRIVATE KEY-----\n"};

class SelfDelegatedCredentialTest : public Test {
 public:
  void SetUp() override {
    CryptoUtils::init();
    parentCert_ = std::make_unique<OpenSSLSelfCertImpl<KeyType::P256>>(
        getKey(), getCertVec());
  }

#if FIZZ_OPENSSL_HAS_ED25519
  folly::ssl::EvpPkeyUniquePtr generateEd25519PrivKey() {
    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr);
    EVP_PKEY_keygen_init(pctx);
    EVP_PKEY_keygen(pctx, &pkey);
    EVP_PKEY_CTX_free(pctx);
    return folly::ssl::EvpPkeyUniquePtr(pkey);
  }
#endif

  folly::ssl::EvpPkeyUniquePtr generateDelegatedPrivkey() {
    folly::ssl::EvpPkeyUniquePtr pk(EVP_PKEY_new());
    folly::ssl::EcGroupUniquePtr grp(
        EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1));
    folly::ssl::EcKeyUniquePtr ec(EC_KEY_new());

    if (!pk || !grp || !ec) {
      throw std::runtime_error("failed to create structs");
    }

    EC_GROUP_set_asn1_flag(grp.get(), OPENSSL_EC_NAMED_CURVE);
    EC_GROUP_set_point_conversion_form(
        grp.get(), POINT_CONVERSION_UNCOMPRESSED);

    if (EC_GROUP_check(grp.get(), nullptr) != 1) {
      throw std::runtime_error("group check failed");
    }

    if (EC_KEY_set_group(ec.get(), grp.get()) != 1) {
      throw std::runtime_error("failed to set group");
    }

    if (EC_KEY_generate_key(ec.get()) != 1) {
      throw std::runtime_error("ec generation failed");
    }

    if (EVP_PKEY_set1_EC_KEY(pk.get(), ec.get()) != 1) {
      throw std::runtime_error("private key assignment failed");
    }

    return pk;
  }

  folly::ssl::EvpPkeyUniquePtr generateRSAKey() {
    folly::ssl::EvpPkeyUniquePtr pk(EVP_PKEY_new());
    folly::ssl::BIGNUMUniquePtr bn(BN_new());
    folly::ssl::RsaUniquePtr rsa(RSA_new());

    if (!rsa || !bn || !rsa) {
      throw std::runtime_error("failed to create structs");
    }

    BN_set_word(bn.get(), RSA_F4);
    if (RSA_generate_key_ex(rsa.get(), 1024, bn.get(), nullptr) != 1) {
      throw std::runtime_error("failed to generate rsa key");
    }

    if (EVP_PKEY_set1_RSA(pk.get(), rsa.get()) != 1) {
      throw std::runtime_error("private key assignment failed");
    }

    return pk;
  }

  folly::ssl::EvpPkeyUniquePtr getKey() {
    folly::ssl::BioUniquePtr b(BIO_new_mem_buf(
        reinterpret_cast<const void*>(kDelegatedKey.data()),
        kDelegatedKey.size()));

    if (!b) {
      throw std::runtime_error("failed to create BIO");
    }

    folly::ssl::EvpPkeyUniquePtr key(
        PEM_read_bio_PrivateKey(b.get(), nullptr, nullptr, nullptr));

    if (!key) {
      throw std::runtime_error("Failed to read key");
    }

    return key;
  }

  folly::ssl::X509UniquePtr getCert() {
    folly::ssl::BioUniquePtr b(BIO_new_mem_buf(
        reinterpret_cast<const void*>(kDelegatedCert.data()),
        kDelegatedCert.size()));

    if (!b) {
      throw std::runtime_error("failed to create BIO");
    }

    folly::ssl::X509UniquePtr cert(
        PEM_read_bio_X509(b.get(), nullptr, nullptr, nullptr));

    if (!cert) {
      throw std::runtime_error("Failed to parse cert");
    }

    return cert;
  }

  std::vector<folly::ssl::X509UniquePtr> getCertVec() {
    std::vector<folly::ssl::X509UniquePtr> vec;
    vec.push_back(getCert());
    return vec;
  }

  Buf getPubkeyDer(const folly::ssl::EvpPkeyUniquePtr& pkey) {
    int sz = i2d_PUBKEY(pkey.get(), nullptr);
    if (sz < 0) {
      throw std::runtime_error("error getting pubkey der size");
    }

    auto buf = IOBuf::create(sz);

    auto ptr = reinterpret_cast<unsigned char*>(buf->writableData());
    if (i2d_PUBKEY(pkey.get(), &ptr) < 0) {
      throw std::runtime_error("failed to convert pubkey to der");
    }

    buf->append(sz);

    return buf;
  }

  void updateSignature(DelegatedCredential& cred) {
    auto toSign = DelegatedCredentialUtils::prepareSignatureBuffer(
        cred, folly::ssl::OpenSSLCertUtils::derEncode(*parentCert_->getX509()));
    cred.signature = parentCert_->sign(
        cred.credential_scheme,
        CertificateVerifyContext::DelegatedCredential,
        toSign->coalesce());
  }

  DelegatedCredential makeCredential(const folly::ssl::EvpPkeyUniquePtr& pkey) {
    DelegatedCredential cred;
    cred.valid_time = 0x1234; // This isn't checked by the self credential code
    const auto keyType = CertUtils::getKeyType(pkey);
    const auto sigSchemes = CertUtils::getSigSchemes(keyType);
    cred.expected_verify_scheme = sigSchemes[0];
    if (pkey) {
      cred.public_key = getPubkeyDer(pkey);
    }
    cred.credential_scheme = SignatureScheme::ecdsa_secp256r1_sha256;

    updateSignature(cred);

    return cred;
  }

  std::unique_ptr<OpenSSLSelfCertImpl<KeyType::P256>> parentCert_;
};

TEST_F(SelfDelegatedCredentialTest, TestConstruction) {
  auto dcKey = generateDelegatedPrivkey();
  auto credential = makeCredential(dcKey);
  auto credCert = std::make_unique<SelfDelegatedCredentialImpl<KeyType::P256>>(
      getCertVec(), std::move(dcKey), std::move(credential));
}

#if FIZZ_OPENSSL_HAS_ED25519
TEST_F(SelfDelegatedCredentialTest, TestEd25519DCConstruction) {
  auto dcKey = generateEd25519PrivKey();
  auto credential = makeCredential(dcKey);
  auto credCert =
      std::make_unique<SelfDelegatedCredentialImpl<KeyType::ED25519>>(
          getCertVec(), std::move(dcKey), std::move(credential));
}
#endif

TEST_F(SelfDelegatedCredentialTest, TestConstructionFailureBadSignature) {
  auto dcKey = generateDelegatedPrivkey();
  auto credential = makeCredential(dcKey);

  credential.signature = IOBuf::copyBuffer("hash algorithm party");
  EXPECT_THROW(
      std::make_unique<SelfDelegatedCredentialImpl<KeyType::P256>>(
          getCertVec(), std::move(dcKey), std::move(credential)),
      std::runtime_error);
}

TEST_F(SelfDelegatedCredentialTest, TestConstructionFailureKeyTypeMismatch) {
  auto dcKey = generateRSAKey();
  auto credential = makeCredential(dcKey);

  // RSA key with EC cert should fail
  EXPECT_THROW(
      std::make_unique<SelfDelegatedCredentialImpl<KeyType::P256>>(
          getCertVec(), std::move(dcKey), std::move(credential)),
      std::runtime_error);
}

TEST_F(
    SelfDelegatedCredentialTest,
    TestConstructionFailureIncompatibleVerifyScheme) {
  auto dcKey = generateDelegatedPrivkey();
  auto credential = makeCredential(dcKey);
  // Set incompatible verify scheme
  credential.expected_verify_scheme = SignatureScheme::ecdsa_secp521r1_sha512;
  updateSignature(credential);

  EXPECT_THROW(
      std::make_unique<SelfDelegatedCredentialImpl<KeyType::P256>>(
          getCertVec(), std::move(dcKey), std::move(credential)),
      std::runtime_error);
}

TEST_F(SelfDelegatedCredentialTest, TestSignatureValidity) {
  auto dcKey = generateDelegatedPrivkey();
  auto credential = makeCredential(dcKey);
  auto credCert = std::make_unique<SelfDelegatedCredentialImpl<KeyType::P256>>(
      getCertVec(), std::move(dcKey), std::move(credential));
  Buf toSign = IOBuf::copyBuffer("signme");

  auto certSig = parentCert_->sign(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      toSign->coalesce());
  auto dcSig = credCert->sign(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      toSign->coalesce());

  auto parentPeerCert = CertUtils::makePeerCert(parentCert_->getX509());

  EXPECT_THROW(
      parentPeerCert->verify(
          SignatureScheme::ecdsa_secp256r1_sha256,
          CertificateVerifyContext::Server,
          toSign->coalesce(),
          dcSig->coalesce()),
      std::runtime_error);

  parentPeerCert->verify(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      toSign->coalesce(),
      certSig->coalesce());
}

} // namespace test
} // namespace extensions
} // namespace fizz
