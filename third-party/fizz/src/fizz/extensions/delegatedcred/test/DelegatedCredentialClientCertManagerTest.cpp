/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/extensions/delegatedcred/DelegatedCredentialClientCertManager.h>
#include <fizz/extensions/delegatedcred/Types.h>

#include <fizz/protocol/test/Mocks.h>

using namespace fizz::test;

namespace fizz {
namespace extensions {
namespace test {

static const std::vector<SignatureScheme> kRsa{SignatureScheme::rsa_pss_sha256};

std::vector<Extension> getDelegatedExt(
    const std::vector<SignatureScheme>& schemes = kRsa) {
  std::vector<Extension> exts;
  DelegatedCredentialSupport supp;
  supp.supported_signature_algorithms = schemes;
  exts.push_back(encodeExtension(supp));
  return exts;
}

class DelegatedClientCertManagerTest : public Test {
 protected:
  std::shared_ptr<MockSelfCert> getCert(std::vector<SignatureScheme> schemes) {
    auto cert = std::make_shared<MockSelfCert>();
    ON_CALL(*cert, getSigSchemes()).WillByDefault(Return(schemes));
    return cert;
  }
  DelegatedCredentialClientCertManager manager_;
};

TEST_F(DelegatedClientCertManagerTest, TestNoCert) {
  auto res = manager_.getCert(folly::none, kRsa, kRsa, {});
  EXPECT_FALSE(res.hasValue());
}

TEST_F(DelegatedClientCertManagerTest, TestBasicMatch) {
  // Only non dc cert must be returned
  auto cert = getCert(kRsa);
  manager_.addCertAndOverride(cert);
  auto res = manager_.getCert(folly::none, kRsa, kRsa, {});
  EXPECT_EQ(res->cert, cert);
  EXPECT_FALSE(manager_.hasDelegatedCredential());
}

TEST_F(DelegatedClientCertManagerTest, TestBasicMatchNoExt) {
  auto cert1 = getCert(kRsa);
  auto cert2 = getCert(kRsa);
  manager_.addCertAndOverride(cert1);
  // Let's just pretend here the impl of the self cert doesnt really matter
  manager_.addDelegatedCredentialAndOverride(cert2);
  EXPECT_TRUE(manager_.hasDelegatedCredential());
  {
    auto res = manager_.getCert(folly::none, kRsa, kRsa, {});
    EXPECT_EQ(res->cert, cert1);
  }
  {
    auto res = manager_.getCert(folly::none, kRsa, kRsa, getDelegatedExt());
    EXPECT_EQ(res->cert, cert2);
  }
}

TEST_F(DelegatedClientCertManagerTest, TestSigSchemes) {
  auto cert = getCert({SignatureScheme::rsa_pss_sha256});
  manager_.addCertAndOverride(cert);
  auto dcCert = getCert({SignatureScheme::rsa_pss_sha512});
  manager_.addDelegatedCredentialAndOverride(dcCert);
  EXPECT_TRUE(manager_.hasDelegatedCredential());

  {
    auto res = manager_.getCert(
        folly::none,
        {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
        {SignatureScheme::rsa_pss_sha256},
        {});
    // Only non dc cert supports sig schemes
    EXPECT_EQ(res->cert, cert);
    EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha256);
  }
  {
    auto res = manager_.getCert(
        folly::none,
        {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
        {SignatureScheme::rsa_pss_sha256},
        getDelegatedExt());
    // Only non dc cert supports sig schemes
    EXPECT_EQ(res->cert, cert);
    EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha256);
  }

  {
    auto res = manager_.getCert(
        folly::none,
        {SignatureScheme::rsa_pss_sha512, SignatureScheme::rsa_pss_sha256},
        {SignatureScheme::rsa_pss_sha512},
        {});
    // Dc doesn't match due to no ext
    EXPECT_FALSE(res.hasValue());
  }
  {
    auto res = manager_.getCert(
        folly::none,
        {SignatureScheme::rsa_pss_sha512, SignatureScheme::rsa_pss_sha256},
        {SignatureScheme::rsa_pss_sha512},
        getDelegatedExt());
    // Dc still doesn't match since the ext doesn't support 512
    EXPECT_FALSE(res.hasValue());
  }
  {
    auto res = manager_.getCert(
        folly::none,
        {SignatureScheme::rsa_pss_sha512, SignatureScheme::rsa_pss_sha256},
        {SignatureScheme::rsa_pss_sha512},
        getDelegatedExt({SignatureScheme::rsa_pss_sha512}));
    EXPECT_EQ(res->cert, dcCert);
    EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha512);
  }
}

TEST_F(DelegatedClientCertManagerTest, TestNoSigSchemeMatch) {
  auto cert = getCert({SignatureScheme::rsa_pss_sha256});
  manager_.addCertAndOverride(cert);
  auto res = manager_.getCert(
      folly::none,
      {SignatureScheme::rsa_pss_sha256},
      {SignatureScheme::rsa_pss_sha512},
      {});
  EXPECT_FALSE(res);
}
} // namespace test
} // namespace extensions
} // namespace fizz
