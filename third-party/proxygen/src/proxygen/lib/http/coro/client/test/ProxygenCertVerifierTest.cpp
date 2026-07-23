/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/ProxygenCertVerifier.h"

#include <fizz/backend/openssl/certificate/OpenSSLCertificateVerifier.h>
#include <fizz/protocol/test/CertUtil.h>
#include <folly/portability/GTest.h>
#include <folly/portability/OpenSSL.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

#include <gmock/gmock.h>

using namespace fizz;
using namespace fizz::test;
using testing::HasSubstr;

namespace proxygen::coro::test {

class ProxygenCertVerifierTest : public testing::Test {
 public:
  void SetUp() override {
    rootCertAndKey_ =
        createCert("root", /*ca=*/true, /*issuer=*/nullptr, KeyType::P256);
    // Leaf cert with CN and SAN "example.com" (createCert adds the CN as a
    // SAN).
    leafCertAndKey_ = createCert(
        "example.com", /*ca=*/false, &rootCertAndKey_, KeyType::P256);
  }

 protected:
  std::shared_ptr<fizz::CertificateVerifier> makeVerifier(
      ExpectedIdentity expectedIdentity, ValidationPolicy policy) {
    folly::ssl::X509StoreUniquePtr store(X509_STORE_new());
    EXPECT_EQ(X509_STORE_add_cert(store.get(), rootCertAndKey_.cert.get()), 1);
    Error err;
    std::unique_ptr<openssl::OpenSSLCertificateVerifier> innerVerifier;
    EXPECT_EQ(
        openssl::OpenSSLCertificateVerifier::create(
            innerVerifier, err, VerificationContext::Client, std::move(store)),
        Status::Success);
    std::shared_ptr<const fizz::CertificateVerifier> inner =
        std::move(innerVerifier);
    return coro::makeVerifier(
        std::move(inner), std::move(expectedIdentity), policy, nullptr);
  }

  CertAndKey rootCertAndKey_;
  CertAndKey leafCertAndKey_;
};

TEST_F(ProxygenCertVerifierTest, MatchingHostnameSucceeds) {
  auto verifier = makeVerifier(ExpectedIdentity::expectDNS("example.com"),
                               ValidationPolicy::Enforcing);
  Error err;
  std::shared_ptr<const Cert> verifiedCert;
  EXPECT_EQ(verifier->verify(verifiedCert, err, {getPeerCert(leafCertAndKey_)}),
            Status::Success);
}

TEST_F(ProxygenCertVerifierTest, MismatchedHostnameFails) {
  auto verifier = makeVerifier(ExpectedIdentity::expectDNS("wrong.example.com"),
                               ValidationPolicy::Enforcing);
  Error err;
  std::shared_ptr<const Cert> verifiedCert;
  EXPECT_NE(verifier->verify(verifiedCert, err, {getPeerCert(leafCertAndKey_)}),
            Status::Success);
  EXPECT_THAT(err.msg(), HasSubstr("identity verification failed"));
  EXPECT_THAT(err.msg(), HasSubstr("expected=wrong.example.com"));
}

TEST_F(ProxygenCertVerifierTest, MismatchedHostnameWithLoggingPolicy) {
  auto verifier = makeVerifier(ExpectedIdentity::expectDNS("wrong.example.com"),
                               ValidationPolicy::Logging);
  Error err;
  std::shared_ptr<const Cert> verifiedCert;
  EXPECT_EQ(verifier->verify(verifiedCert, err, {getPeerCert(leafCertAndKey_)}),
            Status::Success);
}

TEST_F(ProxygenCertVerifierTest, MismatchedIpFails) {
  auto verifier =
      makeVerifier(ExpectedIdentity::expectIP(folly::IPAddress("127.0.0.1")),
                   ValidationPolicy::Enforcing);
  Error err;
  std::shared_ptr<const Cert> verifiedCert;
  EXPECT_NE(verifier->verify(verifiedCert, err, {getPeerCert(leafCertAndKey_)}),
            Status::Success);
  EXPECT_THAT(err.msg(), HasSubstr("identity verification failed"));
  EXPECT_THAT(err.msg(), HasSubstr("expected=127.0.0.1"));
}

TEST_F(ProxygenCertVerifierTest, UnderlyingVerifierFailure) {
  auto untrustedRoot = createCert(
      "untrusted-root", /*ca=*/true, /*issuer=*/nullptr, KeyType::P256);
  auto untrustedLeaf =
      createCert("example.com", /*ca=*/false, &untrustedRoot, KeyType::P256);

  auto verifier = makeVerifier(ExpectedIdentity::expectDNS("example.com"),
                               ValidationPolicy::Logging);
  Error err;
  std::shared_ptr<const Cert> verifiedCert;
  EXPECT_EQ(verifier->verify(verifiedCert, err, {getPeerCert(untrustedLeaf)}),
            Status::Fail);
}

TEST_F(ProxygenCertVerifierTest, NullInnerVerifierFailsAtConstruction) {
  EXPECT_DEATH(coro::makeVerifier(nullptr,
                                  ExpectedIdentity::expectDNS("example.com"),
                                  ValidationPolicy::Logging,
                                  nullptr),
               testing::_);
}

} // namespace proxygen::coro::test
