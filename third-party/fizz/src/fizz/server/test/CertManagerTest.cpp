/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/server/DefaultCertManager.h>

#include <fizz/protocol/test/Mocks.h>

#include <utility>

using namespace fizz::test;

namespace fizz {
namespace server {
namespace test {

static const std::vector<SignatureScheme> kRsa{SignatureScheme::rsa_pss_sha256};
static const std::vector<SignatureScheme> kEcdsa{
    SignatureScheme::ecdsa_secp256r1_sha256};

class CertManagerTest : public Test {
 protected:
  std::shared_ptr<MockSelfCert> getCert(
      std::string identity,
      std::vector<std::string> alts,
      std::vector<SignatureScheme> schemes) {
    auto cert = std::make_shared<MockSelfCert>();
    ON_CALL(*cert, getIdentity()).WillByDefault(Return(std::move(identity)));
    ON_CALL(*cert, getAltIdentities()).WillByDefault(Return(std::move(alts)));
    ON_CALL(*cert, getSigSchemes()).WillByDefault(Return(std::move(schemes)));
    return cert;
  }

  ClientHello getChlo() {
    ClientHello chlo;
    return chlo;
  }

  DefaultCertManager manager_;
};

TEST_F(CertManagerTest, TestNoMatchDefault) {
  auto cert = getCert("blah.com", {}, kRsa);
  manager_.addCertAndSetDefault(cert);
  auto res = manager_.getCert(std::string("test.com"), kRsa, kRsa, getChlo());
  EXPECT_EQ(res->cert, cert);
  EXPECT_TRUE(manager_.hasCerts());

  auto res2 =
      manager_.getCert(std::string("test.com"), kEcdsa, kEcdsa, getChlo());
  EXPECT_FALSE(res2.hasValue());
}

TEST_F(CertManagerTest, TestNoMatchDefaultChooseSigScheme) {
  auto rsaCert = getCert("blah.com", {}, kRsa);
  auto ecCert = getCert("blah.com", {}, kEcdsa);
  manager_.addCertAndSetDefault(rsaCert);
  manager_.addCert(ecCert);
  // test that we match against the default cert, but choose the correct sig
  // scheme
  {
    auto res = manager_.getCert(std::string("test.com"), kRsa, kRsa, getChlo());
    EXPECT_EQ(res->cert, rsaCert);
    EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha256);
  }
  {
    auto res2 =
        manager_.getCert(std::string("test.com"), kEcdsa, kEcdsa, getChlo());
    EXPECT_EQ(res2->cert, ecCert);
    EXPECT_EQ(res2->scheme, SignatureScheme::ecdsa_secp256r1_sha256);
  }
}

TEST_F(CertManagerTest, TestNoSniDefault) {
  auto cert = getCert("blah.com", {}, kRsa);
  manager_.addCertAndSetDefault(cert);
  auto res = manager_.getCert(folly::none, kRsa, kRsa, getChlo());
  EXPECT_EQ(res->cert, cert);
}

TEST_F(CertManagerTest, TestWildcardDefault) {
  auto cert = getCert("*.blah.com", {}, kRsa);
  manager_.addCertAndSetDefault(cert);
  auto res = manager_.getCert(folly::none, kRsa, kRsa, getChlo());
  EXPECT_EQ(res->cert, cert);
}

TEST_F(CertManagerTest, TestUppercaseDefault) {
  auto cert = getCert("BLAH.com", {}, kRsa);
  manager_.addCertAndSetDefault(cert);
  auto res = manager_.getCert(folly::none, kRsa, kRsa, getChlo());
  EXPECT_EQ(res->cert, cert);
}

TEST_F(CertManagerTest, TestNoDefault) {
  EXPECT_FALSE(
      manager_.getCert(std::string("blah.com"), {}, {}, getChlo()).hasValue());
  EXPECT_FALSE(manager_.hasCerts());
}

TEST_F(CertManagerTest, TestSigSchemesServerPref) {
  auto cert = getCert(
      "www.test.com",
      {},
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512});
  manager_.addCert(cert);

  auto res = manager_.getCert(
      std::string("www.test.com"),
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      getChlo());
  EXPECT_EQ(res->cert, cert);
  EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha256);

  res = manager_.getCert(
      std::string("www.test.com"),
      {SignatureScheme::rsa_pss_sha512, SignatureScheme::rsa_pss_sha256},
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      getChlo());
  EXPECT_EQ(res->cert, cert);
  EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha512);
}

TEST_F(CertManagerTest, TestClientSigScheme) {
  auto cert1 = getCert("www.test.com", {}, {SignatureScheme::rsa_pss_sha256});
  auto cert2 = getCert("www.test.com", {}, {SignatureScheme::rsa_pss_sha512});
  manager_.addCert(cert1);
  manager_.addCert(cert2);

  auto res = manager_.getCert(
      std::string("www.test.com"),
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      {SignatureScheme::rsa_pss_sha512},
      getChlo());
  EXPECT_EQ(res->cert, cert2);
  EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha512);
}

TEST_F(CertManagerTest, TestClientSigSchemeWildcardMatch) {
  auto cert1 = getCert("www.test.com", {}, {SignatureScheme::rsa_pss_sha256});
  auto cert2 = getCert("*.test.com", {}, {SignatureScheme::rsa_pss_sha512});
  manager_.addCert(cert1);
  manager_.addCert(cert2);

  auto res = manager_.getCert(
      std::string("www.test.com"),
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      {SignatureScheme::rsa_pss_sha512},
      getChlo());
  EXPECT_EQ(res->cert, cert2);
  EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha512);
}

TEST_F(CertManagerTest, TestClientSigSchemeNoMatch) {
  auto cert = getCert("www.test.com", {}, {SignatureScheme::rsa_pss_sha256});
  manager_.addCert(cert);

  auto res = manager_.getCert(
      std::string("www.test.com"),
      {SignatureScheme::rsa_pss_sha256},
      {SignatureScheme::rsa_pss_sha512},
      getChlo());
  EXPECT_FALSE(res);
}

TEST_F(CertManagerTest, TestAlts) {
  auto cert = getCert(
      "www.test.com",
      {"www.test.com", "www.example.com", "*.example.com"},
      kRsa);
  manager_.addCert(cert);

  auto res =
      manager_.getCert(std::string("www.test.com"), kRsa, kRsa, getChlo());
  EXPECT_EQ(res->cert, cert);

  res = manager_.getCert(std::string("www.example.com"), kRsa, kRsa, getChlo());
  EXPECT_EQ(res->cert, cert);

  res = manager_.getCert(std::string("foo.example.com"), kRsa, kRsa, getChlo());
  EXPECT_EQ(res->cert, cert);
}

TEST_F(CertManagerTest, TestWildcard) {
  auto cert = getCert("*.test.com", {}, kRsa);
  manager_.addCert(cert);

  auto res =
      manager_.getCert(std::string("bar.test.com"), kRsa, kRsa, getChlo());
  EXPECT_EQ(res->cert, cert);

  EXPECT_FALSE(
      manager_.getCert(std::string("foo.bar.test.com"), kRsa, kRsa, getChlo())
          .hasValue());
}

TEST_F(CertManagerTest, TestExactMatch) {
  auto cert1 = getCert("*.test.com", {}, kRsa);
  auto cert2 = getCert("foo.test.com", {}, kRsa);
  manager_.addCert(cert1);
  manager_.addCert(cert2);

  auto res =
      manager_.getCert(std::string("foo.test.com"), kRsa, kRsa, getChlo());
  EXPECT_EQ(res->cert, cert2);
}

TEST_F(CertManagerTest, TestNoWildcard) {
  auto cert = getCert("foo.test.com", {}, kRsa);
  manager_.addCert(cert);

  EXPECT_FALSE(
      manager_.getCert(std::string("blah.test.com"), kRsa, kRsa, getChlo())
          .hasValue());
  EXPECT_FALSE(manager_.getCert(std::string("test.com"), kRsa, kRsa, getChlo())
                   .hasValue());
}

TEST_F(CertManagerTest, TestGetByIdentity) {
  auto cert = getCert("*.test.com", {"www.example.com"}, kRsa);
  manager_.addCert(cert);

  EXPECT_EQ(manager_.getCert("*.test.com"), cert);
  EXPECT_EQ(manager_.getCert("www.example.com"), nullptr);
  EXPECT_EQ(manager_.getCert("foo.test.com"), nullptr);
  EXPECT_EQ(manager_.getCert("www.blah.com"), nullptr);
}

TEST_F(CertManagerTest, TestDN) {
  // This test is largely the same as TestGetByIdentity, but it is asserting
  // that we should not be relying on Certificate::getIdentity() to return a
  // "domain like" string.
  auto cert = getCert("OU=Test Organization, O=Test", {"*.test.com"}, kRsa);
  manager_.addCert(cert);
  EXPECT_EQ(manager_.getCert("OU=Test Organization, O=Test"), cert);
}

TEST_F(CertManagerTest, TestGetCertificatesByIdentity) {
  auto cert1 = getCert("foo.test.com", {}, kRsa);
  auto cert2 = getCert("foo.test.com", {}, kEcdsa);
  manager_.addCert(cert1);
  manager_.addCert(cert2);

  auto certs = manager_.getCertificatesByIdentity();
  EXPECT_EQ(certs.size(), 1);
  auto it = certs.find("foo.test.com");
  EXPECT_TRUE(it != certs.end());
  EXPECT_EQ(it->second.size(), 2);
  EXPECT_EQ(it->second.count(SignatureScheme::rsa_pss_sha256), 1);
  EXPECT_EQ(it->second.count(SignatureScheme::ecdsa_secp256r1_sha256), 1);
}

} // namespace test
} // namespace server
} // namespace fizz
