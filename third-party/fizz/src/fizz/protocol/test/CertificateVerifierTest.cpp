/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/CertificateVerifier.h>
#include <fizz/protocol/test/Mocks.h>
#include <folly/portability/GTest.h>

namespace fizz {
namespace test {

TEST(TerminatingCertificateVerifierTest, TerminatesOnVerify) {
  TerminatingCertificateVerifier verifier(VerificationContext::Client);
  auto cert = std::make_shared<MockPeerCert>();
  std::vector<std::shared_ptr<const PeerCert>> certs{cert};
  EXPECT_DEATH(verifier.verify(certs), "not supported on this platform");
}

TEST(TerminatingCertificateVerifierTest, GetExtensionsReturnsEmpty) {
  TerminatingCertificateVerifier verifier(VerificationContext::Client);
  EXPECT_TRUE(verifier.getCertificateRequestExtensions().empty());
}

TEST(InsecureCertificateVerifierTest, ReturnsFirstCert) {
  InsecureCertificateVerifier verifier(VerificationContext::Client);
  auto cert = std::make_shared<MockPeerCert>();
  std::vector<std::shared_ptr<const PeerCert>> certs{cert};
  EXPECT_EQ(verifier.verify(certs), cert);
}

TEST(InsecureCertificateVerifierTest, ReturnsNullOnEmpty) {
  InsecureCertificateVerifier verifier(VerificationContext::Client);
  std::vector<std::shared_ptr<const PeerCert>> certs;
  EXPECT_EQ(verifier.verify(certs), nullptr);
}

TEST(InsecureCertificateVerifierTest, GetExtensionsReturnsEmpty) {
  InsecureCertificateVerifier verifier(VerificationContext::Client);
  EXPECT_TRUE(verifier.getCertificateRequestExtensions().empty());
}

} // namespace test
} // namespace fizz
