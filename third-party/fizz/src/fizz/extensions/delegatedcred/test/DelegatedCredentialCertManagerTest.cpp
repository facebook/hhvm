/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/extensions/delegatedcred/DelegatedCredentialCertManager.h>

#include <fizz/extensions/delegatedcred/test/Mocks.h>

#include <fizz/protocol/test/Mocks.h>

using namespace fizz::test;

namespace fizz {
namespace extensions {
namespace test {

static const std::vector<SignatureScheme> kRsa{SignatureScheme::rsa_pss_sha256};

static ClientHello getChlo(std::vector<Extension> exts) {
  ClientHello chlo;
  chlo.extensions = std::move(exts);
  return chlo;
}

class DelegatedCredentialCertManagerTest : public Test {
 public:
  template <typename T>
  std::shared_ptr<T> getMock(
      std::string identity,
      std::vector<std::string> alts,
      std::vector<SignatureScheme> schemes) {
    auto cert = std::make_shared<T>();
    ON_CALL(*cert, getIdentity()).WillByDefault(Return(identity));
    ON_CALL(*cert, getAltIdentities()).WillByDefault(Return(alts));
    ON_CALL(*cert, getSigSchemes()).WillByDefault(Return(schemes));
    return cert;
  }

  std::shared_ptr<MockSelfCert> getCert(
      std::string identity,
      std::vector<std::string> alts,
      std::vector<SignatureScheme> schemes) {
    return getMock<MockSelfCert>(
        std::move(identity), std::move(alts), std::move(schemes));
  }

  std::shared_ptr<MockSelfDelegatedCredential> getCredential(
      std::string identity,
      std::vector<std::string> alts,
      std::vector<SignatureScheme> schemes) {
    return getMock<MockSelfDelegatedCredential>(
        std::move(identity), std::move(alts), std::move(schemes));
  }

  DelegatedCredentialCertManager manager_;
};

template <typename T>
class DelegatedCredentialCertManagerTestTyped
    : public DelegatedCredentialCertManagerTest {};

struct DelegatedMode {
  static constexpr bool Delegated = true;
  static std::vector<Extension> Extensions(
      const std::vector<SignatureScheme>& schemes = {
          SignatureScheme::rsa_pss_sha256}) {
    std::vector<Extension> exts;
    DelegatedCredentialSupport supp;
    supp.supported_signature_algorithms = schemes;
    exts.push_back(encodeExtension(supp));
    return exts;
  }
};

struct UndelegatedMode {
  static constexpr bool Delegated = false;
  static std::vector<Extension> Extensions(
      const std::vector<SignatureScheme>& /*schemes*/ = {}) {
    return {};
  }
};

using TestTypes = Types<DelegatedMode, UndelegatedMode>;
TYPED_TEST_SUITE(DelegatedCredentialCertManagerTestTyped, TestTypes);

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestNoMatchDefault) {
  auto cert1 =
      DelegatedCredentialCertManagerTest::getCert("blah.com", {}, kRsa);
  auto cert2 =
      DelegatedCredentialCertManagerTest::getCert("blah.com", {}, kRsa);
  DelegatedCredentialCertManagerTest::manager_.addCertAndSetDefault(cert1);
  // Technically, the default flag doesn't really apply to DC certs.
  // It should always return the undelegated default cert if an exact
  // match can't be found..
  DelegatedCredentialCertManagerTest::manager_.addCertAndSetDefault(cert2);
  auto res = DelegatedCredentialCertManagerTest::manager_.getCert(
      std::string("test.com"), kRsa, kRsa, getChlo(TypeParam::Extensions()));
  EXPECT_EQ(res->cert, cert1);
}

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestNoSniDefault) {
  auto cert = DelegatedCredentialCertManagerTest::getCert("blah.com", {}, kRsa);
  DelegatedCredentialCertManagerTest::manager_.addCertAndSetDefault(cert);
  auto res = DelegatedCredentialCertManagerTest::manager_.getCert(
      folly::none, kRsa, kRsa, getChlo(TypeParam::Extensions()));
  EXPECT_EQ(res->cert, cert);
}

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestWildcardDefault) {
  auto cert =
      DelegatedCredentialCertManagerTest::getCert("*.blah.com", {}, kRsa);
  DelegatedCredentialCertManagerTest::manager_.addCertAndSetDefault(cert);
  auto res = DelegatedCredentialCertManagerTest::manager_.getCert(
      folly::none, kRsa, kRsa, getChlo(TypeParam::Extensions()));
  EXPECT_EQ(res->cert, cert);
}

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestUppercaseDefault) {
  auto cert = DelegatedCredentialCertManagerTest::getCert("BLAH.com", {}, kRsa);
  DelegatedCredentialCertManagerTest::manager_.addCertAndSetDefault(cert);
  auto res = DelegatedCredentialCertManagerTest::manager_.getCert(
      folly::none, kRsa, kRsa, getChlo(TypeParam::Extensions()));
  EXPECT_EQ(res->cert, cert);
}

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestNoDefault) {
  EXPECT_FALSE(
      DelegatedCredentialCertManagerTest::manager_
          .getCert(
              std::string("blah.com"), {}, {}, getChlo(TypeParam::Extensions()))
          .hasValue());
}

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestSigSchemesServerPref) {
  std::shared_ptr<SelfCert> cert;
  if (TypeParam::Delegated) {
    auto cred = DelegatedCredentialCertManagerTest::getCredential(
        "www.test.com",
        {},
        {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512});
    DelegatedCredentialCertManagerTest::manager_.addDelegatedCredential(cred);
    cert = cred;
    EXPECT_TRUE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  } else {
    cert = DelegatedCredentialCertManagerTest::getCert(
        "www.test.com",
        {},
        {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512});
    DelegatedCredentialCertManagerTest::manager_.addCert(cert);
    EXPECT_FALSE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  }

  auto res = DelegatedCredentialCertManagerTest::manager_.getCert(
      std::string("www.test.com"),
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      getChlo(
          TypeParam::Extensions(
              {SignatureScheme::rsa_pss_sha256,
               SignatureScheme::rsa_pss_sha512})));
  EXPECT_EQ(res->cert, cert);
  EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha256);

  res = DelegatedCredentialCertManagerTest::manager_.getCert(
      std::string("www.test.com"),
      {SignatureScheme::rsa_pss_sha512, SignatureScheme::rsa_pss_sha256},
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      getChlo(
          TypeParam::Extensions(
              {SignatureScheme::rsa_pss_sha256,
               SignatureScheme::rsa_pss_sha512})));
  EXPECT_EQ(res->cert, cert);
  EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha512);
}

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestClientSigScheme) {
  std::shared_ptr<SelfCert> cert1, cert2;
  if (TypeParam::Delegated) {
    auto cred1 = DelegatedCredentialCertManagerTest::getCredential(
        "www.test.com", {}, {SignatureScheme::rsa_pss_sha256});
    auto cred2 = DelegatedCredentialCertManagerTest::getCredential(
        "www.test.com", {}, {SignatureScheme::rsa_pss_sha512});
    DelegatedCredentialCertManagerTest::manager_.addDelegatedCredential(cred1);
    DelegatedCredentialCertManagerTest::manager_.addDelegatedCredential(cred2);
    cert1 = cred1;
    cert2 = cred2;
    EXPECT_TRUE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  } else {
    cert1 = DelegatedCredentialCertManagerTest::getCert(
        "www.test.com", {}, {SignatureScheme::rsa_pss_sha256});
    cert2 = DelegatedCredentialCertManagerTest::getCert(
        "www.test.com", {}, {SignatureScheme::rsa_pss_sha512});
    DelegatedCredentialCertManagerTest::manager_.addCert(cert1);
    DelegatedCredentialCertManagerTest::manager_.addCert(cert2);
    EXPECT_FALSE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  }

  auto res = DelegatedCredentialCertManagerTest::manager_.getCert(
      std::string("www.test.com"),
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      {SignatureScheme::rsa_pss_sha256},
      getChlo(TypeParam::Extensions({SignatureScheme::rsa_pss_sha512})));
  // Matching differs; for delegated, source of truth is the extension.
  // For non-delegated, it's client signature scheme list.
  if (TypeParam::Delegated) {
    EXPECT_EQ(res->cert, cert2);
    EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha512);
  } else {
    EXPECT_EQ(res->cert, cert1);
    EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha256);
  }
}

TYPED_TEST(
    DelegatedCredentialCertManagerTestTyped,
    TestClientSigSchemeWildcardMatch) {
  std::shared_ptr<SelfCert> cert1, cert2;
  if (TypeParam::Delegated) {
    auto cred1 = DelegatedCredentialCertManagerTest::getCredential(
        "www.test.com", {}, {SignatureScheme::rsa_pss_sha256});
    auto cred2 = DelegatedCredentialCertManagerTest::getCredential(
        "*.test.com", {}, {SignatureScheme::rsa_pss_sha512});
    DelegatedCredentialCertManagerTest::manager_.addDelegatedCredential(cred1);
    DelegatedCredentialCertManagerTest::manager_.addDelegatedCredential(cred2);
    cert1 = cred1;
    cert2 = cred2;
    EXPECT_TRUE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  } else {
    cert1 = DelegatedCredentialCertManagerTest::getCert(
        "www.test.com", {}, {SignatureScheme::rsa_pss_sha256});
    cert2 = DelegatedCredentialCertManagerTest::getCert(
        "*.test.com", {}, {SignatureScheme::rsa_pss_sha512});
    DelegatedCredentialCertManagerTest::manager_.addCert(cert1);
    DelegatedCredentialCertManagerTest::manager_.addCert(cert2);
    EXPECT_FALSE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  }

  auto res = DelegatedCredentialCertManagerTest::manager_.getCert(
      std::string("www.test.com"),
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      {SignatureScheme::rsa_pss_sha256},
      getChlo(TypeParam::Extensions({SignatureScheme::rsa_pss_sha512})));
  // Match should check based on whether it's delegated or not.
  if (TypeParam::Delegated) {
    EXPECT_EQ(res->cert, cert2);
    EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha512);
  } else {
    EXPECT_EQ(res->cert, cert1);
    EXPECT_EQ(res->scheme, SignatureScheme::rsa_pss_sha256);
  }
}

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestAlts) {
  std::shared_ptr<SelfCert> cert;
  if (TypeParam::Delegated) {
    auto cred = DelegatedCredentialCertManagerTest::getCredential(
        "www.test.com",
        {"www.test.com", "www.example.com", "*.example.com"},
        kRsa);
    DelegatedCredentialCertManagerTest::manager_.addDelegatedCredential(cred);
    cert = cred;
    EXPECT_TRUE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  } else {
    cert = DelegatedCredentialCertManagerTest::getCert(
        "www.test.com",
        {"www.test.com", "www.example.com", "*.example.com"},
        kRsa);
    DelegatedCredentialCertManagerTest::manager_.addCert(cert);
    EXPECT_FALSE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  }

  auto res = DelegatedCredentialCertManagerTest::manager_.getCert(
      std::string("www.test.com"),
      kRsa,
      kRsa,
      getChlo(TypeParam::Extensions()));
  EXPECT_EQ(res->cert, cert);

  res = DelegatedCredentialCertManagerTest::manager_.getCert(
      std::string("www.example.com"),
      kRsa,
      kRsa,
      getChlo(TypeParam::Extensions()));
  EXPECT_EQ(res->cert, cert);

  res = DelegatedCredentialCertManagerTest::manager_.getCert(
      std::string("foo.example.com"),
      kRsa,
      kRsa,
      getChlo(TypeParam::Extensions()));
  EXPECT_EQ(res->cert, cert);
}

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestWildcard) {
  std::shared_ptr<SelfCert> cert;
  if (TypeParam::Delegated) {
    auto cred = DelegatedCredentialCertManagerTest::getCredential(
        "*.test.com", {}, kRsa);
    DelegatedCredentialCertManagerTest::manager_.addDelegatedCredential(cred);
    cert = cred;
    EXPECT_TRUE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  } else {
    cert = DelegatedCredentialCertManagerTest::getCert("*.test.com", {}, kRsa);
    DelegatedCredentialCertManagerTest::manager_.addCert(cert);
    EXPECT_FALSE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  }

  auto res = DelegatedCredentialCertManagerTest::manager_.getCert(
      std::string("bar.test.com"),
      kRsa,
      kRsa,
      getChlo(TypeParam::Extensions()));
  EXPECT_EQ(res->cert, cert);

  EXPECT_FALSE(
      DelegatedCredentialCertManagerTest::manager_
          .getCert(
              std::string("foo.bar.test.com"),
              kRsa,
              kRsa,
              getChlo(TypeParam::Extensions()))
          .hasValue());
}

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestExactMatch) {
  std::shared_ptr<SelfCert> ref;
  if (TypeParam::Delegated) {
    auto cred1 = DelegatedCredentialCertManagerTest::getCredential(
        "*.test.com", {}, kRsa);
    auto cred2 = DelegatedCredentialCertManagerTest::getCredential(
        "foo.test.com", {}, kRsa);
    DelegatedCredentialCertManagerTest::manager_.addDelegatedCredential(cred1);
    DelegatedCredentialCertManagerTest::manager_.addDelegatedCredential(cred2);
    ref = cred2;
    EXPECT_TRUE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  } else {
    auto cert1 =
        DelegatedCredentialCertManagerTest::getCert("*.test.com", {}, kRsa);
    auto cert2 =
        DelegatedCredentialCertManagerTest::getCert("foo.test.com", {}, kRsa);
    DelegatedCredentialCertManagerTest::manager_.addCert(cert1);
    DelegatedCredentialCertManagerTest::manager_.addCert(cert2);
    ref = cert2;
    EXPECT_FALSE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  }

  auto res = DelegatedCredentialCertManagerTest::manager_.getCert(
      std::string("foo.test.com"),
      kRsa,
      kRsa,
      getChlo(TypeParam::Extensions()));
  EXPECT_EQ(res->cert, ref);
}

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestNoWildcard) {
  if (TypeParam::Delegated) {
    DelegatedCredentialCertManagerTest::manager_.addDelegatedCredential(
        DelegatedCredentialCertManagerTest::getCredential(
            "foo.test.com", {}, kRsa));
    EXPECT_TRUE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  } else {
    DelegatedCredentialCertManagerTest::manager_.addCert(
        DelegatedCredentialCertManagerTest::getCert("foo.test.com", {}, kRsa));
    EXPECT_FALSE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  }

  EXPECT_FALSE(
      DelegatedCredentialCertManagerTest::manager_
          .getCert(
              std::string("blah.test.com"),
              kRsa,
              kRsa,
              getChlo(TypeParam::Extensions()))
          .hasValue());
  EXPECT_FALSE(
      DelegatedCredentialCertManagerTest::manager_
          .getCert(
              std::string("test.com"),
              kRsa,
              kRsa,
              getChlo(TypeParam::Extensions()))
          .hasValue());
}

TYPED_TEST(DelegatedCredentialCertManagerTestTyped, TestGetByIdentity) {
  std::shared_ptr<SelfCert> cert;
  if (TypeParam::Delegated) {
    auto cred = DelegatedCredentialCertManagerTest::getCredential(
        "*.test.com", {"www.example.com"}, kRsa);
    DelegatedCredentialCertManagerTest::manager_.addDelegatedCredential(cred);
    cert = cred;
    EXPECT_TRUE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  } else {
    cert = DelegatedCredentialCertManagerTest::getCert(
        "*.test.com", {"www.example.com"}, kRsa);
    DelegatedCredentialCertManagerTest::manager_.addCert(cert);
    EXPECT_FALSE(
        DelegatedCredentialCertManagerTest::manager_.hasDelegatedCredential());
  }

  EXPECT_EQ(
      DelegatedCredentialCertManagerTest::manager_.getCert("*.test.com"), cert);
  EXPECT_EQ(
      DelegatedCredentialCertManagerTest::manager_.getCert("www.example.com"),
      nullptr);
  EXPECT_EQ(
      DelegatedCredentialCertManagerTest::manager_.getCert("foo.test.com"),
      nullptr);
  EXPECT_EQ(
      DelegatedCredentialCertManagerTest::manager_.getCert("www.blah.com"),
      nullptr);
}

TEST_F(DelegatedCredentialCertManagerTest, TestDelegatedMatchPreferred) {
  auto cert1 = getCert("foo.test.com", {}, kRsa);
  auto cert2 = getCredential("foo.test.com", {}, kRsa);
  manager_.addCert(cert1);
  manager_.addDelegatedCredential(cert2);
  EXPECT_TRUE(manager_.hasDelegatedCredential());

  auto res = manager_.getCert(
      std::string("foo.test.com"),
      kRsa,
      kRsa,
      getChlo(DelegatedMode::Extensions({SignatureScheme::rsa_pss_sha256})));
  EXPECT_EQ(res->cert, cert2);
}

TEST_F(DelegatedCredentialCertManagerTest, TestUndelegatedNeverGetsCredential) {
  std::string host1("foo.test.com");
  std::string host2("test.wildcard.test.com");
  std::string host3("test.com");
  std::string host4("foobar.com");

  auto cert1 = getCert("foo.test.com", {}, kRsa);
  auto cert2 = getCert("*.wildcard.test.com", {}, kRsa);
  auto cert3 = getCert("test.com", {}, kRsa);
  auto cert4 = getCredential("foo.test.com", {}, kRsa);
  auto cert5 = getCredential("*.wildcard.test.com", {}, kRsa);
  auto cert6 = getCredential("test.com", {}, kRsa);

  // Add delegated ones first.
  manager_.addDelegatedCredential(cert4);
  manager_.addDelegatedCredential(cert5);
  manager_.addDelegatedCredential(cert6);
  EXPECT_TRUE(manager_.hasDelegatedCredential());
  auto res = manager_.getCert(host1, kRsa, kRsa, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(host1, kRsa, {}, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(host2, kRsa, kRsa, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(host2, kRsa, {}, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(host3, kRsa, kRsa, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(host3, kRsa, {}, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(host4, kRsa, kRsa, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(host4, kRsa, {}, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(folly::none, kRsa, kRsa, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(folly::none, kRsa, {}, {});
  EXPECT_FALSE(res);

  manager_.addCert(cert1);
  manager_.addCert(cert2);
  manager_.addCertAndSetDefault(cert3);

  res = manager_.getCert(host1, kRsa, kRsa, {});
  EXPECT_EQ(res->cert, cert1);
  res = manager_.getCert(host1, kRsa, {}, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(host2, kRsa, kRsa, {});
  EXPECT_EQ(res->cert, cert2);
  res = manager_.getCert(host2, kRsa, {}, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(host3, kRsa, kRsa, {});
  EXPECT_EQ(res->cert, cert3);
  res = manager_.getCert(host3, kRsa, {}, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(host4, kRsa, kRsa, {});
  EXPECT_EQ(res->cert, cert3);
  res = manager_.getCert(host4, kRsa, {}, {});
  EXPECT_FALSE(res);
  res = manager_.getCert(folly::none, kRsa, kRsa, {});
  EXPECT_EQ(res->cert, cert3);
  res = manager_.getCert(folly::none, kRsa, {}, {});
  EXPECT_FALSE(res);
}

TEST_F(
    DelegatedCredentialCertManagerTest,
    TestDelegatedRequestWithOnlyUndelegatedCerts) {
  std::string host1("foo.test.com");
  std::string host2("test.wildcard.test.com");
  std::string host3("test.com");
  std::string host4("foobar.com");

  auto cert1 = getCert("foo.test.com", {}, kRsa);
  auto cert2 = getCert("*.wildcard.test.com", {}, kRsa);
  auto cert3 = getCert("test.com", {}, kRsa);

  manager_.addCert(cert1);
  manager_.addCert(cert2);
  manager_.addCertAndSetDefault(cert3);

  auto res =
      manager_.getCert(host1, kRsa, kRsa, getChlo(DelegatedMode::Extensions()));
  EXPECT_EQ(res->cert, cert1);
  res = manager_.getCert(host1, kRsa, {}, getChlo(DelegatedMode::Extensions()));
  EXPECT_FALSE(res);
  res =
      manager_.getCert(host2, kRsa, kRsa, getChlo(DelegatedMode::Extensions()));
  EXPECT_EQ(res->cert, cert2);
  res = manager_.getCert(host2, kRsa, {}, getChlo(DelegatedMode::Extensions()));
  EXPECT_FALSE(res);
  res =
      manager_.getCert(host3, kRsa, kRsa, getChlo(DelegatedMode::Extensions()));
  EXPECT_EQ(res->cert, cert3);
  res = manager_.getCert(host3, kRsa, {}, getChlo(DelegatedMode::Extensions()));
  EXPECT_FALSE(res);
  res =
      manager_.getCert(host4, kRsa, kRsa, getChlo(DelegatedMode::Extensions()));
  EXPECT_EQ(res->cert, cert3);
  res = manager_.getCert(host4, kRsa, {}, getChlo(DelegatedMode::Extensions()));
  EXPECT_FALSE(res);
  res = manager_.getCert(
      folly::none, kRsa, kRsa, getChlo(DelegatedMode::Extensions()));
  EXPECT_EQ(res->cert, cert3);
  res = manager_.getCert(
      folly::none, kRsa, {}, getChlo(DelegatedMode::Extensions()));
  EXPECT_FALSE(res);
}

TEST_F(
    DelegatedCredentialCertManagerTest,
    TestUndelegatedMatchWhenNoDelegatedMatch) {
  std::string host("foo.test.com");
  auto cert1 = getCert(
      host,
      {},
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512});
  auto cert2 = getCredential(host, {}, {SignatureScheme::rsa_pss_sha256});
  manager_.addCert(cert1);
  manager_.addDelegatedCredential(cert2);
  EXPECT_TRUE(manager_.hasDelegatedCredential());

  auto res = manager_.getCert(
      host,
      {SignatureScheme::rsa_pss_sha256, SignatureScheme::rsa_pss_sha512},
      {SignatureScheme::rsa_pss_sha512},
      getChlo(DelegatedMode::Extensions({SignatureScheme::rsa_pss_sha512})));
  EXPECT_EQ(res->cert, cert1);
}

TEST_F(DelegatedCredentialCertManagerTest, TestDelegatedMatchWithDefaultSet) {
  auto cert1 = getCert("foo.test.com", {}, kRsa);
  auto cert2 = getCredential("foo.test.com", {}, kRsa);
  manager_.addCertAndSetDefault(cert1);
  manager_.addDelegatedCredentialAndSetDefault(cert2);
  EXPECT_TRUE(manager_.hasDelegatedCredential());

  auto res = manager_.getCert(
      std::string("foo_blah"),
      kRsa,
      kRsa,
      getChlo(DelegatedMode::Extensions()));
  EXPECT_EQ(res->cert, cert2);
}

TEST_F(
    DelegatedCredentialCertManagerTest,
    TestDelegatedNotMatchedWithDefaultNotSet) {
  auto cert1 = getCert("foo.test.com", {}, kRsa);
  auto cert2 = getCredential("foo.test.com", {}, kRsa);
  manager_.addCertAndSetDefault(cert1);
  manager_.addDelegatedCredential(cert2);
  EXPECT_TRUE(manager_.hasDelegatedCredential());

  auto res = manager_.getCert(
      std::string("foo_blah"),
      kRsa,
      kRsa,
      getChlo(DelegatedMode::Extensions()));
  EXPECT_EQ(res->cert, cert1);
}
} // namespace test
} // namespace extensions
} // namespace fizz
