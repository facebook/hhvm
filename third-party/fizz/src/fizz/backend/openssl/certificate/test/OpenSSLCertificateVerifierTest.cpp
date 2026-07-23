/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/backend/openssl/certificate/OpenSSLCertificateVerifier.h>
#include <fizz/protocol/test/CertUtil.h>
#include <folly/ssl/OpenSSLCertUtils.h>

using namespace fizz::openssl;

namespace fizz {
namespace test {

class OpenSSLCertificateVerifierTest : public testing::Test {
 public:
  void SetUp() override {
    OpenSSL_add_all_algorithms();
    folly::ssl::X509StoreUniquePtr store(X509_STORE_new());
    ASSERT_TRUE(store);
    rootCertAndKey_ = createCert("root", true, nullptr, KeyType::P256);
    leafCertAndKey_ =
        createCert("leaf", false, &rootCertAndKey_, KeyType::P256);
    ASSERT_EQ(X509_STORE_add_cert(store.get(), rootCertAndKey_.cert.get()), 1);
    Error err;
    std::unique_ptr<OpenSSLCertificateVerifier> verifier;
    ASSERT_EQ(
        OpenSSLCertificateVerifier::create(
            verifier, err, VerificationContext::Client, std::move(store)),
        Status::Success);
    verifier_ = std::move(verifier);
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
  std::unique_ptr<OpenSSLCertificateVerifier> verifier_;
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

TEST_F(OpenSSLCertificateVerifierTest, TestVerifySuccess) {
  Error err;
  std::shared_ptr<const Cert> verifiedCert;
  EXPECT_EQ(
      verifier_->verify(verifiedCert, err, {getPeerCert(leafCertAndKey_)}),
      Status::Success);

  folly::ssl::X509StoreCtxUniquePtr ctx;
  EXPECT_EQ(
      verifier_->verifyWithX509StoreCtx(
          ctx, err, {getPeerCert(leafCertAndKey_)}),
      Status::Success);
  STACK_OF(X509)* certChain = X509_STORE_CTX_get0_chain(ctx.get());
  X509* rootX509 = sk_X509_value(certChain, sk_X509_num(certChain) - 1);
  auto rootCertName = folly::ssl::OpenSSLCertUtils::getCommonName(*rootX509);
  EXPECT_EQ(rootCertName, "root");
}

TEST_F(OpenSSLCertificateVerifierTest, TestVerifyWithIntermediates) {
  auto subauth = createCert("subauth", true, &rootCertAndKey_, KeyType::P256);
  auto subleaf = createCert("subleaf", false, &subauth, KeyType::P256);

  Error err;
  std::shared_ptr<const Cert> verifiedCert;
  EXPECT_EQ(
      verifier_->verify(
          verifiedCert, err, {getPeerCert(subleaf), getPeerCert(subauth)}),
      Status::Success);

  folly::ssl::X509StoreCtxUniquePtr ctx;
  EXPECT_EQ(
      verifier_->verifyWithX509StoreCtx(
          ctx, err, {getPeerCert(subleaf), getPeerCert(subauth)}),
      Status::Success);
  STACK_OF(X509)* certChain = X509_STORE_CTX_get0_chain(ctx.get());
  X509* rootX509 = sk_X509_value(certChain, sk_X509_num(certChain) - 1);
  auto rootCertName = folly::ssl::OpenSSLCertUtils::getCommonName(*rootX509);
  EXPECT_EQ(rootCertName, "root");
}

TEST_F(OpenSSLCertificateVerifierTest, TestVerifySelfSignedCert) {
  auto selfsigned = createCert("self", false, nullptr, KeyType::P256);
  expectThrowWithAlert(
      [&] {
        Error err;
        std::shared_ptr<const Cert> verifiedCert;
        FIZZ_THROW_ON_ERROR(
            verifier_->verify(verifiedCert, err, {getPeerCert(selfsigned)}),
            err);
      },
      AlertDescription::unknown_ca);
}

TEST_F(OpenSSLCertificateVerifierTest, TestVerifySelfSignedCertWithOverride) {
  auto selfsigned = createCert("self", false, nullptr, KeyType::P256);
  verifier_->setCustomVerifyCallback(
      &OpenSSLCertificateVerifierTest::allowSelfSignedLeafCertCallback);
  // Will not throw because the override allows for this type of error.
  Error err;
  std::shared_ptr<const Cert> verifiedCert;
  EXPECT_EQ(
      verifier_->verify(verifiedCert, err, {getPeerCert(selfsigned)}),
      Status::Success);

  folly::ssl::X509StoreCtxUniquePtr ctx;
  EXPECT_EQ(
      verifier_->verifyWithX509StoreCtx(ctx, err, {getPeerCert(selfsigned)}),
      Status::Success);
  STACK_OF(X509)* certChain = X509_STORE_CTX_get0_chain(ctx.get());
  X509* rootX509 = sk_X509_value(certChain, sk_X509_num(certChain) - 1);
  auto rootCertName = folly::ssl::OpenSSLCertUtils::getCommonName(*rootX509);
  EXPECT_EQ(rootCertName, "self");
}

TEST_F(OpenSSLCertificateVerifierTest, TestVerifyWithIntermediateMissing) {
  auto subauth = createCert("subauth", true, &rootCertAndKey_, KeyType::P256);
  auto subleaf = createCert("subleaf", false, &subauth, KeyType::P256);
  expectThrowWithAlert(
      [&] {
        Error err;
        std::shared_ptr<const Cert> verifiedCert;
        FIZZ_THROW_ON_ERROR(
            verifier_->verify(verifiedCert, err, {getPeerCert(subleaf)}), err);
      },
      AlertDescription::unknown_ca);
}

TEST_F(
    OpenSSLCertificateVerifierTest,
    TestVerifyWithIntermediateMissingWithOverride) {
  auto subauth = createCert("subauth", true, &rootCertAndKey_, KeyType::P256);
  auto subleaf = createCert("subleaf", false, &subauth, KeyType::P256);
  verifier_->setCustomVerifyCallback(
      &OpenSSLCertificateVerifierTest::allowSelfSignedLeafCertCallback);
  // The override asserts that self signed certs (chain length = 1) are
  // accepted. But since this certificate is not self signed, this override
  // should effectively be a no-op and normal certificate verification
  // should take place.
  expectThrowWithAlert(
      [&] {
        Error err;
        std::shared_ptr<const Cert> verifiedCert;
        FIZZ_THROW_ON_ERROR(
            verifier_->verify(verifiedCert, err, {getPeerCert(subleaf)}), err);
      },
      AlertDescription::unknown_ca);
}

TEST_F(OpenSSLCertificateVerifierTest, TestVerifyWithBadIntermediate) {
  auto subauth =
      createCert("badsubauth", false, &rootCertAndKey_, KeyType::P256);
  auto subleaf = createCert("badsubleaf", false, &subauth, KeyType::P256);
  expectThrowWithAlert(
      [&] {
        Error err;
        std::shared_ptr<const Cert> verifiedCert;
        FIZZ_THROW_ON_ERROR(
            verifier_->verify(verifiedCert, err, {getPeerCert(subleaf)}), err);
      },
      AlertDescription::unknown_ca);
}

TEST_F(OpenSSLCertificateVerifierTest, TestVerifyWithBadRoot) {
  auto newroot = createCert("root2", true, nullptr, KeyType::P256);
  auto subauth = createCert("subauth2", true, &newroot, KeyType::P256);
  auto subleaf = createCert("leaf2", false, &subauth, KeyType::P256);
  expectThrowWithAlert(
      [&] {
        Error err;
        std::shared_ptr<const Cert> verifiedCert;
        FIZZ_THROW_ON_ERROR(
            verifier_->verify(
                verifiedCert,
                err,
                {getPeerCert(subleaf), getPeerCert(subauth)}),
            err);
      },
      AlertDescription::unknown_ca);
}
TEST_F(OpenSSLCertificateVerifierTest, TestVerifyWithExpiredLeafTooOld) {
  auto now = std::chrono::system_clock::now();
  auto newLeaf = createCert({
      .cn = "expiredLeaf",
      .issuer = &rootCertAndKey_,
      .notBefore = now - std::chrono::hours(24),
      .notAfter = now - std::chrono::hours(23),
      .keyType = KeyType::P256,
  });

  expectThrowWithAlert(
      [&] {
        Error err;
        std::shared_ptr<const Cert> verifiedCert;
        FIZZ_THROW_ON_ERROR(
            verifier_->verify(verifiedCert, err, {getPeerCert(newLeaf)}), err);
      },
      AlertDescription::certificate_expired);
}

TEST_F(OpenSSLCertificateVerifierTest, TestVerifyWithExpiredLeafTooNew) {
  auto now = std::chrono::system_clock::now();
  auto newLeaf = createCert({
      .cn = "expiredLeaf",
      .issuer = &rootCertAndKey_,
      .notBefore = now + std::chrono::hours(24),
      .notAfter = now + std::chrono::hours(25),
      .keyType = KeyType::P256,
  });

  expectThrowWithAlert(
      [&] {
        Error err;
        std::shared_ptr<const Cert> verifiedCert;
        FIZZ_THROW_ON_ERROR(
            verifier_->verify(verifiedCert, err, {getPeerCert(newLeaf)}), err);
      },
      AlertDescription::certificate_expired);
}
} // namespace test
} // namespace fizz
