/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/test/TestUtil.h>
#include <fizz/experimental/batcher/Batcher.h>
#include <fizz/experimental/server/BatchSignatureAsyncSelfCert.h>
#include <fizz/protocol/OpenSSLPeerCertImpl.h>
#include <fizz/protocol/OpenSSLSelfCertImpl.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/server/test/Mocks.h>
#include <folly/portability/GTest.h>

namespace fizz {
namespace server {
namespace test {

TEST(BatchSignatureAsyncSelfCertTest, TestDecoratorLogicWithMockCert) {
  auto mockBaseCert = std::make_shared<MockSelfCert>();
  std::vector<SignatureScheme> schemes = {SignatureScheme::rsa_pss_sha256};
  EXPECT_CALL(*mockBaseCert, getSigSchemes())
      .Times(2)
      .WillRepeatedly(Return(schemes));
  auto batcher = std::make_shared<SynchronizedBatcher<Sha256>>(
      1, mockBaseCert, CertificateVerifyContext::Server);
  BatchSignatureAsyncSelfCert<Sha256> batchCert(batcher);
  EXPECT_EQ(batchCert.getSigner(), mockBaseCert);
  // getIdentity
  EXPECT_CALL(*mockBaseCert, getIdentity())
      .Times(1)
      .WillOnce(Return("identity"));
  EXPECT_EQ(batchCert.getIdentity(), "identity");
  // getAltIdentities
  std::vector<std::string> altIdentities = {"0", "1"};
  EXPECT_CALL(*mockBaseCert, getAltIdentities())
      .Times(1)
      .WillOnce(Return(altIdentities));
  altIdentities = batchCert.getAltIdentities();
  EXPECT_EQ(altIdentities[0], "0");
  EXPECT_EQ(altIdentities[1], "1");
  // getCertMessage
  EXPECT_CALL(*mockBaseCert, _getCertMessage(_)).Times(1);
  auto certMessage = batchCert.getCertMessage();
  // getSigSchemes
  auto returnedSchemes = batchCert.getSigSchemes();
  EXPECT_EQ(returnedSchemes.size(), 2);
  EXPECT_EQ(returnedSchemes[0], SignatureScheme::rsa_pss_sha256);
  EXPECT_EQ(returnedSchemes[1], SignatureScheme::rsa_pss_sha256_batch);
  // getCompressedCert
  EXPECT_CALL(
      *mockBaseCert, getCompressedCert(CertificateCompressionAlgorithm::brotli))
      .Times(1);
  auto compressedCert =
      batchCert.getCompressedCert(CertificateCompressionAlgorithm::brotli);
  // getX509
  EXPECT_CALL(*mockBaseCert, getX509()).Times(1);
  auto x509Cert = batchCert.getX509();
  // sign
  EXPECT_CALL(*mockBaseCert, sign(_, _, _))
      .Times(3)
      .WillRepeatedly(Invoke(
          [](SignatureScheme, CertificateVerifyContext, folly::ByteRange) {
            return folly::IOBuf::copyBuffer("mockSignature");
          }));
  // sign and signFuture with non-batch scheme
  // expect call of base certificate's sign
  auto sig1 = batchCert.sign(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      folly::range(folly::StringPiece("hello")));
  EXPECT_EQ(sig1->moveToFbString(), folly::fbstring("mockSignature"));
  // expect call of base certificate's sign
  auto sig2 = batchCert.signFuture(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      folly::IOBuf::copyBuffer(folly::StringPiece("hello")));
  EXPECT_EQ(
      sig2.value().value()->moveToFbString(), folly::fbstring("mockSignature"));
  // sign with batch scheme
  // expect call of base certificate's sign but with batch scheme
  batchCert.sign(
      SignatureScheme::ecdsa_secp256r1_sha256_batch,
      CertificateVerifyContext::Server,
      folly::range(folly::StringPiece("hello")));
}

TEST(BatchSignatureAsyncSelfCertTest, TestDecoratorLogicWithMockAsyncCert) {
  auto mockBaseCert = std::make_shared<MockAsyncSelfCert>();
  auto batcher = std::make_shared<SynchronizedBatcher<Sha256>>(
      1, mockBaseCert, CertificateVerifyContext::Server);
  BatchSignatureAsyncSelfCert<Sha256> batchCert(batcher);
  EXPECT_EQ(batchCert.getSigner(), mockBaseCert);
  // sign
  EXPECT_CALL(
      *mockBaseCert, signFuture(SignatureScheme::ecdsa_secp256r1_sha256, _, _))
      .Times(1)
      .WillRepeatedly(Invoke([](SignatureScheme,
                                CertificateVerifyContext,
                                std::unique_ptr<folly::IOBuf>) {
        return folly::IOBuf::copyBuffer("mockSignature");
      }));
  // sign will not invoke signer's signFuture
  auto sig1 = batchCert.sign(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      folly::range(folly::StringPiece("hello")));
  // signFuture will invoke signer's signFuture
  auto sig2 = batchCert.signFuture(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      folly::IOBuf::copyBuffer(folly::StringPiece("hello")));
}

TEST(BatchSignatureAsyncSelfCertTest, TestSignAndVerifyP256) {
  // sign
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.emplace_back(getCert(kP256Certificate));
  auto certificate = std::make_shared<OpenSSLSelfCertImpl<KeyType::P256>>(
      getPrivateKey(kP256Key), std::move(certs));
  auto batcher = std::make_shared<SynchronizedBatcher<Sha256>>(
      1, certificate, CertificateVerifyContext::Server);
  BatchSignatureAsyncSelfCert<Sha256> batchCert(batcher);

  // non-batch signature
  auto signature = batchCert.signFuture(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      folly::IOBuf::copyBuffer(folly::StringPiece("Message1")));
  OpenSSLPeerCertImpl<KeyType::P256> peerCert(getCert(kP256Certificate));
  peerCert.verify(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      folly::range(folly::StringPiece("Message1")),
      (*std::move(signature).get())->coalesce());

  // thow when signing a signature scheme that is not supported by base SelfCert
  EXPECT_THROW(
      batchCert.signFuture(
          SignatureScheme::rsa_pss_sha256,
          CertificateVerifyContext::Server,
          folly::IOBuf::copyBuffer(folly::StringPiece("Message1"))),
      std::runtime_error);
  EXPECT_THROW(
      batchCert.signFuture(
          SignatureScheme::rsa_pss_sha256_batch,
          CertificateVerifyContext::Server,
          folly::IOBuf::copyBuffer(folly::StringPiece("Message1"))),
      std::runtime_error);
}

TEST(BatchSignatureAsyncSelfCertTest, TestSignAndVerifyRSA) {
  // sign
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.emplace_back(getCert(kRSACertificate));
  auto certificate = std::make_shared<OpenSSLSelfCertImpl<KeyType::RSA>>(
      getPrivateKey(kRSAKey), std::move(certs));
  auto batcher = std::make_shared<SynchronizedBatcher<Sha256>>(
      1, certificate, CertificateVerifyContext::Server);
  BatchSignatureAsyncSelfCert<Sha256> batchCert(batcher);

  // non-batch signature
  auto signature = batchCert.signFuture(
      SignatureScheme::rsa_pss_sha256,
      CertificateVerifyContext::Server,
      folly::IOBuf::copyBuffer(folly::StringPiece("Message1")));
  OpenSSLPeerCertImpl<KeyType::RSA> peerCert(getCert(kRSACertificate));
  peerCert.verify(
      SignatureScheme::rsa_pss_sha256,
      CertificateVerifyContext::Server,
      folly::range(folly::StringPiece("Message1")),
      (*std::move(signature).get())->coalesce());

  // thow when signing a signature scheme that is not supported by base SelfCert
  EXPECT_THROW(
      batchCert.signFuture(
          SignatureScheme::ecdsa_secp256r1_sha256,
          CertificateVerifyContext::Server,
          folly::IOBuf::copyBuffer(folly::StringPiece("Message1"))),
      std::runtime_error);
  EXPECT_THROW(
      batchCert.signFuture(
          SignatureScheme::ecdsa_secp256r1_sha256_batch,
          CertificateVerifyContext::Server,
          folly::IOBuf::copyBuffer(folly::StringPiece("Message1"))),
      std::runtime_error);
}

TEST(BatchSignatureAsyncSelfCertTest, TestUnsuportedHash) {
  // sign
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.emplace_back(getCert(kP384Certificate));
  auto certificate = std::make_shared<OpenSSLSelfCertImpl<KeyType::P384>>(
      getPrivateKey(kP384Key), std::move(certs));
  auto batcher = std::make_shared<SynchronizedBatcher<Sha256>>(
      1, certificate, CertificateVerifyContext::Server);
  BatchSignatureAsyncSelfCert<Sha256> batchCert(batcher);

  // thow when signing a batch scheme whose Hash doesn't match
  EXPECT_THROW(
      batchCert.signFuture(
          SignatureScheme::ecdsa_secp384r1_sha384_batch,
          CertificateVerifyContext::Server,
          folly::IOBuf::copyBuffer(folly::StringPiece("Message1"))),
      std::runtime_error);
}

} // namespace test
} // namespace server
} // namespace fizz
