/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/test/TestUtil.h>
#include <fizz/experimental/batcher/Batcher.h>
#include <fizz/experimental/client/BatchSignaturePeerCert.h>
#include <fizz/experimental/server/BatchSignatureAsyncSelfCert.h>
#include <fizz/protocol/OpenSSLPeerCertImpl.h>
#include <fizz/protocol/OpenSSLSelfCertImpl.h>
#include <fizz/protocol/test/Mocks.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/portability/GTest.h>

using namespace fizz::test;

namespace fizz {
namespace client {
namespace test {

TEST(BatchSignaturePeerCertTest, TestDecoratorLogicWithMockCert) {
  auto mockBaseCert = std::make_shared<MockPeerCert>();
  BatchSignaturePeerCert batchCert(mockBaseCert);
  // getIdentity
  EXPECT_CALL(*mockBaseCert, getIdentity())
      .Times(1)
      .WillOnce(Return("identity"));
  EXPECT_EQ(batchCert.getIdentity(), "identity");
  // getX509
  EXPECT_CALL(*mockBaseCert, getX509()).Times(1);
  auto x509Cert = batchCert.getX509();
  // verify
  int counter = 0;
  EXPECT_CALL(
      *mockBaseCert, verify(SignatureScheme::ecdsa_secp256r1_sha256, _, _, _))
      .Times(1)
      .WillOnce(Invoke([&](SignatureScheme,
                           CertificateVerifyContext,
                           folly::ByteRange msg,
                           folly::ByteRange) {
        if (counter == 0) {
          EXPECT_EQ(msg.toString(), "hello");
        } else if (counter == 1) {
          EXPECT_TRUE(msg.toString() != "hello");
        }
        counter++;
      }));
  batchCert.verify(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      folly::range(folly::StringPiece("hello")),
      folly::range(folly::StringPiece("fakeSignature")));
}

TEST(BatchSignaturePeerCertTest, TestSignVerifyP256) {
  // sign
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.emplace_back(getCert(kP256Certificate));
  auto certificate = std::make_shared<OpenSSLSelfCertImpl<KeyType::P256>>(
      getPrivateKey(kP256Key), std::move(certs));
  auto batcher = std::make_shared<SynchronizedBatcher<Sha256>>(
      1, certificate, CertificateVerifyContext::Server);
  BatchSignatureAsyncSelfCert<Sha256> batchSelfCert(batcher);
  folly::ManualExecutor executor;
  auto signature1 = batchSelfCert.signFuture(
      SignatureScheme::ecdsa_secp256r1_sha256_batch,
      CertificateVerifyContext::Server,
      folly::IOBuf::copyBuffer(folly::StringPiece("Message1")));
  executor.drain();

  // verify
  auto peerCert = std::make_shared<OpenSSLPeerCertImpl<KeyType::P256>>(
      getCert(kP256Certificate));
  BatchSignaturePeerCert batchPeerCert(peerCert);
  batchPeerCert.verify(
      SignatureScheme::ecdsa_secp256r1_sha256_batch,
      CertificateVerifyContext::Server,
      folly::range(folly::StringPiece("Message1")),
      (*std::move(signature1).get())->coalesce());

  // non-batch signature
  auto signature2 = batchSelfCert.signFuture(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      folly::IOBuf::copyBuffer(folly::StringPiece("Message1")));
  batchPeerCert.verify(
      SignatureScheme::ecdsa_secp256r1_sha256,
      CertificateVerifyContext::Server,
      folly::range(folly::StringPiece("Message1")),
      (*std::move(signature2).get())->coalesce());
}

TEST(BatchSignaturePeerCertTest, TestSignVerifyRSA) {
  // sign
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.emplace_back(getCert(kRSACertificate));
  auto certificate = std::make_shared<OpenSSLSelfCertImpl<KeyType::RSA>>(
      getPrivateKey(kRSAKey), std::move(certs));
  auto batcher = std::make_shared<SynchronizedBatcher<Sha256>>(
      1, certificate, CertificateVerifyContext::Server);
  BatchSignatureAsyncSelfCert<Sha256> batchSelfCert(batcher);
  folly::ManualExecutor executor;
  auto signature1 = batchSelfCert.signFuture(
      SignatureScheme::rsa_pss_sha256_batch,
      CertificateVerifyContext::Server,
      folly::IOBuf::copyBuffer(folly::StringPiece("Message1")));
  executor.drain();

  // verify
  auto peerCert = std::make_shared<OpenSSLPeerCertImpl<KeyType::RSA>>(
      getCert(kRSACertificate));
  BatchSignaturePeerCert batchPeerCert(peerCert);
  batchPeerCert.verify(
      SignatureScheme::rsa_pss_sha256_batch,
      CertificateVerifyContext::Server,
      folly::range(folly::StringPiece("Message1")),
      (*std::move(signature1).get())->coalesce());

  // non-batch signature
  auto signature2 = batchSelfCert.signFuture(
      SignatureScheme::rsa_pss_sha256,
      CertificateVerifyContext::Server,
      folly::IOBuf::copyBuffer(folly::StringPiece("Message1")));
  batchPeerCert.verify(
      SignatureScheme::rsa_pss_sha256,
      CertificateVerifyContext::Server,
      folly::range(folly::StringPiece("Message1")),
      (*std::move(signature2).get())->coalesce());
}

TEST(BatchSignaturePeerCertTest, TestWrongBatchSignature) {
  // sign
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.emplace_back(getCert(kP256Certificate));
  auto certificate = std::make_shared<OpenSSLSelfCertImpl<KeyType::P256>>(
      getPrivateKey(kP256Key), std::move(certs));
  auto batcher = std::make_shared<SynchronizedBatcher<Sha256>>(
      1, certificate, CertificateVerifyContext::Server);
  BatchSignatureAsyncSelfCert<Sha256> batchSelfCert(batcher);
  folly::ManualExecutor executor;
  auto signature = batchSelfCert.signFuture(
      SignatureScheme::ecdsa_secp256r1_sha256_batch,
      CertificateVerifyContext::Server,
      folly::IOBuf::copyBuffer(folly::StringPiece("Message1")));
  executor.drain();
  auto signatureBuf = *std::move(signature).get();

  auto peerCert = std::make_shared<OpenSSLPeerCertImpl<KeyType::P256>>(
      getCert(kP256Certificate));
  BatchSignaturePeerCert batchPeerCert(peerCert);
  // normal verify
  EXPECT_NO_THROW(batchPeerCert.verify(
      SignatureScheme::ecdsa_secp256r1_sha256_batch,
      CertificateVerifyContext::Server,
      folly::range(folly::StringPiece("Message1")),
      signatureBuf->coalesce()));

  // throw when signature's index is larger than 2^31
  folly::io::Cursor cursor(signatureBuf.get());
  auto decodedSig = BatchSignature::decode(cursor);
  LOG(INFO) << decodedSig.getIndex();
  MerkleTreePath newPath{
      .index = std::numeric_limits<uint32_t>::max(),
      .path = decodedSig.getPath()};
  BatchSignature newSig1(std::move(newPath), decodedSig.getSignature());
  EXPECT_THROW(
      batchPeerCert.verify(
          SignatureScheme::ecdsa_secp256r1_sha256_batch,
          CertificateVerifyContext::Server,
          folly::range(folly::StringPiece("Message1")),
          newSig1.encode()->coalesce()),
      std::runtime_error);

  // throw when signature's path length is larger than Sha256::HashLen * 32
  MerkleTreePath newPath2{
      .index = static_cast<uint32_t>(decodedSig.getIndex())};
  size_t badLength = Sha256::HashLen * 33;
  newPath2.path = folly::IOBuf::create(badLength);
  newPath2.path->append(badLength);
  BatchSignature newSig2(std::move(newPath2), decodedSig.getSignature());
  EXPECT_THROW(
      batchPeerCert.verify(
          SignatureScheme::ecdsa_secp256r1_sha256_batch,
          CertificateVerifyContext::Server,
          folly::range(folly::StringPiece("Message1")),
          newSig2.encode()->coalesce()),
      std::runtime_error);
}

} // namespace test
} // namespace client
} // namespace fizz
