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
  std::shared_ptr<const Cert> ret;
  Error err;
  EXPECT_DEATH(
      (void)verifier.verify(ret, err, certs), "not supported on this platform");
}

TEST(TerminatingCertificateVerifierTest, GetExtensionsReturnsEmpty) {
  TerminatingCertificateVerifier verifier(VerificationContext::Client);
  std::vector<Extension> ret;
  Error err;
  EXPECT_EQ(
      verifier.getCertificateRequestExtensions(ret, err), Status::Success);
  EXPECT_TRUE(ret.empty());
}

TEST(InsecureCertificateVerifierTest, ReturnsFirstCert) {
  InsecureCertificateVerifier verifier(VerificationContext::Client);
  auto cert = std::make_shared<MockPeerCert>();
  std::vector<std::shared_ptr<const PeerCert>> certs{cert};
  std::shared_ptr<const Cert> ret;
  Error err;
  EXPECT_EQ(verifier.verify(ret, err, certs), Status::Success);
  EXPECT_EQ(ret, cert);
}

TEST(InsecureCertificateVerifierTest, ReturnsNullOnEmpty) {
  InsecureCertificateVerifier verifier(VerificationContext::Client);
  std::vector<std::shared_ptr<const PeerCert>> certs;
  std::shared_ptr<const Cert> ret;
  Error err;
  EXPECT_EQ(verifier.verify(ret, err, certs), Status::Success);
  EXPECT_EQ(ret, nullptr);
}

TEST(InsecureCertificateVerifierTest, GetExtensionsReturnsEmpty) {
  InsecureCertificateVerifier verifier(VerificationContext::Client);
  std::vector<Extension> ret;
  Error err;
  EXPECT_EQ(
      verifier.getCertificateRequestExtensions(ret, err), Status::Success);
  EXPECT_TRUE(ret.empty());
}

} // namespace test
} // namespace fizz
