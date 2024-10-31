/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/backend/openssl/certificate/OpenSSLSelfCertImpl.h>
#include <fizz/protocol/test/CertTestTypes.h>

using namespace testing;

namespace fizz {
namespace test {

template <typename T>
class CertUtilsTestTyped : public testing::Test {
 public:
  void SetUp() override {
    OpenSSL_add_all_algorithms();
  }
};

using KeyTypes = Types< //
    Ed25519Test,
    P256Test,
    P384Test,
    P521Test,
    RSATest>;
TYPED_TEST_SUITE(CertUtilsTestTyped, KeyTypes);

TYPED_TEST(CertUtilsTestTyped, TestSignVerify) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  folly::ssl::EvpPkeyUniquePtr pubKey(X509_get_pubkey(certs.back().get()));
  openssl::OpenSSLSignature<TypeParam::Type> verificationSignature;
  verificationSignature.setKey(std::move(pubKey));

  openssl::OpenSSLSelfCertImpl<TypeParam::Type> selfCert(
      getKey<TypeParam>(), std::move(certs));

  folly::StringPiece toBeSigned{"ToBeSigned"};
  auto sig = selfCert.sign(
      TypeParam::Scheme, CertificateVerifyContext::Server, toBeSigned);
  openssl::CertUtils::verify(
      verificationSignature,
      TypeParam::Scheme,
      CertificateVerifyContext::Server,
      toBeSigned,
      sig->coalesce());
}

TYPED_TEST(CertUtilsTestTyped, TestSignVerifyBitFlip) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  folly::ssl::EvpPkeyUniquePtr pubKey(X509_get_pubkey(certs.back().get()));
  openssl::OpenSSLSignature<TypeParam::Type> verificationSignature;
  verificationSignature.setKey(std::move(pubKey));

  openssl::OpenSSLSelfCertImpl<TypeParam::Type> selfCert(
      getKey<TypeParam>(), std::move(certs));

  folly::StringPiece toBeSigned{"ToBeSigned"};
  auto sig = selfCert.sign(
      TypeParam::Scheme, CertificateVerifyContext::Server, toBeSigned);
  sig->writableData()[1] ^= 0x20;
  EXPECT_THROW(
      openssl::CertUtils::verify(
          verificationSignature,
          TypeParam::Scheme,
          CertificateVerifyContext::Server,
          toBeSigned,
          sig->coalesce()),
      std::runtime_error);
}

TYPED_TEST(CertUtilsTestTyped, TestSignVerifyWrongSize) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  folly::ssl::EvpPkeyUniquePtr pubKey(X509_get_pubkey(certs.back().get()));
  openssl::OpenSSLSignature<TypeParam::Type> verificationSignature;
  verificationSignature.setKey(std::move(pubKey));

  openssl::OpenSSLSelfCertImpl<TypeParam::Type> selfCert(
      getKey<TypeParam>(), std::move(certs));

  folly::StringPiece toBeSigned{"ToBeSigned"};
  auto sig = selfCert.sign(
      TypeParam::Scheme, CertificateVerifyContext::Server, toBeSigned);
  sig->prependChain(folly::IOBuf::copyBuffer("x"));
  EXPECT_THROW(
      openssl::CertUtils::verify(
          verificationSignature,
          TypeParam::Scheme,
          CertificateVerifyContext::Server,
          toBeSigned,
          sig->coalesce()),
      std::runtime_error);
}

TYPED_TEST(CertUtilsTestTyped, TestSignVerifyWrongScheme) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  folly::ssl::EvpPkeyUniquePtr pubKey(X509_get_pubkey(certs.back().get()));
  openssl::OpenSSLSignature<TypeParam::Type> verificationSignature;
  verificationSignature.setKey(std::move(pubKey));

  openssl::OpenSSLSelfCertImpl<TypeParam::Type> selfCert(
      getKey<TypeParam>(), std::move(certs));

  folly::StringPiece toBeSigned{"ToBeSigned"};
  auto sig = selfCert.sign(
      TypeParam::Scheme, CertificateVerifyContext::Server, toBeSigned);
  EXPECT_THROW(
      openssl::CertUtils::verify(
          verificationSignature,
          TypeParam::Invalid::Scheme,
          CertificateVerifyContext::Server,
          toBeSigned,
          sig->coalesce()),
      std::runtime_error);
}
} // namespace test
} // namespace fizz
