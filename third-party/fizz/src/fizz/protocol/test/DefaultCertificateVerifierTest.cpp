/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/protocol/DefaultCertificateVerifier.h>
#include <fizz/protocol/test/CertUtil.h>
#include <folly/ssl/OpenSSLCertUtils.h>

namespace fizz {
namespace test {

class DefaultCertificateVerifierTest : public testing::Test {
 public:
  void SetUp() override {
    OpenSSL_add_all_algorithms();
    folly::ssl::X509StoreUniquePtr store(X509_STORE_new());
    ASSERT_TRUE(store);
    rootCertAndKey_ = createCert("root", true, nullptr);
    leafCertAndKey_ = createCert("leaf", false, &rootCertAndKey_);
    ASSERT_EQ(X509_STORE_add_cert(store.get(), rootCertAndKey_.cert.get()), 1);
    verifier_ = std::make_unique<DefaultCertificateVerifier>(
        VerificationContext::Client, std::move(store));
  }

  void TearDown() override {}

  static int allowSelfSignedLeafCertCallback(int ok, X509_STORE_CTX* ctx) {
    if (X509_STORE_CTX_get_error(ctx) ==
        X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) {
      return 1;
    }
    return ok;
  }

 protected:
  CertAndKey rootCertAndKey_;
  CertAndKey leafCertAndKey_;
  std::unique_ptr<DefaultCertificateVerifier> verifier_;
};

template <class F1, class F2>
void expectThrowSuchThat(F1 f, F2 g) {
  try {
    f();
    FAIL() << "Expected to throw";
  } catch (const std::exception& ex) {
    g(ex);
  } catch (...) {
    FAIL() << "non std::exception thrown";
  }
}

template <class F>
static void expectThrowWithAlert(F f, AlertDescription ad) {
  return expectThrowSuchThat(std::move(f), [ad](const auto& ex) {
    auto fe = dynamic_cast<const fizz::FizzException*>(&ex);
    ASSERT_TRUE(fe);
    ASSERT_TRUE(fe->getAlert().has_value());
    EXPECT_EQ(fe->getAlert().value(), ad);
  });
}

TEST_F(DefaultCertificateVerifierTest, TestVerifySuccess) {
  verifier_->verify({getPeerCert(leafCertAndKey_)});

  auto ctx = verifier_->verifyWithX509StoreCtx({getPeerCert(leafCertAndKey_)});
  STACK_OF(X509)* certChain = X509_STORE_CTX_get0_chain(ctx.get());
  X509* rootX509 = sk_X509_value(certChain, sk_X509_num(certChain) - 1);
  auto rootCertName = folly::ssl::OpenSSLCertUtils::getCommonName(*rootX509);
  EXPECT_EQ(rootCertName, "root");
}

TEST_F(DefaultCertificateVerifierTest, TestVerifyWithIntermediates) {
  auto subauth = createCert("subauth", true, &rootCertAndKey_);
  auto subleaf = createCert("subleaf", false, &subauth);
  verifier_->verify({getPeerCert(subleaf), getPeerCert(subauth)});

  auto ctx = verifier_->verifyWithX509StoreCtx(
      {getPeerCert(subleaf), getPeerCert(subauth)});
  STACK_OF(X509)* certChain = X509_STORE_CTX_get0_chain(ctx.get());
  X509* rootX509 = sk_X509_value(certChain, sk_X509_num(certChain) - 1);
  auto rootCertName = folly::ssl::OpenSSLCertUtils::getCommonName(*rootX509);
  EXPECT_EQ(rootCertName, "root");
}

TEST_F(DefaultCertificateVerifierTest, TestVerifySelfSignedCert) {
  auto selfsigned = createCert("self", false, nullptr);
  expectThrowWithAlert(
      [&] { verifier_->verify({getPeerCert(selfsigned)}); },
      AlertDescription::bad_certificate);
}

TEST_F(DefaultCertificateVerifierTest, TestVerifySelfSignedCertWithOverride) {
  auto selfsigned = createCert("self", false, nullptr);
  verifier_->setCustomVerifyCallback(
      &DefaultCertificateVerifierTest::allowSelfSignedLeafCertCallback);
  // Will not throw because the override allows for this type of error.
  verifier_->verify({getPeerCert(selfsigned)});
  auto ctx = verifier_->verifyWithX509StoreCtx({getPeerCert(selfsigned)});
  STACK_OF(X509)* certChain = X509_STORE_CTX_get0_chain(ctx.get());
  X509* rootX509 = sk_X509_value(certChain, sk_X509_num(certChain) - 1);
  auto rootCertName = folly::ssl::OpenSSLCertUtils::getCommonName(*rootX509);
  EXPECT_EQ(rootCertName, "self");
}

TEST_F(DefaultCertificateVerifierTest, TestVerifyWithIntermediateMissing) {
  auto subauth = createCert("subauth", true, &rootCertAndKey_);
  auto subleaf = createCert("subleaf", false, &subauth);
  expectThrowWithAlert(
      [&] { verifier_->verify({getPeerCert(subleaf)}); },
      AlertDescription::bad_certificate);
}

TEST_F(
    DefaultCertificateVerifierTest,
    TestVerifyWithIntermediateMissingWithOverride) {
  auto subauth = createCert("subauth", true, &rootCertAndKey_);
  auto subleaf = createCert("subleaf", false, &subauth);
  verifier_->setCustomVerifyCallback(
      &DefaultCertificateVerifierTest::allowSelfSignedLeafCertCallback);
  // The override is irrelevant to the error here. So exception is expected.
  expectThrowWithAlert(
      [&] { verifier_->verify({getPeerCert(subleaf)}); },
      AlertDescription::bad_certificate);
}

TEST_F(DefaultCertificateVerifierTest, TestVerifyWithBadIntermediate) {
  auto subauth = createCert("badsubauth", false, &rootCertAndKey_);
  auto subleaf = createCert("badsubleaf", false, &subauth);
  expectThrowWithAlert(
      [&] { verifier_->verify({getPeerCert(subleaf)}); },
      AlertDescription::bad_certificate);
}

TEST_F(DefaultCertificateVerifierTest, TestVerifyWithBadRoot) {
  auto newroot = createCert("root2", true, nullptr);
  auto subauth = createCert("subauth2", true, &newroot);
  auto subleaf = createCert("leaf2", false, &subauth);
  expectThrowWithAlert(
      [&] { verifier_->verify({getPeerCert(subleaf), getPeerCert(subauth)}); },
      AlertDescription::bad_certificate);
}
TEST_F(DefaultCertificateVerifierTest, TestVerifyWithExpiredLeafTooOld) {
  auto now = std::chrono::system_clock::now();
  auto newLeaf = createCert({
      .cn = "expiredLeaf",
      .issuer = &rootCertAndKey_,
      .notBefore = now - std::chrono::hours(24),
      .notAfter = now - std::chrono::hours(23),
  });

  expectThrowWithAlert(
      [&] { verifier_->verify({getPeerCert(newLeaf)}); },
      AlertDescription::certificate_expired);
}

TEST_F(DefaultCertificateVerifierTest, TestVerifyWithExpiredLeafTooNew) {
  auto now = std::chrono::system_clock::now();
  auto newLeaf = createCert({
      .cn = "expiredLeaf",
      .issuer = &rootCertAndKey_,
      .notBefore = now + std::chrono::hours(24),
      .notAfter = now + std::chrono::hours(25),
  });

  expectThrowWithAlert(
      [&] { verifier_->verify({getPeerCert(newLeaf)}); },
      AlertDescription::certificate_expired);
}
} // namespace test
} // namespace fizz
