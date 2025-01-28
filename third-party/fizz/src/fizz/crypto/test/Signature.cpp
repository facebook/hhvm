/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/test/Signature.h>

#include <folly/Range.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>

using namespace testing;

namespace fizz {
namespace test {

std::unique_ptr<folly::IOBuf> makeCertBuf(std::string certDer) {
  return folly::IOBuf::copyBuffer(certDer.data(), certDer.size());
}

void testCertVerify(
    SignatureTestData testCase,
    std::unique_ptr<PeerCert> (*makePeerCert)(Buf)) {
  std::string certDer = folly::unhexlify(testCase.certDer);
  std::string msg = folly::unhexlify(testCase.msg);
  std::string sig = folly::unhexlify(testCase.sig);

  std::unique_ptr<folly::IOBuf> certBuf = makeCertBuf(certDer);

  if (!testCase.validCert) {
    EXPECT_THROW(makePeerCert(makeCertBuf(certDer)), std::runtime_error);
    return;
  }

  // make sure move works
  auto tempPeerCert = makePeerCert(makeCertBuf(certDer));

  auto peerCert = std::move(tempPeerCert);

  // test getDER()
  auto retDer = peerCert->getDER();
  ASSERT_TRUE(retDer.has_value());

  ASSERT_EQ(memcmp(certDer.c_str(), retDer.value().c_str(), certDer.size()), 0);

  if (!testCase.validSig) {
    EXPECT_THROW(
        peerCert->verify(
            testCase.sigScheme,
            fizz::CertificateVerifyContext::Server,
            folly::ByteRange(
                reinterpret_cast<const unsigned char*>(msg.c_str()),
                msg.size()),
            folly::ByteRange(
                reinterpret_cast<const unsigned char*>(sig.c_str()),
                sig.size())),
        std::runtime_error);
  } else {
    EXPECT_NO_THROW(peerCert->verify(
        testCase.sigScheme,
        fizz::CertificateVerifyContext::Server,
        folly::ByteRange(
            reinterpret_cast<const unsigned char*>(msg.c_str()), msg.size()),
        folly::ByteRange(
            reinterpret_cast<const unsigned char*>(sig.c_str()), sig.size())));
  }
}
} // namespace test
} // namespace fizz
