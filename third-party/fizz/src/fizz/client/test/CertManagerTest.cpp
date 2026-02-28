/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/client/CertManager.h>

#include <fizz/protocol/test/Mocks.h>

using namespace fizz::test;

namespace fizz {
namespace client {
namespace test {

static const std::vector<SignatureScheme> kRsa{SignatureScheme::rsa_pss_sha256};

class CertManagerTest : public Test {
 protected:
  std::shared_ptr<MockSelfCert> getCert(std::vector<SignatureScheme> schemes) {
    auto cert = std::make_shared<MockSelfCert>();
    ON_CALL(*cert, getSigSchemes()).WillByDefault(Return(schemes));
    return cert;
  }
  CertManager manager_;
};

TEST_F(CertManagerTest, TestBasicMatch) {
  auto cert = getCert(kRsa);
  manager_.addCertAndOverride(cert);
  auto res = manager_.getCert(folly::none, kRsa, kRsa, {});
  EXPECT_EQ(res->cert, cert);
  EXPECT_TRUE(manager_.hasCerts());
}

TEST_F(CertManagerTest, TestNoCertsInMgr) {
  EXPECT_FALSE(manager_.getCert(folly::none, {}, {}, {}).hasValue());
  EXPECT_FALSE(manager_.hasCerts());
}

TEST_F(CertManagerTest, TestSigSchemesPref) {
  auto cert = getCert(
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512});
  manager_.addCertAndOverride(cert);

  auto res = manager_.getCert(
      folly::none,
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      {});
  EXPECT_EQ(res->cert, cert);
  EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha256);

  res = manager_.getCert(
      folly::none,
      {SignatureScheme::rsa_pss_sha512, SignatureScheme::rsa_pss_sha256},
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      {});
  EXPECT_EQ(res->cert, cert);
  EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha512);
}

TEST_F(CertManagerTest, TestServerSigScheme) {
  auto cert1 = getCert({SignatureScheme::rsa_pss_sha256});
  auto cert2 = getCert({SignatureScheme::rsa_pss_sha512});
  manager_.addCertAndOverride(cert1);
  manager_.addCertAndOverride(cert2);

  auto res = manager_.getCert(
      folly::none,
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      {SignatureScheme::rsa_pss_sha512},
      {});
  EXPECT_EQ(res->cert, cert2);
  EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha512);
}

TEST_F(CertManagerTest, TestNoSigSchemeMatch) {
  auto cert = getCert({SignatureScheme::rsa_pss_sha256});
  manager_.addCertAndOverride(cert);
  auto res = manager_.getCert(
      folly::none,
      {SignatureScheme::rsa_pss_sha256},
      {SignatureScheme::rsa_pss_sha512},
      {});
  EXPECT_FALSE(res);
}

TEST_F(CertManagerTest, TestBasicMatchNoneOverride) {
  auto cert = getCert(kRsa);
  manager_.addCert(cert);
  auto res = manager_.getCert(folly::none, kRsa, kRsa, {});
  EXPECT_EQ(res->cert, cert);
}

TEST_F(CertManagerTest, TestSameSigSchemeNoOverride) {
  auto cert = getCert(kRsa);
  auto cert2 = getCert(kRsa);
  manager_.addCert(cert);
  manager_.addCert(cert2);
  auto res = manager_.getCert(folly::none, kRsa, kRsa, {});
  EXPECT_EQ(res->cert, cert);
}

TEST_F(CertManagerTest, TestSameSigSchemeOverride) {
  auto cert = getCert(kRsa);
  auto cert2 = getCert(kRsa);
  manager_.addCertAndOverride(cert);
  manager_.addCertAndOverride(cert2);
  auto res = manager_.getCert(folly::none, kRsa, kRsa, {});
  EXPECT_EQ(res->cert, cert2);
}
} // namespace test
} // namespace client
} // namespace fizz
