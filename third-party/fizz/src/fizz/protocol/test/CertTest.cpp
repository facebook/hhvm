/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/backend/openssl/certificate/OpenSSLPeerCertImpl.h>
#include <fizz/backend/openssl/certificate/OpenSSLSelfCertImpl.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/test/CertTestTypes.h>
#include <fizz/util/Logging.h>
#include <folly/String.h>

using namespace folly;
using namespace testing;

namespace {

std::vector<folly::ssl::X509UniquePtr> cloneCerts(
    const std::vector<folly::ssl::X509UniquePtr>& certs) {
  std::vector<folly::ssl::X509UniquePtr> ret;
  for (const auto& cert : certs) {
    X509_up_ref(cert.get());
    ret.push_back(folly::ssl::X509UniquePtr(cert.get()));
  }
  return ret;
}
} // namespace

namespace fizz {
namespace test {

template <typename T>
class CertTestTyped : public Test {
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
TYPED_TEST_SUITE(CertTestTyped, KeyTypes);

TEST(CertTest, GetIdentity) {
  auto cert = getCert(kP256Certificate);
  auto key = getPrivateKey(kP256Key);
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(std::move(cert));
  std::unique_ptr<openssl::OpenSSLSelfCertImpl<openssl::KeyType::P256>>
      certificate;
  Error err;
  EXPECT_EQ(
      openssl::OpenSSLSelfCertImpl<openssl::KeyType::P256>::create(
          certificate, err, std::move(key), std::move(certs)),
      Status::Success);
  EXPECT_EQ(certificate->getIdentity(), "Fizz");
  EXPECT_EQ(certificate->getAltIdentities().size(), 0);
}

TEST(CertTest, GetAltIdentity) {
  auto cert = getCert(kRSACertificate);
  auto key = getPrivateKey(kRSAKey);
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(std::move(cert));
  std::unique_ptr<openssl::OpenSSLSelfCertImpl<openssl::KeyType::RSA>>
      certificate;
  Error err;
  EXPECT_EQ(
      openssl::OpenSSLSelfCertImpl<openssl::KeyType::RSA>::create(
          certificate, err, std::move(key), std::move(certs)),
      Status::Success);
  EXPECT_EQ(certificate->getIdentity(), "Fizz");
  auto alts = certificate->getAltIdentities();
  EXPECT_EQ(alts.size(), 3);
  EXPECT_EQ(alts[0], "*.fizz.com");
  EXPECT_EQ(alts[1], "fizz.com");
  EXPECT_EQ(alts[2], "example.net");
}

TEST(CertTest, GetCertMessage) {
  auto cert = getCert(kP256Certificate);
  auto key = getPrivateKey(kP256Key);
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(std::move(cert));
  std::unique_ptr<openssl::OpenSSLSelfCertImpl<openssl::KeyType::P256>>
      certificate;
  Error err;
  EXPECT_EQ(
      openssl::OpenSSLSelfCertImpl<openssl::KeyType::P256>::create(
          certificate, err, std::move(key), std::move(certs)),
      Status::Success);
  CertificateMsg msg;
  EXPECT_EQ(certificate->getCertMessage(msg, err, nullptr), Status::Success);
  ASSERT_EQ(msg.certificate_list.size(), 1);
  auto& firstCertEntry = msg.certificate_list[0];
  auto firstCertData = firstCertEntry.cert_data->coalesce();
  auto firstCertDataPtr = firstCertData.data();
  folly::ssl::X509UniquePtr firstEncodedCert(
      d2i_X509(nullptr, &firstCertDataPtr, firstCertData.size()));
  FIZZ_CHECK(firstEncodedCert);

  auto certCopy = getCert(kP256Certificate);
  EXPECT_EQ(X509_cmp(firstEncodedCert.get(), certCopy.get()), 0);
}

// example taken from https://tlswg.github.io/tls13-spec/#certificate-verify
TEST(CertTest, PrepareSignData) {
  std::array<uint8_t, 32> toBeSigned;
  memset(toBeSigned.data(), 1, toBeSigned.size());
  auto out = fizz::certverify::prepareSignData(
      CertificateVerifyContext::Server, folly::range(toBeSigned));
  auto hex = hexlify(out->moveToFbString());
  std::string expected =
      "2020202020202020202020202020202020202020202020202020202020202020"
      "2020202020202020202020202020202020202020202020202020202020202020"
      "544c5320312e332c207365727665722043657274696669636174655665726966"
      "79"
      "00"
      "0101010101010101010101010101010101010101010101010101010101010101";
  EXPECT_EQ(hex, expected);
}

TEST(CertTest, MakePeerCertEmpty) {
  Error err;
  EXPECT_THROW(
      {
        std::unique_ptr<PeerCert> ret;
        FIZZ_THROW_ON_ERROR(
            openssl::CertUtils::makePeerCert(ret, err, IOBuf::copyBuffer("")),
            err);
      },
      std::runtime_error);
}

TEST(CertTest, MakePeerCertJunk) {
  Error err;
  EXPECT_THROW(
      {
        std::unique_ptr<PeerCert> ret;
        FIZZ_THROW_ON_ERROR(
            openssl::CertUtils::makePeerCert(
                ret, err, IOBuf::copyBuffer("blah")),
            err);
      },
      std::runtime_error);
}

TEST(CertTest, PeerCertGetX509) {
  std::unique_ptr<openssl::OpenSSLPeerCertImpl<openssl::KeyType::P256>>
      peerCert;
  Error err;
  EXPECT_EQ(
      openssl::OpenSSLPeerCertImpl<openssl::KeyType::P256>::create(
          peerCert, err, getCert(kP256Certificate)),
      Status::Success);
  auto x509 = peerCert->getX509();
  EXPECT_NE(x509.get(), nullptr);
}

TEST(CertTest, GetIdentityLogic) {
  Error err;
  std::unique_ptr<SelfCert> selfCert;
  EXPECT_EQ(
      openssl::CertUtils::makeSelfCert(
          selfCert,
          err,
          kCertWithNoCNButWithSANs.str(),
          kCertWithNoCNButWithSANsKey.str(),
          ""),
      Status::Success);
  EXPECT_EQ("O = interop runner", selfCert->getIdentity());

  std::unique_ptr<PeerCert> peerCert;
  EXPECT_EQ(
      openssl::CertUtils::makePeerCert(
          peerCert, err, fizz::test::getCert(kCertWithNoCNButWithSANs)),
      Status::Success);
  EXPECT_EQ("O = interop runner", peerCert->getIdentity());
}

TYPED_TEST(CertTestTyped, MatchingCert) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  std::unique_ptr<openssl::OpenSSLSelfCertImpl<TypeParam::Type>> certificate;
  Error err;
  EXPECT_EQ(
      openssl::OpenSSLSelfCertImpl<TypeParam::Type>::create(
          certificate, err, getKey<TypeParam>(), std::move(certs)),
      Status::Success);
}

TYPED_TEST(CertTestTyped, MismatchedCert) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  std::unique_ptr<openssl::OpenSSLSelfCertImpl<TypeParam::Type>> certificate;
  Error err;
  EXPECT_NE(
      openssl::OpenSSLSelfCertImpl<TypeParam::Type>::create(
          certificate,
          err,
          getKey<typename TypeParam::Invalid>(),
          std::move(certs)),
      Status::Success);
}

TYPED_TEST(CertTestTyped, SigSchemes) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  std::unique_ptr<openssl::OpenSSLSelfCertImpl<TypeParam::Type>> certificate;
  Error err;
  EXPECT_EQ(
      openssl::OpenSSLSelfCertImpl<TypeParam::Type>::create(
          certificate, err, getKey<TypeParam>(), std::move(certs)),
      Status::Success);

  std::vector<SignatureScheme> expected{TypeParam::Scheme};
  EXPECT_EQ(certificate->getSigSchemes(), expected);
}

TYPED_TEST(CertTestTyped, TestVerifyDecodedCert) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  CertificateMsg msg;
  Error certMsgErr;
  EXPECT_EQ(
      openssl::CertUtils::getCertMessage(msg, certMsgErr, certs, nullptr),
      Status::Success);
  std::unique_ptr<openssl::OpenSSLSelfCertImpl<TypeParam::Type>> selfCert;
  Error err;
  EXPECT_EQ(
      openssl::OpenSSLSelfCertImpl<TypeParam::Type>::create(
          selfCert, err, getKey<TypeParam>(), std::move(certs)),
      Status::Success);

  std::unique_ptr<PeerCert> peerCert;
  EXPECT_EQ(
      openssl::CertUtils::makePeerCert(
          peerCert, err, std::move(msg.certificate_list.front().cert_data)),
      Status::Success);

  StringPiece tbs{"ToBeSigned"};
  Buf sig;
  EXPECT_EQ(
      selfCert->sign(
          sig, err, TypeParam::Scheme, CertificateVerifyContext::Server, tbs),
      Status::Success);
  EXPECT_EQ(
      peerCert->verify(
          err,
          TypeParam::Scheme,
          CertificateVerifyContext::Server,
          tbs,
          sig->coalesce()),
      Status::Success);
}

TYPED_TEST(CertTestTyped, TestGetX509Chain) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  auto cloned = cloneCerts(certs);
  std::unique_ptr<openssl::OpenSSLSelfCertImpl<TypeParam::Type>> certificate;
  Error err;
  EXPECT_EQ(
      openssl::OpenSSLSelfCertImpl<TypeParam::Type>::create(
          certificate, err, getKey<TypeParam>(), std::move(certs)),
      Status::Success);
  auto chain = certificate->getX509Chain();
  EXPECT_EQ(chain.size(), cloned.size());
  for (size_t i = 0; i < chain.size(); i++) {
    EXPECT_EQ(X509_cmp(cloned[i].get(), chain[i].get()), 0);
  }
}
} // namespace test
} // namespace fizz
