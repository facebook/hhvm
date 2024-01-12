/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/crypto/test/TestUtil.h>
#include <fizz/extensions/exportedauth/ExportedAuthenticator.h>
#include <fizz/protocol/OpenSSLSelfCertImpl.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/protocol/test/TestMessages.h>
#include <fizz/record/Extensions.h>
#include <fizz/record/Types.h>
#include <folly/String.h>
#include <folly/ssl/Init.h>

using namespace folly;

namespace fizz {
namespace test {

StringPiece expected_auth_request = {
    "14303132333435363738396162636465666768696a000a000d0006000404030804"};
StringPiece expected_authenticator = {
    "0b000004000000000f00000d040300097369676e617475726514000020b523548c421b05f7f3c33276fbdd5266ba2df103796d7d483368259860a648f2"};
StringPiece expected_cr_context = {"303132333435363738396162636465666768696a"};
StringPiece expected_cert = {
    "308201ee30820195a003020102020900c569eec901ce86d9300a06082a8648ce3d0403023054310b3009060355040613025553310b300906035504080c024e59310b300906035504070c024e59310d300b060355040a0c0446697a7a310d300b060355040b0c0446697a7a310d300b06035504030c0446697a7a301e170d3137303430343138323930395a170d3431313132343138323930395a3054310b3009060355040613025553310b300906035504080c024e59310b300906035504070c024e59310d300b060355040a0c0446697a7a310d300b060355040b0c0446697a7a310d300b06035504030c0446697a7a3059301306072a8648ce3d020106082a8648ce3d030107034200049d87bcaddb65d8dcf6df8b148a9679b5b710db19c95a9badfff13468cb358b4e21d24a5c826112658ebb96d64e2985dfb41c1948334391a4aa81b67837e2dbf0a350304e301d0603551d0e041604143c5b8ba954d9752faf3c8ad6d1a62449dccaa850301f0603551d230418301680143c5b8ba954d9752faf3c8ad6d1a62449dccaa850300c0603551d13040530030101ff300a06082a8648ce3d04030203470030440220349b7d34d7132fb2756576e0bfa36cbe1723337a7a6f5ef9c8d3bf1aa7efa4a5022025c50a91e0aa4272f1f52c3d5583a7d7cee14b178835273a0bd814303e62d714"};
StringPiece expected_empty_authenticator = {
    "1400002011fae4bcdf4673b6dfb276d886c4cd1c5b0920da961643f062d1d4a6062115b1"};

TEST(ExportedAuthenticatorTest, TestAuthenticatorRequest) {
  auto buf = folly::IOBuf::copyBuffer(unhexlify(expected_auth_request));
  folly::io::Cursor cursor(buf.get());
  CertificateRequest cr = decode<CertificateRequest>(cursor);
  EXPECT_EQ(cr.certificate_request_context->computeChainDataLength(), 20);
  EXPECT_EQ(cr.extensions.size(), 1);
  EXPECT_TRUE(getExtension<SignatureAlgorithms>(cr.extensions).has_value());
  auto encodedAuthRequest = ExportedAuthenticator::getAuthenticatorRequest(
      std::move(cr.certificate_request_context), std::move(cr.extensions));
  EXPECT_EQ(
      expected_auth_request,
      StringPiece(hexlify(encodedAuthRequest->coalesce())));
}

TEST(ExportedAuthenticatorTest, TestEmptyAuthenticatorRequest) {
  EXPECT_THROW(
      ExportedAuthenticator::getAuthenticatorRequest(
          nullptr, std::vector<fizz::Extension>()),
      FizzException);
  auto emptyContext = folly::IOBuf::create(0);
  EXPECT_THROW(
      ExportedAuthenticator::getAuthenticatorRequest(
          std::move(emptyContext), std::vector<fizz::Extension>()),
      FizzException);
}

class AuthenticatorTest : public ::testing::Test {
 public:
  void SetUp() override {
    folly::ssl::init();
    CertificateRequest cr;
    cr.certificate_request_context =
        folly::IOBuf::copyBuffer("0123456789abcdefghij");
    SignatureAlgorithms sigAlgs;
    sigAlgs.supported_signature_algorithms.push_back(
        SignatureScheme::ecdsa_secp256r1_sha256);
    cr.extensions.push_back(encodeExtension(std::move(sigAlgs)));
    auto authRequest = encode<CertificateRequest>(std::move(cr));
    authrequest_ = std::move(authRequest);
    CipherSuite cipher = CipherSuite::TLS_AES_128_GCM_SHA256;
    deriver_ = OpenSSLFactory().makeKeyDeriver(cipher);
    handshakeContext_ =
        folly::IOBuf::copyBuffer("12345678901234567890123456789012");
    finishedKey_ = folly::IOBuf::copyBuffer("12345678901234567890123456789012");
    schemes_.push_back(SignatureScheme::ecdsa_secp256r1_sha256);
  }

 protected:
  std::unique_ptr<KeyDerivation> deriver_;
  std::vector<SignatureScheme> schemes_;
  Buf authrequest_;
  Buf handshakeContext_;
  Buf finishedKey_;
};

TEST_F(AuthenticatorTest, TestValidAuthenticator) {
  auto mockCert = std::make_shared<MockSelfCert>();
  EXPECT_CALL(*mockCert, _getCertMessage(_)).WillOnce(InvokeWithoutArgs([]() {
    return TestMessages::certificate();
  }));
  EXPECT_CALL(*mockCert, getSigSchemes())
      .WillOnce(Return(std::vector<SignatureScheme>(
          1, SignatureScheme::ecdsa_secp256r1_sha256)));
  EXPECT_CALL(*mockCert, sign(_, CertificateVerifyContext::Authenticator, _))
      .WillOnce(
          InvokeWithoutArgs([]() { return IOBuf::copyBuffer("signature"); }));

  auto reencodedAuthenticator = ExportedAuthenticator::makeAuthenticator(
      deriver_,
      schemes_,
      *mockCert,
      std::move(authrequest_),
      std::move(handshakeContext_),
      std::move(finishedKey_),
      CertificateVerifyContext::Authenticator);
  EXPECT_EQ(
      expected_authenticator,
      (StringPiece(hexlify(reencodedAuthenticator->coalesce()))));
}

TEST_F(AuthenticatorTest, TestEmptyAuthenticator) {
  auto mockCert = std::make_shared<MockSelfCert>();
  EXPECT_CALL(*mockCert, getSigSchemes())
      .WillOnce(Return(std::vector<SignatureScheme>(
          1, SignatureScheme::ecdsa_secp256r1_sha256)));
  schemes_.clear();
  auto reencodedAuthenticator = ExportedAuthenticator::makeAuthenticator(
      deriver_,
      schemes_,
      *mockCert,
      std::move(authrequest_),
      std::move(handshakeContext_),
      std::move(finishedKey_),
      CertificateVerifyContext::Authenticator);
  EXPECT_EQ(
      expected_empty_authenticator,
      StringPiece(hexlify(reencodedAuthenticator->coalesce())));
}

TEST(ExportedAuthenticatorTest, TestGetContext) {
  StringPiece authenticator = {
      "0b00020f14303132333435363738396162636465666768696a0001f70001f2308201ee30820195a003020102020900c569eec901ce86d9300a06082a8648ce3d0403023054310b3009060355040613025553310b300906035504080c024e59310b300906035504070c024e59310d300b060355040a0c0446697a7a310d300b060355040b0c0446697a7a310d300b06035504030c0446697a7a301e170d3137303430343138323930395a170d3431313132343138323930395a3054310b3009060355040613025553310b300906035504080c024e59310b300906035504070c024e59310d300b060355040a0c0446697a7a310d300b060355040b0c0446697a7a310d300b06035504030c0446697a7a3059301306072a8648ce3d020106082a8648ce3d030107034200049d87bcaddb65d8dcf6df8b148a9679b5b710db19c95a9badfff13468cb358b4e21d24a5c826112658ebb96d64e2985dfb41c1948334391a4aa81b67837e2dbf0a350304e301d0603551d0e041604143c5b8ba954d9752faf3c8ad6d1a62449dccaa850301f0603551d230418301680143c5b8ba954d9752faf3c8ad6d1a62449dccaa850300c0603551d13040530030101ff300a06082a8648ce3d04030203470030440220349b7d34d7132fb2756576e0bfa36cbe1723337a7a6f5ef9c8d3bf1aa7efa4a5022025c50a91e0aa4272f1f52c3d5583a7d7cee14b178835273a0bd814303e62d71400000f00004a04030046304402204ee36706cefd7b5de1b87eef8a756b1f69365451cae050163e030d7cb7594fbc022040aaadc7770b0404c5deb6fd9d9a2161423fb993a0a5b9e38f2c0e0d9183a52d1400002001dd31f46369c46fe41712e83ae7c46d31fdae816024edbb3b58cc29e2234852"};
  auto buf = folly::IOBuf::copyBuffer(unhexlify(authenticator));
  auto certRequestContext =
      ExportedAuthenticator::getAuthenticatorContext(std::move(buf));
  EXPECT_EQ(
      expected_cr_context,
      StringPiece(hexlify(certRequestContext->coalesce())));
}

class ValidateAuthenticatorTest : public ::testing::Test {
 public:
  void SetUp() override {
    folly::ssl::init();
    CipherSuite cipher = CipherSuite::TLS_AES_128_GCM_SHA256;
    deriver_ = OpenSSLFactory().makeKeyDeriver(cipher);
    schemes_.push_back(SignatureScheme::ecdsa_secp256r1_sha256);
    authrequest_ = {
        "14303132333435363738396162636465666768696a0008000d000400020403"};
    handshakeContext_ = {"12345678901234567890123456789012"};
    finishedKey_ = {"12345678901234567890123456789012"};
  }

 protected:
  std::unique_ptr<KeyDerivation> deriver_;
  std::vector<SignatureScheme> schemes_;
  StringPiece authrequest_;
  StringPiece handshakeContext_;
  StringPiece finishedKey_;
};

TEST_F(ValidateAuthenticatorTest, TestValidateValidAuthenticator) {
  auto cert = fizz::test::getCert(kP256Certificate);
  auto key = fizz::test::getPrivateKey(kP256Key);
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(std::move(cert));
  OpenSSLSelfCertImpl<KeyType::P256> certificate(
      std::move(key), std::move(certs));
  auto authenticatorRequest = folly::IOBuf::copyBuffer(unhexlify(authrequest_));
  auto handshakeContext =
      folly::IOBuf::copyBuffer(unhexlify(handshakeContext_));
  auto finishedMacKey = folly::IOBuf::copyBuffer(unhexlify(finishedKey_));
  auto authenticator = ExportedAuthenticator::makeAuthenticator(
      deriver_,
      schemes_,
      certificate,
      std::move(authenticatorRequest),
      std::move(handshakeContext),
      std::move(finishedMacKey),
      CertificateVerifyContext::Authenticator);

  authenticatorRequest = folly::IOBuf::copyBuffer(unhexlify(authrequest_));
  handshakeContext = folly::IOBuf::copyBuffer(unhexlify(handshakeContext_));
  finishedMacKey = folly::IOBuf::copyBuffer(unhexlify(finishedKey_));
  auto decodedCerts = ExportedAuthenticator::validate(
      deriver_,
      std::move(authenticatorRequest),
      std::move(authenticator),
      std::move(handshakeContext),
      std::move(finishedMacKey),
      CertificateVerifyContext::Authenticator);
  EXPECT_TRUE(decodedCerts.has_value());
  EXPECT_EQ((*decodedCerts).size(), 1);
  EXPECT_EQ(
      expected_cert,
      StringPiece(hexlify(((*decodedCerts)[0].cert_data)->coalesce())));
}

TEST_F(ValidateAuthenticatorTest, TestValidateEmptyAuthenticator) {
  auto cert = fizz::test::getCert(kP256Certificate);
  auto key = fizz::test::getPrivateKey(kP256Key);
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(std::move(cert));
  OpenSSLSelfCertImpl<KeyType::P256> certificate(
      std::move(key), std::move(certs));
  schemes_.clear();
  auto authenticatorRequest = folly::IOBuf::copyBuffer(unhexlify(authrequest_));
  auto handshakeContext =
      folly::IOBuf::copyBuffer(unhexlify(handshakeContext_));
  auto finishedMacKey = folly::IOBuf::copyBuffer(unhexlify(finishedKey_));
  auto authenticator = ExportedAuthenticator::makeAuthenticator(
      deriver_,
      schemes_,
      certificate,
      std::move(authenticatorRequest),
      std::move(handshakeContext),
      std::move(finishedMacKey),
      CertificateVerifyContext::Authenticator);

  authenticatorRequest = folly::IOBuf::copyBuffer(unhexlify(authrequest_));
  handshakeContext = folly::IOBuf::copyBuffer(unhexlify(handshakeContext_));
  finishedMacKey = folly::IOBuf::copyBuffer(unhexlify(finishedKey_));
  auto decodedCerts = ExportedAuthenticator::validate(
      deriver_,
      std::move(authenticatorRequest),
      std::move(authenticator),
      std::move(handshakeContext),
      std::move(finishedMacKey),
      CertificateVerifyContext::Authenticator);
  EXPECT_TRUE(decodedCerts.has_value());
  EXPECT_EQ((*decodedCerts).size(), 0);
}

} // namespace test
} // namespace fizz
