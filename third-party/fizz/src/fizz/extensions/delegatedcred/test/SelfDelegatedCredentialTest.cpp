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

#include <fizz/backend/openssl/certificate/OpenSSLSelfCertImpl.h>
#include <fizz/crypto/Utils.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialUtils.h>
#include <fizz/extensions/delegatedcred/SelfDelegatedCredential.h>
#include <fizz/extensions/delegatedcred/test/TestData.h>

using namespace folly;

using namespace testing;

namespace fizz {
namespace extensions {
namespace test {

class SelfDelegatedCredentialTest : public Test {
 public:
  void SetUp() override {
    Error err;
    EXPECT_EQ(CryptoUtils::init(err), Status::Success);
    EXPECT_EQ(
        openssl::OpenSSLSelfCertImpl<openssl::KeyType::P256>::create(
            parentCert_, err, getKey(), getCertVec()),
        Status::Success);
  }

  folly::ssl::EvpPkeyUniquePtr generateEd25519PrivKey() {
    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr);
    EVP_PKEY_keygen_init(pctx);
    EVP_PKEY_keygen(pctx, &pkey);
    EVP_PKEY_CTX_free(pctx);
    return folly::ssl::EvpPkeyUniquePtr(pkey);
  }

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
        reinterpret_cast<const void*>(kP256CredCertKey.data()),
        kP256CredCertKey.size()));

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
        reinterpret_cast<const void*>(kP256CredCert.data()),
        kP256CredCert.size()));

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
    Buf toSign;
    Error err;
    EXPECT_EQ(
        DelegatedCredentialUtils::prepareSignatureBuffer(
            toSign,
            err,
            cred,
            folly::ssl::OpenSSLCertUtils::derEncode(*parentCert_->getX509())),
        Status::Success);
    EXPECT_EQ(
        parentCert_->sign(
            cred.signature,
            err,
            cred.credential_scheme,
            CertificateVerifyContext::ServerDelegatedCredential,
            toSign->coalesce()),
        Status::Success);
  }

  DelegatedCredential makeCredential(const folly::ssl::EvpPkeyUniquePtr& pkey) {
    DelegatedCredential cred;
    cred.valid_time = 0x1234; // This isn't checked by the self credential code
    openssl::KeyType keyType;
    Error err;
    EXPECT_EQ(
        openssl::CertUtils::getKeyType(keyType, err, pkey), Status::Success);
    std::vector<SignatureScheme> sigSchemes;
    EXPECT_EQ(
        openssl::CertUtils::getSigSchemes(sigSchemes, err, keyType),
        Status::Success);
    cred.expected_verify_scheme = sigSchemes[0];
    if (pkey) {
      cred.public_key = getPubkeyDer(pkey);
    }
    cred.credential_scheme = SignatureScheme::ecdsa_secp256r1_sha256;

    updateSignature(cred);

    return cred;
  }

  std::unique_ptr<openssl::OpenSSLSelfCertImpl<openssl::KeyType::P256>>
      parentCert_;
};

TEST_F(SelfDelegatedCredentialTest, TestConstruction) {
  auto dcKey = generateDelegatedPrivkey();
  auto credential = makeCredential(dcKey);
  std::unique_ptr<SelfDelegatedCredentialImpl<openssl::KeyType::P256>> credCert;
  Error err;
  EXPECT_EQ(
      SelfDelegatedCredentialImpl<openssl::KeyType::P256>::create(
          credCert,
          err,
          fizz::extensions::DelegatedCredentialMode::Server,
          getCertVec(),
          std::move(dcKey),
          std::move(credential)),
      Status::Success);
}

TEST_F(SelfDelegatedCredentialTest, TestEd25519DCConstruction) {
  auto dcKey = generateEd25519PrivKey();
  auto credential = makeCredential(dcKey);
  std::unique_ptr<SelfDelegatedCredentialImpl<openssl::KeyType::ED25519>>
      credCert;
  Error err;
  EXPECT_EQ(
      SelfDelegatedCredentialImpl<openssl::KeyType::ED25519>::create(
          credCert,
          err,
          fizz::extensions::DelegatedCredentialMode::Server,
          getCertVec(),
          std::move(dcKey),
          std::move(credential)),
      Status::Success);
}

TEST_F(SelfDelegatedCredentialTest, TestConstructionFailureBadSignature) {
  auto dcKey = generateDelegatedPrivkey();
  auto credential = makeCredential(dcKey);

  credential.signature = IOBuf::copyBuffer("hash algorithm party");
  std::unique_ptr<SelfDelegatedCredentialImpl<openssl::KeyType::P256>> cred;
  Error err;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(
          SelfDelegatedCredentialImpl<openssl::KeyType::P256>::create(
              cred,
              err,
              fizz::extensions::DelegatedCredentialMode::Server,
              getCertVec(),
              std::move(dcKey),
              std::move(credential)),
          err),
      std::runtime_error);
}

TEST_F(SelfDelegatedCredentialTest, TestConstructionFailureKeyTypeMismatch) {
  auto dcKey = generateRSAKey();
  auto credential = makeCredential(dcKey);

  // RSA key with EC cert should fail
  std::unique_ptr<SelfDelegatedCredentialImpl<openssl::KeyType::P256>> cred;
  Error err;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(
          SelfDelegatedCredentialImpl<openssl::KeyType::P256>::create(
              cred,
              err,
              fizz::extensions::DelegatedCredentialMode::Server,
              getCertVec(),
              std::move(dcKey),
              std::move(credential)),
          err),
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
  std::unique_ptr<SelfDelegatedCredentialImpl<openssl::KeyType::P256>> cred;
  Error err;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(
          SelfDelegatedCredentialImpl<openssl::KeyType::P256>::create(
              cred,
              err,
              fizz::extensions::DelegatedCredentialMode::Server,
              getCertVec(),
              std::move(dcKey),
              std::move(credential)),
          err),
      std::runtime_error);
}

TEST_F(SelfDelegatedCredentialTest, TestSignatureValidity) {
  auto dcKey = generateDelegatedPrivkey();
  auto credential = makeCredential(dcKey);
  std::unique_ptr<SelfDelegatedCredentialImpl<openssl::KeyType::P256>> credCert;
  Error err;
  EXPECT_EQ(
      SelfDelegatedCredentialImpl<openssl::KeyType::P256>::create(
          credCert,
          err,
          fizz::extensions::DelegatedCredentialMode::Server,
          getCertVec(),
          std::move(dcKey),
          std::move(credential)),
      Status::Success);
  Buf toSign = IOBuf::copyBuffer("signme");

  Buf certSig;
  EXPECT_EQ(
      parentCert_->sign(
          certSig,
          err,
          SignatureScheme::ecdsa_secp256r1_sha256,
          CertificateVerifyContext::Server,
          toSign->coalesce()),
      Status::Success);
  Buf dcSig;
  EXPECT_EQ(
      credCert->sign(
          dcSig,
          err,
          SignatureScheme::ecdsa_secp256r1_sha256,
          CertificateVerifyContext::Server,
          toSign->coalesce()),
      Status::Success);

  std::unique_ptr<PeerCert> parentPeerCert;
  EXPECT_EQ(
      openssl::CertUtils::makePeerCert(
          parentPeerCert, err, parentCert_->getX509()),
      Status::Success);
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(
          parentPeerCert->verify(
              err,
              SignatureScheme::ecdsa_secp256r1_sha256,
              CertificateVerifyContext::Server,
              toSign->coalesce(),
              dcSig->coalesce()),
          err),
      std::runtime_error);

  EXPECT_EQ(
      parentPeerCert->verify(
          err,
          SignatureScheme::ecdsa_secp256r1_sha256,
          CertificateVerifyContext::Server,
          toSign->coalesce(),
          certSig->coalesce()),
      Status::Success);
}

} // namespace test
} // namespace extensions
} // namespace fizz
