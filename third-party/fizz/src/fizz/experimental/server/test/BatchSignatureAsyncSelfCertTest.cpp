/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/certificate/OpenSSLPeerCertImpl.h>
#include <fizz/backend/openssl/certificate/OpenSSLSelfCertImpl.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/experimental/batcher/Batcher.h>
#include <fizz/experimental/server/BatchSignatureAsyncSelfCert.h>
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
  CertificateMsg certMessage;
  Error certMsgErr;
  EXPECT_EQ(
      batchCert.getCertMessage(certMessage, certMsgErr, nullptr),
      Status::Success);
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
  EXPECT_CALL(*mockBaseCert, _sign(_, _, _))
      .Times(3)
      .WillRepeatedly(Invoke(
          [](SignatureScheme, CertificateVerifyContext, folly::ByteRange) {
            return folly::IOBuf::copyBuffer("mockSignature");
          }));
  // sign and signFuture with non-batch scheme
  // expect call of base certificate's sign
  Buf sig1;
  Error err;
  EXPECT_EQ(
      batchCert.sign(
          sig1,
          err,
          SignatureScheme::ecdsa_secp256r1_sha256,
          CertificateVerifyContext::Server,
          folly::range(folly::StringPiece("hello"))),
      Status::Success);
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
  Buf sig3;
  EXPECT_EQ(
      batchCert.sign(
          sig3,
          err,
          SignatureScheme::ecdsa_secp256r1_sha256_batch,
          CertificateVerifyContext::Server,
          folly::range(folly::StringPiece("hello"))),
      Status::Success);
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
  Buf sig1;
  Error err;
  EXPECT_EQ(
      batchCert.sign(
          sig1,
          err,
          SignatureScheme::ecdsa_secp256r1_sha256,
          CertificateVerifyContext::Server,
          folly::range(folly::StringPiece("hello"))),
      Status::Success);
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
  std::unique_ptr<openssl::OpenSSLSelfCertImpl<openssl::KeyType::P256>>
      certUniq;
  Error err;
  EXPECT_EQ(
      openssl::OpenSSLSelfCertImpl<openssl::KeyType::P256>::create(
          certUniq, err, getPrivateKey(kP256Key), std::move(certs)),
      Status::Success);
  auto certificate = std::shared_ptr<SelfCert>(std::move(certUniq));
  auto batcher = std::make_shared<SynchronizedBatcher<Sha256>>(
      1, certificate, CertificateVerifyContext::Server);
  BatchSignatureAsyncSelfCert<Sha256> batchCert(batcher);

  // non-batch signature
  auto signature = batchCert.signFuture(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      folly::IOBuf::copyBuffer(folly::StringPiece("Message1")));
  std::unique_ptr<openssl::OpenSSLPeerCertImpl<openssl::KeyType::P256>>
      peerCert;
  EXPECT_EQ(
      openssl::OpenSSLPeerCertImpl<openssl::KeyType::P256>::create(
          peerCert, err, getCert(kP256Certificate)),
      Status::Success);
  EXPECT_EQ(
      peerCert->verify(
          err,
          SignatureScheme::ecdsa_secp256r1_sha256,
          CertificateVerifyContext::Server,
          folly::range(folly::StringPiece("Message1")),
          (*std::move(signature).get())->coalesce()),
      Status::Success);

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
  std::unique_ptr<openssl::OpenSSLSelfCertImpl<openssl::KeyType::RSA>> certUniq;
  Error err;
  EXPECT_EQ(
      openssl::OpenSSLSelfCertImpl<openssl::KeyType::RSA>::create(
          certUniq, err, getPrivateKey(kRSAKey), std::move(certs)),
      Status::Success);
  auto certificate = std::shared_ptr<SelfCert>(std::move(certUniq));
  auto batcher = std::make_shared<SynchronizedBatcher<Sha256>>(
      1, certificate, CertificateVerifyContext::Server);
  BatchSignatureAsyncSelfCert<Sha256> batchCert(batcher);

  // non-batch signature
  auto signature = batchCert.signFuture(
      SignatureScheme::rsa_pss_sha256,
      CertificateVerifyContext::Server,
      folly::IOBuf::copyBuffer(folly::StringPiece("Message1")));
  std::unique_ptr<openssl::OpenSSLPeerCertImpl<openssl::KeyType::RSA>> peerCert;
  EXPECT_EQ(
      openssl::OpenSSLPeerCertImpl<openssl::KeyType::RSA>::create(
          peerCert, err, getCert(kRSACertificate)),
      Status::Success);
  EXPECT_EQ(
      peerCert->verify(
          err,
          SignatureScheme::rsa_pss_sha256,
          CertificateVerifyContext::Server,
          folly::range(folly::StringPiece("Message1")),
          (*std::move(signature).get())->coalesce()),
      Status::Success);

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
  std::unique_ptr<openssl::OpenSSLSelfCertImpl<openssl::KeyType::P384>>
      certUniq;
  Error err;
  EXPECT_EQ(
      openssl::OpenSSLSelfCertImpl<openssl::KeyType::P384>::create(
          certUniq, err, getPrivateKey(kP384Key), std::move(certs)),
      Status::Success);
  auto certificate = std::shared_ptr<SelfCert>(std::move(certUniq));
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
